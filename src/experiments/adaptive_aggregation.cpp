#include "adaptive_aggregation.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace egomotion::experiments::detail {
namespace {
// agent_in_sphere_adaptive_matched_filter_test.m: anzahldurchlaeufe=8。
constexpr std::size_t kComparisonRuns=8;
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
       aggregate.fields.empty()||aggregate.fields.size()!=one.fields.size())
        return Status<ExperimentErrorCode>::failure(failure(
            ExperimentErrorCode::DataMismatch,"Adaptive aggregate field/count mismatch"));
    // 全量预检后才修改累加器，缺字段、转置或非有限值不提交半轮数据。
    for(const auto& item:aggregate.fields){
        const auto found=one.fields.find(item.first);
        if(found==one.fields.end()||item.second.info.dimensions!=found->second.info.dimensions||
           item.second.values_column_major.size()!=found->second.values_column_major.size())
            return Status<ExperimentErrorCode>::failure(failure(
                ExperimentErrorCode::DataMismatch,"Adaptive aggregate shape or field mismatch"));
        const auto& target=item.second.values_column_major;
        const auto& values=found->second.values_column_major;
        for(std::size_t i=0;i<target.size();++i)
            if(!std::isfinite(values[i])||!std::isfinite(target[i])||
               (item.first!="rottransratio"&&!std::isfinite(target[i]+values[i])))
                return Status<ExperimentErrorCode>::failure(failure(
                    ExperimentErrorCode::DataMismatch,"Adaptive aggregate value is non-finite"));
    }
    const bool final_run=aggregate.completed_runs+1==aggregate.planned_runs;
    for(auto& item:aggregate.fields){
        auto& target=item.second.values_column_major;
        const auto& values=one.fields.at(item.first).values_column_major;
        if(item.first=="rottransratio"){
            if(final_run)std::copy(values.begin(),values.end(),target.begin());
        }else{
            for(std::size_t i=0;i<target.size();++i)target[i]+=values[i];
        }
    }
    ++aggregate.completed_runs;
    return Status<ExperimentErrorCode>::success();
}

Result<AdaptiveRunData,ExperimentErrorCode> finishAdaptiveAggregation(
    AdaptiveRunData&& aggregate,bool comparison) {
    using Output=Result<AdaptiveRunData,ExperimentErrorCode>;
    if(aggregate.planned_runs==0||aggregate.completed_runs!=aggregate.planned_runs||
       (comparison&&aggregate.planned_runs!=kComparisonRuns))
        return Output::failure(failure(ExperimentErrorCode::IncompleteRun,
            "Adaptive cannot finalize or divide incomplete planned runs"));
    if(comparison){
        // 原四误差先逐轮累计，全部八轮成功后统一除以8；比值仅来自第八轮。
        for(auto& item:aggregate.fields){
            if(item.first=="rottransratio")continue;
            for(double& value:item.second.values_column_major)
                value/=static_cast<double>(kComparisonRuns);
        }
    }
    return Output::success(std::move(aggregate));
}
}  // namespace egomotion::experiments::detail
