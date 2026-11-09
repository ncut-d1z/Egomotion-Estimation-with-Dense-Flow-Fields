#include "egomotion/harmonics/low_order.hpp"
#include "low_order_detail.hpp"

#include <array>
#include <cmath>
#include <vector>

namespace egomotion::harmonics::low_order {
namespace {

struct SourceChannel {
    int row;
    int column;
    int canonical_index;
    double phase;
};

// 原 MATLAB 按 l=0..2、m=-l..l 遍历；所有相位对应原 CS legendre。
// 只复用已定义正确的九个 canonical 基值，避免复制一套不受审计的球谐公式。
constexpr std::array<SourceChannel, 9> kSourceChannels{{
    {0, 0, 0,  1.0},
    {1, 0, 2, -1.0},
    {1, 1, 3,  1.0},
    {1, 2, 1, -1.0},
    {2, 0, 8,  1.0},
    {2, 1, 6, -1.0},
    {2, 2, 4,  1.0},
    {2, 3, 5, -1.0},
    {2, 4, 7,  1.0}
}};

} // namespace

Result<MatlabCoefficients> projectSourceCoefficientsOrderTwo(
    const Eigen::VectorXd& nearness, const Eigen::MatrixXd& setofpoints) {
    const Eigen::Index count = setofpoints.rows();
    if (count <= 0 || setofpoints.cols() < 3 || nearness.size() != count) {
        return Result<MatlabCoefficients>::failure(
            {ErrorCode::InvalidSampleShape, "expected nonempty N nearnesses and N-by-K points (K>=3)"});
    }
    if (!nearness.allFinite()) {
        return Result<MatlabCoefficients>::failure(
            {ErrorCode::NonFiniteInput, "nearness vector must be finite"});
    }

    std::vector<BasisValues> basis_by_row;
    basis_by_row.reserve(static_cast<std::size_t>(count));
    for (Eigen::Index row = 0; row < count; ++row) {
        const Direction direction(setofpoints(row, 0), setofpoints(row, 1),
                                  setofpoints(row, 2));
        const auto basis = basisDirect(direction);
        if (!basis.hasValue()) {
            return Result<MatlabCoefficients>::failure(basis.error());
        }
        basis_by_row.push_back(basis.value());
    }

    MatlabCoefficients output = MatlabCoefficients::Zero();
    const double count_double = static_cast<double>(count);
    for (const auto& channel : kSourceChannels) {
        double sum_numerator = 0.0;
        double sum_denominator = 0.0;
        for (Eigen::Index row = 0; row < count; ++row) {
            const double y = channel.phase *
                basis_by_row[static_cast<std::size_t>(row)](channel.canonical_index);
            sum_numerator += y * nearness(row);
            sum_denominator += y * y;
        }
        // source koeff_aus_near.m: a=(a/N)*4*pi, b=(b/N)*4*pi, a=a/b.
        const double numerator = (sum_numerator / count_double) * (4.0 * detail::kPi);
        const double denominator = (sum_denominator / count_double) * (4.0 * detail::kPi);
        if (!std::isfinite(numerator) || !std::isfinite(denominator)) {
            return Result<MatlabCoefficients>::failure(
                {ErrorCode::NonFiniteResult, "non-finite source harmonic projection accumulation"});
        }
        if (denominator == 0.0) {
            return Result<MatlabCoefficients>::failure(
                {ErrorCode::ZeroProjectionNorm, "source harmonic has zero squared norm on samples"});
        }
        const double projected = numerator / denominator;
        if (!std::isfinite(projected)) {
            return Result<MatlabCoefficients>::failure(
                {ErrorCode::NonFiniteResult, "non-finite source harmonic projection ratio"});
        }
        output(channel.row, channel.column) = projected;
    }
    return Result<MatlabCoefficients>::success(std::move(output));
}

} // namespace egomotion::harmonics::low_order
