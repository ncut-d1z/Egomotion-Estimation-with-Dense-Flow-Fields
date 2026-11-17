#include "egomotion/experiments/production_runner.hpp"
#include "adaptive_publication.hpp"

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

Outcome failed(const RunContext& ctx,ExperimentErrorCode code,std::string message) {
    ErrorDetail d;d.stage=ErrorStage::Simulation;d.message=std::move(message);
    d.run_id=ctx.runId();
    return Outcome::failure({code,std::move(d)});
}
template<class T>
Outcome propagate(const Result<T,ExperimentErrorCode>& child,
                  const RunContext& ctx,std::string message) {
    ErrorDetail d;d.stage=ErrorStage::Simulation;d.message=std::move(message);
    d.run_id=ctx.runId();
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
    if(!source.hasValue())return propagate(source,ctx,"Bias numerical computation failed");
    if(source.value().level_count >
       std::numeric_limits<std::size_t>::max()/3)
        return failed(ctx,ExperimentErrorCode::DataMismatch,
                      "Bias level count overflows source sample count");
    auto packed=makeBiasOutput(kind,source.value());
    if(!packed.hasValue())return propagate(packed,ctx,"Bias current-save encoding failed");
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
}  // namespace

Outcome detail::finishAdaptive(const Result<AdaptiveRunData,ExperimentErrorCode>& source,
                       RunContext& ctx,ExperimentKind kind) {
    if(!source.hasValue()){
        // 终止本轨迹的无定义角也是实际无效样本，不能只留在错误链而漏记计数。
        for(const auto& cause:source.error().detail.causes){
            if(cause.domain==errorDomain<NumericalErrorCode>()&&
               cause.code==errorName(NumericalErrorCode::UndefinedAngle)){
                ctx.noteUndefined();
                break;
            }
        }
        return propagate(source,ctx,"Adaptive numerical computation failed");
    }
    const auto& data=source.value();
    if(data.frame_count<3||data.planned_runs==0||data.completed_runs!=data.planned_runs||
       (data.frame_count-2)>
       std::numeric_limits<std::size_t>::max()/data.planned_runs)
        return failed(ctx,ExperimentErrorCode::IncompleteRun,
                      "Adaptive run/valid frame count is incomplete or overflowing");
    auto packed=makeAdaptiveOutput(kind,data);
    if(!packed.hasValue())return propagate(packed,ctx,"Adaptive current-save encoding failed");
    return publish(packed.value(),ctx,data.completed_runs,
                   (data.frame_count-2)*data.completed_runs);
}

Result<RunOutcome,ExperimentErrorCode> ProductionExperimentRunner::runBiasOccluded(
    const BiasExperimentConfig& config,RunContext& context) {
    if(context.experimentKind()!=ExperimentKind::BiasOccluded)
        return failed(context,ExperimentErrorCode::InvalidParameters,
                      "Composition Root injected wrong Bias experiment kind");
    sampling::OccludedOctahedralSampler sampler;
    simulation::MeanMagnitudeGaussianNoise noise;
    const BiasExperimentRunner calculator(sampler,noise,biasEstimators());
    context.noteAttempt();
    auto data=calculator.compute(config,context.randomSource(),[&context]() noexcept {
        context.noteValid();
    });
    if(data.hasValue())context.noteCompleted();
    return finishBias(data,context,ExperimentKind::BiasOccluded);
}
Result<RunOutcome,ExperimentErrorCode> ProductionExperimentRunner::runBiasProportional(
    const BiasExperimentConfig& config,RunContext& context) {
    if(context.experimentKind()!=ExperimentKind::BiasProportional)
        return failed(context,ExperimentErrorCode::InvalidParameters,
                      "Composition Root injected wrong Bias experiment kind");
    sampling::OctahedralSampler sampler;
    simulation::ComponentProportionalGaussianNoise noise;
    const BiasExperimentRunner calculator(sampler,noise,biasEstimators());
    context.noteAttempt();
    auto data=calculator.compute(config,context.randomSource(),[&context]() noexcept {
        context.noteValid();
    });
    if(data.hasValue())context.noteCompleted();
    return finishBias(data,context,ExperimentKind::BiasProportional);
}
Result<RunOutcome,ExperimentErrorCode> ProductionExperimentRunner::runAdaptiveInitialization(
    const AdaptiveInitializationConfig& config,RunContext& context) {
    if(context.experimentKind()!=ExperimentKind::AdaptiveInitialization)
        return failed(context,ExperimentErrorCode::InvalidParameters,
                      "Composition Root injected wrong Adaptive kind");
    const AdaptiveExperimentRunner calculator(adaptiveSampling(),injected_solver_);
    return detail::finishAdaptive(calculator.computeInitialization(config,progressFor(context)),context,
                          ExperimentKind::AdaptiveInitialization);
}
Result<RunOutcome,ExperimentErrorCode> ProductionExperimentRunner::runAdaptiveComparison(
    const AdaptiveComparisonConfig& config,RunContext& context) {
    if(context.experimentKind()!=ExperimentKind::AdaptiveComparison)
        return failed(context,ExperimentErrorCode::InvalidParameters,
                      "Composition Root injected wrong Adaptive kind");
    const AdaptiveExperimentRunner calculator(adaptiveSampling(),injected_solver_);
    return detail::finishAdaptive(calculator.computeComparison(
                              config,context.randomSource(),progressFor(context)),
                          context,ExperimentKind::AdaptiveComparison);
}
Result<RunOutcome,ExperimentErrorCode> ProductionExperimentRunner::runAdaptiveUpdateRates(
    const AdaptiveUpdateRatesConfig& config,RunContext& context) {
    if(context.experimentKind()!=ExperimentKind::AdaptiveUpdateRates)
        return failed(context,ExperimentErrorCode::InvalidParameters,
                      "Composition Root injected wrong Adaptive kind");
    const AdaptiveExperimentRunner calculator(adaptiveSampling(),injected_solver_);
    return detail::finishAdaptive(calculator.computeUpdateRates(config,progressFor(context)),context,
                          ExperimentKind::AdaptiveUpdateRates);
}

}  // namespace egomotion::experiments
