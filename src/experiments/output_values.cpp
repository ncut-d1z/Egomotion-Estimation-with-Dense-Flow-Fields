#include "egomotion/experiments/output_values.hpp"

#include <cstddef>
#include <cmath>
#include <limits>
#include <new>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

namespace egomotion::experiments {
namespace {
using Output=Result<CurrentOutputBatch,ExperimentErrorCode>;

Output fail(ExperimentErrorCode code,std::string message) {
    ErrorDetail detail;
    detail.stage=ErrorStage::Output;
    detail.message=std::move(message);
    return Output::failure({code,std::move(detail)});
}
DenseDoubleArray convert(std::string name,std::size_t rows,std::size_t cols,
                         const std::vector<double>& values) {
    if (rows==0||cols==0||
        cols>std::numeric_limits<std::size_t>::max()/rows||
        rows*cols>std::numeric_limits<std::size_t>::max()/sizeof(double)||
        rows*cols!=values.size())
        throw std::invalid_argument("Output conversion dimensions are inconsistent");
    DenseDoubleArray out;
    out.info.name=std::move(name);
    out.info.dimensions={rows,cols};
    out.info.original_class_type=6;
    out.info.original_data_type=9;
    out.info.class_name="double";
    out.info.data_type_name="double";
    out.info.data_bytes=rows*cols*sizeof(double);
    out.values_column_major=values;
    return out;
}

using MetricKey=std::tuple<std::size_t,std::string,std::size_t,std::size_t>;
using Masks=std::map<std::string,std::vector<unsigned char>>;
Masks masksFor(const CurrentOutputBatch& fields) {
    Masks masks;
    for(const auto& field:fields) {
        auto& mask=masks[field.first];
        mask.reserve(field.second.values_column_major.size());
        for(double value:field.second.values_column_major)
            mask.push_back(std::isfinite(value)?1:0);
    }
    return masks;
}

// 缺测声明、每轮原值、最终次数及固定分母在写首个文件前相互核对。
// 这不是由有效次数重新估计均值，缺任一轮的均值必须保持NaN。
Status<ExperimentErrorCode> validateAdaptiveEvidence(
    ExperimentKind kind,const AdaptiveRunData& source) {
    const auto reject=[](const char* message) {
        return Status<ExperimentErrorCode>::failure(
            fail(ExperimentErrorCode::DataMismatch,message).error());
    };
    const auto n=kind==ExperimentKind::AdaptiveInitialization?35U:600U;
    if(source.frame_count!=n||source.valid_counts.size()!=source.fields.size()||
       source.repetition_fields.size()!=source.planned_runs)
        return reject("Adaptive frame count, validity fields or repetition evidence is incomplete");
    auto valid=validateMaskedAdaptiveOutput(kind,source.fields,masksFor(source.fields));
    if(!valid.hasValue())return valid;
    for(const auto& repetition:source.repetition_fields) {
        const auto repetition_valid=validateMaskedAdaptiveOutput(kind,repetition,masksFor(repetition));
        if(!repetition_valid.hasValue())return repetition_valid;
    }
    std::set<MetricKey> declared;
    for(const auto& item:source.undefined_metrics) {
        const auto found=source.fields.find(item.variable);
        if(found==source.fields.end()||item.matlab_run_index==0||
           item.matlab_run_index>source.planned_runs||item.matlab_frame_index<2||
           item.matlab_frame_index>=n||item.matlab_column_index==0||
           item.matlab_column_index>found->second.info.dimensions[1]||
           item.reason.detail.message.empty()||
           (item.reason.code!=NumericalErrorCode::UndefinedAngle&&
            item.reason.code!=NumericalErrorCode::ZeroNorm&&
            item.reason.code!=NumericalErrorCode::NonFiniteResult)||
           !declared.emplace(item.matlab_run_index,item.variable,
                             item.matlab_frame_index,item.matlab_column_index).second)
            return reject("Adaptive missing metric has invalid or duplicate coordinates/reason");
    }
    std::vector<unsigned char> missing_frames(n*source.planned_runs,0);
    std::size_t missing_cells=0;
    for(const auto& item:source.fields) {
        const auto found=source.valid_counts.find(item.first);
        if(found==source.valid_counts.end()||
           found->second.size()!=item.second.values_column_major.size())
            return reject("Adaptive valid repetition counts have wrong shape");
        for(std::size_t offset=0;offset<found->second.size();++offset) {
            const auto row=offset%n,column=offset/n;
            const bool endpoint=row==0||row+1==n;
            double sum=0.0,last=0.0;
            std::size_t count=0;
            for(std::size_t run=0;run<source.planned_runs;++run) {
                const double value=source.repetition_fields[run].at(item.first).values_column_major[offset];
                last=value;
                const bool missing=std::isnan(value);
                const bool has_reason=declared.count(MetricKey{run+1,item.first,row+1,column+1})!=0;
                if(endpoint) {
                    if(value!=0.0)return reject("Adaptive structural endpoints must remain source zeros");
                } else if(missing) {
                    if(!has_reason)return reject("Adaptive missing value has no recorded numerical reason");
                    missing_frames[run*n+row]=1;++missing_cells;
                } else {
                    if(has_reason)return reject("Adaptive finite value has a contradictory missing reason");
                    ++count;sum+=value;
                }
            }
            const bool final_only=item.first=="rottransratio";
            const std::size_t expected_count=endpoint?0:
                (final_only?(std::isfinite(last)?1:0):count);
            const bool complete=endpoint||(final_only?std::isfinite(last):count==source.planned_runs);
            const double expected=endpoint?0.0:(final_only?last:sum/static_cast<double>(source.planned_runs));
            const double actual=item.second.values_column_major[offset];
            if(found->second[offset]!=expected_count||
               (complete?(!std::isfinite(expected)||actual!=expected):!std::isnan(actual)))
                return reject("Adaptive aggregate/count does not match its complete repetition evidence");
        }
    }
    std::size_t missing_frame_count=0;
    for(auto flag:missing_frames)missing_frame_count+=flag;
    const auto total=(n-2)*source.planned_runs;
    if(missing_cells!=source.undefined_metrics.size()||
       source.undefined_frames!=missing_frame_count||
       source.valid_frames!=total-missing_frame_count)
        return reject("Adaptive valid/undefined frame counts contradict the recorded metrics");
    return Status<ExperimentErrorCode>::success();
}
}  // namespace

Result<CurrentOutputBatch,ExperimentErrorCode> makeBiasOutput(
    ExperimentKind kind,const BiasRunData& source) {
    if (kind!=ExperimentKind::BiasOccluded &&
        kind!=ExperimentKind::BiasProportional)
        return fail(ExperimentErrorCode::InvalidParameters,
                    "Cannot convert Bias arrays for a non-Bias experiment");
    if (source.level_count==0)
        return fail(ExperimentErrorCode::DataMismatch,
                    "Bias memory output has no subdivision levels");
    try {
        CurrentOutputBatch fields;
        fields.emplace("xwerte",
            convert("xwerte",source.level_count,1,source.xwerte));
        for (std::size_t i=0;i<source.ywerte.size();++i) {
            const std::string name="ywerte"+std::to_string(i+1);
            fields.emplace(name,convert(name,source.level_count,3,
                                        source.ywerte[i]));
        }
        auto valid=validateCurrentOutput(kind,fields);
        if (!valid.hasValue()) return Output::failure(valid.error());
        return Output::success(std::move(fields));
    } catch (const std::invalid_argument& ex) {
        return fail(ExperimentErrorCode::DataMismatch,ex.what());
    } catch (const std::bad_alloc&) {
        return fail(ExperimentErrorCode::SolverFailed,
                    "Cannot allocate owning Bias MAT result values");
    } catch (const std::length_error&) {
        return fail(ExperimentErrorCode::SolverFailed,
                    "Bias result values exceed container capacity");
    }
}
Result<CurrentOutputBatch,ExperimentErrorCode> makeAdaptiveOutput(
    ExperimentKind kind,const AdaptiveRunData& source) {
    if (kind==ExperimentKind::BiasOccluded ||
        kind==ExperimentKind::BiasProportional)
        return fail(ExperimentErrorCode::InvalidParameters,
                    "Cannot convert Adaptive output for a Bias experiment");
    if (source.planned_runs==0||
        source.completed_runs!=source.planned_runs||
        (kind==ExperimentKind::AdaptiveComparison && source.planned_runs!=8)||
        (kind!=ExperimentKind::AdaptiveComparison && source.planned_runs!=1))
        return fail(ExperimentErrorCode::IncompleteRun,
                    "Adaptive result lacks the complete original repetition count");
    try {
        auto validated=validateAdaptiveEvidence(kind,source);
        if (!validated.hasValue())return Output::failure(validated.error());
        return Output::success(source.fields);
    } catch (const std::bad_alloc&) {
        return fail(ExperimentErrorCode::SolverFailed,
                    "Cannot copy owning Adaptive MAT result values");
    } catch (const std::length_error&) {
        return fail(ExperimentErrorCode::SolverFailed,
                    "Adaptive result values exceed container capacity");
    }
}
}  // namespace egomotion::experiments
