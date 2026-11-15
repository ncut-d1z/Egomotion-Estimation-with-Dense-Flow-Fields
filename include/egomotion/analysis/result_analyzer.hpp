#pragma once

#include "egomotion/analysis/analyzer.hpp"

namespace egomotion::analysis {

class MatResultAnalyzer final : public IAnalyzer {
public:
    Result<AnalysisDataset,AnalysisErrorCode> analyze(
        AnalysisKind kind,const std::filesystem::path& input_root,
        IResultReader& reader) override;
};

}  // namespace egomotion::analysis
