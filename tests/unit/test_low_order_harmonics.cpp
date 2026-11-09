#include "egomotion/harmonics/low_order.hpp"

#include <Eigen/LU>
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace sh = egomotion::harmonics::low_order;
namespace {
std::size_t checks = 0;
// 专项回归比较余量，不宣称替代最终DEC-005阈值。
constexpr double kTolerance = 4096.0 * std::numeric_limits<double>::epsilon();
constexpr std::array<sh::RotationMethod, 3> kMethods{{
    sh::RotationMethod::TensorDirect, sh::RotationMethod::Ivanic1996,
    sh::RotationMethod::Choi1999}};

void require(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}

template <class Left, class Right>
void requireNear(const Eigen::MatrixBase<Left>& left,
                 const Eigen::MatrixBase<Right>& right, const char* message) {
    require(left.rows() == right.rows() && left.cols() == right.cols(), "shape mismatch");
    require(left.allFinite() && right.allFinite(), "non-finite comparison");
    require((left - right).cwiseAbs().maxCoeff() <= kTolerance, message);
}

sh::Matrix3 quaternionRotation(double w, double x, double y, double z) {
    const double s = w*w + x*x + y*y + z*z;
    sh::Matrix3 r;
    r << w*w+x*x-y*y-z*z, 2*(x*y-w*z), 2*(x*z+w*y),
         2*(x*y+w*z), w*w-x*x+y*y-z*z, 2*(y*z-w*x),
         2*(x*z-w*y), 2*(y*z+w*x), w*w-x*x-y*y+z*z;
    return r / s;
}

std::vector<sh::Matrix3> rotations() {
    std::vector<sh::Matrix3> result;
    std::array<int, 3> permutation{{0,1,2}};
    do {
        for (int mask = 0; mask < 8; ++mask) {
            sh::Matrix3 r = sh::Matrix3::Zero();
            for (int i = 0; i < 3; ++i) r(i, permutation[i]) = (mask & (1 << i)) ? -1.0 : 1.0;
            if (r.determinant() > 0.0) result.push_back(r);
        }
    } while (std::next_permutation(permutation.begin(), permutation.end()));
    require(result.size() == 24, "all proper signed permutations must be included");
    result.push_back(quaternionRotation(1,2,2,0));
    result.push_back(quaternionRotation(2,3,5,7));
    result.push_back(quaternionRotation(-2,-3,-5,-7));
    result.push_back(quaternionRotation(1,1e-12,0,0));
    result.push_back(quaternionRotation(1e-12,1,2,3));
    result.push_back(quaternionRotation(1+std::sqrt(2.0),1,0,0));
    return result;
}

std::vector<sh::Direction> directions() {
    return {sh::Direction(1,0,0), sh::Direction(-1,0,0), sh::Direction(0,1,0),
        sh::Direction(0,-1,0), sh::Direction(0,0,1), sh::Direction(0,0,-1),
        sh::Direction(1,2,3).normalized(), sh::Direction(-2,1,4).normalized(),
        sh::Direction(1e-12,-2e-12,1).normalized(),
        sh::Direction(1,2,1e-12).normalized()};
}

void testBasisAndAction() {
    const auto ds = directions();
    for (const auto& d : ds) {
        const auto direct = sh::basisDirect(d);
        const auto recurrent = sh::basisRecurrence(d);
        require(direct.hasValue() && recurrent.hasValue(), "basis evaluation failed");
        requireNear(direct.value(), recurrent.value(), "basis direct/recurrence discrepancy");
    }
    for (const auto& r : rotations()) {
        const auto direct = sh::rotationDirect(r);
        const auto ivanic = sh::rotationIvanic(r);
        const auto choi = sh::rotationChoi(r);
        require(direct.hasValue() && ivanic.hasValue() && choi.hasValue(), "rotation failed");
        requireNear(direct.value().dipole, r.transpose(), "direct dipole transpose");
        requireNear(ivanic.value().dipole, r.transpose(), "IR dipole transpose");
        requireNear(choi.value().dipole, r.transpose(), "Choi dipole transpose");
        requireNear(ivanic.value().quadrupole, direct.value().quadrupole, "IR/tensor mismatch");
        requireNear(choi.value().quadrupole, direct.value().quadrupole, "Choi/tensor mismatch");
        requireNear(direct.value().quadrupole.transpose()*direct.value().quadrupole,
                    sh::Matrix5::Identity(), "quadrupole orthogonality");
        for (const auto& d : ds) {
            const auto expected = sh::basisDirect(r*d);
            require(expected.hasValue(), "rotated direction invalid");
            for (int component = 0; component < sh::kCoefficientCount; ++component) {
                sh::Coefficients unit = sh::Coefficients::Zero();
                unit(component) = 1.0;
                for (const auto method : kMethods) {
                    const auto moved = sh::rotate(unit, r, method);
                    require(moved.hasValue(), "coefficient rotation failed");
                    const auto observed = sh::reconstruct(moved.value(), d);
                    require(observed.hasValue(), "field reconstruction failed");
                    require(std::abs(observed.value()-expected.value()(component)) <= kTolerance,
                            "mu_new(d) must equal mu_old(A*d), for every basis coefficient");
                }
            }
        }
    }
}

void testIndependentReference() {
    // 独立张量/精确算术参考，Wolfram结果写于专项报告；不是被测递推自生成。
    const sh::Matrix3 r = quaternionRotation(1,2,2,0);
    const double root_three = std::sqrt(3.0);
    sh::Matrix5 expected;
    expected << 11.0/27, -28.0/(27*root_three), 28.0/(27*root_three), 0, -16.0/(27*root_three),
        28.0/(27*root_three), -23.0/81, -40.0/81, 4.0/9, 28.0/81,
        -28.0/(27*root_three), -40.0/81, -23.0/81, 4.0/9, -28.0/81,
        0, -4.0/9, -4.0/9, -7.0/9, 0,
        -16.0/(27*root_three), -28.0/81, 28.0/81, 0, 65.0/81;
    const auto direct = sh::rotationDirect(r);
    const auto ivanic = sh::rotationIvanic(r);
    const auto choi = sh::rotationChoi(r);
    require(direct.hasValue() && ivanic.hasValue() && choi.hasValue(), "reference rotation failed");
    requireNear(direct.value().quadrupole, expected, "direct/exact reference mismatch");
    requireNear(ivanic.value().quadrupole, expected, "IR/exact reference mismatch");
    requireNear(choi.value().quadrupole, expected, "Choi/exact reference mismatch");
    const auto rx45 = sh::rotationIvanic(quaternionRotation(1+std::sqrt(2.0),1,0,0));
    require(rx45.hasValue(), "x45 failed");
    require(std::abs(rx45.value().quadrupole(2,0)-root_three/2) <= kTolerance,
            "IR98 printed minus-factor mutation must be rejected");
}

void testCompositionAndLayout() {
    const sh::Matrix3 rx = quaternionRotation(1,1,0,0);
    const sh::Matrix3 ry = quaternionRotation(1,0,1,0);
    require((rx*ry-ry*rx).cwiseAbs().maxCoeff() > 0.5, "composition fixture must not commute");
    const auto combined = sh::rotationIvanic(rx*ry);
    const auto bx = sh::rotationIvanic(rx);
    const auto by = sh::rotationIvanic(ry);
    require(combined.hasValue() && bx.hasValue() && by.hasValue(), "composition failed");
    requireNear(combined.value().quadrupole, by.value().quadrupole*bx.value().quadrupole,
                "pullback coefficient composition must reverse order");
    sh::Coefficients coefficients;
    for (int i = 0; i < sh::kCoefficientCount; ++i) coefficients(i) = i-4.0;
    const auto matlab = sh::toMatlabLayout(coefficients);
    require(matlab.hasValue(), "layout conversion failed");
    const auto roundtrip = sh::fromMatlabLayout(matlab.value());
    require(roundtrip.hasValue(), "inverse layout conversion failed");
    requireNear(roundtrip.value(), coefficients, "layout roundtrip");
    require(matlab.value()(0,4) == 0.0 && matlab.value()(1,4) == 0.0, "padding must be zero");
    sh::Coefficients z_coeff = sh::Coefficients::Zero();
    z_coeff(3) = 1.0;
    const auto moved = sh::rotate(z_coeff,rx,sh::RotationMethod::Ivanic1996);
    require(moved.hasValue(), "x90 action failed");
    sh::Coefficients y_coeff = sh::Coefficients::Zero();
    y_coeff(2) = 1.0;
    requireNear(moved.value(),y_coeff,"x90 must map the old positive z field to positive y");
    const auto moved_matlab = sh::toMatlabLayout(moved.value());
    require(moved_matlab.hasValue() && moved_matlab.value()(1,0) == -1.0,
            "MATLAB CS phase requires negative m=-1 coefficient for positive y");
}

void testFailures() {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    require(!sh::basisDirect(sh::Direction::Zero()).hasValue(), "zero direction accepted");
    require(!sh::basisRecurrence(sh::Direction(2,0,0)).hasValue(), "nonunit direction accepted");
    require(!sh::basisDirect(sh::Direction(nan,0,1)).hasValue(), "NaN direction accepted");
    require(!sh::basisRecurrence(sh::Direction(inf,0,1)).hasValue(), "Inf direction accepted");
    sh::Matrix3 reflection = sh::Matrix3::Identity(); reflection(0,0) = -1.0;
    sh::Matrix3 bad = sh::Matrix3::Identity(); bad(0,0) = nan;
    const sh::Coefficients zero = sh::Coefficients::Zero();
    for (const auto method : kMethods) {
        require(!sh::rotate(zero,reflection,method).hasValue(), "reflection accepted as SO(3)");
        require(!sh::rotate(zero,sh::Matrix3::Zero(),method).hasValue(), "zero matrix accepted");
        require(!sh::rotate(zero,2.0*sh::Matrix3::Identity(),method).hasValue(), "scaled matrix accepted");
        require(!sh::rotate(zero,bad,method).hasValue(), "NaN matrix accepted");
    }
    sh::Coefficients bad_coeff = zero; bad_coeff(0) = inf;
    require(!sh::reconstruct(bad_coeff,sh::Direction(1,0,0)).hasValue(), "Inf coefficients accepted");
    require(!sh::rotate(zero,sh::Matrix3::Identity(),static_cast<sh::RotationMethod>(999)).hasValue(),
            "invalid method accepted");
}
} // namespace

int main() {
    try {
        testBasisAndAction();
        testIndependentReference();
        testCompositionAndLayout();
        testFailures();
        std::cout << "PASS spherical-harmonics checks=" << checks << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL after checks=" << checks << ": " << error.what() << '\n';
        return 1;
    }
}
