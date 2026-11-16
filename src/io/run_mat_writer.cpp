#include "egomotion/io/run_mat_writer.hpp"

#include "egomotion/io/mat_result_reader.hpp"
#include "egomotion/io/sha256.hpp"

#include <matio.h>

#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <unistd.h>

#include <array>
#include <cstddef>
#include <limits>
#include <memory>
#include <stdexcept>
#include <new>
#include <string>
#include <string_view>
#include <utility>

namespace egomotion::io {
namespace {
class OwnedFd {
public:
    explicit OwnedFd(int fd=-1) noexcept:fd_(fd) {}
    ~OwnedFd(){if(fd_>=0)::close(fd_);}
    OwnedFd(const OwnedFd&)=delete;
    OwnedFd& operator=(const OwnedFd&)=delete;
    int get() const noexcept{return fd_;}
private:int fd_;
};
// Unlink only the uniquely O_EXCL-created staging name. The owned dirfd
// prevents pathname redirection; this guard never removes the published name.
class StagingGuard {
public:
    StagingGuard(int dir_fd,const std::string& name) noexcept
        : dir_fd_(dir_fd),name_(name) {}
    StagingGuard(const StagingGuard&)=delete;
    StagingGuard& operator=(const StagingGuard&)=delete;
    ~StagingGuard() noexcept {
        if(active_)(void)::unlinkat(dir_fd_,name_.c_str(),0);
    }
    void disarm() noexcept {active_=false;}
private:
    int dir_fd_;
    const std::string& name_;
    bool active_=true;
};
class MatHandle {
public:
    explicit MatHandle(mat_t* mat=nullptr) noexcept:mat_(mat) {}
    ~MatHandle(){if(mat_)Mat_Close(mat_);}
    MatHandle(const MatHandle&)=delete;
    MatHandle& operator=(const MatHandle&)=delete;
    int close() noexcept {
        mat_t* old=mat_;mat_=nullptr;
        return old?Mat_Close(old):-1;
    }
    mat_t* get()const noexcept{return mat_;}
private:mat_t* mat_;
};
struct MatVarFree {
    void operator()(matvar_t* v)const noexcept {if(v)Mat_VarFree(v);}
};
using VarHandle=std::unique_ptr<matvar_t,MatVarFree>;

Status<MatErrorCode> fail(MatErrorCode code,const RunDirectory& dir,
                          std::string name,std::string message) {
    ErrorDetail detail;
    detail.stage=ErrorStage::Output;
    detail.run_id=dir.runId();
    detail.variable=std::move(name);
    detail.source_path=(dir.outputRoot()/dir.runId().text).string();
    detail.message=std::move(message);
    return Status<MatErrorCode>::failure({code,std::move(detail)});
}
bool safeName(std::string_view name) {
    if(name.empty()||name.size()>64)return false;
    if(!((name[0]>='a'&&name[0]<='z')||
         (name[0]>='A'&&name[0]<='Z')))return false;
    for(unsigned char c:name)
        if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||
             (c>='0'&&c<='9')||c=='_'))return false;
    return true;
}
bool validPayload(std::string_view name,const DenseDoubleArray& a) {
    if(!safeName(name)||a.info.name!=name||
       a.info.original_class_type!=static_cast<int>(MAT_C_DOUBLE)||
       a.info.original_data_type!=static_cast<int>(MAT_T_DOUBLE)||
       a.info.is_complex||a.info.is_logical||a.info.dimensions.size()!=2)return false;
    const auto rows=a.info.dimensions[0],cols=a.info.dimensions[1];
    if(!rows||!cols||cols>std::numeric_limits<std::size_t>::max()/rows)return false;
    const auto n=rows*cols;
    if(n>std::numeric_limits<std::size_t>::max()/sizeof(double))return false;
    if(a.values_column_major.size()!=n||
       a.info.data_bytes!=n*sizeof(double))return false;
    for(double x:a.values_column_major)if(!std::isfinite(x))return false;
    return true;
}
bool getRandomName(std::string& out) {
    std::array<unsigned char,12> nonce{};
    unsigned char* p=nonce.data();std::size_t left=nonce.size();
    while(left){
        const auto n=::getrandom(p,left,0);
        if(n<0){if(errno==EINTR)continue;return false;}
        if(n==0)return false;
        left-=static_cast<std::size_t>(n);p+=n;
    }
    static constexpr char hex[]="0123456789abcdef";
    out=".r5_";
    for(const auto x:nonce){out.push_back(hex[x>>4]);out.push_back(hex[x&15]);}
    out+=".mat-tmp";
    return true;
}
bool sameValues(const DenseDoubleArray& expected,const DenseDoubleArray& readback) {
    const auto& x=expected.values_column_major;
    const auto& y=readback.values_column_major;
    return readback.info.name==expected.info.name &&
           readback.info.dimensions==expected.info.dimensions &&
           readback.info.original_class_type==expected.info.original_class_type &&
           readback.info.original_data_type==expected.info.original_data_type &&
           x.size()==y.size() && std::memcmp(x.data(),y.data(),x.size()*sizeof(double))==0;
}
}  // namespace

