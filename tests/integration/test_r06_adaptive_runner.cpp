// 跨 Runner/Session/Writer 的合同测试；替身运动不属于科学结果或 L3 Golden。
// 保留原始35/600帧、级别2/4、8次重复；不运行完整真实数值实验。
#include "egomotion/estimators/adaptive_step.hpp"
#include "egomotion/experiments/production_runner.hpp"
#include "egomotion/experiments/run_context.hpp"
#include "egomotion/io/mat_result_reader.hpp"
#include "egomotion/io/run_directory.hpp"
#include "egomotion/io/run_mat_writer.hpp"
#include "egomotion/io/run_text_publisher.hpp"
#include "egomotion/io/sha256.hpp"
#include "egomotion/sampling/directions.hpp"
#include "egomotion/simulation/flow.hpp"
#include "egomotion/trajectory/trajectory.hpp"
#include "egomotion/contracts/diagnostics.hpp"

#include <Eigen/Geometry>
#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace egomotion;
using experiments::ExperimentKind;
namespace fs=std::filesystem;
// 原脚本结构常量；与本测试替身参数分开。
constexpr std::size_t kInitFrames=35,kFrames=600,kRuns=8,kRates=4;
constexpr std::size_t kLevelTwoDirections=320,kLevelFourDirections=5120,kAxes=3;
constexpr std::array<std::size_t,kRates> kPeriods{{1,5,11,21}};
// 沿用已有 Session 合同测试的严格回归余量；不是 DEC-005 科学验收阈值。
constexpr double kContractAngleTolerance=1e-9;
void require(bool value,const std::string& reason) {
    if(!value)throw std::runtime_error(reason);
}
struct Mode {
    ExperimentKind kind;
    std::size_t frames,runs,directions,comparisons;
    const char* name;
};
const Mode kInitialization{ExperimentKind::AdaptiveInitialization,kInitFrames,1,kLevelTwoDirections,2,"initialization"};
const Mode kComparison{ExperimentKind::AdaptiveComparison,kFrames,kRuns,kLevelFourDirections,2,"comparison"};
const Mode kUpdateRates{ExperimentKind::AdaptiveUpdateRates,kFrames,1,kLevelTwoDirections,kRates,"update-rates"};
MotionEstimate syntheticMotion(std::size_t run,std::size_t frame,std::size_t slot) {
    // 非轴对称、非零的合成输出，各run与各比较列不同，能检出错误平均和列混淆。
    const double r=static_cast<double>(run),f=static_cast<double>(frame),s=static_cast<double>(slot);
    return {Vector3(1+r,2+s,3+f/1000.0),Vector3(3+s,-2-r,1+f/500.0)};
}
struct ScriptedStep final:estimators::IAdaptiveStepSolver {
    const Mode mode;
    std::size_t fail_on_call=0;
    mutable std::size_t calls=0;
    explicit ScriptedStep(Mode selected):mode(selected){}
    Result<estimators::AdaptiveStepResult,NumericalErrorCode> solve(
        const HarmonicCoefficients& old,const FlowField& flow) const override {
        const auto ordinal=calls++;
        const auto calls_per_frame=mode.comparisons+1;
        const auto run=ordinal/((mode.frames-2)*calls_per_frame);
        const auto frame=(ordinal/calls_per_frame)%(mode.frames-2)+2;
        const auto slot=ordinal%calls_per_frame;
        require(run<mode.runs&&flow.observations.cols()==6&&
                flow.observations.rows()==static_cast<Eigen::Index>(mode.directions),
                "Runner changed source frame/run/direction counts");
        double expected=static_cast<double>(frame-1);
        if(slot<mode.comparisons){
            if(mode.kind==ExperimentKind::AdaptiveUpdateRates){
                const auto refreshed=(frame/kPeriods[slot])*kPeriods[slot];
                expected=refreshed<2?1.0:static_cast<double>(refreshed-1);
            }else if(slot==0)expected=1.0;
        }
        require(old.values(0,0)==expected,"Runner/Session reset, refresh or primary order changed");
        auto padding=old.values;padding(0,0)=0;
        require(padding.isZero(0),"Synthetic scalar coefficient acquired other harmonics");
        if(calls==fail_on_call){
            ErrorDetail reason;reason.stage=ErrorStage::Solve;
            reason.message="R6 scripted primary failure";
            return Result<estimators::AdaptiveStepResult,NumericalErrorCode>::failure(
                {NumericalErrorCode::SingularSystem,std::move(reason)});
        }
        estimators::AdaptiveStepResult result;
        result.motion=syntheticMotion(run,frame,slot);
        result.coefficients_next.values.setZero();
        // 比较调用的假 dumi 与主状态明显不同，错误提交会在下一调用被检出。
        result.coefficients_next.values(0,0)=slot==mode.comparisons?
            static_cast<double>(frame):10000.0+static_cast<double>(frame);
        result.trace.coefficients_next=result.coefficients_next;
        return Result<estimators::AdaptiveStepResult,NumericalErrorCode>::success(std::move(result));
    }
};
struct RandomCounts {std::size_t uniform=0,normal=0,fail_on_normal=0;};
struct CountingRandom final:IRandomSource {
    RandomCounts& counts;
    explicit CountingRandom(RandomCounts& target):counts(target){}
    Result<Scalar,NumericalErrorCode> uniform() override {
        ++counts.uniform;return Result<Scalar,NumericalErrorCode>::success(0.5);
    }
    Result<Scalar,NumericalErrorCode> normal() override {
        ++counts.normal;
        if(counts.normal==counts.fail_on_normal){
            ErrorDetail detail;detail.stage=ErrorStage::Simulation;detail.message="R6 scripted RNG failure";
            return Result<Scalar,NumericalErrorCode>::failure(
                {NumericalErrorCode::InvalidRandomSample,std::move(detail)});
        }
        return Result<Scalar,NumericalErrorCode>::success(0.25);
    }
};
using Outcome=Result<RunOutcome,ExperimentErrorCode>;
Outcome execute(experiments::ProductionExperimentRunner& runner,const Mode& mode,RunContext& run,
                double noise=0.0) {
    if(mode.kind==ExperimentKind::AdaptiveInitialization)return runner.runAdaptiveInitialization({},run);
    if(mode.kind==ExperimentKind::AdaptiveComparison)return runner.runAdaptiveComparison({noise},run);
    return runner.runAdaptiveUpdateRates({},run);
}
std::string textFile(const fs::path& path) {
    std::ifstream input(path);
    require(input.is_open(),"Missing Manifest");
    return std::string(std::istreambuf_iterator<char>(input),{});
}
std::string digest(const fs::path& path) {
    const int fd=::open(path.c_str(),O_RDONLY|O_NOFOLLOW|O_CLOEXEC);
    require(fd>=0,"Cannot open published MAT for hashing");
    const auto hash=io::sha256Fd(fd);::close(fd);
    require(hash.hasValue(),"Cannot hash published MAT");return hash.value();
}
struct Sandbox {
    std::unique_ptr<io::RunDirectory> directory;
    std::unique_ptr<io::RunMatWriter> writer;
    std::unique_ptr<io::RunTextPublisher> publisher;
    std::unique_ptr<RunContext> context;
    fs::path path;
    Sandbox(const fs::path& root,const std::string& sha,const Mode& mode,RandomCounts& counts) {
        fs::create_directories(root/"origin");
        auto made=io::RunDirectory::create(root);
        require(made.hasValue(),"Cannot reserve contract-test RunId");
        directory=std::move(made).value();
        writer=std::make_unique<io::RunMatWriter>(*directory);
        publisher=std::make_unique<io::RunTextPublisher>(*directory);
        path=directory->outputRoot()/directory->runId().text;
        require(publisher->publishNew("CONTRACT_TEST_ONLY.txt",
            "Synthetic StepSolver/RNG; not a scientific result or MATLAB reproduction.\n").hasValue(),
            "Cannot mark synthetic output");
        SourceProvenance provenance{sha,"contract-fixture:scripted-step-not-MATLAB",std::nullopt};
        context=std::make_unique<RunContext>(directory->runId(),std::make_unique<CountingRandom>(counts),
            *writer,*publisher,directory->outputRoot(),std::move(provenance),mode.kind,mode.runs);
        std::cout<<"RunId="<<directory->runId().text<<" mode="<<mode.name<<" path="<<path<<'\n';
    }
};
struct ReferenceTrajectory {
    trajectory::TrajectoryPoints points;
    std::vector<trajectory::FrameKinematics> motions;
};
ReferenceTrajectory referenceTrajectory(const Mode& mode) {
    trajectory::TrajectoryGenerator generator;
    const auto points=mode.kind==ExperimentKind::AdaptiveInitialization?
        generator.circle(kInitFrames,0.6,0.3):generator.sinusoidal(kFrames,0.5,1.0,2,0.3);
    require(points.hasValue(),"Reference trajectory failed");
    const auto motions=trajectory::TrajectoryKinematics{}.fromPoints(points.value());
    require(motions.hasValue(),"Reference frame kinematics failed");
    return {points.value(),motions.value()};
}
double angleDegrees(const Vector3& truth,const Vector3& result) {
    // 与Session的 acos(normalized dot) 独立的同角 atan2 表达式。
    return std::atan2(truth.cross(result).norm(),truth.dot(result))*180.0/std::acos(-1.0);
}
double expectedError(const Mode& mode,const trajectory::FrameKinematics& truth,
                     std::size_t row,std::size_t slot,bool rotation) {
    double sum=0;
    for(std::size_t run=0;run<mode.runs;++run){
        const auto motion=syntheticMotion(run,row+1,slot);
        sum+=rotation?angleDegrees(truth.relative_rotation,motion.rotation):
                      angleDegrees(truth.local_translation,motion.translation);
    }
    return sum/static_cast<double>(mode.runs);
}
std::vector<double> idealRatios(const ReferenceTrajectory& reference) {
    // 单独调用已验证的几何/光流叶子，核对Runner传递理想分量统计，无噪声参与。
    const auto directions=sampling::IcosahedralSampler{}.sample(4);
    require(directions.hasValue(),"Reference directions failed");
    std::vector<double> values(kFrames,0);
    for(std::size_t row=1;row+1<kFrames;++row){
        const auto& frame=reference.motions[row];
        const auto near=trajectory::SphereEnvironment{}.nearnessInUnitSphere(directions.value(),
            reference.points.points.row(static_cast<Eigen::Index>(row)).transpose(),frame.absolute_rotation);
        require(near.hasValue(),"Reference sphere nearness failed");
        const auto flow=simulation::FlowSynthesizer{}.synthesize(directions.value(),near.value(),
            frame.local_translation,frame.relative_rotation);
        require(flow.hasValue(),"Reference ideal flow failed");
        values[row]=flow.value().mean_rotational_magnitude/flow.value().mean_translational_magnitude;
    }
    return values;
}
std::vector<std::string> expectedFields(const Mode& mode) {
    if(mode.kind==ExperimentKind::AdaptiveUpdateRates)return {"allemittransfehler","allemitrotfehler","akrot"};
    std::vector<std::string> fields{"allemittransfehler","allemitrotfehler","alleohnetransfehler","alleohnerotfehler"};
    if(mode.kind==ExperimentKind::AdaptiveComparison)fields.push_back("rottransratio");
    return fields;
}
void checkOutput(const Sandbox& sandbox,const Mode& mode) {
    const auto reference=referenceTrajectory(mode);
    const auto ratios=mode.kind==ExperimentKind::AdaptiveComparison?idealRatios(reference):std::vector<double>{};
    const auto receipts=sandbox.writer->fileEvidence();
    const auto names=expectedFields(mode);
    require(receipts.size()==names.size(),"Wrong current-save MAT file count");
    const auto manifest=textFile(sandbox.path/"manifest.json");
    double max_angle_error=0;
    for(std::size_t field=0;field<names.size();++field){
        const auto& name=names[field];
        const auto file=sandbox.path/(name+".mat");
        const auto decoded=io::MatResultReader{}.readNamed(file,name);
        require(decoded.hasValue(),"Cannot read published current-save field");
        const auto& data=decoded.value();
        const auto columns=mode.kind==ExperimentKind::AdaptiveUpdateRates&&name!="akrot"?kRates:1;
        require(data.info.dimensions==std::vector<std::size_t>({mode.frames,columns}),"Output shape changed");
        for(std::size_t column=0;column<columns;++column)
            for(std::size_t row=0;row<mode.frames;++row){
                const double actual=data.values_column_major[row+mode.frames*column];
                if(row==0||row+1==mode.frames){require(actual==0.0,"Source endpoint zero overwritten");continue;}
                if(name=="rottransratio"){
                    require(actual==ratios[row],"Ideal last-run ratio was rescaled, averaged or taken from noisy flow");
                }else if(name=="akrot"){
                    const double expected=reference.motions[row].relative_rotation.norm()*180.0/std::acos(-1.0);
                    require(std::abs(actual-expected)<=kContractAngleTolerance,"akrot angle/unit mismatch");
                }else{
                    const auto slot=mode.kind==ExperimentKind::AdaptiveUpdateRates?column:
                        (name.find("ohne")!=std::string::npos?0:1);
                    const auto expected=expectedError(mode,reference.motions[row],row,slot,name.find("rotfehler")!=std::string::npos);
                    const auto error=std::abs(actual-expected);
                    max_angle_error=std::max(max_angle_error,error);
                    require(std::isfinite(actual)&&error<=kContractAngleTolerance,"Eight-run mean or output column differs from independent angles");
                }
            }
        const auto found=std::find_if(receipts.begin(),receipts.end(),[&](const FileEvidence& item){
            return item.variable==name;
        });
        require(found!=receipts.end(),"Writer receipt missing expected variable identity");
        const auto& receipt=*found;
        const auto hash=digest(file);
        require(receipt.variable==name&&receipt.filename==name+".mat"&&receipt.durably_confirmed&&
                receipt.size_bytes==fs::file_size(file)&&receipt.sha256==hash&&manifest.find(hash)!=std::string::npos,
                "Manifest/Writer receipt differs from actual file identity");
        std::cout<<"SHA256 "<<hash<<' '<<name<<".mat\n";
    }
    std::size_t actual_mats=0;
    for(const auto& file:fs::directory_iterator(sandbox.path))actual_mats+=file.path().extension()==".mat";
    require(actual_mats==names.size(),"Runner wrote extra HistoricalOnly MAT fields");
    std::cout<<"max_contract_angle_error_deg="<<max_angle_error<<'\n';
}
void completeCase(const fs::path& root,const std::string& sha,const Mode& mode) {
    RandomCounts counts;ScriptedStep step(mode);Sandbox sandbox(root,sha,mode,counts);
    experiments::ProductionExperimentRunner runner(&step);
    const auto result=execute(runner,mode,*sandbox.context);
    require(result.hasValue(),result.hasValue()?"":jsonFailure(result.error(),5));
    const auto interior=(mode.frames-2)*mode.runs;
    const auto expected_draws=mode.kind==ExperimentKind::AdaptiveComparison?interior*mode.directions*kAxes:0;
    require(step.calls==interior*(mode.comparisons+1)&&counts.normal==expected_draws&&counts.uniform==0,
            "Runner changed exact StepSolver or RNG consumption counts");
    const auto& outcome=result.value();
    require(outcome.status==RunStatus::Complete&&outcome.attempted==mode.runs&&outcome.completed==mode.runs&&
            outcome.valid==interior&&outcome.failed==0&&outcome.undefined==0,"Incorrect completed outcome counts");
    const auto manifest=textFile(sandbox.path/"manifest.json");
    require(manifest.find("\"completeness\":\"Complete\"")!=std::string::npos&&
            manifest.find("\"valid\":"+std::to_string(interior)+",")!=std::string::npos&&
            manifest.find(sha)!=std::string::npos&&manifest.find("contract-fixture:scripted-step-not-MATLAB")!=std::string::npos,
            "Complete Manifest lost execution identity/counts or synthetic provenance");
    checkOutput(sandbox,mode);
    std::cout<<"PASS complete "<<mode.name<<" steps="<<step.calls<<" normal_draws="<<counts.normal<<'\n';
}
void incompleteCase(const fs::path& root,const std::string& sha,bool rng_failure) {
    RandomCounts counts;ScriptedStep step(kComparison);
    const auto prior_frames=(kRuns-1)*(kFrames-2);
    if(rng_failure)counts.fail_on_normal=7; // 第3条观测的x分量，保留0-based ray index=2。
    else step.fail_on_call=(prior_frames+1)*(kComparison.comparisons+1);
    Sandbox sandbox(root,sha,kComparison,counts);
    experiments::ProductionExperimentRunner runner(&step);
    const auto result=execute(runner,kComparison,*sandbox.context);
    require(!result.hasValue()&&result.error().code==ExperimentErrorCode::SolverFailed,"Partial run returned successful mean");
    const auto& draft=sandbox.context->manifestDraft();
    const auto diagnostic=jsonFailure(result.error(),5);
    std::cout<<"Observed failure: attempted="<<draft.attempted<<" completed="<<draft.completed
             <<" valid="<<draft.valid<<" steps="<<step.calls<<" draws="<<counts.normal<<'\n'<<diagnostic;
    require(sandbox.publisher->publishNew("failure-diagnostic.jsonl",diagnostic).hasValue(),
            "Cannot retain actual failure diagnosis");
    require(draft.completed==(rng_failure?0:kRuns-1)&&draft.valid==(rng_failure?0:prior_frames)&&
            draft.attempted==(rng_failure?1:kRuns)&&draft.status!=RunStatus::Complete,"Partial progress counts were lost or promoted");
    require(step.calls==(rng_failure?0:step.fail_on_call)&&
            counts.normal==(rng_failure?counts.fail_on_normal:(prior_frames+1)*kLevelFourDirections*kAxes),
            "Failure did not stop at exact solver/RNG call");
    require(sandbox.writer->fileEvidence().empty()&&!fs::exists(sandbox.path/"manifest.json"),
            "Incomplete eight-run data reached Writer or Complete Manifest");
    require(!sandbox.context->publishCompleteOutcome().hasValue(),"Missing eighth sample passed Complete gate");
    require(diagnostic.find(rng_failure?"InvalidRandomSample":"SingularSystem")!=std::string::npos&&
            diagnostic.find(rng_failure?"run_matlab_1_based=1":"run_matlab_1_based=8")!=std::string::npos&&
            diagnostic.find("frame_matlab_1_based=2")!=std::string::npos,"Error lost numerical cause or run/frame identity");
    require(sandbox.context->publishFailureRecord(result.error()).hasValue(),"Cannot persist failure audit");
    const auto manifest=textFile(sandbox.path/"manifest.json");
    require(manifest.find(rng_failure?"\"completeness\":\"Failed\"":"\"completeness\":\"Incomplete\"")!=std::string::npos&&
            manifest.find("\"expected_repetitions\":8,")!=std::string::npos&&
            manifest.find("\"mat_files\":[]")!=std::string::npos,"Failure audit changed divisor or invented MAT output");
    std::cout<<"PASS "<<(rng_failure?"RNG-failure":"eighth-run-failure")<<" steps="<<step.calls
             <<" normal_draws="<<counts.normal<<" diagnostic="<<diagnostic;
}
} // namespace
int main(int argc,char** argv) {
    try{
        require(argc==4,"Expected isolated sandbox, fixed build SHA and mode");
        const fs::path root=fs::absolute(argv[1])/"跨帧 合同";
        const std::string sha=argv[2],mode=argv[3];
        require(sha.size()==40,"Expected verified build SHA");
        std::size_t failures=0;
        const auto record=[&](const std::string& label,auto test){
            try{test();}
            catch(const std::exception& error){++failures;std::cerr<<"FAIL "<<label<<": "<<error.what()<<'\n';}
        };
        if(mode=="complete"){
            for(const auto& selected:{kInitialization,kComparison,kUpdateRates})
                record(selected.name,[&]{completeCase(root,sha,selected);});
        }else{
            require(mode=="incomplete","Unknown test mode");
            record("eighth-run-failure",[&]{incompleteCase(root,sha,false);});
            record("RNG-failure",[&]{incompleteCase(root,sha,true);});
        }
        return failures==0?0:1;
    }catch(const std::exception& error){std::cerr<<"FAIL Adaptive Runner contract: "<<error.what()<<'\n';return 1;}
}
