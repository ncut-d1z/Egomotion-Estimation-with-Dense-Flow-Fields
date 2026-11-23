#pragma once

#include "egomotion/experiments/paper_observations.hpp"
#include "egomotion/contracts/interfaces.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace egomotion::paper {

struct FigureConfig {
    unsigned figure;
    bool svg;
    std::optional<std::uint32_t> comparison_seed;
    std::optional<experiments::PaperUpdateSchedule> schedule;
};

// 按论文面板独立生成。样本表与图形由调用方的独占 RunDirectory 发布。
using FigureFiles=Result<std::vector<std::string>,ExperimentErrorCode>;
FigureFiles produce(const FigureConfig&,ITextPublisher&);
bool svgAvailable() noexcept;
std::vector<std::string> sourceBlobs(unsigned figure);

} // namespace egomotion::paper
