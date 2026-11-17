#include "egomotion/harmonics/low_order.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace sh = egomotion::harmonics::low_order;
namespace {
std::size_t checks = 0;
double maximum_residual = 0;
constexpr double kPi = 3.141592653589793238462643383279502884;
// 沿用test_low_order_harmonics的工程回归余量，不定义或更改DEC-005。
constexpr double kTolerance = 4096.0 * std::numeric_limits<double>::epsilon();

void require(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
void near(double actual, double expected, const char* message) {
    require(std::isfinite(actual) && std::isfinite(expected), "finite comparison");
    const double residual = std::abs(actual - expected);
    maximum_residual = std::max(maximum_residual, residual);
    require(residual <= kTolerance, message);
}

void checkReconstruction(const sh::Direction& direction, const sh::SphericalAngles& angles) {
    const double c = std::cos(angles.theta_radians);
    const double s = std::sin(angles.theta_radians);
    const double phi = angles.phi_radians;
    near(s * std::cos(phi), direction.x(), "angles reconstruct x");
    near(s * std::sin(phi), direction.y(), "angles reconstruct y");
    near(c, direction.z(), "angles reconstruct z");

    // 独立的l<=2含CS关联Legendre解析式，按原l,m顺序；不是调用被测基来造期望值。
    const double n0 = 1 / std::sqrt(4 * kPi);
    const double n1 = std::sqrt(3 / (4 * kPi));
    const double n20 = std::sqrt(5 / (4 * kPi));
    const double n21 = std::sqrt(15 / (4 * kPi));
    const double n22 = std::sqrt(15 / (16 * kPi));
    const std::array<double, 9> source{{
        n0, -n1*s*std::sin(phi), n1*c, -n1*s*std::cos(phi),
        n22*s*s*std::sin(2*phi), -n21*c*s*std::sin(phi),
        n20*(3*c*c-1)/2, -n21*c*s*std::cos(phi), n22*s*s*std::cos(2*phi)}};
    constexpr std::array<int, 9> paper_index{{0,2,3,1,8,6,4,5,7}};
    constexpr std::array<double, 9> phase{{1,-1,1,-1,1,-1,1,-1,1}};
    const auto direct = sh::basisDirect(direction);
    const auto recurrence = sh::basisRecurrence(direction);
    require(direct.hasValue() && recurrence.hasValue(), "existing Cartesian bases succeed");
    for (std::size_t i = 0; i < source.size(); ++i) {
        near(phase[i]*direct.value()(paper_index[i]), source[i], "source angle/direct basis");
        near(phase[i]*recurrence.value()(paper_index[i]), source[i], "source angle/recurrence basis");
    }
}

void analyticDirections() {
    // 解析参考：轴角，3-4-5直角三角形，及(1,2,2)/3。
    // 十进制常数对应acos(4/5)、atan2(4,3)、acos(2/3)、atan2(2,1)；
    // 不在测试运行时调用被测API或acos/atan2生成这些期望角。
    constexpr double a = 0.643501108793284386802809228717322638;
    constexpr double b = 0.927295218001612232428512462922428804;
    struct Sample { sh::Direction direction; double theta; double phi; };
    const std::array<Sample, 18> samples{{
        {{1,0,0}, kPi/2, 0}, {{-1,0,0}, kPi/2, kPi},
        {{0,1,0}, kPi/2, kPi/2}, {{0,-1,0}, kPi/2, -kPi/2},
        {{0,0,1}, 0, 0}, {{0,0,-1}, kPi, 0},
        {{0.6,0.8,0}, kPi/2, b}, {{-0.6,0.8,0}, kPi/2, kPi-b},
        {{-0.6,-0.8,0}, kPi/2, -kPi+b}, {{0.6,-0.8,0}, kPi/2, -b},
        {{0.6,0,0.8}, a, 0}, {{0.6,0,-0.8}, kPi-a, 0},
        {{0,0.6,0.8}, a, kPi/2}, {{0,-0.6,0.8}, a, -kPi/2},
        {{0.8,0,0.6}, b, 0}, {{-0.8,0,-0.6}, kPi-b, kPi},
        {{1.0/3,2.0/3,2.0/3}, 0.841068670567930255776525031826430747,
                                1.10714871779409050301706546017853704},
        {{1.0/3,-2.0/3,-2.0/3}, kPi-0.841068670567930255776525031826430747,
                                -1.10714871779409050301706546017853704}
    }};
    for (const auto& sample : samples) {
        const auto before = sample.direction;
        auto result = sh::sourceAnglesFromDirection(sample.direction);
        require(result.hasValue(), "analytic direction accepted");
        const auto angles = result.value();
        near(angles.theta_radians, sample.theta, "analytic polar angle");
        near(angles.phi_radians, sample.phi, "analytic azimuth");
        checkReconstruction(sample.direction, angles);
        require(std::memcmp(before.data(), sample.direction.data(), 3*sizeof(double)) == 0,
                "input must remain unchanged");
        auto owned = std::move(result).value();
        owned.theta_radians = -1;
        require(angles.theta_radians >= 0 && owned.theta_radians == -1, "owning angle value");
    }
}

void signedZerosAndSeam() {
    for (double z : {-1.0, 1.0}) for (bool negative_x : {false,true})
        for (bool negative_y : {false,true}) {
            const double x = std::copysign(0.0, negative_x ? -1.0 : 1.0);
            const double y = std::copysign(0.0, negative_y ? -1.0 : 1.0);
            const auto result = sh::sourceAnglesFromDirection({x,y,z});
            require(result.hasValue(), "pole signed zeros accepted");
            const double phi = result.value().phi_radians;
            near(phi, negative_x ? (negative_y ? -kPi : kPi) : y, "pole atan2 convention");
            require(std::signbit(phi) == negative_y, "azimuth signbit preserved");
        }
    const double epsilon = std::numeric_limits<double>::epsilon();
    for (double y : {-epsilon, -0.0, 0.0, epsilon}) {
        const sh::Direction direction{-1,y,0};
        const auto result = sh::sourceAnglesFromDirection(direction);
        require(result.hasValue(), "azimuth seam accepted");
        require(std::signbit(result.value().phi_radians) == std::signbit(y), "seam side");
        near(std::abs(result.value().phi_radians), kPi, "seam magnitude");
        checkReconstruction(direction, result.value());
    }
}

void invalidInputs() {
    for (int component = 0; component < 3; ++component)
        for (double invalid : {std::numeric_limits<double>::quiet_NaN(),
                               std::numeric_limits<double>::infinity(),
                               -std::numeric_limits<double>::infinity()}) {
            sh::Direction direction = sh::Direction::UnitX();
            direction(component) = invalid;
            const auto result = sh::sourceAnglesFromDirection(direction);
            require(!result.hasValue() && result.error().code == sh::ErrorCode::NonFiniteInput,
                    "nonfinite direction rejected");
        }
    for (const sh::Direction& direction : std::array<sh::Direction,4>{{
            {0,0,0}, {2,0,0}, {0.3,0.4,0}, {std::numeric_limits<double>::max(),0,0}}}) {
        const auto result = sh::sourceAnglesFromDirection(direction);
        require(!result.hasValue() && result.error().code == sh::ErrorCode::NotUnitDirection,
                "nonunit direction rejected without normalization");
    }
    for (double sign : {-1.0,1.0}) {
        const sh::Direction outside{0,0,std::nextafter(sign, 2*sign)};
        require(sh::basisDirect(outside).hasValue(), "existing norm roundoff allowance unchanged");
        const auto result = sh::sourceAnglesFromDirection(outside);
        require(!result.hasValue() && result.error().code == sh::ErrorCode::NonFiniteResult,
                "acos domain must not be silently clamped");
    }
}
} // namespace

int main() {
    try {
        analyticDirections();
        signedZerosAndSeam();
        invalidInputs();
        std::cout << "source-angle checks=" << checks << ", analytic directions=18"
                  << ", maximum absolute residual=" << std::setprecision(17)
                  << maximum_residual << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "source angles: " << error.what() << '\n';
        return 1;
    }
}
