#include "egomotion/harmonics/low_order.hpp"

#include <Eigen/LU>
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace sh = egomotion::harmonics::low_order;
namespace {
std::size_t checks = 0;
std::size_t checkedQuaternionGrid = 0;
double maximumMatrixVectorResidual = 0.0;
double maximumScalarResidual = 0.0;
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
    const double residual = (left - right).cwiseAbs().maxCoeff();
    maximumMatrixVectorResidual = std::max(maximumMatrixVectorResidual, residual);
    require(residual <= kTolerance, message);
}

void requireScalarNear(double observed, double expected, const char* message) {
    require(std::isfinite(observed) && std::isfinite(expected),
            "non-finite scalar comparison");
    const double residual = std::abs(observed - expected);
    maximumScalarResidual = std::max(maximumScalarResidual, residual);
    require(residual <= kTolerance, message);
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
                    requireScalarNear(observed.value(), expected.value()(component),
                                      "mu_new(d) must equal mu_old(A*d), for every basis coefficient");
                }
            }
        }
    }
}

// 源独立的基函数锚点：不能只用直接式与递推式互比，两条路径可能共用同一个错系数。
void testAnalyticBasisAndDyadic() {
    const double pi = std::acos(-1.0);
    const double g0 = 1.0 / std::sqrt(4.0 * pi);
    const double dipole = std::sqrt(3.0 / (4.0 * pi));
    const double zonal = std::sqrt(5.0 / (16.0 * pi));
    const double difference = std::sqrt(15.0 / (16.0 * pi));
    const double mixed = std::sqrt(15.0 / (4.0 * pi));

    const std::array<sh::Direction, 4> axes{{
        sh::Direction::UnitX(), sh::Direction::UnitY(),
        sh::Direction::UnitZ(), sh::Direction(1, 1, 1).normalized()
    }};
    std::array<sh::BasisValues, 4> expected;
    expected[0] << g0, dipole, 0, 0, -zonal, 0, 0, difference, 0;
    expected[1] << g0, 0, dipole, 0, -zonal, 0, 0, -difference, 0;
    expected[2] << g0, 0, 0, dipole, 2*zonal, 0, 0, 0, 0;
    const double inv_sqrt_three = 1.0 / std::sqrt(3.0);
    expected[3] << g0, dipole*inv_sqrt_three, dipole*inv_sqrt_three,
        dipole*inv_sqrt_three, 0, mixed/3.0, mixed/3.0, 0, mixed/3.0;

    for (std::size_t index = 0; index < axes.size(); ++index) {
        const auto direct = sh::basisDirect(axes[index]);
        const auto recurrence = sh::basisRecurrence(axes[index]);
        require(direct.hasValue() && recurrence.hasValue(), "analytic basis input rejected");
        requireNear(direct.value(), expected[index], "direct basis vs independently specified anchor");
        requireNear(recurrence.value(), expected[index], "recurrence basis vs independently specified anchor");
    }

    // 2015 p.30 的 d1^2/d2^2 展开把 h4 符号写反；式(63)--(67)定义要求下述恒等式。
    const double isotropic_scale = std::sqrt(4.0*pi/9.0);
    const double zonal_scale = std::sqrt(4.0*pi/45.0);
    const double quadrupole_scale = std::sqrt(4.0*pi/15.0);
    for (const auto& direction : directions()) {
        const auto result = sh::basisDirect(direction);
        require(result.hasValue(), "dyadic basis input rejected");
        const auto& h = result.value();
        const double isotropic = isotropic_scale*h(0);
        const double zterm = zonal_scale*h(4);
        requireScalarNear(isotropic-zterm+quadrupole_scale*h(7), direction.x()*direction.x(),
                          "2015 dyadic xx h4 sign");
        requireScalarNear(isotropic-zterm-quadrupole_scale*h(7), direction.y()*direction.y(),
                          "2015 dyadic yy h4 sign");
        requireScalarNear(isotropic+2.0*zterm, direction.z()*direction.z(), "2015 dyadic zz");
        requireScalarNear(quadrupole_scale*h(5), direction.x()*direction.z(), "2015 dyadic xz");
        requireScalarNear(quadrupole_scale*h(6), direction.y()*direction.z(), "2015 dyadic yz");
        requireScalarNear(quadrupole_scale*h(8), direction.x()*direction.y(), "2015 dyadic xy");
    }
}

