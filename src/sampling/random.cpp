#include "egomotion/sampling/random.hpp"

#include <cmath>
#include <utility>

namespace egomotion::sampling {
namespace {
Result<Scalar, NumericalErrorCode> invalid(std::string message) {
    ErrorDetail detail;
    detail.stage = ErrorStage::Simulation;
    detail.message = std::move(message);
    return Result<Scalar, NumericalErrorCode>::failure({
        NumericalErrorCode::InvalidRandomSample, std::move(detail)});
}
}  // namespace

Result<Scalar, NumericalErrorCode> Mt19937RandomSource::uniform() {
    const double value = uniform_distribution_(generator_);
    if (!std::isfinite(value) || value < 0.0 || value >= 1.0)
        return invalid("uniform random sample must be finite in [0,1)");
    return Result<Scalar, NumericalErrorCode>::success(value);
}

Result<Scalar, NumericalErrorCode> Mt19937RandomSource::normal() {
    const double value = normal_distribution_(generator_);
    if (!std::isfinite(value))
        return invalid("normal random sample must be finite");
    return Result<Scalar, NumericalErrorCode>::success(value);
}

}  // namespace egomotion::sampling
