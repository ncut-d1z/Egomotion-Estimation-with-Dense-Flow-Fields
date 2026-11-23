#pragma once

#include "egomotion/io/mat_read_observer.hpp"

#include <optional>
#include <string_view>

namespace egomotion::cli {

// 候选的显式借用接口，不依赖进程全局变量或环境中的隐式文件描述符。
class IInvocationObserver : public io::IMatReadObserver {
public:
    Status<MatErrorCode> record(std::string_view event, std::string_view value_json);
    const Failure<MatErrorCode>* failure() const noexcept {
        return failure_ ? &*failure_ : nullptr;
    }
    Status<MatErrorCode> snapshotPrepared(const std::filesystem::path& source,
        std::string_view requested_variable, std::string_view sha256, io::MatReadPurpose purpose) final;
protected:
    // value_json来自应用内部序列化器；实现必须完整写入并确认记录。
    virtual Status<MatErrorCode> append(std::string_view event, std::string_view value_json) = 0;
private:
    std::optional<Failure<MatErrorCode>> failure_;
};

}  // namespace egomotion::cli
