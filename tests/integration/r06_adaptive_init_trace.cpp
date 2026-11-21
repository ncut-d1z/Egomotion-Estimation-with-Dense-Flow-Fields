// 真实初始化边界回归：装饰器只记录并委托生产Step，不替换其数值结果。
// 保留历史 frame14 的两项比较输入，旧 acos 的余弦只作诊断，不固定其舍入符号。
#include "../support/adaptive_scalar_reference.hpp"
#include "egomotion/experiments/adaptive_runner.hpp"
#include "egomotion/estimators/adaptive_step.hpp"
#include "egomotion/sampling/directions.hpp"
#include "egomotion/simulation/flow.hpp"
#include "egomotion/trajectory/trajectory.hpp"
#include "egomotion/contracts/diagnostics.hpp"
#include "numerics/source_motion_errors.hpp"
#include <array>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <stdexcept>

namespace {
using namespace egomotion;
namespace fs=std::filesystem;
void require(bool ok,const std::string& why){if(!ok)throw std::runtime_error(why);}
constexpr std::size_t kRecordedFrame=14,kComparisons=2,kCallsPerFrame=3;
constexpr long double kContractAngleTolerance=1e-9L; // 已有合同回归余量，非 DEC-005 科学阈值。
struct StepRecord {
    HarmonicCoefficients old;
    FlowField flow;
    estimators::AdaptiveStepResult result;
};
struct RecordingStep final:estimators::IAdaptiveStepSolver {
    estimators::AdaptiveMfaStepSolver actual;
    mutable std::size_t calls=0;
    mutable std::array<std::optional<StepRecord>,kComparisons> recorded;
    Result<estimators::AdaptiveStepResult,NumericalErrorCode> solve(
        const HarmonicCoefficients& previous,const FlowField& observations)const override {
        const auto ordinal=calls++;
        auto result=actual.solve(previous,observations);
        const auto frame=ordinal/kCallsPerFrame+2,slot=ordinal%kCallsPerFrame;
        if(frame==kRecordedFrame&&slot<kComparisons&&result.hasValue())
            recorded[slot]=StepRecord{previous,observations,result.value()};
        return result;
    }
};
template<class Matrix>
void matrixCsv(std::ostream& out,std::size_t slot,const char* name,const Matrix& matrix) {
    for(Eigen::Index col=0;col<matrix.cols();++col)for(Eigen::Index row=0;row<matrix.rows();++row)
        out<<kRecordedFrame<<','<<slot<<','<<name<<','<<row+1<<','<<col+1<<','<<matrix(row,col)<<'\n';
}
double scalarCosine(const Vector3& a,const Vector3& b) {
    const std::array<double,3> x{{a.x(),a.y(),a.z()}},y{{b.x(),b.y(),b.z()}};
    const double nx=std::sqrt((x[0]*x[0]+x[1]*x[1])+x[2]*x[2]);
    const double ny=std::sqrt((y[0]*y[0]+y[1]*y[1])+y[2]*y[2]);
    return ((x[0]/nx)*(y[0]/ny)+(x[1]/nx)*(y[1]/ny))+(x[2]/nx)*(y[2]/ny);
}
long double wideCosine(const Vector3& a,const Vector3& b) {
    long double aa=0,bb=0,ab=0;
    for(int i=0;i<3;++i) {
        const long double x=a[i],y=b[i];aa+=x*x;bb+=y*y;ab+=x*y;
    }
    return ab/(std::sqrt(aa)*std::sqrt(bb));
}
void checkRecordedComparison(const RecordingStep& step,std::size_t slot,
                             const Vector3& truth,std::ostream& csv) {
    require(step.recorded[slot].has_value(),"Original frame14 comparison input was not reached");
    const auto& saved=*step.recorded[slot];
    const auto& estimate=saved.result.motion.translation();
    matrixCsv(csv,slot,"truth_translation",truth);matrixCsv(csv,slot,"estimated_translation",estimate);
    matrixCsv(csv,slot,"old_coefficients",saved.old.values());matrixCsv(csv,slot,"observations",saved.flow.observations());
    matrixCsv(csv,slot,"system",saved.result.trace.system_matrix);matrixCsv(csv,slot,"rhs",saved.result.trace.rhs);
    matrixCsv(csv,slot,"solved_motion",saved.result.trace.solved_motion);
    require(truth.allFinite()&&estimate.allFinite()&&truth.norm()>0&&estimate.norm()>0,
            "Boundary regression requires finite nonzero translation directions");
    const auto metric=source_metrics::sourceDirectionErrorDegrees(truth,estimate);
    const adaptive_reference::Vec a{{truth.x(),truth.y(),truth.z()}},
                                  b{{estimate.x(),estimate.y(),estimate.z()}};
    const long double expected=adaptive_reference::angleDegrees(a,b);
    require(metric.hasValue()&&std::isfinite(metric.value())&&
            std::abs(static_cast<long double>(metric.value())-expected)<=kContractAngleTolerance,
            "The retained boundary directions must have the independent atan2 angle");
    const auto replay=step.actual.solve(saved.old,saved.flow);
    require(replay.hasValue()&&
            std::memcmp(replay.value().motion.translation().data(),estimate.data(),3*sizeof(double))==0,
            "Retained real Step translation is not bitwise replayable");
    std::cout<<std::setprecision(21)<<"frame="<<kRecordedFrame<<" comparison_slot_0_based="<<slot
             <<" scalar_binary64_cosine="<<scalarCosine(truth,estimate)
             <<" wide_cosine="<<wideCosine(truth,estimate)
             <<" atan2_angle_degrees="<<metric.value()<<" independent_angle_degrees="<<expected<<'\n';
}
} // namespace
int main(int argc,char** argv) {
    try {
        require(argc==2,"Expected new trace CSV path");
        const fs::path report=argv[1];require(!fs::exists(report),"Refuse to overwrite trace evidence");
        RecordingStep recorded;
        sampling::IcosahedralSampler sampler;
        simulation::MeanMagnitudeGaussianNoise noise;
        trajectory::TrajectoryGenerator generator;
        trajectory::TrajectoryKinematics kinematics;
        trajectory::SphereEnvironment environment;
        experiments::AdaptiveExperimentRunner runner(
            {sampler,noise},{generator,kinematics,environment},&recorded);
        std::size_t started=0,valid=0,undefined=0,completed=0;
        const auto result=runner.computeInitialization({},[&](experiments::AdaptiveProgressEvent event) {
            if(event==experiments::AdaptiveProgressEvent::RunStarted)++started;
            if(event==experiments::AdaptiveProgressEvent::FrameCompleted)++valid;
            if(event==experiments::AdaptiveProgressEvent::FrameWithUndefinedMetrics)++undefined;
            if(event==experiments::AdaptiveProgressEvent::RunCompleted)++completed;
        });
        if(!result.hasValue())std::cerr<<jsonFailure(result.error(),5)<<'\n';
        const auto points=trajectory::TrajectoryGenerator{}.circle(35,0.6,0.3);
        require(points.hasValue(),"Cannot recover source trajectory");
        const auto motions=trajectory::TrajectoryKinematics{}.fromPoints(points.value());
        require(motions.hasValue(),"Cannot recover source kinematics");
        const auto& truth=motions.value()[kRecordedFrame-1].local_translation;
        std::ofstream csv(report);require(csv.is_open(),"Cannot save retained real Step inputs/outputs");
        csv<<"matlab_frame,comparison_slot_0_based,field,matlab_row,matlab_column,value\n"<<std::setprecision(21);
        for(std::size_t slot=0;slot<kComparisons;++slot)checkRecordedComparison(recorded,slot,truth,csv);
        csv.close();require(!csv.fail(),"Trace CSV write failed");
        require(result.hasValue(),"A true solver failure still blocks successful initialization; retained diagnostics explain it");
        require(started==1&&completed==1&&valid+undefined==33&&recorded.calls==33*kCallsPerFrame&&
                result.value().completed_runs==1&&result.value().valid_frames==valid&&
                result.value().undefined_frames==undefined,"Initializer must retain every original interior frame");
        std::cout<<"PASS real initialization boundary trace; real_steps="<<recorded.calls
                 <<" valid_frames="<<valid<<" undefined_frames="<<undefined<<'\n';
        return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
