#pragma once

#include "egomotion/contracts/interfaces.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace egomotion {

enum class AnalysisKind {
    Bias, AdaptiveInitialization, AdaptiveComparison, AdaptiveUpdateRates
};
enum class AnalysisErrorCode {
    MissingResult, MissingVariable, ShapeMismatch, InvalidSegment,
    NonFiniteInput, UndefinedStatistic, UnsupportedExperiment
};

struct AnalysisSeries {
    std::string name;
    std::vector<double> x;
    std::vector<double> y;
};
struct AnalysisDataset { std::vector<AnalysisSeries> curves; };

// R1: the Analyzer reads domain values via IResultReader and owns its output;
// the optional plot backend must not become a dependency of analysis.
class IAnalyzer {
public:
    virtual ~IAnalyzer() = default;
    virtual Result<AnalysisDataset, AnalysisErrorCode> analyze(
        AnalysisKind kind, const std::filesystem::path& input_root,
        IResultReader& reader) = 0;
};

}  // namespace egomotion
