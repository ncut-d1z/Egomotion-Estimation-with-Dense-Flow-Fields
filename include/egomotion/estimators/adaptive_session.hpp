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
    explicit AdaptiveMfaSession(AdaptiveSessionMode mode);

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
    AdaptiveMfaStepSolver solver_;
};

}  // namespace egomotion::estimators
