#include "../support/flow_fixture.hpp"
#include "../support/coefficient_fixture.hpp"
#include "../support/motion_fixture.hpp"
#include "../support/adaptive_scalar_reference.hpp"
// 跨 Runner/Session/Writer 的合同测试；替身运动不属于科学结果或 L3 Golden。
// DEC-R6-02/03：原零旋转帧可评价；真实求解失败仍回滚，指标缺测保留固定分母。
#include "adaptive_aggregation.hpp"
#include "adaptive_publication.hpp"
#include "egomotion/estimators/adaptive_step.hpp"
#include "egomotion/estimators/adaptive_session.hpp"
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
    return test_support::motionValue(Vector3(1+r,2+s,3+f/1000.0),Vector3(3+s,-2-r,1+f/500.0));
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
        require(run<mode.runs&&flow.observations().cols()==6&&
                flow.observations().rows()==static_cast<Eigen::Index>(mode.directions),
                "Runner changed source frame/run/direction counts");
        double expected=static_cast<double>(frame-1);
        if(slot<mode.comparisons){
            if(mode.kind==ExperimentKind::AdaptiveUpdateRates){
                const auto refreshed=(frame/kPeriods[slot])*kPeriods[slot];
                expected=refreshed<2?1.0:static_cast<double>(refreshed-1);
            }else if(slot==0)expected=1.0;
        }
        require(old.values()(0,0)==expected,"Runner/Session reset, refresh or primary order changed");
        auto padding=old.values();padding(0,0)=0;
        require(padding.isZero(0),"Synthetic scalar coefficient acquired other harmonics");
        if(calls==fail_on_call){
            ErrorDetail reason;reason.stage=ErrorStage::Solve;
            reason.message="R6 scripted primary failure";
            return Result<estimators::AdaptiveStepResult,NumericalErrorCode>::failure(
                {NumericalErrorCode::SingularSystem,std::move(reason)});
        }
        estimators::AdaptiveStepResult result;
        result.motion=syntheticMotion(run,frame,slot);
        HarmonicMatrix next=HarmonicMatrix::Zero();
        // 比较调用的假 dumi 与主状态明显不同，错误提交会在下一调用被检出。
        next(0,0)=slot==mode.comparisons?
            static_cast<double>(frame):10000.0+static_cast<double>(frame);
        result.coefficients_next=test_support::coefficientValue(next);
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
using AdaptiveData=experiments::AdaptiveRunData;
using MemoryResult=Result<AdaptiveData,ExperimentErrorCode>;
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
    return std::atan2(truth.cross(result).norm(),truth.dot(result))*180.0/std::acos(-1.0);
}
double rotationErrorDegrees(const Vector3& truth,const Vector3& result) {
    return static_cast<double>(adaptive_reference::rotationErrorDegrees(
        {truth.x(),truth.y(),truth.z()},{result.x(),result.y(),result.z()}));
}
double expectedError(const Mode& mode,const trajectory::FrameKinematics& truth,
                     std::size_t row,std::size_t slot,bool rotation) {
    double sum=0;
    for(std::size_t run=0;run<mode.runs;++run){
        const auto motion=syntheticMotion(run,row+1,slot);
        sum+=rotation?rotationErrorDegrees(truth.relative_rotation,motion.rotation()):
                      angleDegrees(truth.local_translation,motion.translation());
    }
    return sum/static_cast<double>(mode.runs);
}
std::vector<std::string> expectedFields(const Mode& mode) {
    if(mode.kind==ExperimentKind::AdaptiveUpdateRates)return {"allemittransfehler","allemitrotfehler","akrot"};
    std::vector<std::string> fields{"allemittransfehler","allemitrotfehler","alleohnetransfehler","alleohnerotfehler"};
    if(mode.kind==ExperimentKind::AdaptiveComparison)fields.push_back("rottransratio");
    return fields;
}
void checkOutput(const Sandbox& sandbox,const Mode& mode,
                 const experiments::AdaptiveRunData* fixed_expected=nullptr) {
    const auto reference=referenceTrajectory(mode);
    const auto receipts=sandbox.writer->fileEvidence();
    const auto names=expectedFields(mode);
    require(receipts.size()==2*names.size()+1,"Missing policy or validity MAT companion");
    require(!sandbox.context->manifestDraft().files_writer_confirmed.empty()&&
            sandbox.context->manifestDraft().files_writer_confirmed.front()=="egomotion_metric_policy",
            "Metric policy must publish before source fields");
    const auto policy=io::MatResultReader{}.readNamed(sandbox.path/"egomotion_metric_policy.mat","egomotion_metric_policy");
    require(policy.hasValue()&&policy.value().info.dimensions==std::vector<std::size_t>{1,4}&&
            policy.value().values_column_major==std::vector<double>{1,1,1,static_cast<double>(mode.runs)},
            "Exact metric policy and planned-run identity");
    const auto manifest=textFile(sandbox.path/"manifest.json");
    double max_angle_error=0;
    for(std::size_t field=0;field<names.size();++field){
        const auto& name=names[field];
        const auto file=sandbox.path/(name+".mat");
        const auto decoded=io::MatResultReader{}.readNamed(file,name);
        require(decoded.hasValue(),"Cannot read published current-save field");
        const auto& data=decoded.value();
        const auto validity=io::MatResultReader{}.readNamed(sandbox.path/("valid_"+name+".mat"),"valid_"+name);
        require(validity.hasValue()&&validity.value().info.dimensions==data.info.dimensions,
                "Validity companion must match source shape");
        const auto columns=mode.kind==ExperimentKind::AdaptiveUpdateRates&&name!="akrot"?kRates:1;
        require(data.info.dimensions==std::vector<std::size_t>({mode.frames,columns}),"Output shape changed");
        for(std::size_t column=0;column<columns;++column)
            for(std::size_t row=0;row<mode.frames;++row){
                const double actual=data.values_column_major[row+mode.frames*column];
                const auto offset=row+mode.frames*column;
                const double count=validity.value().values_column_major[offset];
                if(row==0||row+1==mode.frames){require(actual==0.0&&count==0,"Source endpoint/invalid marker changed");continue;}
                const auto expected_count=fixed_expected&&!fixed_expected->undefined_metrics.empty()
                    ?fixed_expected->valid_counts.at(name)[offset]:(name=="rottransratio"?1:mode.runs);
                require(count==static_cast<double>(expected_count),"Per-cell valid repetition count");
                if(fixed_expected){
                    const double expected=fixed_expected->fields.at(name).values_column_major[row+mode.frames*column];
                    require(std::isnan(expected)?std::isnan(actual):actual==expected,
                            "Fixed 600-row aggregation/publication value differs");
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
    require(actual_mats==2*names.size()+1,"Runner wrote unexpected fields or lost validity metadata");
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
void sourceFailureCase(const fs::path& root,const std::string& sha,const Mode& mode,bool rng_failure) {
    RandomCounts counts;
    if(rng_failure)counts.fail_on_normal=7; // 第3条观测的x分量，保留0-based ray index=2。
    ScriptedStep selected(mode);
    constexpr std::size_t injected_failure_frame=152,prior_valid_frames=150;
    if(!rng_failure)selected.fail_on_call=prior_valid_frames*(mode.comparisons+1)+1;
    Sandbox sandbox(root,sha,mode,counts);
    experiments::ProductionExperimentRunner runner(&selected);
    const auto result=execute(runner,mode,*sandbox.context);
    const auto expected_code=rng_failure?ExperimentErrorCode::IncompleteRun:ExperimentErrorCode::SolverFailed;
    require(!result.hasValue()&&result.error().code==expected_code,
            "Partial run returned successful mean or mislabeled simulation failure as solver failure");
    require(result.error().detail.run_id&&
            result.error().detail.run_id->text==sandbox.context->runId().text,
            "Production failure must retain the allocated RunId before CLI serialization");
    const auto& draft=sandbox.context->manifestDraft();
    const auto diagnostic=jsonFailure(result.error(),5);
    std::cout<<"Observed failure: attempted="<<draft.attempted<<" completed="<<draft.completed
             <<" valid="<<draft.valid<<" undefined="<<draft.undefined
             <<" steps="<<selected.calls<<" draws="<<counts.normal<<'\n'<<diagnostic;
    require(sandbox.publisher->publishNew("failure-diagnostic.jsonl",diagnostic).hasValue(),
            "Cannot retain actual failure diagnosis");
    const auto expected_steps=prior_valid_frames*(mode.comparisons+1)+1;
    const auto expected_draws=mode.kind==ExperimentKind::AdaptiveComparison?
        (prior_valid_frames+1)*kLevelFourDirections*kAxes:0;
    require(draft.completed==0&&draft.valid==(rng_failure?0:prior_valid_frames)&&
            draft.attempted==1&&draft.status!=RunStatus::Complete,"Partial progress counts were lost or promoted");
    require(draft.undefined==0,"Zero rotation is valid; a solver failure is not a missing metric");
    require(selected.calls==(rng_failure?0:expected_steps)&&
            counts.normal==(rng_failure?counts.fail_on_normal:expected_draws)&&counts.uniform==0,
            "Failure did not stop at exact solver/RNG call");
    require(sandbox.writer->fileEvidence().empty()&&!fs::exists(sandbox.path/"manifest.json"),
            "Incomplete eight-run data reached Writer or Complete Manifest");
    require(!sandbox.context->publishAdaptiveOutcome().hasValue(),"Unfinished solver repetitions passed Adaptive publication gate");
    require(diagnostic.find(rng_failure?"InvalidRandomSample":"SingularSystem")!=std::string::npos&&
            diagnostic.find("run_matlab_1_based=1")!=std::string::npos&&
            diagnostic.find("frame_matlab_1_based="+std::to_string(rng_failure?2:injected_failure_frame))!=std::string::npos,
            "Error lost numerical cause or run/frame identity");
    if(!rng_failure){
        const auto reference=referenceTrajectory(mode);
        require(reference.motions[150].relative_rotation.norm()==0.0&&
                syntheticMotion(0,151,0).rotation().norm()>0,
                "Fixture must cross the formerly undefined zero-rotation frame before real solve failure");
    }
    require(sandbox.context->publishFailureRecord(result.error()).hasValue(),"Cannot persist failure audit");
    const auto manifest=textFile(sandbox.path/"manifest.json");
    require(manifest.find(rng_failure?"\"completeness\":\"Failed\"":"\"completeness\":\"Incomplete\"")!=std::string::npos&&
            manifest.find("\"expected_repetitions\":"+std::to_string(mode.runs)+",")!=std::string::npos&&
            manifest.find("\"undefined\":0,")!=std::string::npos&&
            manifest.find("\"mat_files\":[]")!=std::string::npos,"Failure audit changed divisor or invented MAT output");
    std::cout<<"PASS "<<(rng_failure?"RNG-failure":"zero-rotation-then-solver-failure")<<" steps="<<selected.calls
             <<" normal_draws="<<counts.normal<<" diagnostic="<<diagnostic;
}

AdaptiveData fixedRun(const Mode& mode,std::size_t one_based_run) {
    AdaptiveData data;data.frame_count=mode.frames;data.planned_runs=mode.runs;
    data.valid_frames=mode.frames-2;
    const auto names=expectedFields(mode);
    for(std::size_t field=0;field<names.size();++field){
        DenseDoubleArray array;
        array.info.name=names[field];array.info.original_class_type=6;array.info.original_data_type=9;
        array.info.class_name="double";array.info.data_type_name="double";
        const std::size_t columns=mode.kind==ExperimentKind::AdaptiveUpdateRates&&names[field]!="akrot"?kRates:1;
        array.info.dimensions={mode.frames,columns};array.info.data_bytes=mode.frames*columns*sizeof(double);
        array.values_column_major.assign(mode.frames*columns,0);
        auto& counts=data.valid_counts[names[field]];
        counts.assign(mode.frames*columns,0);
        for(std::size_t column=0;column<columns;++column)
            for(std::size_t row=1;row+1<mode.frames;++row) {
                array.values_column_major[row+mode.frames*column]=
                    static_cast<double>(4*one_based_run+2*field+column)+static_cast<double>(row)/1024.0;
                counts[row+mode.frames*column]=1;
            }
        data.fields.emplace(names[field],std::move(array));
    }
    return data;
}
AdaptiveData emptyAggregate(const Mode& mode) {
    auto data=fixedRun(mode,0);
    for(auto& field:data.fields)
        std::fill(field.second.values_column_major.begin(),field.second.values_column_major.end(),0.0);
    for(auto& field:data.valid_counts)std::fill(field.second.begin(),field.second.end(),0);
    data.valid_frames=0;
    return data;
}
void noteFixedRun(RunContext& context,const Mode& mode) {
    context.noteAttempt();
    for(std::size_t row=1;row+1<mode.frames;++row)context.noteValid();
    context.noteCompleted();
}
void requireUnchanged(const AdaptiveData& actual,const AdaptiveData& before) {
    require(actual.completed_runs==before.completed_runs&&actual.valid_counts==before.valid_counts&&
            actual.valid_frames==before.valid_frames&&actual.undefined_frames==before.undefined_frames&&
            actual.repetition_fields.size()==before.repetition_fields.size()&&
            actual.undefined_metrics.size()==before.undefined_metrics.size(),
            "Rejected run changed counters or retained repetition identity");
    for(const auto& field:before.fields)
        require(actual.fields.at(field.first).values_column_major==field.second.values_column_major,
                "Rejected run partially modified aggregate");
}
void fixedComparisonComplete(const fs::path& root,const std::string& sha) {
    RandomCounts counts;Sandbox sandbox(root,sha,kComparison,counts);
    auto aggregate=emptyAggregate(kComparison);
    for(std::size_t run=1;run<=kRuns;++run){
        const auto result=experiments::detail::accumulateCompleteAdaptiveRun(aggregate,fixedRun(kComparison,run));
        require(result.hasValue()&&aggregate.completed_runs==run,"Complete fixed run was not accumulated");
        noteFixedRun(*sandbox.context,kComparison);
        if(run<kRuns)
            require(aggregate.fields.at("rottransratio").values_column_major==std::vector<double>(kFrames,0),
                    "Earlier run leaked into last-run ratio");
    }
    const auto finished=experiments::detail::finishAdaptiveAggregation(std::move(aggregate),true);
    require(finished.hasValue(),"Eight fixed runs did not finalize");
    // 独立闭式期望：4*mean(1..8)=18；第五字段只取run8，即32+2*4=40。
    auto expected=emptyAggregate(kComparison);
    const auto names=expectedFields(kComparison);
    for(std::size_t field=0;field<names.size();++field){
        auto& values=expected.fields.at(names[field]).values_column_major;
        const double base=field==4?40.0:18.0+static_cast<double>(2*field);
        for(std::size_t row=1;row+1<kFrames;++row)values[row]=base+static_cast<double>(row)/1024.0;
        require(finished.value().fields.at(names[field]).values_column_major==values,
                "Eight-run mean or last-run ratio differs from closed-form fixed reference");
    }
    const auto outcome=experiments::detail::finishAdaptive(finished,*sandbox.context,kComparison.kind);
    require(outcome.hasValue()&&outcome.value().status==RunStatus::Complete&&
            outcome.value().completed==kRuns&&outcome.value().valid==kRuns*(kFrames-2),
            "Fixed aggregate did not pass actual production publication gate");
    checkOutput(sandbox,kComparison,&expected);
    require(counts.normal==0&&counts.uniform==0,"Fixed result aggregation consumed RNG");
    std::cout<<"PASS fixed eight-run aggregate: 8 distinct 600x1 arrays per field; last ratio=run8\n";
}
void fixedComparisonIncomplete(const fs::path& root,const std::string& sha) {
    // k=1..8缺失都必须失败，不允许以任何已有样本数替代分母8。
    for(std::size_t completed=0;completed<kRuns;++completed){
        RandomCounts counts;Sandbox sandbox(root,sha,kComparison,counts);
        auto aggregate=emptyAggregate(kComparison);
        for(std::size_t run=1;run<=completed;++run){
            require(experiments::detail::accumulateCompleteAdaptiveRun(aggregate,fixedRun(kComparison,run)).hasValue(),
                    "Fixed prefix failed");
            noteFixedRun(*sandbox.context,kComparison);
        }
        const auto before=aggregate;
        sandbox.context->noteAttempt(); // 下一轮没有可提交的完整结果。
        const auto finished=experiments::detail::finishAdaptiveAggregation(std::move(aggregate),true);
        require(!finished.hasValue()&&finished.error().code==ExperimentErrorCode::IncompleteRun,
                "Partial eight-run aggregate finalized");
        requireUnchanged(aggregate,before);
        const auto outcome=experiments::detail::finishAdaptive(finished,*sandbox.context,kComparison.kind);
        require(!outcome.hasValue()&&sandbox.writer->fileEvidence().empty()&&
                !sandbox.context->publishAdaptiveOutcome().hasValue(),"Partial aggregate reached MAT/Adaptive publication");
        require(sandbox.context->publishFailureRecord(outcome.error()).hasValue(),"Partial audit did not persist");
        const auto& draft=sandbox.context->manifestDraft();
        require(draft.completed==completed&&draft.attempted==completed+1&&draft.valid==completed*(kFrames-2)&&
                draft.failed==1&&draft.status==(completed==0?RunStatus::Failed:RunStatus::Incomplete),
                "Partial aggregate audit changed reliable progress counts");
        const auto manifest=textFile(sandbox.path/"manifest.json");
        require(manifest.find("\"expected_repetitions\":8,")!=std::string::npos&&
                manifest.find("\"mat_files\":[]")!=std::string::npos,"Partial audit hid original divisor or invented MATs");
    }
    auto aggregate=emptyAggregate(kComparison);
    for(std::size_t run=1;run<kRuns;++run)
        require(experiments::detail::accumulateCompleteAdaptiveRun(aggregate,fixedRun(kComparison,run)).hasValue(),
                "Fixed seven-run prefix failed");
    const auto before=aggregate;
    for(const std::string problem:{"missing-ratio","transposed","nonfinite-ratio"}){
        auto last=fixedRun(kComparison,kRuns);
        if(problem=="missing-ratio")last.fields.erase("rottransratio");
        if(problem=="transposed")last.fields.at("alleohnerotfehler").info.dimensions={1,kFrames};
        if(problem=="nonfinite-ratio")last.fields.at("rottransratio").values_column_major[1]=
            std::numeric_limits<double>::quiet_NaN();
        const auto status=experiments::detail::accumulateCompleteAdaptiveRun(aggregate,last);
        require(!status.hasValue()&&status.error().code==ExperimentErrorCode::DataMismatch,
                "Invalid eighth run was accepted");
        requireUnchanged(aggregate,before);
    }
    require(experiments::detail::accumulateCompleteAdaptiveRun(aggregate,fixedRun(kComparison,kRuns)).hasValue(),
            "Valid eighth run cannot be retried after rejected inputs");
    const auto full=aggregate;
    require(!experiments::detail::accumulateCompleteAdaptiveRun(aggregate,fixedRun(kComparison,kRuns+1)).hasValue(),
            "Unexpected ninth run changed the eight-run mean");
    requireUnchanged(aggregate,full);
    RandomCounts counts;Sandbox sandbox(root,sha,kComparison,counts);
    auto invalid=emptyAggregate(kComparison);invalid.planned_runs=0;
    const auto zero_runs=experiments::detail::finishAdaptive(
        MemoryResult::success(std::move(invalid)),*sandbox.context,kComparison.kind);
    require(!zero_runs.hasValue()&&zero_runs.error().code==ExperimentErrorCode::IncompleteRun&&
            sandbox.writer->fileEvidence().empty(),"Zero planned repetitions reached division/publication");
    require(sandbox.context->publishFailureRecord(zero_runs.error()).hasValue(),"Zero-count failure audit failed");
    std::cout<<"PASS fixed incomplete prefixes 0..7, invalid eighth ratio/shape, atomic retry and ninth-run rejection\n";
}
void fixedComparisonMetricGaps(const fs::path& root,const std::string& sha) {
    RandomCounts counts;Sandbox sandbox(root,sha,kComparison,counts);
    auto aggregate=emptyAggregate(kComparison);
    constexpr std::size_t translation_row=17,ratio_row=42;
    for(std::size_t run=1;run<=kRuns;++run) {
        auto one=fixedRun(kComparison,run);
        const bool missing_translation=run==3,missing_ratio=run==kRuns;
        if(missing_translation||missing_ratio) {
            const std::string name=missing_translation?"allemittransfehler":"rottransratio";
            const auto row=missing_translation?translation_row:ratio_row;
            one.fields.at(name).values_column_major[row]=std::numeric_limits<double>::quiet_NaN();
            one.valid_counts.at(name)[row]=0;
            ErrorDetail reason;reason.stage=ErrorStage::Solve;
            reason.message=missing_translation?"Fixed zero translation direction":"Fixed undefined ideal component ratio";
            one.undefined_metrics.push_back({name,run,row+1,1,
                {NumericalErrorCode::UndefinedAngle,std::move(reason)}});
            --one.valid_frames;++one.undefined_frames;
        }
        sandbox.context->noteAttempt();
        for(std::size_t row=1;row+1<kFrames;++row) {
            if((missing_translation&&row==translation_row)||(missing_ratio&&row==ratio_row))
                sandbox.context->noteUndefined();
            else sandbox.context->noteValid();
        }
        sandbox.context->noteCompleted();
        require(experiments::detail::accumulateCompleteAdaptiveRun(aggregate,one).hasValue(),
                "A complete solve with a marked metric gap must be retained");
    }
    auto finished=experiments::detail::finishAdaptiveAggregation(std::move(aggregate),true);
    require(finished.hasValue(),"Completed eight solves with gaps must form a partial-valid result");
    const auto& data=finished.value();
    require(data.completed_runs==8&&data.repetition_fields.size()==8&&data.undefined_metrics.size()==2,
            "Preserve all repetitions and exact missing reasons");
    require(data.valid_frames==8*(kFrames-2)-2&&data.undefined_frames==2,
            "Frame validity ledger must not discard missing observations");
    require(data.valid_counts.at("allemittransfehler")[translation_row]==7&&
            std::isnan(data.fields.at("allemittransfehler").values_column_major[translation_row]),
            "Seven valid values must not be divided by seven or by eight to fabricate an average");
    require(data.valid_counts.at("rottransratio")[ratio_row]==0&&
            std::isnan(data.fields.at("rottransratio").values_column_major[ratio_row]),
            "Missing final-run ratio must not fall back to any earlier run");
    for(std::size_t run=1;run<=8;++run) {
        const auto& raw=data.repetition_fields[run-1].at("allemittransfehler").values_column_major;
        require(run==3?std::isnan(raw[translation_row]):
                raw[translation_row]==4*static_cast<double>(run)+static_cast<double>(translation_row)/1024,
                "All raw valid and missing per-run values must remain at their source indices");
    }
    require(data.fields.at("allemittransfehler").values_column_major[translation_row+1]==
            18.0+static_cast<double>(translation_row+1)/1024,
            "Neighbouring fully valid cell keeps the original eight-run average");
    const auto outcome=experiments::detail::finishAdaptive(finished,*sandbox.context,kComparison.kind);
    require(outcome.hasValue()&&outcome.value().status==RunStatus::Incomplete&&
            outcome.value().completed==8&&outcome.value().failed==0&&outcome.value().undefined==2,
            "Metric gaps yield a delivered Incomplete outcome without converting solves into failures");
    checkOutput(sandbox,kComparison,&data);
    const auto manifest=textFile(sandbox.path/"manifest.json");
    require(manifest.find("egomotion-run-manifest-v2")!=std::string::npos&&
            manifest.find("\"completeness\":\"Incomplete\"")!=std::string::npos&&
            manifest.find("Fixed zero translation direction")!=std::string::npos&&
            manifest.find("Fixed undefined ideal component ratio")!=std::string::npos&&
            manifest.find("null")!=std::string::npos&&manifest.find(":NaN")==std::string::npos&&
            manifest.find("[NaN")==std::string::npos&&manifest.find(",NaN")==std::string::npos&&
            manifest.find(":nan")==std::string::npos&&manifest.find("[nan")==std::string::npos&&
            manifest.find(",nan")==std::string::npos,
            "Versioned Manifest must preserve missing values and reasons using valid JSON");
}
void fixedUpdateSession(const fs::path& root,const std::string& sha) {
    RandomCounts counts;Sandbox sandbox(root,sha,kUpdateRates,counts);
    ScriptedStep step(kUpdateRates);
    estimators::AdaptiveMfaSession session(estimators::AdaptiveSessionMode::UpdateRates,&step);
    auto data=fixedRun(kUpdateRates,0);
    const Vector3 true_translation(1,2,3),true_rotation(1,-2,3);
    for(std::size_t row=1;row+1<kFrames;++row){
        estimators::AdaptiveFrameInput input;
        input.matlab_frame_index=row+1;input.source_coefficient_rotation=Matrix3::Identity();
        input.truth=test_support::motionValue(true_translation,true_rotation);
        input.observations=test_support::flowValue(FlowMatrix::Zero(kLevelTwoDirections,6));
        const auto frame=session.processFrame(input);
        require(frame.hasValue(),"Fixed nonzero-truth UpdateRates Session failed");
        for(std::size_t column=0;column<kRates;++column){
            const auto motion=syntheticMotion(0,row+1,column);
            const auto& translation=frame.value().comparison_translation_error_deg[column];
            const auto& rotation=frame.value().comparison_rotation_error_deg[column];
            require(translation.hasValue()&&rotation.hasValue(),"Fixed Session metrics must be valid");
            const double t=translation.value(),r=rotation.value();
            require(std::abs(t-angleDegrees(true_translation,motion.translation()))<=kContractAngleTolerance&&
                    std::abs(r-rotationErrorDegrees(true_rotation,motion.rotation()))<=kContractAngleTolerance,
                    "Full 600-frame fixed Session angle/column changed");
            data.fields.at("allemittransfehler").values_column_major[row+kFrames*column]=t;
            data.fields.at("allemitrotfehler").values_column_major[row+kFrames*column]=r;
        }
        data.fields.at("akrot").values_column_major[row]=true_rotation.norm()*180.0/std::acos(-1.0);
    }
    require(step.calls==(kFrames-2)*(kRates+1),"Full fixed UpdateRates Session skipped frames");
    const auto expected=data;
    auto aggregate=emptyAggregate(kUpdateRates);
    require(experiments::detail::accumulateCompleteAdaptiveRun(aggregate,data).hasValue(),"Fixed update result failed");
    noteFixedRun(*sandbox.context,kUpdateRates);
    const auto finished=experiments::detail::finishAdaptiveAggregation(std::move(aggregate),false);
    const auto outcome=experiments::detail::finishAdaptive(finished,*sandbox.context,kUpdateRates.kind);
    require(outcome.hasValue(),"Fixed UpdateRates publication failed");
    checkOutput(sandbox,kUpdateRates,&expected);
    std::cout<<"PASS fixed nonzero-truth 600-frame Session steps="<<step.calls<<" (not original trajectory)\n";
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
            record("initialization",[&]{completeCase(root,sha,kInitialization);});
            record("fixed-eight-run-aggregate",[&]{fixedComparisonComplete(root,sha);});
            record("fixed-update-session",[&]{fixedUpdateSession(root,sha);});
        }else if(mode=="incomplete"){
            record("fixed-incomplete-aggregate",[&]{fixedComparisonIncomplete(root,sha);});
            record("fixed-metric-gaps",[&]{fixedComparisonMetricGaps(root,sha);});
            record("RNG-failure",[&]{sourceFailureCase(root,sha,kComparison,true);});
        }else if(mode=="undefined"||mode=="zero-rotation"){
            for(const auto& selected:{kComparison,kUpdateRates})
                record(selected.name,[&]{sourceFailureCase(root,sha,selected,false);});
        }else{
            require(false,"Unknown test mode");
        }
        return failures==0?0:1;
    }catch(const std::exception& error){std::cerr<<"FAIL Adaptive Runner contract: "<<error.what()<<'\n';return 1;}
}
