#pragma once

#include "egomotion/experiments/run_context.hpp"

#include <string>
#include <vector>

namespace egomotion::experiments {

// Independent of matio and Linux file descriptors. All evidence is
// defined in domain/types.hpp, not duplicated in the experiment layer.
Result<std::string,ExperimentErrorCode> makeCompleteManifestJson(
    const RunContext& run,const std::vector<FileEvidence>& published);

}  // namespace egomotion::experiments
