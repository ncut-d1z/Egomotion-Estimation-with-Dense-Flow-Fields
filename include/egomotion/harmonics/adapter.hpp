#pragma once

#include "egomotion/contracts/result.hpp"

#include <Eigen/Core>

namespace egomotion::harmonics {

// R4 bridge: keep the existing low-order C++17 leaf as the only source
// of basis, coefficient fitting and SO(3) harmonic rotation formulas.
// The public adapter returns the shared NumericalErrorCode Result.
Result<Scalar,NumericalErrorCode> reconstructNear(
    const HarmonicCoefficients& coefficients, const Vector3& direction);

Result<HarmonicCoefficients,NumericalErrorCode> fitSourceNear(
    const Eigen::VectorXd& nearness, const FlowField& observations);

// argument_rotation is EXPLICITLY the input argument of
// mu_new(d) = mu_old(argument_rotation*d). Session chooses R or R^T.
Result<HarmonicCoefficients,NumericalErrorCode> rotateCoefficients(
    const HarmonicCoefficients& coefficients, const Matrix3& argument_rotation);

}  // namespace egomotion::harmonics
