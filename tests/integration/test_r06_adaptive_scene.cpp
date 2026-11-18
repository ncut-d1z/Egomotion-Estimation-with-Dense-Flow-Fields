#include "../support/coefficient_fixture.hpp"
#include "../support/motion_fixture.hpp"
// 合成场景贯穿真实Runner/Session/聚合/发布；不是原MATLAB或科学实验Golden。
#include "egomotion/experiments/production_runner.hpp"
#include "egomotion/experiments/run_context.hpp"
#include "egomotion/estimators/adaptive_step.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
using namespace egomotion;
using namespace egomotion::experiments;
std::size_t checks=0;
constexpr std::size_t kRegularFrames=600,kInitializationFrames=35,kRuns=8;
constexpr double kAngleTolerance=1e-9; // 沿用test_r06_adaptive_runner工程余量。
void require(bool ok,const char* message) {
    ++checks;if(!ok)throw std::runtime_error(message);
}
void near(double actual,double expected) {
    require(std::isfinite(actual)&&std::abs(actual-expected)<=kAngleTolerance,"Analytic contract value");
}
struct Mode {
    ExperimentKind kind;std::size_t frames,runs,slots;const char* name;
};
const std::array<Mode,3> kModes{{
    {ExperimentKind::AdaptiveInitialization,kInitializationFrames,1,3,"init"},
    {ExperimentKind::AdaptiveComparison,kRegularFrames,kRuns,3,"comparison"},
    {ExperimentKind::AdaptiveUpdateRates,kRegularFrames,1,5,"rates"}}};
