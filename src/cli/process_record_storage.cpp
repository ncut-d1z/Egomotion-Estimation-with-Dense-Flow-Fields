#include "process_record_storage.hpp"

#include <array>
#include <cerrno>
#include <fcntl.h>
#include <stdexcept>
#include <sys/random.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

namespace egomotion::cli::process_record {
Fd::~Fd() { reset(); }
void Fd::reset() noexcept { if (value_ >= 0) ::close(std::exchange(value_, -1)); }
Fd::Fd(Fd&& other) noexcept : value_(std::exchange(other.value_, -1)) {}

void checked(bool success, const char* operation) {
    if (!success) throw std::runtime_error(std::string(operation) + "; errno=" + std::to_string(errno));
}
void writeAll(int fd, const std::string& contents) {
    std::size_t written = 0;
    while (written < contents.size()) {
        const auto count = ::write(fd, contents.data() + written, contents.size() - written);
        if (count < 0 && errno == EINTR) continue;
        checked(count > 0, "Cannot write process record");
        written += static_cast<std::size_t>(count);
    }
}
namespace {
void checkedReservedDirectory(bool success, const char* operation,
                              const std::filesystem::path& root, const std::string& id) {
    if (success) return;
    const int failure_errno = errno;
    const auto retained = root / "build-cli-records" / id;
    throw std::runtime_error(std::string(operation) + "; errno=" + std::to_string(failure_errno) +
                             "; retained record: " + retained.string());
}
Fd openDirectory(int parent, const char* name) {
    checked(::mkdirat(parent, name, 0700) == 0 || errno == EEXIST, "Cannot create log ancestor");
    Fd fd(::openat(parent, name, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));
    checked(fd.get() >= 0, "Log ancestor must be a real directory");
    struct stat identity{};
    checked(::fstat(fd.get(), &identity) == 0, "Cannot inspect log ancestor");
    if (identity.st_uid != ::geteuid() || (identity.st_mode & 0077) != 0)
        throw std::runtime_error("Log ancestor has an unsafe owner or permissions");
    checked(::fsync(parent) == 0, "Cannot persist log ancestor");
    return fd;
}
std::string randomId() {
    std::array<unsigned char, 16> bytes{};
    std::size_t filled = 0;
    while (filled < bytes.size()) {
        const auto count = ::getrandom(bytes.data() + filled, bytes.size() - filled, 0);
        if (count < 0 && errno == EINTR) continue;
        checked(count > 0, "Cannot obtain independent OS record identity");
        filled += static_cast<std::size_t>(count);
    }
    constexpr char hex[] = "0123456789abcdef";
    std::string id = "record_";
    for (const auto byte : bytes) { id += hex[byte >> 4]; id += hex[byte & 15]; }
    return id;
}
Fd reserve(const std::filesystem::path& root, std::string& id, std::filesystem::path& path) {
    Fd repo(::open(root.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));
    checked(repo.get() >= 0, "Cannot open trusted checkout");
    struct stat origin{};
    checked(::fstatat(repo.get(), "origin", &origin, AT_SYMLINK_NOFOLLOW) == 0 && S_ISDIR(origin.st_mode),
            "Trusted checkout requires a real origin directory");
    // 独立私有日志根，不要求更改已有CMake build目录的共享权限。
    const auto logs = openDirectory(repo.get(), "build-cli-records");
    constexpr int collision_limit = 4;
    for (int attempt = 0; attempt < collision_limit; ++attempt) {
        id = randomId();
        if (::mkdirat(logs.get(), id.c_str(), 0700) != 0) {
            if (errno == EEXIST) continue;
            checked(false, "Cannot reserve exclusive process record directory");
        }
        Fd directory(::openat(logs.get(), id.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW));
        // mkdir已成功，构造失败时也须保留本次目录身份；碰撞候选不进入此分支。
        checkedReservedDirectory(directory.get() >= 0, "Cannot anchor process record directory", root, id);
        checkedReservedDirectory(::fsync(logs.get()) == 0, "Cannot persist process record directory", root, id);
        path = root / "build-cli-records" / id;
        return directory;
    }
    throw std::runtime_error("Process record identity collision limit exceeded");
}
}  // namespace

Directory::Directory(const std::filesystem::path& root) : directory_(reserve(root, id_, path_)) {}
Fd Directory::newFile(const char* name) const {
    Fd file(::openat(fd(), name, O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600));
    checked(file.get() >= 0, "Cannot create exclusive process log");
    return file;
}
void Directory::publish(const char* name, const std::string& contents) const {
    const std::string temporary = std::string(name) + ".pending";
    auto file = newFile(temporary.c_str());
    writeAll(file.get(), contents);
    checked(::fsync(file.get()) == 0, "Cannot synchronize process record contents");
    checked(::linkat(fd(), temporary.c_str(), fd(), name, 0) == 0, "Cannot publish new process record");
    checked(::fsync(fd()) == 0, "Cannot confirm process record publication");
    // 只有发布确认后才清理本次临时链接；任何失败记录和原始日志均保留。
    (void)::unlinkat(fd(), temporary.c_str(), 0);
}
}  // namespace egomotion::cli::process_record
