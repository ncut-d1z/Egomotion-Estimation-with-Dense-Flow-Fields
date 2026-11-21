#pragma once

#include "egomotion/experiments/run_context.hpp"

#include <string>
#include <vector>

namespace egomotion::experiments {

// Independent of matio and Linux file descriptors. All evidence is
// defined in domain/types.hpp, not duplicated in the experiment layer.
Result<std::string,ExperimentErrorCode> makeCompleteManifestJson(
    const RunContext& run,const std::vector<FileEvidence>& published);

// 完整验证内存结果后生成规范、全精度指标证据；NaN缺测只编码为JSON null。
Result<std::string,ExperimentErrorCode> makeAdaptiveMetricValidityJson(
    const RunContext& run,const AdaptiveRunData& source);

// v2 允许求解完整但统计部分有效的 Incomplete 成功结果，仍核验全部持久化回执。
Result<std::string,ExperimentErrorCode> makeAdaptiveManifestJson(
    const RunContext& run,const std::vector<FileEvidence>& published);

// Never labels a partial run Complete. Persists full outer->inner cause chain
// and all independently confirmed/uncertain output file identities.
Result<std::string,ExperimentErrorCode> makeFailureManifestJson(
    const RunContext& run,const std::vector<FileEvidence>& published);

}  // namespace egomotion::experiments
