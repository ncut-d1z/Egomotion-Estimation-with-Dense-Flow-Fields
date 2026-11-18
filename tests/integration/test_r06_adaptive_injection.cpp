#include "../support/coefficient_fixture.hpp"
#include "../support/motion_fixture.hpp"
// 只验证组合/失败/所有权；固定Step不是数值Golden或完整科学实验。
#include "egomotion/experiments/production_runner.hpp"
#include "egomotion/experiments/run_context.hpp"
#include "egomotion/estimators/adaptive_step.hpp"

#include <array>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
using namespace egomotion;
using namespace egomotion::experiments;
using Events=std::vector<std::string>;
struct Scene {
    trajectory::TrajectoryGenerator generator;
    trajectory::TrajectoryKinematics kinematics;
    trajectory::SphereEnvironment environment;
    AdaptiveScene services() const {return {generator,kinematics,environment};}
};
std::size_t checks=0;
void require(bool value,const char* message) {
    ++checks;if(!value)throw std::runtime_error(message);
}
template<class A,class B>
void same(const Eigen::MatrixBase<A>& a,const Eigen::MatrixBase<B>& b) {
    require(a.rows()==b.rows()&&a.cols()==b.cols(),"Matrix shape");
    for(Eigen::Index i=0;i<a.size();++i){
        const double x=a.derived().data()[i],y=b.derived().data()[i];
        require(std::memcmp(&x,&y,sizeof(double))==0,"Exact routing/layout values");
    }
}
Failure<NumericalErrorCode> failure() {
    ErrorDetail inner;inner.stage=ErrorStage::Read;inner.message="source fixture";
    inner.source_path="依赖 输入.mat";inner.variable="directions";inner.observation_index=2;
    inner.run_id=RunId{"dependency_source"};
    ErrorDetail outer;outer.stage=ErrorStage::Simulation;outer.message="injected service";
    return wrapFailure(NumericalErrorCode::InvalidDirection,std::move(outer),
        Failure<MatErrorCode>{MatErrorCode::InvalidShape,std::move(inner)});
}
enum class Fault { None, Failure, Allocation, Length, Empty, NonFinite, Direction };
struct Sampler final:IDirectionSampler {
    Events& events;Fault fault=Fault::None;bool real=false;
    mutable std::vector<int> levels;
    explicit Sampler(Events& target):events(target){}
    Result<DirectionSet,NumericalErrorCode> sample(int level) const override {
        events.push_back("sample");levels.push_back(level);
        if(fault==Fault::Failure)return Result<DirectionSet,NumericalErrorCode>::failure(failure());
        if(fault==Fault::Allocation)throw std::bad_alloc{};
        if(fault==Fault::Length)throw std::length_error("sampler fixture");
        if(real)return sampling::IcosahedralSampler{}.sample(level);
        DirectionMatrix result;
        if(fault!=Fault::Empty){
            result.resize(6,3);
            result<<1,0,0,-1,0,0,0,1,0,0,-1,0,0,0,1,0,0,-1;
            if(fault==Fault::NonFinite)result(2,0)=std::numeric_limits<double>::infinity();
            if(fault==Fault::Direction)result.row(2)*=2;
        }
        return DirectionSet::fromOwned(std::move(result));
    }
};
struct Random final:IRandomSource {
    Events& events;std::size_t uniforms=0,normals=0,resets=0;
    explicit Random(Events& target):events(target){}
    Result<double,NumericalErrorCode> uniform() override {
        ++uniforms;return Result<double,NumericalErrorCode>::success(0.5);
    }
    Result<double,NumericalErrorCode> normal() override {
        ++normals;events.push_back("normal");return Result<double,NumericalErrorCode>::success(0.5);
    }
    Status<NumericalErrorCode> reseed(std::uint32_t) override {
        ++resets;return Status<NumericalErrorCode>::success();
    }
};
struct Noise final:IFlowNoiseModel {
    Events& events;Fault fault=Fault::None;bool substitute=false;
    mutable std::vector<double> coefficients;
    mutable std::vector<FlowField> outputs;
    explicit Noise(Events& target):events(target){}
    Result<FlowField,NumericalErrorCode> apply(
        const FlowField& ideal,double coefficient,IRandomSource& random) const override {
        events.push_back("noise");coefficients.push_back(coefficient);
        if(fault==Fault::Failure)return Result<FlowField,NumericalErrorCode>::failure(failure());
        if(fault==Fault::Allocation)throw std::bad_alloc{};
        if(fault==Fault::Length)throw std::length_error("noise fixture");
        auto result=simulation::MeanMagnitudeGaussianNoise{}.apply(ideal,coefficient,random);
        if(!result.hasValue())return result;
        auto value=std::move(result).value();
        if(substitute)value.observations(0,4)+=0.25; // e_x方向的y流分量，替换可被求解器观察。
        if(fault==Fault::Empty)value.observations.resize(0,6);
        if(fault==Fault::NonFinite)value.observations(0,4)=std::numeric_limits<double>::quiet_NaN();
        if(fault==Fault::Direction)value.observations(0,0)=-1;
        outputs.push_back(value);
        return Result<FlowField,NumericalErrorCode>::success(std::move(value));
    }
};
struct Step final:estimators::IAdaptiveStepSolver {
    Events& events;std::size_t fail_at=1;
    mutable std::vector<FlowField> flows;
    mutable std::vector<HarmonicCoefficients> coefficients;
    explicit Step(Events& target):events(target){}
    Result<estimators::AdaptiveStepResult,NumericalErrorCode> solve(
        const HarmonicCoefficients& old,const FlowField& field) const override {
        events.push_back("step");flows.push_back(field);coefficients.push_back(old);
        if(flows.size()==fail_at)
            return Result<estimators::AdaptiveStepResult,NumericalErrorCode>::failure(failure());
        estimators::AdaptiveStepResult result;
        result.motion=test_support::motionValue(Vector3{1,2,3},Vector3{3,2,1});
        auto next=old.values();next(0,0)+=0.01;
        result.coefficients_next=test_support::coefficientValue(next);
        result.trace.coefficients_next=result.coefficients_next;
        return Result<estimators::AdaptiveStepResult,NumericalErrorCode>::success(std::move(result));
    }
};
using Data=Result<AdaptiveRunData,ExperimentErrorCode>;
Data compute(int mode,const AdaptiveExperimentRunner& runner,Random& rng,
             const AdaptiveProgress& progress={},double noise=0) {
    if(mode==0)return runner.computeInitialization({},progress);
    if(mode==1)return runner.computeComparison({noise},rng,progress);
    return runner.computeUpdateRates({},progress);
}
void checkCause(const Failure<ExperimentErrorCode>& error) {
    bool found=false;
    for(const auto& cause:error.detail.causes)if(cause.domain=="MatErrorCode") {
        found=true;require(cause.code=="InvalidShape"&&cause.source_path=="依赖 输入.mat"&&
            cause.variable=="directions"&&cause.observation_index==2&&cause.run_id&&
            cause.run_id->text=="dependency_source","Keep complete injected source identity");
    }
    require(found,"Nested cause not lost");
}
void routingAndReplay() {
    for(int mode:{0,1,2})for(double coefficient:{-0.25,0.0,0.5}) {
        Events events;Sampler sampler(events);Noise noise(events);Random rng(events);Step step(events);
        noise.substitute=true;
        const std::size_t slots=mode==2?5:3;step.fail_at=slots+1;
        Scene scene;AdaptiveExperimentRunner runner({sampler,noise},scene.services(),&step);
        auto progress=[&](AdaptiveProgressEvent event){
            events.push_back(event==AdaptiveProgressEvent::RunStarted?"start":
                event==AdaptiveProgressEvent::FrameCompleted?"frame":"complete");};
        const auto result=compute(mode,runner,rng,progress,coefficient);
        require(!result.hasValue()&&result.error().code==ExperimentErrorCode::SolverFailed,"Stop at injected second-frame solver failure");
        checkCause(result.error());
        Events expected{"sample","start"};
        for(int frame=0;frame<2;++frame){
            if(mode==1){expected.push_back("noise");expected.insert(expected.end(),18,"normal");}
            expected.insert(expected.end(),frame==0?slots:1,"step");
            if(frame==0)expected.push_back("frame");
        }
        require(events==expected,"Sampling/noise/RNG/Session order and complete-frame accounting");
        require(sampler.levels==std::vector<int>{mode==1?4:2},"Keep source subdivision levels");
        require(rng.uniforms==0&&rng.resets==0&&rng.normals==(mode==1?36u:0u),"Exact normal consumption without reseed");
        if(mode==1) {
            require(noise.coefficients==std::vector<double>(2,coefficient),"Noise coefficient is passed explicitly");
            for(std::size_t i=0;i<slots;++i)same(step.flows[i].observations,noise.outputs[0].observations);
            same(step.flows.back().observations,noise.outputs[1].observations);
        }else require(noise.coefficients.empty(),"No noise service in non-Comparison entry");
        const auto input=step.flows.front();
        const auto old_calls=step.flows.size();step.fail_at=old_calls+slots+1;
        const auto again=compute(mode,runner,rng);
        require(!again.hasValue(),"Replay has its own Session");
        require(step.flows.size()==old_calls+slots+1,"Replay begins at source frame two");
        for(std::size_t i=0;i<slots;++i)
            same(step.coefficients[old_calls+i].values(),step.coefficients[i].values());
        // 重放Comparison使用新的系数0，故只核对原方向；其它模式核对全部输入。
        same(step.flows[old_calls].observations.leftCols<3>().eval(),input.observations.leftCols<3>().eval());
        if(mode!=1)same(step.flows[old_calls].observations,input.observations);
    }
}
void serviceFailures() {
    for(int mode:{0,1,2})for(Fault fault:{Fault::Failure,Fault::Allocation,Fault::Length,
                                       Fault::Empty,Fault::NonFinite,Fault::Direction}) {
        Events events;Sampler sampler(events);Noise noise(events);Random rng(events);Step step(events);
        sampler.fault=fault;Scene scene;AdaptiveExperimentRunner runner({sampler,noise},scene.services(),&step);
        const auto result=compute(mode,runner,rng);
        require(!result.hasValue()&&step.flows.empty()&&noise.coefficients.empty()&&rng.normals==0,
                "Invalid sampling cannot reach noise or solver");
        if(fault==Fault::Failure)checkCause(result.error());
        if(fault==Fault::NonFinite){
            bool found=false;
            for(const auto& cause:result.error().detail.causes)
                found=found||(cause.domain=="NumericalErrorCode"&&cause.code=="NonFiniteInput"&&
                    cause.stage==ErrorStage::Simulation&&cause.observation_index==2);
            require(found,"Direction factory failure preserves the actual zero-based row through Runner");
        }
    }
    for(Fault fault:{Fault::Failure,Fault::Allocation,Fault::Length,Fault::Empty,Fault::NonFinite,Fault::Direction}) {
        Events events;Sampler sampler(events);Noise noise(events);Random rng(events);Step step(events);
        noise.fault=fault;Scene scene;AdaptiveExperimentRunner runner({sampler,noise},scene.services(),&step);
        const auto result=compute(1,runner,rng);
        require(!result.hasValue()&&step.flows.empty(),"Invalid noise success never reaches Session");
        if(fault==Fault::Failure){require(result.error().code==ExperimentErrorCode::IncompleteRun,"Noise failure classification");checkCause(result.error());}
        if(fault==Fault::Empty||fault==Fault::NonFinite||fault==Fault::Direction)
            require(result.error().code==ExperimentErrorCode::DataMismatch,"Noise contract mismatch classification");
    }
    Events events;Sampler sampler(events);Noise noise(events);Random rng(events);Step step(events);
    Scene scene;const AdaptiveExperimentRunner runner({sampler,noise},scene.services(),&step);
    const auto invalid=compute(1,runner,rng,{},std::numeric_limits<double>::quiet_NaN());
    require(!invalid.hasValue()&&invalid.error().code==ExperimentErrorCode::InvalidParameters&&events.empty(),
            "Invalid config before dependencies");
}
struct Writer final:IResultWriter {
    std::size_t calls=0;
    Status<MatErrorCode> saveNewVariable(const RunId&,std::string_view,const DenseDoubleArray&) override {
        ++calls;return Status<MatErrorCode>::success();
    }
    std::vector<FileEvidence> fileEvidence() const override {return {};}
};
struct Publisher final:ITextPublisher {
    std::size_t calls=0;
    Status<MatErrorCode> publishNew(std::string_view,std::string_view) override {
        ++calls;return Status<MatErrorCode>::success();
    }
};
void productionComposition() {
    for(int mode:{0,1,2}) {
        Events events;Sampler sampler(events);sampler.real=true;Noise noise(events);Step step(events);
        Step default_step(events);Writer writer;Publisher publisher;
        const std::array<ExperimentKind,3> kinds{{ExperimentKind::AdaptiveInitialization,
            ExperimentKind::AdaptiveComparison,ExperimentKind::AdaptiveUpdateRates}};
        std::array<FlowField,2> inputs;std::array<std::size_t,2> normals{};
        for(int injected:{0,1}){
            auto rng=std::make_unique<Random>(events);auto* observed=rng.get();
            RunContext context(RunId{injected?"injected":"default"},std::move(rng),writer,publisher,
                "/MEMORY-ONLY",{std::string(40,'a'),std::string(40,'b'),std::nullopt},kinds[mode],mode==1?8:1);
            ProductionExperimentRunner defaults(&default_step);
            ProductionExperimentRunner explicit_services(AdaptiveSampling{sampler,noise},&step);
            auto& runner=injected?explicit_services:defaults;
            const auto result=mode==0?runner.runAdaptiveInitialization({},context):
                (mode==1?runner.runAdaptiveComparison({0.25},context):runner.runAdaptiveUpdateRates({},context));
            require(!result.hasValue()&&context.manifestDraft().attempted==1&&
                context.manifestDraft().valid==0&&context.manifestDraft().completed==0,"First-frame failure cannot publish Complete");
            require(observed->uniforms==0&&observed->resets==0,"Context RNG is not replaced/reseeded");
            checkCause(result.error());
            const auto& active_step=injected?step:default_step;
            require(active_step.flows.size()==1,"Production reached the injected solver exactly once");
            inputs[injected]=active_step.flows.front();normals[injected]=observed->normals;
        }
        same(inputs[0].observations,inputs[1].observations);
        require(normals[0]==normals[1]&&normals[1]==(mode==1?15360u:0u),"Default/explicit exact source noise consumption");
        require(sampler.levels==std::vector<int>{mode==1?4:2},"Production forwards sampling dependency");
        require(noise.coefficients.size()==(mode==1?1u:0u),"Production noise binding");
        require(writer.calls==0&&publisher.calls==0,"No scientific outputs from failed compute");
    }
}
} // namespace
int main() {
    try {
        routingAndReplay();serviceFailures();productionComposition();
        std::cout<<"adaptive injection checks="<<checks<<"; routing/replay=9; service faults=24; production pairs=3\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<"adaptive injection: "<<error.what()<<'\n';return 1;
    }
}
