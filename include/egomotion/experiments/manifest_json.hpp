#pragma once

#include "egomotion/experiments/run_context.hpp"

#include <string>
#include <vector>

namespace egomotion::experiments {

// Independent of matio and Linux file descriptors: the application passes
// only confirmed file evidence. Different from a filesystem operation.
struct FileEvidence {
    std::string variable;
    std::string filename;
    std::string sha256;
    std::size_t size_bytes=0;
    bool durably_confirmed=false;
};
Result<std::string,ExperimentErrorCode> makeCompleteManifestJson(
    const RunContext& run,const std::vector<FileEvidence>& published);

// Call ONLY after RunManifestPublisher::publish has returned success.
// No empty or partial output may be represented as RunOutcome::Complete.
Result<RunOutcome,ExperimentErrorCode> completedOutcomeAfterManifest(
    const RunContext& run);

}  // namespace egomotion::experiments
