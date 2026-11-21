// DEC-R6-03 独立合同：真实 Session/系数策略，Step 仅为可控事务替身。
// 不依赖 Runner、轨迹、Sophus 或 MAT；不裁剪既有完整 Session 测试。
#include "../support/coefficient_fixture.hpp"
#include "../support/flow_fixture.hpp"
#include "../support/motion_fixture.hpp"
#include "egomotion/estimators/adaptive_session.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
using namespace egomotion;
using namespace egomotion::estimators;
using FrameResult=Result<AdaptiveFrameResult,ExperimentErrorCode>;
constexpr double kAngleMargin=1e-9; // 已有指标合同回归余量，非 DEC-005 科学验收阈值。
constexpr std::size_t kFailureFrame=5;
std::size_t checks=0,successful_frames=0,solver_failures=0,policy_failures=0,invalid_successes=0;
void require(bool condition,const char* message) {
    ++checks;
    if(!condition)throw std::runtime_error(message);
}
std::size_t comparisonCount(AdaptiveSessionMode mode) {
    return mode==AdaptiveSessionMode::UpdateRates?4:2;
}
struct State {
    HarmonicMatrix primary;
    std::array<HarmonicMatrix,4> baselines;
};
State state(const AdaptiveMfaSession& session) {
    State result;result.primary=session.primaryCoefficients().values();
    for(std::size_t slot=0;slot<result.baselines.size();++slot)
        result.baselines[slot]=session.updateRateBaselines()[slot].values();
    return result;
}
void requireUnchanged(const AdaptiveMfaSession& session,const State& before) {
    const auto identical=[](const HarmonicMatrix& a,const HarmonicMatrix& b) {
        return std::memcmp(a.data(),b.data(),static_cast<std::size_t>(a.size())*sizeof(double))==0;
    };
    require(identical(session.primaryCoefficients().values(),before.primary),
            "Rejected frame changed the primary coefficient bytes");
    for(std::size_t slot=0;slot<before.baselines.size();++slot)
        require(identical(session.updateRateBaselines()[slot].values(),before.baselines[slot]),
                "Rejected frame partially committed a periodic baseline");
}
AdaptiveFrameInput frame(std::size_t matlab_index,bool zero_translation,bool rotating=false) {
    AdaptiveFrameInput input;input.matlab_frame_index=matlab_index;
    Vector3 translation=Vector3::UnitX(),rotation=Vector3::Zero();
    if(zero_translation)translation.setZero();
    if(rotating)rotation=Vector3::UnitY();
    input.truth=test_support::motionValue(translation,rotation);
    FlowMatrix observations=FlowMatrix::Zero(6,6);
    for(Eigen::Index row=0;row<6;++row) {
        observations(row,row%3)=row<3?1.0:-1.0;
        observations(row,3+(row+1)%3)=0.25;
    }
    input.observations=test_support::flowValue(observations);
    return input;
}
enum class StepCorruption { Mse,TraceMse,System,Nearness };
const char* corruptedField(StepCorruption corruption) {
    switch(corruption) {
        case StepCorruption::Mse:return "error_mse";
        case StepCorruption::TraceMse:return "trace.error_mse";
        case StepCorruption::System:return "trace.system_matrix";
        case StepCorruption::Nearness:return "trace.recomputed_near";
    }
    throw std::logic_error("Unknown synthetic Step corruption");
}
struct ScriptedStep final:IAdaptiveStepSolver {
    const std::size_t comparisons;
    mutable std::size_t calls=0,attempt_calls=0;
    std::size_t fail_at=0;
    std::size_t invalid_at=0;
    StepCorruption corruption=StepCorruption::Mse;
    bool zero_translation=false;
    explicit ScriptedStep(AdaptiveSessionMode mode):comparisons(comparisonCount(mode)) {}
    void beginAttempt(std::size_t failure_call=0) {
        attempt_calls=0;fail_at=failure_call;
    }
    Result<AdaptiveStepResult,NumericalErrorCode> solve(
        const HarmonicCoefficients& old,const FlowField& observations) const override {
        ++calls;++attempt_calls;
        require(attempt_calls<=comparisons+1&&observations.observations().rows()==6,
                "Session changed the expected comparison/primary calls or input observations");
        if(attempt_calls==fail_at) {
            ErrorDetail detail;detail.stage=ErrorStage::Solve;
            detail.message="Injected Step failure after earlier missing direction metrics";
            detail.observation_index=attempt_calls-1;
            return Result<AdaptiveStepResult,NumericalErrorCode>::failure(
                {NumericalErrorCode::SingularSystem,std::move(detail)});
        }
        AdaptiveStepResult output;
        Vector3 translation=Vector3::UnitX();
        if(zero_translation)translation.setZero();
        output.motion=test_support::motionValue(translation,Vector3::Zero());
        auto next=old.values();
        // 比较 dumi 与主提交明显可区分；主求解只使零阶系数增加一。
        next(0,0)+=attempt_calls<=comparisons?1000.0+static_cast<double>(attempt_calls):1.0;
        output.coefficients_next=test_support::coefficientValue(next);
        output.trace.coefficients_next=output.coefficients_next;
        if(attempt_calls==invalid_at) {
            switch(corruption) {
                case StepCorruption::Mse:
                    output.error_mse=std::numeric_limits<double>::infinity();break;
                case StepCorruption::TraceMse:
                    output.trace.error_mse=std::numeric_limits<double>::quiet_NaN();break;
                case StepCorruption::System:
                    output.trace.system_matrix(4,2)=-std::numeric_limits<double>::infinity();break;
                case StepCorruption::Nearness:
                    output.trace.recomputed_near=Eigen::VectorXd::Zero(observations.observations().rows());
                    output.trace.recomputed_near[3]=std::numeric_limits<double>::quiet_NaN();break;
            }
        }
        return Result<AdaptiveStepResult,NumericalErrorCode>::success(std::move(output));
    }
};
void requireClockRejected(AdaptiveMfaSession& session,ScriptedStep& solver,
                          const AdaptiveFrameInput& input) {
    const auto before=state(session);const auto calls=solver.calls;
    const auto rejected=session.processFrame(input);
    require(!rejected.hasValue()&&rejected.error().code==ExperimentErrorCode::InvalidParameters,
            "Source clock accepted a repeated or skipped frame");
    require(solver.calls==calls,"Clock rejection reached the Step solver");
    requireUnchanged(session,before);
}
void requireCommitted(AdaptiveSessionMode mode,const AdaptiveMfaSession& session,
                      const State& before,std::size_t matlab_frame) {
    auto expected=before.primary;expected(0,0)+=1.0;
    require((session.primaryCoefficients().values().array()==expected.array()).all(),
            "Successful frame must commit exactly its primary solve, never a comparison dumi");
    for(std::size_t slot=0;slot<before.baselines.size();++slot) {
        const bool refresh=mode==AdaptiveSessionMode::UpdateRates&&
                           matlab_frame%kAdaptiveUpdatePeriods[slot]==0;
        const auto& wanted=refresh?before.primary:before.baselines[slot];
        require((session.updateRateBaselines()[slot].values().array()==wanted.array()).all(),
                "Successful metric-gap frame lost or replaced its source periodic baseline");
    }
}
void requireMetrics(const FrameResult& result,std::size_t comparisons,std::size_t matlab_frame,
                    bool missing_translation,double rotation_degrees) {
    require(result.hasValue(),"Missing metrics must not turn a successful frame solve into failure");
    const auto& value=result.value();
    require(value.matlab_frame_index==matlab_frame&&value.comparison_motion.size()==comparisons&&
            value.comparison_translation_error_deg.size()==comparisons&&
            value.comparison_rotation_error_deg.size()==comparisons,
            "Every source comparison slot and MATLAB frame identity must be retained");
    for(std::size_t slot=0;slot<comparisons;++slot) {
        const auto& translation=value.comparison_translation_error_deg[slot];
        const auto& rotation=value.comparison_rotation_error_deg[slot];
        if(missing_translation) {
            require(!translation.hasValue(),"A zero translation direction was fabricated as a finite angle");
            require(translation.error().code==NumericalErrorCode::UndefinedAngle&&
                    translation.error().detail.stage==ErrorStage::Solve&&
                    !translation.error().detail.message.empty(),
                    "A missing direction must retain its numerical code, stage and reason");
        } else {
            require(translation.hasValue()&&translation.value()==0.0,
                    "A later identical nonzero direction must recover a valid zero angle");
        }
        require(rotation.hasValue()&&std::isfinite(rotation.value())&&
                std::abs(rotation.value()-rotation_degrees)<=kAngleMargin,
                "Zero estimated rotation must have its valid SO3 geodesic error");
    }
}
void successfulFrame(AdaptiveSessionMode mode,AdaptiveMfaSession& session,ScriptedStep& solver,
                     const AdaptiveFrameInput& input,bool missing_translation,double rotation_degrees) {
    const auto before=state(session);const auto previous_calls=solver.calls;
    solver.beginAttempt();
    const auto result=session.processFrame(input);
    requireMetrics(result,solver.comparisons,input.matlab_frame_index,missing_translation,rotation_degrees);
    require(solver.calls-previous_calls==solver.comparisons+1,
            "Metric failure skipped a comparison or the primary solve");
    requireCommitted(mode,session,before,input.matlab_frame_index);
    requireClockRejected(session,solver,input);
    auto skipped=input;skipped.matlab_frame_index+=2;
    requireClockRejected(session,solver,skipped);
    ++successful_frames;
}
void metricValidity(AdaptiveSessionMode mode) {
    ScriptedStep solver(mode);AdaptiveMfaSession session(mode,&solver);
    successfulFrame(mode,session,solver,frame(2,true),true,0.0); // 零真平移。
    solver.zero_translation=true;
    successfulFrame(mode,session,solver,frame(3,false,true),true,180.0/std::acos(-1.0)); // 零估计平移。
    successfulFrame(mode,session,solver,frame(4,true),true,0.0); // 两个方向均为零。
    solver.zero_translation=false;
    successfulFrame(mode,session,solver,frame(5,false),false,0.0); // 下一帧恢复有效。
}
void requireNumericalCause(const FrameResult& failed,NumericalErrorCode code,
                           std::optional<std::size_t> index,const std::string& message) {
    require(!failed.hasValue()&&failed.error().code==ExperimentErrorCode::SolverFailed,
            "A true comparison, primary or strategy failure must remain a failed frame");
    require(failed.error().detail.message.find("matlab_frame_index=5")!=std::string::npos,
            "Fatal failure lost its original MATLAB frame identity");
    bool found=false;
    for(const auto& cause:failed.error().detail.causes)
        found=found||(cause.domain=="NumericalErrorCode"&&cause.code==std::string(errorName(code))&&
            cause.stage==ErrorStage::Solve&&cause.observation_index==index&&cause.message==message);
    require(found,"True failure lost its numerical cause or source slot");
}
void seedState(AdaptiveSessionMode mode,AdaptiveMfaSession& session,ScriptedStep& solver) {
    for(std::size_t index=2;index<kFailureFrame;++index)
        successfulFrame(mode,session,solver,frame(index,false),false,0.0);
}
void solveFailureRollback(AdaptiveSessionMode mode,bool primary_failure) {
    ScriptedStep solver(mode);AdaptiveMfaSession session(mode,&solver);
    seedState(mode,session,solver);
    const auto input=frame(kFailureFrame,true);
    const auto before=state(session);
    const auto previous_calls=solver.calls;
    const auto failed_call=primary_failure?solver.comparisons+1:2;
    solver.beginAttempt(failed_call);
    const auto failed=session.processFrame(input);
    requireNumericalCause(failed,NumericalErrorCode::SingularSystem,failed_call-1,
                          "Injected Step failure after earlier missing direction metrics");
    require(solver.calls-previous_calls==failed_call,
            "Missing direction stopped solving before the injected true failure");
    requireUnchanged(session,before);
    requireClockRejected(session,solver,frame(kFailureFrame+1,true));
    successfulFrame(mode,session,solver,input,true,0.0); // 同帧重试完整成功才提交。
    ++solver_failures;
}
void invalidSuccessRollback(AdaptiveSessionMode mode,bool primary_failure,StepCorruption corruption) {
    ScriptedStep solver(mode);AdaptiveMfaSession session(mode,&solver);
    seedState(mode,session,solver);
    const auto input=frame(kFailureFrame,true);
    const auto before=state(session);
    const auto previous_calls=solver.calls;
    solver.beginAttempt();
    solver.invalid_at=primary_failure?solver.comparisons+1:2;
    solver.corruption=corruption;
    const auto failed=session.processFrame(input);
    requireNumericalCause(failed,NumericalErrorCode::NonFiniteResult,std::nullopt,
        std::string("Adaptive Step returned non-finite ")+corruptedField(corruption));
    require(failed.error().detail.message.find(primary_failure?"primary":"baseline=1")!=std::string::npos,
            "Invalid successful payload lost its primary/comparison identity");
    require(solver.calls-previous_calls==solver.invalid_at,
            "A non-finite successful Step must stop later comparisons or the primary solve");
    requireUnchanged(session,before);
    requireClockRejected(session,solver,frame(kFailureFrame+1,true));
    solver.invalid_at=0;
    successfulFrame(mode,session,solver,input,true,0.0);
    ++invalid_successes;
}
struct ObservedPolicy final:ICoefficientUpdatePolicy {
    const ICoefficientUpdatePolicy* delegate=nullptr;
    std::size_t slot=0;
    bool fail=false;
    mutable std::size_t calls=0;
    Result<HarmonicCoefficients,NumericalErrorCode> prepare(
        std::size_t index,const HarmonicCoefficients& primary,
        const HarmonicCoefficients& previous,const Matrix3& rotation) const override {
        ++calls;
        const auto candidate=delegate->prepare(index,primary,previous,rotation);
        if(!candidate.hasValue())return candidate;
        if(fail) {
            ErrorDetail detail;detail.stage=ErrorStage::Solve;
            detail.message="Injected strategy preparation failure";detail.observation_index=slot;
            return Result<HarmonicCoefficients,NumericalErrorCode>::failure(
                {NumericalErrorCode::InvalidShape,std::move(detail)});
        }
        return candidate;
    }
};
void strategyFailureRollback(AdaptiveSessionMode mode) {
    ResetCoefficientUpdatePolicy reset;InheritCoefficientUpdatePolicy inherit;
    std::array<PeriodicCoefficientUpdatePolicy,4> periodic{{
        PeriodicCoefficientUpdatePolicy{1},PeriodicCoefficientUpdatePolicy{5},
        PeriodicCoefficientUpdatePolicy{11},PeriodicCoefficientUpdatePolicy{21}}};
    std::array<ObservedPolicy,4> observed;
    AdaptiveCoefficientPolicies policies{};
    const auto comparisons=comparisonCount(mode);
    for(std::size_t slot=0;slot<comparisons;++slot) {
        observed[slot].slot=slot;
        if(mode==AdaptiveSessionMode::UpdateRates)observed[slot].delegate=&periodic[slot];
        else if(slot==0)observed[slot].delegate=&reset;
        else observed[slot].delegate=&inherit;
        policies[slot]=&observed[slot];
    }
    ScriptedStep solver(mode);AdaptiveMfaSession session(mode,policies,&solver);
    seedState(mode,session,solver);
    const auto input=frame(kFailureFrame,true);
    const auto before=state(session);
    const auto previous_calls=solver.calls;
    std::array<std::size_t,4> policy_calls{};
    for(std::size_t slot=0;slot<comparisons;++slot)policy_calls[slot]=observed[slot].calls;
    observed[comparisons-1].fail=true;solver.beginAttempt();
    const auto failed=session.processFrame(input);
    requireNumericalCause(failed,NumericalErrorCode::InvalidShape,comparisons-1,
                          "Injected strategy preparation failure");
    require(solver.calls==previous_calls,"Strategy failure must stop before all Step calls");
    requireUnchanged(session,before);
    requireClockRejected(session,solver,frame(kFailureFrame+1,true));
    for(std::size_t slot=0;slot<comparisons;++slot)
        require(observed[slot].calls==policy_calls[slot]+1,
                "Strategy preparation or clock rejection changed the source call order");
    observed[comparisons-1].fail=false;
    successfulFrame(mode,session,solver,input,true,0.0);
    ++policy_failures;
}
void paperAndArchiveSchedules() {
    constexpr std::array<std::size_t,4> article{{1,5,10,20}};
    for(const auto& periods:{kAdaptiveUpdatePeriods,article}) {
        ScriptedStep solver(AdaptiveSessionMode::UpdateRates);
        AdaptiveMfaSession session(AdaptiveSessionMode::UpdateRates,&solver,periods);
        for(std::size_t i=2;i<=22;++i) {
            solver.beginAttempt();
            const auto before=state(session);
            auto result=session.processFrame(frame(i,false));
            require(result.hasValue(),"Paper schedule frame unexpectedly failed");
            for(std::size_t j=0;j<periods.size();++j) {
                const bool refreshed=i%periods[j]==0;
                const auto& expected=refreshed?before.primary:before.baselines[j];
                require(result.value().periodic_refreshed[j]==refreshed &&
                        (session.updateRateBaselines()[j].values().array()==expected.array()).all(),
                        "Recorded refresh did not match the committed frame/period");
            }
        }
    }
}
} // namespace

int main() {
    try {
        paperAndArchiveSchedules();
        for(const auto mode:{AdaptiveSessionMode::Initialization,
                             AdaptiveSessionMode::Comparison,AdaptiveSessionMode::UpdateRates}) {
            metricValidity(mode);
            solveFailureRollback(mode,false);
            solveFailureRollback(mode,true);
            strategyFailureRollback(mode);
            for(const auto corruption:{StepCorruption::Mse,StepCorruption::TraceMse,
                                       StepCorruption::System,StepCorruption::Nearness}) {
                invalidSuccessRollback(mode,false,corruption);
                invalidSuccessRollback(mode,true,corruption);
            }
        }
        require(solver_failures==6&&policy_failures==3,"All three modes must exercise each fatal-failure path");
        require(invalid_successes==24,"Every mode must reject each non-finite comparison and primary payload");
        std::cout<<"Session metric-validity checks="<<checks<<" successful_frames="<<successful_frames
                 <<" solver_rollback_cases="<<solver_failures<<" strategy_rollback_cases="<<policy_failures
                 <<" invalid_success_rollback_cases="<<invalid_successes
                 <<" failures=0\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<"Session metric-validity checks="<<checks<<" failure="<<error.what()<<'\n';
        return 1;
    }
}
