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
// mu_new(d) = mu_old(argument_rotation*d). The caller chooses R or R^T.
Result<HarmonicCoefficients,NumericalErrorCode> rotateCoefficients(
    const HarmonicCoefficients& coefficients, const Matrix3& argument_rotation);

// 忠实保留 transfor_koeff(C,r) 的 l<=2 系数动作；r 是原 SHRotate 的输入。
// 原函数直接将无CS旋转块乘源CS布局系数，通常不满足 mu_new(d)=mu_old(r*d)。
// 与上面的物理接口分开命名；两者只在特定旋转（例如绕z）下相同。
Result<HarmonicCoefficients,NumericalErrorCode> rotateSourceCoefficients(
    const HarmonicCoefficients& coefficients, const Matrix3& source_rotation);

}  // namespace egomotion::harmonics