void testMatlabSourcePhase() {
    sh::MatlabCoefficients matlab = sh::MatlabCoefficients::Zero();
    matlab(0,0)=2.0;
    matlab(1,0)=3.0; matlab(1,1)=5.0; matlab(1,2)=7.0;
    matlab(2,0)=11.0; matlab(2,1)=13.0; matlab(2,2)=17.0;
    matlab(2,3)=19.0; matlab(2,4)=23.0;
    // 源 3x5 中不属于 l<=2 的位置不得参与求和。
    matlab(0,1)=101.0; matlab(0,2)=103.0; matlab(0,3)=107.0;
    matlab(0,4)=109.0; matlab(1,3)=113.0; matlab(1,4)=127.0;
    const auto converted = sh::fromMatlabLayout(matlab);
    require(converted.hasValue(), "legacy matlab coefficient conversion failed");
    const double pi = std::acos(-1.0);
    const double d1 = std::sqrt(3.0/(4.0*pi));
    const double d2z = std::sqrt(5.0/(16.0*pi));
    const double d2mixed = std::sqrt(15.0/(4.0*pi));
    const double d2diff = std::sqrt(15.0/(16.0*pi));
    for (const auto& d : directions()) {
        // 这是原 MATLAB 含 Condon-Shortley 相位的九项多项式，独立于布局转换实现。
        const double legacy = matlab(0,0)/std::sqrt(4.0*pi)
            + d1*(-matlab(1,0)*d.y() + matlab(1,1)*d.z() - matlab(1,2)*d.x())
            + d2mixed*(matlab(2,0)*d.x()*d.y() - matlab(2,1)*d.y()*d.z()
                        - matlab(2,3)*d.x()*d.z())
            + d2z*matlab(2,2)*(3.0*d.z()*d.z()-1.0)
            + d2diff*matlab(2,4)*(d.x()*d.x()-d.y()*d.y());
        const auto result = sh::reconstruct(converted.value(), d);
        require(result.hasValue(), "converted MATLAB polynomial reconstruction failed");
        requireScalarNear(result.value(), legacy,
                          "MATLAB CS phase or invalid padding changed the scalar field");
    }
}

// 有限且确定的四元数格点：不使用 RNG，也不把有限样本宣称为全域数学证明。
// 目的：在已由 Wolfram 全域证明的恒等式之外，检查当前 GCC/Eigen double 执行路径。
void testDeterministicQuaternionGrid() {
    constexpr std::array<int, 5> components{{-3, -1, 0, 1, 3}};
    const sh::Direction generalDirection = sh::Direction(2, -1, 3).normalized();
    const sh::Coefficients nontrivial = (sh::Coefficients() <<
        1.0, -3.0, 2.0, 5.0, -7.0, 11.0, -13.0, 17.0, -19.0).finished();
    for (const int w : components) {
        for (const int x : components) {
            for (const int y : components) {
                for (const int z : components) {
                    if (w == 0 && x == 0 && y == 0 && z == 0) continue;
                    const sh::Matrix3 rotation = quaternionRotation(w, x, y, z);
                    const auto tensor = sh::rotationDirect(rotation);
                    const auto ivanic = sh::rotationIvanic(rotation);
                    const auto choi = sh::rotationChoi(rotation);
                    require(tensor.hasValue() && ivanic.hasValue() && choi.hasValue(),
                            "quaternion-grid SO(3) rejected");
                    requireNear(tensor.value().dipole, rotation.transpose(),
                                "quaternion-grid direct dipole");
                    requireNear(ivanic.value().dipole, tensor.value().dipole,
                                "quaternion-grid Ivanic dipole");
                    requireNear(choi.value().dipole, tensor.value().dipole,
                                "quaternion-grid Choi dipole");
                    requireNear(ivanic.value().quadrupole, tensor.value().quadrupole,
                                "quaternion-grid Ivanic quadrupole");
                    requireNear(choi.value().quadrupole, tensor.value().quadrupole,
                                "quaternion-grid Choi quadrupole");
                    requireNear(choi.value().quadrupole.transpose() * choi.value().quadrupole,
                                sh::Matrix5::Identity(), "quaternion-grid quadrupole orthogonality");
                    const auto expectedField = sh::reconstruct(nontrivial, rotation*generalDirection);
                    require(expectedField.hasValue(), "quaternion-grid expected field failed");
                    for (const auto method : kMethods) {
                        const auto coefficients = sh::rotate(nontrivial, rotation, method);
                        require(coefficients.hasValue(), "quaternion-grid coefficient rotation failed");
                        const auto actualField = sh::reconstruct(coefficients.value(), generalDirection);
                        require(actualField.hasValue(), "quaternion-grid actual field failed");
                        requireScalarNear(actualField.value(), expectedField.value(),
                                          "quaternion-grid field action");
                    }
                    ++checkedQuaternionGrid;
                }
            }
        }
    }
    require(checkedQuaternionGrid == 624, "quaternion-grid count mismatch");
}

