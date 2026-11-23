#pragma once

#include "egomotion/contracts/interfaces.hpp"
#include "egomotion/experiments/runner.hpp"
#include "egomotion/experiments/paper_observations.hpp"
#include "egomotion/trajectory/trajectory.hpp"

#include <cstddef>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace egomotion::estimators { class IAdaptiveStepSolver; }

namespace egomotion::experiments {

struct AdaptiveUndefinedMetric {
    std::string variable;
    std::size_t matlab_run_index = 0;
    std::size_t matlab_frame_index = 0;
    std::size_t matlab_column_index = 0;
    Failure<NumericalErrorCode> reason;
};

// 原字段保持列主序；方案B以NaN表示已声明缺测，并保留每轮原值与原因。
// valid_counts按单元计有效重复次数，首尾预分配零为结构端点，次数为0。
// 八轮误差只有8次全有效才提供均值；rottransratio仍仅取最后一轮。
struct AdaptiveRunData {
    std::map<std::string, DenseDoubleArray> fields;
    std::map<std::string, std::vector<std::size_t>> valid_counts;
    std::vector<AdaptiveUndefinedMetric> undefined_metrics;
    std::vector<std::map<std::string, DenseDoubleArray>> repetition_fields;
    std::size_t frame_count = 0;
    std::size_t planned_runs = 0;
    std::size_t completed_runs = 0;
    std::size_t valid_frames = 0;
    std::size_t undefined_frames = 0;
};

enum class AdaptiveProgressEvent {
    RunStarted, FrameCompleted, FrameWithUndefinedMetrics, RunCompleted
};
using AdaptiveProgress = std::function<void(AdaptiveProgressEvent)>;

// 借用服务的生命周期须覆盖Runner；Comparison之外不调用噪声模型。
// 生产装配使用IcosahedralSampler + MeanMagnitudeGaussianNoise，不能从接口猜类型。
struct AdaptiveSampling {
    const IDirectionSampler& sampler;
    const IFlowNoiseModel& comparison_noise;
};

// 场景服务只返回自拥有值，不接触Session/RunContext；引用须长于Runner。
struct AdaptiveScene {
    const trajectory::ITrajectoryGenerator& trajectory;
    const trajectory::ITrajectoryKinematics& kinematics;
    const trajectory::ISphereEnvironment& environment;
};

// 每入口只接受自己的Config；run借用本次RunContext，compute不发布结果。
// Session及聚合缓冲在每次compute内部创建，Runner不持有跨运行可变状态。
class InitializationRunner final {
public:
    InitializationRunner(AdaptiveSampling sampling, AdaptiveScene scene,
        const estimators::IAdaptiveStepSolver* injected_solver=nullptr)
        : sampling_(sampling), scene_(scene), injected_solver_(injected_solver) {}
    Result<AdaptiveRunData,ExperimentErrorCode> compute(
        const AdaptiveInitializationConfig& config = {},
        const AdaptiveProgress& progress = {},const AdaptivePaperObserver& paper = {}) const;
    Result<RunOutcome,ExperimentErrorCode> run(
        const AdaptiveInitializationConfig&,RunContext&) const;
private:
    AdaptiveSampling sampling_;
    AdaptiveScene scene_;
    const estimators::IAdaptiveStepSolver* injected_solver_;
};

class AdaptiveComparisonRunner final {
public:
    AdaptiveComparisonRunner(AdaptiveSampling sampling, AdaptiveScene scene,
        const estimators::IAdaptiveStepSolver* injected_solver=nullptr)
        : sampling_(sampling), scene_(scene), injected_solver_(injected_solver) {}
    // 八轮共用本次外部RNG，不重新播种；noise_scale=0仍消耗原normal序列。
    Result<AdaptiveRunData,ExperimentErrorCode> compute(
        const AdaptiveComparisonConfig&,IRandomSource&,
        const AdaptiveProgress& progress = {},const AdaptivePaperObserver& paper = {}) const;
    Result<RunOutcome,ExperimentErrorCode> run(
        const AdaptiveComparisonConfig&,RunContext&) const;
private:
    AdaptiveSampling sampling_;
    AdaptiveScene scene_;
    const estimators::IAdaptiveStepSolver* injected_solver_;
};

class UpdateRateRunner final {
public:
    UpdateRateRunner(AdaptiveSampling sampling, AdaptiveScene scene,
        const estimators::IAdaptiveStepSolver* injected_solver=nullptr)
        : sampling_(sampling), scene_(scene), injected_solver_(injected_solver) {}
    Result<AdaptiveRunData,ExperimentErrorCode> compute(
        const AdaptiveUpdateRatesConfig& config = {},
        const AdaptiveProgress& progress = {}) const;
    Result<RunOutcome,ExperimentErrorCode> run(
        const AdaptiveUpdateRatesConfig&,RunContext&) const;
    // 专项分析只交付带周期身份的样本，不能误送入当前归档 MAT 发布接口。
    Status<ExperimentErrorCode> observePaperSchedule(
        PaperUpdateSchedule,const AdaptivePaperObserver&) const;
    Status<ExperimentErrorCode> observePaperSchedule(
        PaperUpdateSchedule,const AdaptivePaperObserver&,const AdaptiveProgress&) const;
private:
    AdaptiveSampling sampling_;
    AdaptiveScene scene_;
    const estimators::IAdaptiveStepSolver* injected_solver_;
};

// 旧内存调用面保留为兼容分派；生产装配直接选择各具名Runner。
class AdaptiveExperimentRunner final {
public:
    // 可选借用同一求解器做跨帧合同测试；调用者须保证其生命周期。
    // 空指针保留真实 StepSolver，不改变生产帧数、采样级别或八次重复。
    AdaptiveExperimentRunner(AdaptiveSampling sampling, AdaptiveScene scene,
        const estimators::IAdaptiveStepSolver* injected_solver=nullptr)
        : initialization_(sampling,scene,injected_solver),
          comparison_(sampling,scene,injected_solver),
          rates_(sampling,scene,injected_solver) {}
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
    InitializationRunner initialization_;
    AdaptiveComparisonRunner comparison_;
    UpdateRateRunner rates_;
};

}  // namespace egomotion::experiments
