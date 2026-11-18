// 真实初始化失败取证：装饰器只记录并委托生产Step，不替换其数值结果。
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
struct RecordingStep final:estimators::IAdaptiveStepSolver {
    estimators::AdaptiveMfaStepSolver actual;
    mutable std::size_t calls=0;
    mutable HarmonicCoefficients old;
    mutable FlowField flow;
    mutable std::optional<estimators::AdaptiveStepResult> last;
    Result<estimators::AdaptiveStepResult,NumericalErrorCode> solve(
        const HarmonicCoefficients& previous,const FlowField& observations)const override {
        ++calls;
        auto result=actual.solve(previous,observations);
        require(result.hasValue(),"Unexpected real Step failure before angle comparison");
        old=previous;flow=observations;last=result.value();
        return result;
    }
};
template<class Matrix>
void matrixCsv(std::ostream& out,const char* name,const Matrix& matrix) {
    for(Eigen::Index col=0;col<matrix.cols();++col)for(Eigen::Index row=0;row<matrix.rows();++row)
        out<<name<<','<<row+1<<','<<col+1<<','<<matrix(row,col)<<'\n';
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
        std::size_t started=0,valid=0,completed=0;
        const auto result=runner.computeInitialization({},[&](experiments::AdaptiveProgressEvent event) {
            if(event==experiments::AdaptiveProgressEvent::RunStarted)++started;
            if(event==experiments::AdaptiveProgressEvent::FrameCompleted)++valid;
            if(event==experiments::AdaptiveProgressEvent::RunCompleted)++completed;
        });
        require(!result.hasValue()&&result.error().code==ExperimentErrorCode::SolverFailed,
                "Actual initializer unexpectedly completed");
        std::cout<<jsonFailure(result.error(),5)<<'\n';
        require(started==1&&valid==12&&completed==0&&recorded.last.has_value(),"Failure prefix changed");
        const std::size_t frame=valid+2,slot=recorded.calls-valid*3-1;
        require(frame==14&&slot<2,"Failed call is not a comparison on MATLAB frame14");
        const auto points=trajectory::TrajectoryGenerator{}.circle(35,0.6,0.3);
        require(points.hasValue(),"Cannot recover source trajectory");
        const auto motions=trajectory::TrajectoryKinematics{}.fromPoints(points.value());
        require(motions.hasValue(),"Cannot recover source kinematics");
        const auto& truth=motions.value()[frame-1].local_translation;
        const auto& estimate=recorded.last->motion.translation();
        const double binary64=scalarCosine(truth,estimate);
        const long double wide=wideCosine(truth,estimate);
        const auto error=source_metrics::sourceDirectionErrorDegrees(truth,estimate);
        std::ofstream csv(report);require(csv.is_open(),"Cannot save last real solve input/output");
        csv<<"field,matlab_row,matlab_column,value\n"<<std::setprecision(21);
        matrixCsv(csv,"truth_translation",truth);matrixCsv(csv,"estimated_translation",estimate);
        matrixCsv(csv,"old_coefficients",recorded.old.values());matrixCsv(csv,"observations",recorded.flow.observations);
        matrixCsv(csv,"system",recorded.last->trace.system_matrix);matrixCsv(csv,"rhs",recorded.last->trace.rhs);
        matrixCsv(csv,"solved_motion",recorded.last->trace.solved_motion);
        csv.close();require(!csv.fail(),"Trace CSV write failed");
        std::cout<<std::setprecision(21)<<"frame="<<frame<<" comparison_slot_0_based="<<slot
                 <<" real_steps="<<recorded.calls<<" valid="<<valid<<" scalar_binary64_cosine="<<binary64
                 <<" wide_cosine="<<wide<<'\n';
        std::cout<<"truth="<<truth.transpose()<<" estimate="<<estimate.transpose()<<'\n';
        require(truth.allFinite()&&estimate.allFinite()&&truth.norm()>0&&estimate.norm()>0,
                "This diagnostic is specifically nonzero finite motion");
        require(binary64>1,"Independent scalar normalization did not reproduce acos overflow");
        require(!error.hasValue()&&error.error().code==NumericalErrorCode::UndefinedAngle&&
                error.error().detail.message=="normrelfehler acos argument outside [-1,1]",
                "Real metric rejection not explained by retained vectors");
        const auto replay=recorded.actual.solve(recorded.old,recorded.flow);
        require(replay.hasValue()&&std::memcmp(replay.value().motion.translation().data(),estimate.data(),3*sizeof(double))==0,
                "Retained last real solve is not bitwise replayable");
        std::cout<<"PASS real initialization trace; finite nonzero vectors, no clamping or threshold change\n";
        return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
