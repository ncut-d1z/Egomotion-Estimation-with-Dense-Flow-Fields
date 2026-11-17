// 有界Bias集成：真实采样/噪声/三个求解器/Writer，seed=7/11，level=0..2。
// 验证编排与发布，不把同一求解器的重放当成独立L3或40份论文统计复现。
#include "egomotion/experiments/bias_runner.hpp"
#include "egomotion/experiments/production_runner.hpp"
#include "egomotion/experiments/run_context.hpp"
#include "egomotion/estimators/franz_kvd.hpp"
#include "egomotion/io/mat_result_reader.hpp"
#include "egomotion/io/run_directory.hpp"
#include "egomotion/io/run_mat_writer.hpp"
#include "egomotion/io/run_text_publisher.hpp"
#include "egomotion/io/sha256.hpp"
#include "egomotion/sampling/directions.hpp"
#include "egomotion/sampling/random.hpp"
#include "egomotion/simulation/flow.hpp"

#include <Eigen/Geometry>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
using namespace egomotion;
using experiments::ExperimentKind;
using experiments::BiasRunData;
namespace fs=std::filesystem;
constexpr std::size_t kLevels=3,kMetrics=6,kColumns=3;
constexpr std::array<double,kColumns> kNoise{{0.6,2.4,9.6}};
// 沿用既有KvD/Runner回归余量，不批准DEC-005或统计误差预算。
constexpr double kTolerance=1e-9;
std::size_t checks=0,failures=0;
double maximum_residual=0;
void check(bool ok,const std::string& why) {
    ++checks;
    if(!ok) {++failures;if(failures<=20)std::cerr<<"FAIL "<<why<<'\n';}
}
void require(bool ok,const std::string& why) {if(!ok)throw std::runtime_error(why);}
void near(double actual,double expected,const std::string& why) {
    const double delta=std::abs(actual-expected);
    if(std::isfinite(delta))maximum_residual=std::max(maximum_residual,delta);
    check(std::isfinite(actual)&&std::isfinite(expected)&&delta<=kTolerance,why);
}
bool occluded(ExperimentKind kind) {return kind==ExperimentKind::BiasOccluded;}
const char* name(ExperimentKind kind) {return occluded(kind)?"occluded":"proportional";}
const char* sourceBlob(ExperimentKind kind) {
    return occluded(kind)?"00a3e974fbf068ccd1af4e541a659862beffcad5":
                          "fc3c4f0a7a417171415fb5e3d869a744ba24d04f";
}
struct StdDraws {
    std::mt19937 engine;
    std::uniform_real_distribution<double> u{0,1};
    std::normal_distribution<double> n{0,1};
    explicit StdDraws(std::uint32_t seed):engine(seed){}
    double uniform(){return u(engine);}double normal(){return n(engine);}
    Vector3 direction() {
        // 从z=2u-1直接构造；不同于生产theta/asin路径。
        const double z=2*uniform()-1,phi=2*std::acos(-1.0)*uniform();
        const double radius=std::sqrt(1-z*z);
        return {radius*std::cos(phi),radius*std::sin(phi),z};
    }
};
struct ObservedSampler final:IDirectionSampler {
    const IDirectionSampler& delegate;
    mutable std::vector<int> levels;
    explicit ObservedSampler(const IDirectionSampler& source):delegate(source){}
    Result<DirectionSet,NumericalErrorCode> sample(int level) const override {
        levels.push_back(level);return delegate.sample(level);
    }
};
struct CheckedRandom final:IRandomSource {
    IRandomSource& source;StdDraws& reference;
    bool inject_failure;
    std::size_t normals=0;
    CheckedRandom(IRandomSource& s,StdDraws& r,bool fail):source(s),reference(r),inject_failure(fail){}
    Result<Scalar,NumericalErrorCode> uniform() override {
        throw std::runtime_error("Noise unexpectedly requested an extra uniform");
    }
    Result<Scalar,NumericalErrorCode> normal() override {
        ++normals;
        if(inject_failure&&normals==3) {
            ErrorDetail detail;detail.stage=ErrorStage::Simulation;
            detail.message="R6 Bias injected third XYZ normal failure";
            return Result<Scalar,NumericalErrorCode>::failure(
                {NumericalErrorCode::InvalidRandomSample,std::move(detail)});
        }
        auto actual=source.normal();require(actual.hasValue(),"Actual C++ RNG failed");
        check(actual.value()==reference.normal(),"Continuous normal stream / second seed reset");
        return actual;
    }
};
struct ObservedNoise final:IFlowNoiseModel {
    const IFlowNoiseModel& delegate;
    mutable StdDraws reference;
    Vector3 translation,rotation;
    mutable std::size_t calls=0,normal_calls=0;
    std::size_t fail_call=0;
    mutable std::vector<FlowField> fields;
    ObservedNoise(const IFlowNoiseModel& source,std::uint32_t seed):delegate(source),reference(seed) {
        StdDraws truth(seed);translation=truth.direction();rotation=truth.direction();
    }
    Result<FlowField,NumericalErrorCode> apply(
        const FlowField& ideal,Scalar coefficient,IRandomSource& random) const override {
        const auto ordinal=calls++;
        require(ordinal<kColumns*kLevels,"Unexpected extra Bias sample");
        check(coefficient==kNoise[ordinal/kLevels],"Noise-major, then level-minor source loop");
        const Eigen::Index count=ideal.observations.rows();
        for(Eigen::Index row=0;row<count;++row) {
            const Vector3 d=ideal.observations.row(row).head<3>().transpose();
            const double mu=1/((3*reference.uniform()+1)/2.5);
            const Vector3 expected=-mu*(translation-translation.dot(d)*d)-rotation.cross(d);
            for(int axis=0;axis<3;++axis)
                near(ideal.observations(row,axis+3),expected[axis],"Truth, nearness and source reseed");
        }
        CheckedRandom checked(random,reference,fail_call==calls);
        auto output=delegate.apply(ideal,coefficient,checked);
        normal_calls+=checked.normals;
        if(output.hasValue()) {
            check(checked.normals==3*static_cast<std::size_t>(count),"Exactly 3N XYZ draws");
            fields.push_back(output.value());
        }
        return output;
    }
};
double metric(const Vector3& truth,const Vector3& estimate) {
    // chord length 与 transfehler_div 等价；不用生产误差函数或其归一表达式。
    const double angle=std::atan2(truth.cross(estimate).norm(),truth.dot(estimate));
    return 2*truth.norm()*std::sin(angle/2);
}
BiasRunData observedCompute(ExperimentKind kind,std::uint32_t seed) {
    sampling::OccludedOctahedralSampler missing;sampling::OctahedralSampler full;
    simulation::MeanMagnitudeGaussianNoise mean;simulation::ComponentProportionalGaussianNoise component;
    ObservedSampler sampler(occluded(kind)?static_cast<const IDirectionSampler&>(missing):full);
    ObservedNoise noise(occluded(kind)?static_cast<const IFlowNoiseModel&>(mean):component,seed);
    std::size_t completed=0;
    const auto result=experiments::BiasExperimentRunner(sampler,noise).compute(
        {seed,static_cast<int>(kLevels)-1},[&]{++completed;});
    if(!result.hasValue())throw std::runtime_error("Bounded real Bias compute failed: "+
        std::string(name(kind))+": "+result.error().detail.message);
    check(completed==kLevels*kColumns&&noise.calls==completed,"Complete samples / callbacks");
    check(result.value().level_count==kLevels&&result.value().noise_coefficients==kNoise,"Bias shape/coefficients");
    const std::size_t base=occluded(kind)?6:8;
    std::size_t normal_expected=0;
    for(std::size_t column=0;column<kColumns;++column)for(std::size_t level=0;level<kLevels;++level) {
        const auto slot=column*kLevels+level;const auto& flow=noise.fields.at(slot);
        const auto factor=std::size_t{1}<<(2*level);
        check(sampler.levels.at(slot)==static_cast<int>(level),"Sampler source level order");
        check(flow.observations.rows()==static_cast<Eigen::Index>(base*factor),"Actual direction count");
        normal_expected+=3*base*factor;
        check(result.value().xwerte.at(level)==6.0*static_cast<double>(factor),
              "Source xwerte must remain 6*4^i even for the full sphere");
        const auto f=estimators::FranzMatchedFilter{}.estimate(flow);
        const auto o=estimators::OriginalKvDEstimator{}.estimate(flow,{});
        const auto m=estimators::ModifiedKvDEstimator{}.estimate(flow,{});
        require(f.hasValue()&&o.hasValue()&&m.hasValue(),"Captured-sample estimator replay failed");
        const std::array<MotionEstimate,3> estimates{{f.value(),o.value().motion,m.value().motion}};
        for(std::size_t method=0;method<estimates.size();++method) {
            near(result.value().ywerte[method*2].at(slot),metric(noise.translation,estimates[method].translation),
                 "Translation metric, solver slot and MATLAB column-major output");
            near(result.value().ywerte[method*2+1].at(slot),metric(noise.rotation,estimates[method].rotation),
                 "Rotation metric, solver slot and MATLAB column-major output");
        }
    }
    check(noise.normal_calls==normal_expected,"Whole-run normal draw count");
    std::cout<<"Bias "<<name(kind)<<" seed="<<seed<<" levels=3 samples="<<completed
             <<" normal_draws="<<noise.normal_calls<<'\n';
    return result.value();
}
void failureContracts() {
    sampling::OctahedralSampler full;simulation::ComponentProportionalGaussianNoise component;
    for(const auto fail_call:{1u,5u}) {
        ObservedSampler sampler(full);ObservedNoise noise(component,7);noise.fail_call=fail_call;
        std::size_t completed=0;
        const auto result=experiments::BiasExperimentRunner(sampler,noise).compute({7,2},[&]{++completed;});
        check(!result.hasValue(),"Injected normal failure must stop Bias");
        require(!result.hasValue(),"Cannot inspect a successful injected failure");
        check(result.error().code==ExperimentErrorCode::IncompleteRun,"Flow failure must not be SolverFailed");
        check(completed==fail_call-1&&noise.calls==fail_call,"Preserve completed prefix; no following sample");
        check(result.error().detail.stage==ErrorStage::Simulation&&!result.error().detail.observation_index,
              "Subdivision/noise column must not become an observation index");
        const auto& message=result.error().detail.message;
        check(message.find("subdivision_level="+std::to_string((fail_call-1)%3))!=std::string::npos&&
              message.find("noise_column_matlab_1_based="+std::to_string((fail_call-1)/3+1))!=std::string::npos,
              "Failure retains exact source sample identity");
        bool found=false;
        for(const auto& cause:result.error().detail.causes)
            found|=cause.code=="InvalidRandomSample"&&cause.observation_index&&*cause.observation_index==0;
        check(found,"Nested XYZ normal cause retains direction index zero");
        std::cout<<"Injected Bias failure call="<<fail_call<<" valid_prefix="<<completed
                 <<" outer_code="<<errorName(result.error().code)<<'\n';
    }
    ObservedSampler sampler(full);ObservedNoise noise(component,7);
    std::size_t completed=0;
    const auto invalid=experiments::BiasExperimentRunner(sampler,noise).compute({7,-1},[&]{++completed;});
    check(!invalid.hasValue()&&invalid.error().code==ExperimentErrorCode::InvalidParameters&&
          sampler.levels.empty()&&noise.calls==0&&completed==0,"Negative level rejected before sampling");
}
void contracts() {
    for(const auto kind:{ExperimentKind::BiasOccluded,ExperimentKind::BiasProportional}) {
        const auto first=observedCompute(kind,7);
        observedCompute(kind,11);
        const auto repeated=observedCompute(kind,7);
        check(first.xwerte==repeated.xwerte&&first.ywerte==repeated.ywerte,
              "A later run with another seed must not contaminate deterministic replay");
    }
    failureContracts();
}

