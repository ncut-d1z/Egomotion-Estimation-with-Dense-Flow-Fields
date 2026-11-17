#pragma once

#include "egomotion/experiments/runner.hpp"

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
    const estimators::IAdaptiveStepSolver* injected_solver_;
};

}  // namespace egomotion::experiments