enum class Fault {None,Failure,Allocation,Length,Count,NonFinite,Zero,Negative,MissingMotion};
Failure<NumericalErrorCode> sceneFailure() {
    ErrorDetail leaf;leaf.stage=ErrorStage::Read;leaf.source_path="场景 输入.mat";
    leaf.variable="scene";leaf.observation_index=2;leaf.run_id=RunId{"scene_leaf"};
    leaf.message="independent scene failure";
    ErrorDetail outer;outer.stage=ErrorStage::Simulation;outer.message="scene service";
    return wrapFailure(NumericalErrorCode::InvalidNearness,std::move(outer),
        Failure<MatErrorCode>{MatErrorCode::MissingVariable,std::move(leaf)});
}
void throwIfRequested(Fault f) {
    if(f==Fault::Allocation)throw std::bad_alloc{};
    if(f==Fault::Length)throw std::length_error("scene fixture");
}
Vector3 position(std::size_t row) {return {static_cast<double>(row%100)/1000.0,0.125,0.25};}
struct Generator final:trajectory::ITrajectoryGenerator {
    Mode mode;Fault fault=Fault::None;mutable std::size_t calls=0;
    explicit Generator(Mode value):mode(value){}
    Result<trajectory::TrajectoryPoints,NumericalErrorCode> make(std::size_t count) const {
        ++calls;throwIfRequested(fault);
        if(fault==Fault::Failure)return Result<trajectory::TrajectoryPoints,NumericalErrorCode>::failure(sceneFailure());
        trajectory::TrajectoryPoints result;result.points.resize(count,3);
        for(std::size_t i=0;i<count;++i)result.points.row(i)=position(i).transpose();
        if(fault==Fault::Count)result.points.conservativeResize(count-1,3);
        if(fault==Fault::NonFinite)result.points(0,0)=std::numeric_limits<double>::quiet_NaN();
        return Result<trajectory::TrajectoryPoints,NumericalErrorCode>::success(std::move(result));
    }
    Result<trajectory::TrajectoryPoints,NumericalErrorCode> circle(
        std::size_t count,double radius,double height) const override {
        require(mode.kind==ExperimentKind::AdaptiveInitialization&&count==mode.frames&&radius==0.6&&height==0.3,
                "Original circle parameters and mode");return make(count);
    }
    Result<trajectory::TrajectoryPoints,NumericalErrorCode> sinusoidal(
        std::size_t count,double amplitude,double length,std::size_t periods,double height) const override {
        require(mode.kind!=ExperimentKind::AdaptiveInitialization&&count==mode.frames&&
                amplitude==0.5&&length==1&&periods==2&&height==0.3,"Original sinusoidal parameters");
        return make(count);
    }
};
struct Kinematics final:trajectory::ITrajectoryKinematics {
    Fault fault=Fault::None;mutable std::size_t calls=0;
    Result<std::vector<trajectory::FrameKinematics>,NumericalErrorCode> fromPoints(
        const trajectory::TrajectoryPoints& points) const override {
        ++calls;throwIfRequested(fault);
        if(fault==Fault::Failure)return Result<std::vector<trajectory::FrameKinematics>,NumericalErrorCode>::failure(sceneFailure());
        const auto count=static_cast<std::size_t>(points.points.rows());
        std::vector<trajectory::FrameKinematics> frames(count);
        for(std::size_t i=0;i<count;++i){
            require((points.points.row(i).transpose().array()==position(i).array()).all(),"Generated positions forwarded in order");
            if(i==0||i+1==count)continue;
            frames[i].local_translation=Vector3::UnitX();
            frames[i].relative_rotation=Vector3::UnitZ();
            frames[i].absolute_rotation=Vector3::Zero();
            frames[i].has_original_matlab_motion=true;
        }
        if(fault==Fault::Count)frames.pop_back();
        if(fault==Fault::NonFinite)frames[1].absolute_rotation.x()=std::numeric_limits<double>::infinity();
        if(fault==Fault::MissingMotion)frames[1].has_original_matlab_motion=false;
        return Result<std::vector<trajectory::FrameKinematics>,NumericalErrorCode>::success(std::move(frames));
    }
};
struct Environment final:trajectory::ISphereEnvironment {
    Mode mode;Fault fault=Fault::None;mutable std::size_t calls=0;std::size_t fail_at=0;
    explicit Environment(Mode value):mode(value){}
    Result<Eigen::VectorXd,NumericalErrorCode> nearnessInUnitSphere(
        const DirectionSet& directions,const Vector3& camera,const Vector3& rotation) const override {
        const std::size_t ordinal=calls++;
        const std::size_t row=ordinal%(mode.frames-2)+1,run=ordinal/(mode.frames-2)+1;
        require(directions.directions().rows()==6,"Injected sampler passed to environment");
        require((camera.array()==position(row).array()).all()&&rotation.isZero(0),"Frame position/orientation order");
        throwIfRequested(fault);
        if(fault==Fault::Failure||calls==fail_at)return Result<Eigen::VectorXd,NumericalErrorCode>::failure(sceneFailure());
        Eigen::VectorXd values=Eigen::VectorXd::Constant(6,static_cast<double>(run));
        if(fault==Fault::Count)values.conservativeResize(5);
        if(fault==Fault::NonFinite)values[0]=std::numeric_limits<double>::quiet_NaN();
        if(fault==Fault::Zero)values[0]=0;
        if(fault==Fault::Negative)values[0]=-1;
        return Result<Eigen::VectorXd,NumericalErrorCode>::success(std::move(values));
    }
};
struct Sampler final:IDirectionSampler {
    Mode mode;mutable std::size_t calls=0;
    explicit Sampler(Mode value):mode(value){}
    Result<DirectionSet,NumericalErrorCode> sample(int level) const override {
        ++calls;require(level==(mode.kind==ExperimentKind::AdaptiveComparison?4:2),"Unchanged source sampling level");
        DirectionMatrix result(6,3);
        result<<1,0,0,-1,0,0,0,1,0,0,-1,0,0,0,1,0,0,-1;
        return DirectionSet::fromOwned(std::move(result));
    }
};
struct Random final:IRandomSource {
    std::size_t normals=0,uniforms=0,resets=0;
    Result<double,NumericalErrorCode> normal() override {++normals;return Result<double,NumericalErrorCode>::success(0.5);}
    Result<double,NumericalErrorCode> uniform() override {++uniforms;return Result<double,NumericalErrorCode>::success(0.5);}
    Status<NumericalErrorCode> reseed(std::uint32_t) override {++resets;return Status<NumericalErrorCode>::success();}
};
struct Step final:estimators::IAdaptiveStepSolver {
    Mode mode;mutable std::size_t calls=0;
    explicit Step(Mode value):mode(value){}
    Result<estimators::AdaptiveStepResult,NumericalErrorCode> solve(
        const HarmonicCoefficients& old,const FlowField& flow) const override {
        const auto ordinal=calls++,slot=ordinal%mode.slots;
        const auto row=(ordinal/mode.slots)%(mode.frames-2)+1;
        const auto run=ordinal/(mode.slots*(mode.frames-2))+1;
        require(flow.observations.rows()==6&&flow.observations(4,3)==-static_cast<double>(run),
                "Injected environment values reach real flow synthesis and Step");
        double expected=static_cast<double>(row);
        if(slot+1!=mode.slots){
            if(mode.slots==3&&slot==0)expected=1;
            if(mode.slots==5){
                constexpr std::array<std::size_t,4> periods{{1,5,11,21}};
                const auto refresh=(row+1)/periods[slot]*periods[slot];
                expected=refresh?static_cast<double>(refresh-1):1;
            }
        }
        require(old.values()(0,0)==expected,"Source reset/inherit/periodic candidate before main solve");
        if(row==1){
            HarmonicMatrix initial=HarmonicMatrix::Zero();initial(0,0)=1;
            require((old.values().array()==initial.array()).all(),"Every run resets all primary and baseline coefficients");
        }
        estimators::AdaptiveStepResult result;
        result.motion=test_support::motionValue(Vector3::UnitX(),Vector3::UnitZ());
        if(slot+1!=mode.slots){
            if(mode.slots==5){
                const std::array<Vector3,4> t{{Vector3::UnitX(),Vector3::UnitY(),-Vector3::UnitX(),Vector3::UnitZ()}};
                const std::array<Vector3,4> r{{Vector3::UnitZ(),Vector3::UnitX(),-Vector3::UnitZ(),Vector3::UnitY()}};
                result.motion=test_support::motionValue(t[slot],r[slot]);
            }else{
                result.motion=test_support::motionValue(
                    (slot==0||run%2==0)?Vector3::UnitY():Vector3::UnitX(),
                    slot==0?Vector3::UnitX():Vector3::UnitZ());
            }
        }
        auto next=old.values();next(0,0)+=1;
        next(1,0)=9; // 检出跨run未重置的非零高阶状态。
        result.coefficients_next=test_support::coefficientValue(next);
        result.trace.coefficients_next=result.coefficients_next;
        return Result<estimators::AdaptiveStepResult,NumericalErrorCode>::success(std::move(result));
    }
};
struct Writer final:IResultWriter {
    std::map<std::string,DenseDoubleArray> arrays;std::vector<FileEvidence> receipts;
    Status<MatErrorCode> saveNewVariable(const RunId&,std::string_view name,const DenseDoubleArray& array) override {
        require(arrays.emplace(std::string(name),array).second,"No duplicate synthetic publication");
        receipts.push_back({std::string(name),std::string(name)+".mat",std::string(64,'c'),array.info.data_bytes,true});
        return Status<MatErrorCode>::success();
    }
    std::vector<FileEvidence> fileEvidence() const override {return receipts;}
};
struct Publisher final:ITextPublisher {
    std::string json;
    Status<MatErrorCode> publishNew(std::string_view name,std::string_view value) override {
        require(name=="manifest.json"&&json.empty(),"Single synthetic Manifest");json=value;
        return Status<MatErrorCode>::success();
    }
};
struct Harness {
    Mode mode;Generator generator;Kinematics kinematics;Environment environment;Sampler sampler;Step step;
    simulation::MeanMagnitudeGaussianNoise noise;Writer writer;Publisher publisher;
    explicit Harness(Mode m):mode(m),generator(m),environment(m),sampler(m),step(m){}
    AdaptiveScene scene() const {return {generator,kinematics,environment};}
    AdaptiveSampling sampling() const {return {sampler,noise};}
};
Result<RunOutcome,ExperimentErrorCode> run(ProductionExperimentRunner& runner,Mode mode,RunContext& context) {
    if(mode.kind==ExperimentKind::AdaptiveInitialization)return runner.runAdaptiveInitialization({},context);
    if(mode.kind==ExperimentKind::AdaptiveComparison)return runner.runAdaptiveComparison({0},context);
    return runner.runAdaptiveUpdateRates({},context);
}
void verifyFields(Mode mode,const Writer& writer) {
    require(writer.arrays.size()==(mode.slots==5?3u:(mode.runs==8?5u:4u)),"Exact current-save set");
    for(const auto& [name,array]:writer.arrays){
        const auto columns=name=="akrot"||mode.slots==3?1u:4u;
        require(array.info.dimensions==std::vector<std::size_t>{mode.frames,columns},"Source column-major shape");
        for(std::size_t c=0;c<columns;++c)for(std::size_t r=0;r<mode.frames;++r){
            double expected=0;
            if(r>0&&r+1<mode.frames){
                if(name=="rottransratio")expected=0.125; // 最后第8轮，不是八轮比值的均值。
                else if(name=="akrot")expected=57.2957795130823208768;
                else if(mode.slots==5)expected=std::array<double,4>{{0,90,180,90}}[c];
                else if(name=="allemittransfehler")expected=mode.runs==8?45:0;
                else if(name=="allemitrotfehler")expected=0;
                else expected=90;
            }
            near(array.values_column_major[c*mode.frames+r],expected);
        }
    }
}
void fullRuns() {
    for(const auto mode:kModes){
        Harness h(mode);auto random=std::make_unique<Random>();auto* observed=random.get();
        RunContext context(RunId{mode.name},std::move(random),h.writer,h.publisher,"/MEMORY-ONLY",
            {std::string(40,'a'),std::string(40,'b'),std::nullopt},mode.kind,mode.runs);
        ProductionExperimentRunner runner(h.sampling(),h.scene(),&h.step);
        const auto result=run(runner,mode,context);
        require(result.hasValue()&&result.value().status==RunStatus::Complete,"Synthetic full Runner completes");
        const auto valid=mode.runs*(mode.frames-2);
        require(context.manifestDraft().attempted==mode.runs&&context.manifestDraft().completed==mode.runs&&
                context.manifestDraft().valid==valid,"Exact completed repetitions and valid interior frames");
        require(h.generator.calls==1&&h.kinematics.calls==1&&h.sampler.calls==1&&h.environment.calls==valid,
                "Scene preparation once, environment once per interior frame");
        require(h.step.calls==valid*mode.slots,"All comparison slots and main Step called");
        require(observed->normals==(mode.runs==8?valid*18:0)&&observed->uniforms==0&&observed->resets==0,
                "Zero-noise still 3N normal draws, no reseed across eight runs");
        verifyFields(mode,h.writer);require(h.publisher.json.find("Complete")!=std::string::npos,"Synthetic Complete Manifest");
    }
}
void checkCause(const Failure<ExperimentErrorCode>& failure) {
    bool found=false;
    for(const auto& c:failure.detail.causes)if(c.domain=="MatErrorCode") {
        found=true;require(c.code=="MissingVariable"&&c.source_path=="场景 输入.mat"&&c.variable=="scene"&&
            c.observation_index==2&&c.run_id&&c.run_id->text=="scene_leaf","Full scene source chain");
    }
    require(found,"Source error retained");
}
void failures() {
    for(const auto mode:kModes)for(int stage:{0,1,2})for(Fault fault:{Fault::Failure,Fault::Allocation,
        Fault::Length,Fault::Count,Fault::NonFinite,Fault::Zero,Fault::Negative,Fault::MissingMotion}) {
        if((fault==Fault::Zero||fault==Fault::Negative)&&stage!=2)continue;
        if(fault==Fault::MissingMotion&&stage!=1)continue;
        Harness h(mode);Random rng;
        if(stage==0)h.generator.fault=fault;
        if(stage==1)h.kinematics.fault=fault;
        if(stage==2)h.environment.fault=fault;
        AdaptiveExperimentRunner runner(h.sampling(),h.scene(),&h.step);
        std::size_t valid=0;
        const AdaptiveProgress progress=[&](AdaptiveProgressEvent e){if(e==AdaptiveProgressEvent::FrameCompleted)++valid;};
        const auto result=mode.kind==ExperimentKind::AdaptiveInitialization?runner.computeInitialization({},progress):
            (mode.kind==ExperimentKind::AdaptiveComparison?runner.computeComparison({0},rng,progress):runner.computeUpdateRates({},progress));
        require(!result.hasValue()&&h.step.calls==0&&valid==0&&rng.normals==0,"Scene failure stops before solver/RNG");
        require(stage!=0||h.kinematics.calls==0,"Invalid trajectory cannot reach kinematics");
        require(stage!=1||fault==Fault::MissingMotion||h.sampler.calls==0,"Invalid kinematics cannot reach sampler");
        if(fault==Fault::Failure)checkCause(result.error());
        if((stage<2&&(fault==Fault::Count||fault==Fault::NonFinite))||fault==Fault::MissingMotion)
            require(result.error().code==ExperimentErrorCode::DataMismatch,"Invalid scene success classification");
    }
}
void fifthRunFailure() {
    const auto mode=kModes[1];Harness h(mode);
    constexpr std::size_t kCompletedRuns=4,kFailureRow=16;
    const auto valid=kCompletedRuns*(mode.frames-2)+kFailureRow-1;
    h.environment.fail_at=valid+1;
    auto random=std::make_unique<Random>();auto* observed=random.get();
    RunContext context(RunId{"partial_scene"},std::move(random),h.writer,h.publisher,"/MEMORY-ONLY",
        {std::string(40,'a'),std::string(40,'b'),std::nullopt},mode.kind,mode.runs);
    ProductionExperimentRunner runner(h.sampling(),h.scene(),&h.step);
    const auto result=run(runner,mode,context);
    require(!result.hasValue()&&h.writer.arrays.empty()&&h.publisher.json.empty(),"Fifth-run failure cannot publish partial averages");
    checkCause(result.error());
    bool located=false;
    for(const auto& cause:result.error().detail.causes)
        located=located||cause.message.find("run_matlab_1_based=5, frame_matlab_1_based=17")!=std::string::npos;
    require(located,"Failure retains exact source run/frame identity");
    require(context.manifestDraft().expected_repetitions==8&&context.manifestDraft().attempted==5&&
        context.manifestDraft().completed==4&&context.manifestDraft().valid==valid,"Retain expected8/completed4/partial valid counts");
    require(h.step.calls==valid*3&&observed->normals==valid*18,"No draws or steps after failing scene service");
    const auto saved=context.publishFailureRecord(result.error());
    require(saved.hasValue()&&context.manifestDraft().status==RunStatus::Incomplete&&
        context.manifestDraft().failed==1&&h.publisher.json.find("Incomplete")!=std::string::npos,
        "Synthetic failure record remains Incomplete without MAT results");
}
// 共用借用服务而不共用Context；各Runner对象贯穿所有交错调用。
struct DirectEntries {
    InitializationRunner initialization;
    AdaptiveComparisonRunner comparison;
    UpdateRateRunner rates;
    explicit DirectEntries(const Harness& h)
        : initialization(h.sampling(),h.scene(),&h.step),
          comparison(h.sampling(),h.scene(),&h.step),rates(h.sampling(),h.scene(),&h.step) {}
    Result<RunOutcome,ExperimentErrorCode> run(Mode mode,RunContext& context) const {
        if(mode.kind==ExperimentKind::AdaptiveInitialization)return initialization.run({},context);
        if(mode.kind==ExperimentKind::AdaptiveComparison)return comparison.run({0},context);
        return rates.run({},context);
    }
};
void prepareScene(Harness& h,Mode mode) {
    // 只复位可计数的替身输入，不能访问或重建被测Runner/Session。
    h.mode=mode;h.generator.mode=mode;h.environment.mode=mode;h.sampler.mode=mode;h.step.mode=mode;
    h.generator.calls=0;h.kinematics.calls=0;h.environment.calls=0;h.sampler.calls=0;h.step.calls=0;
    h.environment.fail_at=0;
}
void completeEntry(const DirectEntries& entries,Harness& h,Mode mode,const std::string& id) {
    prepareScene(h,mode);Writer writer;Publisher publisher;
    auto random=std::make_unique<Random>();auto* observed=random.get();
    RunContext context(RunId{id},std::move(random),writer,publisher,"/MEMORY-ONLY",
        {std::string(40,'a'),std::string(40,'b'),std::nullopt},mode.kind,mode.runs);
    const auto result=entries.run(mode,context);
    require(result.hasValue()&&result.value().status==RunStatus::Complete,"Reused named Runner completes");
    const auto valid=mode.runs*(mode.frames-2);
    const auto& draft=context.manifestDraft();
    require(draft.attempted==mode.runs&&draft.completed==mode.runs&&draft.valid==valid&&draft.failed==0,
            "Fresh Context has only this run counts");
    require(h.generator.calls==1&&h.kinematics.calls==1&&h.sampler.calls==1&&h.environment.calls==valid&&
            h.step.calls==valid*mode.slots,"Full source loop and fresh Session for every repeat");
    require(observed->normals==(mode.runs==8?valid*18:0)&&observed->resets==0&&observed->uniforms==0,
            "Independent Context RNG with unchanged consumption");
    require(publisher.json.find(id)!=std::string::npos&&publisher.json.find("Complete")!=std::string::npos,
            "Manifest belongs to this Context");
    verifyFields(mode,writer);
}
void wrongContext(const DirectEntries& entries,Harness& h) {
    const std::array<ExperimentKind,5> kinds{{ExperimentKind::BiasOccluded,ExperimentKind::BiasProportional,
        ExperimentKind::AdaptiveInitialization,ExperimentKind::AdaptiveComparison,ExperimentKind::AdaptiveUpdateRates}};
    for(const auto mode:kModes)for(const auto kind:kinds){
        if(kind==mode.kind)continue;
        prepareScene(h,mode);Writer writer;Publisher publisher;
        auto random=std::make_unique<Random>();auto* observed=random.get();
        RunContext context(RunId{"wrong_context"},std::move(random),writer,publisher,"/MEMORY-ONLY",
            {std::string(40,'a'),std::string(40,'b'),std::nullopt},kind,mode.runs);
        const auto result=entries.run(mode,context);
        require(!result.hasValue()&&result.error().code==ExperimentErrorCode::InvalidParameters&&
            result.error().detail.stage==ErrorStage::Simulation&&result.error().detail.run_id&&
            result.error().detail.run_id->text=="wrong_context","Wrong kind reports current RunId before work");
        require(h.generator.calls==0&&h.kinematics.calls==0&&h.sampler.calls==0&&h.environment.calls==0&&
            h.step.calls==0&&observed->normals==0&&observed->resets==0&&observed->uniforms==0&&
            context.manifestDraft().attempted==0&&context.manifestDraft().valid==0&&
            writer.arrays.empty()&&publisher.json.empty(),"Wrong Context has no scene/RNG/state/output side effects");
    }
}
void entryFailureRecovery(const DirectEntries& entries,Harness& h) {
    const auto mode=kModes[1];prepareScene(h,mode);
    constexpr std::size_t kValidBeforeFailure=4*(kRegularFrames-2)+15;
    h.environment.fail_at=kValidBeforeFailure+1;
    Writer writer;Publisher publisher;auto random=std::make_unique<Random>();auto* observed=random.get();
    RunContext context(RunId{"entry_failed"},std::move(random),writer,publisher,"/MEMORY-ONLY",
        {std::string(40,'a'),std::string(40,'b'),std::nullopt},mode.kind,mode.runs);
    const auto result=entries.run(mode,context);
    require(!result.hasValue()&&writer.arrays.empty()&&publisher.json.empty(),"Direct failure publishes no partial averages");
    checkCause(result.error());
    require(context.manifestDraft().attempted==5&&context.manifestDraft().completed==4&&
            context.manifestDraft().valid==kValidBeforeFailure&&context.manifestDraft().expected_repetitions==8&&
            h.step.calls==kValidBeforeFailure*3&&observed->normals==kValidBeforeFailure*18,
            "Direct failure retains denominator eight and exact interrupted prefix");
    const auto saved=context.publishFailureRecord(result.error());
    require(saved.hasValue()&&context.manifestDraft().status==RunStatus::Incomplete&&
            context.manifestDraft().failed==1,"Direct incomplete record preserved");
    const auto old_manifest=publisher.json;
    completeEntry(entries,h,mode,"entry_recovered");
    require(publisher.json==old_manifest&&writer.arrays.empty()&&context.manifestDraft().completed==4&&
            context.manifestDraft().valid==kValidBeforeFailure&&observed->normals==kValidBeforeFailure*18,
            "Recovery cannot change failed Context, RNG or publication");
}
void entryIsolation() {
    Harness h(kModes[0]);const DirectEntries entries(h);
    wrongContext(entries,h);
    std::size_t invocation=0;
    for(const auto index:{0u,1u,2u,1u,0u,2u})
        completeEntry(entries,h,kModes[index],std::string("interleaved_")+
            std::to_string(++invocation)+"_"+kModes[index].name);
    // NaN配置在轨迹/RNG前拒绝；下一次合法调用仍从独立Session开始。
    prepareScene(h,kModes[1]);Writer writer;Publisher publisher;auto random=std::make_unique<Random>();
    auto* observed=random.get();
    RunContext context(RunId{"invalid_config"},std::move(random),writer,publisher,"/MEMORY-ONLY",
        {std::string(40,'a'),std::string(40,'b'),std::nullopt},kModes[1].kind,kRuns);
    const auto invalid=entries.comparison.run({std::numeric_limits<double>::quiet_NaN()},context);
    require(!invalid.hasValue()&&invalid.error().code==ExperimentErrorCode::InvalidParameters&&
            h.generator.calls==0&&observed->normals==0&&context.manifestDraft().attempted==0&&
            writer.arrays.empty()&&publisher.json.empty(),"Entry-specific invalid Config has no side effects");
    entryFailureRecovery(entries,h);
}
} // namespace
int main(int argc,char** argv) {
    try {
        if(argc==2&&std::string(argv[1])=="entry-isolation"){
            entryIsolation();
            std::cout<<"adaptive entry checks="<<checks<<"; interleaved complete=6; recovered complete=1; wrong contexts=12; fifth-run failure=1\n";
            return 0;
        }
        if(argc!=1)throw std::runtime_error("Unexpected scene test argument");
        fullRuns();failures();fifthRunFailure();
        std::cout<<"adaptive scene checks="<<checks<<"; full modes=3; complete comparison runs=8; service faults=54; fifth-run failure=1\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<"adaptive scene: "<<error.what()<<'\n';return 1;
    }
}
