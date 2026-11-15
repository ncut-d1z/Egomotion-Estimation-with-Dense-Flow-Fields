#include "egomotion/io/manifest_publisher.hpp"

#include <cerrno>
#include <fcntl.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <unistd.h>

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

namespace egomotion::io {
namespace {
class Fd {
public:
    explicit Fd(int fd=-1)noexcept:fd_(fd){}
    ~Fd(){if(fd_>=0)::close(fd_);}
    Fd(const Fd&)=delete;
    Fd& operator=(const Fd&)=delete;
    int get()const noexcept{return fd_;}
private:int fd_;
};
Status<MatErrorCode> fail(const RunDirectory& run,MatErrorCode code,
                          const std::string& message) {
    ErrorDetail d;d.stage=ErrorStage::Output;d.run_id=run.runId();
    d.source_path=(run.outputRoot()/run.runId().text/"manifest.json").string();
    d.message=message;
    return Status<MatErrorCode>::failure({code,std::move(d)});
}
bool stagingName(std::string& name) {
    unsigned char b[12]{};
    std::size_t left=sizeof(b),at=0;
    while(left){
        auto n=::getrandom(b+at,left,0);
        if(n<0){if(errno==EINTR)continue;return false;}
        if(n==0)return false;
        left-=static_cast<std::size_t>(n);at+=static_cast<std::size_t>(n);
    }
    static constexpr char h[]="0123456789abcdef";
    name=".manifest_";
    for(auto x:b){name.push_back(h[x>>4]);name.push_back(h[x&15]);}
    name+=".tmp";
    return true;
}
} // namespace

Status<MatErrorCode> RunManifestPublisher::publish(std::string_view json) const {
    if(directory_.runFd()<0||json.empty())
        return fail(directory_,MatErrorCode::OutputPathForbidden,
                    "Manifest needs an anchored RunId directory and nonempty JSON");
    struct stat existing{};
    if(::fstatat(directory_.runFd(),"manifest.json",&existing,AT_SYMLINK_NOFOLLOW)==0)
        return fail(directory_,MatErrorCode::OutputExists,
                    "Manifest already exists; never replace it");
    if(errno!=ENOENT)
        return fail(directory_,MatErrorCode::OutputPathForbidden,
                    "Cannot safely inspect manifest path");
    std::string temporary;
    if(!stagingName(temporary))
        return fail(directory_,MatErrorCode::WriteFailed,
                    "OS randomness unavailable for manifest staging");
    Fd fd(::openat(directory_.runFd(),temporary.c_str(),
                   O_CREAT|O_EXCL|O_WRONLY|O_CLOEXEC|O_NOFOLLOW,0600));
    if(fd.get()<0)
        return fail(directory_,errno==EEXIST?MatErrorCode::OutputExists:
                    MatErrorCode::WriteFailed,"Manifest staging creation failed");
    std::size_t consumed=0;
    while(consumed<json.size()){
        const auto n=::write(fd.get(),json.data()+consumed,json.size()-consumed);
        if(n<0){if(errno==EINTR)continue;
            return fail(directory_,MatErrorCode::WriteFailed,
                        "Manifest staging write failed");}
        if(n==0)return fail(directory_,MatErrorCode::WriteFailed,
                            "Manifest staging short write");
        consumed+=static_cast<std::size_t>(n);
    }
    if(::fsync(fd.get())!=0)
        return fail(directory_,MatErrorCode::WriteFailed,
                    "Manifest staging fsync failed");
    if(::linkat(directory_.runFd(),temporary.c_str(),
                directory_.runFd(),"manifest.json",0)!=0)
        return fail(directory_,errno==EEXIST?MatErrorCode::OutputExists:
                    MatErrorCode::WriteFailed,
                    "Atomic manifest no-overwrite publication failed");
    if(::fsync(directory_.runFd())!=0)
        return fail(directory_,MatErrorCode::ReadbackMismatch,
                    "Manifest published but directory durability is not confirmed");
    // Clean up ONLY the private staging inode this invocation created.
    (void)::unlinkat(directory_.runFd(),temporary.c_str(),0);
    return Status<MatErrorCode>::success();
}

}  // namespace egomotion::io
