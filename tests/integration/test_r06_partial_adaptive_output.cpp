// 纯内存合同：不链接 MAT/Sophus 适配器，不创建文件，不执行轨迹或 Step 求解。
// 替身回执只测试发布门禁；不构成 MAT 持久化或完整科学实验的证据。
#include "adaptive_aggregation.hpp"
#include "egomotion/experiments/output_values.hpp"
#include "egomotion/experiments/run_context.hpp"

#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {
using namespace egomotion;
using experiments::AdaptiveRunData;
using experiments::ExperimentKind;
constexpr std::size_t kFrames=600,kRuns=8,kMissingRun=3,kMissingRow=17;
constexpr const char* kMissingField="allemittransfehler";
std::size_t checks=0;
void require(bool condition,const std::string& message) {
    ++checks;
    if(!condition)throw std::runtime_error(message);
}
AdaptiveRunData oneRepetition(std::size_t run,bool missing) {
    AdaptiveRunData data;
    data.frame_count=kFrames;data.planned_runs=kRuns;data.valid_frames=kFrames-2;
    std::size_t field=0;
    for(const auto& name:experiments::currentSaveFields(ExperimentKind::AdaptiveComparison)) {
        DenseDoubleArray array;
        array.info.name=name;array.info.dimensions={kFrames,1};
        array.info.original_class_type=6;array.info.original_data_type=9;
        array.info.data_bytes=kFrames*sizeof(double);
        array.values_column_major.assign(kFrames,0.0);
        auto& counts=data.valid_counts[name];counts.assign(kFrames,0);
        for(std::size_t row=1;row+1<kFrames;++row) {
            array.values_column_major[row]=4.0*static_cast<double>(run)+
                static_cast<double>(field)/16.0+static_cast<double>(row)/1024.0;
            counts[row]=1;
        }
        data.fields.emplace(name,std::move(array));++field;
    }
    if(missing&&run==kMissingRun) {
        data.fields.at(kMissingField).values_column_major[kMissingRow]=
            std::numeric_limits<double>::quiet_NaN();
        data.valid_counts.at(kMissingField)[kMissingRow]=0;
        --data.valid_frames;++data.undefined_frames;
        ErrorDetail reason;reason.stage=ErrorStage::Solve;
        reason.message="Synthetic zero translation direction at original run/frame";
        data.undefined_metrics.push_back({kMissingField,run,kMissingRow+1,1,
            {NumericalErrorCode::UndefinedAngle,std::move(reason)}});
    }
    return data;
}
AdaptiveRunData aggregate(bool missing) {
    auto total=oneRepetition(1,false);
    total.valid_frames=0;
    for(auto& item:total.fields)
        for(double& value:item.second.values_column_major)value=0.0;
    for(auto& item:total.valid_counts)
        for(auto& count:item.second)count=0;
    for(std::size_t run=1;run<=kRuns;++run) {
        const auto one=oneRepetition(run,missing);
        require(experiments::detail::accumulateCompleteAdaptiveRun(total,one).hasValue(),
                "Complete solver repetition must retain valid or explicitly missing metrics");
    }
    auto finished=experiments::detail::finishAdaptiveAggregation(std::move(total),true);
    require(finished.hasValue(),"Eight solved repetitions must finalize even with missing metrics");
    return std::move(finished).value();
}
void rejectInvalidEvidence(const AdaptiveRunData& partial) {
    auto rejected=[](const AdaptiveRunData& value,const char* why) {
        require(!experiments::makeAdaptiveOutput(ExperimentKind::AdaptiveComparison,value).hasValue(),why);
    };
    auto changed=partial;
    double available_sum=0;
    for(const auto& repetition:partial.repetition_fields) {
        const double value=repetition.at(kMissingField).values_column_major[kMissingRow];
        if(std::isfinite(value))available_sum+=value;
    }
    changed.fields.at(kMissingField).values_column_major[kMissingRow]=available_sum/7.0;
    rejected(changed,"Changing the denominator to seven must not produce an output");
    changed=partial;changed.undefined_metrics.clear();
    rejected(changed,"An undeclared raw NaN must fail before any publication");
    changed=partial;changed.valid_counts.at(kMissingField)[kMissingRow]=kRuns;
    rejected(changed,"A count of eight cannot cover a missing repetition");
    changed=partial;changed.fields.at(kMissingField).values_column_major[18]=
        std::numeric_limits<double>::infinity();
    rejected(changed,"Infinity is invalid, not a missing metric");
    changed=partial;changed.repetition_fields.front().at(kMissingField).values_column_major[18]=
        std::numeric_limits<double>::infinity();
    rejected(changed,"Infinite raw repetition must be rejected");
    changed=partial;changed.fields.at(kMissingField).values_column_major[0]=1.0;
    rejected(changed,"Structural first frame cannot be filled with a synthetic value");
    changed=partial;changed.repetition_fields.front().at(kMissingField).values_column_major[kFrames-1]=1.0;
    rejected(changed,"Structural last frame must be zero in every raw repetition");
    changed=partial;changed.valid_counts.at(kMissingField)[0]=kRuns;
    rejected(changed,"A structural endpoint cannot count as eight observations");
    changed=partial;changed.undefined_metrics.push_back(changed.undefined_metrics.front());
    rejected(changed,"Duplicate missing coordinates must not inflate evidence");
    changed=partial;changed.undefined_metrics.front().matlab_frame_index=1;
    rejected(changed,"Missing coordinates must retain the original interior frame");
    changed=partial;changed.repetition_fields.pop_back();
    rejected(changed,"All eight raw repetitions are required");
    changed=partial;--changed.completed_runs;
    rejected(changed,"A partially solved run cannot masquerade as partial metric delivery");
    changed=partial;++changed.valid_frames;
    rejected(changed,"Frame validity totals must agree with raw missing coordinates");
}
struct FixedRandom final:IRandomSource {
    Result<Scalar,NumericalErrorCode> uniform() override {
        return Result<Scalar,NumericalErrorCode>::success(0.5);
    }
    Result<Scalar,NumericalErrorCode> normal() override {
        return Result<Scalar,NumericalErrorCode>::success(0.0);
    }
};
struct MemoryWriter final:IResultWriter {
    std::map<std::string,DenseDoubleArray> fields;
    std::map<std::string,std::vector<unsigned char>> masks;
    std::vector<FileEvidence> receipts;
    Status<MatErrorCode> saveNewVariable(const RunId&,std::string_view name,
                                       const DenseDoubleArray& value) override {
        for(double element:value.values_column_major)
            require(std::isfinite(element),"Unmasked memory write received a missing/nonfinite value");
        return retain(name,value);
    }
    Status<MatErrorCode> saveNewMaskedVariable(const RunId&,std::string_view name,
        const DenseDoubleArray& value,const std::vector<unsigned char>& defined) override {
        require(defined.size()==value.values_column_major.size(),"Defined mask shape changed");
        for(std::size_t i=0;i<defined.size();++i)
            require((defined[i]==1&&std::isfinite(value.values_column_major[i]))||
                    (defined[i]==0&&std::isnan(value.values_column_major[i])),
                    "Only a declared missing cell may contain NaN");
        require(masks.emplace(std::string(name),defined).second,"Duplicate defined mask write");
        return retain(name,value);
    }
    Status<MatErrorCode> retain(std::string_view name,const DenseDoubleArray& value) {
        const std::string key(name);
        require(fields.emplace(key,value).second,"A memory output cannot overwrite its earlier value");
        receipts.push_back({key,key+".mat",std::string(64,'a'),value.info.data_bytes,true});
        return Status<MatErrorCode>::success();
    }
    std::vector<FileEvidence> fileEvidence() const override {return receipts;}
};
struct MemoryPublisher final:ITextPublisher {
    std::string contents;
    Status<MatErrorCode> publishNew(std::string_view filename,std::string_view data) override {
        require(filename=="manifest.json"&&contents.empty(),"Manifest must be new and have its fixed name");
        contents=std::string(data);
        return Status<MatErrorCode>::success();
    }
};
void verifyPublication(const AdaptiveRunData& data,bool partial) {
    MemoryWriter writer;MemoryPublisher publisher;
    RunContext context(RunId{partial?"memory_partial":"memory_complete"},
        std::make_unique<FixedRandom>(),writer,publisher,"/MEMORY-ONLY-NO-FILES",
        SourceProvenance{std::string(40,'a'),std::string(40,'b'),7},
        ExperimentKind::AdaptiveComparison,kRuns);
    require(!context.writeCurrentResults(data.fields).hasValue()&&
            !context.noteCompletedMemory(kRuns,kRuns*(kFrames-2)).hasValue()&&
            !context.publishCompleteOutcome().hasValue(),
            "Legacy context methods must not bypass Adaptive metric policy v2");
    require(writer.fields.empty()&&publisher.contents.empty()&&
            context.manifestDraft().completed==0,"Rejected legacy calls must have no side effects");
    require(context.noteAdaptiveCompletedMemory(data).hasValue(),"Complete memory evidence must be accepted");
    require(context.writeAdaptiveResults(data).hasValue(),"Explicit validity data must reach masked writes");
    require(writer.receipts.size()==11&&writer.receipts.front().variable=="egomotion_metric_policy",
            "Marker must be written before five companions and five source fields");
    const auto& marker=writer.fields.at("egomotion_metric_policy");
    require(marker.info.dimensions==std::vector<std::size_t>({1,4})&&
            marker.values_column_major==std::vector<double>({1,1,1,8}),
            "Metric policy marker must identify the exact semantics and denominator");
    for(const auto& field:data.fields) {
        const auto& count=writer.fields.at("valid_"+field.first);
        require(count.info.dimensions==field.second.info.dimensions,"Validity companion must preserve dimensions");
        for(std::size_t row=0;row<kFrames;++row)
            require(count.values_column_major[row]==static_cast<double>(data.valid_counts.at(field.first)[row]),
                    "Validity counts changed at the writer boundary");
        const auto& mask=writer.masks.at(field.first);
        require(mask.front()==1&&mask.back()==1,"Structural zeros are defined, with separate zero counts");
    }
    require(writer.masks.at(kMissingField)[kMissingRow]==(partial?0:1),
            "Missing aggregate must have an explicit false defined mask");
    const auto outcome=context.publishAdaptiveOutcome();
    require(outcome.hasValue()&&outcome.value().status==(partial?RunStatus::Incomplete:RunStatus::Complete),
            "Successful delivery must report the actual statistical status");
    require(outcome.value().attempted==kRuns&&outcome.value().completed==kRuns&&
            outcome.value().failed==0&&outcome.value().undefined==(partial?1:0)&&
            outcome.value().valid==kRuns*(kFrames-2)-(partial?1:0),
            "Missing metrics must not be counted as failed or unsolved repetitions");
    const auto& json=publisher.contents;
    require(json.find("\"schema\":\"egomotion-run-manifest-v2\"")!=std::string::npos&&
            json.find("\"computation_complete\":true")!=std::string::npos&&
            json.find("\"metric_policy\":")!=std::string::npos&&
            json.find("\"repetitions\":[")!=std::string::npos&&
            json.find("\"aggregation_denominator\":8")!=std::string::npos&&
            json.find("NaN")==std::string::npos,"Manifest v2 must retain complete metric semantics and raw repetitions");
    require(json.find(partial?"\"statistics_complete\":false":"\"statistics_complete\":true")!=std::string::npos,
            "Manifest statistical completeness must match the delivered outcome");
    if(partial) {
        require(json.find("null")!=std::string::npos&&
                json.find("\"matlab_run_index\":3,\"matlab_frame_index\":18,\"matlab_column_index\":1")!=std::string::npos&&
                json.find("Synthetic zero translation direction at original run/frame")!=std::string::npos,
                "Missing raw values require JSON null, original 1-based identity, and a numerical reason");
    }
    require(!context.publishAdaptiveOutcome().hasValue(),"Published outcome cannot be replaced");
}
} // namespace

