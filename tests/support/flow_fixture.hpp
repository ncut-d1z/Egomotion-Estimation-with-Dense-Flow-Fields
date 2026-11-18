#pragma once

#include "egomotion/contracts/result.hpp"

#include <stdexcept>
#include <utility>

namespace egomotion::test_support {
inline FlowField flowValue(const FlowMatrix& observations) {
    auto result=FlowField::create(observations);
    if(!result.hasValue())throw std::runtime_error("Invalid finite flow fixture");
    return std::move(result).value();
}
} // namespace egomotion::test_support
