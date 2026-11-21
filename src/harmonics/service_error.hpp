#pragma once

#include "egomotion/harmonics/services.hpp"
#include "egomotion/harmonics/low_order.hpp"

#include <string>
#include <string_view>
#include <utility>

namespace egomotion::harmonics::detail {
inline std::string_view sourceName(low_order::ErrorCode code) {
    switch (code) {
    case low_order::ErrorCode::NonFiniteInput: return "NonFiniteInput";
    case low_order::ErrorCode::NotUnitDirection: return "NotUnitDirection";
    case low_order::ErrorCode::NotProperRotation: return "NotProperRotation";
    case low_order::ErrorCode::NonFiniteResult: return "NonFiniteResult";
    case low_order::ErrorCode::ComplexResidue: return "ComplexResidue";
    case low_order::ErrorCode::InvalidMethod: return "InvalidMethod";
    case low_order::ErrorCode::InvalidSampleShape: return "InvalidSampleShape";
    case low_order::ErrorCode::ZeroProjectionNorm: return "ZeroProjectionNorm";
    }
    return "UnknownLeafError";
}
inline NumericalErrorCode numericalCode(low_order::ErrorCode code) {
    switch (code) {
    case low_order::ErrorCode::NonFiniteInput: return NumericalErrorCode::NonFiniteInput;
    case low_order::ErrorCode::NotUnitDirection: return NumericalErrorCode::InvalidDirection;
    case low_order::ErrorCode::NotProperRotation: return NumericalErrorCode::InvalidDirection;
    case low_order::ErrorCode::NonFiniteResult: return NumericalErrorCode::NonFiniteResult;
    case low_order::ErrorCode::ComplexResidue: return NumericalErrorCode::NonFiniteResult;
    case low_order::ErrorCode::InvalidMethod: return NumericalErrorCode::InvalidShape;
    case low_order::ErrorCode::InvalidSampleShape: return NumericalErrorCode::InvalidShape;
    case low_order::ErrorCode::ZeroProjectionNorm: return NumericalErrorCode::SingularSystem;
    }
    return NumericalErrorCode::NonFiniteResult;
}
template<class T>
Result<T,NumericalErrorCode> bridge(const low_order::Error& leaf,
                                   std::string operation) {
    ErrorDetail error;
    error.stage=ErrorStage::Solve;
    error.message=std::move(operation)+": "+leaf.message;
    ErrorFrame source;
    source.domain="LowOrderHarmonicsErrorCode";
    source.code=std::string(sourceName(leaf.code));
    source.stage=ErrorStage::Solve;
    source.message=leaf.message;
    error.causes.push_front(std::move(source));
    return Result<T,NumericalErrorCode>::failure(
        {numericalCode(leaf.code),std::move(error)});
}
template<class T>
Result<T,NumericalErrorCode> resourceFailure() {
    ErrorDetail error;
    error.stage=ErrorStage::Solve;
    error.message="Low-order harmonic adapter allocation failure";
    return Result<T,NumericalErrorCode>::failure(
        {NumericalErrorCode::ResourceLimitExceeded,std::move(error)});
}
} // namespace egomotion::harmonics::detail
