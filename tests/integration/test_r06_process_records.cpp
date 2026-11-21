#include "process_record.hpp"
#include "application_main.hpp"
#include "egomotion/cli/invocation_observer.hpp"
#include "egomotion/contracts/diagnostics.hpp"

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {
enum class Fault { None, StartLink, StartSync, FinishLink, FinishSync, ChannelSync, Fork, Random,
                   Collision, ContextSync, ReservationSync };
Fault fault = Fault::None;
int failed_sync_fd = -1;
bool forked = false;
bool child_signal_blocked_before_fork = false;
bool expected_child_signal_blocked = false;
volatile sig_atomic_t handler_reaped_child = 0;
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
    if (fault == Fault::ReservationSync) {
        struct stat logs{}, target{};
        if (::stat("build-cli-records", &logs) == 0 && ::fstat(fd, &target) == 0 &&
            logs.st_dev == target.st_dev && logs.st_ino == target.st_ino) {
            ++hits; errno = EIO; return -1;
        }
    }
    if (fault == Fault::ContextSync) { ++hits; errno = EIO; return -1; }
    if (fd == failed_sync_fd || (forked && fault == Fault::ChannelSync && hits == 0)) {
        failed_sync_fd = -1; ++hits; errno = EIO; return -1;
    }
    return __real_fsync(fd);
}
extern "C" pid_t __wrap_fork() {
    if (fault == Fault::Fork) { ++hits; errno = EAGAIN; return -1; }
    sigset_t mask{};
    child_signal_blocked_before_fork = ::sigprocmask(SIG_SETMASK, nullptr, &mask) == 0 &&
        ::sigismember(&mask, SIGCHLD) == 1;
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
int fixture(int argc, char** argv, egomotion::cli::IInvocationObserver* observer) {
    if (argc != 2) return 91;
    std::ofstream("entered") << "same-build fixture\n";
    std::cout.write("ok\0tail\n", 8);
    std::cerr << "{\"fixture\":\"中文\",\"causes\":[{\"message\":\"inner\"}]}\n";
    std::cout.flush(); std::cerr.flush();
    const std::string mode = argv[1];
    if (mode == "exit") ::_exit(0);
    if (mode == "unacknowledged") {
        if (!observer->record("application_returned", "{\"application_exit_code\":0}").hasValue()) return 93;
        ::_exit(0);
    }
    if (mode == "context-sync") fault = Fault::ContextSync;
    if (mode == "kill") ::raise(SIGKILL);
    if (mode == "pipe") ::raise(SIGPIPE);
    if (mode == "forward") {
        if (::kill(::getppid(), SIGTERM) != 0) return 92;
        for (;;) ::pause();
    }
    return mode == "137" ? 137 : mode == "6" ? 6 : 0;
}
struct Response { int code; std::string receipt, errors; };
Response invoke(const fs::path& root, const char* mode = "ok", bool bad_receipt = false, bool bad_errors = false,
                egomotion::cli::RecordedApplication application = fixture) {
    ++calls;
    fs::current_path(root);
    char program[] = "test-recorder";
    std::string argument = mode;
    char* args[] = {program, argument.data(), nullptr};
    std::ostringstream receipt, errors;
    if (bad_receipt) receipt.setstate(std::ios::badbit);
    if (bad_errors) errors.setstate(std::ios::badbit);
    hits = 0; forked = false; failed_sync_fd = -1; child_signal_blocked_before_fork = false;
    sigset_t before{}, after{};
    struct sigaction pipe_before{}, pipe_after{};
    struct sigaction child_before{}, child_after{};
    require(::sigprocmask(SIG_SETMASK, nullptr, &before) == 0, "Read initial mask");
    require(::sigaction(SIGPIPE, nullptr, &pipe_before) == 0, "Read initial pipe action");
    require(::sigaction(SIGCHLD, nullptr, &child_before) == 0, "Read initial child action");
    const int code = egomotion::cli::recordProcess(2, args, application, root, receipt, errors);
    std::ofstream(root / "fixture-response.txt") << "exit=" << code << "\nhits=" << hits
        << "\nreceipt=" << receipt.str() << "\nerrors=" << errors.str();
    require(::sigprocmask(SIG_SETMASK, nullptr, &after) == 0, "Read restored mask");
    require(::sigaction(SIGPIPE, nullptr, &pipe_after) == 0, "Read restored pipe action");
    require(::sigaction(SIGCHLD, nullptr, &child_after) == 0, "Read restored child action");
    require(pipe_before.sa_handler == pipe_after.sa_handler, "Do not change caller SIGPIPE disposition");
    require(child_before.sa_handler == child_after.sa_handler && child_before.sa_flags == child_after.sa_flags,
            "Do not change caller SIGCHLD disposition or flags");
    for (const int signal : {SIGINT, SIGTERM, SIGQUIT, SIGHUP, SIGPIPE, SIGCHLD})
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
    for (const auto& entry : fs::directory_iterator(root / "build-cli-records")) records.push_back(entry.path());
    require(records.size() == 1, "Exactly one exclusive process record");
    return records.front();
}
void reapChildren(int) {
    const int saved_errno = errno;
    for (;;) {
        const auto child = ::waitpid(-1, nullptr, WNOHANG);
        if (child > 0) { handler_reaped_child = 1; continue; }
        if (child < 0 && errno == EINTR) continue;
        break;
    }
    errno = saved_errno;
}
int observeChildSignal(int argc, char** argv, egomotion::cli::IInvocationObserver* observer) {
    sigset_t mask{};
    struct sigaction action{};
    if (::sigprocmask(SIG_SETMASK, nullptr, &mask) != 0 ||
        ::sigaction(SIGCHLD, nullptr, &action) != 0 || action.sa_handler != reapChildren ||
        ::sigismember(&mask, SIGCHLD) != static_cast<int>(expected_child_signal_blocked)) return 94;
    std::ofstream("child-signal-restored") << "original SIGCHLD action and mask\n";
    return fixture(argc, argv, observer);
}
void childSignalDispositions(const fs::path& base) {
    for (const bool ignore : {false, true}) {
        struct sigaction action{}, original{};
        action.sa_handler = ignore ? SIG_IGN : SIG_DFL;
        action.sa_flags = ignore ? 0 : SA_NOCLDWAIT;
        ::sigemptyset(&action.sa_mask);
        require(::sigaction(SIGCHLD, &action, &original) == 0, "Set inherited automatic child reaping");
        const auto root = newRoot(base, ignore ? "sigchld-ignored" : "sigchld-no-cld-wait");
        const auto result = invoke(root);
        require(::sigaction(SIGCHLD, &original, nullptr) == 0, "Restore original child action");
        require(result.code == 6 && result.receipt.empty(), "Reject unobservable child termination");
        contains(result.errors, "\"error_code\":\"OutputFailed\"");
        contains(result.errors, "SIGCHLD ignored or SA_NOCLDWAIT set");
        require(!forked && !fs::exists(root / "entered"), "Automatic reaping must be rejected before fork");
        require(!fs::exists(root / "build-cli-records"), "Rejected signal state has no started record");
    }
    for (const bool blocked : {false, true}) {
        struct sigaction action{}, original_action{};
        action.sa_handler = reapChildren;
        action.sa_flags = SA_RESTART;
        ::sigemptyset(&action.sa_mask);
        sigset_t original_mask{}, mask{};
        require(::sigprocmask(SIG_SETMASK, nullptr, &original_mask) == 0, "Save caller child signal mask");
        mask = original_mask;
        if (blocked) ::sigaddset(&mask, SIGCHLD); else ::sigdelset(&mask, SIGCHLD);
        require(::sigaction(SIGCHLD, &action, &original_action) == 0, "Set inherited child reaper");
        require(::sigprocmask(SIG_SETMASK, &mask, nullptr) == 0, "Set inherited child signal mask");
        expected_child_signal_blocked = blocked;
        for (const auto* mode : {"137", "forward"}) {
            handler_reaped_child = 0;
            const auto root = newRoot(base, std::string("sigchld-handler-") + (blocked ? "blocked-" : "unblocked-") + mode);
            const auto result = invoke(root, mode, false, false, observeChildSignal);
            const bool signaled = std::strcmp(mode, "forward") == 0;
            require(result.code == (signaled ? 128 + SIGTERM : 137) && result.errors.empty(),
                    "Inherited handler cannot steal actual exit or signal status");
            require(child_signal_blocked_before_fork && handler_reaped_child == 0,
                    "Only supervisor waitpid can reap its child");
            require(fs::exists(root / "child-signal-restored"), "Application sees original SIGCHLD action and mask");
            const auto finish = read(onlyRecord(root) / "finish.json");
            contains(finish, signaled ? "\"termination\":\"signaled\"" : "\"termination\":\"exited\"");
            contains(finish, signaled ? "\"child_exit_code\":null" : "\"child_exit_code\":137");
            contains(finish, signaled ? "\"signal\":15" : "\"signal\":null");
            contains(finish, signaled ? "\"forwarded_signals\":[{\"signal\":15,\"kill_errno\":0}]" : "\"forwarded_signals\":[]");
        }
        // 原mask阻塞SIGCHLD时，记录器按约定保留pending；由本夹具handler消化后再恢复测试入口状态。
        ::sigdelset(&mask, SIGCHLD);
        require(::sigprocmask(SIG_SETMASK, &mask, nullptr) == 0, "Deliver retained fixture child notification");
        sigset_t pending{};
        require(::sigpending(&pending) == 0 && ::sigismember(&pending, SIGCHLD) == 0,
                "No fixture child notification may affect subsequent regression cases");
        require(handler_reaped_child == 0, "Deferred handler cannot reap already observed child");
        require(::sigaction(SIGCHLD, &original_action, nullptr) == 0, "Restore original child handler");
        require(::sigprocmask(SIG_SETMASK, &original_mask, nullptr) == 0, "Restore original child signal mask");
    }
}
int missingWorkingDirectory(int argc, char** argv, egomotion::cli::IInvocationObserver* observer) {
    // 只移除本测试子进程新建的空目录；父进程和已打开的日志目录保持有效。
    require(::mkdir("unlinked-cwd", 0700) == 0, "Create disposable child working directory");
    require(::chdir("unlinked-cwd") == 0, "Enter disposable child working directory");
    require(::rmdir("../unlinked-cwd") == 0, "Unlink disposable child working directory");
    return egomotionApplication(argc, argv, observer);
}
void startupFailure(const fs::path& base) {
    const auto direct = newRoot(base, "startup-direct");
    fs::current_path(direct);
    const int out = ::open("stdout.bin", O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC, 0600);
    const int err = ::open("stderr.bin", O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC, 0600);
    require(out >= 0 && err >= 0, "Create retained direct-entry channels");
    std::cout.flush(); std::cerr.flush();
    const auto pid = __real_fork();
    require(pid >= 0, "Fork isolated application startup");
    if (pid == 0) {
        if (::dup2(out, STDOUT_FILENO) < 0 || ::dup2(err, STDERR_FILENO) < 0) ::_exit(91);
        ::close(out); ::close(err);
        char name[] = "egomotion", mode[] = "--help";
        char* args[] = {name, mode, nullptr};
        try { ::_exit(missingWorkingDirectory(2, args, nullptr)); }
        catch (...) { ::_exit(92); }
    }
    ++calls; ::close(out); ::close(err);
    int status = 0; pid_t observed;
    do { observed = ::waitpid(pid, &status, 0); } while (observed < 0 && errno == EINTR);
    require(observed == pid && WIFEXITED(status) && WEXITSTATUS(status) == 6,
            "Application startup exception must return 6 without escaping");
    const auto diagnostic = read(direct / "stderr.bin");
    contains(diagnostic, "\"error_domain\":\"ExperimentErrorCode\"");
    contains(diagnostic, "\"error_code\":\"OutputFailed\"");
    contains(diagnostic, "\"stage\":\"output\"");
    contains(diagnostic, "\"exit_code\":6");
    contains(diagnostic, "\"causes\":[]");
    require(diagnostic.find("\"run_id\"") == std::string::npos, "Startup failure has no allocated RunId");
    require(read(direct / "stdout.bin").empty(), "Startup failure cannot print successful help");
    require(!fs::exists(direct / "output"), "Startup failure cannot create scientific outputs");

    const auto supervised = newRoot(base, "startup-recorded");
    const auto response = invoke(supervised, "--help", false, false, missingWorkingDirectory);
    require(response.code == 6 && response.errors.empty(), "Recorder preserves actual startup failure exit");
    const auto directory = onlyRecord(supervised);
    const auto finish = read(directory / "finish.json");
    contains(finish, "\"termination\":\"exited\"");
    contains(finish, "\"child_exit_code\":6");
    contains(finish, "\"context_return_confirmed\":true");
    contains(read(directory / "context.jsonl"), "\"event\":\"application_returned\"");
    require(read(directory / "stderr.bin") == diagnostic, "Both entries retain the same startup diagnostic");
    require(read(directory / "stdout.bin").empty(), "Recorded startup failure cannot print successful help");
    require(!fs::exists(supervised / "output"), "Recorded startup failure has no scientific output");
}
void termination(const fs::path& base) {
    for (const auto* mode : {"ok", "137", "6", "kill", "pipe", "forward"}) {
        const auto root = newRoot(base, mode);
        fs::create_directories(root / "build");
        fs::permissions(root / "build", fs::perms::owner_all | fs::perms::group_all |
                                        fs::perms::others_read | fs::perms::others_exec);
        const auto result = invoke(root, mode);
        const int signal = std::strcmp(mode, "kill") == 0 ? SIGKILL : std::strcmp(mode, "pipe") == 0 ? SIGPIPE :
                           std::strcmp(mode, "forward") == 0 ? SIGTERM : 0;
        const int expected = signal ? 128 + signal : std::atoi(mode);
        require(result.code == expected && result.errors.empty(), "Actual termination code preserved");
        const auto directory = onlyRecord(root);
        const auto start = read(directory / "start.json");
        const auto finish = read(directory / "finish.json");
        contains(start, "\"application_context\":{\"file\":\"context.jsonl\"");
        contains(finish, "\"context_return_confirmed\":" + std::string(signal ? "false" : "true"));
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
        require(::stat((root / "build").c_str(), &info) == 0 && (info.st_mode & 0777) == 0775, "Existing shared build permissions unchanged");
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
void incompleteContexts(const fs::path& base) {
    for (const auto* mode : {"exit", "unacknowledged", "context-sync"}) {
        const auto root = newRoot(base, mode);
        const auto result = invoke(root, mode);
        require(result.code == 6 && !result.errors.empty(), "Missing confirmed return must fail process audit");
        const auto directory = onlyRecord(root);
        const auto finish = read(directory / "finish.json");
        contains(finish, "\"termination\":\"exited\"");
        contains(finish, "\"context_return_confirmed\":false");
        contains(finish, "\"channels_confirmed\":true");
        contains(finish, std::strcmp(mode, "context-sync") == 0 ? "\"child_exit_code\":6" : "\"child_exit_code\":0");
        const auto context = read(directory / "context.jsonl");
        contains(context, "\"event\":\"application_entering\"");
        if (std::strcmp(mode, "exit") != 0) contains(context, "\"event\":\"application_returned\"");
    }
}
void reservationFailure(const fs::path& base) {
    const auto root = newRoot(base, "reservation_sync");
    fault = Fault::ReservationSync;
    const auto response = invoke(root);
    fault = Fault::None;
    require(hits == 1 && response.code == 6, "Reserved directory sync failure surfaced exactly once");
    require(response.receipt.empty() && !fs::exists(root / "entered"),
            "No receipt or child before record directory confirmation");
    const auto directory = onlyRecord(root);
    require(fs::is_directory(directory), "Keep the directory created before failed confirmation");
    const auto message = "Cannot persist process record directory; errno=" + std::to_string(EIO) +
        "; retained record: " + directory.string();
    contains(response.errors, "\"message\":" + egomotion::jsonQuote(message));
    require(!fs::exists(directory / "start.json") && !fs::exists(directory / "finish.json"),
            "Directory reservation failure cannot fabricate process records");
}
void paths(const fs::path& base) {
    const auto outside = newRoot(base, "outside");
    for (const bool symlink : {true, false}) {
        const auto root = newRoot(base, symlink ? "logs_link" : "logs_file");
        if (symlink) fs::create_directory_symlink(outside, root / "build-cli-records");
        else std::ofstream(root / "build-cli-records") << "retained regular file";
        require(invoke(root).code == 6 && !fs::exists(root / "entered"), "Reject non-directory audit root before child");
        require(!fs::exists(outside / "build-cli-records"), "Do not write through symlink");
        if (!symlink) require(read(root / "build-cli-records") == "retained regular file", "Keep regular-file blocker");
    }
    const auto collision = newRoot(base, "collision");
    const auto occupied = collision / "build-cli-records/record_00000000000000000000000000000000";
    fs::create_directories(occupied);
    fs::permissions(collision / "build-cli-records", fs::perms::owner_all);
    std::ofstream(occupied / "retained") << "never replace";
    fault = Fault::Collision;
    const auto response = invoke(collision);
    fault = Fault::None;
    require(response.code == 6 && hits == 4, "Bounded independent identity retries");
    require(response.receipt.empty() && response.errors.find("retained record:") == std::string::npos,
            "Collision candidates cannot be claimed as a retained record from this invocation");
    require(read(occupied / "retained") == "never replace", "Collision preserves existing evidence");
    require(!fs::exists(collision / "entered"), "No child after identity failure");
    const auto shared = newRoot(base, "shared_logs");
    fs::create_directories(shared / "build-cli-records");
    fs::permissions(shared / "build-cli-records", fs::perms::owner_all | fs::perms::group_all);
    require(invoke(shared).code == 6, "Reject shared log directory");
    const auto public_logs = newRoot(base, "public_logs");
    fs::create_directories(public_logs / "build-cli-records");
    fs::permissions(public_logs / "build-cli-records", fs::perms::owner_all | fs::perms::others_read);
    require(invoke(public_logs).code == 6 && !fs::exists(public_logs / "entered"),
            "Reject public audit root even when others cannot write");
}
void brokenPipes(const fs::path& base) {
    for (const int phase : {0, 1, 2}) {
        const auto root = newRoot(base, "closed_pipe_" + std::to_string(phase));
        ++calls;
        int pipe_fds[2]{};
        require(::pipe(pipe_fds) == 0, "Create actual pipe");
        require(::close(pipe_fds[0]) == 0, "Remove every pipe reader before fork");
        const auto pid = __real_fork();
        require(pid >= 0, "Fork isolated supervisor pipe probe");
        if (pid == 0) {
            struct sigaction action{}; action.sa_handler = SIG_DFL; ::sigemptyset(&action.sa_mask);
            if (::sigaction(SIGPIPE, &action, nullptr) != 0 ||
                ::dup2(pipe_fds[1], phase == 2 ? STDOUT_FILENO : STDERR_FILENO) < 0) ::_exit(90);
            ::close(pipe_fds[1]);
            fs::current_path(root);
            fault = phase == 0 ? Fault::StartLink : phase == 1 ? Fault::FinishLink : Fault::None;
            char name[] = "pipe-probe", mode[] = "ok";
            char* args[] = {name, mode, nullptr};
            std::ostringstream receipt, errors;
            const int code = egomotion::cli::recordProcess(2, args, fixture, root,
                phase == 2 ? std::cout : receipt, phase == 2 ? errors : std::cerr);
            ::_exit(code);
        }
        ::close(pipe_fds[1]);
        int status = 0;
        pid_t observed;
        do { observed = ::waitpid(pid, &status, 0); } while (observed < 0 && errno == EINTR);
        std::ofstream(root / "supervisor-wait-status.txt") << status << '\n';
        require(observed == pid && WIFEXITED(status) && WEXITSTATUS(status) == 6,
                "Recorder diagnostic/receipt pipe failures return 6, not SIGPIPE");
        require(fs::exists(root / "entered") == (phase != 0), "Start gate remains effective with broken diagnostics");
        const auto directory = onlyRecord(root);
        require(fs::exists(directory / "finish.json") == (phase == 2), "Keep confirmed finish on broken receipt only");
        if (phase != 0) require(read(directory / "stdout.bin") == std::string("ok\0tail\n", 8), "Child log survives supervisor pipe failure");
    }
}
}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 2) throw std::runtime_error("Expected sandbox base");
        fs::create_directories(argv[1]);
        std::string pattern = fs::absolute(fs::path(argv[1]) / "运行 记录-XXXXXX").string();
        require(::mkdtemp(pattern.data()) != nullptr, "Unique retained sandbox");
        const fs::path base(pattern);
        startupFailure(base);
        termination(base);
        childSignalDispositions(base);
        publicationFaults(base);
        incompleteContexts(base);
        reservationFailure(base);
        paths(base);
        brokenPipes(base);
        std::cout << "Process records: " << checks << " checks, " << calls << " application/recorder calls; retained " << base << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Process record test failed: " << error.what() << '\n';
        return 1;
    }
}
