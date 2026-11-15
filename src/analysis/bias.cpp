#include "reader_helpers.hpp"

#include <array>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace egomotion::analysis::detail {
namespace {
namespace fs=std::filesystem;

Result<std::vector<fs::path>,AnalysisErrorCode> runDirectories(
    const fs::path& root) {
    // Source calls num2str(k) for k=1..number_of_cores.
    // The CLI has no number_of_cores switch; infer a contiguous 1..K group.
    std::map<std::size_t,fs::path> numbered;
    for(const auto& entry:fs::directory_iterator(root)) {
        if(!entry.is_directory())continue;
        const std::string text=entry.path().filename().string();
        if(text.empty())continue;
        std::size_t value=0;
        const auto* first=text.data();
        const auto parsed=std::from_chars(first,first+text.size(),value);
        if(parsed.ec!=std::errc{}||parsed.ptr!=first+text.size()||value==0||
           std::to_string(value)!=text)continue;
        numbered.emplace(value,entry.path());
    }
    if(numbered.empty()){
        // Also support a single self-contained C++ RunId directory.
        if(fs::exists(root/"xwerte.mat"))
            return Result<std::vector<fs::path>,AnalysisErrorCode>::success({root});
        ErrorDetail d;d.stage=ErrorStage::Read;d.source_path=root.string();
        d.message="Bias expects numbered 1..K run directories or single-run MAT files";
        return Result<std::vector<fs::path>,AnalysisErrorCode>::failure({
            AnalysisErrorCode::MissingResult,std::move(d)});
    }
    std::vector<fs::path> out;
    out.reserve(numbered.size());
    std::size_t expected=1;
    for(const auto& pair:numbered){
        if(pair.first!=expected){
            ErrorDetail d;d.stage=ErrorStage::Analyze;d.source_path=root.string();
            d.message="Bias source run directory sequence has a gap";
            return Result<std::vector<fs::path>,AnalysisErrorCode>::failure({
                AnalysisErrorCode::MissingResult,std::move(d)});
        }
        out.push_back(pair.second);
        ++expected;
    }
    return Result<std::vector<fs::path>,AnalysisErrorCode>::success(std::move(out));
}
} // namespace

DatasetResult bias(const std::filesystem::path& root,IResultReader& reader) {
    auto runs=runDirectories(root);
    if(!runs.hasValue())
        return DatasetResult::failure(std::move(runs).error());
    const auto& dirs=runs.value();
    constexpr std::array<const char*,4> ys{{"ywerte3","ywerte4","ywerte5","ywerte6"}};
    std::size_t rows=0;
    std::vector<double> last_x;
    std::array<std::array<std::vector<double>,3>,4> totals{};
    std::array<std::vector<AnalysisInput>,4> provenance_y;
    AnalysisInput provenance_x;
    for(std::size_t k=0;k<dirs.size();++k){
        auto x=load(reader,dirs[k],"xwerte",rows,1);
        if(!x.hasValue())return DatasetResult::failure(std::move(x).error());
        if(!rows){
            rows=x.value().info.dimensions[0];
            if(rows<3)return error(AnalysisErrorCode::ShapeMismatch,
                                   "Bias analysis requires >=3 subdivision rows");
            for(auto& group:totals)for(auto& values:group)values.assign(rows,0.0);
        }
        last_x=column(x.value(),0); // source overwrites x every k
        if(k+1==dirs.size())provenance_x=provenance(dirs[k],"xwerte",k+1,"x_last");
        for(std::size_t kind=0;kind<ys.size();++kind){
            const std::string name=ys[kind];
            auto y=load(reader,dirs[k],name,rows,3);
            if(!y.hasValue())return DatasetResult::failure(std::move(y).error());
            provenance_y[kind].push_back(provenance(dirs[k],name,k+1,"y_member"));
            for(std::size_t c=0;c<3;++c)
                for(std::size_t row=0;row<rows;++row)
                    totals[kind][c][row]+=y.value().values_column_major[c*rows+row];
        }
    }
    AnalysisDataset out;
    // Original MATLAB source for graphs_bias_test.m at the audited Git blob.
    out.source_analysis_blob="112aa9eca1fa9728f493d31fc8e59620a8445230";
    std::vector<double> axis,reference;
    axis.reserve(rows-2);reference.reserve(rows-2);
    for(std::size_t i=2;i<rows;++i){
        const double original_x=last_x[i];
        if(!(original_x>0.0)||!std::isfinite(original_x))
            return error(AnalysisErrorCode::UndefinedStatistic,
                         "Bias xwerte must be positive to compute 100/sqrt(x_last)");
        axis.push_back(original_x);
        reference.push_back(100.0/std::sqrt(original_x));
    }
    auto theoretical=series("100/sqrt(xwerte_last)","bias-loglog",
                            axis,reference,{provenance_x},3,rows);
    theoretical.x_unit="original MATLAB xwerte";
    theoretical.y_unit="reference scaled error";
    theoretical.x_scale=theoretical.y_scale="log";
    out.curves.push_back(std::move(theoretical));
    for(std::size_t kind=0;kind<ys.size();++kind){
        for(std::size_t c=0;c<3;++c){
            std::vector<double> actual;
            actual.reserve(rows-2);
            for(std::size_t i=2;i<rows;++i){
                const double mean=totals[kind][c][i]/
                    static_cast<double>(dirs.size());
                if(!std::isfinite(mean))
                    return error(AnalysisErrorCode::UndefinedStatistic,
                                 "Bias cross-run mean overflow");
                actual.push_back(mean);
            }
            auto inputs=provenance_y[kind];
            inputs.push_back(provenance_x);
            auto curve=series(std::string(ys[kind])+".column"+std::to_string(c+1),
                              "bias-loglog",axis,std::move(actual),
                              std::move(inputs),3,rows);
            curve.x_unit="original MATLAB xwerte";
            curve.y_unit="scaled translation/rotation vector error";
            curve.x_scale=curve.y_scale="log";
            out.curves.push_back(std::move(curve));
        }
    }
    // One original loglog axis: reference + 4 y variables * 3 coefficients.
    return DatasetResult::success(std::move(out));
}

}  // namespace egomotion::analysis::detail
