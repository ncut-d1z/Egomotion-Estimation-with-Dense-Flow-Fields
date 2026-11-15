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
    Status<MatErrorCode> publishNew(
        std::string_view filename,std::string_view data) override {
        if(fail){
            ErrorDetail detail;detail.stage=ErrorStage::Output;
            detail.message="Injected Manifest publication failure";
            return Status<MatErrorCode>::failure(
                {MatErrorCode::WriteFailed,std::move(detail)});
        }
        name=std::string(filename);
        contents=std::string(data);
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
}

int main() {
    using namespace egomotion;
    using egomotion::experiments::ExperimentKind;
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
}
