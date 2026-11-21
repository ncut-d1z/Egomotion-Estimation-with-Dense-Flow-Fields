#pragma once

#include "egomotion/cli/invocation_observer.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>

namespace egomotion::cli {

// fd由进程记录器拥有；日志只追加。本对象只在fork后的单线程子进程使用。
class ContextJournal final : public IInvocationObserver {
public:
    ContextJournal(int fd, std::string record_id, std::filesystem::path path)
        : fd_(fd), record_id_(std::move(record_id)), path_(std::move(path)) {}
protected:
    Status<MatErrorCode> append(std::string_view event, std::string_view value_json) override;
private:
    int fd_;
    std::string record_id_;
    std::filesystem::path path_;
    std::uint64_t sequence_ = 0;
};

}  // namespace egomotion::cli
