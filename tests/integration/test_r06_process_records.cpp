#include "process_record.hpp"

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace {
enum class Fault { None, StartLink, StartSync, FinishLink, FinishSync, ChannelSync, Fork, Random, Collision };
Fault fault = Fault::None;
int failed_sync_fd = -1;
bool forked = false;
std::size_t checks = 0, calls = 0, hits = 0;
}
extern "C" int __real_linkat(int, const char*, int, const char*, int);
extern "C" int __real_fsync(int);
extern "C" pid_t __real_fork();
extern "C" ssize_t __real_getrandom(void*, std::size_t, unsigned int);
extern "C" int __wrap_linkat(int from, const char* source, int to, const char* name, int flags) {
    const bool start = std::strcmp(name, "start.json") == 0;
    const bool finish = std::strcmp(name, "finish.json") == 0;
    if ((start && fault == Fault::StartLink) || (finish && fault == Fault::FinishLink)) {
        ++hits; errno = EIO; return -1;
    }
    const int result = __real_linkat(from, source, to, name, flags);
    if (result == 0 && ((start && fault == Fault::StartSync) || (finish && fault == Fault::FinishSync)))
        failed_sync_fd = to;
    return result;
}
extern "C" int __wrap_fsync(int fd) {
    if (fd == failed_sync_fd || (forked && fault == Fault::ChannelSync && hits == 0)) {
        failed_sync_fd = -1; ++hits; errno = EIO; return -1;
    }
    return __real_fsync(fd);
}
extern "C" pid_t __wrap_fork() {
    if (fault == Fault::Fork) { ++hits; errno = EAGAIN; return -1; }
    const auto pid = __real_fork();
    forked = pid > 0;
    return pid;
}
extern "C" ssize_t __wrap_getrandom(void* bytes, std::size_t count, unsigned int flags) {
    if (fault == Fault::Random) { ++hits; errno = EIO; return -1; }
    if (fault == Fault::Collision) { ++hits; std::memset(bytes, 0, count); return static_cast<ssize_t>(count); }
    return __real_getrandom(bytes, count, flags);
}

