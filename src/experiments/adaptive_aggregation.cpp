#include "adaptive_aggregation.hpp"
#include "adaptive_evidence.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <new>
#include <stdexcept>
#include <utility>

namespace egomotion::experiments::detail {
namespace {
// agent_in_sphere_adaptive_matched_filter_test.m: anzahldurchlaeufe=8。
constexpr std::size_t kComparisonRuns=8;
constexpr std::size_t kInitializationFrames=35;
ExperimentKind aggregateKind(const AdaptiveRunData& data) {
    if(data.planned_runs==kComparisonRuns)return ExperimentKind::AdaptiveComparison;
    return data.frame_count==kInitializationFrames?ExperimentKind::AdaptiveInitialization:
                                                  ExperimentKind::AdaptiveUpdateRates;
}
Failure<ExperimentErrorCode> failure(ExperimentErrorCode code,const char* message) {
    ErrorDetail detail;detail.stage=ErrorStage::Simulation;detail.message=message;
    return {code,std::move(detail)};
}
}  // namespace

Status<ExperimentErrorCode> accumulateCompleteAdaptiveRun(
    AdaptiveRunData& aggregate,const AdaptiveRunData& one) {
    if(aggregate.planned_runs==0||aggregate.completed_runs>=aggregate.planned_runs)
        return Status<ExperimentErrorCode>::failure(failure(
            ExperimentErrorCode::DataMismatch,"Adaptive aggregate already has all planned runs"));
    if(aggregate.frame_count!=one.frame_count||aggregate.planned_runs!=one.planned_runs||
       aggregate.frame_count<3||aggregate.fields.empty()||
       aggregate.fields.size()!=one.fields.size()||
       aggregate.valid_counts.size()!=aggregate.fields.size()||
       one.valid_counts.size()!=one.fields.size()||
       one.valid_frames>one.frame_count-2||
       one.undefined_frames!=one.frame_count-2-one.valid_frames||
       aggregate.repetition_fields.size()!=aggregate.completed_runs)
        return Status<ExperimentErrorCode>::failure(failure(
            ExperimentErrorCode::DataMismatch,"Adaptive aggregate field/count mismatch"));
    try {
        // 旧前缀与追加后的候选都复用发布边界的证据门禁，避免坏前缀和坏轮次
        // 在相加后相互抵消。分配/验证失败不能提交任何计数、原值或缺测理由。
        const auto kind=aggregateKind(aggregate);
        auto prefix=validateAdaptiveEvidence(kind,aggregate,AdaptiveEvidencePhase::Accumulating);
        if(!prefix.hasValue())return prefix;
        for(const auto& item:aggregate.fields){
            const auto found=one.fields.find(item.first);
            const auto counts=one.valid_counts.find(item.first);
            const auto totals=aggregate.valid_counts.find(item.first);
            if(found==one.fields.end()||item.second.info.dimensions!=found->second.info.dimensions||
               item.second.info.dimensions.size()!=2||
               item.second.info.dimensions[0]!=aggregate.frame_count||
               item.second.values_column_major.size()!=found->second.values_column_major.size()||
               counts==one.valid_counts.end()||totals==aggregate.valid_counts.end()||
               counts->second.size()!=found->second.values_column_major.size()||
               totals->second.size()!=item.second.values_column_major.size())
                return Status<ExperimentErrorCode>::failure(failure(
                    ExperimentErrorCode::DataMismatch,"Adaptive aggregate shape or field mismatch"));
            const auto& target=item.second.values_column_major;
            const auto& values=found->second.values_column_major;
            for(std::size_t i=0;i<target.size();++i) {
                const bool endpoint=i%aggregate.frame_count==0||
                                    i%aggregate.frame_count+1==aggregate.frame_count;
                const auto count=counts->second[i];
                const bool source_valid=endpoint?(values[i]==0.0&&count==0):
                    ((count==1&&std::isfinite(values[i]))||(count==0&&std::isnan(values[i])));
                if(!source_valid||
                   (item.first!="rottransratio"&&std::isfinite(target[i])&&
                    std::isfinite(values[i])&&!std::isfinite(target[i]+values[i])))
                    return Status<ExperimentErrorCode>::failure(failure(
                        ExperimentErrorCode::DataMismatch,"Adaptive aggregate value/count contract mismatch"));
            }
        }
        for(const auto& missing:one.undefined_metrics)
            if(missing.matlab_run_index!=aggregate.completed_runs+1)
                return Status<ExperimentErrorCode>::failure(failure(
                    ExperimentErrorCode::DataMismatch,"Adaptive missing metric has wrong repetition index"));
        AdaptiveRunData next=aggregate;
        const bool final_run=next.completed_runs+1==next.planned_runs;
        for(auto& item:next.fields){
            auto& target=item.second.values_column_major;
            auto& totals=next.valid_counts.at(item.first);
            const auto& values=one.fields.at(item.first).values_column_major;
            const auto& counts=one.valid_counts.at(item.first);
            if(item.first=="rottransratio"){
                if(final_run){target=values;totals=counts;}
            }else{
                for(std::size_t i=0;i<target.size();++i){
                    totals[i]+=counts[i];
                    target[i]=std::isfinite(target[i])&&std::isfinite(values[i])?
                        target[i]+values[i]:std::numeric_limits<double>::quiet_NaN();
                }
            }
        }
        next.repetition_fields.push_back(one.fields);
        next.undefined_metrics.insert(next.undefined_metrics.end(),
            one.undefined_metrics.begin(),one.undefined_metrics.end());
        next.valid_frames+=one.valid_frames;
        next.undefined_frames+=one.undefined_frames;
        ++next.completed_runs;
        auto validated=validateAdaptiveEvidence(kind,next,AdaptiveEvidencePhase::Accumulating);
        if(!validated.hasValue())return validated;
        aggregate=std::move(next);
    } catch(const std::bad_alloc&) {
        return Status<ExperimentErrorCode>::failure(failure(
            ExperimentErrorCode::SolverFailed,"Cannot allocate complete Adaptive repetition evidence"));
    } catch(const std::length_error&) {
        return Status<ExperimentErrorCode>::failure(failure(
            ExperimentErrorCode::SolverFailed,"Adaptive repetition evidence exceeds container capacity"));
    }
    return Status<ExperimentErrorCode>::success();
}

Result<AdaptiveRunData,ExperimentErrorCode> finishAdaptiveAggregation(
    AdaptiveRunData&& aggregate,bool comparison) {
    using Output=Result<AdaptiveRunData,ExperimentErrorCode>;
    if(aggregate.planned_runs==0||aggregate.completed_runs!=aggregate.planned_runs||
       aggregate.repetition_fields.size()!=aggregate.planned_runs||
       (comparison?aggregate.planned_runs!=kComparisonRuns:aggregate.planned_runs!=1))
        return Output::failure(failure(ExperimentErrorCode::IncompleteRun,
            "Adaptive cannot finalize or divide incomplete planned runs"));
    try {
        // 在副本内最终化；证据/资源失败不能把调用者的累计和部分除以8。
        AdaptiveRunData next=aggregate;
        if(comparison){
            // 原四误差全部八轮有效才除以8；任一缺测保留NaN，绝不改除数。
            for(auto& item:next.fields){
                if(item.first=="rottransratio")continue;
                for(double& value:item.second.values_column_major)
                    value/=static_cast<double>(kComparisonRuns);
            }
        }
        auto validated=validateAdaptiveEvidence(
            aggregateKind(next),next,AdaptiveEvidencePhase::Finalized);
        if(!validated.hasValue())return Output::failure(std::move(validated).error());
        return Output::success(std::move(next));
    } catch(const std::bad_alloc&) {
        return Output::failure(failure(ExperimentErrorCode::SolverFailed,
            "Cannot allocate finalized Adaptive repetition evidence"));
    } catch(const std::length_error&) {
        return Output::failure(failure(ExperimentErrorCode::SolverFailed,
            "Finalized Adaptive repetition evidence exceeds container capacity"));
    }
}
}  // namespace egomotion::experiments::detail
