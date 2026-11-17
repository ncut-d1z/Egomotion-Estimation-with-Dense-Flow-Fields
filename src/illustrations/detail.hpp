#pragma once
#include "egomotion/illustrations/illustration.hpp"

namespace egomotion::illustrations::detail {
inline Failure<NumericalErrorCode> failure(NumericalErrorCode code,std::string message) {
    ErrorDetail detail;detail.stage=ErrorStage::Analyze;detail.message=std::move(message);
    return {code,std::move(detail)};
}
Status<NumericalErrorCode> validate(const Data&);
} // namespace egomotion::illustrations::detail
