#include "egomotion/harmonics/low_order.hpp"
#include "low_order_detail.hpp"

#include <array>
#include <cmath>

namespace egomotion::harmonics::low_order {
namespace {

// h_i(d)=sqrt(15/(4pi))*d^T Q_i d；Tr(Q_i Q_j)=delta_ij/2。
// 张量由2015式(63)--(67)直接构造，不调用任何旋转递推。
std::array<Matrix3, 5> quadrupoleTensors() {
    std::array<Matrix3, 5> tensors;
    for (auto& tensor : tensors) tensor.setZero();
    const double zonal = 1.0 / std::sqrt(12.0);
    tensors[0].diagonal() << -zonal, -zonal, 2.0 * zonal;
    tensors[1](0, 2) = tensors[1](2, 0) = 0.5;
    tensors[2](1, 2) = tensors[2](2, 1) = 0.5;
    tensors[3].diagonal() << 0.5, -0.5, 0.0;
    tensors[4](0, 1) = tensors[4](1, 0) = 0.5;
    return tensors;
}

Matrix3 irDegreeOne(const Matrix3& argument_rotation) {
    // IR论文的轴矩阵为 A^T；一阶为 P*A^T*P^T，P:xyz->yzx。
    const auto& a = argument_rotation;
    Matrix3 block;
    block << a(1, 1), a(2, 1), a(0, 1),
             a(1, 2), a(2, 2), a(0, 2),
             a(1, 0), a(2, 0), a(0, 0);
    return block;
}

// IR98 Table2的P边界；l=2时前阶为1，只允许a,i在[-1,1]。
// 所有调用方先检查零权重，不能访问无效前阶行后再乘0。
double auxiliaryP(const Matrix3& block, int i, int a, int n) {
    const int row = i + 1;
    const int lower_row = a + 1;
    if (n == -2) {
        return block(row, 2) * block(lower_row, 0) +
               block(row, 0) * block(lower_row, 2);
    }
    if (n == 2) {
        return block(row, 2) * block(lower_row, 2) -
               block(row, 0) * block(lower_row, 0);
    }
    return block(row, 1) * block(lower_row, n + 1);
}

double auxiliaryV(const Matrix3& block, int m, int n) {
    if (m == 0) {
        return auxiliaryP(block, 1, 1, n) + auxiliaryP(block, -1, -1, n);
    }
    if (m > 0) {
        const int delta = (m == 1) ? 1 : 0;
        return std::sqrt(1.0 + delta) * auxiliaryP(block, 1, m - 1, n) -
               (1.0 - delta) * auxiliaryP(block, -1, -m + 1, n);
    }
    const int delta = (m == -1) ? 1 : 0;
    // IR98 Table2此处印作sqrt(1-delta)，与其式6.7、定义及IR96表3矛盾。
    // 加号经一般四元数三路符号恒等验证；见验证报告的x45反例。
    return (1.0 - delta) * auxiliaryP(block, 1, m + 1, n) +
           std::sqrt(1.0 + delta) * auxiliaryP(block, -1, -m - 1, n);
}

Matrix5 irDegreeTwo(const Matrix3& block) {
    Matrix5 result;
    for (int m = -2; m <= 2; ++m) {
        for (int n = -2; n <= 2; ++n) {
            const double denominator = (std::abs(n) == 2) ? 12.0 : 4.0 - n * n;
            const int delta = (m == 0) ? 1 : 0;
            const int absolute_m = std::abs(m);
            const int u_numerator = 4 - m * m;
            double value = 0.0;
            if (u_numerator != 0) {
                value = std::sqrt(u_numerator / denominator) *
                        auxiliaryP(block, 0, m, n);
            }
            const double v = (1.0 - 2.0 * delta) * 0.5 *
                std::sqrt((1.0 + delta) * (1 + absolute_m) * (2 + absolute_m) /
                          denominator);
            value += v * auxiliaryV(block, m, n);
            // l=2全部m的w精确为0，不调用W，不扩展到l>2。
            result(m + 2, n + 2) = value;
        }
    }
    return result;
}

} // namespace

Result<RotationBlocks> rotationDirect(const Matrix3& argument_rotation) {
    if (const auto error = detail::validateRotation(argument_rotation)) {
        return Result<RotationBlocks>::failure(*error);
    }
    const auto tensors = quadrupoleTensors();
    RotationBlocks output;
    output.dipole = argument_rotation.transpose();
    for (int j = 0; j < 5; ++j) {
        const Matrix3 transformed =
            argument_rotation.transpose() * tensors[j] * argument_rotation;
        for (int i = 0; i < 5; ++i) {
            output.quadrupole(i, j) = 2.0 * tensors[i].cwiseProduct(transformed).sum();
        }
    }
    return detail::checkedBlocks(std::move(output));
}

Result<RotationBlocks> rotationIvanic(const Matrix3& argument_rotation) {
    if (const auto error = detail::validateRotation(argument_rotation)) {
        return Result<RotationBlocks>::failure(*error);
    }
    const Matrix3 one = irDegreeOne(argument_rotation);
    const Matrix5 two = irDegreeTwo(one);
    return detail::checkedBlocks(detail::toPaperOrder(one, two));
}

} // namespace egomotion::harmonics::low_order
