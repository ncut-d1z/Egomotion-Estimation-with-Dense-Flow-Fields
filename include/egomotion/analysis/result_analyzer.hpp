#pragma once

#include "egomotion/analysis/analyzer.hpp"

namespace egomotion::analysis {

// 仅读取借用Reader，返回自拥有数值及来源；不创建Writer、Exporter或运行状态。
class BiasResultAnalyzer final {
public:
    Result<AnalysisDataset,AnalysisErrorCode> aggregate(
        const std::filesystem::path& input_root,IResultReader& reader) const;
private:
    Result<AnalysisDataset,AnalysisErrorCode> aggregateSource(
        const std::filesystem::path& input_root,IResultReader& reader) const;
};

class AdaptiveResultAnalyzer final {
public:
    Result<AnalysisDataset,AnalysisErrorCode> analyze(
        AnalysisKind kind,const std::filesystem::path& input_root,IResultReader& reader) const;
private:
    Result<AnalysisDataset,AnalysisErrorCode> analyzeSource(
        AnalysisKind kind,const std::filesystem::path& input_root,IResultReader& reader) const;
};

// 既有IAnalyzer入口仅分派，不重复两类源分析逻辑。
class MatResultAnalyzer final : public IAnalyzer {
public:
    Result<AnalysisDataset,AnalysisErrorCode> analyze(
        AnalysisKind kind,const std::filesystem::path& input_root,
        IResultReader& reader) override;
private:
    BiasResultAnalyzer bias_;
    AdaptiveResultAnalyzer adaptive_;
};

}  // namespace egomotion::analysis
