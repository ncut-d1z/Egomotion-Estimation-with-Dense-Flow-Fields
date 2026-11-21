#pragma once

#include "reader_helpers.hpp"

#include <new>
#include <stdexcept>

namespace egomotion::analysis::detail {

// 保留既有只读分析入口的空路径和异常分类，供两个具体组件共用。
template<class Operation>
DatasetResult guardAnalysis(const std::filesystem::path& root,Operation&& operation) {
    if(root.empty())
        return error(AnalysisErrorCode::MissingResult,"Empty analysis input root");
    try {
        return operation();
    }catch(const std::bad_alloc&) {
        return error(AnalysisErrorCode::UndefinedStatistic,
                             "Insufficient memory for analysis data");
    }catch(const std::length_error&) {
        return error(AnalysisErrorCode::ShapeMismatch,
                             "Analysis input array exceeds container capacity");
    }catch(const std::filesystem::filesystem_error& e) {
        return error(AnalysisErrorCode::MissingResult,
                             std::string("Input directory access failed: ")+e.what());
    }catch(const std::out_of_range& e){
        return error(AnalysisErrorCode::ShapeMismatch,
                             std::string("Source MATLAB index exceeded an array: ")+e.what());
    }
}

} // namespace egomotion::analysis::detail