std::vector<FileEvidence> RunMatWriter::fileEvidence() const {
    std::vector<FileEvidence> result;
    result.reserve(receipts_.size());
    for(const auto& pair:receipts_) {
        const auto& r=pair.second;
        result.push_back(FileEvidence{r.variable,r.filename,r.sha256,
                                      r.bytes,r.durably_confirmed});
    }
    return result;
}

Status<MatErrorCode> RunMatWriter::saveNewVariable(
    const RunId& run_id,std::string_view variable_name,
    const DenseDoubleArray& payload) {
    if(run_id.text!=directory_.runId().text||directory_.runFd()<0)
        return fail(MatErrorCode::OutputPathForbidden,directory_,
                    std::string(variable_name),"RunId does not match reserved output inode");
    if(!validPayload(variable_name,payload))
        return fail(MatErrorCode::OutputPathForbidden,directory_,
                    std::string(variable_name),"Illegal MAT variable name, type, shape or content");
    try {
        const std::string name(variable_name);
        const std::string final_name=name+".mat";
        struct stat existing{};
        if(::fstatat(directory_.runFd(),final_name.c_str(),&existing,
                     AT_SYMLINK_NOFOLLOW)==0)
            return fail(MatErrorCode::OutputExists,directory_,name,
                        "A file with this MAT variable name already exists");
        if(errno!=ENOENT)
            return fail(MatErrorCode::OutputPathForbidden,directory_,name,
                        "Cannot safely inspect the final MAT path");
        std::string temporary;
        if(!getRandomName(temporary))
            return fail(MatErrorCode::WriteFailed,directory_,name,
                        "OS random source unavailable for temporary file");
        // Linux dirfd pins the actual RunId directory. Never use the lexical
        // output path in Mat_CreateVer or Mat_Open; path components may change.
        OwnedFd file(::openat(directory_.runFd(),temporary.c_str(),
                             O_CREAT|O_EXCL|O_RDWR|O_CLOEXEC|O_NOFOLLOW,0600));
        if(file.get()<0)
            return fail(errno==EEXIST?MatErrorCode::OutputExists:
                        MatErrorCode::OutputPathForbidden,directory_,name,
                        "Exclusive MAT staging-file creation failed");
        StagingGuard staging(directory_.runFd(),temporary);
        // Stable /proc/self/fd/NN refers to this very inode. matio may
        // reopen that descriptor but cannot follow a replaced RunId symlink.
        const std::string fd_path="/proc/self/fd/"+std::to_string(file.get());
        MatHandle mat(Mat_CreateVer(fd_path.c_str(),nullptr,MAT_FT_MAT5));
        if(!mat.get())
            return fail(MatErrorCode::WriteFailed,directory_,name,
                        "matio could not create MAT v5 on the owned staging inode");
        std::size_t dims[2]{payload.info.dimensions[0],payload.info.dimensions[1]};
        // flags=0 -> matio copies values. It will own and free the copy.
        VarHandle variable(Mat_VarCreate(name.c_str(),MAT_C_DOUBLE,MAT_T_DOUBLE,
                             2,dims,
                             payload.values_column_major.data(),0));
        if(!variable)
            return fail(MatErrorCode::WriteFailed,directory_,name,
                        "matio could not allocate the named double variable");
        if(Mat_VarWrite(mat.get(),variable.get(),MAT_COMPRESSION_ZLIB)!=0)
            return fail(MatErrorCode::WriteFailed,directory_,name,
                        "Mat_VarWrite failed before file publication");
        variable.reset();
        if(mat.close()!=0||::fsync(file.get())!=0)
            return fail(MatErrorCode::WriteFailed,directory_,name,
                        "MAT close or staging fsync failed; no final file published");
        // Read from the same pinned file inode. No exists/read Named split
        // against a different pathname can substitute this check.
        MatResultReader reader;
        auto back=reader.readNamed(fd_path,name);
        if(!back.hasValue()) {
            ErrorDetail outer;
            outer.stage=ErrorStage::Output;
            outer.run_id=directory_.runId();
            outer.variable=name;
            outer.source_path=(directory_.outputRoot()/
                               directory_.runId().text).string();
            outer.message="Staging MAT cannot be fully decoded";
            return Status<MatErrorCode>::failure(wrapFailure(
                MatErrorCode::ReadbackMismatch,std::move(outer),back.error()));
        }
        if(!sameValues(payload,back.value()))
            return fail(MatErrorCode::ReadbackMismatch,directory_,name,
                        "Staging MAT variable name/type/shape/bytes do not match");
        auto sha=sha256Fd(file.get());
        if(!sha.hasValue()) {
            ErrorDetail outer;
            outer.stage=ErrorStage::Output;
            outer.run_id=directory_.runId();
            outer.variable=name;
            outer.message="Cannot SHA-256 the staging MAT file";
            return Status<MatErrorCode>::failure(wrapFailure(
                MatErrorCode::ReadbackMismatch,std::move(outer),sha.error()));
        }
        struct stat before{};
        if(::fstat(file.get(),&before)!=0||!S_ISREG(before.st_mode))
            return fail(MatErrorCode::OutputPathForbidden,directory_,name,
                        "Staging inode is not a regular file");
        MatFileReceipt receipt{name,final_name,sha.value(),
                               static_cast<std::size_t>(before.st_size)};
        // Reserve receipt memory BEFORE publishing: a later map allocation
        // failure must not turn a successfully published file into an
        // untracked orphan.
        auto inserted=receipts_.emplace(name,std::move(receipt));
        if(!inserted.second)
            return fail(MatErrorCode::OutputExists,directory_,name,
                        "Writer already reported this MAT variable");
        // linkat is atomic and fails with EEXIST; unlike rename() it never
        // silently replaces an existing user file/symlink.
        if(::linkat(directory_.runFd(),temporary.c_str(),
                    directory_.runFd(),final_name.c_str(),0)!=0) {
            receipts_.erase(name);
            return fail(errno==EEXIST?MatErrorCode::OutputExists:
                        MatErrorCode::WriteFailed,directory_,name,
                        "Atomic no-overwrite MAT publication failed");
        }
        // If verification after publish fails, preserve the published file
        // and return non-success. NEVER hide it, overwrite it or claim Complete.
        struct stat published{};
        if(::fstatat(directory_.runFd(),final_name.c_str(),&published,
                     AT_SYMLINK_NOFOLLOW)!=0 ||
           published.st_ino!=before.st_ino||published.st_dev!=before.st_dev||
           ::fsync(directory_.runFd())!=0) {
            // Retain its *actual* published identity for a partial-run audit,
            // but do not mark it durably confirmed.
            return fail(MatErrorCode::ReadbackMismatch,directory_,name,
                        "Published MAT inode or directory durability could not be confirmed");
        }
        // Publishing the final name does not authorize an orphan staging
        // hard-link. Failure to remove/sync it is a non-success output event;
        // the final MAT and receipt remain available for Incomplete audit.
        if(::unlinkat(directory_.runFd(),temporary.c_str(),0)!=0)
            return fail(MatErrorCode::ReadbackMismatch,directory_,name,
                        "Published MAT exists, but staging hard-link cleanup failed");
        staging.disarm();
        if(::fsync(directory_.runFd())!=0)
            return fail(MatErrorCode::ReadbackMismatch,directory_,name,
                        "Published MAT exists, but staging cleanup durability is unconfirmed");
        receipts_.at(name).durably_confirmed=true;
        return Status<MatErrorCode>::success();
    } catch(const std::bad_alloc&) {
        return fail(MatErrorCode::WriteFailed,directory_,
                    std::string(variable_name),"MAT Writer allocation failed");
    } catch(const std::length_error&) {
        return fail(MatErrorCode::SizeOverflow,directory_,
                    std::string(variable_name),"MAT Writer data exceeds container capacity");
    }
}
}  // namespace egomotion::io
