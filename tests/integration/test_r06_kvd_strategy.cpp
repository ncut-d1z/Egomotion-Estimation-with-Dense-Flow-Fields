#include "../support/flow_fixture.hpp"
#include "../support/motion_fixture.hpp"
#include "egomotion/estimators/franz_kvd.hpp"
#include "egomotion/contracts/diagnostics.hpp"

#include <array>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <new>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace egomotion;
using namespace egomotion::estimators;
std::size_t checks=0;
void require(bool pass,const char* reason) {
    ++checks;if(!pass)throw std::runtime_error(reason);
}
void same(double a,double b,const char* reason) {require(std::memcmp(&a,&b,sizeof(a))==0,reason);}
template<class A,class B>
void same(const Eigen::MatrixBase<A>& a,const Eigen::MatrixBase<B>& b,const char* reason) {
    require(a.rows()==b.rows()&&a.cols()==b.cols(),"Matrix shape");
    for(Eigen::Index i=0;i<a.rows();++i)for(Eigen::Index j=0;j<a.cols();++j)same(a(i,j),b(i,j),reason);
}
FlowField signedField() {
    // 静态源翻号夹具：四条近x轴视线的负near压过另外四条。
    const double c=std::sqrt(99.0)/10.0,s=0.1;
    const std::array<Vector3,8> directions{{{c,s,0},{c,-s,0},{-c,s,0},{-c,-s,0},
                                         {0,1,0},{0,-1,0},{0,0,1},{0,0,-1}}};
    FlowMatrix field;field.resize(8,6);
    for(std::size_t i=0;i<directions.size();++i) {
        const auto& d=directions[i];field.row(i).head<3>()=d.transpose();
        field.row(i).tail<3>()=((i<4?10.0:-1.0)*(Vector3::UnitX()-d.x()*d)).transpose();
    }
    return test_support::flowValue(field);
}
FlowField axialField() {
    FlowMatrix field;field.resize(6,6);
    field<<1,0,0,0,-0.5,0, -1,0,0,0,0.5,0,
        0,1,0,-0.5,0,0, 0,-1,0,-1.5,0,0, 0,0,1,-2,0,0, 0,0,-1,-2,0,0;
    return test_support::flowValue(field);
}
void directRules() {
    const auto field=axialField();Eigen::VectorXd near(6);near<<1,2,3,4,5,6;
    const auto motion=test_support::motionValue(Vector3{1,0,0},Vector3{0,0,0.5});
    OriginalKvDTranslationRule original;ModifiedKvDTranslationRule modified;
    const auto a=original.compute(field,near,motion),b=modified.compute(field,near,motion);
    require(a.hasValue()&&b.hasValue(),"Direct translation rules");
    // 六方向逐项有理求和，不用生产求和/迭代引擎产生参考。
    same(a.value().mean_a,Vector3{-29.5/6,0.5/6,0},"Original mean near*p");
    same(a.value().mean_b,Vector3{-1.0/6,-1.0/6,-1.0/6},"Original mean near*d");
    same(a.value().mean_c,Vector3{5.0/6,0,0},"Original mean near squared projection");
    same(b.value().mean_a,Vector3{-1,0,0},"Modified mean p");
    same(b.value().mean_b,Vector3::Zero(),"Modified mean d");
    same(b.value().mean_c,Vector3{0.5,0,0},"Modified mean near projection");
    same(a.value().raw_translation.x(),-(-29.5/6+0.5/6-5.0/6),"Original raw scalar reference");
    same(b.value().raw_translation.x(),1.5,"Modified raw translation is not normalized");
    const auto original_input=field.observations();
    const std::array<const IKvDTranslationRule*,2> rules{{&original,&modified}};
    for(const auto* rule:rules) {
        FlowField empty;
        require(rule->compute(empty,Eigen::VectorXd{},motion).error().code==NumericalErrorCode::EmptyInput,"Empty rule input");
        require(rule->compute(field,Eigen::VectorXd::Zero(5),motion).error().code==NumericalErrorCode::InvalidShape,"Near size checked before access");
        auto invalid_near=near;invalid_near[2]=std::numeric_limits<double>::infinity();
        require(rule->compute(field,invalid_near,motion).error().code==NumericalErrorCode::NonFiniteInput,"Non-finite near");
        auto invalid_rotation=motion.rotation();invalid_rotation.x()=std::numeric_limits<double>::quiet_NaN();
        const auto invalid_motion=MotionEstimate::create(motion.translation(),invalid_rotation);
        require(!invalid_motion.hasValue()&&invalid_motion.error().code==NumericalErrorCode::NonFiniteInput,
                "Non-finite old rotation rejected at value construction before rule");
        auto invalid_raw=field.observations();invalid_raw(0,0)=2;
        auto invalid_field=test_support::flowValue(invalid_raw);
        require(rule->compute(invalid_field,near,motion).error().code==NumericalErrorCode::InvalidDirection,"Non-unit rule direction");
        invalid_raw=field.observations();invalid_raw(0,4)=std::numeric_limits<double>::infinity();
        const auto invalid=FlowField::create(invalid_raw);
        require(!invalid.hasValue()&&invalid.error().code==NumericalErrorCode::NonFiniteInput,
                "Non-finite flow rejected at value construction before rule");
    }
    same(field.observations(),original_input,"Direct rules retain input");
}
struct Call { const FlowField* field;Eigen::VectorXd near;MotionEstimate motion; };
enum class Behavior { Delegate, Failure, InvalidMean, InfiniteRaw, ZeroRaw, AllocationFailure, DoubleRaw };
class ObservedRule final : public IKvDTranslationRule {
public:
    explicit ObservedRule(const IKvDTranslationRule& delegate):delegate_(delegate) {}
    mutable std::vector<Call> calls;
    Behavior behavior=Behavior::Delegate;
    std::size_t trigger=3;
    Result<KvDTranslationUpdate,NumericalErrorCode> compute(
        const FlowField& field,const Eigen::VectorXd& near,const MotionEstimate& motion) const override {
        calls.push_back({&field,near,motion});
        if(calls.size()==trigger&&behavior==Behavior::AllocationFailure)throw std::bad_alloc{};
        if(calls.size()==trigger&&behavior==Behavior::Failure) {
            ErrorDetail inner;inner.stage=ErrorStage::Read;inner.message="inner";
            inner.source_path="策略 来源.mat";inner.variable="value";inner.observation_index=7;inner.run_id=RunId{"source_run"};
            ErrorDetail outer;outer.stage=ErrorStage::Solve;outer.message="strategy failure";
            return Result<KvDTranslationUpdate,NumericalErrorCode>::failure(wrapFailure(
                NumericalErrorCode::InvalidNearness,std::move(outer),Failure<MatErrorCode>{MatErrorCode::InvalidShape,std::move(inner)}));
        }
        auto result=delegate_.compute(field,near,motion);if(!result.hasValue())return result;
        auto update=std::move(result).value();
        if(calls.size()==trigger) {
            if(behavior==Behavior::InvalidMean)update.mean_b.x()=std::numeric_limits<double>::quiet_NaN();
            if(behavior==Behavior::InfiniteRaw)update.raw_translation.x()=std::numeric_limits<double>::infinity();
            if(behavior==Behavior::ZeroRaw)update.raw_translation.setZero();
        }
        if(behavior==Behavior::DoubleRaw)update.raw_translation*=2;
        return Result<KvDTranslationUpdate,NumericalErrorCode>::success(std::move(update));
    }
private:
    const IKvDTranslationRule& delegate_;
};
void compare(const KvDResult& a,const KvDResult& b) {
    same(a.motion.translation(),b.motion.translation(),"Final translation");same(a.motion.rotation(),b.motion.rotation(),"Final rotation");
    same(a.final_nearness,b.final_nearness,"Final near");same(a.final_nearness_mean,b.final_nearness_mean,"Mean near");
    require(a.iteration_count==b.iteration_count&&a.trace.size()==b.trace.size(),"Same iteration and trace counts");
    for(std::size_t i=0;i<a.trace.size();++i) {
        const auto& x=a.trace[i];const auto& y=b.trace[i];
        same(x.nearness_raw,y.nearness_raw,"Raw near");same(x.nearness_after_sign,y.nearness_after_sign,"Signed near");
        same(x.translation_candidate_raw,y.translation_candidate_raw,"Raw translation");
        same(x.translation_candidate_normalized,y.translation_candidate_normalized,"Normalized translation");
        same(x.rotation_after,y.rotation_after,"Old translation rotation update");
        same(x.predicted_flow_xyz,y.predicted_flow_xyz,"Prediction");same(x.residual_rms,y.residual_rms,"RMS");
        same(x.residual_change,y.residual_change,"Delta");same(x.active_threshold,y.active_threshold,"Threshold");
        require(x.continue_after_iteration==y.continue_after_iteration&&x.sign_flipped==y.sign_flipped,"Stop/sign choices");
    }
}
void observations(const FlowField& field,const KvDResult& result,const ObservedRule& rule) {
    require(rule.calls.size()==result.iteration_count,"Exactly one strategy call per iteration");
    for(std::size_t i=0;i<rule.calls.size();++i) {
        const auto& call=rule.calls[i];const auto& trace=result.trace[i];
        require(call.field==&field,"Borrow caller flow only during compute");
        same(call.near,trace.nearness_after_sign,"Strategy sees corrected near");
        same(call.motion.translation(),trace.translation_after_sign,"Strategy sees corrected OLD translation");
        same(call.motion.rotation(),trace.rotation_before,"Strategy sees OLD rotation");
    }
}
void engineComposition() {
    OriginalKvDTranslationRule original;ModifiedKvDTranslationRule modified;KvDIterationEngine engine;
    KvDParameters parameters;parameters.retain_trace=true;
    const std::array<FlowField,2> fields{{axialField(),signedField()}};
    for(const auto& field:fields) {
        const auto before=field.observations();
        const auto expected_a=OriginalKvDEstimator{}.estimate(field,parameters),expected_b=ModifiedKvDEstimator{}.estimate(field,parameters);
        require(expected_a.hasValue()&&expected_b.hasValue(),"Default composed estimators solve fixtures");
        ObservedRule a(original),b(modified);
        const auto first=engine.estimate(field,parameters,a),second=engine.estimate(field,parameters,b);
        require(first.hasValue()&&second.hasValue(),"One engine accepts either explicit strategy");
        compare(first.value(),expected_a.value());compare(second.value(),expected_b.value());
        observations(field,first.value(),a);observations(field,second.value(),b);
        const auto repeat=engine.estimate(field,parameters,original);
        require(repeat.hasValue(),"Interleaved strategy reuse succeeds");compare(first.value(),repeat.value());
        auto quiet=parameters;quiet.retain_trace=false;
        const auto no_trace=engine.estimate(field,quiet,modified);
        require(no_trace.hasValue()&&no_trace.value().trace.empty(),"Trace opt-out retained");
        same(no_trace.value().motion.translation(),second.value().motion.translation(),"Trace cannot affect motion");
        same(field.observations(),before,"Engine does not modify flow");
        require(first.value().trace.front().translation_candidate_raw.x()!=second.value().trace.front().translation_candidate_raw.x(),"Original/Modified formulas remain distinct");
        if(field.observations().rows()==8)require(first.value().trace.front().sign_flipped,"Sign-flip path actually covered");
    }
}
void strategyFailures() {
    OriginalKvDTranslationRule original;KvDIterationEngine engine;KvDParameters parameters;
    parameters.retain_trace=true;const auto field=axialField();
    for(const auto mode:{Behavior::Failure,Behavior::InvalidMean,Behavior::InfiniteRaw,Behavior::ZeroRaw,Behavior::AllocationFailure}) {
        ObservedRule rule(original);rule.behavior=mode;
        const auto result=engine.estimate(field,parameters,rule);
        require(!result.hasValue()&&rule.calls.size()==rule.trigger,"Stop exactly at failing strategy call");
        const auto expected=mode==Behavior::Failure?NumericalErrorCode::InvalidNearness:
            mode==Behavior::ZeroRaw?NumericalErrorCode::ZeroNorm:
            mode==Behavior::AllocationFailure?NumericalErrorCode::ResourceLimitExceeded:NumericalErrorCode::NonFiniteResult;
        require(result.error().code==expected,"Strategy failure classification");
        if(mode==Behavior::Failure) {
            const auto json=jsonFailure(result.error(),5);
            require(json.find("策略 来源.mat")!=std::string::npos&&json.find("source_run")!=std::string::npos&&
                    json.find("\"observation_index\":7")!=std::string::npos&&json.find("\"variable\":\"value\"")!=std::string::npos,
                    "Strategy failure preserves full nested location");
        }
        require(engine.estimate(field,parameters,original).hasValue(),"Failed call leaves engine reusable");
    }
    ObservedRule unused(original);auto invalid=parameters;invalid.minimum_iterations=0;
    require(!engine.estimate(field,invalid,unused).hasValue()&&unused.calls.empty(),"Parameters fail before strategy");
    FlowField empty;require(!engine.estimate(empty,parameters,unused).hasValue()&&unused.calls.empty(),"Franz failure precedes strategy");
    ObservedRule doubled(original);doubled.behavior=Behavior::DoubleRaw;
    const auto altered=engine.estimate(field,parameters,doubled),normal=engine.estimate(field,parameters,original);
    require(altered.hasValue()&&normal.hasValue(),"Replacement supplies usable raw candidate");
    for(std::size_t i=0;i<altered.value().trace.size();++i) {
        same(altered.value().trace[i].translation_candidate_raw,2*normal.value().trace[i].translation_candidate_raw,"Engine uses replacement candidate");
        same(altered.value().trace[i].translation_candidate_normalized,normal.value().trace[i].translation_candidate_normalized,"Normalization belongs to engine once");
    }
}
} // namespace
int main() {
    directRules();engineComposition();strategyFailures();
    std::cout<<"PASS KvD strategy composition checks="<<checks<<'\n';
}