int main() {
    try {
        const auto partial=aggregate(true);
        require(partial.completed_runs==8&&partial.repetition_fields.size()==8&&
                partial.undefined_metrics.size()==1,"All original solved repetitions must survive aggregation");
        require(partial.valid_counts.at(kMissingField)[kMissingRow]==7&&
                std::isnan(partial.fields.at(kMissingField).values_column_major[kMissingRow]),
                "Seven valid samples must leave the fixed-eight aggregate missing");
        for(std::size_t run=1;run<=kRuns;++run) {
            const auto raw=partial.repetition_fields[run-1].at(kMissingField).values_column_major[kMissingRow];
            const auto original=oneRepetition(run,true).fields.at(kMissingField).values_column_major[kMissingRow];
            require(run==kMissingRun?std::isnan(raw):raw==original,"Raw values and indices cannot be dropped or renumbered");
        }
        require(experiments::makeAdaptiveOutput(ExperimentKind::AdaptiveComparison,partial).hasValue(),
                "A complete solver result with declared missing metrics is deliverable");
        const auto neighbor=partial.fields.at(kMissingField).values_column_major[kMissingRow+1];
        double expected=0;
        for(std::size_t run=1;run<=kRuns;++run)
            expected+=oneRepetition(run,false).fields.at(kMissingField).values_column_major[kMissingRow+1];
        require(neighbor==expected/8.0,"Unaffected neighboring cell must keep the fixed denominator");
        require(partial.fields.at("rottransratio").values_column_major[18]==
                partial.repetition_fields.back().at("rottransratio").values_column_major[18]&&
                partial.valid_counts.at("rottransratio")[18]==1,"Ratio remains a last-repetition field");
        rejectInvalidEvidence(partial);
        verifyPublication(partial,true);
        const auto complete=aggregate(false);
        require(experiments::makeAdaptiveOutput(ExperimentKind::AdaptiveComparison,complete).hasValue(),
                "Complete finite reference must satisfy the same strict output gate");
        verifyPublication(complete,false);
        std::cout<<"Pure-memory Adaptive output checks="<<checks<<" failures=0\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<"Pure-memory Adaptive output checks="<<checks<<" failure="<<error.what()<<'\n';
        return 1;
    }
}
