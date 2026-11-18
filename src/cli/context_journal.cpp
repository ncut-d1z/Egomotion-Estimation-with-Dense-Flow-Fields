#include "context_journal.hpp"
#include "execution_record.hpp"
#include "process_record_storage.hpp"
#include "egomotion/contracts/diagnostics.hpp"

#include <limits>
#include <stdexcept>
#include <unistd.h>

namespace egomotion::cli {
Status<MatErrorCode> ContextJournal::append(std::string_view event, std::string_view value_json) {
    try {
        if (sequence_ == std::numeric_limits<std::uint64_t>::max())
            throw std::runtime_error("Context journal sequence exhausted");
        // 既有invocation序列化器末尾带换行；事件仍保持每条一个物理JSONL行。
        while (!value_json.empty() && (value_json.back() == '\n' || value_json.back() == '\r'))
            value_json.remove_suffix(1);
        const auto json = "{\"schema\":\"egomotion-context-event-v1\",\"record_id\":" + jsonQuote(record_id_) +
            ",\"sequence\":" + std::to_string(sequence_ + 1) + ",\"observed_utc\":" + processTimestamp() +
            ",\"event\":" + jsonQuote(event) + ",\"data\":" + std::string(value_json) + "}\n";
        process_record::writeAll(fd_, json);
        process_record::checked(::fsync(fd_) == 0, "Cannot confirm context event");
        ++sequence_;
        return Status<MatErrorCode>::success();
    } catch (const std::exception& error) {
        ErrorDetail detail; detail.stage = ErrorStage::Output; detail.source_path = path_.string();
        detail.variable = std::string(event); detail.message = error.what();
        return Status<MatErrorCode>::failure({MatErrorCode::WriteFailed, std::move(detail)});
    }
}
}  // namespace egomotion::cli
