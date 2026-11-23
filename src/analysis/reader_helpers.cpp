#include "reader_helpers.hpp"

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <new>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace egomotion::analysis::detail {

DatasetResult error(AnalysisErrorCode code,std::string message,
                    std::filesystem::path path,std::string variable) {
    ErrorDetail d;d.stage=ErrorStage::Analyze;
    d.message=std::move(message);d.source_path=path.string();
    d.variable=std::move(variable);
    return DatasetResult::failure({code,std::move(d)});
}
namespace {
ArrayResult loadArray(IResultReader& reader,const std::filesystem::path& root,
                      const std::string& name,std::size_t rows,std::size_t cols) {
    const auto path=root/(name+".mat");
    auto read=reader.readNamed(path,name);
    if(!read.hasValue()){
        const auto& original=read.error();
        const auto code=original.code==MatErrorCode::MissingFile?
                        AnalysisErrorCode::MissingResult:
                        (original.code==MatErrorCode::MissingVariable?
                         AnalysisErrorCode::MissingVariable:AnalysisErrorCode::ShapeMismatch);
        ErrorDetail outer;outer.stage=ErrorStage::Analyze;
        outer.source_path=path.string();outer.variable=name;
        outer.message="Read-only MAT analysis input is missing or incompatible";
        return ArrayResult::failure(wrapFailure(code,std::move(outer),original));
    }
    auto value=std::move(read).value();
    const auto& info=value.info;
    if(info.name!=name||info.dimensions.size()!=2||
       info.original_class_type!=6||info.original_data_type!=9||
       info.is_complex||info.is_logical||
       (rows&&info.dimensions[0]!=rows)||
       (cols&&info.dimensions[1]!=cols))
        return ArrayResult::failure({
            AnalysisErrorCode::ShapeMismatch,
            ErrorDetail{std::nullopt,ErrorStage::Decode,
                        "MAT variable name/type/shape mismatch",path.string(),name,
                        std::nullopt,{}}});
    const auto n=info.dimensions[0];
    const auto c=info.dimensions[1];
    if(!n||!c||c>std::numeric_limits<std::size_t>::max()/n||
       n*c>std::numeric_limits<std::size_t>::max()/sizeof(double)||
       value.values_column_major.size()!=n*c||info.data_bytes!=n*c*sizeof(double))
        return ArrayResult::failure({
            AnalysisErrorCode::ShapeMismatch,
            ErrorDetail{std::nullopt,ErrorStage::Decode,
                        "MAT column-major array length mismatch",path.string(),name,
                        std::nullopt,{}}});
    return ArrayResult::success(std::move(value));
}
} // namespace

ArrayResult load(IResultReader& reader,const std::filesystem::path& root,
                 const std::string& name,std::size_t rows,std::size_t cols) {
    auto input=loadArray(reader,root,name,rows,cols);
    if(!input.hasValue())return input;
    for(double x:input.value().values_column_major)
        if(!std::isfinite(x)){
            ErrorDetail d;d.stage=ErrorStage::Analyze;
            d.source_path=(root/(name+".mat")).string();
            d.variable=name;d.message="MAT analysis input contains NaN or infinity";
            return ArrayResult::failure({AnalysisErrorCode::NonFiniteInput,std::move(d)});
        }
    return input;
}

MetricArrayResult loadDeclaredMetric(
    IResultReader& reader,const std::filesystem::path& root,
    const std::string& name,std::size_t rows,std::size_t cols,
    std::size_t expected_repetitions) {
    auto source=loadArray(reader,root,name,rows,cols);
    if(!source.hasValue())return MetricArrayResult::failure(std::move(source).error());
    auto counts=load(reader,root,"valid_"+name,rows,cols);
    if(!counts.hasValue())return MetricArrayResult::failure(std::move(counts).error());
    const auto& values=source.value().values_column_major;
    const auto& repetitions=counts.value().values_column_major;
    const auto source_rows=source.value().info.dimensions[0];
    std::vector<unsigned char> defined(values.size(),1);
    for(std::size_t i=0;i<values.size();++i) {
        const auto count=repetitions[i];
        const auto value=values[i];
        const bool endpoint=i%source_rows==0||i%source_rows+1==source_rows;
        const bool valid_count=std::isfinite(count)&&count>=0.0&&
            count<=static_cast<double>(expected_repetitions)&&std::floor(count)==count;
        const bool complete=count==static_cast<double>(expected_repetitions);
        const bool consistent=endpoint?(value==0.0&&count==0.0):
            (complete?std::isfinite(value):std::isnan(value));
        if(expected_repetitions==0||!valid_count||!consistent) {
            ErrorDetail d;d.stage=ErrorStage::Analyze;
            d.source_path=(root/(name+".mat")).string();d.variable=name;
            d.observation_index=i%source_rows+1;
            d.message="Declared metric/count mismatch at column "+std::to_string(i/source_rows+1)+
                "; endpoints require zero/count zero, complete counts require finite values, and missing counts require NaN";
            return MetricArrayResult::failure({
                AnalysisErrorCode::UndefinedStatistic,std::move(d)});
        }
        defined[i]=static_cast<unsigned char>(endpoint||complete);
    }
    MetricArray result;
    result.array=std::move(source).value();
    result.valid_counts=std::move(counts).value();
    result.defined=std::move(defined);
    return MetricArrayResult::success(std::move(result));
}
std::vector<double> column(const DenseDoubleArray& array,std::size_t col) {
    const auto rows=array.info.dimensions.at(0);
    const auto cols=array.info.dimensions.at(1);
    if(col>=cols)throw std::out_of_range("MAT analysis column out of bounds");
    const auto first=array.values_column_major.begin()+
        static_cast<std::ptrdiff_t>(col*rows);
    return std::vector<double>(first,first+static_cast<std::ptrdiff_t>(rows));
}
AnalysisInput provenance(const std::filesystem::path& root,
                         const DenseDoubleArray& source,std::size_t matlab_run,
                         std::string role) {
    AnalysisInput p;
    p.path=root/(source.info.name+".mat");
    p.variable=source.info.name;
    p.mat_sha256=source.source_mat_sha256;
    p.matlab_run_ordinal=matlab_run;
    p.role=std::move(role);
    return p;
}
AnalysisSeries series(std::string name,std::string panel,
                      std::vector<double> x,std::vector<double> y,
                      std::vector<AnalysisInput> sources,
                      std::size_t begin,std::size_t end) {
    AnalysisSeries result;
    result.name=std::move(name);
    result.panel=std::move(panel);
    result.x=std::move(x);result.y=std::move(y);
    result.inputs=std::move(sources);
    result.source_begin_matlab=begin;result.source_end_matlab=end;
    return result;
}
} // namespace egomotion::analysis::detail
