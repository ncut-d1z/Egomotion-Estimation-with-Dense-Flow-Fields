#pragma once

#include "egomotion/experiments/runner.hpp"

namespace egomotion::experiments {

// Pure business implementation behind the public narrow runner interface.
// It knows the source current-save sets but has no matio/dirfd dependency.
class ProductionExperimentRunner final : public IExperimentRunner {
public:
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
};

}  // namespace egomotion::experiments
