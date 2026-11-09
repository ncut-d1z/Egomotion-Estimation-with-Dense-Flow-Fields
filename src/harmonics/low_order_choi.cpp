#include "egomotion/harmonics/low_order.hpp"
#include "low_order_detail.hpp"

#include <cmath>
#include <complex>

namespace egomotion::harmonics::low_order {
namespace {

using Complex = std::complex<double>;
using ComplexMatrix3 = Eigen::Matrix<Complex, 3, 3>;
using ComplexMatrix5 = Eigen::Matrix<Complex, 5, 5>;

// Choi et al. 1999 Eqs.(5.4),(5.5)，与IR实递推无共用运算步骤。
ComplexMatrix3 complexDegreeOne(const Matrix3& argument_rotation) {
    const Matrix3 r = argument_rotation.transpose(); // 论文行坐标轴矩阵R=A^T。
    const double inverse_sqrt_two = 1.0 / std::sqrt(2.0);
    Matrix3 real;
    real << (r(1, 1) + r(0, 0)) / 2.0, r(0, 2) * inverse_sqrt_two,
            (r(1, 1) - r(0, 0)) / 2.0,
            r(2, 0) * inverse_sqrt_two, r(2, 2), -r(2, 0) * inverse_sqrt_two,
            (r(1, 1) - r(0, 0)) / 2.0, -r(0, 2) * inverse_sqrt_two,
            (r(1, 1) + r(0, 0)) / 2.0;
    Matrix3 imag;
    imag << (r(1, 0) - r(0, 1)) / 2.0, r(1, 2) * inverse_sqrt_two,
            -(r(1, 0) + r(0, 1)) / 2.0,
            -r(2, 1) * inverse_sqrt_two, 0.0, -r(2, 1) * inverse_sqrt_two,
            (r(1, 0) + r(0, 1)) / 2.0, r(1, 2) * inverse_sqrt_two,
            (r(0, 1) - r(1, 0)) / 2.0;
    return real.cast<Complex>() + Complex(0.0, 1.0) * imag.cast<Complex>();
}

// 先以整数分子判精确零，再访问前阶元素。l=2时索引均为[-1,1]。
void addTerm(Complex& value, int numerator, int denominator,
             const ComplexMatrix3& lower, int i, int j, int m, int n) {
    if (numerator == 0) return;
    const double weight = std::sqrt(static_cast<double>(numerator) / denominator);
    value += weight * lower(i + 1, j + 1) * lower(m + 1, n + 1);
}

ComplexMatrix5 complexDegreeTwo(const ComplexMatrix3& lower) {
    ComplexMatrix5 output;
    for (int m = -2; m <= 2; ++m) {
        for (int n = -2; n <= 2; ++n) {
            Complex value(0.0, 0.0);
            if (std::abs(n) < 2) {
                // Eq.(6.1)，a/b按(6.2)/(6.3)。
                const int denominator = 4 - n * n;
                addTerm(value, 4 - m * m, denominator, lower, 0, 0, m, n);
                addTerm(value, (2 + m) * (1 + m), 2 * denominator,
                        lower, 1, 0, m - 1, n);
                addTerm(value, (2 - m) * (1 - m), 2 * denominator,
                        lower, -1, 0, m + 1, n);
            } else {
                // 边界n=-2用(6.9)，n=+2用(6.14)；前阶列为n-sign(n)。
                const int sign = (n > 0) ? 1 : -1;
                const int previous_n = n - sign;
                constexpr int kBoundaryDenominator = 12; // (l+|n|)(l+|n|-1)
                addTerm(value, 2 * (4 - m * m), kBoundaryDenominator,
                        lower, 0, sign, m, previous_n);
                addTerm(value, (2 + m) * (1 + m), kBoundaryDenominator,
                        lower, 1, sign, m - 1, previous_n);
                addTerm(value, (2 - m) * (1 - m), kBoundaryDenominator,
                        lower, -1, sign, m + 1, previous_n);
            }
            output(m + 2, n + 2) = value;
        }
    }
    return output;
}

template <int Degree>
Eigen::Matrix<Complex, 2 * Degree + 1, 2 * Degree + 1> realToComplexBasis() {
    using Block = Eigen::Matrix<Complex, 2 * Degree + 1, 2 * Degree + 1>;
    Block w = Block::Zero();
    w(Degree, Degree) = 1.0;
    const double inverse_sqrt_two = 1.0 / std::sqrt(2.0);
    // Eq.(8.6)，行列均按-m,+m；转换必须使用adjoint，不是transpose。
    for (int m = 1; m <= Degree; ++m) {
        const double sign = (m % 2 == 0) ? 1.0 : -1.0;
        w(Degree - m, Degree - m) = Complex(0.0, -inverse_sqrt_two);
        w(Degree - m, Degree + m) = Complex(0.0, sign * inverse_sqrt_two);
        w(Degree + m, Degree - m) = inverse_sqrt_two;
        w(Degree + m, Degree + m) = sign * inverse_sqrt_two;
    }
    return w;
}

} // namespace

Result<RotationBlocks> rotationChoi(const Matrix3& argument_rotation) {
    if (const auto error = detail::validateRotation(argument_rotation)) {
        return Result<RotationBlocks>::failure(*error);
    }
    const ComplexMatrix3 one = complexDegreeOne(argument_rotation);
    const ComplexMatrix5 two = complexDegreeTwo(one);
    const auto w_one = realToComplexBasis<1>();
    const auto w_two = realToComplexBasis<2>();
    // Eq.(8.7): D=W^dagger R W，所以R=W D W^dagger。
    const ComplexMatrix3 real_one = w_one * one * w_one.adjoint();
    const ComplexMatrix5 real_two = w_two * two * w_two.adjoint();
    if (!real_one.allFinite() || !real_two.allFinite()) {
        return Result<RotationBlocks>::failure(
            {ErrorCode::NonFiniteResult, "non-finite complex recursion result"});
    }
    if (real_one.imag().cwiseAbs().maxCoeff() > detail::kInputRoundoffAllowance ||
        real_two.imag().cwiseAbs().maxCoeff() > detail::kInputRoundoffAllowance) {
        return Result<RotationBlocks>::failure(
            {ErrorCode::ComplexResidue, "real harmonic rotation has unexpected imaginary part"});
    }
    const Matrix3 ir_one = real_one.real();
    const Matrix5 ir_two = real_two.real();
    return detail::checkedBlocks(detail::toPaperOrder(ir_one, ir_two));
}

} // namespace egomotion::harmonics::low_order
