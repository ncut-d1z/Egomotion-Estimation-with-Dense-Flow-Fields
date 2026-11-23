#pragma once

#include "egomotion/analysis/analyzer.hpp"

#include <array>

namespace egomotion::analysis::detail {

struct Segment { std::size_t begin=0,end=0; }; // MATLAB 闭区间，1-based
using Segments=Result<std::array<Segment,4>,AnalysisErrorCode>;
Segments fourSourceSegments(const std::vector<double>& ratio,double threshold);

struct Smoothed { std::vector<double> values; std::vector<unsigned char> defined; };
using SmoothedResult=Result<Smoothed,AnalysisErrorCode>;
SmoothedResult smoothSource(const std::vector<double>&,const std::vector<unsigned char>&,
    std::size_t half,const std::filesystem::path&,const std::string& variable,std::size_t first_matlab_frame);

} // namespace egomotion::analysis::detail
