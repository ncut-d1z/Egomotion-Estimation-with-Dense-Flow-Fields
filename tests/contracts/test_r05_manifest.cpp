#include "egomotion/experiments/run_context.hpp"
#include "egomotion/experiments/manifest_json.hpp"
#include "egomotion/experiments/output_contract.hpp"

#include <cstddef>
#include <filesystem>
#include <map>
#include <memory>
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
    std::vector<FileEvidence> receipts;
    Status<MatErrorCode> saveNewVariable(
        const RunId&,std::string_view name,const DenseDoubleArray& a) override {
        ++calls;
        if(fail_at&&calls==fail_at){
            ErrorDetail d;d.stage=ErrorStage::Output;
            d.message="Injected MAT Writer failure";
            return Status<MatErrorCode>::failure(
                {MatErrorCode::WriteFailed,std::move(d)});
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
    return result;
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
void checkPublishedButUnconfirmedManifest() {
    using egomotion::experiments::ExperimentKind;
    FakeWriter writer;
    FakePublisher publisher;
    publisher.fail_after_publication=true;
    RunContext run(RunId{"test_manifest_postpublication"},
        std::make_unique<FixedRandom>(),writer,publisher,
        std::filesystem::path("/R5-UNIT-ONLY"),provenance(),
        ExperimentKind::AdaptiveInitialization,1);
    require(run.noteCompletedMemory(1,33).hasValue()&&
            run.writeCurrentResults(fields(ExperimentKind::AdaptiveInitialization)).hasValue(),
            "Synthetic scientific/Writer inputs must be complete");

    // 模拟 link 成功后 fsync 失败：目录中已有文件，但调用者没有成功回执。
    const auto outcome=run.publishCompleteOutcome();
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
    require(!run.publishCompleteOutcome().hasValue()&&publisher.contents==visible_contents,
            "Retrying Complete must neither overwrite evidence nor return success");
}
}

int main() {
    using namespace egomotion;
    using egomotion::experiments::ExperimentKind;
    checkPublishedButUnconfirmedManifest();
    FakeWriter writer;
    FakePublisher publisher;
    RunContext run(RunId{"test_complete"},std::make_unique<FixedRandom>(),
        writer,publisher,std::filesystem::path("/R5-UNIT-ONLY"),provenance(),
        ExperimentKind::AdaptiveInitialization,1);

    require(!run.publishCompleteOutcome().hasValue(),
            "No Complete outcome may precede file writing");
    require(run.noteCompletedMemory(1,33).hasValue(),
            "Complete independent scientific memory output");
    auto data=fields(ExperimentKind::AdaptiveInitialization);
    require(run.writeCurrentResults(data).hasValue(),"Writer should receive all four MAT");
    require(writer.calls==4,"Writer current-save calls must equal source saves");
    auto result=run.publishCompleteOutcome();
    require(result.hasValue()&&result.value().status==RunStatus::Complete,
            "Only after durable manifest is Complete possible");
    require(publisher.name=="manifest.json","Manifest filename must be exact");
    require(publisher.contents.find("\"completeness\":\"Complete\"")!=std::string::npos,
            "Durable Manifest content must reflect verified complete data");
    require(!run.publishCompleteOutcome().hasValue(),
            "Double Complete publication must be rejected");

    FakeWriter failed_writer;
    failed_writer.fail_at=2;
    FakePublisher failure_pub;
    RunContext partial(RunId{"test_partial"},std::make_unique<FixedRandom>(),
        failed_writer,failure_pub,std::filesystem::path("/R5-UNIT-ONLY"),provenance(),
        ExperimentKind::AdaptiveInitialization,1);
    auto cannot_write=partial.writeCurrentResults(data);
    require(!cannot_write.hasValue(),"Injected Writer failure must propagate");
    require(failed_writer.calls==2,"Failure cannot skip a planned Writer call");
    require(!partial.publishCompleteOutcome().hasValue(),
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
    output.emplace("rottransratio",matrix("rottransratio",600,1)); // HistoricalOnly
    require(!rates.writeCurrentResults(output).hasValue(),
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
    require(unverified.noteCompletedMemory(1,33).hasValue(),
            "Synthetic in-memory run must set truthful completed count");
    require(unverified.writeCurrentResults(data).hasValue(),
            "Synthetic Writer must accept all current MATLAB fields");
    unverified_writer.receipts[0].sha256="NOT_A_VERIFIED_SHA256";
    auto denied_sha=unverified.publishCompleteOutcome();
    require(!denied_sha.hasValue()&&unverified_publisher.contents.empty(),
            "Missing/invalid SHA256 evidence must block Complete publication");
    require(unverified.publishFailureRecord(denied_sha.error()).hasValue(),
            "Unverified receipts must remain in an Incomplete audit, not disappear");
    require(unverified_publisher.contents.find("\"completeness\":\"Incomplete\"")!=
                std::string::npos &&
            unverified_publisher.contents.find("\"sha256\":null")!=
                std::string::npos,
            "Incomplete Manifest must identify unverified SHA as null");
    require(!unverified.publishCompleteOutcome().hasValue(),
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
    require(incomplete.noteCompletedMemory(1,33).hasValue()&&
            incomplete.writeCurrentResults(data).hasValue(),
            "Before publication all synthetic scientific/Writer inputs must be ready");
    auto cannot_complete=incomplete.publishCompleteOutcome();
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
    require(dirty.noteCompletedMemory(1,33).hasValue()&&
            dirty.writeCurrentResults(data).hasValue(),
            "Synthetic unverified-build input setup failed");
    auto blocked=dirty.publishCompleteOutcome();
    require(!blocked.hasValue()&&dirty_publisher.name.empty(),
            "Unverified executable source identity must block Complete Manifest");
}
