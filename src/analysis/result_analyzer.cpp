#include "egomotion/analysis/result_analyzer.hpp"

namespace egomotion::analysis {

Result<AnalysisDataset,AnalysisErrorCode> MatResultAnalyzer::analyze(
    AnalysisKind kind,const std::filesystem::path& root,IResultReader& reader) {
    if(kind==AnalysisKind::Bias)return bias_.aggregate(root,reader);
    return adaptive_.analyze(kind,root,reader);
}

} // namespace egomotion::analysis
