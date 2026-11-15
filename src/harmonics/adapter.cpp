#include "egomotion/harmonics/adapter.hpp"
#include "egomotion/harmonics/low_order.hpp"

#include <new>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace egomotion::harmonics {
namespace {
std::string_view sourceName(low_order::ErrorCode code) {
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
NumericalErrorCode numericalCode(low_order::ErrorCode code) {
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
}  // namespace

Result<Scalar,NumericalErrorCode> reconstructNear(
    const HarmonicCoefficients& coeff, const Vector3& direction) {
    auto converted=low_order::fromMatlabLayout(coeff.values);
    if (!converted.hasValue())
        return bridge<Scalar>(converted.error(),"Near reconstruction layout");
    auto scalar=low_order::reconstruct(converted.value(),direction);
    if (!scalar.hasValue())
        return bridge<Scalar>(scalar.error(),"Near reconstruction");
    return Result<Scalar,NumericalErrorCode>::success(std::move(scalar).value());
}

Result<HarmonicCoefficients,NumericalErrorCode> fitSourceNear(
    const Eigen::VectorXd& near, const FlowField& field) {
    if (near.size()!=field.observations.rows() || near.size()==0) {
        ErrorDetail error;
        error.stage=ErrorStage::Solve;
        error.message="SH source fitting requires one near per observed ray";
        return Result<HarmonicCoefficients,NumericalErrorCode>::failure(
            {NumericalErrorCode::InvalidShape,std::move(error)});
    }
    try {
        // Original koeff_aus_near.m consumes N×6 and only its first 3 columns.
        // The pre-existing leaf takes an owning dynamic N×K matrix.
        const Eigen::MatrixXd points=field.observations;
        auto fitted=low_order::projectSourceCoefficientsOrderTwo(near,points);
        if (!fitted.hasValue())
            return bridge<HarmonicCoefficients>(fitted.error(),"SH source fit");
        HarmonicCoefficients result;
        result.values=std::move(fitted).value();
        return Result<HarmonicCoefficients,NumericalErrorCode>::success(
            std::move(result));
    } catch (const std::bad_alloc&) {
        return resourceFailure<HarmonicCoefficients>();
    } catch (const std::length_error&) {
        return resourceFailure<HarmonicCoefficients>();
    }
}

Result<HarmonicCoefficients,NumericalErrorCode> rotateCoefficients(
    const HarmonicCoefficients& coeff, const Matrix3& argument_rotation) {
    auto converted=low_order::fromMatlabLayout(coeff.values);
    if (!converted.hasValue())
        return bridge<HarmonicCoefficients>(converted.error(),"SH rotation layout");
    auto rotated=low_order::rotate(converted.value(),argument_rotation,
                                    low_order::RotationMethod::Ivanic1996);
    if (!rotated.hasValue())
        return bridge<HarmonicCoefficients>(rotated.error(),"SH rotation");
    auto returned=low_order::toMatlabLayout(rotated.value());
    if (!returned.hasValue())
        return bridge<HarmonicCoefficients>(returned.error(),"SH output layout");
    HarmonicCoefficients result;
    result.values=std::move(returned).value();
    return Result<HarmonicCoefficients,NumericalErrorCode>::success(std::move(result));
}

}  // namespace egomotion::harmonics
