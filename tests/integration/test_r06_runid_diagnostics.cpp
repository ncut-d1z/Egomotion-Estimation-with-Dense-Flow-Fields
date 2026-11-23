// 仅故障合同替身；真实 CLI 装配、RunId 预留和失败 Manifest 发布，不执行科学实验。
#include "egomotion/cli/command.hpp"
#include "egomotion/contracts/diagnostics.hpp"
#include "egomotion/experiments/run_context.hpp"
#include "egomotion/io/mat_result_reader.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace egomotion;
namespace fs=std::filesystem;
std::size_t checks=0,calls=0;
void require(bool value,const char* why) {
    ++checks;if(!value)throw std::runtime_error(why);
}
std::string read(const fs::path& path) {
    std::ifstream file(path);require(file.is_open(),"Missing published failure audit");
    return {std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>()};
}
void save(const fs::path& path,const std::string& value) {
    std::ofstream file(path);file<<value;require(file.good(),"Cannot save test evidence");
}
ErrorDetail rootDetail() {
    ErrorDetail d;d.message="Injected root \"failure\"\n中文";d.stage=ErrorStage::Read;
    d.source_path="只读 source.mat";d.variable="xwerte";d.observation_index=7;
    d.run_id=RunId{"retained_inner_identity"};return d;
}
Failure<ExperimentErrorCode> numericalFailure() {
    ErrorDetail outer;outer.stage=ErrorStage::Simulation;outer.message="Injected Runner failure";
    return wrapFailure(ExperimentErrorCode::SolverFailed,std::move(outer),
        Failure<NumericalErrorCode>{NumericalErrorCode::UndefinedAngle,rootDetail()});
}
Failure<PlotErrorCode> plotFailure() {
    ErrorDetail outer;outer.stage=ErrorStage::Plot;outer.message="Injected exporter failure";
    return wrapFailure(PlotErrorCode::WriteFailed,std::move(outer),
        Failure<MatErrorCode>{MatErrorCode::WriteFailed,rootDetail()});
}
struct FailedRunner final:IExperimentRunner {
    RunId last;
    Result<RunOutcome,ExperimentErrorCode> fail(RunContext& ctx) {
        last=ctx.runId();return Result<RunOutcome,ExperimentErrorCode>::failure(numericalFailure());
    }
    Result<RunOutcome,ExperimentErrorCode> runBiasOccluded(const BiasExperimentConfig&,RunContext& c) override {return fail(c);}
    Result<RunOutcome,ExperimentErrorCode> runBiasProportional(const BiasExperimentConfig&,RunContext& c) override {return fail(c);}
    Result<RunOutcome,ExperimentErrorCode> runAdaptiveInitialization(const AdaptiveInitializationConfig&,RunContext& c) override {return fail(c);}
    Result<RunOutcome,ExperimentErrorCode> runAdaptiveComparison(const AdaptiveComparisonConfig&,RunContext& c) override {return fail(c);}
    Result<RunOutcome,ExperimentErrorCode> runAdaptiveUpdateRates(const AdaptiveUpdateRatesConfig&,RunContext& c) override {return fail(c);}
};
struct FixedAnalyzer final:IAnalyzer {
    Result<AnalysisDataset,AnalysisErrorCode> analyze(AnalysisKind,const fs::path&,IResultReader&) override {
        return Result<AnalysisDataset,AnalysisErrorCode>::success(AnalysisDataset{});
    }
};
struct FailedExporter final:IPlotExporter,illustrations::IExporter {
    Result<std::vector<std::string>,PlotErrorCode> exportPlot(const AnalysisDataset&,ITextPublisher&) override {
        return Result<std::vector<std::string>,PlotErrorCode>::failure(plotFailure());
    }
    Result<std::vector<std::string>,PlotErrorCode> exportSvg(const illustrations::Data&,ITextPublisher&) override {
        return Result<std::vector<std::string>,PlotErrorCode>::failure(plotFailure());
    }
};
std::string invoke(const fs::path& root,const cli::ApplicationCompositionRoot& app,
                   const std::vector<std::string>& args,int expected) {
    std::ostringstream out,err,metadata;
    const int code=cli::run(args,out,err,app);++calls;
    metadata<<"cwd="<<fs::current_path()<<"\nexpected_exit="<<expected<<"\nexit="<<code<<'\n';
    for(const auto& arg:args)metadata<<"argv="<<arg<<'\n';
    const auto log=root/"evidence"/std::to_string(calls);
    save(log.string()+".meta",metadata.str());save(log.string()+".stdout",out.str());
    save(log.string()+".stderr",err.str());
    require(code==expected&&out.str().empty(),"Wrong failure exit or unexpected success output");
    return err.str();
}
void verify(const fs::path& root) {
    fs::create_directories(root/"origin");fs::create_directories(root/"evidence");
    FailedRunner runner;FixedAnalyzer analyzer;FailedExporter exporter;io::MatResultReader reader;
    cli::ApplicationCompositionRoot app(&reader,&runner,&analyzer,&exporter,root,&exporter);
    const std::vector<std::vector<std::string>> commands={
        {"run-bias-occluded","--seed","7","--level","0"},
        {"run-bias-proportional","--seed","11","--level","0","--run-id","bias"},
        {"run-adaptive-init","--run-id","initialization"},
        {"run-adaptive-comparison","--noise-scale","0","--run-id","comparison"},
        {"run-adaptive-update-rates","--run-id","rates"}};
    for(const auto& command:commands) {
        const auto diagnostic=invoke(root,app,command,5);
        require(command.front()=="run-bias-occluded"
                    ? runner.last.text.size()==36&&runner.last.text.substr(0,4)=="run_"
                    : runner.last.text==command.back(),"Allocated identity differs from automatic/manual selection");
        auto expected=numericalFailure();expected.detail.run_id=runner.last;
        require(diagnostic==jsonFailure(expected,5)+"\n","CLI must fill only the allocated outer RunId");
        const auto manifest=read(root/"output"/runner.last.text/"manifest.json");
        require(manifest.find("\"errors\":["+jsonFailure(expected,5)+"]")!=std::string::npos,
                "stderr and stored error must be identical, including the complete causes");
        require(manifest.find("\"completeness\":\"Failed\"")!=std::string::npos,
                "Injected failure must never become Complete");
        std::cout<<"contextual failure RunId="<<runner.last.text<<'\n';
    }
    auto plot_expected=plotFailure();plot_expected.detail.run_id=RunId{"plot"};
    require(invoke(root,app,{"plot","--experiment","bias","--input-root","unused","--run-id","plot"},6)
                ==jsonFailure(plot_expected,6)+"\n","Plot failure lost allocated identity or changed causes");
    auto illustration_expected=plotFailure();illustration_expected.detail.run_id=RunId{"illustration"};
    require(invoke(root,app,{"illustrate","--kind","circle","--format","svg","--run-id","illustration"},6)
                ==jsonFailure(illustration_expected,6)+"\n","Illustration failure lost allocated identity");
    cli::ApplicationCompositionRoot no_exporter(&reader,&runner,&analyzer,nullptr,root);
    const auto unallocated=invoke(root,no_exporter,
        {"plot","--experiment","bias","--input-root","unused","--run-id","never_allocated"},7);
    require(unallocated.find("\"run_id\"")==std::string::npos&&
            !fs::exists(root/"output"/"never_allocated"),"Requested but unallocated RunId must not be invented");
    require(fs::is_empty(root/"origin"),"Original marker must remain empty");
}
} // namespace
int main(int argc,char** argv) {
    try {
        require(argc==2,"Expected isolated binary sandbox");
        const fs::path base=fs::path(argv[1])/"诊断 中文 空格";fs::create_directories(base);
        auto pattern=(base/"runid-XXXXXX").string();std::vector<char> bytes(pattern.begin(),pattern.end());bytes.push_back('\0');
        const char* created=::mkdtemp(bytes.data());require(created!=nullptr,"Cannot reserve fresh test evidence");
        const fs::path root(created);std::cout<<"evidence="<<root<<'\n';
        verify(root);std::cout<<"PASS RunId diagnostics checks="<<checks<<" CLI dispatches="<<calls<<'\n';return 0;
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
