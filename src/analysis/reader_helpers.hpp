#pragma once

#include "egomotion/analysis/analyzer.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace egomotion::analysis::detail {

using DatasetResult=Result<AnalysisDataset,AnalysisErrorCode>;
using ArrayResult=Result<DenseDoubleArray,AnalysisErrorCode>;

DatasetResult error(AnalysisErrorCode code,std::string message,
                    std::filesystem::path path={},
                    std::string variable={});
ArrayResult load(IResultReader& reader,const std::filesystem::path& root,
                 const std::string& name,std::size_t expected_rows=0,
                 std::size_t expected_cols=0);
std::vector<double> column(const DenseDoubleArray& array,std::size_t col);
AnalysisInput provenance(const std::filesystem::path& root,
                         const DenseDoubleArray& source,std::size_t matlab_run,
                         std::string role);
AnalysisSeries series(std::string name,std::string panel,
                      std::vector<double> x,std::vector<double> y,
                      std::vector<AnalysisInput> sources,
                      std::size_t begin,std::size_t end);
DatasetResult bias(const std::filesystem::path& root,IResultReader& reader);
DatasetResult adaptive(AnalysisKind kind,const std::filesystem::path& root,
                       IResultReader& reader);
}  // namespace egomotion::analysis::detail
