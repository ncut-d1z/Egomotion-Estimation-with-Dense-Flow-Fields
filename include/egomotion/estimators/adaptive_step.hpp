#pragma once

#include "egomotion/contracts/result.hpp"

#include <Eigen/Core>

namespace egomotion::estimators {

struct AdaptiveStepTrace {
    Eigen::VectorXd near_raw;
    Eigen::VectorXd near_after_sign;
    Eigen::VectorXd near_normalized;
    Scalar original_near_mean = 0.0;
    Matrix3 m1=Matrix3::Zero(),m2=Matrix3::Zero();
    Matrix3 m3=Matrix3::Zero(),m4=Matrix3::Zero();
    Vector6 rhs=Vector6::Zero(),solved_motion=Vector6::Zero();
    Matrix6 system_matrix=Matrix6::Zero();
    Vector3 translation_raw=Vector3::Zero();
    Scalar error_mse=0.0;
    Eigen::VectorXd recomputed_near;
    HarmonicCoefficients coefficients_next;
};
struct AdaptiveStepResult {
    MotionEstimate motion;
    HarmonicCoefficients coefficients_next;
    Scalar error_mse=0.0; // original oneit... returned MSE, NOT RMS
    AdaptiveStepTrace trace;
};

// A narrow, read-only numerical step seam permits deterministic Session
// transaction tests. A Session never owns a caller-injected solver.
// 成功载荷的 MSE 及已提供的 trace 数值须有限；Session 在提交前核对，
// 不要求事务替身填充全部 trace，也不从 trace 重新计算求解结果。
class IAdaptiveStepSolver {
public:
    virtual ~IAdaptiveStepSolver() = default;
    virtual Result<AdaptiveStepResult,NumericalErrorCode> solve(
        const HarmonicCoefficients& old, const FlowField& observations) const = 0;
};

// Production algorithm remains the default in every public Runner.
// Only a successful StepResult may update a Session's main coefficients.
class AdaptiveMfaStepSolver final : public IAdaptiveStepSolver {
public:
    Result<AdaptiveStepResult,NumericalErrorCode> solve(
        const HarmonicCoefficients& old, const FlowField& observations) const override;
};

}  // namespace egomotion::estimators
