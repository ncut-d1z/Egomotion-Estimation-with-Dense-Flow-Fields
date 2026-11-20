#pragma once

#include "egomotion/experiments/adaptive_runner.hpp"
#include "egomotion/experiments/output_contract.hpp"

namespace egomotion::experiments::detail {

enum class AdaptiveEvidencePhase { Accumulating, Finalized };

// 两个阶段共用原值/缺测原因/次数/帧账本规则。累计阶段只允许已完成的
// 原始轮次及其和；最终阶段必须完成计划轮次，并使用既定固定分母。
// 此内部验证不是发布许可；公开输出转换只使用 Finalized。
Status<ExperimentErrorCode> validateAdaptiveEvidence(
    ExperimentKind kind,const AdaptiveRunData& source,AdaptiveEvidencePhase phase);

}  // namespace egomotion::experiments::detail
