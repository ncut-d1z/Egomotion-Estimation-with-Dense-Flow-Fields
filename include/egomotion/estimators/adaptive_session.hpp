#pragma once

#include "egomotion/estimators/adaptive_step.hpp"
#include "egomotion/estimators/coefficient_update.hpp"

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
    // 原 vordrehmatrix=rodrigues(reldrehvektoren(i-1,:))，直接交给源动作适配。
    // 不把此输入误称为源CS近度函数的物理参数旋转。
    Matrix3 source_coefficient_rotation=Matrix3::Identity();
    FlowField observations;
    MotionEstimate truth;
};
struct AdaptiveFrameResult {
    std::size_t matlab_frame_index=0;
    AdaptiveStepResult primary_step;
    std::vector<MotionEstimate> comparison_motion;
    // 指标无定义与求解失败分离：仅指标失败不回滚已成功的帧求解。
    std::vector<Result<Scalar,NumericalErrorCode>> comparison_translation_error_deg;
    std::vector<Result<Scalar,NumericalErrorCode>> comparison_rotation_error_deg;
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

    // 显式替换比较策略：前两槽用于Initialization/Comparison，四槽用于
    // UpdateRates。使用的槽不可为空；策略及solver须比Session活得更久。
    // Session复制指针数组，不借用数组本身；策略只提供临时候选。
    AdaptiveMfaSession(AdaptiveSessionMode mode,
                       const AdaptiveCoefficientPolicies& policies,
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
    ResetCoefficientUpdatePolicy reset_policy_;
    InheritCoefficientUpdatePolicy inherit_policy_;
    std::array<PeriodicCoefficientUpdatePolicy,4> periodic_policies_{{
        PeriodicCoefficientUpdatePolicy{kAdaptiveUpdatePeriods[0]},
        PeriodicCoefficientUpdatePolicy{kAdaptiveUpdatePeriods[1]},
        PeriodicCoefficientUpdatePolicy{kAdaptiveUpdatePeriods[2]},
        PeriodicCoefficientUpdatePolicy{kAdaptiveUpdatePeriods[3]}}};
    AdaptiveCoefficientPolicies policies_{};
    AdaptiveMfaStepSolver production_solver_;
    const IAdaptiveStepSolver* solver_=nullptr;
    // Each successful call consumes exactly one original MATLAB frame.
    // Invalid/out-of-order frames never advance persistent state.
    std::size_t expected_matlab_frame_index_ = 2;
};

}  // namespace egomotion::estimators
