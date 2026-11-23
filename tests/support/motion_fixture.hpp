#pragma once

#include "egomotion/contracts/result.hpp"

#include <stdexcept>
#include <utility>

namespace egomotion::test_support {
// 固定合法夹具也显式核验工厂状态，不能从失败Result读取默认运动。
inline MotionEstimate motionValue(const Vector3& translation,const Vector3& rotation) {
    auto result=MotionEstimate::create(translation,rotation);
    if(!result.hasValue())throw std::runtime_error("Invalid finite motion fixture");
    return std::move(result).value();
}
}  // namespace egomotion::test_support
