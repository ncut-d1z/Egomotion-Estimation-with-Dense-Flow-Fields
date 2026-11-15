#include "egomotion/experiments/output_values.hpp"

#include <cstddef>
#include <limits>
#include <new>
#include <stdexcept>
#include <string>
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
        CurrentOutputBatch fields=source.fields;
        auto validated=validateCurrentOutput(kind,fields);
        if (!validated.hasValue())return Output::failure(validated.error());
        return Output::success(std::move(fields));
    } catch (const std::bad_alloc&) {
        return fail(ExperimentErrorCode::SolverFailed,
                    "Cannot copy owning Adaptive MAT result values");
    } catch (const std::length_error&) {
        return fail(ExperimentErrorCode::SolverFailed,
                    "Adaptive result values exceed container capacity");
    }
}
}  // namespace egomotion::experiments
