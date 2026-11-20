#include "egomotion/cli/command.hpp"
#include "egomotion/contracts/diagnostics.hpp"
#include "egomotion/experiments/production_runner.hpp"
#include "egomotion/experiments/run_context.hpp"
#include "egomotion/io/mat_result_reader.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
enum class Fault { None, StartLink, StartSync, FinishLink, FinishSync };
Fault fault = Fault::None;
int fail_sync_fd = -1;
std::size_t hits = 0, checks = 0, calls = 0;
}
extern "C" int __real_linkat(int, const char*, int, const char*, int);
extern "C" int __real_fsync(int);
extern "C" int __wrap_linkat(int from_fd, const char* from, int to_fd, const char* to, int flags) {
    const bool start = std::strcmp(to, "invocation.json") == 0;
    const bool finish = std::strcmp(to, "execution.json") == 0;
    if ((start && fault == Fault::StartLink) || (finish && fault == Fault::FinishLink)) {
        ++hits; errno = EIO; return -1;
    }
    const int result = __real_linkat(from_fd, from, to_fd, to, flags);
    if (result == 0 && ((start && fault == Fault::StartSync) || (finish && fault == Fault::FinishSync)))
        fail_sync_fd = to_fd;
    return result;
}
extern "C" int __wrap_fsync(int fd) {
    if (fd == fail_sync_fd) {
        fail_sync_fd = -1; ++hits; errno = EIO; return -1;
    }
    return __real_fsync(fd);
}

