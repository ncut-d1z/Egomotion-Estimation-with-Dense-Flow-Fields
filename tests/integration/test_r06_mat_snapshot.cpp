// Linux链接层确定性竞态/故障注入；只改本次沙箱合成MAT，不操作原始归档。
#include "egomotion/io/mat_result_reader.hpp"
#include "egomotion/io/sha256.hpp"
#include <matio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstdarg>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

extern "C" {
int __real_memfd_create(const char*,unsigned);
ssize_t __real_pread(int,void*,size_t,off_t);
ssize_t __real_write(int,const void*,size_t);
int __real_fcntl(int,int,...);
matvar_t* __real_Mat_VarRead(mat_t*,const char*);
}
namespace {
using namespace egomotion;
namespace fs=std::filesystem;
enum class Fault {None,Create,Read,ReadInterrupted,ReadShort,Write,WriteZero,
    WriteInterrupted,WriteShort,Seal,SealConfirmation,Hash,ChangeDuringCopy,
    ReplaceBeforeDecode,OverwriteBeforeDecode};
struct Injection {
    bool armed=false,sealed=false;
    Fault fault=Fault::None;
    int snapshot=-1;
    unsigned triggers=0,creates=0,decoded=0;
    bool write_denied=false,shrink_denied=false;
    fs::path source,replacement;
} injection;
std::size_t checks=0,cases=0;
void require(bool value,const std::string& why) {
    ++checks;if(!value)throw std::runtime_error(why);
}
int fail(int code=EIO) {++injection.triggers;errno=code;return -1;}
bool once(Fault point) {return injection.armed&&injection.fault==point&&injection.triggers==0;}
void overwrite() {
    // 改写当前源inode；保持参考文件和替换文件，便于失败后复核。
    std::ifstream input(injection.replacement,std::ios::binary);
    std::ofstream output(injection.source,std::ios::binary|std::ios::trunc);
    output<<input.rdbuf();output.close();
    require(input.good()&&!output.fail(),"Cannot inject synthetic inode overwrite");
}
std::size_t descriptors() {
    return static_cast<std::size_t>(std::distance(fs::directory_iterator("/proc/self/fd"),fs::directory_iterator{}));
}
std::string hash(const fs::path& path) {
    const int fd=::open(path.c_str(),O_RDONLY|O_CLOEXEC);
    require(fd>=0,"Cannot open fixture hash");const auto result=io::sha256Fd(fd);::close(fd);
    require(result.hasValue(),"Cannot hash fixture");return result.value();
}
void fixture(const fs::path& path,double start) {
    mat_t* mat=Mat_CreateVer(path.c_str(),"R6 fixed snapshot fixture",MAT_FT_MAT5);
    require(mat!=nullptr,"Cannot create isolated MAT fixture");
    size_t dims[2]={2,3};double values[6]={start,start+1,start+2,start+3,start+4,start+5};
    matvar_t* value=Mat_VarCreate("alpha",MAT_C_DOUBLE,MAT_T_DOUBLE,2,dims,values,0);
    require(value!=nullptr,"Cannot create fixture array");
    const int written=Mat_VarWrite(mat,value,MAT_COMPRESSION_ZLIB);Mat_VarFree(value);
    const int closed=Mat_Close(mat);require(written==0&&closed==0,"Cannot finish fixture MAT");
}
void oneCase(const fs::path& root,Fault fault,bool compatible,bool success,const std::string& expected) {
    const auto dir=root/std::to_string(++cases);fs::create_directory(dir);
    const auto source=dir/"读取 中文.mat",replacement=dir/"replacement.mat";
    fs::copy_file(root/"A-reference.mat",source);fs::copy_file(root/"B-reference.mat",replacement);
    const auto before=descriptors();
    injection=Injection{};injection.armed=true;injection.fault=fault;
    injection.source=source;injection.replacement=replacement;
    auto value=compatible?io::MatResultReader{}.readFirstCompatible(source):
                          io::MatResultReader{}.readNamed(source,"alpha");
    injection.armed=false;
    require(value.hasValue()==success,"Snapshot success/failure differs from injected condition");
    require(descriptors()==before,"Snapshot/MAT file descriptor leaked on success or failure");
    require(injection.creates==1,"Numeric read must create exactly one snapshot");
    if(fault!=Fault::None)require(injection.triggers==1,"Fault did not trigger exactly once");
    if(success) {
        require(value.value().source_mat_sha256&&*value.value().source_mat_sha256==expected,
                "Digest must identify decoded frozen A bytes, never current pathname B");
        require(value.value().info.name=="alpha"&&
                value.value().values_column_major==std::vector<double>({1,2,3,4,5,6}),
                "Decode selected a different file version");
        require(injection.decoded==1&&injection.write_denied&&injection.shrink_denied,
                "Successful decode must use a sealed snapshot that rejects writes/truncation");
    } else {
        require(value.error().code==MatErrorCode::InvalidMatHeader&&
                value.error().detail.stage==ErrorStage::Read&&value.error().detail.source_path==source.string(),
                "Snapshot failure must retain read stage and original source path");
        require(injection.decoded==0,"Unconfirmed snapshot must fail before matio numeric decode");
        if(fault==Fault::ChangeDuringCopy)
            require(value.error().detail.message.find("changed while capturing")!=std::string::npos,
                    "Detected source change must not become an unverified success");
    }
    if(fault==Fault::ReplaceBeforeDecode||fault==Fault::OverwriteBeforeDecode||fault==Fault::ChangeDuringCopy)
        require(hash(source)==hash(root/"B-reference.mat"),"Mutation did not actually replace source bytes");
    std::cout<<"case="<<cases<<" fault="<<static_cast<int>(fault)<<" compatible="<<compatible
             <<" success="<<value.hasValue()<<" triggers="<<injection.triggers<<" path="<<source<<'\n';
}
} // namespace

