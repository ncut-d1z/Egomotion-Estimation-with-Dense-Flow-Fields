// 独立 L2 Reader 对照：按 MAT v5 标签解析原始值，参考路径不调用 matio。
// 限定为本仓库单变量、little-endian、zlib、二维 mxDOUBLE 归档。
// 格式依据 MathWorks MAT-File Format；不执行 MATLAB，不作为生产 Reader。
#include "egomotion/io/mat_result_reader.hpp"
#include "egomotion/io/sha256.hpp"

#include <zlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
namespace fs=std::filesystem;
using Bytes=std::vector<unsigned char>;
// MAT v5 规定的类型/头长度，不是求解器参数。
constexpr std::uint32_t kInt8=1,kInt32=5,kUint32=6,kDouble=9,kMatrix=14,kCompressed=15;
constexpr std::uint32_t kDoubleClass=6,kComplexFlag=0x800,kLogicalFlag=0x200;
constexpr std::size_t kHeaderBytes=128,kTagBytes=8;
constexpr std::size_t kFixtureByteLimit=1024*1024; // 仅本测试固定归档的读取上界。
// 固定原始归档及独立身份清单的实测数量，不是可调通过阈值。
constexpr std::size_t kArchiveFiles=582,kPackedInt32Arrays=80,kDoubleArrays=502;
void require(bool condition,const std::string& message) {
    if(!condition)throw std::runtime_error(message);
}
std::uint32_t u32(const Bytes& bytes,std::size_t offset) {
    require(offset<=bytes.size()&&bytes.size()-offset>=4,"Truncated uint32");
    std::uint32_t value=0;
    for(unsigned i=0;i<4;++i)value|=std::uint32_t(bytes[offset+i])<<(8*i);
    return value;
}
struct Element {
    std::uint32_t type;
    std::size_t bytes,start,end,next;
};
Element element(const Bytes& buffer,std::size_t offset) {
    require(offset<=buffer.size()&&buffer.size()-offset>=kTagBytes,"Truncated tag");
    const auto word=u32(buffer,offset),small=word>>16;
    const auto bytes=small?small:u32(buffer,offset+4);
    require(small<=4,"Invalid small-data-element length");
    const auto start=offset+(small?4:kTagBytes);
    require(bytes<=buffer.size()-start,"Tag payload exceeds buffer");
    const auto end=start+bytes;
    return {small?word&0xffff:word,bytes,start,end,
            small?offset+kTagBytes:start+((std::size_t(bytes)+7)/8)*8};
}
Bytes readFile(const fs::path& path) {
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    require(file.is_open(),"Cannot open independent reference file: "+path.string());
    const auto length=file.tellg();
    require(length>=0&&length<=static_cast<std::streamoff>(kFixtureByteLimit),
            "Independent fixture exceeds bounded input size");
    Bytes bytes(static_cast<std::size_t>(length));
    file.seekg(0);
    require(bool(file.read(reinterpret_cast<char*>(bytes.data()),length)),"Incomplete file read");
    return bytes;
}
Bytes inflateMatrix(const Bytes& raw) {
    require(raw.size()>=kHeaderBytes+kTagBytes&&raw[124]==0&&raw[125]==1&&
            raw[126]=='I'&&raw[127]=='M',"Expected little-endian MAT v5 header");
    const auto compressed=element(raw,kHeaderBytes);
    require(compressed.type==kCompressed&&compressed.end==raw.size(),
            "Expected exactly one compressed array (no inferred padding)");
    Bytes input(raw.begin()+compressed.start,raw.begin()+compressed.end);
    Bytes output(kFixtureByteLimit);
    z_stream stream{};
    stream.next_in=input.data();
    stream.avail_in=static_cast<uInt>(compressed.bytes);
    stream.next_out=output.data();stream.avail_out=static_cast<uInt>(output.size());
    require(::inflateInit(&stream)==Z_OK,"Independent zlib init failed");
    const auto status=::inflate(&stream,Z_FINISH);
    const auto consumed=stream.total_in,written=stream.total_out;
    const auto ended=::inflateEnd(&stream);
    require(status==Z_STREAM_END&&ended==Z_OK&&consumed==compressed.bytes,
            "Compressed array truncated, trailing data present or output bound exceeded");
    output.resize(written);return output;
}
struct Reference {
    std::string name;
    std::uint32_t storage_type=0;
    std::size_t rows=0,cols=0;
    std::vector<double> values;
};
Reference decodeIndependent(const fs::path& path) {
    const auto data=inflateMatrix(readFile(path));
    const auto matrix=element(data,0);
    require(matrix.type==kMatrix&&matrix.end==data.size(),"Unexpected matrix envelope");
    const auto flags=element(data,matrix.start),dims=element(data,flags.next);
    const auto name=element(data,dims.next),real=element(data,name.next);
    require(flags.type==kUint32&&flags.bytes==8&&dims.type==kInt32&&dims.bytes==8&&
            name.type==kInt8&&real.next==matrix.end,"Unsupported independent array layout");
    const auto array_flags=u32(data,flags.start);
    require((array_flags&0xff)==kDoubleClass&&!(array_flags&(kComplexFlag|kLogicalFlag)),
            "Expected real non-logical MATLAB double class");
    Reference result;
    result.name.assign(reinterpret_cast<const char*>(data.data()+name.start),name.bytes);
    result.rows=u32(data,dims.start);result.cols=u32(data,dims.start+4);
    result.storage_type=real.type;
    require(result.rows>0&&result.cols>0&&result.cols<=kFixtureByteLimit/result.rows,
            "Invalid independent shape");
    require(real.type==kDouble||real.type==kInt32,"Unexpected raw storage type");
    const auto width=real.type==kDouble?8U:4U;
    const auto count=result.rows*result.cols;
    require(count==real.bytes/width&&real.bytes%width==0,"Raw byte count does not match shape");
    result.values.reserve(count);
    for(std::size_t i=0;i<count;++i){
        const auto offset=real.start+i*width;
        double value=0;
        if(real.type==kDouble){
            const std::uint64_t word=u32(data,offset)|(std::uint64_t(u32(data,offset+4))<<32);
            std::memcpy(&value,&word,sizeof(value));
        }else{
            const auto word=u32(data,offset);
            // miINT32 的每一个整数都能精确表示成 IEEE double；没有 single 升级。
            const std::int64_t signed_value=word<=INT32_MAX?std::int64_t(word):
                std::int64_t(word)-(std::int64_t(1)<<32);
            value=static_cast<double>(signed_value);
        }
        result.values.push_back(value);
    }
    return result;
}
void checkFile(const fs::path& root,const std::string& relative,const std::string& expected_sha,
               std::uint32_t storage_type,std::size_t rows,std::size_t cols,const std::string& name) {
    using namespace egomotion;
    const fs::path path=root/relative;
    require(!fs::is_symlink(fs::symlink_status(path)),"Reference path cannot be a symlink");
    const auto reference=decodeIndependent(path);
    require(reference.name==name&&reference.rows==rows&&reference.cols==cols&&
            reference.storage_type==storage_type,"Raw fields differ from independent inventory: "+relative);
    const int fd=::open(path.c_str(),O_RDONLY|O_CLOEXEC|O_NOFOLLOW);
    require(fd>=0,"Cannot open SHA reference");
    const auto digest=io::sha256Fd(fd);::close(fd);
    require(digest.hasValue()&&digest.value()==expected_sha,"SHA-256 differs from fixed inventory: "+relative);
    const auto decoded=io::MatResultReader{}.readNamed(path,name);
    require(decoded.hasValue(),"Production Reader failed: "+relative);
    const auto& value=decoded.value();
    require(value.info.dimensions==std::vector<std::size_t>({rows,cols})&&
            value.info.original_class_type==kDoubleClass&&value.info.original_data_type==kDouble&&
            !value.info.is_complex&&!value.info.is_logical&&
            value.info.data_bytes==reference.values.size()*sizeof(double)&&
            value.values_column_major.size()==reference.values.size()&&
            std::memcmp(value.values_column_major.data(),reference.values.data(),value.info.data_bytes)==0,
            "Production decoded values/column order differ bitwise from independent parser: "+relative);
}
} // namespace
int main(int argc,char** argv) {
    static_assert(sizeof(double)==8&&std::numeric_limits<double>::is_iec559,
                  "Independent MAT reference needs IEEE binary64");
    try{
        require(argc==3,"Expected original archive and fixed independent inventory");
        const fs::path root(argv[1]);
        std::ifstream inventory(argv[2]);
        require(inventory.is_open(),"Cannot open independent inventory");
        std::string line;std::set<std::string> files;
        std::size_t int32_arrays=0,double_arrays=0,elements=0;
        while(std::getline(inventory,line)){
            if(line.empty()||line.front()=='#')continue;
            std::istringstream fields(line);
            std::string sha,name,relative;std::uint32_t storage=0;std::size_t rows=0,cols=0;
            require(bool(fields>>sha>>storage>>rows>>cols>>name>>relative),"Malformed reference row");
            std::string trailing;
            require(!(fields>>trailing)&&sha.size()==64&&files.insert(relative).second&&
                    !fs::path(relative).is_absolute()&&relative.find("..")==std::string::npos,
                    "Duplicate, unsafe or malformed reference identity");
            checkFile(root,relative,sha,storage,rows,cols,name);
            int32_arrays+=storage==kInt32;double_arrays+=storage==kDouble;elements+=rows*cols;
        }
        require(files.size()==kArchiveFiles&&int32_arrays==kPackedInt32Arrays&&double_arrays==kDoubleArrays,
                "Independent file/type inventory differs from the original archive");
        std::cout<<"PASS independent L2 Reader: files="<<files.size()<<" elements="<<elements
                 <<" mxDOUBLE/miINT32="<<int32_arrays<<" mxDOUBLE/miDOUBLE="<<double_arrays
                 <<"; SHA-256 and column-major binary64 values exact (candidate reference)\n";
        return 0;
    }catch(const std::exception& error){std::cerr<<"FAIL independent MAT: "<<error.what()<<'\n';return 1;}
}
