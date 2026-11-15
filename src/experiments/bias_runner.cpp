#include "egomotion/experiments/bias_runner.hpp"

#include "egomotion/estimators/franz_kvd.hpp"
#include "egomotion/sampling/random.hpp"
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
template<class T>
ErrorResult fromSolver(const Result<T,NumericalErrorCode>& failed,
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
        ExperimentErrorCode::SolverFailed, std::move(outer), failed.error()));
}
Result<Scalar,NumericalErrorCode> normalizedVectorError(
    const Vector3& truth, const Vector3& estimate) {
    const double nt=truth.norm(), ne=estimate.norm();
    if (!std::isfinite(nt)||!std::isfinite(ne)) {
        ErrorDetail detail;
        detail.stage=ErrorStage::Solve;
        detail.message="Source transfehler_div input norm is non-finite";
        return Result<Scalar,NumericalErrorCode>::failure(
            {NumericalErrorCode::NonFiniteInput,std::move(detail)});
    }
    // transfehler_div yields mathematical zero for zero truth and nonzero
    // estimate, but that is NOT a valid error statistic; never count it.
    if (nt==0 || ne==0) {
        ErrorDetail detail;
        detail.stage=ErrorStage::Solve;
        detail.message="Source transfehler_div has undefined zero vector norm";
        return Result<Scalar,NumericalErrorCode>::failure(
            {NumericalErrorCode::ZeroNorm,std::move(detail)});
    }
    const double scaled=(truth-estimate*(nt/ne)).norm();
    if (!std::isfinite(scaled)) {
        ErrorDetail detail;
        detail.stage=ErrorStage::Solve;
        detail.message="Source-normalized motion error is non-finite";
        return Result<Scalar,NumericalErrorCode>::failure(
            {NumericalErrorCode::NonFiniteResult,std::move(detail)});
    }
    return Result<Scalar,NumericalErrorCode>::success(scaled);
}

ErrorResult computeImpl(const BiasExperimentConfig& config,
                        const IDirectionSampler& sampler,
                    const IFlowNoiseModel& noise) {
    // One sample for each (noise coefficient, subdivision level); source
    // uses 3 noise coefficients in this order, then levels 0..maximum.
    if (config.maximum_subdivision_level<0)
        return invalid("Bias maximum subdivision level is negative");
    const std::size_t levels=
        static_cast<std::size_t>(config.maximum_subdivision_level)+1;
    if (levels>std::numeric_limits<std::size_t>::max()/3)
        return invalid("Bias output shape overflows size_t");
    try {
        sampling::Mt19937RandomSource rng(config.seed);
        auto truth_t=sampling::randomUnitDirection(rng);
        if (!truth_t.hasValue()) return fromSolver(truth_t,
            "Bias truth translation RNG failed",0);
        auto truth_w=sampling::randomUnitDirection(rng);
        if (!truth_w.hasValue()) return fromSolver(truth_w,
            "Bias truth rotation RNG failed",0);
        const Vector3 t=std::move(truth_t).value(), w=std::move(truth_w).value();

        // The source calls RandStream.setDefaultStream(seed) a SECOND time
        // after drawing the two fixed truth directions.
        rng.reseed(config.seed);
        BiasRunData data;
        data.level_count=levels;
        data.xwerte.assign(levels,0.0);
        for (auto& y:data.ywerte) y.assign(levels*3,0.0);
        // Caller/Composition Root owns the single injected sampler/noise pair.
        // No duplicate concrete model is created or selected in the Runner.
        const simulation::FlowSynthesizer synthesizer;
        const estimators::FranzMatchedFilter franz;
        const estimators::OriginalKvDEstimator original;
        const estimators::ModifiedKvDEstimator modified;
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
                    return fromSolver(scene,"Bias flow generation failed",level,v);

                const FlowField& flow=scene.value().noisy;
                auto franz_result=franz.estimate(flow);
                if (!franz_result.hasValue())
                    return fromSolver(franz_result,"Franz failed on Bias sample",level,v);
                auto orig_result=original.estimate(flow,kvd);
                if (!orig_result.hasValue())
                    return fromSolver(orig_result,"Original KvD failed on Bias sample",level,v);
                auto mod_result=modified.estimate(flow,kvd);
                if (!mod_result.hasValue())
                    return fromSolver(mod_result,"Modified KvD failed on Bias sample",level,v);
                const MotionEstimate& f=franz_result.value();
                const MotionEstimate& o=orig_result.value().motion;
                const MotionEstimate& m=mod_result.value().motion;
                const std::array<std::pair<Vector3,Vector3>,6> pairs = {{
                    {t,f.translation}, {w,f.rotation},
                    {t,o.translation}, {w,o.rotation},
                    {t,m.translation}, {w,m.rotation}
                }};
                for (std::size_t metric=0;metric<6;++metric) {
                    auto err=normalizedVectorError(pairs[metric].first,
                                                   pairs[metric].second);
                    if (!err.hasValue())
                        return fromSolver(err,
                            "Bias ywerte"+std::to_string(metric+1)+" error undefined",
                            level,v);
                    data.ywerte[metric][v*levels+level]=err.value();
                }
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
    const BiasExperimentConfig& config) const {
    return computeImpl(config,sampler_,noise_);
}

}  // namespace egomotion::experiments
