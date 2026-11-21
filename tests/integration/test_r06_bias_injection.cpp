#include "../support/motion_fixture.hpp"
// 固定替身验证依赖边界；真实求解器/MAT数值回归由test_r06_bias_runner保留。
#include "egomotion/experiments/bias_runner.hpp"
#include "egomotion/experiments/production_runner.hpp"
#include "egomotion/experiments/run_context.hpp"
#include "egomotion/sampling/directions.hpp"
#include "egomotion/sampling/random.hpp"
#include "egomotion/simulation/flow.hpp"

#include <Eigen/Geometry>
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace egomotion;
using namespace egomotion::experiments;
using Events=std::vector<std::string>;
constexpr std::size_t kSamples=6; // 三噪声列，每列level 0和1。
constexpr double kTolerance=1e-9; // 沿用既有Bias回归阈值，不批准DEC-005。
std::size_t checks=0;
void require(bool ok,const char* message) {
    ++checks;if(!ok)throw std::runtime_error(message);
}
Failure<NumericalErrorCode> injectedFailure(const std::string& label) {
    ErrorDetail leaf;leaf.stage=ErrorStage::Solve;leaf.message="leaf:"+label;
    leaf.observation_index=2;
    ErrorDetail outer;outer.stage=ErrorStage::Simulation;outer.message="injected:"+label;
    return wrapFailure(NumericalErrorCode::SingularSystem,std::move(outer),
        Failure<NumericalErrorCode>{NumericalErrorCode::ZeroNorm,std::move(leaf)});
}
struct ScriptRandom final:IRandomSource {
    Events& events;
    std::mt19937 engine;
    std::uniform_real_distribution<double> u{0,1};
    std::normal_distribution<double> n{0,1};
    std::size_t resets=0,uniforms=0,normals=0;
    std::size_t fail_reset=0,fail_uniform=0,fail_normal=0;
    explicit ScriptRandom(Events& target):events(target){}
    Status<NumericalErrorCode> reseed(std::uint32_t seed) override {
        events.push_back("R"+std::to_string(seed));
        if(++resets==fail_reset)return Status<NumericalErrorCode>::failure(injectedFailure("R"));
        engine.seed(seed);u.reset();n.reset();
        return Status<NumericalErrorCode>::success();
    }
    Result<Scalar,NumericalErrorCode> uniform() override {
        events.push_back("U");
        if(++uniforms==fail_uniform)return Result<Scalar,NumericalErrorCode>::failure(injectedFailure("U"));
        return Result<Scalar,NumericalErrorCode>::success(u(engine));
    }
    Result<Scalar,NumericalErrorCode> normal() override {
        events.push_back("N");
        if(++normals==fail_normal)return Result<Scalar,NumericalErrorCode>::failure(injectedFailure("N"));
        return Result<Scalar,NumericalErrorCode>::success(n(engine));
    }
};
MotionEstimate motion(std::size_t slot) {
    const std::array<Vector3,6> vectors{{{1,2,3},{-2,1,3},{3,-1,2},
                                       {1,3,-2},{-3,2,1},{2,-3,1}}};
    return test_support::motionValue(vectors[2*slot],vectors[2*slot+1]);
}
struct SolverState {
    Events& events;std::string label;std::size_t slot;
    mutable std::size_t calls=0;std::size_t fail_at=0;
    mutable std::vector<double> flow_sums;
    bool called(const FlowField& field) const {
        events.push_back(label);flow_sums.push_back(field.observations().sum());
        return ++calls==fail_at;
    }
};
struct Franz final:estimators::IFranzMatchedFilter {
    SolverState& state;
    explicit Franz(SolverState& source):state(source){}
    Result<MotionEstimate,NumericalErrorCode> estimate(const FlowField& field) const override {
        if(state.called(field))return Result<MotionEstimate,NumericalErrorCode>::failure(injectedFailure(state.label));
        return Result<MotionEstimate,NumericalErrorCode>::success(motion(state.slot));
    }
};
struct KvD final:estimators::IKvDEstimator {
    SolverState& state;
    explicit KvD(SolverState& source):state(source){}
    Result<estimators::KvDResult,NumericalErrorCode> estimate(
        const FlowField& field,const estimators::KvDParameters& parameters) const override {
        const estimators::KvDParameters source;
        require(parameters.nearness_denominator_epsilon==source.nearness_denominator_epsilon&&
            parameters.initial_threshold==source.initial_threshold&&
            parameters.absolute_threshold==source.absolute_threshold&&
            parameters.accuracy_factor==source.accuracy_factor&&
            parameters.minimum_iterations==source.minimum_iterations&&
            parameters.nominal_maximum_iterations==source.nominal_maximum_iterations&&
            parameters.retain_trace==source.retain_trace,"Keep all KvD source defaults");
        if(state.called(field))return Result<estimators::KvDResult,NumericalErrorCode>::failure(injectedFailure(state.label));
        estimators::KvDResult result;result.motion=motion(state.slot);
        result.initial_franz.before_translation_normalization=motion(state.slot);
        return Result<estimators::KvDResult,NumericalErrorCode>::success(std::move(result));
    }
};
struct ObservedSampler final:IDirectionSampler {
    Events& events;const IDirectionSampler& delegate;
    ObservedSampler(Events& e,const IDirectionSampler& d):events(e),delegate(d){}
    Result<DirectionSet,NumericalErrorCode> sample(int level) const override {
        events.push_back("S"+std::to_string(level));return delegate.sample(level);
    }
};
struct ObservedNoise final:IFlowNoiseModel {
    Events& events;const IFlowNoiseModel& delegate;IRandomSource& expected_random;
    mutable std::vector<double> coefficients;
    ObservedNoise(Events& e,const IFlowNoiseModel& d,IRandomSource& r):events(e),delegate(d),expected_random(r){}
    Result<FlowField,NumericalErrorCode> apply(
        const FlowField& ideal,Scalar coefficient,IRandomSource& random) const override {
        require(&random==&expected_random,"Noise must receive the injected per-run RNG");
        events.push_back("noise");coefficients.push_back(coefficient);
        return delegate.apply(ideal,coefficient,random);
    }
};
struct Harness {
    Events events;
    ScriptRandom random{events};
    SolverState f{events,"F",0,0,0,{}},o{events,"O",1,0,0,{}},m{events,"M",2,0,0,{}};
    Franz franz{f};KvD original{o},modified{m};
    sampling::OccludedOctahedralSampler missing;sampling::OctahedralSampler full;
    simulation::MeanMagnitudeGaussianNoise mean;simulation::ComponentProportionalGaussianNoise component;
    ObservedSampler sampler;ObservedNoise noise;
    explicit Harness(bool occluded):
        sampler(events,occluded?static_cast<const IDirectionSampler&>(missing):full),
        noise(events,occluded?static_cast<const IFlowNoiseModel&>(mean):component,random){}
    BiasEstimators solvers() const {return {franz,original,modified};}
    Result<BiasRunData,ExperimentErrorCode> run(std::uint32_t seed,std::size_t& valid,
        const std::function<void()>& interleave={}) {
        return BiasExperimentRunner(sampler,noise,solvers()).compute({seed,1},random,[&]{
            events.push_back("valid");++valid;if(interleave)interleave();
        });
    }
};
Events expectedEvents(bool occluded,std::uint32_t seed) {
    const std::string reset="R"+std::to_string(seed);
    Events e{reset,"U","U","U","U",reset};
    for(std::size_t column=0;column<3;++column)for(int level=0;level<2;++level) {
        const std::size_t count=(occluded?6:8)*(level==0?1:4);
        e.push_back("S"+std::to_string(level));e.insert(e.end(),count,"U");
        e.push_back("noise");e.insert(e.end(),3*count,"N");
        for(const auto* name:{"F","O","M","valid"})e.push_back(name);
    }
    return e;
}
void checkMetrics(const BiasRunData& data,std::uint32_t seed) {
    std::mt19937 engine(seed);std::uniform_real_distribution<double> u(0,1);
    auto direction=[&] {
        const double z=2*u(engine)-1,phi=2*std::acos(-1.0)*u(engine);
        const double r=std::sqrt(1-z*z);return Vector3{r*std::cos(phi),r*std::sin(phi),z};
    };
    const std::array<Vector3,2> truth{{direction(),direction()}};
    require(data.level_count==2&&data.xwerte==std::vector<double>({6,24}),"Source xwerte remains 6*4^level in both modes");
    for(std::size_t metric=0;metric<6;++metric) {
        const auto estimate=motion(metric/2);
        const Vector3 value=metric%2?estimate.rotation():estimate.translation();
        const Vector3 reference=truth[metric%2];
        const double angle=std::atan2(reference.cross(value).norm(),reference.dot(value));
        const double expected=2*reference.norm()*std::sin(angle/2);
        require(data.ywerte[metric].size()==kSamples,"Six column-major metric samples");
        for(double actual:data.ywerte[metric])require(std::abs(actual-expected)<=kTolerance,
            "Output must use the matching injected solver and translation/rotation slot");
    }
}
void checkCompleteAndIsolation() {
    for(bool occluded:{true,false}) {
        Harness baseline(occluded),nested(!occluded),interleaved(occluded),nested_baseline(!occluded);
        std::size_t a=0,b=0,c=0,d=0;
        auto expected=baseline.run(7,a);auto expected_nested=nested_baseline.run(11,b);
        require(expected.hasValue()&&expected_nested.hasValue(),"Independent baseline runs complete");
        auto actual=interleaved.run(7,c,[&]{if(c==1){
            auto other=nested.run(11,d);require(other.hasValue(),"Interleaved second mode completes");
            require(other.value().ywerte==expected_nested.value().ywerte,"Independent nested output");
        }});
        require(expected.hasValue()&&expected_nested.hasValue()&&actual.hasValue(),"Injected complete runs");
        require(a==kSamples&&b==kSamples&&c==kSamples&&d==kSamples,"All run counters are separate");
        require(actual.value().ywerte==expected.value().ywerte,"Interleaving preserves all output bits");
        require(interleaved.events==expectedEvents(occluded,7)&&
            nested.events==expectedEvents(!occluded,11),"Exact two resets and complete source call order");
        require(interleaved.f.flow_sums==baseline.f.flow_sums&&nested.f.flow_sums==nested_baseline.f.flow_sums,
            "Interleaving preserves RNG-dependent solver inputs");
        require(interleaved.f.flow_sums==interleaved.o.flow_sums&&interleaved.o.flow_sums==interleaved.m.flow_sums,
            "All three solvers receive the same flow");
        require(interleaved.noise.coefficients==std::vector<double>({0.6,0.6,2.4,2.4,9.6,9.6}),"Noise-major loop order");
        checkMetrics(actual.value(),7);checkMetrics(expected_nested.value(),11);
    }
}
void checkCause(const Failure<ExperimentErrorCode>& failure,const std::string& label) {
    bool parent=false,leaf=false;
    for(const auto& cause:failure.detail.causes) {
        if(cause.code=="SingularSystem"&&cause.message=="injected:"+label)parent=true;
        if(cause.code=="ZeroNorm"&&cause.message=="leaf:"+label&&cause.observation_index==2)leaf=true;
    }
    require(parent&&leaf,"Retain both injected error layers and leaf observation identity");
}
void checkFailures() {
    for(bool occluded:{true,false})for(const std::string stage:{"F","O","M","R1","R2","U1","U3","U5","N3"}) {
        Harness h(occluded);std::size_t valid=0;
        if(stage=="F")h.f.fail_at=2;
        if(stage=="O")h.o.fail_at=2;
        if(stage=="M")h.m.fail_at=2;
        if(stage[0]=='R')h.random.fail_reset=std::stoul(stage.substr(1));
        if(stage[0]=='U')h.random.fail_uniform=std::stoul(stage.substr(1));
        if(stage[0]=='N')h.random.fail_normal=std::stoul(stage.substr(1));
        const auto result=h.run(7,valid);const bool solver=stage.size()==1;
        require(!result.hasValue(),"Injected failure must stop the run");
        require(result.error().code==(solver?ExperimentErrorCode::SolverFailed:ExperimentErrorCode::IncompleteRun),
            "Keep solver/RNG failure category");
        require(valid==(solver?1u:0u),"Only fully completed samples count");
        checkCause(result.error(),stage.substr(0,1));
        auto expected=expectedEvents(occluded,7);
        const auto event=stage[0]=='R'?"R7":stage.substr(0,1);
        const std::size_t occurrence=solver?2:std::stoul(stage.substr(1));
        std::size_t seen=0,end=0;
        for(;end<expected.size();++end)if(expected[end]==event&&++seen==occurrence){++end;break;}
        expected.resize(end);
        require(h.events==expected,"No extra draw/solver/callback after the first failing dependency");
    }
    Harness h(true);auto invalid=BiasExperimentRunner(h.sampler,h.noise,h.solvers()).compute({7,-1},h.random);
    require(!invalid.hasValue()&&h.events.empty(),"Invalid level must not touch any dependency");
}
struct NonResettable final:IRandomSource {
    std::size_t draws=0;
    Result<Scalar,NumericalErrorCode> uniform() override {++draws;return Result<Scalar,NumericalErrorCode>::success(0.5);}
    Result<Scalar,NumericalErrorCode> normal() override {++draws;return Result<Scalar,NumericalErrorCode>::success(0);}
};
void checkReseeding() {
    Harness h(true);NonResettable random;
    const auto result=BiasExperimentRunner(h.sampler,h.noise,h.solvers()).compute({7,1},random);
    require(!result.hasValue()&&random.draws==0&&h.events.empty(),"Unsupported reset fails without fallback RNG");
    require(result.error().detail.causes.front().code=="InvalidRandomSample","Explicit unsupported-reset cause");
    sampling::Mt19937RandomSource actual(11),fresh(7);
    require(actual.normal().hasValue()&&actual.reseed(7).hasValue(),"Reset after an odd normal draw");
    for(int i=0;i<8;++i) {
        require(actual.normal().value()==fresh.normal().value(),"Reset must discard cached normal variate");
        require(actual.uniform().value()==fresh.uniform().value(),"Reset must restore complete engine state");
    }
}
// 纯内存交付替身，不声称生成MAT或验证持久化；原有真实Writer测试继续执行。
struct MemoryWriter final:IResultWriter {
    std::map<std::string,DenseDoubleArray> arrays;std::vector<FileEvidence> receipts;
    std::vector<std::string> run_ids;std::size_t fail_at=0,calls=0;
    Status<MatErrorCode> saveNewVariable(const RunId& id,std::string_view name,const DenseDoubleArray& array) override {
        if(++calls==fail_at){ErrorDetail d;d.stage=ErrorStage::Output;d.message="injected writer";
            return Status<MatErrorCode>::failure({MatErrorCode::WriteFailed,std::move(d)});}
        arrays.emplace(std::string(name),array);run_ids.push_back(id.text);
        receipts.push_back({std::string(name),std::string(name)+".mat",std::string(64,'a'),array.info.data_bytes,true});
        return Status<MatErrorCode>::success();
    }
    std::vector<FileEvidence> fileEvidence() const override {return receipts;}
};
struct MemoryPublisher final:ITextPublisher {
    std::string content;
    Status<MatErrorCode> publishNew(std::string_view name,std::string_view data) override {
        require(name=="manifest.json"&&content.empty(),"Publish one Manifest per context");
        content=std::string(data);return Status<MatErrorCode>::success();
    }
};
void checkProduction() {
    for(bool occluded:{true,false})for(int failure:{0,1,2}) {
        Harness h(occluded);Events random_events;
        auto random=std::make_unique<ScriptRandom>(random_events);auto* observed=random.get();
        MemoryWriter writer;MemoryPublisher publisher;
        const auto kind=occluded?ExperimentKind::BiasOccluded:ExperimentKind::BiasProportional;
        const std::string id=std::string(occluded?"occluded_":"proportional_")+std::to_string(failure);
        const std::uint32_t seed=occluded?7:11;
        RunContext context(RunId{id},std::move(random),writer,publisher,"/MEMORY-ONLY",
            {std::string(40,'a'),std::string(40,'b'),seed},kind,1);
        ProductionExperimentRunner runner(h.solvers());
        if(failure==1)h.o.fail_at=2;
        if(failure==2)writer.fail_at=4;
        const auto outcome=occluded?runner.runBiasOccluded({seed,1},context):runner.runBiasProportional({seed,1},context);
        require(observed->resets==2&&h.random.resets==0,"Production must use context-owned RNG");
        require(context.manifestDraft().attempted==1,"One source run attempted");
        if(failure==0) {
            require(outcome.hasValue()&&outcome.value().status==RunStatus::Complete,"Injected production run completes");
            require(writer.arrays.size()==7&&context.manifestDraft().valid==kSamples,"Seven current-save outputs and six samples");
            BiasRunData data;data.level_count=2;data.xwerte=writer.arrays.at("xwerte").values_column_major;
            for(std::size_t i=0;i<6;++i)data.ywerte[i]=writer.arrays.at("ywerte"+std::to_string(i+1)).values_column_major;
            checkMetrics(data,seed);
        } else {
            require(!outcome.hasValue()&&outcome.error().code==(failure==1?ExperimentErrorCode::SolverFailed:ExperimentErrorCode::OutputFailed),
                "Production retains the failing dependency category");
            require(writer.arrays.size()==(failure==1?0u:3u),"Keep exact output prefix; no MAT before numerical completion");
            require(context.manifestDraft().valid==(failure==1?1u:kSamples)&&
                context.manifestDraft().completed==(failure==1?0u:1u),"Keep scientific counters on solver/Writer failure");
            require(context.publishFailureRecord(outcome.error()).hasValue()&&
                context.manifestDraft().status==RunStatus::Incomplete,"Failure with a valid prefix is Incomplete");
        }
        require(std::all_of(writer.run_ids.begin(),writer.run_ids.end(),[&](const auto& saved){return saved==id;}),
            "Writer receives this context's identity only");
        require(publisher.content.find(id)!=std::string::npos,"Manifest retains per-context identity");
    }
}
}
int main() {
    try {checkCompleteAndIsolation();checkFailures();checkReseeding();checkProduction();}
    catch(const std::exception& error){std::cerr<<"FAIL after "<<checks<<" checks: "<<error.what()<<'\n';return 1;}
    std::cout<<"Bias dependency injection: "<<checks<<" checks passed; two modes, exact event order, isolated RNG, failure prefixes\n";
    return 0;
}
