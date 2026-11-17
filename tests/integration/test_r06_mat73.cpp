// 真实合成v7.3文件的拒绝路径；普通运行不要求matio有HDF5创建/解码能力。
#include "egomotion/io/mat_result_reader.hpp"
#include "egomotion/io/run_directory.hpp"
#include "egomotion/io/sha256.hpp"
#include <matio.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <array>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

extern "C" {
mat_t* __real_Mat_Open(const char*,int);
matvar_t* __real_Mat_VarRead(mat_t*,const char*);
ssize_t __real_pread(int,void*,size_t,off_t);
}
namespace {
using namespace egomotion;
namespace fs=std::filesystem;
using Bytes=std::vector<unsigned char>;
constexpr std::size_t kHeaderBytes=128,kHdf5Offset=512;
constexpr const char* kFixtureSha="f6b703ff389c19c8a871009f299ffeb5fcb004273dc7f2b290479cf35e29b2a5";
enum class Api {Named,Compatible,FullScan};
enum class Fault {None,Interrupted,Short,ReadError};
struct Observation {
    bool armed=false,deny_backend=false;
    Fault fault=Fault::None;
    std::size_t opens=0,decodes=0,header_reads=0,triggers=0;
} observation;
std::size_t checks=0,cases=0,failures=0;
void require(bool ok,const std::string& message) {++checks;if(!ok)throw std::runtime_error(message);}
template<class Check> void scenario(const std::string& name,Check check) {
    ++cases;
    try {check();std::cout<<"PASS "<<name<<'\n';}
    catch(const std::exception& e){observation.armed=false;++failures;std::cerr<<"FAIL "<<name<<": "<<e.what()<<'\n';}
}
std::string digest(const fs::path& path) {
    const int fd=::open(path.c_str(),O_RDONLY|O_NOFOLLOW|O_CLOEXEC);
    require(fd>=0,"Cannot open fixture for hashing");const auto value=io::sha256Fd(fd);::close(fd);
    require(value.hasValue(),"Cannot hash fixture");return value.value();
}
std::size_t descriptors() {
    return static_cast<std::size_t>(std::distance(fs::directory_iterator("/proc/self/fd"),fs::directory_iterator{}));
}
Bytes load(const fs::path& path) {
    std::ifstream input(path,std::ios::binary);require(bool(input),"Cannot read fixed fixture");
    return Bytes(std::istreambuf_iterator<char>(input),{});
}
void save(const fs::path& path,const Bytes& bytes) {
    require(!fs::exists(path),"Refuse synthetic variant overwrite");
    std::ofstream output(path,std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()));
    output.close();require(bool(output),"Cannot save isolated fixture variant");
}
Status<MatErrorCode> read(Api api,const fs::path& path) {
    const io::MatResultReader reader;
    if(api==Api::FullScan) {
        auto result=reader.scanFile(path,io::ScanMode::VerifyDecoded);
        return result.hasValue()?Status<MatErrorCode>::success():Status<MatErrorCode>::failure(result.error());
    }
    auto result=api==Api::Named?reader.readNamed(path,"value"):reader.readFirstCompatible(path);
    return result.hasValue()?Status<MatErrorCode>::success():Status<MatErrorCode>::failure(result.error());
}
void rejected(const fs::path& path,Api api,Fault fault,bool deny_backend) {
    const auto before=digest(path);
    const auto fds=descriptors();
    observation={true,deny_backend,fault,0,0,0,0};
    const auto result=read(api,path);observation.armed=false;
    std::cout<<"observed api="<<static_cast<int>(api)<<" deny_backend="<<deny_backend
        <<" fault="<<static_cast<int>(fault)<<" code="<<(result.hasValue()?"SUCCESS":errorName(result.error().code))
        <<" Mat_Open="<<observation.opens<<" Mat_VarRead="<<observation.decodes<<'\n';
    const bool io_failure=fault==Fault::ReadError;
    require(!result.hasValue()&&result.error().code==(io_failure?MatErrorCode::InvalidMatHeader:MatErrorCode::UnsupportedClassOrType),
        "Reject the unsupported version independently of HDF5, or preserve the actual header read error");
    require(result.error().detail.stage==(io_failure?ErrorStage::Read:ErrorStage::Decode)&&
        result.error().detail.source_path==path.string()&&
        result.error().detail.variable==(api==Api::Named?"value":""),"Preserve known source/stage/requested variable");
    require(observation.opens==0&&observation.decodes==0,"Version rejection must precede every matio backend call");
    require(observation.header_reads>=1&&observation.triggers==(fault==Fault::None?0u:1u),"Header I/O fault/short-read path must actually execute");
    require(descriptors()==fds,"Version/header failure leaked a snapshot or source descriptor");
    require(digest(path)==before,"Reader changed the synthetic version input");
}
void versionCases(const fs::path& path) {
    for(Api api:{Api::Named,Api::Compatible,Api::FullScan})for(bool denied:{false,true})
        for(Fault fault:{Fault::None,Fault::Interrupted,Fault::Short,Fault::ReadError}) {
            const auto name="v73-api"+std::to_string(static_cast<int>(api))+"-deny"+std::to_string(denied)+
                "-fault"+std::to_string(static_cast<int>(fault));
            scenario(name,[&]{rejected(path,api,fault,denied);});
        }
}
void variants(const fs::path& root,Bytes bytes) {
    // 只改变自己复制的版本头；HDF5内容保持固定。该变体仅用于header识别，不宣称额外格式支持。
    bytes[124]=2;bytes[125]=0;bytes[126]='M';bytes[127]='I';
    const auto big=root/"big-header.mat";save(big,bytes);
    for(Api api:{Api::Named,Api::Compatible,Api::FullScan})
        scenario("big-header-api"+std::to_string(static_cast<int>(api)),[&]{rejected(big,api,Fault::None,true);});
    const auto stem=root/"中文 省略后缀";const auto file=fs::path(stem.string()+".mat");save(file,bytes);
    scenario("compatible-extension",[&]{
        observation={true,true,Fault::None,0,0,0,0};
        const auto result=io::MatResultReader{}.readFirstCompatible(stem);observation.armed=false;
        require(!result.hasValue()&&result.error().code==MatErrorCode::UnsupportedClassOrType&&
            result.error().detail.source_path==file.string()&&observation.opens==0,
            "Extension resolution precedes version rejection; no HDF5 call");
    });
    scenario("unsupported-before-variable-selection",[&]{
        observation={true,true,Fault::None,0,0,0,0};
        const auto result=io::MatResultReader{}.readNamed(file,"missing-variable");observation.armed=false;
        require(!result.hasValue()&&result.error().code==MatErrorCode::UnsupportedClassOrType&&
            result.error().detail.variable=="missing-variable"&&observation.opens==0,
            "Do not inspect a variable directory in an unsupported numeric format");
    });
}
void supportedControl(const fs::path& root) {
    const auto path=root/"supported-v5.mat";
    std::unique_ptr<mat_t,decltype(&Mat_Close)> mat(Mat_CreateVer(path.c_str(),"R6 v5 control",MAT_FT_MAT5),Mat_Close);
    require(bool(mat),"Cannot create v5 control");
    std::array<double,6> data{{1,-0.0,-3.5,4,0.25,6}};std::size_t dimensions[2]={2,3};
    std::unique_ptr<matvar_t,decltype(&Mat_VarFree)> var(Mat_VarCreate("value",MAT_C_DOUBLE,MAT_T_DOUBLE,2,dimensions,data.data(),0),Mat_VarFree);
    require(bool(var)&&Mat_VarWrite(mat.get(),var.get(),MAT_COMPRESSION_ZLIB)==MATIO_E_NO_ERROR,"Cannot write v5 control");
    require(Mat_Close(mat.release())==MATIO_E_NO_ERROR,"Cannot close v5 control");
    for(Api api:{Api::Named,Api::Compatible,Api::FullScan})scenario("v5-control-api"+std::to_string(static_cast<int>(api)),[&]{
        const auto hash=digest(path);observation={true,false,Fault::None,0,0,0,0};
        const auto result=read(api,path);observation.armed=false;
        require(result.hasValue()&&observation.opens==1&&observation.decodes==1,"Supported v5 still uses normal backend exactly once");
        require(digest(path)==hash,"Control read changed v5 input");
    });
}
}
extern "C" mat_t* __wrap_Mat_Open(const char* path,int mode) {
    if(observation.armed){++observation.opens;if(observation.deny_backend){errno=EINVAL;return nullptr;}}
    return __real_Mat_Open(path,mode);
}
extern "C" matvar_t* __wrap_Mat_VarRead(mat_t* mat,const char* name) {
    if(observation.armed)++observation.decodes;
    return __real_Mat_VarRead(mat,name);
}
extern "C" ssize_t __wrap_pread(int fd,void* data,size_t count,off_t offset) {
    if(observation.armed&&offset==0&&count==kHeaderBytes) {
        ++observation.header_reads;
        if(observation.triggers==0&&observation.fault!=Fault::None) {
            ++observation.triggers;
            if(observation.fault==Fault::Short)return __real_pread(fd,data,7,offset);
            errno=observation.fault==Fault::Interrupted?EINTR:EIO;return -1;
        }
    }
    return __real_pread(fd,data,count,offset);
}
int main(int argc,char** argv) {
    try {
        require(argc==3,"Expected fixed fixture and isolated build sandbox");
        const fs::path source=argv[1];require(digest(source)==kFixtureSha,"Fixed synthetic MAT73 identity changed");
        const auto bytes=load(source);const std::array<unsigned char,8> magic{{0x89,'H','D','F',0x0d,0x0a,0x1a,0x0a}};
        require(bytes.size()==3408&&std::equal(magic.begin(),magic.end(),bytes.begin()+kHdf5Offset),"Missing real HDF5 signature in fixed fixture");
        require(bytes[124]==0&&bytes[125]==2&&bytes[126]=='I'&&bytes[127]=='M',"Unexpected original MAT73 version/endian header");
        const auto root=fs::absolute(argv[2])/"中文 空格";fs::create_directories(root/"origin");
        auto allocated=io::RunDirectory::create(root);require(allocated.hasValue(),"Cannot reserve isolated fixture RunId");
        auto run=std::move(allocated).value();const auto directory=run->outputRoot()/run->runId().text;
        std::cout<<"RunId="<<run->runId().text<<" fixture_dir="<<directory<<" SHA256="<<kFixtureSha<<'\n';
        versionCases(source);variants(directory,bytes);supportedControl(directory);
        require(digest(source)==kFixtureSha,"Original synthetic fixture changed after tests");
    }catch(const std::exception& e){std::cerr<<"FAIL fixture setup: "<<e.what()<<'\n';return 1;}
    std::cout<<"MAT73 version checks="<<checks<<" cases="<<cases<<" failures="<<failures
        <<"; backend unavailability is injected, not a separate no-HDF5 build\n";
    return failures?1:0;
}
