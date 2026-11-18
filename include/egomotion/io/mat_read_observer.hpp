#pragma once

#include "egomotion/contracts/result.hpp"

#include <filesystem>
#include <string_view>

namespace egomotion::io {

enum class MatReadPurpose { Metadata, Named, FirstCompatible, DictionaryDecode };

class IMatReadObserver {
public:
    virtual ~IMatReadObserver() = default;
    // 摘要属于已封存字节；发生在matio打开/解码前，不声称变量已被成功解码。
    // 返回失败时Reader不得继续消费快照。观察者由调用者拥有且必须长于Reader。
    virtual Status<MatErrorCode> snapshotPrepared(const std::filesystem::path& source,
        std::string_view requested_variable, std::string_view sha256, MatReadPurpose purpose) = 0;
};

}  // namespace egomotion::io
