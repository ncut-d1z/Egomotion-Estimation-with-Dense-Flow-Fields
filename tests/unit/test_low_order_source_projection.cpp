#include "egomotion/harmonics/low_order.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace sh = egomotion::harmonics::low_order;
namespace {
std::size_t checks = 0;
double maximum_residual = 0.0;
// Regression tolerance only, not the owner-approved DEC-005 scientific threshold.
constexpr double kTolerance = 4096.0 * std::numeric_limits<double>::epsilon();

void require(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
void requireNear(double actual, double expected, const char* message) {
    require(std::isfinite(actual) && std::isfinite(expected), "nonfinite projection comparison");
    const double residual = std::abs(actual - expected);
    maximum_residual = std::max(maximum_residual, residual);
    require(residual <= kTolerance, message);
}

struct Case {
    Eigen::MatrixXd points;
    Eigen::VectorXd near;
};

Case nonSymmetricSamples() {
    // Seven precisely defined unit directions. Only the first three columns are interpreted.
    // The fourth/sixth column data are ignored just as in koeff_aus_near.m.
    Case input{Eigen::MatrixXd::Zero(7, 6), Eigen::VectorXd(7)};
    input.points << 1,0,0,0,0,0,
                    0,1,0,0,0,0,
                    0,0,1,0,0,0,
                    3.0/5,4.0/5,0,0,0,0,
                    0,3.0/5,4.0/5,0,0,0,
                    4.0/5,0,3.0/5,0,0,0,
                    1.0/3,2.0/3,2.0/3,0,0,0;
    input.near << 2,-1,4,1.0/2,-3,5.0/4,11.0/2;
    return input;
}

sh::MatlabCoefficients independentWolframReference() {
    // Independently derived exact original scalar projection:
    // SUM_i(near_i Y_lm(d_i)) / SUM_i(Y_lm(d_i)^2).
    // The nonzero off-diagonal Gram entries make this distinguishable from joint LS.
    const double pi = std::acos(-1.0);
    const double sqrt_pi = std::sqrt(pi);
    const double sqrt_3pi = std::sqrt(3*pi);
    const double sqrt_5pi = std::sqrt(5*pi);
    const double sqrt_15pi = std::sqrt(15*pi);
    sh::MatlabCoefficients expected = sh::MatlabCoefficients::Zero();
    expected(0,0) = 37*sqrt_pi/14;
    expected(1,0) = -19*sqrt_3pi/55;
    expected(1,1) = 361*sqrt_3pi/220;
    expected(1,2) = -154*sqrt_3pi/95;
    expected(2,0) = 4935*sqrt_15pi/7082;
    expected(2,1) = -1695*sqrt_15pi/5416;
    expected(2,2) = 25530*sqrt_5pi/44797;
    expected(2,3) = -3075*sqrt_15pi/3541;
    expected(2,4) = 4360*sqrt_15pi/15349;
    return expected;
}

void testNonSymmetricSourceProjection() {
    const auto input = nonSymmetricSamples();
    const auto fitted = sh::projectSourceCoefficientsOrderTwo(input.near, input.points);
    require(fitted.hasValue(), "seven-direction source projection failed");
    const auto reference = independentWolframReference();
    for (int row=0; row<3; ++row) {
        for (int column=0; column<5; ++column) {
            requireNear(fitted.value()(row,column), reference(row,column),
                        "source projection differs from independently derived coefficient");
        }
    }
    // The original MATLAB padding is zero-filled, NOT an extra usable coefficient.
    for (int column=1; column<5; ++column) {
        require(fitted.value()(0,column)==0.0, "l=0 invalid column nonzero");
    }
    for (int column=3; column<5; ++column) {
        require(fitted.value()(1,column)==0.0, "l=1 invalid column nonzero");
    }
    // Trailing flow/metadata columns are not used by the source formula.
    Case ignored = nonSymmetricSamples();
    ignored.points(0,5) = std::numeric_limits<double>::infinity();
    const auto unchanged = sh::projectSourceCoefficientsOrderTwo(ignored.near,ignored.points);
    require(unchanged.hasValue(), "ignored trailing column rejected");
    for (int row=0; row<3; ++row)
        for (int column=0; column<5; ++column)
            requireNear(unchanged.value()(row,column), reference(row,column),
                        "trailing non-harmonic columns affected projection");
}

void testProjectionErrors() {
    Case input = nonSymmetricSamples();
    const Eigen::VectorXd emptyNear(0);
    const Eigen::MatrixXd emptyPoints(0, 6);
    auto result = sh::projectSourceCoefficientsOrderTwo(emptyNear, emptyPoints);
    require(!result.hasValue() && result.error().code==sh::ErrorCode::InvalidSampleShape,
            "N=0 should be rejected as invalid sample shape");
    result = sh::projectSourceCoefficientsOrderTwo(input.near.head(6), input.points);
    require(!result.hasValue() && result.error().code==sh::ErrorCode::InvalidSampleShape,
            "mismatched near/point row count must be rejected");
    result = sh::projectSourceCoefficientsOrderTwo(input.near,input.points.leftCols(2));
    require(!result.hasValue() && result.error().code==sh::ErrorCode::InvalidSampleShape,
            "missing direction columns must be rejected");

    input = nonSymmetricSamples();
    input.points(0,0)=2.0;
    result = sh::projectSourceCoefficientsOrderTwo(input.near,input.points);
    require(!result.hasValue() && result.error().code==sh::ErrorCode::NotUnitDirection,
            "nonunit source sample must be rejected");
    input = nonSymmetricSamples();
    input.points(0,0)=std::numeric_limits<double>::quiet_NaN();
    result = sh::projectSourceCoefficientsOrderTwo(input.near,input.points);
    require(!result.hasValue() && result.error().code==sh::ErrorCode::NonFiniteInput,
            "nonfinite direction sample must be rejected");
    input = nonSymmetricSamples();
    input.near(0)=std::numeric_limits<double>::infinity();
    result = sh::projectSourceCoefficientsOrderTwo(input.near,input.points);
    require(!result.hasValue() && result.error().code==sh::ErrorCode::NonFiniteInput,
            "nonfinite nearness must be rejected");

    // For an axis-only support many harmonic basis functions vanish; do not return NaNs.
    Eigen::MatrixXd pole(1,3);
    pole << 0,0,1;
    Eigen::VectorXd near(1); near << 1.0;
    result = sh::projectSourceCoefficientsOrderTwo(near,pole);
    require(!result.hasValue() && result.error().code==sh::ErrorCode::ZeroProjectionNorm,
            "zero harmonic norm must be detected");
    input = nonSymmetricSamples();
    input.near.setConstant(std::numeric_limits<double>::max());
    result = sh::projectSourceCoefficientsOrderTwo(input.near,input.points);
    require(!result.hasValue() && result.error().code==sh::ErrorCode::NonFiniteResult,
            "finite large near must not overflow into successful coefficient");
}
} // namespace

int main() {
    try {
        testNonSymmetricSourceProjection();
        testProjectionErrors();
        std::cout << std::setprecision(17)
                  << "PASS source SH scalar projection checks=" << checks
                  << " max_abs_residual=" << maximum_residual
                  << " regression_tolerance=" << kTolerance << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL source SH scalar projection after checks=" << checks
                  << ": " << error.what() << '\n';
        return 1;
    }
}
