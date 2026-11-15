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

// Stateless. Only a successful StepResult may update a Session's main coeff.
class AdaptiveMfaStepSolver {
public:
    Result<AdaptiveStepResult,NumericalErrorCode> solve(
        const HarmonicCoefficients& old, const FlowField& observations) const;
};

}  // namespace egomotion::estimators
