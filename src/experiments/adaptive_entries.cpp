#include "egomotion/experiments/adaptive_runner.hpp"
#include "egomotion/experiments/run_context.hpp"
#include "adaptive_publication.hpp"

#include <string>
#include <utility>

namespace egomotion::experiments {
namespace {
Result<RunOutcome,ExperimentErrorCode> failed(
    const RunContext& ctx,ExperimentErrorCode code,std::string message) {
    ErrorDetail d;d.stage=ErrorStage::Simulation;d.message=std::move(message);
    d.run_id=ctx.runId();
    return Result<RunOutcome,ExperimentErrorCode>::failure({code,std::move(d)});
}
AdaptiveProgress progressFor(RunContext& ctx) {
    return [&ctx](AdaptiveProgressEvent event) noexcept {
        switch(event) {
        case AdaptiveProgressEvent::RunStarted:ctx.noteAttempt();break;
        case AdaptiveProgressEvent::FrameCompleted:ctx.noteValid();break;
        case AdaptiveProgressEvent::RunCompleted:ctx.noteCompleted();break;
        }
    };
}
}  // namespace

Result<RunOutcome,ExperimentErrorCode> InitializationRunner::run(
    const AdaptiveInitializationConfig& config,RunContext& context) const {
    if(context.experimentKind()!=ExperimentKind::AdaptiveInitialization)
        return failed(context,ExperimentErrorCode::InvalidParameters,
                      "Composition Root injected wrong Adaptive kind");
    return detail::finishAdaptive(compute(config,progressFor(context)),context,
                          ExperimentKind::AdaptiveInitialization);
}
Result<RunOutcome,ExperimentErrorCode> AdaptiveComparisonRunner::run(
    const AdaptiveComparisonConfig& config,RunContext& context) const {
    if(context.experimentKind()!=ExperimentKind::AdaptiveComparison)
        return failed(context,ExperimentErrorCode::InvalidParameters,
                      "Composition Root injected wrong Adaptive kind");
    return detail::finishAdaptive(compute(
                              config,context.randomSource(),progressFor(context)),
                          context,ExperimentKind::AdaptiveComparison);
}
Result<RunOutcome,ExperimentErrorCode> UpdateRateRunner::run(
    const AdaptiveUpdateRatesConfig& config,RunContext& context) const {
    if(context.experimentKind()!=ExperimentKind::AdaptiveUpdateRates)
        return failed(context,ExperimentErrorCode::InvalidParameters,
                      "Composition Root injected wrong Adaptive kind");
    return detail::finishAdaptive(compute(config,progressFor(context)),context,
                          ExperimentKind::AdaptiveUpdateRates);
}

}  // namespace egomotion::experiments
