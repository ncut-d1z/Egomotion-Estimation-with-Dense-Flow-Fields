#include "egomotion/io/run_text_publisher.hpp"

#include <cerrno>
#include <fcntl.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <unistd.h>

#include <array>
#include <cstddef>
#include <limits>
#include <string>
#include <string_view>
#include <utility>

namespace egomotion::io {
namespace {
class Fd{
public:
    explicit Fd(int fd=-1)noexcept:fd_(fd){}
    ~Fd(){if(fd_>=0)::close(fd_);}
    Fd(const Fd&)=delete;
    Fd& operator=(const Fd&)=delete;
    int get()const noexcept{return fd_;}
private:int fd_;
};
Status<MatErrorCode> fail(const RunDirectory& run,MatErrorCode code,
                          std::string_view name,std::string message) {
    ErrorDetail d;d.stage=ErrorStage::Output;d.run_id=run.runId();
    d.source_path=(run.outputRoot()/run.runId().text).string();
    d.variable=std::string(name);d.message=std::move(message);
    return Status<MatErrorCode>::failure({code,std::move(d)});
}
bool safeBasename(std::string_view x) {
    if(x.empty()||x.size()>128||x=="."||x==".."||x.find("..")!=x.npos)return false;
    for(unsigned char c:x)
        if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||
             (c>='0'&&c<='9')||c=='.'||c=='_'||c=='-'))return false;
    return true;
}
bool temporaryName(std::string& out){
    std::array<unsigned char,12> random{};
    auto* p=random.data();std::size_t remaining=random.size();
    while(remaining){
        const auto n=::getrandom(p,remaining,0);
        if(n<0){if(errno==EINTR)continue;return false;}
        if(!n)return false;
        p+=n;remaining-=static_cast<std::size_t>(n);
    }
    static constexpr char h[]="0123456789abcdef";
    out=".text_";
    for(auto x:random){out.push_back(h[x>>4]);out.push_back(h[x&15]);}
    out+=".tmp";
    return true;
}
} // namespace

Status<MatErrorCode> RunTextPublisher::publishNew(
    std::string_view filename,std::string_view contents) {
    if(run_.runFd()<0||!safeBasename(filename)||contents.empty())
        return fail(run_,MatErrorCode::OutputPathForbidden,filename,
                    "Invalid safe file name, empty contents or detached RunId");
    const std::string name(filename);
    struct stat existing{};
    if(::fstatat(run_.runFd(),name.c_str(),&existing,AT_SYMLINK_NOFOLLOW)==0)
        return fail(run_,MatErrorCode::OutputExists,filename,
                    "Output file already exists and cannot be replaced");
    if(errno!=ENOENT)
        return fail(run_,MatErrorCode::OutputPathForbidden,filename,
                    "Unable to inspect output file identity");
    std::string tmp;
    if(!temporaryName(tmp))
        return fail(run_,MatErrorCode::WriteFailed,filename,
                    "OS entropy unavailable for output temporary file");
    Fd fd(::openat(run_.runFd(),tmp.c_str(),
                   O_CREAT|O_EXCL|O_WRONLY|O_CLOEXEC|O_NOFOLLOW,0600));
    if(fd.get()<0)
        return fail(run_,errno==EEXIST?MatErrorCode::OutputExists:
                    MatErrorCode::WriteFailed,filename,
                    "Exclusive output staging creation failed");
    std::size_t n=0;
    while(n<contents.size()){
        constexpr std::size_t chunk=1<<20;
        const std::size_t remaining=contents.size()-n;
        const auto w=::write(fd.get(),contents.data()+n,
                             remaining<chunk?remaining:chunk);
        if(w<0){if(errno==EINTR)continue;
            return fail(run_,MatErrorCode::WriteFailed,filename,
                        "Staging write failed");}
        if(w==0)return fail(run_,MatErrorCode::WriteFailed,filename,
                            "Staging write made no progress");
        n+=static_cast<std::size_t>(w);
    }
    if(::fsync(fd.get())!=0)
        return fail(run_,MatErrorCode::WriteFailed,filename,
                    "Cannot sync output staging inode");
    if(::linkat(run_.runFd(),tmp.c_str(),run_.runFd(),name.c_str(),0)!=0)
        return fail(run_,errno==EEXIST?MatErrorCode::OutputExists:
                    MatErrorCode::WriteFailed,filename,
                    "Atomic non-overwrite text publication failed");
    if(::fsync(run_.runFd())!=0)
        return fail(run_,MatErrorCode::ReadbackMismatch,filename,
                    "Output file exists but directory durability is unconfirmed");
    // This temporary is exclusively owned by this invocation only.
    (void)::unlinkat(run_.runFd(),tmp.c_str(),0);
    return Status<MatErrorCode>::success();
}
} // namespace egomotion::io
