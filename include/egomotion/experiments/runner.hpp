#pragma once

#include "egomotion/contracts/interfaces.hpp"

#include <cstdint>
#include <memory>

namespace egomotion {

struct BiasExperimentConfig {
    std::uint32_t seed;
    int maximum_subdivision_level;
};
struct AdaptiveInitializationConfig {};
struct AdaptiveComparisonConfig { Scalar noise_scale; };
struct AdaptiveUpdateRatesConfig {};

// R1 only: a single session owns RNG/output state; the concrete constructor,
// RunId generation and publication are introduced with R3/R5.
class RunContext;

class IExperimentRunner {
public:
    virtual ~IExperimentRunner() = default;
    virtual Result<RunOutcome, ExperimentErrorCode> runBiasOccluded(
        const BiasExperimentConfig&, RunContext&) = 0;
    virtual Result<RunOutcome, ExperimentErrorCode> runBiasProportional(
        const BiasExperimentConfig&, RunContext&) = 0;
    virtual Result<RunOutcome, ExperimentErrorCode> runAdaptiveInitialization(
        const AdaptiveInitializationConfig&, RunContext&) = 0;
    virtual Result<RunOutcome, ExperimentErrorCode> runAdaptiveComparison(
        const AdaptiveComparisonConfig&, RunContext&) = 0;
    virtual Result<RunOutcome, ExperimentErrorCode> runAdaptiveUpdateRates(
        const AdaptiveUpdateRatesConfig&, RunContext&) = 0;
};

}  // namespace egomotion
