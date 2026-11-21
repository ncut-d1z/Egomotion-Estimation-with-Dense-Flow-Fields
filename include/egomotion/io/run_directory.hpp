#pragma once

#include "egomotion/contracts/result.hpp"

#include <filesystem>
#include <memory>
#include <optional>

namespace egomotion::io {

// Linux-only, owns the stable file descriptors for output/<RunId>.
// Repository root is supplied by the application from a trusted checkout;
// no CLI option may select another output root. All descendants use openat().
class RunDirectory final {
public:
    static Result<std::unique_ptr<RunDirectory>,MatErrorCode> create(
        const std::filesystem::path& trusted_repository_root,
        const std::optional<RunId>& requested = std::nullopt);

    RunDirectory(const RunDirectory&)=delete;
    RunDirectory& operator=(const RunDirectory&)=delete;
    RunDirectory(RunDirectory&&)=delete;
    RunDirectory& operator=(RunDirectory&&)=delete;
    ~RunDirectory();

    const RunId& runId() const noexcept { return id_; }
    const std::filesystem::path& outputRoot() const noexcept { return output_root_; }
    int runFd() const noexcept { return run_fd_; }

private:
    RunDirectory(RunId id,std::filesystem::path root,int output_fd,int run_fd)
       : id_(std::move(id)),output_root_(std::move(root)),
         output_fd_(output_fd),run_fd_(run_fd) {}
    RunId id_;
    std::filesystem::path output_root_;
    int output_fd_=-1;
    int run_fd_=-1;
};

}  // namespace egomotion::io
