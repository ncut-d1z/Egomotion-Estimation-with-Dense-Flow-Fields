#include "../support/adaptive_result_fixture.hpp"
#include "egomotion/experiments/run_context.hpp"
#include "egomotion/experiments/manifest_json.hpp"
#include "egomotion/experiments/output_contract.hpp"
#include "egomotion/contracts/diagnostics.hpp"

#include <cstddef>
#include <filesystem>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {
using namespace egomotion;
void require(bool value,const char* message) {
    if(!value)throw std::runtime_error(message);
}
struct FixedRandom final: IRandomSource {
    Result<Scalar,NumericalErrorCode> uniform() override {
        return Result<Scalar,NumericalErrorCode>::success(0.5);
    }
    Result<Scalar,NumericalErrorCode> normal() override {
        return Result<Scalar,NumericalErrorCode>::success(0.0);
    }
};
struct FakeWriter final:IResultWriter {
    std::size_t calls=0;
    std::size_t fail_at=0;
    bool fail_after_publication=false;
    std::vector<FileEvidence> receipts;
    Status<MatErrorCode> saveNewVariable(
        const RunId&,std::string_view name,const DenseDoubleArray& a) override {
        ++calls;
        if(fail_at&&calls==fail_at){
            if(fail_after_publication)
                receipts.push_back(FileEvidence{
                    std::string(name),std::string(name)+".mat",
                    std::string(64,'a'),a.info.data_bytes,false});
            ErrorDetail d;d.stage=ErrorStage::Output;
            d.message=fail_after_publication?
                "MAT linked but directory durability unconfirmed":"Injected MAT Writer failure";
            return Status<MatErrorCode>::failure(
                {fail_after_publication?MatErrorCode::ReadbackMismatch:
                    MatErrorCode::WriteFailed,std::move(d)});
        }
        receipts.push_back(FileEvidence{
            std::string(name),std::string(name)+".mat",
            std::string(64,'a'),a.info.data_bytes,true});
        return Status<MatErrorCode>::success();
    }
    std::vector<FileEvidence> fileEvidence() const override {return receipts;}
};
struct FakePublisher final:ITextPublisher{
    std::string name;
    std::string contents;
    bool fail=false;
    bool fail_after_publication=false;
    Status<MatErrorCode> publishNew(
        std::string_view filename,std::string_view data) override {
        if(!name.empty()){
            ErrorDetail detail;detail.stage=ErrorStage::Output;
            detail.message="Manifest name already exists; no overwrite";
            return Status<MatErrorCode>::failure(
                {MatErrorCode::OutputExists,std::move(detail)});
        }
        if(fail){
            ErrorDetail detail;detail.stage=ErrorStage::Output;
            detail.message="Injected Manifest publication failure";
            return Status<MatErrorCode>::failure(
                {MatErrorCode::WriteFailed,std::move(detail)});
        }
        name=std::string(filename);
        contents=std::string(data);
        if(fail_after_publication){
            ErrorDetail detail;detail.stage=ErrorStage::Output;
            detail.message="Manifest linked but directory durability unconfirmed";
            return Status<MatErrorCode>::failure(
                {MatErrorCode::ReadbackMismatch,std::move(detail)});
        }
        return Status<MatErrorCode>::success();
    }
};
DenseDoubleArray matrix(const std::string& name,std::size_t rows,std::size_t cols){
    DenseDoubleArray a;
    a.info.name=name;
    a.info.dimensions={rows,cols};
    a.info.original_class_type=6;
    a.info.original_data_type=9;
    a.info.data_bytes=rows*cols*sizeof(double);
    a.values_column_major.assign(rows*cols,1.0);
    return a;
}
auto fields(egomotion::experiments::ExperimentKind kind){
    std::map<std::string,DenseDoubleArray> result;
    for(const auto& name:egomotion::experiments::currentSaveFields(kind)){
        const bool rates=kind==egomotion::experiments::ExperimentKind::AdaptiveUpdateRates;
        const std::size_t rows=kind==egomotion::experiments::ExperimentKind::AdaptiveInitialization?35:600;
        const std::size_t cols=rates&&name!="akrot"?4:1;
        result.emplace(name,matrix(name,rows,cols));
    }
    const std::size_t rows=kind==egomotion::experiments::ExperimentKind::AdaptiveInitialization?35:600;
    return test_support::completeAdaptiveData(std::move(result),rows);
}
SourceProvenance provenance() {
    SourceProvenance p;
    p.repository_commit_sha=std::string(40,'a');
    p.matlab_source_blob_sha=std::string(40,'b');
    p.rng_seed=13;
    return p;
}
bool hasMatCause(const Failure<ExperimentErrorCode>& failure,const char* code) {
    for(const auto& cause:failure.detail.causes)
        if(cause.domain=="MatErrorCode"&&cause.code==code)return true;
    return false;
}
std::string lifecycleSnapshot(const RunContext& run) {
    const auto& draft=run.manifestDraft();
    std::ostringstream snapshot;
    snapshot<<static_cast<int>(draft.status)<<' '<<draft.attempted<<' '<<draft.completed
            <<' '<<draft.valid<<' '<<draft.failed<<' '<<draft.undefined
            <<' '<<draft.expected_repetitions<<' '<<run.adaptiveComputationComplete()
            <<' '<<run.adaptiveStatisticsComplete()<<'\n';
    for(const auto& name:draft.files_writer_confirmed)snapshot<<name<<'\n';
    for(const auto& error:draft.errors)snapshot<<jsonFailure(error,6)<<'\n';
    for(const auto& name:run.expectedOutputNames())snapshot<<name<<'\n';
    snapshot<<run.adaptiveMetricValidityJson();
    return snapshot.str();
}
void checkBiasCompletionNeedsValidSamples() {
    using egomotion::experiments::ExperimentKind;
    for(const auto kind:{ExperimentKind::BiasOccluded,ExperimentKind::BiasProportional}) {
        FakeWriter writer;FakePublisher publisher;
        RunContext run(RunId{"bias_missing_valid_samples"},std::make_unique<FixedRandom>(),
            writer,publisher,"/R5-UNIT-ONLY",provenance(),kind,1);
        require(!run.noteCompletedMemory(1,0).hasValue(),
                "Bias completion requires at least one valid source observation");
        // 公开进度入口不能绕过完成入口对有效样本的要求；全套 MAT 回执不替代计数。
        run.noteAttempt();run.noteCompleted();
        std::map<std::string,DenseDoubleArray> outputs;
        for(const auto& name:experiments::currentSaveFields(kind))
            outputs.emplace(name,matrix(name,1,name=="xwerte"?1:3));
        require(run.writeCurrentResults(outputs).hasValue(),"Prepare all Bias output receipts");
        const auto before=lifecycleSnapshot(run);
        const auto written=writer.calls;
        const auto denied=run.publishCompleteOutcome();
        require(!denied.hasValue()&&denied.error().code==ExperimentErrorCode::OutputFailed&&
                denied.error().detail.stage==ErrorStage::Output&&denied.error().detail.run_id&&
                denied.error().detail.run_id->text==run.runId().text,
                "Complete publication must reject zero valid Bias samples with RunId");
        require(publisher.name.empty()&&publisher.contents.empty()&&writer.calls==written&&
                lifecycleSnapshot(run)==before,
                "Rejected completion must publish nothing and preserve the existing audit and MAT receipts");

        // 补齐实际三组样本进度后，同一批已确认 MAT 可以正常完成，无须重写。
        run.noteValid();run.noteValid();run.noteValid();
        const auto completed=run.publishCompleteOutcome();
        require(completed.hasValue()&&completed.value().status==RunStatus::Complete&&
                completed.value().valid==3&&run.manifestDraft().valid==3&&writer.calls==written&&
                publisher.contents.find("\"valid\":3")!=std::string::npos,
                "Recorded valid samples must permit Complete with unchanged output receipts");
    }
}
void checkPublishedBiasLifecycle() {
    using egomotion::experiments::ExperimentKind;
    ErrorDetail detail;detail.stage=ErrorStage::Output;
    detail.message="Synthetic terminal output failure";
    const Failure<ExperimentErrorCode> original{ExperimentErrorCode::OutputFailed,std::move(detail)};
    const auto adaptive=fields(ExperimentKind::AdaptiveInitialization);
    for(const auto kind:{ExperimentKind::BiasOccluded,ExperimentKind::BiasProportional}) {
        std::map<std::string,DenseDoubleArray> outputs;
        for(const auto& name:experiments::currentSaveFields(kind))
            outputs.emplace(name,matrix(name,1,name=="xwerte"?1:3));
        for(const auto terminal:{RunStatus::Complete,RunStatus::Incomplete,RunStatus::Failed}) {
            FakeWriter writer;FakePublisher publisher;
            RunContext run(RunId{"sealed_bias"},std::make_unique<FixedRandom>(),writer,publisher,
                "/R5-UNIT-ONLY",provenance(),kind,1);
            if(terminal!=RunStatus::Failed)
                require(run.noteCompletedMemory(1,3).hasValue(),"Prepare completed Bias memory");
            if(terminal==RunStatus::Complete) {
                require(run.writeCurrentResults(outputs).hasValue(),"Prepare complete Bias receipts");
                require(run.publishCompleteOutcome().hasValue(),"Publish Complete Bias outcome");
            } else {
                // 两类失败均未开始写 MAT；仅靠 write_started_ 无法封存此 RunId。
                require(run.publishFailureRecord(original).hasValue(),"Publish terminal Bias failure");
            }
            require(run.manifestDraft().status==terminal,"Fixture must publish its intended status");
            const auto before=lifecycleSnapshot(run);
            const auto manifest=publisher.contents;
            const auto calls=writer.calls;
            const auto written=run.writeCurrentResults(outputs);
            const auto counted=run.noteCompletedMemory(1,9);
            require(!written.hasValue()&&written.error().code==ExperimentErrorCode::OutputFailed&&
                    written.error().detail.stage==ErrorStage::Output&&
                    written.error().detail.run_id&&written.error().detail.run_id->text==run.runId().text&&
                    !counted.hasValue()&&counted.error().code==ExperimentErrorCode::IncompleteRun&&
                    counted.error().detail.stage==ErrorStage::Simulation&&
                    counted.error().detail.run_id&&counted.error().detail.run_id->text==run.runId().text,
                    "Published Bias must reject later writing/completion with contextual diagnostics");
            run.noteAttempt();run.noteCompleted();run.noteValid();run.noteFailed();run.noteUndefined();
            run.recordFailure(original);
            require(!run.writeAdaptiveResults(adaptive).hasValue()&&
                    !run.noteAdaptiveCompletedMemory(adaptive).hasValue()&&
                    !run.publishCompleteOutcome().hasValue()&&
                    !run.publishAdaptiveOutcome().hasValue()&&
                    !run.publishFailureRecord(original).hasValue(),
                    "No stateful gateway may replace a published Bias outcome");
            require(lifecycleSnapshot(run)==before&&writer.calls==calls&&
                    publisher.name=="manifest.json"&&publisher.contents==manifest,
                    "Late progress, errors and retries must preserve the entire published audit snapshot");
        }
    }
}
void checkFailedWriteAuditStatus() {
    using egomotion::experiments::ExperimentKind;
    for(const auto kind:{ExperimentKind::BiasOccluded,ExperimentKind::BiasProportional}) {
        std::map<std::string,DenseDoubleArray> outputs;
        for(const auto& name:experiments::currentSaveFields(kind))
            outputs.emplace(name,matrix(name,1,name=="xwerte"?1:3));
        for(const bool published:{false,true}) {
            FakeWriter writer;FakePublisher publisher;
            writer.fail_at=1;writer.fail_after_publication=published;
            RunContext run(RunId{"first_write_audit"},std::make_unique<FixedRandom>(),
                writer,publisher,"/R5-UNIT-ONLY",provenance(),kind,1);
            // 替身模拟发布前失败或发布后未确认；不进行文件 I/O，也未调用完成计数。
            const auto written=run.writeCurrentResults(outputs);
            require(!written.hasValue()&&writer.calls==1&&
                    run.manifestDraft().files_writer_confirmed.empty(),
                    "The failed first write must not create a confirmed-file identity");
            require(run.manifestDraft().completed==0&&run.manifestDraft().valid==0,
                    "The fixture must not infer completion from valid output arrays");
            const auto before=jsonFailure(written.error(),6);
            require(run.publishFailureRecord(written.error()).hasValue(),
                    "First-write failure must retain a truthful audit");
            const auto expected=published?RunStatus::Incomplete:RunStatus::Failed;
            require(run.manifestDraft().status==expected&&
                    publisher.contents.find(published?"\"completeness\":\"Incomplete\"":
                        "\"completeness\":\"Failed\"")!=std::string::npos,
                    "In-memory and serialized failure status must agree on published evidence");
            require(run.manifestDraft().failed==1&&run.manifestDraft().errors.size()==1&&
                    jsonFailure(run.manifestDraft().errors.front(),6)==before&&
                    written.error().detail.run_id&&
                    written.error().detail.run_id->text==run.runId().text&&
                    hasMatCause(written.error(),published?"ReadbackMismatch":"WriteFailed"),
                    "Failure audit must preserve the original RunId and full Writer error chain");
            if(published) {
                require(writer.receipts.size()==1&&!writer.receipts.front().durably_confirmed&&
                        publisher.contents.find("\"file\":\"xwerte.mat\"")!=std::string::npos&&
                        publisher.contents.find("\"durably_confirmed\":false")!=std::string::npos,
                        "An unconfirmed published MAT must remain visible and unconfirmed");
            } else {
                require(writer.receipts.empty()&&
                        publisher.contents.find("\"mat_files\":[]")!=std::string::npos,
                        "A failure before publication must not invent output evidence");
            }
            const auto retained=publisher.contents;
            require(!run.publishCompleteOutcome().hasValue()&&publisher.contents==retained,
                    "A published failure audit cannot be replaced by Complete");
        }
    }
}
void checkFailureRunIdentity() {
    ErrorDetail root;root.stage=ErrorStage::Read;
    root.source_path="只读 输入.mat";root.variable="xwerte";root.observation_index=7;
    root.run_id=RunId{"retained_inner_identity"};root.message="Root read failure";
    ErrorDetail outer;outer.stage=ErrorStage::Simulation;outer.message="Outer failure";
    const auto original=wrapFailure(ExperimentErrorCode::SourceUnavailable,std::move(outer),
        Failure<MatErrorCode>{MatErrorCode::MissingFile,std::move(root)});
    const auto before=jsonFailure(original,5);
    for(const std::string id:{"identity_a","identity_b","identity_known"}) {
        FakeWriter writer;FakePublisher publisher;
        RunContext run(RunId{id},std::make_unique<FixedRandom>(),writer,publisher,
            "/R5-UNIT-ONLY",provenance(),experiments::ExperimentKind::AdaptiveInitialization,1);
        auto submitted=original;
        if(id=="identity_known")submitted.detail.run_id=RunId{"retained_outer_identity"};
        auto expected=submitted;
        if(!expected.detail.run_id)expected.detail.run_id=run.runId();
        require(run.publishFailureRecord(submitted).hasValue(),"Publish contextual failure");
        require(run.manifestDraft().errors.size()==1&&
                jsonFailure(run.manifestDraft().errors.front(),5)==jsonFailure(expected,5),
                "Fill only missing outer RunId; preserve every original error field and cause");
        require(publisher.contents.find("\"errors\":["+jsonFailure(expected,5)+"]")!=std::string::npos,
                "Manifest must persist the same contextual error");
        require(jsonFailure(original,5)==before,"Annotating a copy must not mutate shared source failure");
    }
}
void checkPublishedButUnconfirmedManifest() {
    using egomotion::experiments::ExperimentKind;
    FakeWriter writer;
    FakePublisher publisher;
    publisher.fail_after_publication=true;
    RunContext run(RunId{"test_manifest_postpublication"},
        std::make_unique<FixedRandom>(),writer,publisher,
        std::filesystem::path("/R5-UNIT-ONLY"),provenance(),
        ExperimentKind::AdaptiveInitialization,1);
    require(run.noteAdaptiveCompletedMemory(fields(ExperimentKind::AdaptiveInitialization)).hasValue()&&
            run.writeAdaptiveResults(fields(ExperimentKind::AdaptiveInitialization)).hasValue(),
            "Synthetic scientific/Writer inputs must be complete");

    // 模拟 link 成功后 fsync 失败：目录中已有文件，但调用者没有成功回执。
    const auto outcome=run.publishAdaptiveOutcome();
    require(!outcome.hasValue()&&outcome.error().code==ExperimentErrorCode::OutputFailed&&
            hasMatCause(outcome.error(),"ReadbackMismatch"),
            "Unconfirmed durability must preserve the cause and reject success");
    require(run.manifestDraft().status!=RunStatus::Complete,
            "Visible Manifest bytes alone cannot commit the in-memory Complete state");
    require(publisher.name=="manifest.json"&&
            publisher.contents.find("\"completeness\":\"Complete\"")!=std::string::npos,
            "Post-publication failure fixture must retain the already linked bytes");
    const auto visible_contents=publisher.contents;

    publisher.fail_after_publication=false;
    const auto audit=run.publishFailureRecord(outcome.error());
    require(!audit.hasValue()&&audit.error().code==ExperimentErrorCode::OutputFailed&&
            hasMatCause(audit.error(),"OutputExists"),
            "An existing Manifest must cause a separate failure-record error");
    require(publisher.contents==visible_contents&&
            run.manifestDraft().status==RunStatus::Incomplete&&
            run.manifestDraft().errors.size()==1&&
            hasMatCause(run.manifestDraft().errors.front(),"ReadbackMismatch"),
            "Preserve published evidence and the original error without claiming recovery");
    require(!run.publishAdaptiveOutcome().hasValue()&&publisher.contents==visible_contents,
            "Retrying Complete must neither overwrite evidence nor return success");
}
}

