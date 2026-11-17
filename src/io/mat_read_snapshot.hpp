#pragma once

#include "egomotion/contracts/result.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <unistd.h>

namespace egomotion::io::detail {
// Linux匿名封存字节副本；不写原路径或持久临时文件。
class MatReadSnapshot {
public:
    MatReadSnapshot(int fd,std::string sha):fd_(fd),sha_(std::move(sha)) {}
    ~MatReadSnapshot(){if(fd_>=0)::close(fd_);}
    MatReadSnapshot(const MatReadSnapshot&)=delete;
    MatReadSnapshot& operator=(const MatReadSnapshot&)=delete;
    MatReadSnapshot(MatReadSnapshot&& other) noexcept
        :fd_(std::exchange(other.fd_,-1)),sha_(std::move(other.sha_)) {}
    MatReadSnapshot& operator=(MatReadSnapshot&&)=delete;
    int fd() const noexcept{return fd_;}
    const std::string& sha256() const noexcept{return sha_;}
private:
    int fd_;
    std::string sha_;
};
Result<MatReadSnapshot,MatErrorCode> snapshotMatInput(
    int source,const std::filesystem::path& path,std::string_view variable);
} // namespace egomotion::io::detail
