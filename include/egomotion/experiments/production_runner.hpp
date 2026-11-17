#pragma once

#include "egomotion/experiments/runner.hpp"
#include "egomotion/experiments/bias_runner.hpp"

#include <optional>

namespace egomotion::estimators { class IAdaptiveStepSolver; }

namespace egomotion::experiments {

// Pure business implementation behind the public narrow runner interface.
// It knows the source current-save sets but has no matio/dirfd dependency.
class ProductionExperimentRunner final : public IExperimentRunner {
public:
    // 注入只用于有确定预期值的合同测试，CLI 默认构造仍选真实数值求解器。
    explicit ProductionExperimentRunner(
        const estimators::IAdaptiveStepSolver* injected_solver=nullptr)
        : injected_solver_(injected_solver) {}
    // 求解服务借用到调用结束；RNG始终使用每个RunContext独占的实例。
    explicit ProductionExperimentRunner(BiasEstimators bias_estimators,
        const estimators::IAdaptiveStepSolver* injected_solver=nullptr)
        : injected_solver_(injected_solver), injected_bias_(bias_estimators) {}
    Result<RunOutcome,ExperimentErrorCode> runBiasOccluded(
        const BiasExperimentConfig&,RunContext&) override;
    Result<RunOutcome,ExperimentErrorCode> runBiasProportional(
        const BiasExperimentConfig&,RunContext&) override;
    Result<RunOutcome,ExperimentErrorCode> runAdaptiveInitialization(
        const AdaptiveInitializationConfig&,RunContext&) override;
    Result<RunOutcome,ExperimentErrorCode> runAdaptiveComparison(
        const AdaptiveComparisonConfig&,RunContext&) override;
    Result<RunOutcome,ExperimentErrorCode> runAdaptiveUpdateRates(
        const AdaptiveUpdateRatesConfig&,RunContext&) override;
private:
    BiasEstimators biasEstimators() const noexcept {
        if(injected_bias_)return *injected_bias_;
        return {franz_,original_,modified_};
    }
    const estimators::IAdaptiveStepSolver* injected_solver_;
    std::optional<BiasEstimators> injected_bias_;
    estimators::FranzMatchedFilter franz_;
    estimators::OriginalKvDEstimator original_;
    estimators::ModifiedKvDEstimator modified_;
};

}  // namespace egomotion::experiments
