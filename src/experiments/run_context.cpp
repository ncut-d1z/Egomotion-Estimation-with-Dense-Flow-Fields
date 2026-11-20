#include "egomotion/experiments/run_context.hpp"
#include "egomotion/experiments/manifest_json.hpp"
#include "egomotion/experiments/output_values.hpp"
#include "../run_diagnostics.hpp"

#include <cmath>
#include <cstddef>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>

namespace egomotion {
namespace {
bool adaptiveKind(experiments::ExperimentKind kind) {
    return kind==experiments::ExperimentKind::AdaptiveInitialization||
           kind==experiments::ExperimentKind::AdaptiveComparison||
           kind==experiments::ExperimentKind::AdaptiveUpdateRates;
}
Status<ExperimentErrorCode> contextFailure(
    const RunId& id,ExperimentErrorCode code,std::string message,
    ErrorStage stage=ErrorStage::Output) {
    ErrorDetail detail;detail.stage=stage;detail.run_id=id;
    detail.message=std::move(message);
    return Status<ExperimentErrorCode>::failure({code,std::move(detail)});
}
struct AdaptivePlannedFile {
    DenseDoubleArray value;
    std::vector<unsigned char> defined;
    bool masked=false;
};
DenseDoubleArray policyMarker(std::size_t repetitions) {
    DenseDoubleArray marker;
    marker.info.name="egomotion_metric_policy";
    marker.info.dimensions={1,4};
    marker.info.original_class_type=6;marker.info.original_data_type=9;
    marker.info.class_name="double";marker.info.data_type_name="double";
    marker.info.data_bytes=4*sizeof(double);
    marker.values_column_major={1.0,1.0,1.0,static_cast<double>(repetitions)};
    return marker;
}
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
    if(adaptiveKind(kind_))
        return contextFailure(run_id_,ExperimentErrorCode::InvalidParameters,
            "Adaptive results require writeAdaptiveResults and metric policy v2");
    if(write_started_) {
        ErrorDetail d;d.stage=ErrorStage::Output;d.run_id=run_id_;
        d.message="Duplicate writeCurrentResults is prohibited";
        return Status<ExperimentErrorCode>::failure({
            ExperimentErrorCode::OutputFailed,std::move(d)});
    }
    auto validated=experiments::validateCurrentOutput(kind_,outputs);
    if (!validated.hasValue())
        return Status<ExperimentErrorCode>::failure(
            detail::withRunIdIfMissing(std::move(validated).error(),run_id_));
    write_started_=true;

