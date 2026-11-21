#pragma once

// 仅测试使用的独立 MAT v5 标签解析器，从既有582文件L2测试提取。
// 不调用 matio 或生产Reader，只支持此固定归档的有界little-endian压缩数组。
#include <zlib.h>

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace archive_reference {
namespace fs=std::filesystem;
using Bytes=std::vector<unsigned char>;
// MAT v5 规定的类型/头长度，不是求解器参数。
constexpr std::uint32_t kInt8=1,kInt32=5,kUint32=6,kDouble=9,kMatrix=14,kCompressed=15;
constexpr std::uint32_t kDoubleClass=6,kComplexFlag=0x800,kLogicalFlag=0x200;
constexpr std::size_t kHeaderBytes=128,kTagBytes=8;
constexpr std::size_t kFixtureByteLimit=1024*1024; // 仅本测试固定归档的读取上界。
inline void require(bool condition,const std::string& message) {
    if(!condition)throw std::runtime_error(message);
}
inline std::uint32_t u32(const Bytes& bytes,std::size_t offset) {
    require(offset<=bytes.size()&&bytes.size()-offset>=4,"Truncated uint32");
    std::uint32_t value=0;
    for(unsigned i=0;i<4;++i)value|=std::uint32_t(bytes[offset+i])<<(8*i);
    return value;
}
struct Element {
    std::uint32_t type;
    std::size_t bytes,start,end,next;
};
inline Element element(const Bytes& buffer,std::size_t offset) {
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
inline Bytes readFile(const fs::path& path) {
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
inline Bytes inflateMatrix(const Bytes& raw) {
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
inline Reference decodeIndependent(const fs::path& path) {
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
} // namespace archive_reference
