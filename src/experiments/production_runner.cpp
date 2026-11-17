#include "egomotion/experiments/production_runner.hpp"

#include "egomotion/experiments/adaptive_runner.hpp"
#include "egomotion/experiments/bias_runner.hpp"
#include "egomotion/experiments/output_values.hpp"
#include "egomotion/experiments/run_context.hpp"
#include "egomotion/sampling/directions.hpp"
#include "egomotion/simulation/flow.hpp"

#include <cstddef>
#include <limits>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>

namespace egomotion::experiments {
namespace {
using Outcome=Result<RunOutcome,ExperimentErrorCode>;

Outcome failed(ExperimentErrorCode code,std::string message) {
    ErrorDetail d;d.stage=ErrorStage::Simulation;d.message=std::move(message);
    return Outcome::failure({code,std::move(d)});
}
template<class T>
Outcome propagate(const Result<T,ExperimentErrorCode>& child,
                  std::string message) {
    ErrorDetail d;d.stage=ErrorStage::Simulation;d.message=std::move(message);
    return Outcome::failure(wrapFailure(child.error().code,std::move(d),child.error()));
}
Outcome publish(const CurrentOutputBatch& outputs,RunContext& ctx,
                std::size_t source_repetitions,std::size_t valid_samples) {
    auto counted=ctx.noteCompletedMemory(source_repetitions,valid_samples);
    if(!counted.hasValue())return Outcome::failure(std::move(counted).error());
    auto written=ctx.writeCurrentResults(outputs);
    if(!written.hasValue())return Outcome::failure(std::move(written).error());
    return ctx.publishCompleteOutcome();
}
Outcome finishBias(const Result<BiasRunData,ExperimentErrorCode>& source,
                   RunContext& ctx,ExperimentKind kind) {
    if(!source.hasValue())return propagate(source,"Bias numerical computation failed");
    if(source.value().level_count >
       std::numeric_limits<std::size_t>::max()/3)
        return failed(ExperimentErrorCode::DataMismatch,
                      "Bias level count overflows source sample count");
    auto packed=makeBiasOutput(kind,source.value());
    if(!packed.hasValue())return propagate(packed,"Bias current-save encoding failed");
    return publish(packed.value(),ctx,1,3*source.value().level_count);
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
Outcome finishAdaptive(const Result<AdaptiveRunData,ExperimentErrorCode>& source,
                       RunContext& ctx,ExperimentKind kind) {
    if(!source.hasValue())return propagate(source,"Adaptive numerical computation failed");
    const auto& data=source.value();
    if(data.frame_count<3||data.completed_runs!=data.planned_runs||
       (data.frame_count-2)>
       std::numeric_limits<std::size_t>::max()/data.planned_runs)
        return failed(ExperimentErrorCode::IncompleteRun,
                      "Adaptive run/valid frame count is incomplete or overflowing");
    auto packed=makeAdaptiveOutput(kind,data);
    if(!packed.hasValue())return propagate(packed,"Adaptive current-save encoding failed");
    return publish(packed.value(),ctx,data.completed_runs,
                   (data.frame_count-2)*data.completed_runs);
}
}  // namespace

Result<RunOutcome,ExperimentErrorCode> ProductionExperimentRunner::runBiasOccluded(
    const BiasExperimentConfig& config,RunContext& context) {
    if(context.experimentKind()!=ExperimentKind::BiasOccluded)
        return failed(ExperimentErrorCode::InvalidParameters,
                      "Composition Root injected wrong Bias experiment kind");
    sampling::OccludedOctahedralSampler sampler;
    simulation::MeanMagnitudeGaussianNoise noise;
    const BiasExperimentRunner calculator(sampler,noise);
    context.noteAttempt();
    auto data=calculator.compute(config,[&context]() noexcept {
        context.noteValid();
    });
    if(data.hasValue())context.noteCompleted();
    return finishBias(data,context,ExperimentKind::BiasOccluded);
}
Result<RunOutcome,ExperimentErrorCode> ProductionExperimentRunner::runBiasProportional(
    const BiasExperimentConfig& config,RunContext& context) {
    if(context.experimentKind()!=ExperimentKind::BiasProportional)
        return failed(ExperimentErrorCode::InvalidParameters,
                      "Composition Root injected wrong Bias experiment kind");
    sampling::OctahedralSampler sampler;
    simulation::ComponentProportionalGaussianNoise noise;
    const BiasExperimentRunner calculator(sampler,noise);
    context.noteAttempt();
    auto data=calculator.compute(config,[&context]() noexcept {
        context.noteValid();
    });
    if(data.hasValue())context.noteCompleted();
    return finishBias(data,context,ExperimentKind::BiasProportional);
}
Result<RunOutcome,ExperimentErrorCode> ProductionExperimentRunner::runAdaptiveInitialization(
    const AdaptiveInitializationConfig& config,RunContext& context) {
    if(context.experimentKind()!=ExperimentKind::AdaptiveInitialization)
        return failed(ExperimentErrorCode::InvalidParameters,
                      "Composition Root injected wrong Adaptive kind");
    const AdaptiveExperimentRunner calculator(injected_solver_);
    return finishAdaptive(calculator.computeInitialization(config,progressFor(context)),context,
                          ExperimentKind::AdaptiveInitialization);
}
Result<RunOutcome,ExperimentErrorCode> ProductionExperimentRunner::runAdaptiveComparison(
    const AdaptiveComparisonConfig& config,RunContext& context) {
    if(context.experimentKind()!=ExperimentKind::AdaptiveComparison)
        return failed(ExperimentErrorCode::InvalidParameters,
                      "Composition Root injected wrong Adaptive kind");
    const AdaptiveExperimentRunner calculator(injected_solver_);
    return finishAdaptive(calculator.computeComparison(
                              config,context.randomSource(),progressFor(context)),
                          context,ExperimentKind::AdaptiveComparison);
}
Result<RunOutcome,ExperimentErrorCode> ProductionExperimentRunner::runAdaptiveUpdateRates(
    const AdaptiveUpdateRatesConfig& config,RunContext& context) {
    if(context.experimentKind()!=ExperimentKind::AdaptiveUpdateRates)
        return failed(ExperimentErrorCode::InvalidParameters,
                      "Composition Root injected wrong Adaptive kind");
    const AdaptiveExperimentRunner calculator(injected_solver_);
    return finishAdaptive(calculator.computeUpdateRates(config,progressFor(context)),context,
                          ExperimentKind::AdaptiveUpdateRates);
}

}  // namespace egomotion::experiments
