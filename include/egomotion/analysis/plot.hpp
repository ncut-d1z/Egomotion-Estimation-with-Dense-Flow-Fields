#pragma once

#include "egomotion/analysis/analyzer.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace egomotion {

enum class PlotErrorCode {
    BackendUnavailable, InvalidSeries, OutputForbidden, WriteFailed
};
template<> constexpr std::string_view errorDomain<PlotErrorCode>() noexcept {
    return "PlotErrorCode";
}
inline constexpr std::string_view errorName(PlotErrorCode x) noexcept {
    switch(x){
    case PlotErrorCode::BackendUnavailable:return "BackendUnavailable";
    case PlotErrorCode::InvalidSeries:return "InvalidSeries";
    case PlotErrorCode::OutputForbidden:return "OutputForbidden";
    case PlotErrorCode::WriteFailed:return "WriteFailed";
    }
    return "Unknown";
}

class IPlotExporter {
public:
    virtual ~IPlotExporter()=default;
    virtual Result<std::vector<std::string>,PlotErrorCode> exportPlot(
        const AnalysisDataset& data,ITextPublisher& output)=0;
};

namespace analysis {
// Zero external graphing dependencies; optional in the application.
// Generates SVG panels only from numeric AnalysisDataset, never reopens MAT.
class SvgPlotExporter final : public IPlotExporter {
public:
    Result<std::vector<std::string>,PlotErrorCode> exportPlot(
        const AnalysisDataset&,ITextPublisher&) override;
};
}  // namespace analysis
}  // namespace egomotion
