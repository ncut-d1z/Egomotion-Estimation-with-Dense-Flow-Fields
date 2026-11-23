// 仅CTest进程使用的ELF插桩。精确匹配新沙箱报告，不修改生产代码或其它文件。
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <limits.h>
#include <sys/uio.h>
#include <unistd.h>

namespace {
bool matches(int fd, const char* operation) {
    const char* requested = std::getenv("EGOMOTION_TEST_REPORT_FAULT");
    const char* target = std::getenv("EGOMOTION_TEST_REPORT_PATH");
    if (!requested || !target || std::strcmp(requested, operation) != 0) return false;
    char proc[64], path[PATH_MAX + 1];
    const int length = std::snprintf(proc, sizeof(proc), "/proc/self/fd/%d", fd);
    if (length <= 0 || static_cast<std::size_t>(length) >= sizeof(proc)) return false;
    const auto size = ::readlink(proc, path, PATH_MAX);
    if (size < 0 || size == PATH_MAX) return false;
    path[size] = '\0';
    return std::strcmp(target, path) == 0;
}
void evidence(const char* operation) {
    // stderr记录明确的注入事件，测试必须实际看到它才可宣称覆盖此故障。
    std::fprintf(stderr, "INJECT dictionary report %s EIO\n", operation);
}
}  // namespace

extern "C" ssize_t write(int fd, const void* bytes, size_t size) {
    using Function = ssize_t (*)(int, const void*, size_t);
    static const auto actual = reinterpret_cast<Function>(::dlsym(RTLD_NEXT, "write"));
    if (!actual) { errno = ENOSYS; return -1; }
    if (matches(fd, "write")) { evidence("write"); errno = EIO; return -1; }
    return actual(fd, bytes, size);
}
extern "C" ssize_t writev(int fd, const struct iovec* vectors, int count) {
    using Function = ssize_t (*)(int, const struct iovec*, int);
    static const auto actual = reinterpret_cast<Function>(::dlsym(RTLD_NEXT, "writev"));
    if (!actual) { errno = ENOSYS; return -1; }
    if (matches(fd, "write")) { evidence("write"); errno = EIO; return -1; }
    return actual(fd, vectors, count);
}
extern "C" int fclose(FILE* file) {
    using Function = int (*)(FILE*);
    static const auto actual = reinterpret_cast<Function>(::dlsym(RTLD_NEXT, "fclose"));
    if (!actual) { errno = ENOSYS; return EOF; }
    const bool inject = matches(::fileno(file), "close");
    const int result = actual(file);
    // 真实关闭一次后才注入失败；保留文件，不泄漏fd、不重复关闭。
    if (inject && result == 0) { evidence("close"); errno = EIO; return EOF; }
    return result;
}
