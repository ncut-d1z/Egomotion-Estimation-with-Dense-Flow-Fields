#include "egomotion/io/run_directory.hpp"

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <unistd.h>

#include <array>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace egomotion::io {
namespace {
class OwnedFd {
public:
    explicit OwnedFd(int fd=-1) noexcept:fd_(fd) {}
    ~OwnedFd(){if(fd_>=0) ::close(fd_);}
    OwnedFd(const OwnedFd&)=delete;
    OwnedFd& operator=(const OwnedFd&)=delete;
    int get() const noexcept{return fd_;}
    int release() noexcept{int old=fd_;fd_=-1;return old;}
private:int fd_;
};
bool validRunId(const std::string& id) {
    if(id.empty()||id.size()>64)return false;
    for(unsigned char c:id)
        if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||
             (c>='0'&&c<='9')||c=='_'||c=='-'))return false;
    return true;
}
Result<std::unique_ptr<RunDirectory>,MatErrorCode> fail(
    MatErrorCode code,std::string message,const std::filesystem::path& path) {
    ErrorDetail d;
    d.stage=ErrorStage::Output;d.source_path=path.string();
    d.message=std::move(message);
    return Result<std::unique_ptr<RunDirectory>,MatErrorCode>::failure(
        {code,std::move(d)});
}
bool randomBytes(void* p,std::size_t n) {
    auto* b=static_cast<unsigned char*>(p);
    while(n) {
        const ssize_t count=::getrandom(b,n,0);
        if(count<0){if(errno==EINTR)continue;return false;}
        if(count==0)return false;
        b+=count;n-=static_cast<std::size_t>(count);
    }
    return true;
}
std::optional<RunId> newRandomRunId() {
    std::array<unsigned char,16> bytes{};
    if(!randomBytes(bytes.data(),bytes.size()))return std::nullopt;
    static constexpr char hex[]="0123456789abcdef";
    std::string id="run_";
    for(auto byte:bytes){id.push_back(hex[byte>>4]);id.push_back(hex[byte&15]);}
    return RunId{std::move(id)};
}
}  // namespace

RunDirectory::~RunDirectory() {
    if(run_fd_>=0)::close(run_fd_);
    if(output_fd_>=0)::close(output_fd_);
}
Result<std::unique_ptr<RunDirectory>,MatErrorCode> RunDirectory::create(
    const std::filesystem::path& repository_root,
    const std::optional<RunId>& requested) {
    using ResultType=Result<std::unique_ptr<RunDirectory>,MatErrorCode>;
    if(repository_root.empty())
        return fail(MatErrorCode::OutputPathForbidden,"Empty trusted repository root",repository_root);
    // Trust must originate in the application, not an unvalidated CLI path.
    OwnedFd repo(::open(repository_root.c_str(),
                        O_RDONLY|O_DIRECTORY|O_CLOEXEC|O_NOFOLLOW));
    if(repo.get()<0)
        return fail(MatErrorCode::OutputPathForbidden,
                    "Repository root cannot be opened without following a symlink",repository_root);
    struct stat origin_stat{};
    if(::fstatat(repo.get(),"origin",&origin_stat,AT_SYMLINK_NOFOLLOW)!=0 ||
       !S_ISDIR(origin_stat.st_mode))
        return fail(MatErrorCode::OutputPathForbidden,
                    "Expected original read-only archive under trusted repository",repository_root);
    if(::mkdirat(repo.get(),"output",0700)!=0 && errno!=EEXIST)
        return fail(MatErrorCode::OutputPathForbidden,
                    "Cannot create controlled output directory",repository_root);
    OwnedFd output(::openat(repo.get(),"output",
                            O_RDONLY|O_DIRECTORY|O_CLOEXEC|O_NOFOLLOW));
    if(output.get()<0)
        return fail(MatErrorCode::OutputPathForbidden,
                    "Output root is not a safe directory",repository_root);
    const auto actual_root=repository_root/"output";
    if(requested && !validRunId(requested->text))
        return fail(MatErrorCode::OutputPathForbidden,"Invalid RunId",actual_root);
    constexpr int max_tries=4;
    for(int attempt=0;attempt<(requested?1:max_tries);++attempt){
        const auto id=requested?requested:newRandomRunId();
        if(!id)return fail(MatErrorCode::WriteFailed,"OS getrandom failed",actual_root);
        if(::mkdirat(output.get(),id->text.c_str(),0700)!=0) {
            if(errno==EEXIST) {
                if(requested)return fail(MatErrorCode::OutputExists,
                                         "RunId directory already exists",actual_root/id->text);
                continue;
            }
            return fail(MatErrorCode::OutputPathForbidden,
                        "Exclusive RunId creation failed",actual_root/id->text);
        }
        OwnedFd run(::openat(output.get(),id->text.c_str(),
                            O_RDONLY|O_DIRECTORY|O_CLOEXEC|O_NOFOLLOW));
        if(run.get()<0)
            return fail(MatErrorCode::OutputPathForbidden,
                        "Created RunId directory cannot be reopened safely",actual_root/id->text);
        // The owned fd anchors subsequent writes to the directory inode even
        // if an attacker replaces a path component after this point.
        // 路径或RunId复制仍可能抛出；构造成功前由局部RAII继续持有fd。
        auto directory=std::unique_ptr<RunDirectory>(
            new RunDirectory(*id,actual_root,output.get(),run.get()));
        output.release();
        run.release();
        return ResultType::success(std::move(directory));
    }
    return fail(MatErrorCode::OutputExists,"Random RunId collision limit exceeded",actual_root);
}
}  // namespace egomotion::io
