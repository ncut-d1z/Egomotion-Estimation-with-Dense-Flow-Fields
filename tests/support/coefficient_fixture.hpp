#pragma once

#include "egomotion/contracts/result.hpp"

#include <stdexcept>
#include <utility>

namespace egomotion::test_support {
inline HarmonicCoefficients coefficientValue(const HarmonicMatrix& values) {
    auto result=HarmonicCoefficients::create(values);
    if(!result.hasValue())throw std::runtime_error("Invalid finite harmonic fixture");
    return std::move(result).value();
}
}  // namespace egomotion::test_support
