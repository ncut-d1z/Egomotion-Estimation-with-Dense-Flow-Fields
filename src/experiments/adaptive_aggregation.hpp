#pragma once

#include "egomotion/experiments/adaptive_runner.hpp"

namespace egomotion::experiments::detail {

// 私有流水线接缝：调用者仅可在整轮逐帧计算成功后追加。
// 与固定600行替身共用生产聚合逻辑，不替换生产轨迹或角度失败策略。
Status<ExperimentErrorCode> accumulateCompleteAdaptiveRun(
    AdaptiveRunData& aggregate, const AdaptiveRunData& one);
Result<AdaptiveRunData,ExperimentErrorCode> finishAdaptiveAggregation(
    AdaptiveRunData&& aggregate, bool comparison);

}  // namespace egomotion::experiments::detail