int main() {
    using namespace egomotion;
    using egomotion::experiments::ExperimentKind;
    checkBiasCompletionNeedsValidSamples();
    checkPublishedBiasLifecycle();
    checkFailedWriteAuditStatus();
    checkFailureRunIdentity();
    checkPublishedButUnconfirmedManifest();
    FakeWriter writer;
    FakePublisher publisher;
    RunContext run(RunId{"test_complete"},std::make_unique<FixedRandom>(),
        writer,publisher,std::filesystem::path("/R5-UNIT-ONLY"),provenance(),
        ExperimentKind::AdaptiveInitialization,1);

    require(!run.publishAdaptiveOutcome().hasValue(),
            "No Complete outcome may precede file writing");
    require(run.noteAdaptiveCompletedMemory(fields(ExperimentKind::AdaptiveInitialization)).hasValue(),
            "Complete independent scientific memory output");
    auto data=fields(ExperimentKind::AdaptiveInitialization);
    require(run.writeAdaptiveResults(data).hasValue(),"Writer should receive source MATs and validity companions");
    require(writer.calls==9,"Writer must receive policy, four masks and four source fields");
    auto result=run.publishAdaptiveOutcome();
    require(result.hasValue()&&result.value().status==RunStatus::Complete,
            "Only after durable manifest is Complete possible");
    require(publisher.name=="manifest.json","Manifest filename must be exact");
    require(publisher.contents.find("\"completeness\":\"Complete\"")!=std::string::npos,
            "Durable Manifest content must reflect verified complete data");
    require(!run.publishAdaptiveOutcome().hasValue(),
            "Double Complete publication must be rejected");

    FakeWriter failed_writer;
    failed_writer.fail_at=2;
    FakePublisher failure_pub;
    RunContext partial(RunId{"test_partial"},std::make_unique<FixedRandom>(),
        failed_writer,failure_pub,std::filesystem::path("/R5-UNIT-ONLY"),provenance(),
        ExperimentKind::AdaptiveInitialization,1);
    auto cannot_write=partial.writeAdaptiveResults(data);
    require(!cannot_write.hasValue(),"Injected Writer failure must propagate");
    require(failed_writer.calls==2,"Failure cannot skip a planned Writer call");
    require(!partial.publishAdaptiveOutcome().hasValue(),
            "Partial MAT receipts cannot publish Complete");
    require(partial.publishFailureRecord(cannot_write.error()).hasValue(),
            "Failed/Incomplete manifest must remain publishable");
    require(failure_pub.contents.find("\"completeness\":\"Incomplete\"")!=std::string::npos,
            "One confirmed MAT is an Incomplete, not Failed, audit");
    require(failure_pub.contents.find("MatErrorCode")!=std::string::npos,
            "Failure Manifest must preserve underlying MAT error family");

    FakeWriter rates_writer;
    FakePublisher rates_pub;
    RunContext rates(RunId{"test_rates"},std::make_unique<FixedRandom>(),
        rates_writer,rates_pub,std::filesystem::path("/R5-UNIT-ONLY"),provenance(),
        ExperimentKind::AdaptiveUpdateRates,1);
    auto output=fields(ExperimentKind::AdaptiveUpdateRates);
    output.fields.emplace("rottransratio",matrix("rottransratio",600,1)); // HistoricalOnly
    const auto invalid_fields=rates.writeAdaptiveResults(output);
    require(!invalid_fields.hasValue()&&invalid_fields.error().detail.run_id&&
            invalid_fields.error().detail.run_id->text==rates.runId().text,
            "UpdateRates must refuse HistoricalOnly fields before MAT IO");
    require(rates_writer.calls==0,"Illegal save set must not reach Writer");

    // R6: receipts must contain a verified per-file digest, not just a
    // successful Writer status. Invalid evidence cannot create Complete.
    FakeWriter unverified_writer;
    FakePublisher unverified_publisher;
    RunContext unverified(RunId{"test_unverified_sha"},
        std::make_unique<FixedRandom>(),unverified_writer,
        unverified_publisher,std::filesystem::path("/R5-UNIT-ONLY"),
        provenance(),ExperimentKind::AdaptiveInitialization,1);
    require(unverified.noteAdaptiveCompletedMemory(fields(ExperimentKind::AdaptiveInitialization)).hasValue(),
            "Synthetic in-memory run must set truthful completed count");
    require(unverified.writeAdaptiveResults(data).hasValue(),
            "Synthetic Writer must accept all current MATLAB fields");
    unverified_writer.receipts[0].sha256="NOT_A_VERIFIED_SHA256";
    auto denied_sha=unverified.publishAdaptiveOutcome();
    require(!denied_sha.hasValue()&&unverified_publisher.contents.empty(),
            "Missing/invalid SHA256 evidence must block Complete publication");
    require(unverified.publishFailureRecord(denied_sha.error()).hasValue(),
            "Unverified receipts must remain in an Incomplete audit, not disappear");
    require(unverified_publisher.contents.find("\"completeness\":\"Incomplete\"")!=
                std::string::npos &&
            unverified_publisher.contents.find("\"sha256\":null")!=
                std::string::npos,
            "Incomplete Manifest must identify unverified SHA as null");
    require(!unverified.publishAdaptiveOutcome().hasValue(),
            "Published Incomplete Manifest must permanently bar later Complete");

    // R6: a Publisher failure is independent of scientific success. In
    // particular, all MAT receipts do NOT authorize a success RunOutcome
    // when manifest.json publication/durability cannot be confirmed.
    FakeWriter complete_writer;
    FakePublisher unreliable_publisher;
    unreliable_publisher.fail=true;
    RunContext incomplete(RunId{"test_manifest_failure"},
        std::make_unique<FixedRandom>(),complete_writer,
        unreliable_publisher,std::filesystem::path("/R5-UNIT-ONLY"),
        provenance(),ExperimentKind::AdaptiveInitialization,1);
    require(incomplete.noteAdaptiveCompletedMemory(fields(ExperimentKind::AdaptiveInitialization)).hasValue()&&
            incomplete.writeAdaptiveResults(data).hasValue(),
            "Before publication all synthetic scientific/Writer inputs must be ready");
    auto cannot_complete=incomplete.publishAdaptiveOutcome();
    require(!cannot_complete.hasValue()&&unreliable_publisher.contents.empty(),
            "Injected failed Manifest write must never claim a Complete outcome");
    bool underlying_mat_writer=false;
    for(const auto& cause:cannot_complete.error().detail.causes)
        underlying_mat_writer|=(cause.domain=="MatErrorCode"&&
                                cause.code=="WriteFailed");
    require(underlying_mat_writer,
            "Manifest publication failure must retain underlying MAT I/O cause");
    unreliable_publisher.fail=false;
    require(incomplete.publishFailureRecord(cannot_complete.error()).hasValue(),
            "Independent failure record should remain publishable when backend recovers");
    require(unreliable_publisher.contents.find("\"completeness\":\"Incomplete\"")!=
                std::string::npos &&
            unreliable_publisher.contents.find("\"completeness\":\"Complete\"")==
                std::string::npos,
            "After a failed Complete publication only truthful Incomplete is allowed");

    // An executable built from an unverified/dirty source identity may write
    // an Incomplete audit but must not attach a fictional 40-hex repo_sha.
    auto missing_provenance=provenance();
    missing_provenance.repository_commit_sha="UNVERIFIED_BUILD_SOURCE";
    FakeWriter dirty_writer;
    FakePublisher dirty_publisher;
    RunContext dirty(RunId{"test_dirty_source"},
        std::make_unique<FixedRandom>(),dirty_writer,
        dirty_publisher,std::filesystem::path("/R5-UNIT-ONLY"),
        std::move(missing_provenance),ExperimentKind::AdaptiveInitialization,1);
    require(dirty.noteAdaptiveCompletedMemory(fields(ExperimentKind::AdaptiveInitialization)).hasValue()&&
            dirty.writeAdaptiveResults(data).hasValue(),
            "Synthetic unverified-build input setup failed");
    auto blocked=dirty.publishAdaptiveOutcome();
    require(!blocked.hasValue()&&dirty_publisher.name.empty(),
            "Unverified executable source identity must block Complete Manifest");
    require(blocked.error().detail.run_id&&blocked.error().detail.run_id->text==dirty.runId().text,
            "Manifest serialization failure must retain allocated RunId");
}
