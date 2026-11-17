#include "egomotion/io/run_directory.hpp"
#include "egomotion/io/run_mat_writer.hpp"
#include "egomotion/io/run_text_publisher.hpp"
#include "egomotion/io/sha256.hpp"
#include "egomotion/io/mat_result_reader.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <filesystem>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace {
void require(bool condition,const char* why){
    if(!condition)throw std::runtime_error(why);
}
void requireUnchanged(const egomotion::DenseDoubleArray& actual,
                      const egomotion::DenseDoubleArray& expected) {
    const auto metadata=[](const egomotion::MatVariableInfo& info){
        return std::tie(info.name,info.dimensions,info.original_class_type,
                        info.original_data_type,info.is_complex,info.is_logical,
                        info.is_global,info.is_compressed,info.data_bytes,
                        info.class_name,info.data_type_name);
    };
    require(metadata(actual.info)==metadata(expected.info)&&
            actual.values_column_major==expected.values_column_major,
            "MAT Writer must leave the caller's const payload unchanged");
}
void requireNoStagingFiles(const std::filesystem::path& run_path) {
    for(const auto& entry:std::filesystem::directory_iterator(run_path)){
        const auto name=entry.path().filename().string();
        require(name.rfind(".r5_",0)!=0&&name.rfind(".text_",0)!=0,
                "Writer left an orphan MAT/text staging file or hard link");
    }
}
}
int main(int argc,char** argv){
    using namespace egomotion;
    namespace fs=std::filesystem;
    if(argc!=2)throw std::invalid_argument("Expected isolated build-tree sandbox");
    // The sandbox is NEVER the user repo's origin/, and no cleanup/deletion
    // is performed: repeated runs create different, OS-random RunIds.
    const fs::path root=fs::absolute(argv[1]);
    fs::create_directories(root/"origin");
    auto directory=io::RunDirectory::create(root);
    require(directory.hasValue(),"Cannot allocate isolated synthetic RunId");
    auto run=std::move(directory).value();

    io::RunTextPublisher text(*run);
    require(text.publishNew("hello.txt","hello").hasValue(),
            "Cannot publish original synthetic text");
    auto duplicate=text.publishNew("hello.txt","overwrite");
    require(!duplicate.hasValue()&&duplicate.error().code==MatErrorCode::OutputExists,
            "Second publication must not overwrite existing text");
    require(!text.publishNew("../origin/attack.txt","bad").hasValue(),
            "Text publisher must reject parent directory escape");

    const int fd=::openat(run->runFd(),"hello.txt",O_RDONLY|O_CLOEXEC|O_NOFOLLOW);
    require(fd>=0,"Anchored text file cannot be reopened");
    auto digest=io::sha256Fd(fd);
    ::close(fd);
    require(digest.hasValue()&&
            digest.value()=="2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824",
            "SHA256 of hello differs from published FIPS-180-4 reference");

    const DenseDoubleArray a=[] {
        DenseDoubleArray value;
        value.info.name="xwerte";
        value.info.original_class_type=6;
        value.info.original_data_type=9;
        value.info.dimensions={2,3};
        value.info.data_bytes=6*sizeof(double);
        value.values_column_major={1,2,3,4,5,6};
        return value;
    }();
    const auto original=a;
    io::RunMatWriter writer(*run);
    require(writer.saveNewVariable(run->runId(),"xwerte",a).hasValue(),
            "Synthetic MAT Writer cannot publish and read back 2x3 double");
    requireUnchanged(a,original);
    const auto evidence=writer.fileEvidence();
    require(evidence.size()==1&&evidence[0].durably_confirmed&&
            evidence[0].sha256.size()==64,
            "A successfully published MAT must have durable SHA256 evidence");
    auto already=writer.saveNewVariable(run->runId(),"xwerte",a);
    require(!already.hasValue()&&already.error().code==MatErrorCode::OutputExists,
            "MAT Writer must reject overwriting its own result");
    requireUnchanged(a,original);
    auto unsafe=writer.saveNewVariable(run->runId(),"../origin/attack",a);
    require(!unsafe.hasValue(),"MAT Writer must reject invalid target basename");
    requireUnchanged(a,original);

    io::MatResultReader reader;
    auto back=reader.readNamed(run->outputRoot()/run->runId().text/"xwerte.mat","xwerte");
    require(back.hasValue()&&back.value().info.dimensions==a.info.dimensions&&
            back.value().values_column_major==a.values_column_major,
            "MAT published output must decode to the exact original column-major values");
    // Both successful publications and negative duplicate attempts must
    // leave no orphan random staging names in the pinned RunId directory.
    requireNoStagingFiles(run->outputRoot()/run->runId().text);
}
