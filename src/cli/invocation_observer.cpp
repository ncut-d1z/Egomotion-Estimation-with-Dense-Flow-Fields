#include "egomotion/cli/invocation_observer.hpp"
#include "egomotion/contracts/diagnostics.hpp"

#include <exception>
#include <string>
#include <utility>

namespace egomotion::cli {
Status<MatErrorCode> IInvocationObserver::record(std::string_view event, std::string_view value_json) {
    if (failure_) return Status<MatErrorCode>::failure(*failure_);
    try {
        auto result = append(event, value_json);
        if (!result.hasValue()) failure_ = result.error();
        return result;
    } catch (const std::exception& error) {
        ErrorDetail detail; detail.stage = ErrorStage::Output;
        detail.variable = std::string(event); detail.message = error.what();
        failure_.emplace(Failure<MatErrorCode>{MatErrorCode::WriteFailed, std::move(detail)});
        return Status<MatErrorCode>::failure(*failure_);
    }
}
Status<MatErrorCode> IInvocationObserver::snapshotPrepared(const std::filesystem::path& source,
        std::string_view variable, std::string_view sha256, io::MatReadPurpose purpose) {
    const char* operation = "unknown";
    switch (purpose) {
    case io::MatReadPurpose::Metadata: operation = "metadata"; break;
    case io::MatReadPurpose::Named: operation = "named"; break;
    case io::MatReadPurpose::FirstCompatible: operation = "first_compatible"; break;
    case io::MatReadPurpose::DictionaryDecode: operation = "dictionary_decode"; break;
    }
    return record("mat_snapshot_prepared", "{\"path\":" + jsonQuote(source.string()) +
        ",\"requested_variable\":" + (variable.empty() ? "null" : jsonQuote(variable)) +
        ",\"mat_sha256\":" + jsonQuote(sha256) + ",\"purpose\":" + jsonQuote(operation) + '}');
}
}  // namespace egomotion::cli
