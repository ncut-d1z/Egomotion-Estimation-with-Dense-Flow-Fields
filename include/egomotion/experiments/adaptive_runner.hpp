#pragma once

#include "egomotion/contracts/interfaces.hpp"
#include "egomotion/experiments/runner.hpp"

#include <cstddef>
#include <map>
#include <string>

namespace egomotion::experiments {

// The memory-only R4 result contains exactly the current MATLAB 'save'
// variables. All values own column-major dense data and have unique names.
// R5 will be responsible for RunContext, MAT Writer and Manifest persistence.
struct AdaptiveRunData {
    std::map<std::string, DenseDoubleArray> fields;
    std::size_t frame_count = 0;
    std::size_t planned_runs = 0;
    std::size_t completed_runs = 0;
};

class AdaptiveExperimentRunner {
public:
    Result<AdaptiveRunData,ExperimentErrorCode> computeInitialization(
        const AdaptiveInitializationConfig& config = {}) const;

    // External RNG is not reseeded between the eight runs. In particular,
    // noise_scale=0 still consumes all the original nonpropfehler normals.
    Result<AdaptiveRunData,ExperimentErrorCode> computeComparison(
        const AdaptiveComparisonConfig& config, IRandomSource& rng) const;

    Result<AdaptiveRunData,ExperimentErrorCode> computeUpdateRates(
        const AdaptiveUpdateRatesConfig& config = {}) const;
};

}  // namespace egomotion::experiments
