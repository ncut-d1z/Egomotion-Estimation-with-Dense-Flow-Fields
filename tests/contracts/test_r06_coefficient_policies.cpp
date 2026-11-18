#include "../support/coefficient_fixture.hpp"
#include "../support/motion_fixture.hpp"
// 替身只验证源状态编排；数学回归另由独立SH/单步L3和真实Session测试承担。
#include "egomotion/estimators/adaptive_session.hpp"
#include "egomotion/harmonics/adapter.hpp"
#include "egomotion/contracts/diagnostics.hpp"

#include <array>
#include <cstring>
#include <cmath>
#include <iostream>
#include <limits>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
using namespace egomotion;
using namespace egomotion::estimators;
std::size_t checks=0;
void require(bool value,const char* message) {
    ++checks;if(!value)throw std::runtime_error(message);
}
template<class A,class B>
void same(const Eigen::MatrixBase<A>& a,const Eigen::MatrixBase<B>& b,const char* message) {
    require(a.rows()==b.rows()&&a.cols()==b.cols(),"Shape");
    for(Eigen::Index i=0;i<a.rows();++i)for(Eigen::Index j=0;j<a.cols();++j){
        const double x=a(i,j),y=b(i,j);require(std::memcmp(&x,&y,sizeof(x))==0,message);
    }
}
HarmonicCoefficients coefficients(double constant) {
    HarmonicMatrix result;result.setZero();result(0,0)=constant;
    result.row(1).head<3>()<<0.125,0.25,-0.5;
    result.row(2)<<0.0625,-0.125,0.25,0.5,-1;
    return test_support::coefficientValue(result);
}
AdaptiveFrameInput frame(std::size_t index) {
    AdaptiveFrameInput result;result.matlab_frame_index=index;
    result.source_coefficient_rotation<<0,-1,0,1,0,0,0,0,1;
    result.truth=test_support::motionValue(Vector3::UnitX(),Vector3::UnitY());
    return result; // 空流场仅交给确定性替身，不声称是真实数值输入。
}
struct Policies {
    ResetCoefficientUpdatePolicy reset;
    InheritCoefficientUpdatePolicy inherit;
    std::array<PeriodicCoefficientUpdatePolicy,4> periodic{{
        PeriodicCoefficientUpdatePolicy{1},PeriodicCoefficientUpdatePolicy{5},
        PeriodicCoefficientUpdatePolicy{11},PeriodicCoefficientUpdatePolicy{21}}};
    AdaptiveCoefficientPolicies forMode(AdaptiveSessionMode mode) const {
        if(mode==AdaptiveSessionMode::UpdateRates)
            return {{&periodic[0],&periodic[1],&periodic[2],&periodic[3]}};
        return {{&reset,&inherit,nullptr,nullptr}};
    }
};
void directPolicies() {
    Policies policies;const auto primary=coefficients(101),baseline=coefficients(7);
    const auto rotation=frame(2).source_coefficient_rotation;
    const auto rotated=harmonics::rotateSourceCoefficients(baseline,rotation);
    require(rotated.hasValue(),"Existing source action provides routing reference");
    for(std::size_t i=2;i<=43;++i){
        const auto reset=policies.reset.prepare(i,primary,baseline,rotation);
        const auto inherit=policies.inherit.prepare(i,primary,baseline,rotation);
        require(reset.hasValue()&&inherit.hasValue(),"Reset/inherit succeed");
        HarmonicMatrix constant=HarmonicMatrix::Zero();constant(0,0)=1;
        same(reset.value().values(),constant,"Exact constant reset");
        same(inherit.value().values(),primary.values(),"Inherit without rotating again");
        for(std::size_t j=0;j<4;++j){
            auto result=policies.periodic[j].prepare(i,primary,baseline,rotation);
            require(result.hasValue(),"Periodic prepare succeeds");
            same(result.value().values(),i%kAdaptiveUpdatePeriods[j]==0?primary.values():rotated.value().values(),
                 "MATLAB modulo selects pre-solve primary or rotated baseline");
            auto owned=std::move(result).value();
            owned=HarmonicCoefficients{}; // 替换整个自拥有值，不修改输入或Result载荷。
        }
    }
    same(primary.values(),coefficients(101).values(),"Primary input remains unchanged");
    same(baseline.values(),coefficients(7).values(),"Baseline input remains unchanged");
    for(const auto* policy:policies.forMode(AdaptiveSessionMode::UpdateRates))
        require(policy->prepare(1,primary,baseline,rotation).error().code==NumericalErrorCode::InvalidShape,"Reject non-source frame");
    require(!policies.reset.prepare(0,primary,baseline,rotation).hasValue(),"Reset frame validation");
    require(!policies.inherit.prepare(1,primary,baseline,rotation).hasValue(),"Inherit frame validation");
    PeriodicCoefficientUpdatePolicy zero(0);
    require(zero.prepare(2,primary,baseline,rotation).error().code==NumericalErrorCode::InvalidShape,"Reject zero before modulo");
    HarmonicMatrix large=HarmonicMatrix::Zero();
    large(1,0)=std::numeric_limits<double>::max();large(1,2)=std::numeric_limits<double>::max();
    const auto overflowing=test_support::coefficientValue(large);
    const double diagonal=std::sqrt(0.5);
    Matrix3 mixed;mixed<<diagonal,-diagonal,0,diagonal,diagonal,0,0,0,1;
    const auto invalid_rotation=policies.periodic[1].prepare(5,primary,overflowing,mixed);
    require(!invalid_rotation.hasValue()&&invalid_rotation.error().code==NumericalErrorCode::NonFiniteResult,
            "Refresh cannot skip finite baseline rotation overflow");
    const Matrix3 reflection=-Matrix3::Identity();
    require(!policies.periodic[1].prepare(5,primary,baseline,reflection).hasValue(),"Refresh cannot skip improper rotation");
    auto invalid=primary.values();invalid(0,0)=std::numeric_limits<double>::infinity();
    const auto invalid_primary=HarmonicCoefficients::create(invalid);
    require(!invalid_primary.hasValue()&&invalid_primary.error().code==NumericalErrorCode::NonFiniteInput,
            "Reject non-finite inherited primary at construction");
}
struct Call {std::size_t frame;HarmonicMatrix primary,baseline;Matrix3 rotation;};
enum class Fault { None, Failure, NonFinite, Allocation, Length };
struct ObservedPolicy final:ICoefficientUpdatePolicy {
    const ICoefficientUpdatePolicy& delegate;
    std::size_t slot;
    std::vector<std::string>& events;
    mutable std::vector<Call> calls;
    Fault fault=Fault::None;
    double offset=0;
    ObservedPolicy(const ICoefficientUpdatePolicy& value,std::size_t index,std::vector<std::string>& log)
        :delegate(value),slot(index),events(log) {}
    Result<HarmonicCoefficients,NumericalErrorCode> prepare(
        std::size_t i,const HarmonicCoefficients& main,const HarmonicCoefficients& baseline,
        const Matrix3& rotation) const override {
        events.push_back("P"+std::to_string(slot));calls.push_back({i,main.values(),baseline.values(),rotation});
        if(fault==Fault::Allocation)throw std::bad_alloc{};
        if(fault==Fault::Length)throw std::length_error("policy fixture");
        if(fault==Fault::Failure){
            ErrorDetail inner;inner.stage=ErrorStage::Read;inner.message="inner";
            inner.source_path="策略 输入.mat";inner.variable="coeff";inner.observation_index=9;inner.run_id=RunId{"source_policy"};
            ErrorDetail outer;outer.stage=ErrorStage::Solve;outer.message="policy failure";
            return Result<HarmonicCoefficients,NumericalErrorCode>::failure(wrapFailure(
                NumericalErrorCode::InvalidNearness,std::move(outer),Failure<MatErrorCode>{MatErrorCode::InvalidShape,std::move(inner)}));
        }
        auto result=delegate.prepare(i,main,baseline,rotation);if(!result.hasValue())return result;
        auto candidate=result.value().values();
        if(fault==Fault::NonFinite)candidate(0,0)=std::numeric_limits<double>::quiet_NaN();
        candidate(0,0)+=offset;
        auto owned=HarmonicCoefficients::create(candidate);
        if(!owned.hasValue()){
            ErrorDetail detail;detail.stage=ErrorStage::Solve;
            detail.message="Coefficient update policy produced non-finite candidate";
            return Result<HarmonicCoefficients,NumericalErrorCode>::failure(wrapFailure(
                NumericalErrorCode::NonFiniteResult,std::move(detail),owned.error()));
        }
        return owned;
    }
};
struct Solver final:IAdaptiveStepSolver {
    std::vector<std::string>& events;
    mutable std::vector<HarmonicMatrix> received;
    std::size_t fail_at=0;
    explicit Solver(std::vector<std::string>& log):events(log) {}
    Result<AdaptiveStepResult,NumericalErrorCode> solve(const HarmonicCoefficients& old,const FlowField&) const override {
        events.push_back("S");received.push_back(old.values());
        if(fail_at==received.size()){
            ErrorDetail detail;detail.stage=ErrorStage::Solve;detail.message="injected solver";
            return Result<AdaptiveStepResult,NumericalErrorCode>::failure({NumericalErrorCode::SingularSystem,std::move(detail)});
        }
        AdaptiveStepResult result;result.motion=test_support::motionValue(Vector3::UnitX(),Vector3::UnitY());
        result.coefficients_next=test_support::coefficientValue(old.values()+coefficients(100).values());result.error_mse=0;
        result.trace.coefficients_next=result.coefficients_next;
        return Result<AdaptiveStepResult,NumericalErrorCode>::success(std::move(result));
    }
};
struct ObservedPolicies {
    std::array<ObservedPolicy,4> values;
    ObservedPolicies(const Policies& policies,AdaptiveSessionMode mode,std::vector<std::string>& log)
      :values{{{mode==AdaptiveSessionMode::UpdateRates?static_cast<const ICoefficientUpdatePolicy&>(policies.periodic[0]):policies.reset,0,log},
               {mode==AdaptiveSessionMode::UpdateRates?static_cast<const ICoefficientUpdatePolicy&>(policies.periodic[1]):policies.inherit,1,log},
               {policies.periodic[2],2,log},{policies.periodic[3],3,log}}} {}
    AdaptiveCoefficientPolicies pointers() const {return {{&values[0],&values[1],&values[2],&values[3]}};}
};
void stateEquals(const AdaptiveMfaSession& a,const AdaptiveMfaSession& b) {
    same(a.primaryCoefficients().values(),b.primaryCoefficients().values(),"Primary state equality");
    for(std::size_t j=0;j<4;++j)same(a.updateRateBaselines()[j].values(),b.updateRateBaselines()[j].values(),"Baseline state equality");
}
std::size_t count(AdaptiveSessionMode mode){return mode==AdaptiveSessionMode::UpdateRates?4:2;}
void composition(AdaptiveSessionMode mode) {
    Policies policies;std::vector<std::string> events,reference_events;
    ObservedPolicies observed(policies,mode,events);auto pointers=observed.pointers();
    Solver solver(events),reference_solver(reference_events);
    AdaptiveMfaSession session(mode,pointers,&solver),reference(mode,&reference_solver);
    pointers.fill(nullptr); // Session拥有指针数组副本。
    for(std::size_t i=2;i<=43;++i){
        const auto old_primary=session.primaryCoefficients();const auto old_baselines=session.updateRateBaselines();
        events.clear();const auto input=frame(i);const auto output=session.processFrame(input),expected=reference.processFrame(input);
        require(output.hasValue()&&expected.hasValue(),"Both assemblies succeed");stateEquals(session,reference);
        same(output.value().primary_step.coefficients_next.values(),expected.value().primary_step.coefficients_next.values(),"Primary result unchanged");
        same(output.value().primary_step.trace.coefficients_next.values(),output.value().primary_step.coefficients_next.values(),"Synthetic trace is fully initialized and consistent");
        require(output.value().comparison_motion.size()==count(mode),"Comparison count unchanged");
        require(events.size()==count(mode)*2+1,"All policies then comparisons then primary");
        auto primary=mode==AdaptiveSessionMode::Initialization
            ?Result<HarmonicCoefficients,NumericalErrorCode>::success(old_primary)
            :harmonics::rotateSourceCoefficients(old_primary,input.source_coefficient_rotation);
        require(primary.hasValue(),"Reference main source action");
        for(std::size_t j=0;j<count(mode);++j){
            require(events[j]=="P"+std::to_string(j)&&events[count(mode)+j]=="S","Policy order precedes solver");
            const auto& call=observed.values[j].calls.back();require(call.frame==i,"MATLAB index unchanged");
            same(call.primary,primary.value().values(),"Policy sees rotated pre-solve main");
            same(call.baseline,old_baselines[j].values(),"Policy sees previous persistent baseline");
            same(call.rotation,input.source_coefficient_rotation,"Source rotation passed exactly");
        }
        require(events.back()=="S","Main solver last");
    }
    require(solver.received.size()==reference_solver.received.size(),"Same solve count");
    for(std::size_t i=0;i<solver.received.size();++i)same(solver.received[i],reference_solver.received[i],"All solver inputs unchanged");
}
void failures(AdaptiveSessionMode mode) {
    Policies policies;
    for(std::size_t slot=0;slot<count(mode);++slot)for(auto fault:{Fault::Failure,Fault::NonFinite,Fault::Allocation,Fault::Length}){
        std::vector<std::string> events,ref_events;Solver solver(events),ref_solver(ref_events);
        ObservedPolicies observed(policies,mode,events);
        AdaptiveMfaSession session(mode,observed.pointers(),&solver),reference(mode,&ref_solver);
        for(std::size_t i=2;i<5;++i){require(session.processFrame(frame(i)).hasValue(),"Prime session");require(reference.processFrame(frame(i)).hasValue(),"Prime reference");}
        const auto calls=solver.received.size();events.clear();observed.values[slot].fault=fault;
        const auto failed=session.processFrame(frame(5));require(!failed.hasValue(),"Policy failure aborts frame");
        require(solver.received.size()==calls&&events.size()==slot+1,"No solve after failed preparation");stateEquals(session,reference);
        const auto json=jsonFailure(failed.error(),5);
        if(fault==Fault::Failure)require(json.find("策略 输入.mat")!=std::string::npos&&json.find("source_policy")!=std::string::npos&&
            json.find("\"observation_index\":9")!=std::string::npos&&json.find("\"variable\":\"coeff\"")!=std::string::npos,"Full policy failure chain");
        else require(json.find(fault==Fault::NonFinite?"NonFiniteResult":"ResourceLimitExceeded")!=std::string::npos,"Policy failure category");
        observed.values[slot].fault=Fault::None;
        require(session.processFrame(frame(5)).hasValue()&&reference.processFrame(frame(5)).hasValue(),"Exact frame retry");stateEquals(session,reference);
    }
    for(std::size_t failing=1;failing<=count(mode)+1;++failing){
        std::vector<std::string> events,ref_events;Solver solver(events),ref_solver(ref_events);
        ObservedPolicies observed(policies,mode,events);
        AdaptiveMfaSession session(mode,observed.pointers(),&solver),reference(mode,&ref_solver);
        solver.fail_at=failing;
        require(!session.processFrame(frame(2)).hasValue(),"Each comparison/main failure aborts");stateEquals(session,reference);
        solver.fail_at=0;
        require(session.processFrame(frame(2)).hasValue()&&reference.processFrame(frame(2)).hasValue(),"Prepared candidates not committed on solve failure");stateEquals(session,reference);
    }
}
void replacementAndIsolation() {
    Policies policies;std::vector<std::string> events;Solver solver(events),peer_solver(events);
    ObservedPolicies observed(policies,AdaptiveSessionMode::UpdateRates,events);
    observed.values[1].offset=16;
    AdaptiveMfaSession session(AdaptiveSessionMode::UpdateRates,observed.pointers(),&solver),peer(AdaptiveSessionMode::UpdateRates,&peer_solver);
    require(session.processFrame(frame(2)).hasValue(),"Replacement policy used");
    require(solver.received[1](0,0)==17&&session.updateRateBaselines()[1].values()(0,0)==17,"Candidate used and baseline committed, comparison dumi discarded");
    require(session.primaryCoefficients().values()(0,0)==101&&solver.received.back()(0,0)==1,"Policy cannot replace primary");
    require(peer.primaryCoefficients().values()(0,0)==1,"Peer remains untouched");
    require(peer.processFrame(frame(2)).hasValue()&&peer.updateRateBaselines()[1].values()(0,0)==1,"Default peer retains own baseline");
    AdaptiveMfaSession shared(AdaptiveSessionMode::UpdateRates,observed.pointers(),&peer_solver);
    require(shared.processFrame(frame(2)).hasValue(),"Policies may be shared without sharing Session state");stateEquals(session,shared);
    for(auto mode:{AdaptiveSessionMode::Initialization,AdaptiveSessionMode::Comparison,AdaptiveSessionMode::UpdateRates}){
        auto missing=policies.forMode(mode);missing[count(mode)-1]=nullptr;
        AdaptiveMfaSession invalid(mode,missing,&solver);const auto previous=solver.received.size();
        require(invalid.processFrame(frame(2)).error().code==ExperimentErrorCode::InvalidParameters,"Missing policy rejected");
        require(solver.received.size()==previous,"Missing policy has no solver effect");
        ObservedPolicies watch(policies,mode,events);AdaptiveMfaSession ordered(mode,watch.pointers(),&solver);
        require(!ordered.processFrame(frame(3)).hasValue()&&watch.values[0].calls.empty(),"Invalid frame rejected before policy");
    }
}
} // namespace
int main() {
    directPolicies();
    for(auto mode:{AdaptiveSessionMode::Initialization,AdaptiveSessionMode::Comparison,AdaptiveSessionMode::UpdateRates}){
        composition(mode);failures(mode);
    }
    replacementAndIsolation();
    std::cout<<"PASS coefficient policies checks="<<checks<<'\n';
}
