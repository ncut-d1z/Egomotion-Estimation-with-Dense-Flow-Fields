#include "egomotion/cli/command.hpp"
#include "../run_diagnostics.hpp"
#include "stream_output.hpp"

#include "egomotion/analysis/dataset_json.hpp"
#include "egomotion/contracts/diagnostics.hpp"
#include "egomotion/experiments/output_contract.hpp"
#include "egomotion/experiments/run_context.hpp"
#include "egomotion/io/run_directory.hpp"
#include "egomotion/io/run_mat_writer.hpp"
#include "egomotion/io/run_text_publisher.hpp"
#include "egomotion/sampling/random.hpp"

#include <cerrno>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <locale>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/random.h>
#include <utility>
#include <vector>

#ifndef EGOMOTION_BUILD_SOURCE_SHA
// R6 must inject the build's ACTUAL source commit; the Manifest validator
// deliberately refuses to classify an unverified build as Complete.
#define EGOMOTION_BUILD_SOURCE_SHA "UNVERIFIED_BUILD_SOURCE"
#endif

namespace egomotion::cli {
namespace {
int matExit(MatErrorCode code) {
    switch(code){
    case MatErrorCode::UnsupportedClassOrType:
    case MatErrorCode::InvalidShape:
    case MatErrorCode::SizeOverflow:return 4;
    case MatErrorCode::OutputPathForbidden:
    case MatErrorCode::OutputExists:
    case MatErrorCode::WriteFailed:
    case MatErrorCode::ReadbackMismatch:return 6;
    default:return 3;
    }
}
int analysisExit(AnalysisErrorCode code) {
    switch(code){
    case AnalysisErrorCode::MissingResult:
    case AnalysisErrorCode::MissingVariable:return 3;
    case AnalysisErrorCode::ShapeMismatch:return 4;
    default:return 5;
    }
}
int plotExit(PlotErrorCode code) {
    switch(code){
    case PlotErrorCode::BackendUnavailable:return 7;
    case PlotErrorCode::OutputForbidden:
    case PlotErrorCode::WriteFailed:return 6;
    case PlotErrorCode::InvalidSeries:return 5;
    }
    return 5;
}
int unavailable(std::ostream& err,std::string_view stage,
                 std::string_view component,int code=5) {
    return streams::writeAndFlush(err,jsonBackendUnavailable(stage,
        std::string(component)+" backend is not connected",code)+"\n")?code:6;
}
bool randomSeed(std::uint32_t& seed) {
    auto* bytes=reinterpret_cast<unsigned char*>(&seed);
    std::size_t remain=sizeof(seed);
    while(remain){
        const auto n=::getrandom(bytes,remain,0);
        if(n<0){if(errno==EINTR)continue;return false;}
        if(n==0)return false;
        bytes+=n;remain-=static_cast<std::size_t>(n);
    }
    return true;
}
experiments::ExperimentKind sourceKind(CommandKind kind) {
    switch(kind){
    case CommandKind::BiasOccluded:return experiments::ExperimentKind::BiasOccluded;
    case CommandKind::BiasProportional:return experiments::ExperimentKind::BiasProportional;
    case CommandKind::AdaptiveInitialization:return experiments::ExperimentKind::AdaptiveInitialization;
    case CommandKind::AdaptiveComparison:return experiments::ExperimentKind::AdaptiveComparison;
    case CommandKind::AdaptiveUpdateRates:return experiments::ExperimentKind::AdaptiveUpdateRates;
    default:throw std::invalid_argument("Not an experiment command");
    }
}
std::string sourceBlob(CommandKind kind) {
    switch(kind){
    case CommandKind::BiasOccluded:return "00a3e974fbf068ccd1af4e541a659862beffcad5";
    case CommandKind::BiasProportional:return "fc3c4f0a7a417171415fb5e3d869a744ba24d04f";
    case CommandKind::AdaptiveInitialization:return "a3154ae391be82ccf48c9f3a03c55109dde85f65";
    case CommandKind::AdaptiveComparison:return "c9548c83fd0a2ce13d96394e98f6c7573119a7d5";
    case CommandKind::AdaptiveUpdateRates:return "4629901c1233cdee5d6d22c917603720ffd1fab4";
    default:throw std::invalid_argument("Not an experiment source");
    }
}
std::optional<RunId> runIdOption(const Command& command) {
    switch(command.kind){
    case CommandKind::BiasOccluded:
    case CommandKind::BiasProportional:
        return std::get<BiasArgs>(command.arguments).run_id;
    case CommandKind::AdaptiveInitialization:
        return std::get<AdaptiveInitArgs>(command.arguments).run_id;
    case CommandKind::AdaptiveComparison:
        return std::get<AdaptiveComparisonArgs>(command.arguments).run_id;
    case CommandKind::AdaptiveUpdateRates:
        return std::get<AdaptiveRatesArgs>(command.arguments).run_id;
    default:return std::nullopt;
    }
}
std::size_t expectedRuns(CommandKind kind) {
    return kind==CommandKind::AdaptiveComparison?8:1;
}
int failure(std::ostream& err,ExperimentErrorCode code,std::string message,int exit=5){
    ErrorDetail d;d.stage=ErrorStage::Simulation;d.message=std::move(message);
    return streams::report(err,Failure<ExperimentErrorCode>{code,std::move(d)},exit);
}
int runExperiment(const Command& command,
                  const std::filesystem::path& trusted_root,
                  IExperimentRunner& runner,std::ostream& out,std::ostream& err) {
    if(trusted_root.empty())
        return failure(err,ExperimentErrorCode::SourceUnavailable,
                       "No trusted repository checkout for output root",6);
    std::uint32_t seed=0;
    if(command.kind==CommandKind::BiasOccluded||
       command.kind==CommandKind::BiasProportional)
        seed=std::get<BiasArgs>(command.arguments).config.seed;
    else if(!randomSeed(seed))
        return failure(err,ExperimentErrorCode::SourceUnavailable,
                       "OS random source unavailable for experiment seed");
    // No scientific RNG is consumed for generating a RunId.
    auto reservation=io::RunDirectory::create(trusted_root,runIdOption(command));
    if(!reservation.hasValue()){
        const auto exit=matExit(reservation.error().code);
        return streams::report(err,reservation.error(),exit);
    }
    auto directory=std::move(reservation).value();
    io::RunMatWriter writer(*directory);
    io::RunTextPublisher text(*directory);
    auto random=std::make_unique<sampling::Mt19937RandomSource>(seed);
    SourceProvenance provenance;
    provenance.repository_commit_sha=EGOMOTION_BUILD_SOURCE_SHA;
    provenance.matlab_source_blob_sha=sourceBlob(command.kind);
    provenance.rng_seed=seed;
    RunContext context(directory->runId(),std::move(random),writer,text,
                       directory->outputRoot(),std::move(provenance),
                       sourceKind(command.kind),expectedRuns(command.kind));
    const auto result=[&]() -> Result<RunOutcome,ExperimentErrorCode> {
        switch(command.kind){
        case CommandKind::BiasOccluded:
            return runner.runBiasOccluded(
                std::get<BiasArgs>(command.arguments).config,context);
        case CommandKind::BiasProportional:
            return runner.runBiasProportional(
                std::get<BiasArgs>(command.arguments).config,context);
        case CommandKind::AdaptiveInitialization:
            return runner.runAdaptiveInitialization(
                AdaptiveInitializationConfig{},context);
        case CommandKind::AdaptiveComparison:
            return runner.runAdaptiveComparison(
                std::get<AdaptiveComparisonArgs>(command.arguments).config,context);
        case CommandKind::AdaptiveUpdateRates:
            return runner.runAdaptiveUpdateRates(
                AdaptiveUpdateRatesConfig{},context);
        default:throw std::logic_error("Experiment dispatch received non-experiment");
        }
    }();
    if(!result.hasValue()) {
        const int code=result.error().detail.stage==ErrorStage::Output?6:5;
        // The computational failure and an independent Manifest failure are
        // distinct events: never fake a common causal chain for both.
        const auto error=detail::withRunIdIfMissing(result.error(),context.runId());
        const int reported=streams::report(err,error,code);
        // stderr失败不阻止独立Manifest通道保存原始计算失败。
        auto status=context.publishFailureRecord(error);
        if(!status.hasValue()&&!streams::diagnostic(err,status.error(),6))
            return 6;
        return reported;
    }
    const auto& state=result.value();
    std::ostringstream response;
    response.imbue(std::locale::classic());
    response.exceptions(std::ios::badbit|std::ios::failbit);
    response<<"{\"run_id\":"<<jsonQuote(state.run_id.text)
       <<",\"status\":\"Complete\",\"attempted\":"<<state.attempted
       <<",\"completed\":"<<state.completed
       <<",\"valid\":"<<state.valid
       <<",\"failed\":"<<state.failed
       <<",\"undefined\":"<<state.undefined<<"}\n";
    return streams::success(out,err,response.str(),&state.run_id);
}
int analyzeOrPlot(const Command& cmd,IAnalyzer& analyzer,
                  IPlotExporter* plotter,IResultReader& reader,
                  const std::filesystem::path& repository_root,
                  std::ostream& out,std::ostream& err) {
    const auto& args=std::get<AnalysisArgs>(cmd.arguments);
    auto result=analyzer.analyze(args.kind,args.input_root,reader);
    if(!result.hasValue()){
        const int code=analysisExit(result.error().code);
        return streams::report(err,result.error(),code);
    }
    if(cmd.kind==CommandKind::Analyze){
        auto json=analysis::datasetJson(result.value());
        if(!json.hasValue()){
            const int code=analysisExit(json.error().code);
            return streams::report(err,json.error(),code);
        }
        return streams::success(out,err,json.value());
    }
    if(!plotter){
        return unavailable(err,"plot","Optional SVG exporter",7);
    }
    auto reserved=io::RunDirectory::create(repository_root,args.run_id);
    if(!reserved.hasValue()){
        const auto code=matExit(reserved.error().code);
        return streams::report(err,reserved.error(),code);
    }
    auto dir=std::move(reserved).value();
    io::RunTextPublisher publisher(*dir);
    auto exported=plotter->exportPlot(result.value(),publisher);
    if(!exported.hasValue()){
        const auto code=plotExit(exported.error().code);
        return streams::report(err,detail::withRunIdIfMissing(exported.error(),dir->runId()),code);
    }
    std::ostringstream response;
    response.imbue(std::locale::classic());
    response.exceptions(std::ios::badbit|std::ios::failbit);
    response<<"{\"run_id\":"<<jsonQuote(dir->runId().text)<<",\"plot_files\":[";
    bool first=true;
    for(const auto& file:exported.value()){
        if(!first)response<<",";
        first=false;response<<jsonQuote(file);
    }
    response<<"]}\n";
    return streams::success(out,err,response.str(),&dir->runId());
}
int illustrate(const IllustrationArgs& args,illustrations::IExporter* exporter,
               const std::filesystem::path& repository_root,std::ostream& out,std::ostream& err) {
    auto data=illustrations::generate(args.config);
    if(!data.hasValue())return streams::report(err,data.error(),5);
    if(!args.svg) {
        auto json=illustrations::dataJson(data.value());
        if(!json.hasValue())return streams::report(err,json.error(),5);
        return streams::success(out,err,json.value());
    }
    if(!exporter)return unavailable(err,"plot","Optional illustration SVG exporter",7);
    auto reserved=io::RunDirectory::create(repository_root,args.run_id);
    if(!reserved.hasValue()) {
        const auto code=matExit(reserved.error().code);
        return streams::report(err,reserved.error(),code);
    }
    auto directory=std::move(reserved).value();
    io::RunTextPublisher publisher(*directory);
    auto files=exporter->exportSvg(data.value(),publisher);
    if(!files.hasValue()) {
        const auto code=plotExit(files.error().code);
        return streams::report(err,detail::withRunIdIfMissing(files.error(),directory->runId()),code);
    }
    std::ostringstream response;
    response.imbue(std::locale::classic());
    response.exceptions(std::ios::badbit|std::ios::failbit);
    response<<"{\"run_id\":"<<jsonQuote(directory->runId().text)<<",\"plot_files\":[";
    for(std::size_t i=0;i<files.value().size();++i){if(i)response<<',';response<<jsonQuote(files.value()[i]);}
    response<<"]}\n";
    return streams::success(out,err,response.str(),&directory->runId());
}
} // namespace

int ApplicationCompositionRoot::dispatch(
    const Command& command,std::ostream& out,std::ostream& err) const {
    switch(command.kind){
    case CommandKind::Help:
        return streams::success(out,err,"Usage: egomotion <command> [--option VALUE]\n"
              "Commands: inspect-mat, run-bias-occluded, run-bias-proportional,\n"
              "  run-adaptive-init, run-adaptive-comparison,\n"
              "  run-adaptive-update-rates, analyze, plot, illustrate, help, version\n"
              "illustrate --kind circle|sinusoidal|harmonic [--format json|svg]\n"
              "  harmonic requires --degree N --order M; --run-id ID is SVG-only\n");
    case CommandKind::Version:
        return streams::success(out,err,"egomotion 0.1.0 (R5 full-source bootstrap; native checks pending)\n");
    case CommandKind::InspectMat: {
        if(!reader_)return unavailable(err,"read","MAT Reader");
        const auto& params=std::get<InspectMatArgs>(command.arguments);
        auto inspected=reader_->inspect(params.source);
        if(!inspected.hasValue()){
            const int code=matExit(inspected.error().code);
            return streams::report(err,inspected.error(),code);
        }
        bool matched=false;
        std::ostringstream response;
        response.imbue(std::locale::classic());
        response.exceptions(std::ios::badbit|std::ios::failbit);
        for(const auto& v:inspected.value()){
            if(params.variable&&v.name!=*params.variable)continue;
            matched=true;
            response<<"variable="<<v.name<<" class="<<v.original_class_type
               <<" type="<<v.original_data_type
               <<" rank="<<v.dimensions.size()<<" dims=";
            for(std::size_t i=0;i<v.dimensions.size();++i){
                if(i)response<<'x';
                response<<v.dimensions[i];
            }
            response<<" data_bytes="<<v.data_bytes<<" compressed="<<v.is_compressed
               <<" complex="<<v.is_complex<<" logical="<<v.is_logical
               <<" global="<<v.is_global<<"\n";
        }
        if(params.variable&&!matched){
            ErrorDetail d;d.stage=ErrorStage::Read;
            d.message="MAT variable not found";
            d.source_path=params.source.string();d.variable=*params.variable;
            return streams::report(err,Failure<MatErrorCode>{
                MatErrorCode::MissingVariable,std::move(d)},3);
        }
        return streams::success(out,err,response.str());
    }
    case CommandKind::Analyze:
    case CommandKind::Plot:
        if(!reader_||!analyzer_)return unavailable(err,"analyze","Analyzer/Reader");
        return analyzeOrPlot(command,*analyzer_,plotter_,*reader_,
                             trusted_repo_,out,err);
    case CommandKind::Illustrate:
        return illustrate(std::get<IllustrationArgs>(command.arguments),illustrator_,trusted_repo_,out,err);
    default:
        if(!runner_)return unavailable(err,"simulation","Experiment Runner");
        return runExperiment(command,trusted_repo_,*runner_,out,err);
    }
}
int run(const std::vector<std::string>& args,std::ostream& out,
        std::ostream& err,const ApplicationCompositionRoot& app) {
    try{
        auto parsed=parseCommand(args);
        if(!parsed.hasValue()){
            return streams::report(err,parsed.error(),2);
        }
        return app.dispatch(parsed.value(),out,err);
    }catch(const std::exception& e){
        return streams::outputFailure(err,e.what());
    }
}
} // namespace egomotion::cli
