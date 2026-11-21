#include "egomotion/experiments/bias_runner.hpp"
#include "../numerics/source_motion_errors.hpp"

#include "egomotion/sampling/directions.hpp"
#include "egomotion/simulation/flow.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <memory>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace egomotion::experiments {
namespace {
using ErrorResult = Result<BiasRunData,ExperimentErrorCode>;

ErrorResult invalid(std::string message) {
    ErrorDetail detail;
    detail.stage=ErrorStage::Parse;
    detail.message=std::move(message);
    return ErrorResult::failure({ExperimentErrorCode::InvalidParameters, std::move(detail)});
}
template<class NumericalResult>
ErrorResult fromNumerical(const NumericalResult& failed,
                       ExperimentErrorCode outer_code,
                       std::string message, std::size_t level,
                       std::optional<std::size_t> coefficient_column=std::nullopt) {
    ErrorDetail outer;
    outer.stage=ErrorStage::Simulation;
    // A subdivision level or 1-based MATLAB noise column is NOT a direction
    // observation index. Keep the leaf's actual observation_index unchanged.
    outer.message=std::move(message)+" (subdivision_level="+
                  std::to_string(level);
    if (coefficient_column)
        outer.message+=", noise_column_matlab_1_based="+
                       std::to_string(*coefficient_column+1);
    outer.message+=")";
    return ErrorResult::failure(wrapFailure(
        outer_code, std::move(outer), failed.error()));
}

ErrorResult computeImpl(const BiasExperimentConfig& config,
                        const IDirectionSampler& sampler,
                        const IFlowNoiseModel& noise,
                        const BiasEstimators& estimators, IRandomSource& rng,
                        const std::function<void()>& sample_completed,
                        const BiasPaperObserver& paper_observer) {
    // 保留源脚本的单一嵌套循环及就地失败边界，避免拆分后传递真值/列/层/进度状态。
    // One sample for each (noise coefficient, subdivision level); source
    // uses 3 noise coefficients in this order, then levels 0..maximum.
    if (config.maximum_subdivision_level<0)
        return invalid("Bias maximum subdivision level is negative");
    const std::size_t levels=
        static_cast<std::size_t>(config.maximum_subdivision_level)+1;
    if (levels>std::numeric_limits<std::size_t>::max()/3)
        return invalid("Bias output shape overflows size_t");
    try {
        auto initialized=rng.reseed(config.seed);
        if(!initialized.hasValue())return fromNumerical(initialized,ExperimentErrorCode::IncompleteRun,
            "Bias initial RNG reseed failed",0);
        auto truth_t=sampling::randomUnitDirection(rng);
        if (!truth_t.hasValue()) return fromNumerical(truth_t,ExperimentErrorCode::IncompleteRun,
            "Bias truth translation RNG failed",0);
        auto truth_w=sampling::randomUnitDirection(rng);
        if (!truth_w.hasValue()) return fromNumerical(truth_w,ExperimentErrorCode::IncompleteRun,
            "Bias truth rotation RNG failed",0);
        const Vector3 t=std::move(truth_t).value(), w=std::move(truth_w).value();

        // The source calls RandStream.setDefaultStream(seed) a SECOND time
        // after drawing the two fixed truth directions.
        auto restarted=rng.reseed(config.seed);
        if(!restarted.hasValue())return fromNumerical(restarted,ExperimentErrorCode::IncompleteRun,
            "Bias post-truth RNG reseed failed",0);
        BiasRunData data;
        data.level_count=levels;
        data.xwerte.assign(levels,0.0);
        for (auto& y:data.ywerte) y.assign(levels*3,0.0);
        // Caller/Composition Root owns the single injected sampler/noise pair.
        // No duplicate concrete model is created or selected in the Runner.
        const simulation::FlowSynthesizer synthesizer;
        const estimators::KvDParameters kvd;
        // Both Bias scripts explicitly store 8*4^i*(6/8), EVEN when the
        // proportional experiment actually uses 8*4^i directions.
        std::size_t x_count=6;
        for (std::size_t v=0;v<data.noise_coefficients.size();++v) {
            x_count=6;
            for (std::size_t level=0;level<levels;++level) {
                data.xwerte[level]=static_cast<double>(x_count);
                simulation::BiasFlowParameters params;
                params.subdivision_level=static_cast<int>(level);
                params.truth_translation=t;
                params.truth_rotation=w;
                params.noise_coefficient=data.noise_coefficients[v];
                auto scene=simulation::generateBiasFlow(
                    params,sampler,synthesizer,noise,rng);
                if (!scene.hasValue())
                    return fromNumerical(scene,ExperimentErrorCode::IncompleteRun,
                                         "Bias flow generation failed",level,v);

                const FlowField& flow=scene.value().noisy;
                auto franz_result=estimators.franz.estimate(flow);
                if (!franz_result.hasValue())
                    return fromNumerical(franz_result,ExperimentErrorCode::SolverFailed,
                                         "Franz failed on Bias sample",level,v);
                auto orig_result=estimators.original.estimate(flow,kvd);
                if (!orig_result.hasValue())
                    return fromNumerical(orig_result,ExperimentErrorCode::SolverFailed,
                                         "Original KvD failed on Bias sample",level,v);
                auto mod_result=estimators.modified.estimate(flow,kvd);
                if (!mod_result.hasValue())
                    return fromNumerical(mod_result,ExperimentErrorCode::SolverFailed,
                                         "Modified KvD failed on Bias sample",level,v);
                const MotionEstimate& f=franz_result.value();
                const MotionEstimate& o=orig_result.value().motion;
                const MotionEstimate& m=mod_result.value().motion;
                const std::array<std::pair<Vector3,Vector3>,6> pairs = {{
                    {t,f.translation()}, {w,f.rotation()},
                    {t,o.translation()}, {w,o.rotation()},
                    {t,m.translation()}, {w,m.rotation()}
                }};
                for (std::size_t metric=0;metric<6;++metric) {
                    auto err=source_metrics::normalizedVectorError(pairs[metric].first,
                                                   pairs[metric].second);
                    if (!err.hasValue())
                        return fromNumerical(err,ExperimentErrorCode::SolverFailed,
                            "Bias ywerte"+std::to_string(metric+1)+" error undefined",
                            level,v);
                    data.ywerte[metric][v*levels+level]=err.value();
                }
                // A Bias sample becomes valid only when all six metrics exist.
                if (paper_observer) {
                    // Fig. 1 只画 Original/Modified KvD 的四条方向误差。
                    constexpr std::size_t first_kvd_metric=2;
                    for (std::size_t k=first_kvd_metric;k<pairs.size();++k)
                        paper_observer({level,v+1,static_cast<std::size_t>(flow.observations().rows()),
                            k+1,data.xwerte[level],data.noise_coefficients[v],data.ywerte[k][v*levels+level],
                            source_metrics::sourceDirectionErrorDegrees(pairs[k].first,pairs[k].second)});
                }
                if(sample_completed)sample_completed();
                if (level+1<levels) {
                    if (x_count>std::numeric_limits<std::size_t>::max()/4)
                        return invalid("Bias xwerte sample count overflows");
                    x_count*=4;
                }
            }
        }
        return ErrorResult::success(std::move(data));
    } catch (const std::bad_alloc&) {
        ErrorDetail inner;
        inner.stage=ErrorStage::Simulation;
        inner.message="Bias experiment allocation failed";
        ErrorDetail outer;
        outer.stage=ErrorStage::Simulation;
        outer.message="Bias experiment cannot continue after allocation failure";
        return ErrorResult::failure(wrapFailure(
            ExperimentErrorCode::SolverFailed,std::move(outer),
            Failure<NumericalErrorCode>{NumericalErrorCode::ResourceLimitExceeded,
                std::move(inner)}));
    } catch (const std::length_error&) {
        ErrorDetail inner;
        inner.stage=ErrorStage::Simulation;
        inner.message="Bias experiment container exceeds maximum size";
        ErrorDetail outer;
        outer.stage=ErrorStage::Simulation;
        outer.message="Bias experiment cannot continue after size overflow";
        return ErrorResult::failure(wrapFailure(
            ExperimentErrorCode::SolverFailed,std::move(outer),
            Failure<NumericalErrorCode>{NumericalErrorCode::ResourceLimitExceeded,
                std::move(inner)}));
    }
}
}  // namespace

Result<BiasRunData,ExperimentErrorCode> BiasExperimentRunner::compute(
    const BiasExperimentConfig& config, IRandomSource& random,
    const std::function<void()>& sample_completed,const BiasPaperObserver& paper_observer) const {
    return computeImpl(config,sampler_,noise_,estimators_,random,sample_completed,paper_observer);
}

}  // namespace egomotion::experiments
