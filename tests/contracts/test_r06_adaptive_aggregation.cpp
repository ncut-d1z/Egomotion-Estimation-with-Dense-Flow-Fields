// 纯内存追加/最终化合同：不运行轨迹、求解器或 MAT I/O。
#include "adaptive_aggregation.hpp"
#include "egomotion/experiments/output_values.hpp"
#include "egomotion/contracts/diagnostics.hpp"

#include <cmath>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

namespace {
using namespace egomotion;
using experiments::AdaptiveRunData;
using experiments::ExperimentKind;
using experiments::detail::accumulateCompleteAdaptiveRun;
using experiments::detail::finishAdaptiveAggregation;
constexpr std::size_t kRuns=8,kFrames=600,kGapFrame=18;
constexpr const char* kTranslation="allemittransfehler";
constexpr const char* kRotation="allemitrotfehler";
std::size_t checks=0;
void require(bool value,const std::string& message) {
    ++checks;
    if(!value)throw std::runtime_error(message);
}
AdaptiveRunData repetition(ExperimentKind kind,std::size_t run) {
    AdaptiveRunData data;
    data.frame_count=kind==ExperimentKind::AdaptiveInitialization?35:kFrames;
    data.planned_runs=kind==ExperimentKind::AdaptiveComparison?kRuns:1;
    data.valid_frames=data.frame_count-2;
    std::size_t field=0;
    for(const auto& name:experiments::currentSaveFields(kind)) {
        const std::size_t columns=kind==ExperimentKind::AdaptiveUpdateRates&&name!="akrot"?4:1;
        DenseDoubleArray value;
        value.info.name=name;value.info.dimensions={data.frame_count,columns};
        value.info.original_class_type=6;value.info.original_data_type=9;
        value.info.class_name="double";value.info.data_type_name="double";
        value.info.data_bytes=data.frame_count*columns*sizeof(double);
        value.values_column_major.assign(data.frame_count*columns,0.0);
        auto& counts=data.valid_counts[name];counts.assign(data.frame_count*columns,0);
        for(std::size_t column=0;column<columns;++column)
            for(std::size_t row=1;row+1<data.frame_count;++row) {
                const auto offset=column*data.frame_count+row;
                value.values_column_major[offset]=static_cast<double>(run)+
                    static_cast<double>(field)/16.0+static_cast<double>(column)/8.0+
                    static_cast<double>(row)/1024.0;
                counts[offset]=1;
            }
        data.fields.emplace(name,std::move(value));++field;
    }
    return data;
}
AdaptiveRunData empty(ExperimentKind kind=ExperimentKind::AdaptiveComparison) {
    auto data=repetition(kind,0);data.valid_frames=0;
    for(auto& field:data.fields)for(auto& value:field.second.values_column_major)value=0.0;
    for(auto& field:data.valid_counts)for(auto& count:field.second)count=0;
    return data;
}
void gap(AdaptiveRunData& data,const std::string& variable,std::size_t run,
         std::size_t frame,std::size_t column=1) {
    bool missing_frame=false;
    for(const auto& missing:data.undefined_metrics)
        missing_frame|=missing.matlab_frame_index==frame;
    if(!missing_frame){--data.valid_frames;++data.undefined_frames;}
    const auto offset=(column-1)*data.frame_count+frame-1;
    data.fields.at(variable).values_column_major[offset]=std::numeric_limits<double>::quiet_NaN();
    data.valid_counts.at(variable)[offset]=0;
    const auto code=variable=="rottransratio"?NumericalErrorCode::ZeroNorm:NumericalErrorCode::UndefinedAngle;
    ErrorDetail root;root.stage=ErrorStage::Solve;
    root.message=variable=="rottransratio"?"Fixed zero component denominator":"Fixed zero direction";
    Failure<NumericalErrorCode> reason{code,std::move(root)};
    ErrorDetail outer;outer.stage=ErrorStage::Simulation;outer.message="Fixed missing comparison metric";
    outer.variable=variable;outer.run_id=RunId{"aggregation-fixture"};
    data.undefined_metrics.push_back({variable,run,frame,column,
        wrapFailure(code,std::move(outer),reason)});
}
void sameFields(const std::map<std::string,DenseDoubleArray>& actual,
                const std::map<std::string,DenseDoubleArray>& expected) {
    require(actual.size()==expected.size(),"Rejected operation changed field count");
    for(const auto& field:expected) {
        const auto& a=actual.at(field.first);const auto& b=field.second;
        const auto& x=a.info;const auto& y=b.info;
        require(std::tie(x.name,x.dimensions,x.original_class_type,x.original_data_type,
                        x.is_complex,x.is_logical,x.is_global,x.is_compressed,x.data_bytes,
                        x.class_name,x.data_type_name)==
                std::tie(y.name,y.dimensions,y.original_class_type,y.original_data_type,
                         y.is_complex,y.is_logical,y.is_global,y.is_compressed,y.data_bytes,
                         y.class_name,y.data_type_name)&&a.source_mat_sha256==b.source_mat_sha256,
                "Rejected operation changed field identity or metadata");
        require(a.values_column_major.size()==b.values_column_major.size(),"Payload length changed");
        require(a.values_column_major.empty()||std::memcmp(a.values_column_major.data(),
                b.values_column_major.data(),a.values_column_major.size()*sizeof(double))==0,
                "Rejected operation changed finite or missing raw payload bytes");
    }
}
void unchanged(const AdaptiveRunData& actual,const AdaptiveRunData& expected) {
    require(std::tie(actual.frame_count,actual.planned_runs,actual.completed_runs,
                     actual.valid_frames,actual.undefined_frames)==
            std::tie(expected.frame_count,expected.planned_runs,expected.completed_runs,
                     expected.valid_frames,expected.undefined_frames)&&
            actual.valid_counts==expected.valid_counts,"Rejected operation changed counters");
    sameFields(actual.fields,expected.fields);
    require(actual.repetition_fields.size()==expected.repetition_fields.size(),"Raw repetition count changed");
    for(std::size_t i=0;i<actual.repetition_fields.size();++i)
        sameFields(actual.repetition_fields[i],expected.repetition_fields[i]);
    require(actual.undefined_metrics.size()==expected.undefined_metrics.size(),"Missing reason count changed");
    for(std::size_t i=0;i<actual.undefined_metrics.size();++i) {
        const auto& a=actual.undefined_metrics[i];const auto& b=expected.undefined_metrics[i];
        require(std::tie(a.variable,a.matlab_run_index,a.matlab_frame_index,a.matlab_column_index)==
                std::tie(b.variable,b.matlab_run_index,b.matlab_frame_index,b.matlab_column_index)&&
                jsonFailure(a.reason,5)==jsonFailure(b.reason,5),"Missing identity or error chain changed");
    }
}
void rejectAppend(AdaptiveRunData& total,const AdaptiveRunData& one,const std::string& message) {
    const auto before=total;const auto original=one;
    const auto result=accumulateCompleteAdaptiveRun(total,one);
    require(!result.hasValue()&&result.error().code==ExperimentErrorCode::DataMismatch,message);
    unchanged(total,before);unchanged(one,original);
}
void invalidRepetition() {
    auto total=empty();
    for(std::size_t run=1;run<3;++run)
        require(accumulateCompleteAdaptiveRun(total,repetition(ExperimentKind::AdaptiveComparison,run)).hasValue(),
                "Valid prefix setup failed");
    auto valid=repetition(ExperimentKind::AdaptiveComparison,3);
    gap(valid,kTranslation,3,kGapFrame);gap(valid,kRotation,3,kGapFrame);
    for(const std::string fault:{"no-reason","duplicate","unknown-field","wrong-frame","wrong-column",
            "wrong-run","empty-message","solver-failure","finite-with-reason","wrong-frames",
            "double-counted-frame","wrong-count","wrong-type","wrong-name","wrong-bytes"}) {
        auto bad=valid;
        if(fault=="no-reason")bad.undefined_metrics.clear();
        else if(fault=="duplicate")bad.undefined_metrics.push_back(bad.undefined_metrics.front());
        else if(fault=="unknown-field")bad.undefined_metrics.front().variable="not_a_source_field";
        else if(fault=="wrong-frame")bad.undefined_metrics.front().matlab_frame_index=1;
        else if(fault=="wrong-column")bad.undefined_metrics.front().matlab_column_index=2;
        else if(fault=="wrong-run")bad.undefined_metrics.front().matlab_run_index=2;
        else if(fault=="empty-message")bad.undefined_metrics.front().reason.detail.message.clear();
        else if(fault=="solver-failure")bad.undefined_metrics.front().reason.code=NumericalErrorCode::SingularSystem;
        else if(fault=="finite-with-reason") {
            bad.fields.at(kTranslation).values_column_major[kGapFrame-1]=1.0;
            bad.valid_counts.at(kTranslation)[kGapFrame-1]=1;
        }else if(fault=="wrong-frames") {bad.valid_frames=kFrames-2;bad.undefined_frames=0;}
        else if(fault=="double-counted-frame") {--bad.valid_frames;++bad.undefined_frames;}
        else if(fault=="wrong-count")bad.valid_counts.at(kTranslation)[kGapFrame-1]=1;
        else if(fault=="wrong-type")bad.fields.at(kTranslation).info.is_logical=true;
        else if(fault=="wrong-name")bad.fields.at(kTranslation).info.name="different_internal_identity";
        else if(fault=="wrong-bytes")--bad.fields.at(kTranslation).info.data_bytes;
        rejectAppend(total,bad,"Bad repetition was committed: "+fault);
    }
    require(total.completed_runs==2,"Rejected repetitions advanced the original run index");
    require(accumulateCompleteAdaptiveRun(total,valid).hasValue(),"Same original repetition must remain retryable");
    require(total.completed_runs==3&&total.undefined_frames==1&&
            total.valid_frames==3*(kFrames-2)-1&&total.undefined_metrics.size()==2,
            "Two missing metrics on one frame must count exactly one undefined frame");
}
void invalidPrefix() {
    auto biased=empty();biased.fields.at(kTranslation).values_column_major[2]=1.0;
    rejectAppend(biased,repetition(ExperimentKind::AdaptiveComparison,1),
                 "A zero-run aggregate must not silently include an initial numeric offset");
    auto total=empty();
    require(accumulateCompleteAdaptiveRun(total,repetition(ExperimentKind::AdaptiveComparison,1)).hasValue(),
            "One-run prefix setup failed");
    // 若只检查新候选，下一轮NaN会掩盖旧累计和与原值的矛盾。
    total.fields.at(kTranslation).values_column_major[kGapFrame-1]+=1.0;
    auto next=repetition(ExperimentKind::AdaptiveComparison,2);gap(next,kTranslation,2,kGapFrame);
    rejectAppend(total,next,"New missing value must not hide a corrupted earlier finite sum");
}
AdaptiveRunData completePrefix() {
    auto total=empty();
    for(std::size_t run=1;run<=kRuns;++run) {
        auto one=repetition(ExperimentKind::AdaptiveComparison,run);
        if(run==2)gap(one,"rottransratio",run,23);
        if(run==3){gap(one,kTranslation,run,kGapFrame);gap(one,kRotation,run,kGapFrame);}
        if(run==8)gap(one,"rottransratio",run,42);
        require(accumulateCompleteAdaptiveRun(total,one).hasValue(),"Consistent missing evidence must be accepted");
        if(run<kRuns) {
            const auto& ratio=total.fields.at("rottransratio").values_column_major;
            require(ratio[22]==0.0&&ratio[41]==0.0&&total.valid_counts.at("rottransratio")[22]==0,
                    "An earlier repetition supplied the final-only ratio");
        }
    }
    return total;
}
void rejectFinalization(AdaptiveRunData& total,bool comparison,const std::string& message) {
    const auto before=total;
    const auto result=finishAdaptiveAggregation(std::move(total),comparison);
    require(!result.hasValue(),message);unchanged(total,before);
}
void finalization() {
    const auto prefix=completePrefix();
    for(const std::string fault:{"no-reason","wrong-count","raw-value","wrong-frames","endpoint"}) {
        auto bad=prefix;
        if(fault=="no-reason")bad.undefined_metrics.clear();
        else if(fault=="wrong-count")bad.valid_counts.at(kTranslation)[kGapFrame-1]=8;
        else if(fault=="raw-value")bad.repetition_fields.front().at(kTranslation).values_column_major[2]+=1.0;
        else if(fault=="wrong-frames"){++bad.valid_frames;--bad.undefined_frames;}
        else if(fault=="endpoint")bad.fields.at(kTranslation).values_column_major[0]=1.0;
        rejectFinalization(bad,true,"Finalization accepted invalid evidence: "+fault);
    }
    auto wrong_mode=prefix;
    rejectFinalization(wrong_mode,false,"Eight repetitions must not bypass their fixed denominator");
    auto incomplete=empty();
    require(accumulateCompleteAdaptiveRun(incomplete,repetition(ExperimentKind::AdaptiveComparison,1)).hasValue(),
            "Incomplete-prefix setup failed");
    rejectFinalization(incomplete,true,"Incomplete solver repetitions cannot be finalized");
    auto total=prefix;
    const auto finished=finishAdaptiveAggregation(std::move(total),true);
    require(finished.hasValue(),"Complete solves with consistent metric gaps must finalize");
    const auto& data=finished.value();
    require(data.completed_runs==8&&data.repetition_fields.size()==8&&data.undefined_frames==3&&
            data.valid_frames==8*(kFrames-2)-3&&data.undefined_metrics.size()==4,
            "Finalization lost original repetitions or distinct-frame accounting");
    require(data.valid_counts.at(kTranslation)[kGapFrame-1]==7&&
            std::isnan(data.fields.at(kTranslation).values_column_major[kGapFrame-1]),
            "Seven values cannot produce a smaller-denominator mean");
    require(data.fields.at(kTranslation).values_column_major[2]==4.5+2.0/1024.0,
            "An unaffected exact-binary mean must retain the original denominator eight");
    require(std::isnan(data.repetition_fields[1].at("rottransratio").values_column_major[22])&&
            data.fields.at("rottransratio").values_column_major[22]==8.0+4.0/16.0+22.0/1024.0&&
            data.valid_counts.at("rottransratio")[22]==1,"Earlier missing ratio must survive only in its raw repetition");
    require(std::isnan(data.fields.at("rottransratio").values_column_major[41])&&
            data.valid_counts.at("rottransratio")[41]==0,"Missing last ratio cannot fall back to an earlier run");
    require(experiments::makeAdaptiveOutput(ExperimentKind::AdaptiveComparison,data).hasValue(),
            "Finalization and publication must agree on the same evidence contract");
}
void singleRepetition(ExperimentKind kind) {
    auto total=empty(kind);auto one=repetition(kind,1);
    const auto columns=kind==ExperimentKind::AdaptiveUpdateRates?4U:1U;
    gap(one,kTranslation,1,5,columns);gap(one,kRotation,1,5);
    require(accumulateCompleteAdaptiveRun(total,one).hasValue(),"Single-run metric gaps were rejected");
    const auto finished=finishAdaptiveAggregation(std::move(total),false);
    require(finished.hasValue()&&finished.value().undefined_frames==1&&
            finished.value().valid_frames==one.frame_count-3,
            "Single-run finalization changed frame validity accounting");
    sameFields(finished.value().fields,one.fields);
    require(experiments::makeAdaptiveOutput(kind,finished.value()).hasValue(),
            "Initialization/UpdateRates must retain their own shapes and denominator one");
}
}  // namespace

int main() {
    try {
        invalidRepetition();invalidPrefix();finalization();
        singleRepetition(ExperimentKind::AdaptiveInitialization);
        singleRepetition(ExperimentKind::AdaptiveUpdateRates);
        std::cout<<"Adaptive aggregation evidence checks="<<checks<<" failures=0\n";
        return 0;
    }catch(const std::exception& error) {
        std::cerr<<"Adaptive aggregation evidence checks="<<checks<<" failure="<<error.what()<<'\n';
        return 1;
    }
}
