#include "egomotion/analysis/result_analyzer.hpp"
#include "reader_helpers.hpp"

#include <new>
#include <stdexcept>
#include <string>

namespace egomotion::analysis {

Result<AnalysisDataset,AnalysisErrorCode> MatResultAnalyzer::analyze(
    AnalysisKind kind,const std::filesystem::path& root,IResultReader& reader) {
    if(root.empty())
        return detail::error(AnalysisErrorCode::MissingResult,"Empty analysis input root");
    try {
        if(kind==AnalysisKind::Bias)return detail::bias(root,reader);
        return detail::adaptive(kind,root,reader);
    }catch(const std::bad_alloc&) {
        return detail::error(AnalysisErrorCode::UndefinedStatistic,
                             "Insufficient memory for analysis data");
    }catch(const std::length_error&) {
        return detail::error(AnalysisErrorCode::ShapeMismatch,
                             "Analysis input array exceeds container capacity");
    }catch(const std::filesystem::filesystem_error& e) {
        return detail::error(AnalysisErrorCode::MissingResult,
                             std::string("Input directory access failed: ")+e.what());
    }catch(const std::out_of_range& e){
        return detail::error(AnalysisErrorCode::ShapeMismatch,
                             std::string("Source MATLAB index exceeded an array: ")+e.what());
    }
}

} // namespace egomotion::analysis
