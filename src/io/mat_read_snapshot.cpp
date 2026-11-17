#include "mat_read_snapshot.hpp"
#include "egomotion/io/sha256.hpp"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdint>
#include <new>
#include <stdexcept>

namespace egomotion::io::detail {
namespace {
using SnapshotResult=Result<MatReadSnapshot,MatErrorCode>;
struct Descriptor {
    int fd=-1;
    ~Descriptor(){if(fd>=0)::close(fd);}
};
SnapshotResult fail(const std::filesystem::path& path,std::string_view variable,
                    const char* message,MatErrorCode code=MatErrorCode::InvalidMatHeader) {
    ErrorDetail d;d.stage=ErrorStage::Read;d.source_path=path.string();
    d.variable=std::string(variable);d.message=message;
    return SnapshotResult::failure({code,std::move(d)});
}
bool sameSource(const struct stat& before,const struct stat& after) {
    return before.st_dev==after.st_dev&&before.st_ino==after.st_ino&&
           before.st_size==after.st_size&&before.st_mtim.tv_sec==after.st_mtim.tv_sec&&
           before.st_mtim.tv_nsec==after.st_mtim.tv_nsec&&
           before.st_ctim.tv_sec==after.st_ctim.tv_sec&&before.st_ctim.tv_nsec==after.st_ctim.tv_nsec;
}
bool copyBytes(int source,int destination,off_t size) {
    constexpr std::size_t kCopyBufferBytes=64*1024;
    std::array<unsigned char,kCopyBufferBytes> buffer{};
    off_t offset=0;
    while(offset<size) {
        const auto count=static_cast<std::size_t>(std::min<std::uintmax_t>(
            static_cast<std::uintmax_t>(size-offset),buffer.size()));
        const auto got=::pread(source,buffer.data(),count,offset);
        if(got<0&&errno==EINTR)continue;
        if(got<=0)return false;
        std::size_t written=0;
        while(written<static_cast<std::size_t>(got)) {
            const auto n=::write(destination,buffer.data()+written,static_cast<std::size_t>(got)-written);
            if(n<0&&errno==EINTR)continue;
            if(n<=0)return false;
            written+=static_cast<std::size_t>(n);
        }
        offset+=got;
    }
    return true;
}
} // namespace
SnapshotResult snapshotMatInput(int source,const std::filesystem::path& path,
                                std::string_view variable) {
    try {
        struct stat before{},after{};
        if(::fstat(source,&before)!=0||!S_ISREG(before.st_mode)||before.st_size<0)
            return fail(path,variable,"MAT snapshot requires a regular readable file");
        Descriptor snapshot{::memfd_create("egomotion-mat-read",MFD_CLOEXEC|MFD_ALLOW_SEALING)};
        if(snapshot.fd<0)return fail(path,variable,"Cannot allocate sealed MAT input snapshot");
        if(!copyBytes(source,snapshot.fd,before.st_size))
            return fail(path,variable,"Cannot copy complete MAT input snapshot");
        if(::fstat(source,&after)!=0||!sameSource(before,after))
            return fail(path,variable,"MAT input changed while capturing snapshot");
        // 先封存，再校验/摘要/解码；即使原inode随后被改写，这三者仍消费相同字节。
        constexpr int kSeals=F_SEAL_WRITE|F_SEAL_GROW|F_SEAL_SHRINK|F_SEAL_SEAL;
        if(::fcntl(snapshot.fd,F_ADD_SEALS,kSeals)!=0)
            return fail(path,variable,"Cannot seal MAT input snapshot");
        const int actual_seals=::fcntl(snapshot.fd,F_GET_SEALS);
        if(actual_seals<0||(actual_seals&kSeals)!=kSeals)
            return fail(path,variable,"MAT input snapshot seals are unconfirmed");
        auto hash=sha256Fd(snapshot.fd);
        if(!hash.hasValue()) {
            ErrorDetail d;d.stage=ErrorStage::Read;d.source_path=path.string();
            d.variable=std::string(variable);d.message="Cannot hash sealed MAT input snapshot";
            return SnapshotResult::failure(wrapFailure(MatErrorCode::InvalidMatHeader,std::move(d),hash.error()));
        }
        auto digest=std::move(hash).value();
        return SnapshotResult::success(MatReadSnapshot(std::exchange(snapshot.fd,-1),std::move(digest)));
    } catch(const std::bad_alloc&) {
        return fail(path,variable,"Cannot allocate MAT input snapshot metadata",MatErrorCode::SizeOverflow);
    } catch(const std::length_error&) {
        return fail(path,variable,"MAT input snapshot exceeds container capacity",MatErrorCode::SizeOverflow);
    }
}
} // namespace egomotion::io::detail
