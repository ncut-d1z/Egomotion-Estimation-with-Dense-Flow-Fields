#include "context_journal.hpp"
#include "egomotion/cli/command.hpp"
#include "egomotion/contracts/diagnostics.hpp"
#include "egomotion/experiments/run_context.hpp"
#include "egomotion/io/mat_result_reader.hpp"
#include "egomotion/io/sha256.hpp"

#include <matio.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>

namespace {
using namespace egomotion;
namespace fs = std::filesystem;
enum class Fault { None, Interrupted, Short, Zero, Write, Sync };
Fault fault = Fault::None;
int journal_fd = -1;
unsigned hits = 0, opened = 0, decoded = 0, metadata = 0;
std::size_t checks = 0;
void require(bool value, const char* message) {
    ++checks; if (!value) throw std::runtime_error(message);
}
void contains(const std::string& value, const std::string& part) {
    ++checks;
    if (value.find(part) == std::string::npos) throw std::runtime_error("Missing context field: " + part + "; actual=" + value);
}
std::string read(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    require(input.good(), "Missing evidence file");
    return {std::istreambuf_iterator<char>(input), {}};
}
class Capture final : public cli::IInvocationObserver {
public:
    std::vector<std::pair<std::string, std::string>> events;
    std::string fail_event;
    std::function<void()> snapshot;
    Status<MatErrorCode> append(std::string_view event, std::string_view data) override {
        events.emplace_back(event, data);
        if (event == "mat_snapshot_prepared" && snapshot) snapshot();
        if (event != fail_event) return Status<MatErrorCode>::success();
        ErrorDetail detail; detail.stage = ErrorStage::Output; detail.variable = std::string(event);
        detail.message = "Injected journal failure";
        return Status<MatErrorCode>::failure({MatErrorCode::WriteFailed, std::move(detail)});
    }
};
std::string hash(const fs::path& path) {
    const int fd = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
    require(fd >= 0, "Fixture open");
    const auto result = io::sha256Fd(fd); ::close(fd);
    require(result.hasValue(), "Fixture SHA"); return result.value();
}
void fixture(const fs::path& path, double start) {
    auto* mat = Mat_CreateVer(path.c_str(), "Context snapshot fixture", MAT_FT_MAT5);
    require(mat != nullptr, "Create synthetic MAT");
    size_t dims[2] = {2, 3}; double data[] = {start, start+1, start+2, start+3, start+4, start+5};
    auto* value = Mat_VarCreate(start == 1 ? "alpha" : "beta", MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, data, 0);
    require(value != nullptr, "Create synthetic variable");
    const int written = Mat_VarWrite(mat, value, MAT_COMPRESSION_ZLIB); Mat_VarFree(value);
    const int closed = Mat_Close(mat); require(written == 0 && closed == 0, "Finish synthetic MAT");
}
void journalCases(const fs::path& root) {
    for (auto kind : {Fault::None, Fault::Interrupted, Fault::Short, Fault::Zero, Fault::Write, Fault::Sync}) {
        const auto path = root / ("journal-" + std::to_string(static_cast<int>(kind)) + ".jsonl");
        journal_fd = ::open(path.c_str(), O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, 0600);
        require(journal_fd >= 0, "Create journal");
        cli::ContextJournal journal(journal_fd, "record_fixture", path);
        fault = kind; hits = 0;
        const bool success = kind == Fault::None || kind == Fault::Interrupted || kind == Fault::Short;
        const auto first = journal.record("quoted\"\n中文", "{\"run_id\":\"actual\"}\n");
        require(first.hasValue() == success, "Journal fault outcome");
        require(hits == (kind == Fault::None ? 0U : 1U), "Fault actually exercised");
        fault = Fault::None;
        const auto before = read(path);
        const auto next = journal.record("next", "{}");
        require(next.hasValue() == success, "First journal failure remains sticky");
        if (!success) {
            require(read(path) == before, "No writes after failed confirmation");
            require(next.error().code == MatErrorCode::WriteFailed && next.error().detail.stage == ErrorStage::Output,
                    "Journal failures use existing Output error");
        } else {
            const auto bytes = read(path);
            contains(bytes, "\"sequence\":1,"); contains(bytes, "\"sequence\":2,");
            contains(bytes, "\"event\":\"quoted\\\"\\n中文\"");
            contains(bytes, "\"data\":{\"run_id\":\"actual\"}}\n");
            require(std::count(bytes.begin(), bytes.end(), '\n') == 2, "Each event is exactly one JSONL line");
        }
        ::close(journal_fd); journal_fd = -1;
    }
}
void snapshots(const fs::path& root) {
    fixture(root / "A.mat", 1); fixture(root / "B.mat", 101);
    const auto expected = hash(root / "A.mat");
    for (int mode = 0; mode < 4; ++mode) for (const bool overwrite : {false, true}) {
        const auto dir = root / ("snapshot-" + std::to_string(mode) + (overwrite ? "-inode" : "-path"));
        fs::create_directory(dir);
        const auto source = dir / "中文 输入.mat", replacement = dir / "replacement.mat";
        fs::copy_file(root / "A.mat", source); fs::copy_file(root / "B.mat", replacement);
        Capture observer;
        observer.snapshot = [&] {
            require(opened == 0 && decoded == 0 && metadata == 0, "Observe frozen identity before matio opens");
            if (overwrite) {
                std::ifstream input(replacement, std::ios::binary);
                std::ofstream output(source, std::ios::binary | std::ios::trunc);
                output << input.rdbuf(); output.close(); require(!output.fail(), "Overwrite fixture inode");
            } else fs::rename(replacement, source);
        };
        io::MatResultReader reader(&observer);
        opened = decoded = metadata = 0;
        if (mode == 0) {
            const auto info = reader.inspect(source);
            require(info.hasValue() && info.value().size() == 1 && info.value()[0].name == "alpha", "Metadata snapshot");
            require(decoded == 0 && metadata == 1, "Inspect never invokes numeric decode");
        } else if (mode == 3) {
            const auto scan = reader.scanFile(source, io::ScanMode::VerifyDecoded);
            require(scan.hasValue() && scan.value().variables[0].status == "READ_OK", "Dictionary snapshot");
            require(decoded == 1, "Dictionary deliberately verifies decode");
        } else {
            const auto value = mode == 1 ? reader.readNamed(source, "alpha") :
                reader.readFirstCompatible(dir / "中文 输入");
            require(value.hasValue(), "Numeric snapshot read");
            require(value.value().values_column_major == std::vector<double>({1,2,3,4,5,6}), "Decode frozen A, not changed B");
            require(value.value().source_mat_sha256 == expected, "Value identity matches observer");
        }
        require(observer.events.size() == 1, "One prepared snapshot event");
        contains(observer.events[0].second, expected); contains(observer.events[0].second, source.string());
        const char* purposes[] = {"metadata", "named", "first_compatible", "dictionary_decode"};
        contains(observer.events[0].second, purposes[mode]);
        require(hash(source) == hash(root / "B.mat"), "Actual path changed after snapshot");
    }
    for (const bool inspect : {false, true}) {
        Capture observer; observer.fail_event = "mat_snapshot_prepared";
        io::MatResultReader reader(&observer); opened = decoded = metadata = 0;
        if (inspect) require(!reader.inspect(root / "A.mat").hasValue(), "Rejected metadata observer");
        else require(!reader.readNamed(root / "A.mat", "alpha").hasValue(), "Rejected numeric observer");
        require(opened == 0 && decoded == 0 && metadata == 0, "No matio consumption after journal failure");
        require(observer.failure() && observer.failure()->detail.stage == ErrorStage::Output, "Keep journal failure");
    }
    Capture missing; io::MatResultReader reader(&missing);
    require(!reader.inspect(root / "missing.mat").hasValue() && missing.events.empty(), "No fabricated missing-file hash");
}
class Runner final : public IExperimentRunner {
public:
    unsigned calls = 0;
    Result<RunOutcome, ExperimentErrorCode> fail(RunContext&) {
        ++calls; ErrorDetail d; d.stage = ErrorStage::Simulation; d.message = "Fixture runner failure";
        return Result<RunOutcome, ExperimentErrorCode>::failure({ExperimentErrorCode::SolverFailed, std::move(d)});
    }
    Result<RunOutcome, ExperimentErrorCode> runBiasOccluded(const BiasExperimentConfig&, RunContext& c) override { return fail(c); }
    Result<RunOutcome, ExperimentErrorCode> runBiasProportional(const BiasExperimentConfig&, RunContext& c) override { return fail(c); }
    Result<RunOutcome, ExperimentErrorCode> runAdaptiveInitialization(const AdaptiveInitializationConfig&, RunContext& c) override { return fail(c); }
    Result<RunOutcome, ExperimentErrorCode> runAdaptiveComparison(const AdaptiveComparisonConfig&, RunContext& c) override { return fail(c); }
    Result<RunOutcome, ExperimentErrorCode> runAdaptiveUpdateRates(const AdaptiveUpdateRatesConfig&, RunContext& c) override { return fail(c); }
};
class Exporter final : public illustrations::IExporter {
public:
    unsigned calls = 0;
    Result<std::vector<std::string>, PlotErrorCode> exportSvg(const illustrations::Data&, ITextPublisher&) override {
        ++calls;
        return Result<std::vector<std::string>, PlotErrorCode>::success(std::vector<std::string>{});
    }
};
void artifactCases(const fs::path& root) {
    for (const auto* event : {"illustration_ready", "run_id_reserved", "artifact_context"}) {
        const auto dir = root / event; fs::create_directories(dir / "origin");
        Capture observer; observer.fail_event = event; Exporter exporter;
        cli::ApplicationCompositionRoot app(nullptr, nullptr, nullptr, nullptr, dir, &exporter, &observer);
        std::ostringstream out, err;
        require(cli::run({"illustrate", "--kind", "circle", "--format", "svg", "--run-id", "actual"}, out, err, app) == 6,
                "Artifact audit failure uses Output exit");
        require(exporter.calls == 0 && out.str().empty(), "Unconfirmed context cannot start exporter");
        if (observer.events.size() >= 3) contains(err.str(), "\"run_id\":\"actual\"");
        require(!fs::exists(dir / "output/actual/invocation.json"), "Do not publish failed invocation");
    }
}
void applicationCases(const fs::path& root) {
    unsigned index = 0;
    for (const auto* event : {"command_accepted", "seed_selected", "run_id_reserved", "run_context", ""}) {
        const auto dir = root / ("app-" + std::to_string(index++)); fs::create_directories(dir / "origin");
        Capture observer; observer.fail_event = event; Runner runner;
        cli::ApplicationCompositionRoot app(nullptr, &runner, nullptr, nullptr, dir, nullptr, &observer);
        std::ostringstream out, err;
        const bool fail = *event != '\0';
        const int code = cli::run({"run-bias-occluded", "--seed", "7", "--level", "2", "--run-id", "actual"}, out, err, app);
        require(code == (fail ? 6 : 5) && out.str().empty(), "Observation failure must become Output exit 6");
        require(runner.calls == (fail ? 0U : 1U), "No runner after failed audit");
        require(observer.events.size() == (fail ? index : 4), "Ordered fail-stop events");
        const std::vector<std::string> names = {"command_accepted", "seed_selected", "run_id_reserved", "run_context"};
        for (std::size_t n = 0; n < observer.events.size(); ++n) require(observer.events[n].first == names[n], "Actual event order");
        if (index >= 3) contains(observer.events[2].second, "\"run_id\":\"actual\"");
        if (!fail) require(observer.events.back().second == read(dir / "output/actual/invocation.json"), "Same serialized actual RunContext");
    }
    Capture observer; observer.fail_event = "mat_snapshot_prepared";
    io::MatResultReader reader(&observer);
    cli::ApplicationCompositionRoot app(&reader, nullptr, nullptr, nullptr, {}, nullptr, &observer);
    std::ostringstream out, err;
    require(cli::run({"inspect-mat", "--file", (root / "A.mat").string()}, out, err, app) == 6 && out.str().empty(),
            "Inspect audit failure is Output, not a fabricated MAT decode failure");
}
} // namespace

