// 独立按MAT v5字节布局构造小夹具，不用matio生成预期输入；原始MAT只读。
#include "egomotion/io/mat_result_reader.hpp"
#include "egomotion/io/run_directory.hpp"
#include "egomotion/io/sha256.hpp"

#include <matio.h>
#include <zlib.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace {std::size_t numeric_read_calls=0;}
extern "C" matvar_t* __real_Mat_VarRead(mat_t*,const char*);
extern "C" matvar_t* __wrap_Mat_VarRead(mat_t* file,const char* name){
    ++numeric_read_calls;return __real_Mat_VarRead(file,name);
}

namespace {
using namespace egomotion;
namespace fs=std::filesystem;
using Bytes=std::vector<unsigned char>;
constexpr std::uint32_t kInt8=1,kInt32=5,kUint32=6,kSingle=7,kDouble=9,kMatrix=14,kCompressed=15,kUtf8=16;
constexpr std::size_t kHeaderBytes=128,kTagBytes=8;
const std::vector<double> kValues{1.0,-0.0,-3.5,4.0,0.25,6.0};
void require(bool condition,const std::string& message){if(!condition)throw std::runtime_error(message);}
void word(Bytes& out,std::uint64_t value,std::size_t width,bool little){
    for(std::size_t i=0;i<width;++i)
        out.push_back(static_cast<unsigned char>(value>>(8*(little?i:width-1-i))));
}
void replaceWord(Bytes& out,std::size_t offset,std::uint32_t value,bool little){
    Bytes encoded;word(encoded,value,4,little);
    for(std::size_t i=0;i<encoded.size();++i)out.at(offset+i)=encoded[i];
}
void element(Bytes& out,std::uint32_t type,const Bytes& payload,bool little,bool small=false){
    if(small){
        require(!payload.empty()&&payload.size()<=4,"Invalid small-element fixture");
        word(out,(static_cast<std::uint32_t>(payload.size())<<16)|type,4,little);
        out.insert(out.end(),payload.begin(),payload.end());
        out.insert(out.end(),4-payload.size(),0);
    }else{
        word(out,type,4,little);word(out,payload.size(),4,little);
        out.insert(out.end(),payload.begin(),payload.end());
        out.insert(out.end(),(kTagBytes-payload.size()%kTagBytes)%kTagBytes,0);
    }
}
Bytes numbers(const std::vector<double>& values,std::uint32_t type,bool little){
    Bytes out;
    for(const double value:values){
        if(type==kDouble){std::uint64_t bits;std::memcpy(&bits,&value,sizeof(bits));word(out,bits,8,little);}
        else if(type==kSingle){
            const float narrowed=static_cast<float>(value);std::uint32_t bits;
            std::memcpy(&bits,&narrowed,sizeof(bits));word(out,bits,4,little);
        }else if(type==kUtf8)word(out,static_cast<unsigned char>(value),1,little);
        else word(out,static_cast<std::uint32_t>(static_cast<std::int32_t>(value)),4,little);
    }
    return out;
}
struct Fixture {
    std::string label,name="value";
    std::optional<std::string> requested_name;
    std::vector<std::uint32_t> dims{2,3};
    std::vector<double> values=kValues;
    std::uint32_t storage=kDouble;
    std::uint32_t class_type=6;
    bool small_name=false,small_real=false;
    bool single_neighbor=false;
    std::string single_neighbor_name="other";
    std::optional<std::string> additional_name;
    bool additional_single_class=false;
    std::optional<MatErrorCode> rejected;
    std::string mutation;
};
Bytes matrix(const Fixture& fixture,bool little){
    Bytes body,flags,dims;
    word(flags,fixture.class_type,4,little);word(flags,0,4,little); // 非logical/complex。
    element(body,kUint32,flags,little);
    for(auto value:fixture.dims)word(dims,value,4,little);
    element(body,kInt32,dims,little);
    element(body,kInt8,Bytes(fixture.name.begin(),fixture.name.end()),little,fixture.small_name);
    const std::size_t data_offset=body.size();
    auto payload=numbers(fixture.values,fixture.storage,little);
    if(fixture.mutation=="partial-number")payload.pop_back();
    element(body,fixture.storage,payload,little,fixture.small_real);
    if(fixture.mutation=="tag-overrun")replaceWord(body,data_offset+4,0x7fffffff,little);
    if(fixture.mutation=="flags-length")replaceWord(body,4,16,little);
    if(fixture.mutation=="dims-type")replaceWord(body,16,kUint32,little);
    if(fixture.mutation=="dims-byte-count")replaceWord(body,20,7,little);
    if(fixture.mutation=="unknown-storage")replaceWord(body,data_offset,99,little);
    Bytes out;element(out,kMatrix,body,little);
    if(fixture.mutation=="root-short")replaceWord(out,4,body.size()-kTagBytes,little);
    if(fixture.mutation=="root-long")replaceWord(out,4,body.size()+kTagBytes,little);
    if(fixture.mutation=="extra-element")element(out,kDouble,numbers({7.0},kDouble,little),little);
    return out;
}
Bytes fileBytes(const Fixture& fixture,bool little,bool compressed){
    Bytes out(kHeaderBytes,0);
    const std::string header="MATLAB 5.0 MAT-file, R6 independent payload fixture";
    std::copy(header.begin(),header.end(),out.begin());
    out[little?125:124]=1;out[126]=little?'I':'M';out[127]=little?'M':'I';
    auto raw=matrix(fixture,little);
    if(compressed){
        Bytes packed(::compressBound(raw.size()));uLongf size=packed.size();
        require(::compress2(packed.data(),&size,raw.data(),raw.size(),Z_BEST_SPEED)==Z_OK,
                "Fixture compression failed");
        packed.resize(size);word(out,kCompressed,4,little);word(out,packed.size(),4,little);
        out.insert(out.end(),packed.begin(),packed.end()); // miCOMPRESSED后没有8字节补齐。
    }else out.insert(out.end(),raw.begin(),raw.end());
    if(fixture.single_neighbor){
        Fixture other;other.name=fixture.single_neighbor_name;other.storage=kSingle;
        const auto encoded=fileBytes(other,little,compressed);
        out.insert(out.end(),encoded.begin()+kHeaderBytes,encoded.end());
    }
    if(fixture.additional_name){
        Fixture other;other.name=*fixture.additional_name;other.dims={1,2};other.values={7,-2};
        if(fixture.additional_single_class){other.class_type=MAT_C_SINGLE;other.storage=kSingle;}
        const auto encoded=fileBytes(other,little,compressed);
        out.insert(out.end(),encoded.begin()+kHeaderBytes,encoded.end());
    }
    return out;
}
void save(const fs::path& path,const Bytes& bytes){
    require(!fs::exists(path),"Refuse fixture overwrite");
    std::ofstream output(path,std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()));
    output.close();require(bool(output),"Cannot write fixture");
}
std::string digest(const fs::path& path){
    const int fd=::open(path.c_str(),O_RDONLY|O_NOFOLLOW|O_CLOEXEC);
    require(fd>=0,"Cannot open fixture for SHA");
    const auto value=io::sha256Fd(fd);::close(fd);
    require(value.hasValue(),"Cannot hash fixture");return value.value();
}
void checkDirectory(const fs::path& path,const Fixture& fixture){
    io::MatResultReader reader;
    const auto ambiguous=[&](const auto& result,const std::string& variable){
        require(!result.hasValue()&&result.error().code==MatErrorCode::AmbiguousVariable,
                "Duplicate or ambiguous directory must not select a variable");
        require(result.error().detail.source_path==path.string()&&
                result.error().detail.variable==variable&&result.error().detail.stage==ErrorStage::Read,
                "Ambiguous directory lost source/name/stage");
    };
    if(fixture.rejected){
        ambiguous(reader.inspect(path),fixture.name);
        ambiguous(reader.scanFile(path,io::ScanMode::MetadataOnly),fixture.name);
        ambiguous(reader.scanFile(path,io::ScanMode::VerifyDecoded),fixture.name);
        ambiguous(reader.readNamed(path,fixture.name),fixture.name);
        ambiguous(reader.readFirstCompatible(path),{});
        require(numeric_read_calls==0,"Ambiguous directory reached numeric decoding");
        return;
    }
    const auto metadata=reader.inspect(path);
    require(metadata.hasValue()&&metadata.value().size()==2,"Unique names must retain both structures");
    const auto checkInfo=[&](const MatVariableInfo& first,const MatVariableInfo& second){
        require(first.name==fixture.name&&first.original_class_type==MAT_C_DOUBLE&&
                first.dimensions==std::vector<std::size_t>({2,3}),"First directory structure changed");
        require(second.name==*fixture.additional_name&&
                second.original_class_type==(fixture.additional_single_class?MAT_C_SINGLE:MAT_C_DOUBLE)&&
                second.dimensions==std::vector<std::size_t>({1,2}),"Second directory structure was aliased");
    };
    checkInfo(metadata.value()[0],metadata.value()[1]);
    const auto info=reader.scanFile(path,io::ScanMode::MetadataOnly);
    require(info.hasValue()&&info.value().variables.size()==2,"Unique metadata scan failed");
    checkInfo(info.value().variables[0].info,info.value().variables[1].info);
    for(const auto& entry:info.value().variables)require(entry.status=="METADATA_OK","Metadata status changed");
    require(numeric_read_calls==0,"Directory inspection decoded numeric data");
    const auto decoded=reader.scanFile(path,io::ScanMode::VerifyDecoded);
    require(decoded.hasValue()&&decoded.value().variables.size()==2,"Unique full scan failed");
    checkInfo(decoded.value().variables[0].info,decoded.value().variables[1].info);
    for(const auto& entry:decoded.value().variables)require(entry.status=="READ_OK","Full scan status changed");
    const auto named=reader.readNamed(path,fixture.name);
    require(named.hasValue()&&named.value().values_column_major.size()==fixture.values.size()&&
            std::memcmp(named.value().values_column_major.data(),fixture.values.data(),
                        fixture.values.size()*sizeof(double))==0,"Unique named payload changed");
    ambiguous(reader.readFirstCompatible(path),{});
    require(numeric_read_calls==3,"Unique directory did not decode each intended variable exactly once");
}
void check(const fs::path& path,const Fixture& fixture){
    if(fixture.additional_name){checkDirectory(path,fixture);return;}
    io::MatResultReader reader;
    const auto& requested=fixture.requested_name?*fixture.requested_name:fixture.name;
    for(const bool compatible:{false,true}){
        const auto result=compatible?reader.readFirstCompatible(path):reader.readNamed(path,requested);
        if(compatible&&fixture.single_neighbor){
            require(!result.hasValue(),"Compatible reader guessed a value from a mixed file");
        }else if(fixture.rejected){
            require(!result.hasValue(),"Malformed/unsupported payload was silently accepted");
            require(result.error().code==*fixture.rejected,
                    "Wrong error: "+std::string(errorName(result.error().code))+" "+result.error().detail.message);
            require(result.error().detail.source_path==path.string(),"Failure lost source identity");
            if(!compatible)require(result.error().detail.variable==requested,"Failure lost requested variable");
        }else{
            require(result.hasValue(),result.hasValue()?"":"Valid fixture rejected: "+result.error().detail.message);
            const auto& data=result.value();
            require(data.info.name==requested&&data.info.dimensions==
                    std::vector<std::size_t>(fixture.dims.begin(),fixture.dims.end()),"Raw fixture shape/name changed");
            require(data.values_column_major.size()==fixture.values.size()&&
                    std::memcmp(data.values_column_major.data(),fixture.values.data(),fixture.values.size()*sizeof(double))==0,
                    "Raw fixture value/order/negative zero changed");
        }
    }
    const auto scan=reader.scanFile(path,io::ScanMode::VerifyDecoded);
    require((fixture.rejected||fixture.single_neighbor)?!scan.hasValue():scan.hasValue(),
            "Full dictionary scan disagrees with raw payload validity");
    const std::size_t expected_reads=fixture.rejected?0:(fixture.single_neighbor?1:3);
    require(numeric_read_calls==expected_reads,"Invalid storage reached matio numeric allocation/decoding");
}
bool isolatedCheck(const fs::path& path,const Fixture& fixture){
    std::cout.flush();std::cerr.flush();
    const pid_t child=::fork();require(child>=0,"Cannot isolate malformed-input test");
    if(child==0){
        // 仅限制本测试子进程，防止旧Reader按恶意维度分配；不是生产预算/验收阈值。
        constexpr rlim_t kChildAddressSpace=256*1024*1024,kChildCpuSeconds=5;
        const rlimit memory{kChildAddressSpace,kChildAddressSpace},cpu{kChildCpuSeconds,kChildCpuSeconds};
        if(::setrlimit(RLIMIT_AS,&memory)!=0||::setrlimit(RLIMIT_CPU,&cpu)!=0)::_exit(2);
        try{check(path,fixture);::_exit(0);}
        catch(const std::exception& error){std::cerr<<"FAIL "<<path.filename()<<": "<<error.what()<<'\n';::_exit(1);}
    }
    int status=0;pid_t waited;
    do{waited=::waitpid(child,&status,0);}while(waited<0&&errno==EINTR);
    require(waited==child,"Cannot collect isolated test");
    if(!WIFEXITED(status)||WEXITSTATUS(status)!=0){
        std::cerr<<"Child failure status="<<status<<" file="<<path.filename()<<'\n';return false;
    }
    return true;
}
std::vector<Fixture> fixtures(){
    std::vector<Fixture> out;
    Fixture base;base.label="double";out.push_back(base);
    for(const bool single_class:{false,true}){
        Fixture multiple;multiple.additional_single_class=single_class;
        multiple.label=single_class?"unique-single-class-neighbor":"unique-double-neighbor";
        multiple.additional_name="other";out.push_back(multiple);
        multiple.label=single_class?"duplicate-different-class":"duplicate-different-shape";
        multiple.additional_name="value";multiple.rejected=MatErrorCode::AmbiguousVariable;out.push_back(multiple);
        multiple.label+="-nul-alias";multiple.additional_name=std::string("value\0suffix",12);out.push_back(multiple);
    }
    base.label="named-selection-with-single-neighbor";base.single_neighbor=true;out.push_back(base);
    base.label="named-selection-with-nul-single-neighbor";
    base.single_neighbor_name=std::string("other\0suffix",12);out.push_back(base);
    base.label="named-selection-longer-than-neighbor-prefix";base.name="value_suffix";
    base.single_neighbor_name=std::string("value\0suffix",12);out.push_back(base);
    base.label="named-selection-shorter-than-neighbor-prefix";base.name="value";
    base.single_neighbor_name=std::string("values\0suffix",13);out.push_back(base);
    base=Fixture{};
    base.label="small-name";base.name="v";base.small_name=true;out.push_back(base);
    // 原始名字与matio查找用的C字符串分开；别名不得绕过已有single禁止门禁。
    for(const std::size_t prefix:{1U,5U,127U,128U,129U}){
        Fixture alias;alias.label="nul-name-"+std::to_string(prefix);
        alias.requested_name=std::string(prefix,'v');
        alias.name=*alias.requested_name+std::string("\0x",2);
        alias.small_name=prefix==1;
        out.push_back(alias); // 支持的double仍按同一后端名字可读。
        alias.label+="-single";alias.storage=kSingle;
        alias.rejected=MatErrorCode::UnsupportedClassOrType;out.push_back(std::move(alias));
    }
    base=Fixture{};base.label="int32-packed-double";base.storage=kInt32;base.values={1,-2,3,-4,5,6};out.push_back(base);
    base.label="small-int32-real";base.dims={1,1};base.values={-7};base.small_real=true;out.push_back(base);
    // 总解压长度在8KiB边界前/等于/后；另用固定置换值跨越输入与输出多个缓冲块。
    constexpr std::uint32_t kFixturePermutation=2654435761U;
    for(const std::size_t count:{1015U,1016U,1017U,4096U}){
        Fixture stream;stream.label="stream-boundary-"+std::to_string(count);
        stream.dims={1,static_cast<std::uint32_t>(count)};stream.values.resize(count);
        for(std::size_t i=0;i<count;++i)
            stream.values[i]=static_cast<double>(static_cast<std::uint32_t>(i)*kFixturePermutation)+0.25;
        out.push_back(std::move(stream));
    }
    const auto add=[&](const std::string& label,MatErrorCode code,std::string mutation){
        Fixture value;value.label=label;value.rejected=code;value.mutation=std::move(mutation);out.push_back(value);
    };
    add("short-payload",MatErrorCode::InvalidShape,{});out.back().values.pop_back();
    add("long-payload",MatErrorCode::InvalidShape,{});out.back().values.push_back(7);
    add("partial-number",MatErrorCode::InvalidShape,"partial-number");
    add("single-under-double",MatErrorCode::UnsupportedClassOrType,{});out.back().storage=kSingle;
    add("negative-dimension",MatErrorCode::InvalidShape,{});out.back().dims={0xffffffff,3};
    add("byte-count-overflow",MatErrorCode::SizeOverflow,{});out.back().dims={0x7fffffff,0x7fffffff};
    add("tag-overrun",MatErrorCode::CorruptCompressedData,"tag-overrun");
    add("flags-length",MatErrorCode::InvalidMatHeader,"flags-length");
    add("dims-type",MatErrorCode::InvalidShape,"dims-type");
    add("dims-byte-count",MatErrorCode::InvalidShape,"dims-byte-count");
    add("unknown-storage",MatErrorCode::UnsupportedClassOrType,"unknown-storage");
    add("root-short",MatErrorCode::CorruptCompressedData,"root-short");
    add("root-long",MatErrorCode::CorruptCompressedData,"root-long");
    add("char-root-long",MatErrorCode::CorruptCompressedData,"root-long");
    out.back().class_type=4;out.back().storage=kUtf8;out.back().values={'a','b','c','d','e','f'};
    // 压缩流内必须只有一份miMATRIX，不能忽略剩余解压元素。
    add("extra-element",MatErrorCode::CorruptCompressedData,"extra-element");
    return out;
}
} // namespace
int main(int argc,char** argv){
    try{
        require(argc==2,"Expected build-tree sandbox");
        const fs::path root=fs::absolute(argv[1])/"原始标签 合成";fs::create_directories(root/"origin");
        auto made=io::RunDirectory::create(root);require(made.hasValue(),"Cannot reserve RunId");
        auto run=std::move(made).value();const auto directory=run->outputRoot()/run->runId().text;
        std::cout<<"RunId="<<run->runId().text<<" path="<<directory<<'\n';
        std::size_t total=0,failed=0;
        for(const bool little:{true,false})for(const bool compressed:{false,true})
            for(const auto& fixture:fixtures()){
                if(fixture.label=="extra-element"&&!compressed)continue;
                const auto name=std::string(little?"le-":"be-")+(compressed?"zlib-":"plain-")+fixture.label;
                const auto path=directory/(name+".mat");save(path,fileBytes(fixture,little,compressed));
                const auto before=digest(path);++total;
                const bool passed=isolatedCheck(path,fixture);
                if(!passed)++failed;
                require(digest(path)==before,"Reader modified input fixture");
                std::cout<<(passed?"PASS ":"FAIL ")<<name<<" SHA256="<<before<<'\n';
            }
        std::cout<<"Raw MAT payload scenarios="<<total<<" failed="<<failed<<'\n';
        return failed==0?0:1;
    }catch(const std::exception& error){std::cerr<<"FAIL setup: "<<error.what()<<'\n';return 1;}
}
