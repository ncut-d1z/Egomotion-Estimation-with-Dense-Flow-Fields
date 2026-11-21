#pragma once

#include "egomotion/analysis/analyzer.hpp"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace egomotion::analysis::detail {

using DatasetResult=Result<AnalysisDataset,AnalysisErrorCode>;
using ArrayResult=Result<DenseDoubleArray,AnalysisErrorCode>;
struct MetricArray {
    DenseDoubleArray array;
    std::optional<DenseDoubleArray> valid_counts;
    std::optional<DenseDoubleArray> policy_marker;
    std::vector<unsigned char> defined;
};
using MetricArrayResult=Result<MetricArray,AnalysisErrorCode>;

DatasetResult error(AnalysisErrorCode code,std::string message,
                    std::filesystem::path path={},
                    std::string variable={});
ArrayResult load(IResultReader& reader,const std::filesystem::path& root,
                 const std::string& name,std::size_t expected_rows=0,
                 std::size_t expected_cols=0);
// 仅在调用方已验证egomotion_metric_policy后使用。计数必须是同shape的
// 精确整数；完整固定分母才有有限结果，缺少任一重复则只能是NaN。
MetricArrayResult loadDeclaredMetric(
    IResultReader& reader,const std::filesystem::path& root,
    const std::string& name,std::size_t expected_rows,
    std::size_t expected_cols,std::size_t expected_repetitions);
std::vector<double> column(const DenseDoubleArray& array,std::size_t col);
AnalysisInput provenance(const std::filesystem::path& root,
                         const DenseDoubleArray& source,std::size_t matlab_run,
                         std::string role);
AnalysisSeries series(std::string name,std::string panel,
                      std::vector<double> x,std::vector<double> y,
                      std::vector<AnalysisInput> sources,
                      std::size_t begin,std::size_t end);
}  // namespace egomotion::analysis::detail
