#include "egomotion/estimators/adaptive_session.hpp"
#include "../numerics/source_motion_errors.hpp"
#include "egomotion/harmonics/adapter.hpp"

#include <array>
#include <cmath>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>

namespace egomotion::estimators {
namespace {
constexpr std::array<std::size_t,4> kUpdatePeriods{{1,5,11,21}};

HarmonicCoefficients originalInitialCoefficients() {
    HarmonicCoefficients value;
    value.values.setZero();
    value.values(0,0)=1.0; // original MATLAB akkoeff(1,1)=1
    return value;
}
Result<AdaptiveFrameResult,ExperimentErrorCode> fail(
    ExperimentErrorCode code,std::string message) {
    ErrorDetail error;
    error.stage=ErrorStage::Simulation;
    error.message=std::move(message);
    return Result<AdaptiveFrameResult,ExperimentErrorCode>::failure(
        {code,std::move(error)});
}
Result<AdaptiveFrameResult,ExperimentErrorCode> propagate(
    const Failure<NumericalErrorCode>& child,std::string message,
    std::size_t frame_index) {
    ErrorDetail outer;
    outer.stage=ErrorStage::Simulation;
    outer.message=std::move(message)+" (matlab_frame_index="+
                  std::to_string(frame_index)+")";
    return Result<AdaptiveFrameResult,ExperimentErrorCode>::failure(
        wrapFailure(ExperimentErrorCode::SolverFailed,std::move(outer),child));
}
}  // namespace

AdaptiveMfaSession::AdaptiveMfaSession(
    AdaptiveSessionMode mode,const IAdaptiveStepSolver* injected_solver)
    : mode_(mode),primary_(originalInitialCoefficients()),
      solver_(injected_solver ? injected_solver : &production_solver_) {
    baselines_.fill(originalInitialCoefficients());
}

Result<AdaptiveFrameResult,ExperimentErrorCode>
AdaptiveMfaSession::processFrame(const AdaptiveFrameInput& frame) {
    if (frame.matlab_frame_index!=expected_matlab_frame_index_)
        return fail(ExperimentErrorCode::InvalidParameters,
                    "Adaptive frames must be processed in original consecutive "
                    "MATLAB 1-based order, starting with i=2 (expected i="+
                    std::to_string(expected_matlab_frame_index_)+")");
    if (!frame.source_coefficient_rotation.allFinite()||
        !frame.truth.translation.allFinite()||!frame.truth.rotation.allFinite())
        return fail(ExperimentErrorCode::InvalidParameters,
                    "Adaptive frame rotation/truth contains non-finite values");
    try {
        // Copy first: no state changes become visible before every required
        // comparison AND the primary solve have succeeded.
        HarmonicCoefficients next_primary=primary_;
        auto next_baselines=baselines_;
        if (mode_!=AdaptiveSessionMode::Initialization) {
            auto rotated=harmonics::rotateSourceCoefficients(
                next_primary,frame.source_coefficient_rotation);
            if (!rotated.hasValue())
                return propagate(rotated.error(),
                                 "Adaptive primary coefficient rotation failed",
                                 frame.matlab_frame_index);
            next_primary=std::move(rotated).value();
        }
        if (mode_==AdaptiveSessionMode::UpdateRates) {
            for (std::size_t j=0;j<next_baselines.size();++j) {
                auto rotated=harmonics::rotateSourceCoefficients(
                    next_baselines[j],frame.source_coefficient_rotation);
                if (!rotated.hasValue())
                    return propagate(rotated.error(),
                                     "Adaptive baseline rotation failed (period="+
                                         std::to_string(kUpdatePeriods[j])+")",
                                     frame.matlab_frame_index);
                next_baselines[j]=std::move(rotated).value();
                // Source refreshes from current main BEFORE this frame's primary solve.
                if (frame.matlab_frame_index%kUpdatePeriods[j]==0)
                    next_baselines[j]=next_primary;
            }
        }

        AdaptiveFrameResult output;
        output.matlab_frame_index=frame.matlab_frame_index;
        const std::size_t comparison_count=
            mode_==AdaptiveSessionMode::UpdateRates ? kUpdatePeriods.size() : 2u;
        output.comparison_motion.reserve(comparison_count);
        output.comparison_translation_error_deg.reserve(comparison_count);
        output.comparison_rotation_error_deg.reserve(comparison_count);
        for (std::size_t j=0;j<comparison_count;++j) {
            HarmonicCoefficients trial;
            if (mode_==AdaptiveSessionMode::UpdateRates)
                trial=next_baselines[j];
            else if (j==0)
                trial=originalInitialCoefficients(); // ohne
            else
                trial=next_primary; // mit
            // The source does one comparison iteration per frame.
            auto comparison=solver_->solve(trial,frame.observations);
            if (!comparison.hasValue())
                return propagate(comparison.error(),
                                 "Adaptive comparison solve failed (baseline="+
                                     std::to_string(j)+")",
                                 frame.matlab_frame_index);
            // For UpdateRates the first return (dumi) MUST be discarded.
            // In initialization/comparison the returned coeff is local to this
            // frame only (source anzahlitfuerini=1).
            const MotionEstimate& motion=comparison.value().motion;
            auto t_error=source_metrics::sourceDirectionErrorDegrees(
                frame.truth.translation,motion.translation);
            if (!t_error.hasValue())
                return propagate(t_error.error(),
                                 "Adaptive comparison translation angle undefined",
                                 frame.matlab_frame_index);
            auto r_error=source_metrics::sourceDirectionErrorDegrees(
                frame.truth.rotation,motion.rotation);
            if (!r_error.hasValue())
                return propagate(r_error.error(),
                                 "Adaptive comparison rotation angle undefined",
                                 frame.matlab_frame_index);
            output.comparison_motion.push_back(motion);
            output.comparison_translation_error_deg.push_back(t_error.value());
            output.comparison_rotation_error_deg.push_back(r_error.value());
        }
        auto primary_result=solver_->solve(next_primary,frame.observations);
        if (!primary_result.hasValue())
            return propagate(primary_result.error(),
                             "Adaptive primary solve failed; state not committed",
                             frame.matlab_frame_index);
        output.primary_step=std::move(primary_result).value();
        next_primary=output.primary_step.coefficients_next;
        // Construct the full success alternative BEFORE changing persistent
        // state. A failure while allocating/moving the output cannot commit
        // the main or any baseline coefficients.
        auto complete=Result<AdaptiveFrameResult,ExperimentErrorCode>::success(
            std::move(output));
        primary_=std::move(next_primary);
        if (mode_==AdaptiveSessionMode::UpdateRates)
            baselines_=std::move(next_baselines);
        ++expected_matlab_frame_index_;
        return complete;
    } catch (const std::bad_alloc&) {
        Failure<NumericalErrorCode> inner{
            NumericalErrorCode::ResourceLimitExceeded,
            ErrorDetail{}};
        inner.detail.stage=ErrorStage::Solve;
        inner.detail.message="Adaptive session allocation failed";
        return propagate(inner,"Adaptive session failed without committing state",
                         frame.matlab_frame_index);
    } catch (const std::length_error&) {
        Failure<NumericalErrorCode> inner{
            NumericalErrorCode::ResourceLimitExceeded,
            ErrorDetail{}};
        inner.detail.stage=ErrorStage::Solve;
        inner.detail.message="Adaptive session comparison array too large";
        return propagate(inner,"Adaptive session failed without committing state",
                         frame.matlab_frame_index);
    }
}

}  // namespace egomotion::estimators