namespace {
using namespace egomotion;
namespace fs = std::filesystem;
void require(bool value, const char* reason) {
    ++checks;
    if (!value) throw std::runtime_error(reason);
}
std::string read(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    require(file.good(), "Missing actual record");
    return {std::istreambuf_iterator<char>(file), {}};
}
void contains(const std::string& text, const std::string& expected, const char* reason) {
    require(text.find(expected) != std::string::npos, reason);
}
class TrackingRunner final : public IExperimentRunner {
public:
    std::size_t invocations = 0;
    bool real_bias = false, block_finish = false, throw_failure = false;
    bool note_progress_before_throw = false;
    Result<RunOutcome, ExperimentErrorCode> failed(RunContext& context) {
        ++invocations;
        const auto directory = context.approvedOutputRoot() / context.runId().text;
        contains(read(directory / "invocation.json"), "\"phase\":\"started\"",
                 "Invocation must be saved before calling numerical runner");
        if (block_finish) {
            std::ofstream existing(directory / "execution.json");
            existing << "retained blocker\n";
        }
        if (throw_failure) {
            if (note_progress_before_throw) {
                context.noteAttempt();
                context.noteValid();
                context.noteUndefined();
            }
            throw std::runtime_error("Injected \"runner\" exception\n第二行");
        }
        ErrorDetail detail; detail.stage = ErrorStage::Simulation;
        detail.message = "Injected \"runner\" failure\n第二行";
        return Result<RunOutcome, ExperimentErrorCode>::failure(
            {ExperimentErrorCode::SolverFailed, std::move(detail)});
    }
    Result<RunOutcome, ExperimentErrorCode> runBiasOccluded(const BiasExperimentConfig& c, RunContext& r) override {
        if (!real_bias) return failed(r);
        ++invocations;
        experiments::ProductionExperimentRunner production;
        return production.runBiasOccluded(c, r);
    }
    Result<RunOutcome, ExperimentErrorCode> runBiasProportional(const BiasExperimentConfig&, RunContext& r) override { return failed(r); }
    Result<RunOutcome, ExperimentErrorCode> runAdaptiveInitialization(const AdaptiveInitializationConfig&, RunContext& r) override { return failed(r); }
    Result<RunOutcome, ExperimentErrorCode> runAdaptiveComparison(const AdaptiveComparisonConfig&, RunContext& r) override { return failed(r); }
    Result<RunOutcome, ExperimentErrorCode> runAdaptiveUpdateRates(const AdaptiveUpdateRatesConfig&, RunContext& r) override { return failed(r); }
};
struct Response { int code; std::string out, err; };
Response invoke(const fs::path& root, TrackingRunner& runner,
                const std::vector<std::string>& args, bool direct = false) {
    ++calls;
    std::ostringstream out, err;
    cli::ApplicationCompositionRoot app(nullptr, &runner, nullptr, nullptr, root);
    const int code = direct ? app.dispatch(cli::parseCommand(args).value(), out, err)
                            : cli::run(args, out, err, app);
    return {code, out.str(), err.str()};
}
void allEntries(const fs::path& root) {
    TrackingRunner runner;
    const std::vector<std::vector<std::string>> arguments = {
        {"run-bias-occluded", "--seed", "17", "--level", "2"},
        {"run-bias-proportional", "--seed", "23", "--level", "4"},
        {"run-adaptive-init"},
        {"run-adaptive-comparison", "--noise-scale", "-0.125"},
        {"run-adaptive-update-rates"}};
    for (std::size_t i = 0; i < arguments.size(); ++i) {
        auto args = arguments[i];
        const auto id = "entry_" + std::to_string(i);
        args.insert(args.end(), {"--run-id", id});
        const auto result = invoke(root, runner, args);
        require(result.code == 5 && result.out.empty(), "Original calculation failure remains exit 5");
        const auto start = read(root / "output" / id / "invocation.json");
        const auto finish = read(root / "output" / id / "execution.json");
        contains(start, "\"command\":" + jsonQuote(args[0]), "Correct entry identity");
        contains(start, "\"arguments\":[" + jsonQuote(args[0]), "Original argument sequence");
        if (i == 0) contains(start, "\"effective_config\":{\"seed\":17,\"maximum_subdivision_level\":2}", "Actual Bias config");
        if (i == 1) contains(start, "\"effective_config\":{\"seed\":23,\"maximum_subdivision_level\":4}", "Independent Bias config");
        if (i == 3) contains(start, "\"effective_config\":{\"noise_scale\":-0.125}", "Finite signed noise config");
        contains(finish, "\"attempted_text\":" + jsonQuote(result.err), "Whole escaped JSONL diagnostic retained");
        contains(finish, "\"exit_code_if_record_published\":5", "Computed return status");
        contains(finish, "\"write_and_flush_succeeded\":true", "Actual successful diagnostic flush");
    }
    const auto response = invoke(root, runner,
        {"run-adaptive-init", "--run-id", "direct_dispatch"}, true);
    require(response.code == 5, "Direct dispatch remains supported");
    contains(read(root / "output/direct_dispatch/invocation.json"), "\"arguments\":null",
             "Do not reconstruct a fake argv for direct library dispatch");
}
void recordFaults(const fs::path& root) {
    for (const auto kind : {Fault::StartLink, Fault::StartSync, Fault::FinishLink, Fault::FinishSync}) {
        fault = kind; hits = 0;
        const auto id = "fault_" + std::to_string(static_cast<int>(kind));
        const bool start_failed = kind == Fault::StartLink || kind == Fault::StartSync;
        TrackingRunner runner;
        const auto result = invoke(root, runner,
            {"run-bias-occluded", "--seed", "7", "--level", "2", "--run-id", id});
        fault = Fault::None;
        require(hits == 1, "Publication fault must actually fire exactly once");
        require(result.code == 6, "Unconfirmed execution audit must return 6");
        require(runner.invocations == (start_failed ? 0U : 1U), "No computation before confirmed invocation record");
        const auto manifest = read(root / "output" / id / "manifest.json");
        contains(manifest, "\"completeness\":\"Failed\"", "Audit failure cannot fabricate completed calculation");
        if (start_failed) {
            contains(read(root / "output" / id / "execution.json"), "\"invocation_confirmed\":false",
                     "Published-but-unconfirmed start must stay unconfirmed");
            contains(result.err, "experiment was not started", "Explain pre-computation audit failure");
        } else {
            contains(manifest, "SolverFailed", "Independent end-audit error must not replace calculation error");
            if (kind == Fault::FinishSync) {
                const auto record = read(root / "output" / id / "execution.json");
                contains(record, "\"exit_code_if_record_published\":5", "Conditional command status retained");
                contains(record, "\"exit_code_on_record_failure\":6", "Actual audit failure return described separately");
            } else require(!fs::exists(root / "output" / id / "execution.json"), "Pre-publication error cannot claim record exists");
        }
    }
}
void runnerExceptions(const fs::path& root) {
    for (const bool direct : {false,true}) {
        for (const bool progress : {false,true}) {
            TrackingRunner runner;
            runner.throw_failure=true;
            runner.note_progress_before_throw=progress;
            const auto id=std::string("runner_exception_")+(direct?"direct_":"cli_")+
                (progress?"partial":"empty");
            const auto response=invoke(root,runner,
                {"run-adaptive-init","--run-id",id},direct);
            require(response.code==6&&response.out.empty()&&runner.invocations==1,
                    "Runner exception preserves CLI fallback without escaping dispatch");
            contains(response.err,"\"error_code\":\"OutputFailed\"",
                     "Do not reinterpret a standard exception as a numerical Failure");
            contains(response.err,"\"stage\":\"output\"","Keep existing exception stage");
            contains(response.err,"\"run_id\":"+jsonQuote(id),"Exception keeps reserved RunId");
            contains(response.err,"\"message\":"+jsonQuote("Injected \"runner\" exception\n第二行"),
                     "Exception text is retained and escaped");
            contains(response.err,"\"causes\":[]","No invented numerical cause chain");
            const auto directory=root/"output"/id;
            const auto manifest=read(directory/"manifest.json");
            contains(manifest,progress?"\"completeness\":\"Incomplete\"":
                                      "\"completeness\":\"Failed\"",
                     "Exception failure record reflects prior progress");
            contains(manifest,"\"completed\":0","Exception cannot fabricate completed run");
            contains(manifest,progress?"\"valid\":1":"\"valid\":0","Retain actual valid count");
            contains(manifest,progress?"\"undefined\":1":"\"undefined\":0",
                     "Retain metric absence separately from exception");
            contains(manifest,"OutputFailed","Manifest records original exception classification");
            const auto finish=read(directory/"execution.json");
            contains(finish,"\"invocation_confirmed\":true","Confirmed invocation remains recorded");
            contains(finish,"\"exit_code_if_record_published\":6","Exception execution record keeps exit 6");
            contains(finish,"\"attempted_text\":"+jsonQuote(response.err),
                     "Execution record retains entire exception diagnostic");
            require(!fs::exists(directory/"allemittransfehler.mat"),
                    "Runner exception cannot publish fabricated result arrays");
        }
    }
}
void preserveCompleteAndExisting(const fs::path& root) {
    TrackingRunner runner; runner.real_bias = true;
    fault = Fault::FinishLink; hits = 0;
    const auto response = invoke(root, runner,
        {"run-bias-occluded", "--seed", "7", "--level", "2", "--run-id", "complete"});
    fault = Fault::None;
    require(hits == 1 && response.code == 6, "Final audit failure must not be hidden after successful calculation");
    contains(read(root / "output/complete/manifest.json"), "\"completeness\":\"Complete\"",
             "Do not downgrade or overwrite successful calculation evidence");
    require(fs::exists(root / "output/complete/ywerte6.mat"), "Keep published MAT data");
    runner.real_bias = false; runner.block_finish = true;
    const auto blocked = invoke(root, runner, {"run-adaptive-init", "--run-id", "blocked"});
    require(blocked.code == 6, "Existing execution record is a publication failure");
    require(read(root / "output/blocked/execution.json") == "retained blocker\n", "Never overwrite previous execution evidence");
}
class FixedAnalyzer final : public IAnalyzer {
public:
    bool revised = true;
    Result<AnalysisDataset,AnalysisErrorCode> analyze(AnalysisKind,const fs::path&,IResultReader&) override {
        AnalysisDataset data;data.source_analysis_blob="fixture-analysis-blob";
        AnalysisSeries curve;curve.name="曲线\"\n";curve.x={1};curve.y={2};
        if(revised) {
            data.metric_policy="adaptive-atan2-so3-geodesic-v1";
            data.planned_repetitions=1;
            curve.metric_semantics="rotation-so3-geodesic-degrees";
            curve.y_unit="rotation SO(3) geodesic error in degree";
        }
        curve.helper_source_blob_shas={"fixture-helper"};
        // 故意不可打开；记录必须借用Analyzer快照身份，不能重读这些路径。
        curve.inputs={{"missing/中文 引号\".mat","v",7,"retained snapshot",std::string(64,'a')},
                      {"missing/unknown.mat","u",8,"unknown digest",std::nullopt}};
        data.curves.push_back(std::move(curve));
        return Result<AnalysisDataset,AnalysisErrorCode>::success(std::move(data));
    }
};
class AnalysisObserver final : public cli::IInvocationObserver {
public:
    std::string ready;
    Status<MatErrorCode> append(std::string_view event,std::string_view data) override {
        if(event=="analysis_ready")ready=std::string(data);
        return Status<MatErrorCode>::success();
    }
};
void metricIdentity(const std::string& record,bool revised) {
    contains(record,"\"metric_policy\":"+jsonQuote(revised?
        "adaptive-atan2-so3-geodesic-v1":"source-defined"),"Retain actual analysis policy");
    contains(record,revised?"\"planned_repetitions\":1":"\"planned_repetitions\":0",
        "Do not infer repetitions for an unknown historical result");
    contains(record,"\"metric_semantics\":"+jsonQuote(revised?
        "rotation-so3-geodesic-degrees":""),"Keep each curve metric distinct from source blob");
    contains(record,"\"y_unit\":"+jsonQuote(revised?
        "rotation SO(3) geodesic error in degree":""),"Keep actual metric units");
}
class TrackingExporter final : public IPlotExporter,public illustrations::IExporter {
public:
    fs::path directory;
    std::size_t invocations=0;
    bool throws=false,block_finish=false;
    std::optional<PlotErrorCode> failure;
    Result<std::vector<std::string>,PlotErrorCode> publish(ITextPublisher& publisher) {
        ++invocations;
        contains(read(directory/"invocation.json"),"\"phase\":\"started\"","Record precedes exporter");
        require(publisher.publishNew("kept.txt","Retained artifact\n").hasValue(),"Artifact fixture publication");
        if(block_finish)std::ofstream(directory/"execution.json")<<"retained blocker\n";
        if(throws)throw std::runtime_error("Injected exporter exception");
        if(failure) {
            ErrorDetail inner;inner.stage=ErrorStage::Read;inner.source_path="输入.mat";
            inner.variable="value";inner.observation_index=7;inner.run_id=RunId{"source_run"};
            inner.message="Injected nested failure\n第二行";
            ErrorDetail outer;outer.stage=ErrorStage::Plot;outer.message="Injected exporter failure";
            return Result<std::vector<std::string>,PlotErrorCode>::failure(wrapFailure(*failure,
                std::move(outer),Failure<MatErrorCode>{MatErrorCode::InvalidMatHeader,std::move(inner)}));
        }
        return Result<std::vector<std::string>,PlotErrorCode>::success({"kept.txt"});
    }
    Result<std::vector<std::string>,PlotErrorCode> exportPlot(const AnalysisDataset&,ITextPublisher& p) override {return publish(p);}
    Result<std::vector<std::string>,PlotErrorCode> exportSvg(const illustrations::Data&,ITextPublisher& p) override {return publish(p);}
};
class FlushFailure final : public std::stringbuf {
    int sync() override {return -1;}
};
Response artifact(const fs::path& root,TrackingExporter& exporter,bool plot,const std::string& id,
                  bool direct=false,bool bad_out=false,bool bad_err=false,bool revised=true) {
    ++calls;
    io::MatResultReader reader;FixedAnalyzer analyzer;analyzer.revised=revised;
    AnalysisObserver observer;
    cli::ApplicationCompositionRoot app(&reader,nullptr,&analyzer,&exporter,root,&exporter,&observer);
    auto args=plot?std::vector<std::string>{"plot","--experiment",revised?"adaptive-init":"bias","--input-root","missing"}:
        std::vector<std::string>{"illustrate","--kind","harmonic","--degree","3","--order","2","--format","svg"};
    args.insert(args.end(),{"--run-id",id});
    exporter.directory=root/"output"/id;
    FlushFailure out_buffer,err_buffer;
    std::ostringstream good_out,good_err;
    std::ostream failed_out(&out_buffer),failed_err(&err_buffer);
    std::ostream& out=bad_out?failed_out:good_out;
    std::ostream& err=bad_err?failed_err:good_err;
    auto parsed=cli::parseCommand(args);
    require(parsed.hasValue(),"Artifact fixture must satisfy established CLI contract");
    const auto code=direct?app.dispatch(parsed.value(),out,err):cli::run(args,out,err,app);
    if(plot)metricIdentity(observer.ready,revised);
    return {code,bad_out?out_buffer.str():good_out.str(),bad_err?err_buffer.str():good_err.str()};
}
void artifactSuccess(const fs::path& root,bool plot,bool revised=true) {
    TrackingExporter exporter;
    const auto response=artifact(root,exporter,plot,revised?"success":"legacy_success",true,false,false,revised);
    require(response.code==0&&response.err.empty()&&exporter.invocations==1,"Successful export");
    const auto start=read(exporter.directory/"invocation.json");
    const auto finish=read(exporter.directory/"execution.json");
    contains(start,"\"arguments\":null","Direct dispatch does not invent argv");
    contains(start,"\"engine\":null,\"seed\":null,\"seed_origin\":\"not_used\"","No scientific RNG used");
    contains(start,"\"calculation_manifest\":null","No invented calculation manifest");
    contains(finish,"\"calculation_manifest\":null","No invented end manifest");
    contains(finish,"\"attempted_text\":"+jsonQuote(response.out),"Whole output retained");
    contains(finish,"\"exit_code_if_record_published\":0","Successful audit exit");
    require(!fs::exists(exporter.directory/"manifest.json"),"No computation manifest for export");
    if(plot) {
        metricIdentity(start,revised);
        contains(start,"\"mat_sha256\":"+jsonQuote(std::string(64,'a')),"Retain snapshot digest");
        contains(start,"\"mat_sha256\":null","Unknown digest remains unknown");
        contains(start,"\"matlab_run_ordinal\":7","Keep 1-based source ordinal");
        contains(start,"\"path\":"+jsonQuote("missing/中文 引号\".mat"),"Escaped original path");
        contains(start,"fixture-helper","Retain source helper");
    } else contains(start,"\"kind\":\"harmonic\",\"degree\":3,\"order\":2","Actual harmonic config");
}
void artifactRecordFaults(const fs::path& root,bool plot) {
    for(const auto kind:{Fault::StartLink,Fault::StartSync,Fault::FinishLink,Fault::FinishSync}) {
        TrackingExporter exporter;fault=kind;hits=0;
        const bool start_failed=kind==Fault::StartLink||kind==Fault::StartSync;
        const auto response=artifact(root,exporter,plot,"record_"+std::to_string(static_cast<int>(kind)));
        fault=Fault::None;
        require(response.code==6&&hits==1,"Artifact audit fault actually fired");
        require(exporter.invocations==(start_failed?0U:1U),"No exporter before confirmed start");
        require(fs::exists(exporter.directory/"kept.txt")==!start_failed,"Preserve published artifact");
        require(fs::exists(exporter.directory/"invocation.json")==(kind!=Fault::StartLink),"Start publication state");
        if(kind==Fault::FinishLink)require(!fs::exists(exporter.directory/"execution.json"),"Unpublished end absent");
        else {
            const auto finish=read(exporter.directory/"execution.json");
            contains(finish,start_failed?"\"invocation_confirmed\":false":"\"invocation_confirmed\":true","Actual start confirmation");
            contains(finish,start_failed?"\"exit_code_if_record_published\":6":"\"exit_code_if_record_published\":0","Conditional outcome");
            contains(finish,"\"exit_code_on_record_failure\":6","Record failure condition retained");
        }
    }
}
void artifactFailures(const fs::path& root,bool plot) {
    const std::vector<std::pair<PlotErrorCode,int>> failures={
        {PlotErrorCode::BackendUnavailable,7},{PlotErrorCode::InvalidSeries,5},
        {PlotErrorCode::OutputForbidden,6},{PlotErrorCode::WriteFailed,6}};
    for(std::size_t i=0;i<failures.size();++i) {
        TrackingExporter exporter;exporter.failure=failures[i].first;
        const auto response=artifact(root,exporter,plot,"export_"+std::to_string(i));
        require(response.code==failures[i].second&&response.out.empty(),"Original export classification");
        contains(response.err,"\"path\":\"输入.mat\"","Nested source identity");
        contains(response.err,"\"run_id\":\"source_run\"","Nested RunId retained");
        contains(response.err,"\"observation_index\":7","Nested observation index retained");
        contains(read(exporter.directory/"execution.json"),"\"attempted_text\":"+jsonQuote(response.err),"Complete diagnostic chain retained");
        require(read(exporter.directory/"kept.txt")=="Retained artifact\n","Failure preserves prior artifact");
    }
    for(const auto mode:{"exception","blocked","stdout","stderr"}) {
        TrackingExporter exporter;const std::string id(mode);
        exporter.throws=id=="exception";exporter.block_finish=id=="blocked";
        if(id=="stderr")exporter.failure=PlotErrorCode::InvalidSeries;
        const auto response=artifact(root,exporter,plot,id,false,id=="stdout",id=="stderr");
        require(response.code==6,"Exception, record collision and stream failures return 6");
        require(read(exporter.directory/"kept.txt")=="Retained artifact\n","All failures preserve artifact");
        const auto finish=read(exporter.directory/"execution.json");
        if(id=="blocked")require(finish=="retained blocker\n","End record never overwritten");
        else {
            contains(finish,"\"exit_code_if_record_published\":6","Final output failure retained");
            if(id=="exception")contains(finish,"Injected exporter exception","Thrown exporter recorded with RunId");
            else contains(finish,"\"write_and_flush_succeeded\":false","Stream delivery failure retained");
        }
    }
}
void artifacts(const fs::path& root) {
    for(const bool plot:{true,false}) {
        const auto repo=root/(plot?"plot":"illustration");fs::create_directories(repo/"origin");
        artifactSuccess(repo,plot);artifactRecordFaults(repo,plot);artifactFailures(repo,plot);
        if(plot)artifactSuccess(repo,plot,false);
        require(fs::is_empty(repo/"origin"),"Artifact original marker untouched");
    }
}
}  // namespace
int main(int argc, char** argv) {
    require(argc == 2, "Expected sandbox directory");
    fs::create_directories(argv[1]);
    std::string pattern = (fs::path(argv[1]) / "运行记录 中文-XXXXXX").string();
    require(::mkdtemp(pattern.data()) != nullptr, "Cannot reserve new execution sandbox");
    const fs::path root(pattern); fs::create_directory(root / "origin");
    allEntries(root); recordFaults(root); runnerExceptions(root);
    preserveCompleteAndExisting(root); artifacts(root);
    require(fs::is_empty(root / "origin"), "Original marker untouched");
    std::cout << "PASS execution records checks=" << checks << " calls=" << calls
              << " evidence=" << root << '\n';
}
