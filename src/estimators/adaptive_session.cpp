#include "egomotion/estimators/adaptive_session.hpp"
#include "../numerics/source_motion_errors.hpp"
#include "egomotion/harmonics/services.hpp"

#include <array>
#include <cmath>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>

namespace egomotion::estimators {
namespace {
constexpr std::size_t kResetAndInheritComparisons=2;

HarmonicCoefficients originalInitialCoefficients() {
    return HarmonicCoefficients::zeroOrderUnit(); // original MATLAB akkoeff(1,1)=1
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
    if(mode_==AdaptiveSessionMode::UpdateRates) {
        for(std::size_t j=0;j<policies_.size();++j)policies_[j]=&periodic_policies_[j];
    } else {
        policies_[0]=&reset_policy_;
        policies_[1]=&inherit_policy_;
    }
}

AdaptiveMfaSession::AdaptiveMfaSession(
    AdaptiveSessionMode mode,const AdaptiveCoefficientPolicies& policies,
    const IAdaptiveStepSolver* injected_solver)
    : AdaptiveMfaSession(mode,injected_solver) { policies_=policies; }

// 旋转、策略候选、比较、主求解和提交集中保持源时序，保留完整事务体便于审计。
Result<AdaptiveFrameResult,ExperimentErrorCode>
AdaptiveMfaSession::processFrame(const AdaptiveFrameInput& frame) {
    if (frame.matlab_frame_index!=expected_matlab_frame_index_)
        return fail(ExperimentErrorCode::InvalidParameters,
                    "Adaptive frames must be processed in original consecutive "
                    "MATLAB 1-based order, starting with i=2 (expected i="+
                    std::to_string(expected_matlab_frame_index_)+")");
    if (!frame.source_coefficient_rotation.allFinite()||
        !frame.truth.translation().allFinite()||!frame.truth.rotation().allFinite())
        return fail(ExperimentErrorCode::InvalidParameters,
                    "Adaptive frame rotation/truth contains non-finite values");
    const std::size_t comparison_count=mode_==AdaptiveSessionMode::UpdateRates
        ? kAdaptiveUpdatePeriods.size() : kResetAndInheritComparisons;
    for(std::size_t j=0;j<comparison_count;++j)
        if(policies_[j]==nullptr)
            return fail(ExperimentErrorCode::InvalidParameters,
                        "Adaptive comparison coefficient policy is missing");
    try {
        // Copy first: no state changes become visible before every required
        // comparison AND the primary solve have succeeded.
        HarmonicCoefficients next_primary=primary_;
        auto next_baselines=baselines_;
        if (mode_!=AdaptiveSessionMode::Initialization) {
            const harmonics::HarmonicRotator rotator;
            auto rotated=rotator.rotateSourceCoefficients(
                next_primary,frame.source_coefficient_rotation);
            if (!rotated.hasValue())
                return propagate(rotated.error(),
                                 "Adaptive primary coefficient rotation failed",
                                 frame.matlab_frame_index);
            next_primary=std::move(rotated).value();
        }
        for(std::size_t j=0;j<comparison_count;++j) {
            auto candidate=policies_[j]->prepare(frame.matlab_frame_index,next_primary,
                                                baselines_[j],frame.source_coefficient_rotation);
            if(!candidate.hasValue())
                return propagate(candidate.error(),
                                 "Adaptive coefficient update failed (baseline="+
                                     std::to_string(j)+")",frame.matlab_frame_index);
            if(!candidate.value().values().allFinite()) {
                ErrorDetail detail;detail.stage=ErrorStage::Solve;
                detail.message="Coefficient update policy returned non-finite candidate";
                return propagate({NumericalErrorCode::NonFiniteResult,std::move(detail)},
                                 "Adaptive coefficient candidate invalid",frame.matlab_frame_index);
            }
            next_baselines[j]=std::move(candidate).value();
        }

        AdaptiveFrameResult output;
        output.matlab_frame_index=frame.matlab_frame_index;
        output.comparison_motion.reserve(comparison_count);
        output.comparison_translation_error_deg.reserve(comparison_count);
        output.comparison_rotation_error_deg.reserve(comparison_count);
        for (std::size_t j=0;j<comparison_count;++j) {
            const HarmonicCoefficients& trial=next_baselines[j];
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
                frame.truth.translation(),motion.translation());
            auto r_error=source_metrics::rotationGeodesicErrorDegrees(
                frame.truth.rotation(),motion.rotation());
            output.comparison_motion.push_back(motion);
            output.comparison_translation_error_deg.push_back(std::move(t_error));
            output.comparison_rotation_error_deg.push_back(std::move(r_error));
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