void testFailureTaxonomyAndFiniteOverflow() {
    const double inf = std::numeric_limits<double>::infinity();
    const double maximum = std::numeric_limits<double>::max();
    const auto invalidDirection = sh::basisDirect(sh::Direction(2, 0, 0));
    require(!invalidDirection.hasValue(), "nonunit direction accepted");
    require(invalidDirection.error().code == sh::ErrorCode::NotUnitDirection,
            "nonunit direction wrong error code");

    const auto infiniteDirection = sh::basisRecurrence(sh::Direction(inf, 0, 0));
    require(!infiniteDirection.hasValue(), "infinite direction accepted");
    require(infiniteDirection.error().code == sh::ErrorCode::NonFiniteInput,
            "infinite direction wrong error code");

    sh::Matrix3 reflection = sh::Matrix3::Identity();
    reflection(0, 0) = -1.0;
    const sh::Coefficients zero = sh::Coefficients::Zero();
    for (const auto method : kMethods) {
        const auto result = sh::rotate(zero, reflection, method);
        require(!result.hasValue(), "reflection incorrectly accepted");
        require(result.error().code == sh::ErrorCode::NotProperRotation,
                "reflection wrong error code");
    }

    const auto invalidMethod =
        sh::rotate(zero, sh::Matrix3::Identity(), static_cast<sh::RotationMethod>(999));
    require(!invalidMethod.hasValue(), "invalid rotation method accepted");
    require(invalidMethod.error().code == sh::ErrorCode::InvalidMethod,
            "invalid rotation method wrong error code");

    sh::MatlabCoefficients badPadding = sh::MatlabCoefficients::Zero();
    badPadding(0, 4) = inf;
    const auto layout = sh::fromMatlabLayout(badPadding);
    require(!layout.hasValue(), "nonfinite MATLAB padding accepted");
    require(layout.error().code == sh::ErrorCode::NonFiniteInput,
            "nonfinite MATLAB padding wrong error code");

    sh::Coefficients large = sh::Coefficients::Zero();
    large(1) = maximum; // at e_x both f1 and h4 are positive
    large(7) = maximum; // their positive weighted sum exceeds DBL_MAX
    const auto overflow = sh::reconstruct(large, sh::Direction::UnitX());
    require(!overflow.hasValue(), "finite input with overflowing reconstruction accepted");
    require(overflow.error().code == sh::ErrorCode::NonFiniteResult,
            "overflowing reconstruction wrong error code");
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
    requireScalarNear(rx45.value().quadrupole(2,0), root_three/2,
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
        testAnalyticBasisAndDyadic();
        testMatlabSourcePhase();
        testDeterministicQuaternionGrid();
        testIndependentReference();
        testCompositionAndLayout();
        testFailures();
        testFailureTaxonomyAndFiniteOverflow();
        std::cout << std::setprecision(17) << "PASS spherical-harmonics checks=" << checks
                  << " quaternion_grid=" << checkedQuaternionGrid
                  << " max_matrix_vector_abs_residual=" << maximumMatrixVectorResidual
                  << " max_scalar_abs_residual=" << maximumScalarResidual
                  << " regression_tolerance=" << kTolerance << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL after checks=" << checks << ": " << error.what() << '\n';
        return 1;
    }
}
