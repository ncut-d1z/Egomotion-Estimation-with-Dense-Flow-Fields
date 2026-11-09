#include "egomotion/harmonics/low_order.hpp"
#include "low_order_detail.hpp"

#include <cmath>

namespace egomotion::harmonics::low_order {
namespace {

Result<BasisValues> checkedBasis(BasisValues values) {
    if (!values.allFinite()) {
        return Result<BasisValues>::failure(
            {ErrorCode::NonFiniteResult, "non-finite spherical harmonic value"});
    }
    return Result<BasisValues>::success(std::move(values));
}

Result<RotationBlocks> selectedRotation(const Matrix3& rotation, RotationMethod method) {
    switch (method) {
    case RotationMethod::TensorDirect: return rotationDirect(rotation);
    case RotationMethod::Ivanic1996: return rotationIvanic(rotation);
    case RotationMethod::Choi1999: return rotationChoi(rotation);
    }
    return Result<RotationBlocks>::failure({ErrorCode::InvalidMethod, "unknown rotation method"});
}

} // namespace

Result<BasisValues> basisDirect(const Direction& direction) {
    if (const auto error = detail::validateDirection(direction)) {
        return Result<BasisValues>::failure(*error);
    }
    const double x = direction.x();
    const double y = direction.y();
    const double z = direction.z();
    const double monopole = 1.0 / std::sqrt(4.0 * detail::kPi);
    const double dipole = std::sqrt(3.0 / (4.0 * detail::kPi));
    const double zonal = std::sqrt(5.0 / (16.0 * detail::kPi));
    const double mixed = std::sqrt(15.0 / (4.0 * detail::kPi));
    const double difference = std::sqrt(15.0 / (16.0 * detail::kPi));
    BasisValues values;
    // Struebbe et al. 2015, Eqs. (59)--(67), theta 为极角。
    values << monopole, dipole * x, dipole * y, dipole * z,
        zonal * (3.0 * z * z - 1.0), mixed * x * z, mixed * y * z,
        difference * (x * x - y * y), mixed * x * y;
    return checkedBasis(std::move(values));
}

Result<BasisValues> basisRecurrence(const Direction& direction) {
    if (const auto error = detail::validateDirection(direction)) {
        return Result<BasisValues>::failure(*error);
    }
    const double x = direction.x();
    const double y = direction.y();
    const double z = direction.z();
    // 无CS相位的关联Legendre递推，以 P_l^m*cos(m phi)、P_l^m*sin(m phi)
    // 的笛卡尔多项式形式执行。无 atan2/除 sin(theta)，两极也有定义。
    const double p00 = 1.0;
    const double p10 = z * p00;
    const double p11_cos = x * p00;
    const double p11_sin = y * p00;
    const double p20 = (3.0 * z * p10 - p00) / 2.0;
    const double p21_cos = 3.0 * z * p11_cos;
    const double p21_sin = 3.0 * z * p11_sin;
    const double p22_cos = 3.0 * (x * p11_cos - y * p11_sin);
    const double p22_sin = 3.0 * (x * p11_sin + y * p11_cos);
    // N_lm=sqrt((2l+1)/(4pi)*(l-m)!/(l+m)!); m>0再乘sqrt(2)。
    const double n00 = 1.0 / std::sqrt(4.0 * detail::kPi);
    const double n10 = std::sqrt(3.0 / (4.0 * detail::kPi));
    const double n20 = std::sqrt(5.0 / (4.0 * detail::kPi));
    const double n21 = std::sqrt(5.0 / (12.0 * detail::kPi));
    const double n22 = std::sqrt(5.0 / (48.0 * detail::kPi));
    BasisValues values;
    values << n00 * p00, n10 * p11_cos, n10 * p11_sin, n10 * p10,
        n20 * p20, n21 * p21_cos, n21 * p21_sin,
        n22 * p22_cos, n22 * p22_sin;
    return checkedBasis(std::move(values));
}

Result<double> reconstruct(const Coefficients& coefficients, const Direction& direction) {
    if (!coefficients.allFinite()) {
        return Result<double>::failure({ErrorCode::NonFiniteInput, "coefficients must be finite"});
    }
    const auto basis = basisDirect(direction);
    if (!basis.hasValue()) return Result<double>::failure(basis.error());
    const double value = coefficients.dot(basis.value());
    if (!std::isfinite(value)) {
        return Result<double>::failure({ErrorCode::NonFiniteResult, "reconstruction overflow"});
    }
    return Result<double>::success(value);
}

Result<Coefficients> rotate(const Coefficients& coefficients,
                            const Matrix3& argument_rotation, RotationMethod method) {
    if (!coefficients.allFinite()) {
        return Result<Coefficients>::failure(
            {ErrorCode::NonFiniteInput, "coefficients must be finite"});
    }
    const auto blocks = selectedRotation(argument_rotation, method);
    if (!blocks.hasValue()) return Result<Coefficients>::failure(blocks.error());
    Coefficients output;
    output(0) = coefficients(0);
    output.segment<3>(1) = blocks.value().dipole * coefficients.segment<3>(1);
    output.segment<5>(4) = blocks.value().quadrupole * coefficients.segment<5>(4);
    if (!output.allFinite()) {
        return Result<Coefficients>::failure(
            {ErrorCode::NonFiniteResult, "coefficient rotation overflow"});
    }
    return Result<Coefficients>::success(std::move(output));
}

Result<Coefficients> fromMatlabLayout(const MatlabCoefficients& matlab) {
    if (!matlab.allFinite()) {
        return Result<Coefficients>::failure(
            {ErrorCode::NonFiniteInput, "MATLAB coefficient matrix must be finite"});
    }
    Coefficients paper;
    paper << matlab(0, 0), -matlab(1, 2), -matlab(1, 0), matlab(1, 1),
        matlab(2, 2), -matlab(2, 3), -matlab(2, 1), matlab(2, 4), matlab(2, 0);
    return Result<Coefficients>::success(std::move(paper));
}

Result<MatlabCoefficients> toMatlabLayout(const Coefficients& paper) {
    if (!paper.allFinite()) {
        return Result<MatlabCoefficients>::failure(
            {ErrorCode::NonFiniteInput, "paper coefficients must be finite"});
    }
    MatlabCoefficients matlab = MatlabCoefficients::Zero();
    matlab(0, 0) = paper(0);
    matlab(1, 0) = -paper(2);
    matlab(1, 1) = paper(3);
    matlab(1, 2) = -paper(1);
    matlab(2, 0) = paper(8);
    matlab(2, 1) = -paper(6);
    matlab(2, 2) = paper(4);
    matlab(2, 3) = -paper(5);
    matlab(2, 4) = paper(7);
    return Result<MatlabCoefficients>::success(std::move(matlab));
}

} // namespace egomotion::harmonics::low_order
