#pragma once

#include "egomotion/estimators/adaptive_step.hpp"

#include <array>
#include <cstddef>
#include <vector>

namespace egomotion::estimators {

enum class AdaptiveSessionMode {
    Initialization,  // do not rotate coeff; compare constant vs current
    Comparison,      // rotate main before one comparison per baseline
    UpdateRates      // rotate four baselines, refresh 1/5/11/21, discard dumi
};
struct AdaptiveFrameInput {
    std::size_t matlab_frame_index=0; // original 1-based i (not C++ index)
    Matrix3 coefficient_argument_rotation=Matrix3::Identity();
    FlowField observations;
    MotionEstimate truth;
};
struct AdaptiveFrameResult {
    std::size_t matlab_frame_index=0;
    AdaptiveStepResult primary_step;
    std::vector<MotionEstimate> comparison_motion;
    std::vector<Scalar> comparison_translation_error_deg;
    std::vector<Scalar> comparison_rotation_error_deg;
};

// Each session owns its own primary and baseline states. It never writes
// an intermediate coefficient, even if a comparison succeeds but the
// subsequent primary StepSolver fails.
class AdaptiveMfaSession {
public:
    // Default uses the real source-derived solver; optional injection is
    // intended for deterministic tests. The supplied solver must outlive the
    // Session. No change to the scientific algorithm for production callers.
    explicit AdaptiveMfaSession(AdaptiveSessionMode mode,
                                const IAdaptiveStepSolver* injected_solver = nullptr);

    // The default solver pointer refers to this Session's owned instance;
    // copying or moving would otherwise leave an unsafe internal pointer.
    AdaptiveMfaSession(const AdaptiveMfaSession&)=delete;
    AdaptiveMfaSession& operator=(const AdaptiveMfaSession&)=delete;
    AdaptiveMfaSession(AdaptiveMfaSession&&)=delete;
    AdaptiveMfaSession& operator=(AdaptiveMfaSession&&)=delete;

    Result<AdaptiveFrameResult,ExperimentErrorCode> processFrame(
        const AdaptiveFrameInput& frame);

    const HarmonicCoefficients& primaryCoefficients() const noexcept {
        return primary_;
    }
    // UpdateRates: 1/5/11/21; compare calls must not commit their new coeff.
    const std::array<HarmonicCoefficients,4>& updateRateBaselines() const noexcept {
        return baselines_;
    }
private:
    AdaptiveSessionMode mode_;
    HarmonicCoefficients primary_;
    std::array<HarmonicCoefficients,4> baselines_;
    AdaptiveMfaStepSolver production_solver_;
    const IAdaptiveStepSolver* solver_=nullptr;
    // Each successful call consumes exactly one original MATLAB frame.
    // Invalid/out-of-order frames never advance persistent state.
    std::size_t expected_matlab_frame_index_ = 2;
};

}  // namespace egomotion::estimators
