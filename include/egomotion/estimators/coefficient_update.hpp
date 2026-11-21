#pragma once

#include "egomotion/contracts/result.hpp"

#include <array>
#include <cstddef>

namespace egomotion::estimators {

// agent_in_sphere_adaptive_matched_filter_many_parameter.m: zeitkonstanten。
inline constexpr std::array<std::size_t,4> kAdaptiveUpdatePeriods{{1,5,11,21}};

class ICoefficientUpdatePolicy {
public:
    virtual ~ICoefficientUpdatePolicy() = default;
    // 只准备比较输入；primary已按入口完成旋转、尚未本帧求解。
    // 所有引用仅在调用期间借用，策略不持有或提交Session的可变状态。
    virtual Result<HarmonicCoefficients,NumericalErrorCode> prepare(
        std::size_t matlab_frame_index,
        const HarmonicCoefficients& primary_before_solve,
        const HarmonicCoefficients& previous_baseline,
        const Matrix3& source_rotation) const = 0;
};

class ResetCoefficientUpdatePolicy final : public ICoefficientUpdatePolicy {
public:
    Result<HarmonicCoefficients,NumericalErrorCode> prepare(
        std::size_t, const HarmonicCoefficients&, const HarmonicCoefficients&,
        const Matrix3&) const override;
};
class InheritCoefficientUpdatePolicy final : public ICoefficientUpdatePolicy {
public:
    Result<HarmonicCoefficients,NumericalErrorCode> prepare(
        std::size_t, const HarmonicCoefficients&, const HarmonicCoefficients&,
        const Matrix3&) const override;
};
class PeriodicCoefficientUpdatePolicy final : public ICoefficientUpdatePolicy {
public:
    explicit PeriodicCoefficientUpdatePolicy(std::size_t period) noexcept : period_(period) {}
    bool refreshesAt(std::size_t matlab_frame) const noexcept {
        return period_!=0 && matlab_frame>=2 && matlab_frame%period_==0;
    }
    Result<HarmonicCoefficients,NumericalErrorCode> prepare(
        std::size_t, const HarmonicCoefficients&, const HarmonicCoefficients&,
        const Matrix3&) const override;
private:
    const std::size_t period_;
};

using AdaptiveCoefficientPolicies = std::array<const ICoefficientUpdatePolicy*,4>;

} // namespace egomotion::estimators
