#pragma once

#include "egomotion/contracts/interfaces.hpp"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace egomotion {

enum class AnalysisKind {
    Bias, AdaptiveInitialization, AdaptiveComparison, AdaptiveUpdateRates
};
enum class AnalysisErrorCode {
    MissingResult, MissingVariable, ShapeMismatch, InvalidSegment,
    NonFiniteInput, UndefinedStatistic, UnsupportedExperiment
};

template<> constexpr std::string_view errorDomain<AnalysisErrorCode>() noexcept {
    return "AnalysisErrorCode";
}
inline constexpr std::string_view errorName(AnalysisErrorCode code) noexcept {
    switch(code){
    case AnalysisErrorCode::MissingResult:return "MissingResult";
    case AnalysisErrorCode::MissingVariable:return "MissingVariable";
    case AnalysisErrorCode::ShapeMismatch:return "ShapeMismatch";
    case AnalysisErrorCode::InvalidSegment:return "InvalidSegment";
    case AnalysisErrorCode::NonFiniteInput:return "NonFiniteInput";
    case AnalysisErrorCode::UndefinedStatistic:return "UndefinedStatistic";
    case AnalysisErrorCode::UnsupportedExperiment:return "UnsupportedExperiment";
    }
    return "Unknown";
}
struct AnalysisInput {
    std::filesystem::path path;
    std::string variable;
    std::size_t matlab_run_ordinal=1;
    std::string role;
    // No digest may be invented from the original .m blob or probe FNV64.
    std::optional<std::string> mat_sha256;
};
struct AnalysisSeries {
    std::string name;
    std::vector<double> x;
    std::vector<double> y;
    std::string x_unit="MATLAB 1-based frame";
    std::string y_unit;
    std::string x_scale="linear";
    std::string y_scale="linear";
    std::string panel;
    std::vector<AnalysisInput> inputs;
    std::size_t source_begin_matlab=0;
    std::size_t source_end_matlab=0;
    std::size_t smoothing_half_window=0;
    // For segment-local smoothing, retain ALL temporary values (zero edges)
    // in addition to the truncated actual plotted curve x/y.
    std::vector<double> smoothing_full;
};
struct AnalysisDataset {
    std::string source_analysis_blob; // exact original .m Git blob
    std::vector<AnalysisSeries> curves;
};

// Numeric-only, read-only Analyzer interface. No runtime plotting required.
class IAnalyzer {
public:
    virtual ~IAnalyzer() = default;
    virtual Result<AnalysisDataset,AnalysisErrorCode> analyze(
        AnalysisKind kind,const std::filesystem::path& input_root,
        IResultReader& reader) = 0;
};

}  // namespace egomotion
