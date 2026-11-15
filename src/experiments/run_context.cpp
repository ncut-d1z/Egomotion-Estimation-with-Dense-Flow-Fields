#include "egomotion/experiments/run_context.hpp"
#include "egomotion/experiments/manifest_json.hpp"

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
    ITextPublisher& manifest_publisher,
    std::filesystem::path validated_output_root,SourceProvenance provenance,
    experiments::ExperimentKind kind,std::size_t expected_repetitions)
    : run_id_(std::move(run_id)),random_(std::move(random)),writer_(writer),
      manifest_publisher_(manifest_publisher),
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
    if(write_started_) {
        ErrorDetail d;d.stage=ErrorStage::Output;d.run_id=run_id_;
        d.message="Duplicate writeCurrentResults is prohibited";
        return Status<ExperimentErrorCode>::failure({
            ExperimentErrorCode::OutputFailed,std::move(d)});
    }
    auto validated=experiments::validateCurrentOutput(kind_,outputs);
    if (!validated.hasValue())
        return validated;
    write_started_=true;

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


Status<ExperimentErrorCode> RunContext::noteCompletedMemory(
    std::size_t repetitions,std::size_t valid_samples) {
    if(repetitions!=draft_.expected_repetitions||!repetitions||!valid_samples) {
        ErrorDetail d;d.stage=ErrorStage::Simulation;d.run_id=run_id_;
        d.message="Completed source repetitions or valid observations are missing";
        return Status<ExperimentErrorCode>::failure({
            ExperimentErrorCode::IncompleteRun,std::move(d)});
    }
    draft_.attempted=repetitions;
    draft_.completed=repetitions;
    draft_.valid=valid_samples;
    return Status<ExperimentErrorCode>::success();
}

Result<RunOutcome,ExperimentErrorCode> RunContext::publishCompleteOutcome() {
    using Output=Result<RunOutcome,ExperimentErrorCode>;
    if(manifest_published_ || !write_started_) {
        ErrorDetail d;d.stage=ErrorStage::Output;d.run_id=run_id_;
        d.message="Manifest cannot be republished or published before outputs";
        return Output::failure({ExperimentErrorCode::OutputFailed,std::move(d)});
    }
    try {
        auto json=experiments::makeCompleteManifestJson(*this,writer_.fileEvidence());
        if(!json.hasValue())
            return Output::failure(std::move(json).error());
        // Prepare the entire success alternative BEFORE publishing. If
        // copying RunId or allocating the Result fails, no Complete Manifest
        // has been published yet; after publication only noexcept moves
        // remain. Disk and in-process success cannot disagree on allocation.
        RunOutcome outcome;
        outcome.run_id=run_id_;
        outcome.status=RunStatus::Complete;
        outcome.attempted=draft_.attempted;
        outcome.completed=draft_.completed;
        outcome.valid=draft_.valid;
        outcome.failed=draft_.failed;
        outcome.undefined=draft_.undefined;
        auto completed=Output::success(std::move(outcome));
        auto persisted=manifest_publisher_.publishNew("manifest.json",json.value());
        if(!persisted.hasValue()) {
            ErrorDetail d;d.stage=ErrorStage::Output;d.run_id=run_id_;
            d.message="Complete Manifest publication failed or durability unconfirmed";
            return Output::failure(wrapFailure(
                ExperimentErrorCode::OutputFailed,std::move(d),persisted.error()));
        }
        manifest_published_=true;
        draft_.status=RunStatus::Complete;
        return completed;
    } catch(const std::bad_alloc&) {
        ErrorDetail d;d.stage=ErrorStage::Output;d.run_id=run_id_;
        d.message="Manifest evidence allocation failed before confirmed outcome";
        return Output::failure({ExperimentErrorCode::OutputFailed,std::move(d)});
    } catch(const std::length_error&) {
        ErrorDetail d;d.stage=ErrorStage::Output;d.run_id=run_id_;
        d.message="Manifest evidence exceeded container capacity";
        return Output::failure({ExperimentErrorCode::OutputFailed,std::move(d)});
    }
}


Status<ExperimentErrorCode> RunContext::publishFailureRecord(
    Failure<ExperimentErrorCode> original) {
    if(manifest_published_) {
        ErrorDetail d;d.stage=ErrorStage::Output;d.run_id=run_id_;
        d.message="A RunId Manifest has already been published";
        return Status<ExperimentErrorCode>::failure(
            {ExperimentErrorCode::OutputFailed,std::move(d)});
    }
    try{
        recordFailure(std::move(original));
        ++draft_.failed;
        // Partial in-memory or verified file results remain an audit record,
        // not a substitute for a successful Complete run.
        draft_.status=(draft_.completed>0||draft_.valid>0||
                       !draft_.files_writer_confirmed.empty())
                      ?RunStatus::Incomplete:RunStatus::Failed;
        auto json=experiments::makeFailureManifestJson(
            *this,writer_.fileEvidence());
        if(!json.hasValue())
            return Status<ExperimentErrorCode>::failure(std::move(json).error());
        auto persisted=manifest_publisher_.publishNew("manifest.json",json.value());
        if(!persisted.hasValue()){
            ErrorDetail d;d.stage=ErrorStage::Output;d.run_id=run_id_;
            d.message="Failure Manifest itself could not be durably published";
            return Status<ExperimentErrorCode>::failure(wrapFailure(
                ExperimentErrorCode::OutputFailed,std::move(d),persisted.error()));
        }
        manifest_published_=true;
        return Status<ExperimentErrorCode>::success();
    }catch(const std::bad_alloc&){
        ErrorDetail d;d.stage=ErrorStage::Output;d.run_id=run_id_;
        d.message="Cannot allocate the failure Manifest audit record";
        return Status<ExperimentErrorCode>::failure(
            {ExperimentErrorCode::OutputFailed,std::move(d)});
    }catch(const std::length_error&){
        ErrorDetail d;d.stage=ErrorStage::Output;d.run_id=run_id_;
        d.message="Failure Manifest audit container exceeds capacity";
        return Status<ExperimentErrorCode>::failure(
            {ExperimentErrorCode::OutputFailed,std::move(d)});
    }
}

void RunContext::recordFailure(Failure<ExperimentErrorCode> failure) {
    // ErrorDetail.causes is kept whole; do not collapse to root code.
    draft_.errors.push_back(std::move(failure));
}

}  // namespace egomotion