    // Plan all strings/vector capacity BEFORE the first Writer operation.
    // After a successful write, vector insertion moves a preallocated name,
    // and therefore cannot lose the audit identity due to allocation.
    std::vector<std::string> ordered_names;
    try {
        ordered_names=experiments::currentSaveFields(kind_);
        expected_output_names_=ordered_names;
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

Status<ExperimentErrorCode> RunContext::retainAdaptiveEvidence(
    const experiments::AdaptiveRunData& outputs,std::string serialized) {
    if(!adaptive_metric_validity_json_.empty()&&adaptive_metric_validity_json_!=serialized)
        return contextFailure(run_id_,ExperimentErrorCode::DataMismatch,
            "Adaptive memory evidence changed after being retained by this RunContext");
    // 调用者已经完成全部分配和源合同验证；交换字符串后只写入无异常标量。
    adaptive_metric_validity_json_.swap(serialized);
    adaptive_expected_repetitions_=outputs.planned_runs;
    adaptive_valid_frames_=outputs.valid_frames;
    adaptive_undefined_frames_=outputs.undefined_frames;
    adaptive_statistics_complete_=outputs.undefined_frames==0&&outputs.undefined_metrics.empty();
    return Status<ExperimentErrorCode>::success();
}

Status<ExperimentErrorCode> RunContext::writeAdaptiveResults(
    const experiments::AdaptiveRunData& outputs) {
    if(!adaptiveKind(kind_)||write_started_||manifest_published_||
       draft_.failed!=0||!draft_.errors.empty())
        return contextFailure(run_id_,ExperimentErrorCode::OutputFailed,
            "Adaptive outputs require an unused Adaptive RunContext without failures");
    if(outputs.planned_runs!=draft_.expected_repetitions)
        return contextFailure(run_id_,ExperimentErrorCode::DataMismatch,
            "Adaptive repetition count differs from the RunContext plan");
    try {
        auto validated=experiments::makeAdaptiveOutput(kind_,outputs);
        if(!validated.hasValue())
            return Status<ExperimentErrorCode>::failure(
                detail::withRunIdIfMissing(std::move(validated).error(),run_id_));
        auto serialized=experiments::makeAdaptiveMetricValidityJson(*this,outputs);
        if(!serialized.hasValue())
            return Status<ExperimentErrorCode>::failure(std::move(serialized).error());
        auto fields=std::move(validated).value();
        const auto source_names=experiments::currentSaveFields(kind_);
        std::vector<AdaptivePlannedFile> files;
        files.reserve(1+2*source_names.size());
        files.push_back({policyMarker(outputs.planned_runs),{},false});
        for(const auto& name:source_names) {
            auto& source=fields.at(name);
            const auto& counts=outputs.valid_counts.at(name);
            DenseDoubleArray validity;
            validity.info=source.info;
            validity.info.name="valid_"+name;
            validity.values_column_major.reserve(counts.size());
            for(std::size_t count:counts)
                validity.values_column_major.push_back(static_cast<double>(count));
            files.push_back({std::move(validity),{},false});
            std::vector<unsigned char> defined(source.values_column_major.size(),1);
            for(std::size_t i=0;i<defined.size();++i)
                if(std::isnan(source.values_column_major[i]))defined[i]=0;
            files.push_back({std::move(source),std::move(defined),true});
        }
        // marker、每项有效次数和原字段均先完成分配；成功写入后只移动预备身份。
        std::vector<std::string> expected,confirmed;
        expected.reserve(files.size());
        for(const auto& file:files)expected.push_back(file.value.info.name);
        confirmed=expected;
        draft_.files_writer_confirmed.reserve(draft_.files_writer_confirmed.size()+files.size());
        auto retained=retainAdaptiveEvidence(outputs,std::move(serialized).value());
        if(!retained.hasValue())return retained;
        expected_output_names_.swap(expected);
        write_started_=true;
        for(std::size_t i=0;i<files.size();++i) {
            const auto& file=files[i];
            auto committed=file.masked
                ? writer_.saveNewMaskedVariable(run_id_,file.value.info.name,file.value,file.defined)
                : writer_.saveNewVariable(run_id_,file.value.info.name,file.value);
            if(!committed.hasValue())
                return outputFailure(committed.error(),run_id_,file.value.info.name);
            draft_.files_writer_confirmed.push_back(std::move(confirmed[i]));
            draft_.status=RunStatus::Incomplete;
        }
        return Status<ExperimentErrorCode>::success();
    } catch(const std::bad_alloc&) {
        return contextFailure(run_id_,ExperimentErrorCode::OutputFailed,
            "Cannot allocate Adaptive output or retained metric evidence");
    } catch(const std::length_error&) {
        return contextFailure(run_id_,ExperimentErrorCode::OutputFailed,
            "Adaptive output or retained metric evidence exceeds container capacity");
    } catch(const std::out_of_range&) {
        return contextFailure(run_id_,ExperimentErrorCode::DataMismatch,
            "Validated Adaptive fields and validity counts disagree");
    }
}

Status<ExperimentErrorCode> RunContext::noteCompletedMemory(
    std::size_t repetitions,std::size_t valid_samples) {
    if(adaptiveKind(kind_))
        return contextFailure(run_id_,ExperimentErrorCode::InvalidParameters,
            "Adaptive counters require noteAdaptiveCompletedMemory with complete metric evidence",
            ErrorStage::Simulation);
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

Status<ExperimentErrorCode> RunContext::noteAdaptiveCompletedMemory(
    const experiments::AdaptiveRunData& outputs) {
    if(!adaptiveKind(kind_)||manifest_published_||draft_.failed!=0||!draft_.errors.empty())
        return contextFailure(run_id_,ExperimentErrorCode::IncompleteRun,
            "A failed or published run cannot be reclassified as Adaptive completion",
            ErrorStage::Simulation);
    if(outputs.planned_runs!=draft_.expected_repetitions)
        return contextFailure(run_id_,ExperimentErrorCode::DataMismatch,
            "Adaptive memory repetition count differs from the RunContext plan",
            ErrorStage::Simulation);
    try {
        // serializer 内部再次调用完整 makeAdaptiveOutput 门禁，不能只相信外部计数。
        auto serialized=experiments::makeAdaptiveMetricValidityJson(*this,outputs);
        if(!serialized.hasValue())
            return Status<ExperimentErrorCode>::failure(std::move(serialized).error());
        auto retained=retainAdaptiveEvidence(outputs,std::move(serialized).value());
        if(!retained.hasValue())return retained;
        draft_.attempted=outputs.planned_runs;
        draft_.completed=outputs.completed_runs;
        draft_.valid=outputs.valid_frames;
        draft_.undefined=outputs.undefined_frames;
        adaptive_memory_noted_=true;
        return Status<ExperimentErrorCode>::success();
    } catch(const std::bad_alloc&) {
        return contextFailure(run_id_,ExperimentErrorCode::OutputFailed,
            "Cannot retain completed Adaptive metric evidence");
    } catch(const std::length_error&) {
        return contextFailure(run_id_,ExperimentErrorCode::OutputFailed,
            "Completed Adaptive metric evidence exceeds container capacity");
    }
}

bool RunContext::adaptiveComputationComplete() const noexcept {
    return adaptive_memory_noted_&&!adaptive_metric_validity_json_.empty()&&
        adaptive_expected_repetitions_==draft_.expected_repetitions&&
        draft_.attempted==adaptive_expected_repetitions_&&
        draft_.completed==adaptive_expected_repetitions_&&
        draft_.valid==adaptive_valid_frames_&&draft_.undefined==adaptive_undefined_frames_;
}

Result<RunOutcome,ExperimentErrorCode> RunContext::publishCompleteOutcome() {
    using Output=Result<RunOutcome,ExperimentErrorCode>;
    if(adaptiveKind(kind_)) {
        auto rejected=contextFailure(run_id_,ExperimentErrorCode::InvalidParameters,
            "Adaptive outcomes require publishAdaptiveOutcome and metric policy v2");
        return Output::failure(std::move(rejected).error());
    }
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

Result<RunOutcome,ExperimentErrorCode> RunContext::publishAdaptiveOutcome() {
    using Output=Result<RunOutcome,ExperimentErrorCode>;
    if(!adaptiveKind(kind_)||manifest_published_||!write_started_) {
        auto rejected=contextFailure(run_id_,ExperimentErrorCode::OutputFailed,
            "Adaptive Manifest cannot precede outputs or replace a published Manifest");
        return Output::failure(std::move(rejected).error());
    }
    try {
        auto json=experiments::makeAdaptiveManifestJson(*this,writer_.fileEvidence());
        if(!json.hasValue())return Output::failure(std::move(json).error());
        RunOutcome outcome;
        outcome.run_id=run_id_;
        outcome.status=adaptiveStatisticsComplete()?RunStatus::Complete:RunStatus::Incomplete;
        outcome.attempted=draft_.attempted;outcome.completed=draft_.completed;
        outcome.valid=draft_.valid;outcome.failed=draft_.failed;outcome.undefined=draft_.undefined;
        const RunStatus status=outcome.status;
        auto completed=Output::success(std::move(outcome));
        auto persisted=manifest_publisher_.publishNew("manifest.json",json.value());
        if(!persisted.hasValue()) {
            ErrorDetail d;d.stage=ErrorStage::Output;d.run_id=run_id_;
            d.message="Adaptive Manifest publication failed or durability unconfirmed";
            return Output::failure(wrapFailure(
                ExperimentErrorCode::OutputFailed,std::move(d),persisted.error()));
        }
        manifest_published_=true;draft_.status=status;
        return completed;
    } catch(const std::bad_alloc&) {
        auto rejected=contextFailure(run_id_,ExperimentErrorCode::OutputFailed,
            "Adaptive Manifest evidence allocation failed before confirmed outcome");
        return Output::failure(std::move(rejected).error());
    } catch(const std::length_error&) {
        auto rejected=contextFailure(run_id_,ExperimentErrorCode::OutputFailed,
            "Adaptive Manifest evidence exceeds container capacity");
        return Output::failure(std::move(rejected).error());
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
                       !draft_.files_writer_confirmed.empty()||
                       (adaptiveKind(kind_)&&(draft_.undefined>0||
                        !adaptive_metric_validity_json_.empty())))
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
    draft_.errors.push_back(detail::withRunIdIfMissing(std::move(failure),run_id_));
}

}  // namespace egomotion