extern "C" ssize_t __real_write(int, const void*, size_t);
extern "C" int __real_fsync(int);
extern "C" mat_t* __real_Mat_Open(const char*, int);
extern "C" matvar_t* __real_Mat_VarRead(mat_t*, const char*);
extern "C" matvar_t* __real_Mat_VarReadInfo(mat_t*, const char*);
extern "C" ssize_t __wrap_write(int fd, const void* data, size_t count) {
    if (fd == journal_fd && hits == 0 && fault != Fault::None && fault != Fault::Sync) {
        ++hits;
        if (fault == Fault::Short) return __real_write(fd, data, 1);
        if (fault == Fault::Zero) return 0;
        errno = fault == Fault::Interrupted ? EINTR : EIO; return -1;
    }
    return __real_write(fd, data, count);
}
extern "C" int __wrap_fsync(int fd) {
    if (fd == journal_fd && fault == Fault::Sync && hits == 0) { ++hits; errno = EIO; return -1; }
    return __real_fsync(fd);
}
extern "C" mat_t* __wrap_Mat_Open(const char* path, int mode) { ++opened; return __real_Mat_Open(path, mode); }
extern "C" matvar_t* __wrap_Mat_VarRead(mat_t* mat, const char* name) { ++decoded; return __real_Mat_VarRead(mat, name); }
extern "C" matvar_t* __wrap_Mat_VarReadInfo(mat_t* mat, const char* name) { ++metadata; return __real_Mat_VarReadInfo(mat, name); }
int main(int argc, char** argv) {
    try {
        require(argc == 2, "Sandbox argument"); fs::create_directories(argv[1]);
        std::string pattern = (fs::path(argv[1]) / "context-XXXXXX").string();
        require(::mkdtemp(pattern.data()) != nullptr, "Exclusive sandbox");
        const fs::path root = fs::absolute(pattern);
        std::cout << "Context journal sandbox=" << root << '\n';
        journalCases(root); snapshots(root); applicationCases(root); artifactCases(root);
        std::cout << "Context journal checks=" << checks << " sandbox=" << root << '\n'; return 0;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
