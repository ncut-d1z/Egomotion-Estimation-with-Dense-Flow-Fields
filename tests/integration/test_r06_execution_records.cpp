#include "egomotion/cli/command.hpp"
#include "egomotion/contracts/diagnostics.hpp"
#include "egomotion/experiments/production_runner.hpp"
#include "egomotion/experiments/run_context.hpp"

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
    bool real_bias = false, block_finish = false;
    Result<RunOutcome, ExperimentErrorCode> failed(RunContext& context) {
        ++invocations;
        const auto directory = context.approvedOutputRoot() / context.runId().text;
        contains(read(directory / "invocation.json"), "\"phase\":\"started\"",
                 "Invocation must be saved before calling numerical runner");
        if (block_finish) {
            std::ofstream existing(directory / "execution.json");
            existing << "retained blocker\n";
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
}  // namespace
int main(int argc, char** argv) {
    require(argc == 2, "Expected sandbox directory");
    fs::create_directories(argv[1]);
    std::string pattern = (fs::path(argv[1]) / "运行记录 中文-XXXXXX").string();
    require(::mkdtemp(pattern.data()) != nullptr, "Cannot reserve new execution sandbox");
    const fs::path root(pattern); fs::create_directory(root / "origin");
    allEntries(root); recordFaults(root); preserveCompleteAndExisting(root);
    require(fs::is_empty(root / "origin"), "Original marker untouched");
    std::cout << "PASS execution records checks=" << checks << " calls=" << calls
              << " evidence=" << root << '\n';
}
