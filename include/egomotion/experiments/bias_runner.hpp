#pragma once

#include "egomotion/experiments/runner.hpp"

#include <array>
#include <cstddef>
#include <functional>
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
    // Composition Root must inject the EXACT one permitted pair:
    // OccludedOctahedralSampler + MeanMagnitudeGaussianNoise OR
    // OctahedralSampler + ComponentProportionalGaussianNoise.
    // There is no runtime kind() or dual-noise mode.
    BiasExperimentRunner(const IDirectionSampler& sampler,
                         const IFlowNoiseModel& noise) noexcept
        : sampler_(sampler), noise_(noise) {}

    Result<BiasRunData, ExperimentErrorCode> compute(
        const BiasExperimentConfig& config,
        const std::function<void()>& sample_completed={}) const;

private:
    const IDirectionSampler& sampler_;
    const IFlowNoiseModel& noise_;
};

}  // namespace egomotion::experiments
