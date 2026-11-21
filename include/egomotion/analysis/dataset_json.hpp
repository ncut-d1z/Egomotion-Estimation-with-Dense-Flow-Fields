#pragma once

#include "egomotion/analysis/analyzer.hpp"

#include <string>

namespace egomotion::analysis {

// JSONL value only. File publication, if any, is a separate composition-root
// operation and cannot affect read-only numeric analysis.
Result<std::string,AnalysisErrorCode> datasetJson(const AnalysisDataset& dataset);

}  // namespace egomotion::analysis
