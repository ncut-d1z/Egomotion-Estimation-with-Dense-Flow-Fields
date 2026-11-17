#pragma once

#include "egomotion/estimators/adaptive_step.hpp"

namespace egomotion::estimators::detail {

// 仅内部数值阶段：尚未执行 koeff_aus_near，不能作为完整 StepResult 提交。
// trace.coefficients_next 不在此阶段赋值；完整公共入口始终继续投影。
struct AdaptiveMotionResult {
    MotionEstimate motion;
    Scalar error_mse=0;
    AdaptiveStepTrace trace;
};

Result<AdaptiveMotionResult,NumericalErrorCode> solveAdaptiveMotion(
    const HarmonicCoefficients& old,const FlowField& observations);

}  // namespace egomotion::estimators::detail