struct FailingWriter final:IResultWriter {
    IResultWriter& real;std::string fail_name;
    explicit FailingWriter(IResultWriter& writer):real(writer){}
    Status<MatErrorCode> saveNewVariable(const RunId& id,std::string_view variable,const DenseDoubleArray& data) override {
        if(variable==fail_name) {
            ErrorDetail detail;detail.stage=ErrorStage::Output;detail.variable=std::string(variable);
            detail.message="R6 deliberate Bias output failure";
            return Status<MatErrorCode>::failure({MatErrorCode::ReadbackMismatch,std::move(detail)});
        }
        return real.saveNewVariable(id,variable,data);
    }
    std::vector<FileEvidence> fileEvidence() const override {return real.fileEvidence();}
};
std::string textFile(const fs::path& path) {
    std::ifstream in(path);require(in.is_open(),"Missing file "+path.string());
    return {std::istreambuf_iterator<char>(in),{}};
}
std::string digest(const fs::path& path) {
    const int fd=::open(path.c_str(),O_RDONLY|O_NOFOLLOW|O_CLOEXEC);
    require(fd>=0,"Cannot open MAT for hash");const auto result=io::sha256Fd(fd);::close(fd);
    require(result.hasValue(),"Cannot hash MAT");return result.value();
}
struct Sandbox {
    std::unique_ptr<io::RunDirectory> directory;
    std::unique_ptr<io::RunMatWriter> writer;
    std::unique_ptr<FailingWriter> forwarding;
    std::unique_ptr<io::RunTextPublisher> publisher;
    std::unique_ptr<RunContext> context;
    fs::path path;
    Sandbox(const fs::path& root,const std::string& sha,ExperimentKind kind) {
        fs::create_directories(root/"origin");auto made=io::RunDirectory::create(root);
        require(made.hasValue(),"Cannot reserve Bias test RunId");directory=std::move(made).value();
        path=directory->outputRoot()/directory->runId().text;
        writer=std::make_unique<io::RunMatWriter>(*directory);
        forwarding=std::make_unique<FailingWriter>(*writer);
        publisher=std::make_unique<io::RunTextPublisher>(*directory);
        require(publisher->publishNew("BOUNDED_TEST_ONLY.txt",
            "CTest seed=7 level=0..2; not full paper experiments or approved numerical Golden.\n").hasValue(),"Mark bounded output");
        context=std::make_unique<RunContext>(directory->runId(),std::make_unique<sampling::Mt19937RandomSource>(7),
            *forwarding,*publisher,directory->outputRoot(),SourceProvenance{sha,sourceBlob(kind),7},kind,1);
        std::cout<<"RunId="<<directory->runId().text<<" kind="<<name(kind)<<" path="<<path<<'\n';
    }
};
Result<RunOutcome,ExperimentErrorCode> run(ExperimentKind kind,RunContext& context,int level=2) {
    experiments::ProductionExperimentRunner runner;
    return occluded(kind)?runner.runBiasOccluded({7,level},context):runner.runBiasProportional({7,level},context);
}
void verifyPublished(const Sandbox& box,const BiasRunData& expected,const std::string& sha) {
    const auto fields=experiments::currentSaveFields(box.context->experimentKind());
    const auto evidence=box.writer->fileEvidence();const auto manifest=textFile(box.path/"manifest.json");
    check(evidence.size()==7&&box.context->manifestDraft().files_writer_confirmed==fields,"Exact seven-file source save order");
    check(manifest.find("\"completeness\":\"Complete\"")!=std::string::npos&&
          manifest.find("\"repo_sha\":\""+sha+"\"")!=std::string::npos&&
          manifest.find("\"algorithm_rng_seed\":7")!=std::string::npos,"Manifest source/seed/status");
    for(std::size_t i=0;i<fields.size();++i) {
        const auto file=box.path/(fields[i]+".mat");const auto data=io::MatResultReader{}.readNamed(file,fields[i]);
        require(data.hasValue(),"Cannot read produced Bias MAT");
        const auto& values=i==0?expected.xwerte:expected.ywerte[i-1];
        check(data.value().info.dimensions==std::vector<std::size_t>{kLevels,i==0?1u:3u},"Exact Bias MAT shape");
        check(data.value().values_column_major==values,"Published binary64 values equal validated memory output");
        const auto receipt=std::find_if(evidence.begin(),evidence.end(),[&](const auto& e){return e.variable==fields[i];});
        require(receipt!=evidence.end(),"Missing receipt");const auto hash=digest(file);
        check(receipt->durably_confirmed&&receipt->sha256==hash&&manifest.find(hash)!=std::string::npos,"Durable MAT digest/Manifest");
    }
    const auto& d=box.context->manifestDraft();
    check(d.attempted==1&&d.completed==1&&d.valid==9&&d.failed==0&&d.undefined==0,"Source sample counters");
}
void publication(const fs::path& root,const std::string& sha) {
    for(const auto kind:{ExperimentKind::BiasOccluded,ExperimentKind::BiasProportional}) {
        const auto expected=observedCompute(kind,7);Sandbox complete(root,sha,kind);
        const auto result=run(kind,*complete.context);
        if(!result.hasValue())throw std::runtime_error("Real Production Bias failed: "+result.error().detail.message);
        check(result.value().status==RunStatus::Complete&&result.value().valid==9,"Complete only after durable Manifest");
        verifyPublished(complete,expected,sha);

        Sandbox partial(root,sha,kind);partial.forwarding->fail_name="ywerte3";
        const auto failed=run(kind,*partial.context);
        require(!failed.hasValue(),"Deliberate Writer failure returned success");
        check(failed.error().code==ExperimentErrorCode::OutputFailed&&partial.writer->fileEvidence().size()==3,
              "Partial Writer must retain xwerte/ywerte1/ywerte2 only");
        check(!fs::exists(partial.path/"manifest.json")&&!fs::exists(partial.path/"ywerte3.mat"),"No premature Complete or failed MAT");
        require(partial.context->publishFailureRecord(failed.error()).hasValue(),"Persist partial failure");
        const auto& d=partial.context->manifestDraft();
        check(d.status==RunStatus::Incomplete&&d.completed==1&&d.valid==9&&d.failed==1,"Partial output preserves numerical completion");
        check(textFile(partial.path/"manifest.json").find("\"completeness\":\"Incomplete\"")!=std::string::npos,
              "Partial publication Manifest must be Incomplete");
        Sandbox invalid(root,sha,kind);const auto rejected=run(kind,*invalid.context,-1);
        require(!rejected.hasValue(),"Negative production level accepted");
        check(rejected.error().code==ExperimentErrorCode::InvalidParameters&&invalid.writer->fileEvidence().empty(),"Invalid request writes no MAT");
        require(invalid.context->publishFailureRecord(rejected.error()).hasValue(),"Persist invalid request");
        check(invalid.context->manifestDraft().status==RunStatus::Failed&&invalid.context->manifestDraft().valid==0,
              "Invalid request is Failed rather than fabricated samples");
    }
}
} // namespace
int main(int argc,char** argv) {
    try {
        require(argc==2||argc==4,"Expected contract or publication arguments");
        const std::string mode=argv[1];
        if(mode=="contract")contracts();
        else {require(mode=="publication"&&argc==4,"Wrong test mode");publication(fs::path(argv[2])/"Bias 中文 合同",argv[3]);}
        std::cout<<std::setprecision(17)<<"Bias "<<mode<<" checks="<<checks<<" failed="<<failures
                 <<" max_absolute_residual="<<maximum_residual<<'\n';
        return failures==0?0:1;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
