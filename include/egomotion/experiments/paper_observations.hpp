#pragma once

#include "egomotion/contracts/result.hpp"

#include <array>
#include <cstddef>
#include <functional>

namespace egomotion::experiments {

using PaperMetric = Result<Scalar,NumericalErrorCode>;

// 仅从本次成功求解取值。ywerte 编号和噪声列从 1 起算，细分级从 0 起算。
struct BiasPaperSample {
    std::size_t level, noise_column, direction_count, ywerte_number;
    double source_xwerte, noise_multiplier, source_chord;
    PaperMetric direction_degrees;
};
using BiasPaperObserver = std::function<void(const BiasPaperSample&)>;

enum class PaperUpdateSchedule { Archive, Article };
// Fig. 5E/F 图注给出的逐帧周期。
inline constexpr std::array<std::size_t,4> kArticleUpdatePeriods{{1,5,10,20}};

// 比较槽从 1 起算。Initialization/Comparison 的两槽依次为原 MFA 和 Adaptive。
// 两个旋转指标来自同一估计，各自保留数值或缺测原因。period=0 表示非周期实验。
struct AdaptivePaperSample {
    std::size_t repetition, matlab_frame, baseline, direction_count, period;
    bool refreshed;
    double ideal_rotation_mean, ideal_translation_mean;
    PaperMetric ideal_flow_ratio, translation_degrees, rotation_axis_degrees,
                rotation_geodesic_degrees;
};
using AdaptivePaperObserver = std::function<void(const AdaptivePaperSample&)>;

} // namespace egomotion::experiments
