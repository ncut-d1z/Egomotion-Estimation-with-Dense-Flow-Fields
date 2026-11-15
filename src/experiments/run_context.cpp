#include "egomotion/experiments/run_context.hpp"

#include <cstddef>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>

namespace egomotion {
namespace {
bool validRunId(const std::string& id) {
    if (id.empty() || id.size()>64)return false;
    for (unsigned char c:id)
        if (!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||
              (c>='0'&&c<='9')||c=='_'||c=='-')) return false;
    return true;
}
Status<ExperimentErrorCode> outputFailure(
    const Failure<MatErrorCode>& inner,const RunId& id,
    std::string field) {
    ErrorDetail context;
    context.stage=ErrorStage::Output;
    context.run_id=id;
    context.variable=std::move(field);
    context.message="Single MAT output failed; overall run remains non-complete";
    return Status<ExperimentErrorCode>::failure(
        wrapFailure(ExperimentErrorCode::OutputFailed,std::move(context),inner));
}
}  // namespace

RunContext::RunContext(
    RunId run_id,std::unique_ptr<IRandomSource> random,IResultWriter& writer,
    std::filesystem::path validated_output_root,SourceProvenance provenance,
    experiments::ExperimentKind kind,std::size_t expected_repetitions)
    : run_id_(std::move(run_id)),random_(std::move(random)),writer_(writer),
      output_root_(std::move(validated_output_root)),
      provenance_(std::move(provenance)),kind_(kind) {
    if (!validRunId(run_id_.text))
        throw std::invalid_argument("RunId must be 1-64 ASCII alphanumeric/_/-");
    if (!random_)
        throw std::invalid_argument("RunContext requires a dedicated RNG");
    if (output_root_.empty())
        throw std::invalid_argument("RunContext requires validated output root");
    if (expected_repetitions==0)
        throw std::invalid_argument("RunContext requires positive expected repetitions");
    draft_.expected_repetitions=expected_repetitions;
}

Status<ExperimentErrorCode> RunContext::writeCurrentResults(
    const std::map<std::string,DenseDoubleArray>& outputs) {
    auto validated=experiments::validateCurrentOutput(kind_,outputs);
    if (!validated.hasValue())
        return validated;

    // Plan all strings/vector capacity BEFORE the first Writer operation.
    // After a successful write, vector insertion moves a preallocated name,
    // and therefore cannot lose the audit identity due to allocation.
    std::vector<std::string> ordered_names;
    try {
        ordered_names=experiments::currentSaveFields(kind_);
        draft_.files_writer_confirmed.reserve(
            draft_.files_writer_confirmed.size()+ordered_names.size());
    } catch (const std::bad_alloc&) {
        ErrorDetail detail;
        detail.stage=ErrorStage::Output;
        detail.run_id=run_id_;
        detail.message="Cannot reserve output audit identities before writing";
        return Status<ExperimentErrorCode>::failure(
            {ExperimentErrorCode::OutputFailed,std::move(detail)});
    } catch (const std::length_error&) {
        ErrorDetail detail;
        detail.stage=ErrorStage::Output;
        detail.run_id=run_id_;
        detail.message="Output audit identities exceed container capacity";
        return Status<ExperimentErrorCode>::failure(
            {ExperimentErrorCode::OutputFailed,std::move(detail)});
    }
    // Safe ordering comes from the source save sequence, not map iteration.
    for (std::string& name:ordered_names) {
        const DenseDoubleArray& value=outputs.at(name);
        auto committed=writer_.saveNewVariable(run_id_,name,value);
        if (!committed.hasValue())
            return outputFailure(committed.error(),run_id_,name);
        draft_.files_writer_confirmed.push_back(std::move(name));
        draft_.status=RunStatus::Incomplete; // only an unpersisted draft
    }
    // Deliberately no RunOutcome{Complete} and no saved-Manifest claim.
    return Status<ExperimentErrorCode>::success();
}

void RunContext::recordFailure(Failure<ExperimentErrorCode> failure) {
    // ErrorDetail.causes is kept whole; do not collapse to root code.
    draft_.errors.push_back(std::move(failure));
}

}  // namespace egomotion
