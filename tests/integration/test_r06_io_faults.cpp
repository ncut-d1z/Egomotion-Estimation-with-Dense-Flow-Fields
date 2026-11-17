#include "../support/io_fault_injection.hpp"

#include "egomotion/experiments/run_context.hpp"
#include "egomotion/io/mat_result_reader.hpp"
#include "egomotion/io/run_mat_writer.hpp"
#include "egomotion/io/run_text_publisher.hpp"
#include "egomotion/io/sha256.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
using namespace egomotion;
using io_fault_test::Point;
using io_fault_test::Observed;
namespace fs=std::filesystem;
void require(bool condition,const char* message) {
    if(!condition)throw std::runtime_error(message);
}
const char* pointName(Point point) {
    switch(point){
    case Point::None:return "success";
    case Point::OpenStaging:return "staging-open";
    case Point::WriteError:return "write-error";
    case Point::WriteZero:return "write-zero";
    case Point::WriteInterrupted:return "write-eintr";
    case Point::WriteShort:return "write-short";
    case Point::FileSync:return "file-fsync";
    case Point::LinkError:return "link-error";
    case Point::LinkCollision:return "link-eexist";
    case Point::PublishedIdentity:return "published-fstatat";
    case Point::DirectorySyncFirst:return "first-directory-fsync";
    case Point::UnlinkOnce:return "unlink-once";
    case Point::UnlinkPersistent:return "unlink-persistent";
    case Point::DirectorySyncLast:return "last-directory-fsync";
    }
    throw std::logic_error("Unknown injection point");
}
bool succeeds(Point point) {
    return point==Point::None||point==Point::WriteInterrupted||point==Point::WriteShort;
}
bool published(Point point) {
    return succeeds(point)||point==Point::PublishedIdentity||
        point==Point::DirectorySyncFirst||point==Point::UnlinkOnce||
        point==Point::UnlinkPersistent||point==Point::DirectorySyncLast;
}
MatErrorCode expectedCode(Point point,bool mat) {
    if(published(point))return MatErrorCode::ReadbackMismatch;
    if(point==Point::LinkCollision)return MatErrorCode::OutputExists;
    if(point==Point::OpenStaging&&mat)return MatErrorCode::OutputPathForbidden;
    return MatErrorCode::WriteFailed;
}
std::unique_ptr<io::RunDirectory> newRun(const fs::path& root,const char* kind,Point point) {
    auto result=io::RunDirectory::create(root);
    require(result.hasValue(),"Cannot create isolated fault-test RunId");
    auto run=std::move(result).value();
    std::cout<<kind<<" "<<pointName(point)<<" RunId="<<run->runId().text<<std::endl;
    return run;
}
std::string readText(const fs::path& path) {
    std::ifstream input(path,std::ios::binary);
    require(input.is_open(),"Expected published file cannot be opened");
    return {std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>()};
}
DenseDoubleArray matrix(std::string name,std::size_t rows=2,std::size_t cols=3) {
    DenseDoubleArray result;
    result.info.name=std::move(name);
    result.info.original_class_type=6;
    result.info.original_data_type=9;
    result.info.dimensions={rows,cols};
    result.info.data_bytes=rows*cols*sizeof(double);
    result.values_column_major.assign(rows*cols,1.25);
    return result;
}
void checkEvents(Point point,const Observed& events) {
    unsigned file_syncs=1,dir_syncs=0,unlinks=1;
    if(point==Point::OpenStaging){file_syncs=0;unlinks=0;}
    if(point==Point::WriteError||point==Point::WriteZero)file_syncs=0;
    if(point==Point::DirectorySyncFirst)dir_syncs=1;
    if(point==Point::UnlinkOnce||point==Point::UnlinkPersistent){dir_syncs=1;unlinks=2;}
    if(succeeds(point)||point==Point::DirectorySyncLast)dir_syncs=2;
    const unsigned triggered=point==Point::None?0:
        (point==Point::UnlinkPersistent?2:1);
    require(events.triggered==triggered,"Fault hook did not execute the exact expected times");
    require(events.file_syncs==file_syncs&&events.directory_syncs==dir_syncs&&
            events.unlink_calls==unlinks&&events.linked==published(point),
            "Publication syscalls ran in an unexpected phase or count");
}
void checkStaging(const io::RunDirectory& run,Point point,const char* final_name) {
    const auto path=run.outputRoot()/run.runId().text;
    const bool exists=published(point)||point==Point::LinkCollision;
    require(fs::exists(path/final_name)==exists,"Final name presence disagrees with publication");
    std::size_t staging=0;
    for(const auto& entry:fs::directory_iterator(path)){
        const auto name=entry.path().filename().string();
        if(name.rfind(".r5_",0)!=0&&name.rfind(".text_",0)!=0)continue;
        ++staging;
        require(point==Point::UnlinkPersistent&&fs::equivalent(entry.path(),path/final_name),
                "Unexpected orphan staging file or wrong retained inode");
    }
    require(staging==(point==Point::UnlinkPersistent?1U:0U),
            "Persistent cleanup failure must retain its evidence; other cleanup must succeed");
    if(exists)require(fs::hard_link_count(path/final_name)==1+staging,
                      "Unexpected hard-link count on published evidence");
}
void checkCompetitor(const io::RunDirectory& run,const char* filename,const Observed& events) {
    struct stat actual{};
    require(events.competitor_created&&
            ::fstatat(run.runFd(),filename,&actual,AT_SYMLINK_NOFOLLOW)==0&&
            actual.st_ino==events.competitor_identity.st_ino&&
            actual.st_dev==events.competitor_identity.st_dev,
            "Publication collision replaced the competitor inode");
    require(readText(run.outputRoot()/run.runId().text/filename)==
            io_fault_test::competitor_contents,"Publication collision modified competitor bytes");
}
void checkStatus(const Status<MatErrorCode>& status,Point point,bool mat,
                 const io::RunDirectory& run) {
    require(status.hasValue()==succeeds(point),"Writer returned the wrong success state");
    if(!status.hasValue()){
        require(status.error().code==expectedCode(point,mat)&&
                status.error().detail.stage==ErrorStage::Output&&
                status.error().detail.run_id&&
                status.error().detail.run_id->text==run.runId().text,
                "Writer failed to preserve error code, stage or RunId");
    }
}
void checkText(const fs::path& root,Point point) {
    auto run=newRun(root,"text",point);
    io::RunTextPublisher publisher(*run);
    constexpr char contents[]="{\"synthetic\":\"UTF-8 中文 and spaces\"}\n";
    Observed events;
    const auto status=[&]{
        io_fault_test::Scope fault(point,run->runFd(),"manifest.json");
        auto result=publisher.publishNew("manifest.json",contents);
        events=fault.observed();return result;
    }();
    checkEvents(point,events);
    checkStatus(status,point,false,*run);
    checkStaging(*run,point,"manifest.json");
    if(published(point))require(readText(run->outputRoot()/run->runId().text/"manifest.json")==contents,
                                "Published text bytes changed after failure or partial write retry");
    if(point==Point::LinkCollision)checkCompetitor(*run,"manifest.json",events);
}
void checkMat(const fs::path& root,Point point) {
    auto run=newRun(root,"mat",point);
    io::RunMatWriter writer(*run);
    const auto payload=[] {
        auto value=matrix("xwerte");
        value.values_column_major={0.0,-0.0,1.5,-2.25,1e-100,1e100};
        return value;
    }();
    Observed events;
    const auto status=[&]{
        io_fault_test::Scope fault(point,run->runFd(),"xwerte.mat");
        auto result=writer.saveNewVariable(run->runId(),"xwerte",payload);
        events=fault.observed();return result;
    }();
    checkEvents(point,events);
    checkStatus(status,point,true,*run);
    checkStaging(*run,point,"xwerte.mat");
    const auto receipts=writer.fileEvidence();
    require(receipts.size()==(published(point)?1U:0U),
            "Writer must retain only actually published MAT receipts");
    if(published(point)){
        require(receipts[0].durably_confirmed==succeeds(point),
                "Unconfirmed MAT cannot expose a durable receipt");
        const auto path=run->outputRoot()/run->runId().text/"xwerte.mat";
        const auto decoded=io::MatResultReader{}.readNamed(path,"xwerte");
        require(decoded.hasValue()&&decoded.value().info.dimensions==payload.info.dimensions&&
                decoded.value().values_column_major.size()==payload.values_column_major.size()&&
                std::memcmp(decoded.value().values_column_major.data(),
                    payload.values_column_major.data(),payload.info.data_bytes)==0,
                "Published MAT must preserve all column-major bits even after a later failure");
        const int fd=::open(path.c_str(),O_RDONLY|O_CLOEXEC|O_NOFOLLOW);
        require(fd>=0,"Cannot open retained MAT for independent digest verification");
        const auto digest=io::sha256Fd(fd);::close(fd);
        require(digest.hasValue()&&digest.value()==receipts[0].sha256&&
                receipts[0].size_bytes==fs::file_size(path),
                "Retained MAT receipt must describe the actual published file");
    }
    if(point==Point::LinkCollision)checkCompetitor(*run,"xwerte.mat",events);
}
struct FixedRandom final:IRandomSource {
    Result<Scalar,NumericalErrorCode> uniform() override {
        return Result<Scalar,NumericalErrorCode>::success(0.5);
    }
    Result<Scalar,NumericalErrorCode> normal() override {
        return Result<Scalar,NumericalErrorCode>::success(0.0);
    }
};
bool hasCause(const Failure<ExperimentErrorCode>& failure,const char* code) {
    for(const auto& cause:failure.detail.causes)
        if(cause.domain=="MatErrorCode"&&cause.code==code)return true;
    return false;
}
void checkManifest(const fs::path& root,Point point) {
    auto directory=newRun(root,"manifest",point);
    io::RunMatWriter writer(*directory);
    io::RunTextPublisher publisher(*directory);
    // 合成输入与来源标记，不宣称执行了原始科学实验。
    SourceProvenance provenance{std::string(40,'a'),std::string(40,'b'),13};
    constexpr auto kind=experiments::ExperimentKind::AdaptiveInitialization;
    RunContext run(directory->runId(),std::make_unique<FixedRandom>(),writer,publisher,
                   directory->outputRoot(),std::move(provenance),kind,1);
    std::map<std::string,DenseDoubleArray> values;
    for(const auto& name:experiments::currentSaveFields(kind))values.emplace(name,matrix(name,35,1));
    require(run.noteCompletedMemory(1,33).hasValue()&&run.writeCurrentResults(values).hasValue(),
            "Real MAT Writer must finish all synthetic scientific inputs before Manifest fault");
    Observed events;
    const auto outcome=[&]{
        io_fault_test::Scope fault(point,directory->runFd(),"manifest.json");
        auto result=run.publishCompleteOutcome();events=fault.observed();return result;
    }();
    checkEvents(point,events);
    checkStaging(*directory,point,"manifest.json");
    const auto path=directory->outputRoot()/directory->runId().text/"manifest.json";
    if(succeeds(point)){
        require(outcome.hasValue()&&outcome.value().status==RunStatus::Complete&&
                run.manifestDraft().status==RunStatus::Complete,
                "Confirmed real Manifest must yield a successful Complete outcome");
        require(!run.publishCompleteOutcome().hasValue(),"Complete cannot be published twice");
        return;
    }
    require(!outcome.hasValue()&&outcome.error().code==ExperimentErrorCode::OutputFailed&&
            hasCause(outcome.error(),published(point)?"ReadbackMismatch":"WriteFailed")&&
            run.manifestDraft().status!=RunStatus::Complete,
            "Unconfirmed real Manifest must reject Complete and preserve its I/O cause");
    const auto before=published(point)?readText(path):std::string{};
    const auto audit=run.publishFailureRecord(outcome.error());
    require(run.manifestDraft().status==RunStatus::Incomplete,
            "Successful MAT inputs plus failed Manifest require Incomplete in-memory audit");
    if(published(point)){
        require(!audit.hasValue()&&hasCause(audit.error(),"OutputExists")&&readText(path)==before,
                "Failure audit must report its own collision without overwriting linked Manifest");
        require(before.find("\"completeness\":\"Complete\"")!=std::string::npos,
                "Post-publication fixture must include real Complete text lacking confirmation");
    }else{
        require(audit.hasValue()&&readText(path).find("\"completeness\":\"Incomplete\"")!=std::string::npos,
                "A pre-publication failure should permit a later truthful Incomplete audit");
    }
}
void checkWriters(const fs::path& root) {
    constexpr std::array<Point,9> common{{Point::None,Point::OpenStaging,Point::FileSync,
        Point::LinkError,Point::LinkCollision,Point::DirectorySyncFirst,
        Point::UnlinkOnce,Point::UnlinkPersistent,Point::DirectorySyncLast}};
    for(const auto point:common){checkText(root,point);checkMat(root,point);}
    checkMat(root,Point::PublishedIdentity);
    for(const auto point:{Point::WriteError,Point::WriteZero,Point::WriteInterrupted,Point::WriteShort})
        checkText(root,point);
    std::cout<<"PASS Linux Writer fault scenarios=23; exact syscall stages, receipts, preserved bytes\n";
}
} // namespace

int main(int argc,char** argv) {
    try{
        require(argc==3,"Expected isolated sandbox path and writers/manifest mode");
        const fs::path root=fs::absolute(argv[1])/u8"故障 沙箱";
        fs::create_directories(root/"origin");
        const std::string mode(argv[2]);
        if(mode=="writers")checkWriters(root);
        else if(mode=="manifest"){
            for(const auto point:{Point::None,Point::FileSync,Point::DirectorySyncFirst,
                    Point::UnlinkOnce,Point::UnlinkPersistent,Point::DirectorySyncLast})
                checkManifest(root,point);
            std::cout<<"PASS real Manifest fault scenarios=6; separate output/audit errors\n";
        }else throw std::invalid_argument("Unknown fault-test mode");
        return 0;
    }catch(const std::exception& error){
        std::cerr<<"FAIL Linux I/O fault regression: "<<error.what()<<'\n';return 1;
    }
}
