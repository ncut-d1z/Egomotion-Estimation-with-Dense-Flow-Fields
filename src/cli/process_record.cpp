#include "process_record.hpp"
#include "process_record_storage.hpp"
#include "execution_record.hpp"
#include "context_journal.hpp"
#include "stream_output.hpp"
#include "egomotion/io/sha256.hpp"

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <fcntl.h>
#include <poll.h>
#include <stdexcept>
#include <sys/signalfd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

namespace egomotion::cli {
namespace {
using process_record::checked;
using process_record::Directory;
using process_record::Fd;
constexpr int record_failure_exit = 6;

class Signals {
public:
    Signals() {
        ::sigemptyset(&watched_);
        for (const int signal : {SIGINT, SIGTERM, SIGHUP, SIGQUIT}) ::sigaddset(&watched_, signal);
        struct sigaction ignore{};
        ignore.sa_handler = SIG_IGN;
        ::sigemptyset(&ignore.sa_mask);
        checked(::sigaction(SIGPIPE, &ignore, &old_pipe_) == 0, "Cannot protect recorder receipt");
        pipe_changed_ = true;
        if (::sigprocmask(SIG_BLOCK, &watched_, &old_mask_) != 0) {
            restore();
            checked(false, "Cannot block supervisor signals");
        }
        mask_changed_ = true;
    }
    ~Signals() { restore(); }
    Signals(const Signals&) = delete;
    Signals& operator=(const Signals&) = delete;
    Fd descriptor() const {
        Fd fd(::signalfd(-1, &watched_, SFD_CLOEXEC | SFD_NONBLOCK));
        checked(fd.get() >= 0, "Cannot observe supervisor signals");
        return fd;
    }
    bool restoreChild() const noexcept {
        return ::sigaction(SIGPIPE, &old_pipe_, nullptr) == 0 &&
               ::sigprocmask(SIG_SETMASK, &old_mask_, nullptr) == 0;
    }
private:
    void restore() noexcept {
        if (pipe_changed_) { (void)::sigaction(SIGPIPE, &old_pipe_, nullptr); pipe_changed_ = false; }
        if (mask_changed_) { (void)::sigprocmask(SIG_SETMASK, &old_mask_, nullptr); mask_changed_ = false; }
    }
    sigset_t watched_{}, old_mask_{};
    struct sigaction old_pipe_{};
    bool pipe_changed_ = false, mask_changed_ = false;
};

class Child {
public:
    explicit Child(pid_t pid) noexcept : pid_(pid) {}
    ~Child() {
        if (pid_ <= 0) return;
        // 仅在记录器自身失败时回收本次仍未wait的子进程，不能留下后台实验。
        (void)::kill(pid_, SIGKILL);
        while (::waitpid(pid_, nullptr, 0) < 0 && errno == EINTR) {}
    }
    void reaped() noexcept { pid_ = -1; }
private:
    pid_t pid_;
};

std::string startRecord(const Directory& directory, int argc, char** argv) {
    const auto cwd = std::filesystem::current_path();
    std::error_code error;
    const auto executable = std::filesystem::read_symlink("/proc/self/exe", error);
    std::string json = "{\"schema\":\"egomotion-process-start-v2\",\"phase\":\"started\"";
    json += ",\"record_id\":" + jsonQuote(directory.id());
    json += ",\"started_utc\":" + processTimestamp();
    json += ",\"repo_sha\":" + jsonQuote(EGOMOTION_BUILD_SOURCE_SHA);
    json += ",\"working_directory\":" + jsonQuote(cwd.string());
    json += ",\"executable\":" + (error ? "null" : jsonQuote(executable.string()));
    json += ",\"execution_mode\":\"fork_same_build_application\",\"arguments_exclude_argv0\":true,\"arguments\":[";
    for (int index = 1; index < argc; ++index) {
        if (index > 1) json += ',';
        json += jsonQuote(argv[index]);
    }
    json += "],\"environment\":" + processEnvironment();
    // RecordId不是实验RunId；解析和预留尚未执行，不推断配置/seed/输入身份。
    json += ",\"application_context\":{\"file\":\"context.jsonl\",\"schema\":\"egomotion-context-event-v1\"}";
    json += ",\"record_directory\":" + jsonQuote(directory.path().string());
    return json + ",\"stdout\":\"stdout.bin\",\"stderr\":\"stderr.bin\",\"finish_record\":\"finish.json\"}\n";
}

int waitForChild(pid_t pid, int signal_fd, std::string& forwarded) {
    for (;;) {
        int status = 0;
        const auto result = ::waitpid(pid, &status, WNOHANG);
        if (result == pid) return status;
        if (result < 0 && errno == EINTR) continue;
        checked(result >= 0, "Cannot observe child termination");
        // 尚未回收的子进程（包括zombie）不会被复用为无关PID。
        struct pollfd event{signal_fd, POLLIN, 0};
        constexpr int exit_observation_interval_ms = 100;
        const int ready = ::poll(&event, 1, exit_observation_interval_ms);
        if (ready < 0 && errno == EINTR) continue;
        checked(ready >= 0 && !(event.revents & (POLLERR | POLLHUP | POLLNVAL)), "Cannot poll supervisor signals");
        if (!(event.revents & POLLIN)) continue;
        struct signalfd_siginfo signal{};
        const auto count = ::read(signal_fd, &signal, sizeof(signal));
        if (count < 0 && (errno == EAGAIN || errno == EINTR)) continue;
        checked(count == static_cast<ssize_t>(sizeof(signal)), "Cannot read supervisor signal");
        const int number = static_cast<int>(signal.ssi_signo);
        const int sent = ::kill(pid, number);
        const int send_errno = sent == 0 ? 0 : errno;
        if (!forwarded.empty()) forwarded += ',';
        forwarded += "{\"signal\":" + std::to_string(number) + ",\"kill_errno\":" + std::to_string(send_errno) + '}';
    }
}

std::string channelRecord(int fd, const char* name, bool& confirmed) {
    const bool synced = ::fsync(fd) == 0;
    struct stat identity{};
    const bool measured = ::fstat(fd, &identity) == 0 && identity.st_size >= 0;
    auto hash = io::sha256Fd(fd);
    confirmed = confirmed && synced && measured && hash.hasValue();
    return "{\"file\":" + jsonQuote(name) + ",\"fsync_succeeded\":" + (synced ? "true" : "false") +
        ",\"bytes\":" + (measured ? std::to_string(identity.st_size) : "null") +
        ",\"sha256\":" + (hash.hasValue() ? jsonQuote(hash.value()) : "null") + '}';
}

struct Completion { int exit_code; bool audit_confirmed; std::string json; };
Completion finishRecord(const Directory& directory, int status, int out, int err, int journal,
                        bool context_return_confirmed, const std::string& forwarded) {
    bool confirmed = true;
    auto channels = '[' + channelRecord(out, "stdout.bin", confirmed);
    channels += ',' + channelRecord(err, "stderr.bin", confirmed);
    channels += ',' + channelRecord(journal, "context.jsonl", confirmed) + ']';
    const bool exited = WIFEXITED(status);
    const int child_code = exited ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
    std::string json = "{\"schema\":\"egomotion-process-finish-v2\",\"record_id\":" + jsonQuote(directory.id());
    json += ",\"finished_utc\":" + processTimestamp() + ",\"wait_status\":" + std::to_string(status);
    json += ",\"termination\":" + jsonQuote(exited ? "exited" : "signaled");
    json += ",\"child_exit_code\":" + (exited ? std::to_string(WEXITSTATUS(status)) : "null");
    json += ",\"signal\":" + (exited ? "null" : std::to_string(WTERMSIG(status)));
    json += ",\"core_dump_flag\":" + std::string(WIFSIGNALED(status) && WCOREDUMP(status) ? "true" : "false");
    json += ",\"forwarded_signals\":[" + forwarded + "],\"channels\":" + channels;
    json += ",\"channels_confirmed\":" + std::string(confirmed ? "true" : "false");
    json += ",\"context_return_confirmed\":" + std::string(context_return_confirmed ? "true" : "false");
    const bool audit_confirmed = confirmed && (!exited || context_return_confirmed);
    const int code = audit_confirmed ? child_code : record_failure_exit;
    json += ",\"exit_code_if_record_and_receipt_confirmed\":" + std::to_string(code);
    json += ",\"exit_code_on_record_or_receipt_failure\":6}\n";
    return {code, audit_confirmed, std::move(json)};
}

int invokeObserved(int argc, char** argv, RecordedApplication application, ContextJournal& journal, int receipt) {
    auto entering = journal.record("application_entering", "{}");
    if (!entering.hasValue()) return streams::report(std::cerr, entering.error(), record_failure_exit);
    const int code = application(argc, argv, &journal);
    if (journal.failure()) return record_failure_exit;
    auto returned = journal.record("application_returned", "{\"application_exit_code\":" + std::to_string(code) + '}');
    if (!returned.hasValue()) return streams::report(std::cerr, returned.error(), record_failure_exit);
    constexpr char confirmed = 1;
    ssize_t count;
    do { count = ::write(receipt, &confirmed, 1); } while (count < 0 && errno == EINTR);
    return count == 1 ? code : streams::outputFailure(std::cerr, "Cannot confirm application context return");
}
bool contextReturnConfirmed(int receipt) {
    char confirmed = 0;
    ssize_t count;
    do { count = ::read(receipt, &confirmed, 1); } while (count < 0 && errno == EINTR);
    return count == 1 && confirmed == 1;
}
[[noreturn]] void executeChild(int argc, char** argv, RecordedApplication application,
                               const Signals& signals, int out, int err, ContextJournal& journal, int receipt) noexcept {
    if (::dup2(out, STDOUT_FILENO) < 0 || ::dup2(err, STDERR_FILENO) < 0 || !signals.restoreChild()) {
        constexpr char failed[] = "Child stream/signal setup failed before application entry\n";
        (void)::write(err, failed, sizeof(failed) - 1);
        ::_exit(record_failure_exit);
    }
    const int code = invokeObserved(argc, argv, application, journal, receipt);
    std::cout.flush();
    std::cerr.flush();
    const bool flushed = ::fflush(nullptr) == 0 && std::cout.good() && std::cerr.good();
    ::_exit(flushed ? code : record_failure_exit);
}
int recordWithSignals(int argc, char** argv, RecordedApplication application,
                      const std::filesystem::path& trusted_root, const Signals& signals,
                      std::ostream& receipt, std::ostream& errors) {
    std::string location;
    try {
        const auto signal_fd = signals.descriptor();
        const Directory directory(trusted_root);
        location = directory.path().string();
        const auto out = directory.newFile("stdout.bin");
        const auto err = directory.newFile("stderr.bin");
        const auto context = directory.newFile("context.jsonl");
        ContextJournal journal(context.get(), directory.id(), directory.path()/"context.jsonl");
        int receipt_pipe[2]{};
        checked(::pipe2(receipt_pipe, O_CLOEXEC | O_NONBLOCK) == 0, "Cannot create context completion channel");
        Fd receipt_read(receipt_pipe[0]), receipt_write(receipt_pipe[1]);
        checked(::fflush(nullptr) == 0, "Cannot flush pre-fork stdio");
        std::cout.flush(); std::cerr.flush();
        checked(std::cout.good() && std::cerr.good(), "Cannot flush pre-fork streams");
        directory.publish("start.json", startRecord(directory, argc, argv));
        const pid_t pid = ::fork();
        checked(pid >= 0, "Cannot fork recorded application");
        if (pid == 0) {
            receipt_read.reset();
            executeChild(argc, argv, application, signals, out.get(), err.get(), journal, receipt_write.get());
        }
        receipt_write.reset();
        Child child(pid);
        std::string forwarded;
        const int status = waitForChild(pid, signal_fd.get(), forwarded);
        child.reaped();
        const auto completion = finishRecord(directory, status, out.get(), err.get(), context.get(),
            contextReturnConfirmed(receipt_read.get()), forwarded);
        directory.publish("finish.json", completion.json);
        const auto response = "{\"record_id\":" + jsonQuote(directory.id()) +
            ",\"record_directory\":" + jsonQuote(location) +
            ",\"exit_code_if_receipt_delivered\":" + std::to_string(completion.exit_code) + "}\n";
        if (!streams::writeAndFlush(receipt, response))
            return streams::outputFailure(errors, "Process receipt delivery failed; retained record: " + location);
        if (!completion.audit_confirmed)
            return streams::outputFailure(errors, "Cannot confirm child logs or context return; retained record: " + location);
        return completion.exit_code;
    } catch (const std::exception& error) {
        std::string message = error.what();
        if (!location.empty()) message += "; retained record: " + location;
        return streams::outputFailure(errors, message);
    }
}
}  // namespace

int recordProcess(int argc, char** argv, RecordedApplication application,
                  const std::filesystem::path& trusted_root,
                  std::ostream& receipt, std::ostream& errors) {
    try {
        Signals signals;
        // 自身故障的诊断也属于父进程保护范围；异常展开不能提前恢复SIGPIPE。
        return recordWithSignals(argc, argv, application, trusted_root, signals, receipt, errors);
    } catch (const std::exception& error) {
        return streams::outputFailure(errors, error.what());
    }
}
}  // namespace egomotion::cli
