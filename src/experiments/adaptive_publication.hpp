#pragma once

#include "egomotion/experiments/adaptive_runner.hpp"
#include "egomotion/experiments/output_contract.hpp"

namespace egomotion::experiments::detail {
// 生产与固定结果测试共用同一输出门禁；不是对CLI开放的新入口。
Result<RunOutcome,ExperimentErrorCode> finishAdaptive(
    const Result<AdaptiveRunData,ExperimentErrorCode>& source,
    RunContext& context,ExperimentKind kind);
}  // namespace egomotion::experiments::detail
