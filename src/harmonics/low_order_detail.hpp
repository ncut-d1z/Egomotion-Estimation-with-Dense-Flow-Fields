#pragma once

#include "egomotion/harmonics/low_order.hpp"

#include <Eigen/LU>
#include <array>
#include <cmath>
#include <limits>
#include <optional>

namespace egomotion::harmonics::low_order::detail {

inline constexpr double kPi = 3.141592653589793238462643383279502884;
// 输入有效性舍入余量，不是 DEC-005 的算法验收误差阈值。
inline constexpr double kInputRoundoffAllowance =
    128.0 * std::numeric_limits<double>::epsilon();
inline constexpr std::array<int, 3> kPaperDipoleFromIr{{2, 0, 1}};
inline constexpr std::array<int, 5> kPaperQuadrupoleFromIr{{2, 3, 1, 4, 0}};

inline std::optional<Error> validateDirection(const Direction& direction) {
    if (!direction.allFinite()) {
        return Error{ErrorCode::NonFiniteInput, "direction must be finite"};
    }
    const double norm_squared = direction.squaredNorm();
    if (!std::isfinite(norm_squared) ||
        std::abs(norm_squared - 1.0) > kInputRoundoffAllowance) {
        return Error{ErrorCode::NotUnitDirection, "direction must have unit norm"};
    }
    return std::nullopt;
}

inline std::optional<Error> validateRotation(const Matrix3& rotation) {
    if (!rotation.allFinite()) {
        return Error{ErrorCode::NonFiniteInput, "rotation must be finite"};
    }
    const Matrix3 gram = rotation.transpose() * rotation;
    const double determinant = rotation.determinant();
    if (!gram.allFinite() || !std::isfinite(determinant) ||
        (gram - Matrix3::Identity()).cwiseAbs().maxCoeff() > kInputRoundoffAllowance ||
        std::abs(determinant - 1.0) > kInputRoundoffAllowance) {
        return Error{ErrorCode::NotProperRotation, "rotation must be in SO(3)"};
    }
    return std::nullopt;
}

inline Result<RotationBlocks> checkedBlocks(RotationBlocks blocks) {
    if (!blocks.dipole.allFinite() || !blocks.quadrupole.allFinite()) {
        return Result<RotationBlocks>::failure(
            {ErrorCode::NonFiniteResult, "non-finite harmonic rotation"});
    }
    return Result<RotationBlocks>::success(std::move(blocks));
}

// IR 计算基为正 (y,z,x)，m=-1,0,1；二阶为 (xy,yz,h1,xz,h4)。
inline RotationBlocks toPaperOrder(const Matrix3& ir_one, const Matrix5& ir_two) {
    RotationBlocks result;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result.dipole(i, j) = ir_one(kPaperDipoleFromIr[i], kPaperDipoleFromIr[j]);
        }
    }
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            result.quadrupole(i, j) =
                ir_two(kPaperQuadrupoleFromIr[i], kPaperQuadrupoleFromIr[j]);
        }
    }
    return result;
}

} // namespace egomotion::harmonics::low_order::detail