extern "C" int __wrap_memfd_create(const char* name,unsigned flags) {
    if(injection.armed) {
        ++injection.creates;
        if(once(Fault::Create))return fail(ENOMEM);
    }
    const int fd=__real_memfd_create(name,flags);
    if(injection.armed)injection.snapshot=fd;
    return fd;
}
extern "C" ssize_t __wrap_pread(int fd,void* data,size_t count,off_t offset) {
    if(injection.armed&&fd!=injection.snapshot) {
        if(once(Fault::Read))return fail();
        if(once(Fault::ReadInterrupted))return fail(EINTR);
        if(once(Fault::ReadShort)&&count>1){++injection.triggers;return __real_pread(fd,data,1,offset);}
        const auto n=__real_pread(fd,data,count,offset);
        if(once(Fault::ChangeDuringCopy)&&n>0){++injection.triggers;overwrite();}
        return n;
    }
    if(injection.armed&&fd==injection.snapshot&&once(Fault::Hash))return fail();
    return __real_pread(fd,data,count,offset);
}
extern "C" ssize_t __wrap_write(int fd,const void* data,size_t count) {
    if(injection.armed&&fd==injection.snapshot) {
        if(once(Fault::Write))return fail();
        if(once(Fault::WriteZero)){++injection.triggers;return 0;}
        if(once(Fault::WriteInterrupted))return fail(EINTR);
        if(once(Fault::WriteShort)&&count>1){++injection.triggers;return __real_write(fd,data,1);}
    }
    return __real_write(fd,data,count);
}
extern "C" int __wrap_fcntl(int fd,int command,...) {
    if(command==F_GET_SEALS) {
        if(injection.armed&&fd==injection.snapshot&&once(Fault::SealConfirmation)){++injection.triggers;return 0;}
        return __real_fcntl(fd,command);
    }
    va_list args;va_start(args,command);const int value=va_arg(args,int);va_end(args);
    if(injection.armed&&fd==injection.snapshot&&command==F_ADD_SEALS) {
        if(once(Fault::Seal))return fail(EPERM);
        const int result=__real_fcntl(fd,command,value);injection.sealed=result==0;return result;
    }
    return __real_fcntl(fd,command,value);
}
extern "C" matvar_t* __wrap_Mat_VarRead(mat_t* mat,const char* name) {
    if(injection.armed) {
        ++injection.decoded;
        require(injection.sealed,"Decode began before snapshot sealing");
        const unsigned char byte=0;
        injection.write_denied=::pwrite(injection.snapshot,&byte,1,0)<0&&errno==EPERM;
        injection.shrink_denied=::ftruncate(injection.snapshot,0)<0&&errno==EPERM;
        if(once(Fault::ReplaceBeforeDecode)) {++injection.triggers;fs::rename(injection.replacement,injection.source);}
        if(once(Fault::OverwriteBeforeDecode)) {++injection.triggers;overwrite();}
    }
    return __real_Mat_VarRead(mat,name);
}
int main(int argc,char** argv) {
    try {
        require(argc==2,"Expected isolated binary sandbox");
        fs::create_directories(argv[1]);auto pattern=(fs::path(argv[1])/"snapshot-XXXXXX").string();
        std::vector<char> storage(pattern.begin(),pattern.end());storage.push_back('\0');
        const char* created=::mkdtemp(storage.data());require(created!=nullptr,"Cannot reserve evidence");
        const fs::path root(created);std::cout<<"evidence="<<root<<'\n';
        fixture(root/"A-reference.mat",1);fixture(root/"B-reference.mat",11);
        const auto expected=hash(root/"A-reference.mat");
        for(const auto fault:{Fault::None,Fault::ReadInterrupted,Fault::ReadShort,
                Fault::WriteInterrupted,Fault::WriteShort,Fault::ReplaceBeforeDecode,Fault::OverwriteBeforeDecode})
            for(bool compatible:{false,true})oneCase(root,fault,compatible,true,expected);
        for(const auto fault:{Fault::Create,Fault::Read,Fault::Write,Fault::WriteZero,
                Fault::Seal,Fault::SealConfirmation,Fault::Hash,Fault::ChangeDuringCopy})
            for(bool compatible:{false,true})oneCase(root,fault,compatible,false,expected);
        injection=Injection{};injection.armed=true;injection.fault=Fault::Create;
        const auto metadata=io::MatResultReader{}.inspect(root/"A-reference.mat");injection.armed=false;
        require(metadata.hasValue()&&injection.creates==0&&injection.decoded==0,
                "Metadata-only inspection must not acquire numeric snapshots or decode values");
        require(hash(root/"A-reference.mat")==expected,"Reference fixture must remain unchanged");
        std::cout<<"PASS snapshot checks="<<checks<<" numeric_cases="<<cases<<" metadata_cases=1\n";return 0;
    } catch(const std::exception& error) {injection.armed=false;std::cerr<<error.what()<<'\n';return 1;}
}
