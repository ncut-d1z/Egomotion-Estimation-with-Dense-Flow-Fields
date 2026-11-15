#pragma once

#include "egomotion/experiments/runner.hpp"

#include <array>
#include <cstddef>
#include <vector>

namespace egomotion::experiments {

// Owns exactly the seven current MATLAB result matrices. Each ywerte array
// has shape (levels x 3), flattened in MATLAB column-major order.
// No MAT file/RunContext or fake RunOutcome is created in R3.
struct BiasRunData {
    std::vector<double> xwerte;
    std::array<std::vector<double>, 6> ywerte;
    std::array<double, 3> noise_coefficients{{0.6, 2.4, 9.6}};
    std::size_t level_count = 0;
};

class BiasExperimentRunner {
public:
    Result<BiasRunData, ExperimentErrorCode> computeOccluded(
        const BiasExperimentConfig& config) const;
    Result<BiasRunData, ExperimentErrorCode> computeProportional(
        const BiasExperimentConfig& config) const;
};

}  // namespace egomotion::experiments
