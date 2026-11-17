#pragma once

#include "egomotion/contracts/interfaces.hpp"
#include "egomotion/experiments/runner.hpp"

#include <cstddef>
#include <functional>
#include <map>
#include <string>

namespace egomotion::estimators { class IAdaptiveStepSolver; }

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

enum class AdaptiveProgressEvent { RunStarted, FrameCompleted, RunCompleted };
using AdaptiveProgress = std::function<void(AdaptiveProgressEvent)>;

// 借用服务的生命周期须覆盖Runner；Comparison之外不调用噪声模型。
// 生产装配使用IcosahedralSampler + MeanMagnitudeGaussianNoise，不能从接口猜类型。
struct AdaptiveSampling {
    const IDirectionSampler& sampler;
    const IFlowNoiseModel& comparison_noise;
};

class AdaptiveExperimentRunner {
public:
    // 可选借用同一求解器做跨帧合同测试；调用者须保证其生命周期。
    // 空指针保留真实 StepSolver，不改变生产帧数、采样级别或八次重复。
    AdaptiveExperimentRunner(AdaptiveSampling sampling,
        const estimators::IAdaptiveStepSolver* injected_solver=nullptr)
        : sampling_(sampling), injected_solver_(injected_solver) {}
    Result<AdaptiveRunData,ExperimentErrorCode> computeInitialization(
        const AdaptiveInitializationConfig& config = {},
        const AdaptiveProgress& progress = {}) const;

    // External RNG is not reseeded between the eight runs. In particular,
    // noise_scale=0 still consumes all the original nonpropfehler normals.
    Result<AdaptiveRunData,ExperimentErrorCode> computeComparison(
        const AdaptiveComparisonConfig& config, IRandomSource& rng,
        const AdaptiveProgress& progress = {}) const;

    Result<AdaptiveRunData,ExperimentErrorCode> computeUpdateRates(
        const AdaptiveUpdateRatesConfig& config = {},
        const AdaptiveProgress& progress = {}) const;
private:
    AdaptiveSampling sampling_;
    const estimators::IAdaptiveStepSolver* injected_solver_;
};

}  // namespace egomotion::experiments
