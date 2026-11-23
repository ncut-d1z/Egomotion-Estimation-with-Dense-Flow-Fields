#pragma once

#include "egomotion/contracts/result.hpp"

#include <stdexcept>
#include <utility>

namespace egomotion::test_support {
inline DirectionSet directionValue(const DirectionMatrix& directions) {
    auto result=DirectionSet::create(directions);
    if(!result.hasValue())throw std::runtime_error("Invalid finite direction fixture");
    return std::move(result).value();
}
} // namespace egomotion::test_support
