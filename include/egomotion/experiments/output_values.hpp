#pragma once

#include "egomotion/experiments/output_contract.hpp"
#include "egomotion/experiments/bias_runner.hpp"
#include "egomotion/experiments/adaptive_runner.hpp"

namespace egomotion::experiments {

using CurrentOutputBatch = std::map<std::string,DenseDoubleArray>;

// R5 adapter: translate R3's seven owning Bias arrays to the same narrow
// IResultWriter value type used by R4, without introducing a matio dependency.
Result<CurrentOutputBatch,ExperimentErrorCode> makeBiasOutput(
    ExperimentKind kind, const BiasRunData& source);

// Only a fully completed Adaptive memory result can be packaged as the
// current save set. RunContext remains responsible for file/Manifest status.
Result<CurrentOutputBatch,ExperimentErrorCode> makeAdaptiveOutput(
    ExperimentKind kind, const AdaptiveRunData& source);

}  // namespace egomotion::experiments
