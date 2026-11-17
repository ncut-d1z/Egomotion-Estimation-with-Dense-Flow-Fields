#pragma once

#include "egomotion/experiments/runner.hpp"
#include "egomotion/estimators/franz_kvd.hpp"

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

// 借用的无状态求解服务必须活得比Runner长；两个KvD槽保留独立算法身份。
struct BiasEstimators {
    const estimators::IFranzMatchedFilter& franz;
    const estimators::IKvDEstimator& original;
    const estimators::IKvDEstimator& modified;
};

class BiasExperimentRunner {
public:
    // Composition Root must inject the EXACT one permitted pair:
    // OccludedOctahedralSampler + MeanMagnitudeGaussianNoise OR
    // OctahedralSampler + ComponentProportionalGaussianNoise.
    // There is no runtime kind() or dual-noise mode.
    BiasExperimentRunner(const IDirectionSampler& sampler,
                         const IFlowNoiseModel& noise,
                         BiasEstimators estimators) noexcept
        : sampler_(sampler), noise_(noise), estimators_(estimators) {}

    Result<BiasRunData, ExperimentErrorCode> compute(
        const BiasExperimentConfig& config, IRandomSource& random,
        const std::function<void()>& sample_completed={}) const;

private:
    const IDirectionSampler& sampler_;
    const IFlowNoiseModel& noise_;
    BiasEstimators estimators_;
};

}  // namespace egomotion::experiments