namespace {
namespace fs = std::filesystem;
void require(bool okay, const char* reason) {
    ++checks;
    if (!okay) throw std::runtime_error(reason);
}
std::string read(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    require(file.good(), "Expected evidence file missing");
    return {std::istreambuf_iterator<char>(file), {}};
}
void contains(const std::string& text, const std::string& wanted) {
    require(text.find(wanted) != std::string::npos, "Required record field missing");
}
int fixture(int argc, char** argv) {
    if (argc != 2) return 91;
    std::ofstream("entered") << "same-build fixture\n";
    std::cout.write("ok\0tail\n", 8);
    std::cerr << "{\"fixture\":\"中文\",\"causes\":[{\"message\":\"inner\"}]}\n";
    std::cout.flush(); std::cerr.flush();
    const std::string mode = argv[1];
    if (mode == "kill") ::raise(SIGKILL);
    if (mode == "pipe") ::raise(SIGPIPE);
    if (mode == "forward") {
        if (::kill(::getppid(), SIGTERM) != 0) return 92;
        for (;;) ::pause();
    }
    return mode == "137" ? 137 : mode == "6" ? 6 : 0;
}
struct Response { int code; std::string receipt, errors; };
Response invoke(const fs::path& root, const char* mode = "ok", bool bad_receipt = false, bool bad_errors = false) {
    ++calls;
    fs::current_path(root);
    char program[] = "test-recorder";
    std::string argument = mode;
    char* args[] = {program, argument.data(), nullptr};
    std::ostringstream receipt, errors;
    if (bad_receipt) receipt.setstate(std::ios::badbit);
    if (bad_errors) errors.setstate(std::ios::badbit);
    hits = 0; forked = false; failed_sync_fd = -1;
    sigset_t before{}, after{};
    struct sigaction pipe_before{}, pipe_after{};
    require(::sigprocmask(SIG_SETMASK, nullptr, &before) == 0, "Read initial mask");
    require(::sigaction(SIGPIPE, nullptr, &pipe_before) == 0, "Read initial pipe action");
    const int code = egomotion::cli::recordProcess(2, args, fixture, root, receipt, errors);
    std::ofstream(root / "fixture-response.txt") << "exit=" << code << "\nhits=" << hits
        << "\nreceipt=" << receipt.str() << "\nerrors=" << errors.str();
    require(::sigprocmask(SIG_SETMASK, nullptr, &after) == 0, "Read restored mask");
    require(::sigaction(SIGPIPE, nullptr, &pipe_after) == 0, "Read restored pipe action");
    require(pipe_before.sa_handler == pipe_after.sa_handler, "Do not change caller SIGPIPE disposition");
    for (const int signal : {SIGINT, SIGTERM, SIGQUIT, SIGHUP, SIGPIPE})
        require(::sigismember(&before, signal) == ::sigismember(&after, signal), "Restore caller signal mask");
    return {code, receipt.str(), errors.str()};
}
fs::path newRoot(const fs::path& base, const std::string& name) {
    const auto root = base / name;
    fs::create_directories(root / "origin");
    return root;
}
fs::path onlyRecord(const fs::path& root) {
    std::vector<fs::path> records;
    for (const auto& entry : fs::directory_iterator(root / "build/cli-records")) records.push_back(entry.path());
    require(records.size() == 1, "Exactly one exclusive process record");
    return records.front();
}
void termination(const fs::path& base) {
    for (const auto* mode : {"ok", "137", "6", "kill", "pipe", "forward"}) {
        const auto root = newRoot(base, mode);
        const auto result = invoke(root, mode);
        const int signal = std::strcmp(mode, "kill") == 0 ? SIGKILL : std::strcmp(mode, "pipe") == 0 ? SIGPIPE :
                           std::strcmp(mode, "forward") == 0 ? SIGTERM : 0;
        const int expected = signal ? 128 + signal : std::atoi(mode);
        require(result.code == expected && result.errors.empty(), "Actual termination code preserved");
        const auto directory = onlyRecord(root);
        const auto start = read(directory / "start.json");
        const auto finish = read(directory / "finish.json");
        contains(start, "\"application_context\":\"not_observed_by_supervisor\"");
        contains(start, "\"arguments\":[\"" + std::string(mode) + "\"]");
        contains(finish, signal ? "\"termination\":\"signaled\"" : "\"termination\":\"exited\"");
        contains(finish, "\"child_exit_code\":" + (signal ? std::string("null") : std::to_string(expected)));
        contains(finish, "\"signal\":" + (signal ? std::to_string(signal) : std::string("null")));
        contains(finish, "\"channels_confirmed\":true");
        if (std::strcmp(mode, "forward") == 0) contains(finish, "\"forwarded_signals\":[{\"signal\":15,\"kill_errno\":0}]");
        require(read(directory / "stdout.bin") == std::string("ok\0tail\n", 8), "Raw stdout including NUL retained");
        contains(read(directory / "stderr.bin"), "\"causes\":[{\"message\":\"inner\"}]");
        require(!fs::exists(root / "output"), "Process audit must not create an experiment output root");
        struct stat info{};
        require(::stat(directory.c_str(), &info) == 0 && (info.st_mode & 0777) == 0700, "Private record directory");
        require(::stat((directory / "stdout.bin").c_str(), &info) == 0 && (info.st_mode & 0777) == 0600, "Private channel log");
        require(!fs::exists(directory / "start.json.pending"), "Successful temporary link cleaned");
    }
}
void publicationFaults(const fs::path& base) {
    for (const auto kind : {Fault::StartLink, Fault::StartSync, Fault::FinishLink, Fault::FinishSync,
                            Fault::ChannelSync, Fault::Fork, Fault::Random}) {
        const auto root = newRoot(base, "fault_" + std::to_string(static_cast<int>(kind)));
        fault = kind;
        const auto result = invoke(root);
        fault = Fault::None;
        require(hits == 1 && result.code == 6 && !result.errors.empty(), "Injected recorder failure surfaced exactly once");
        const bool executed = kind == Fault::FinishLink || kind == Fault::FinishSync || kind == Fault::ChannelSync;
        require(fs::exists(root / "entered") == executed, "No child before confirmed start and successful fork");
        if (kind == Fault::Random) continue;
        const auto directory = onlyRecord(root);
        if (executed) require(read(directory / "stdout.bin") == std::string("ok\0tail\n", 8), "Keep child output despite recorder failure");
        if (kind == Fault::FinishSync || kind == Fault::ChannelSync) {
            const auto finish = read(directory / "finish.json");
            contains(finish, "\"child_exit_code\":0");
            contains(finish, "\"exit_code_on_record_or_receipt_failure\":6");
            if (kind == Fault::ChannelSync) contains(finish, "\"channels_confirmed\":false");
        }
        if (kind == Fault::StartLink || kind == Fault::StartSync)
            require(fs::exists(directory / "start.json.pending"), "Keep unconfirmed start evidence");
        if (kind == Fault::FinishLink || kind == Fault::FinishSync)
            require(fs::exists(directory / "finish.json.pending"), "Keep unconfirmed finish evidence");
        if (kind == Fault::Fork) require(!fs::exists(directory / "finish.json"), "No invented child termination after fork failure");
    }
    for (const bool bad_errors : {false, true}) {
        const auto root = newRoot(base, bad_errors ? "both_channels_bad" : "receipt_bad");
        const auto result = invoke(root, "ok", true, bad_errors);
        require(result.code == 6 && result.receipt.empty(), "Receipt failure remains failure even if diagnostic also fails");
        contains(read(onlyRecord(root) / "finish.json"), "\"child_exit_code\":0");
    }
}
void paths(const fs::path& base) {
    const auto outside = newRoot(base, "outside");
    for (const auto* component : {"build", "build/cli-records"}) {
        const auto root = newRoot(base, std::strcmp(component, "build") == 0 ? "build_link" : "logs_link");
        fs::create_directories((root / component).parent_path());
        if (std::strcmp(component, "build") != 0)
            fs::permissions(root / "build", fs::perms::owner_all);
        fs::create_directory_symlink(outside, root / component);
        require(invoke(root).code == 6 && !fs::exists(root / "entered"), "Reject log directory symlink before child");
        require(!fs::exists(outside / "cli-records"), "Do not write through symlink");
    }
    const auto collision = newRoot(base, "collision");
    const auto occupied = collision / "build/cli-records/record_00000000000000000000000000000000";
    fs::create_directories(occupied);
    fs::permissions(collision / "build", fs::perms::owner_all);
    fs::permissions(collision / "build/cli-records", fs::perms::owner_all);
    std::ofstream(occupied / "retained") << "never replace";
    fault = Fault::Collision;
    const auto response = invoke(collision);
    fault = Fault::None;
    require(response.code == 6 && hits == 4, "Bounded independent identity retries");
    require(read(occupied / "retained") == "never replace", "Collision preserves existing evidence");
    require(!fs::exists(collision / "entered"), "No child after identity failure");
    const auto shared = newRoot(base, "shared_logs");
    fs::create_directories(shared / "build/cli-records");
    fs::permissions(shared / "build", fs::perms::owner_all);
    fs::permissions(shared / "build/cli-records", fs::perms::owner_all | fs::perms::group_all);
    require(invoke(shared).code == 6, "Reject shared log directory");
    const auto unsafe_build = newRoot(base, "shared_build");
    fs::create_directories(unsafe_build / "build");
    fs::permissions(unsafe_build / "build", fs::perms::owner_all | fs::perms::group_all);
    require(invoke(unsafe_build).code == 6 && !fs::exists(unsafe_build / "entered"),
            "Keep unsafe build ancestor rejection independently of final directory");
}
}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 2) throw std::runtime_error("Expected sandbox base");
        fs::create_directories(argv[1]);
        std::string pattern = fs::absolute(fs::path(argv[1]) / "运行 记录-XXXXXX").string();
        require(::mkdtemp(pattern.data()) != nullptr, "Unique retained sandbox");
        const fs::path base(pattern);
        termination(base);
        publicationFaults(base);
        paths(base);
        std::cout << "Process records: " << checks << " checks, " << calls << " supervised calls; retained " << base << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Process record test failed: " << error.what() << '\n';
        return 1;
    }
}
