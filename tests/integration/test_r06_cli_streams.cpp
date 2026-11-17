// 真实CLI装配和限定Bias产物；仅输出流/失败Runner/可选导出器为故障替身。
#include "egomotion/cli/command.hpp"
#include "egomotion/contracts/diagnostics.hpp"
#include "egomotion/analysis/result_analyzer.hpp"
#include "egomotion/experiments/production_runner.hpp"
#include "egomotion/experiments/run_context.hpp"
#include "egomotion/io/mat_result_reader.hpp"

#include <cstdlib>
#include <cerrno>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>

namespace {
using namespace egomotion;
namespace fs=std::filesystem;
std::size_t checks=0,calls=0;
void require(bool value,const char* reason) {
    ++checks;if(!value)throw std::runtime_error(reason);
}
std::string read(const fs::path& path) {
    std::ifstream file(path,std::ios::binary);require(file.good(),"Missing audit artifact");
    return {std::istreambuf_iterator<char>(file),{}};
}
enum class Fault { ShortWrite, PartialWrite, Flush, ThrowWrite, ThrowFlush, SecondWrite };
class FaultBuffer final:public std::stringbuf {
public:
    explicit FaultBuffer(Fault fault):fault_(fault) {}
protected:
    std::streamsize xsputn(const char* text,std::streamsize size) override {
        ++writes_;
        if(fault_==Fault::ThrowWrite)throw std::runtime_error("Injected write exception");
        if(fault_==Fault::ShortWrite||(fault_==Fault::SecondWrite&&writes_>1))return 0;
        if(fault_==Fault::PartialWrite)return std::stringbuf::xsputn(text,size/2);
        return std::stringbuf::xsputn(text,size);
    }
    int sync() override {
        if(fault_==Fault::ThrowFlush)throw std::runtime_error("Injected flush exception");
        return fault_==Fault::Flush?-1:0;
    }
private:
    Fault fault_;
    int writes_=0;
};
int invoke(const std::vector<std::string>& args,std::ostream& out,std::ostream& err,
           const cli::ApplicationCompositionRoot& app) {
    ++calls;return cli::run(args,out,err,app);
}
void deliveryError(const std::string& error,const std::string& run_id={}) {
    require(error.find("\"error_code\":\"OutputFailed\"")!=std::string::npos,"Output error classification");
    require(error.find("\"stage\":\"output\"")!=std::string::npos,"Output error stage");
    require(error.find("\"exit_code\":6")!=std::string::npos,"Output error exit");
    require(error.find("\"causes\":[]")!=std::string::npos,"No invented causal chain");
    if(run_id.empty())require(error.find("\"run_id\"")==std::string::npos,"Do not invent unallocated identity");
    else require(error.find("\"run_id\":"+jsonQuote(run_id))!=std::string::npos,"Allocated identity retained");
}
Failure<ExperimentErrorCode> failedCalculation() {
    ErrorDetail inner;inner.stage=ErrorStage::Solve;inner.message="Injected numerical failure";
    inner.observation_index=7;
    ErrorDetail outer;outer.stage=ErrorStage::Simulation;outer.message="Injected experiment failure";
    return wrapFailure(ExperimentErrorCode::SolverFailed,std::move(outer),
        Failure<NumericalErrorCode>{NumericalErrorCode::UndefinedAngle,std::move(inner)});
}
class FailedRunner final:public IExperimentRunner {
public:
    bool block_manifest=false;
    Result<RunOutcome,ExperimentErrorCode> fail(RunContext& context) {
        if(block_manifest)
            fs::create_directory(context.approvedOutputRoot()/context.runId().text/"manifest.json");
        return Result<RunOutcome,ExperimentErrorCode>::failure(failedCalculation());
    }
    Result<RunOutcome,ExperimentErrorCode> runBiasOccluded(const BiasExperimentConfig&,RunContext& c) override {return fail(c);}
    Result<RunOutcome,ExperimentErrorCode> runBiasProportional(const BiasExperimentConfig&,RunContext& c) override {return fail(c);}
    Result<RunOutcome,ExperimentErrorCode> runAdaptiveInitialization(const AdaptiveInitializationConfig&,RunContext& c) override {return fail(c);}
    Result<RunOutcome,ExperimentErrorCode> runAdaptiveComparison(const AdaptiveComparisonConfig&,RunContext& c) override {return fail(c);}
    Result<RunOutcome,ExperimentErrorCode> runAdaptiveUpdateRates(const AdaptiveUpdateRatesConfig&,RunContext& c) override {return fail(c);}
};
class Exporter final:public IPlotExporter,public illustrations::IExporter {
    Result<std::vector<std::string>,PlotErrorCode> publish(ITextPublisher& publisher) {
        const auto result=publisher.publishNew("delivery-fixture.txt","Retained publication fixture\n");
        require(result.hasValue(),"Fixture publication failed");
        return Result<std::vector<std::string>,PlotErrorCode>::success({"delivery-fixture.txt"});
    }
public:
    Result<std::vector<std::string>,PlotErrorCode> exportPlot(const AnalysisDataset&,ITextPublisher& p) override {return publish(p);}
    Result<std::vector<std::string>,PlotErrorCode> exportSvg(const illustrations::Data&,ITextPublisher& p) override {return publish(p);}
};
fs::path uniqueOutput(const fs::path& root) {
    std::vector<fs::path> paths;
    for(const auto& entry:fs::directory_iterator(root/"output"))paths.push_back(entry.path());
    require(paths.size()==1,"Exactly one reserved RunId");return paths.front();
}
void pureCommands(const fs::path& fixture,const cli::ApplicationCompositionRoot& app,Fault fault,bool throws) {
    const std::vector<std::vector<std::string>> commands={
        {"help"},{"version"},{"inspect-mat","--file",(fixture/"xwerte.mat").string()},
        {"analyze","--experiment","bias","--input-root",fixture.string()},
        {"illustrate","--kind","circle"}};
    for(const auto& args:commands) {
        FaultBuffer buffer(fault);std::ostream out(&buffer);std::ostringstream err;
        if(throws)out.exceptions(std::ios::badbit|std::ios::failbit);
        require(invoke(args,out,err,app)==6,"No success after failed stdout");
        deliveryError(err.str());require(!out.good(),"Do not clear stdout failure bits");
        if(fault==Fault::Flush||fault==Fault::ThrowFlush||fault==Fault::PartialWrite)
            require(!buffer.str().empty(),"Do not erase bytes already delivered");
    }
}
void successfulArtifacts(const fs::path& root,const fs::path& fixture,Fault fault,bool throws) {
    io::MatResultReader reader;analysis::MatResultAnalyzer analyzer;
    experiments::ProductionExperimentRunner runner;Exporter exporter;
    for(const auto& kind:{std::string("bias"),std::string("plot"),std::string("illustration")}) {
        const auto repo=root/kind;fs::create_directories(repo/"origin");
        cli::ApplicationCompositionRoot app(&reader,&runner,&analyzer,&exporter,repo,&exporter);
        std::vector<std::string> args;
        if(kind=="bias")args={"run-bias-occluded","--seed","7","--level","2"};
        else if(kind=="plot")args={"plot","--experiment","bias","--input-root",fixture.string()};
        else args={"illustrate","--kind","circle","--format","svg"};
        FaultBuffer buffer(fault);std::ostream out(&buffer);std::ostringstream err;
        if(throws)out.exceptions(std::ios::badbit|std::ios::failbit);
        require(invoke(args,out,err,app)==6,"Publication response failure must be nonzero");
        const auto directory=uniqueOutput(repo);deliveryError(err.str(),directory.filename().string());
        if(kind=="bias") {
            const auto manifest=read(directory/"manifest.json");
            require(manifest.find("\"completeness\":\"Complete\"")!=std::string::npos,
                    "Response delivery failure must not rewrite durable computation evidence");
            std::size_t files=0;for(const auto& entry:fs::directory_iterator(directory)){(void)entry;++files;}
            require(files==8,"All seven verified MAT files and Manifest preserved");
        } else require(read(directory/"delivery-fixture.txt")=="Retained publication fixture\n","Published artifact preserved");
        require(fs::is_empty(repo/"origin"),"Original marker untouched");
    }
}
void failedDiagnostics(const fs::path& root,Fault fault,bool throws) {
    fs::create_directories(root/"origin");FailedRunner runner;
    cli::ApplicationCompositionRoot app(nullptr,&runner,nullptr,nullptr,root);
    for(const auto& args:std::vector<std::vector<std::string>>{
            {"unknown"},{"inspect-mat","--file","unused"},
            {"run-bias-occluded","--seed","7","--level","0","--run-id","failed"}}) {
        FaultBuffer buffer(fault);std::ostream err(&buffer);std::ostringstream out;
        if(throws)err.exceptions(std::ios::badbit|std::ios::failbit);
        require(invoke(args,out,err,app)==6,"Failed stderr must not escape or report original delivery success");
        require(out.str().empty(),"Diagnostic failure must not contaminate stdout");
    }
    auto expected=failedCalculation();expected.detail.run_id=RunId{"failed"};
    const auto manifest=read(root/"output/failed/manifest.json");
    require(manifest.find("\"errors\":["+jsonFailure(expected,5)+"]")!=std::string::npos,
            "Broken stderr must not prevent complete original chain from reaching Manifest");
    require(manifest.find("\"completeness\":\"Failed\"")!=std::string::npos,"Failure is never Complete");
    FaultBuffer out_buffer(fault),err_buffer(fault);std::ostream out(&out_buffer),err(&err_buffer);
    if(throws){out.exceptions(std::ios::badbit|std::ios::failbit);err.exceptions(std::ios::badbit|std::ios::failbit);}
    require(invoke({"help"},out,err,app)==6,"Both channels failed without recursive exception");
}
void secondDiagnostic(const fs::path& root) {
    fs::create_directories(root/"origin");FailedRunner runner;runner.block_manifest=true;
    cli::ApplicationCompositionRoot app(nullptr,&runner,nullptr,nullptr,root);
    FaultBuffer buffer(Fault::SecondWrite);std::ostream err(&buffer);std::ostringstream out;
    err.exceptions(std::ios::badbit|std::ios::failbit);
    require(invoke({"run-bias-occluded","--seed","7","--level","0","--run-id","two_events"},out,err,app)==6,
            "Second independent diagnostic delivery failure must be visible in exit code");
    auto expected=failedCalculation();expected.detail.run_id=RunId{"two_events"};
    require(buffer.str()==jsonFailure(expected,5)+"\n","First diagnostic preserved with original exit and chain");
    require(fs::is_directory(root/"output/two_events/manifest.json"),"Failed Manifest publication must not overwrite blocker");
}
void brokenPipeAudit(const fs::path& root) {
    fs::create_directories(root/"origin");FailedRunner runner;
    cli::ApplicationCompositionRoot app(nullptr,&runner,nullptr,nullptr,root);
    int descriptors[2];require(::pipe(descriptors)==0,"Cannot create stderr pipe");
    // fork之前关闭唯一读端，避免子进程写入与父进程关闭之间的竞态。
    require(::close(descriptors[0])==0,"Cannot close pipe reader");
    std::cout.flush();
    const auto child=::fork();require(child>=0,"Cannot fork isolated signal test");
    if(child==0) {
        if(::dup2(descriptors[1],STDERR_FILENO)<0)::_exit(90);
        ::close(descriptors[1]);
        if(std::signal(SIGPIPE,SIG_DFL)==SIG_ERR)::_exit(91);
        const int code=cli::run({"run-bias-occluded","--seed","7","--level","0","--run-id","broken_pipe"},
                                std::cout,std::cerr,app);
        ::_exit(code);
    }
    ++calls;::close(descriptors[1]);int status=0;pid_t waited;
    do {waited=::waitpid(child,&status,0);}while(waited<0&&errno==EINTR);
    require(waited==child&&WIFSIGNALED(status)&&WTERMSIG(status)==SIGPIPE,
            "Default SIGPIPE behavior must remain unchanged");
    auto expected=failedCalculation();expected.detail.run_id=RunId{"broken_pipe"};
    const auto manifest=read(root/"output/broken_pipe/manifest.json");
    require(manifest.find("\"errors\":["+jsonFailure(expected,5)+"]")!=std::string::npos,
            "Original audit must be durable before fatal stderr delivery");
    require(manifest.find("\"completeness\":\"Failed\"")!=std::string::npos,"SIGPIPE cannot invent Complete");
    std::cout<<"SIGPIPE child terminated; original Failed Manifest and complete chain preserved\n";
}
void verify(const fs::path& root) {
    const auto repo=root/"baseline";fs::create_directories(repo/"origin");
    io::MatResultReader reader;analysis::MatResultAnalyzer analyzer;experiments::ProductionExperimentRunner runner;
    cli::ApplicationCompositionRoot app(&reader,&runner,&analyzer,nullptr,repo);
    std::ostringstream out,err;out.exceptions(std::ios::badbit|std::ios::failbit);err.exceptions(std::ios::badbit|std::ios::failbit);
    require(invoke({"run-bias-occluded","--seed","7","--level","2","--run-id","baseline"},out,err,app)==0,
            "Healthy exception-enabled output must still work");
    require(err.str().empty(),"Healthy output has no diagnostic");
    const auto fixture=repo/"output/baseline";
    std::map<fs::path,std::string> before;
    for(const auto& entry:fs::directory_iterator(fixture))before.emplace(entry.path(),read(entry.path()));
    int scenario=0;
    for(const auto fault:{Fault::ShortWrite,Fault::PartialWrite,Fault::Flush,Fault::ThrowWrite,Fault::ThrowFlush})
        for(bool throws:{false,true}) {
            const auto base=root/("scenario_"+std::to_string(++scenario));
            pureCommands(fixture,app,fault,throws);
            successfulArtifacts(base/"successful",fixture,fault,throws);
            failedDiagnostics(base/"failed",fault,throws);
        }
    secondDiagnostic(root/"second_diagnostic");
    brokenPipeAudit(root/"broken_pipe");
    for(const auto& entry:before)require(read(entry.first)==entry.second,"Read-only analysis/inspect changed fixture");
}
} // namespace
int main(int argc,char** argv) {
    try {
        require(argc==2,"Expected isolated sandbox");
        fs::create_directories(argv[1]);auto pattern=(fs::path(argv[1])/"流 中文-XXXXXX").string();
        std::vector<char> bytes(pattern.begin(),pattern.end());bytes.push_back('\0');
        const char* root=::mkdtemp(bytes.data());require(root,"Cannot allocate isolated evidence");
        std::cout<<"evidence="<<root<<'\n';verify(root);
        std::cout<<"PASS CLI streams checks="<<checks<<" dispatches="<<calls<<'\n';return 0;
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
