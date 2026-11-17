// R6：原 transfor_koeff 的系数动作与物理场动作分别验证。
// 独立参考采用已审阅 IR96 Table 3 的显式二次表，不调用生产递推/张量。
// 来源：verification/harmonics/verify_low_order_layout.wl（一般 SO(3) 符号核对）。
// 不运行 MATLAB，不将替身 Session 称为科学实验或批准 Golden。
#include "egomotion/estimators/adaptive_session.hpp"
#include "egomotion/harmonics/adapter.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
using namespace egomotion;
using namespace egomotion::estimators;
using Matrix5=Eigen::Matrix<double,5,5>;
// 沿用既有球谐专项比较余量；不是 DEC-005 的最终验收阈值。
constexpr double kTolerance=4096.0*std::numeric_limits<double>::epsilon();
constexpr std::array<std::size_t,4> kPeriods{{1,5,11,21}};
std::size_t checks=0,failures=0;
double max_residual=0.0;

void check(bool ok,const std::string& context) {
    ++checks;
    if(!ok) {
        ++failures;
        if(failures<=20)std::cerr<<"FAIL "<<context<<'\n';
    }
}
void near(double actual,double expected,const std::string& context) {
    const double error=std::abs(actual-expected);
    if(std::isfinite(error))max_residual=std::max(max_residual,error);
    check(std::isfinite(actual)&&std::isfinite(expected)&&error<=kTolerance,
          context+" actual="+std::to_string(actual)+" expected="+std::to_string(expected));
}
void same(const HarmonicCoefficients& actual,const HarmonicCoefficients& expected,
          const std::string& context) {
    for(int row=0;row<3;++row)for(int col=0;col<5;++col)
        near(actual.values(row,col),expected.values(row,col),
             context+" C("+std::to_string(row+1)+","+std::to_string(col+1)+")");
}

struct ReferenceBlocks { Matrix3 one;Matrix5 two; };
ReferenceBlocks sourceBlocks(const Matrix3& r) {
    ReferenceBlocks out;
    // SHRotate.m:33–43；m=-1,0,1，列系数左乘，不能把 r 当成此块。
    out.one<<r(1,1),r(2,1),r(0,1),r(1,2),r(2,2),r(0,2),r(1,0),r(2,0),r(0,0);
    // a..i 是上述一阶块的逐行符号，与独立 Wolfram 显式表一致。
    const double a=out.one(0,0),b=out.one(0,1),c=out.one(0,2);
    const double d=out.one(1,0),e=out.one(1,1),f=out.one(1,2);
    const double g=out.one(2,0),h=out.one(2,1),i=out.one(2,2);
    const double root_three=std::sqrt(3.0);
    out.two<<i*a+g*c,g*b+h*a,root_three*h*b,i*b+h*c,i*c-g*a,
        c*d+a*f,a*e+b*d,root_three*b*e,c*e+b*f,c*f-a*d,
        root_three*f*d,root_three*d*e,(3*e*e-1)/2,root_three*f*e,
            root_three*(f*f-d*d)/2,
        i*d+g*f,g*e+h*d,root_three*h*e,i*e+h*f,i*f-g*d,
        i*g-c*a,g*h-a*b,root_three*(h*h-b*b)/2,i*h-c*b,
            (i*i-g*g-c*c+a*a)/2;
    return out;
}
HarmonicCoefficients sourceAction(const HarmonicCoefficients& old,const Matrix3& r) {
    const auto blocks=sourceBlocks(r);
    HarmonicCoefficients out;
    out.values.setZero();
    out.values(0,0)=old.values(0,0);
    out.values.row(1).head<3>()=(blocks.one*old.values.row(1).head<3>().transpose()).transpose();
    out.values.row(2)=(blocks.two*old.values.row(2).transpose()).transpose();
    return out;
}
HarmonicCoefficients phase(HarmonicCoefficients c) {
    c.values(1,0)=-c.values(1,0);c.values(1,2)=-c.values(1,2);
    c.values(2,1)=-c.values(2,1);c.values(2,3)=-c.values(2,3);
    return c;
}
double sourceField(const HarmonicCoefficients& c,const Vector3& v) {
    // kugelflfunc2 的九个 Cartesian 多项式；不调用被测重建器。
    const double pi=std::acos(-1.0),x=v.x(),y=v.y(),z=v.z();
    return c.values(0,0)/std::sqrt(4*pi)+std::sqrt(3/(4*pi))*
        (-c.values(1,0)*y+c.values(1,1)*z-c.values(1,2)*x)+
        std::sqrt(15/(4*pi))*(c.values(2,0)*x*y-c.values(2,1)*y*z-
            c.values(2,3)*x*z)+std::sqrt(5/(16*pi))*c.values(2,2)*(3*z*z-1)+
        std::sqrt(15/(16*pi))*c.values(2,4)*(x*x-y*y);
}
HarmonicCoefficients asymmetric() {
    HarmonicCoefficients c;
    c.values<<2,0,0,0,0, 0.125,-0.25,0.375,0,0, -0.125,0.25,0.5,-0.375,0.625;
    return c;
}
std::vector<Matrix3> rotations() {
    Matrix3 x,y,z,mixed;
    x<<1,0,0,0,0,-1,0,1,0;
    y<<0,0,1,0,1,0,-1,0,0;
    z<<0,-1,0,1,0,0,0,0,1;
    // 独立有理旋转 q=(1,2,2,0)，固定参考已保存于 verification_results。
    mixed<<1,8,4,8,1,-4,-4,4,-7;mixed/=9.0;
    return {Matrix3::Identity(),x,y,z,x.transpose(),y.transpose(),
            mixed,mixed.transpose(),x*y,y*x};
}
std::vector<Vector3> directions() {
    return {Vector3::UnitX(),Vector3::UnitY(),Vector3::UnitZ(),
        -Vector3::UnitX(),-Vector3::UnitY(),-Vector3::UnitZ(),
        Vector3(3.0/5,4.0/5,0),Vector3(0,3.0/5,4.0/5),
        Vector3(1.0/3,2.0/3,2.0/3)};
}

void verifyAdapter() {
    const std::array<std::array<int,2>,9> positions{{
        {{0,0}},{{1,0}},{{1,1}},{{1,2}},{{2,0}},{{2,1}},{{2,2}},{{2,3}},{{2,4}}}};
    Matrix3 coordinate_phase=Matrix3::Identity();
    coordinate_phase(0,0)=coordinate_phase(1,1)=-1;
    std::size_t index=0;
    for(const auto& r:rotations()) {
        for(std::size_t column=0;column<=positions.size();++column) {
            HarmonicCoefficients old=asymmetric();
            if(column<positions.size()) {
                old.values.setZero();old.values(positions[column][0],positions[column][1])=1;
            }
            const std::string label="rotation="+std::to_string(index)+" column="+std::to_string(column);
            const auto raw=sourceAction(old,r);
            const auto physical=harmonics::rotateCoefficients(old,r);
            if(!physical.hasValue())throw std::runtime_error("Physical adapter unexpectedly failed");
            same(physical.value(),phase(sourceAction(phase(old),r)),label+" physical phase");
            for(const auto& v:directions()) {
                near(sourceField(physical.value(),v),sourceField(old,r*v),label+" physical field");
                near(sourceField(raw,v),sourceField(old,coordinate_phase*r*coordinate_phase*v),
                     label+" original field");
            }
        }
        ++index;
    }
    HarmonicCoefficients positive;
    positive.values.setZero();
    positive.values(0,0)=2*std::sqrt(4*std::acos(-1.0));
    positive.values(1,1)=std::sqrt(4*std::acos(-1.0)/3);
    const Matrix3 x=rotations()[1];
    const auto physical=harmonics::rotateCoefficients(positive,x);
    if(!physical.hasValue())throw std::runtime_error("Counterexample rotation failed");
    near(sourceField(sourceAction(positive,x),Vector3::UnitY()),1,"raw 2+z at y");
    near(sourceField(physical.value(),Vector3::UnitY()),3,"physical 2+z at y");
    std::cout<<"Independent source/physical counterexample: raw=1 physical=3\n";
}

struct TraceSolver final:IAdaptiveStepSolver {
    mutable std::vector<HarmonicCoefficients> received;
    std::optional<std::size_t> fail_call;
    Result<AdaptiveStepResult,NumericalErrorCode> solve(
        const HarmonicCoefficients& old,const FlowField&) const override {
        received.push_back(old);
        if(fail_call&&received.size()==*fail_call) {
            ErrorDetail detail;detail.stage=ErrorStage::Solve;detail.message="Synthetic SH rollback failure";
            return Result<AdaptiveStepResult,NumericalErrorCode>::failure(
                {NumericalErrorCode::SingularSystem,std::move(detail)});
        }
        AdaptiveStepResult out;
        out.motion.translation=Vector3::UnitX();out.motion.rotation=Vector3::UnitY();
        out.error_mse=0;out.coefficients_next=old;
        out.coefficients_next.values+=asymmetric().values;
        return Result<AdaptiveStepResult,NumericalErrorCode>::success(std::move(out));
    }
};
AdaptiveFrameInput frame(std::size_t index) {
    AdaptiveFrameInput in;
    in.matlab_frame_index=index;
    in.coefficient_argument_rotation=rotations()[(index-2)%rotations().size()];
    in.truth.translation=Vector3::UnitX();in.truth.rotation=Vector3::UnitY();
    in.observations.observations.resize(0,6);
    return in;
}
HarmonicCoefficients initial() {
    HarmonicCoefficients c;c.values.setZero();c.values(0,0)=1;return c;
}
struct Model {
    HarmonicCoefficients primary=initial();
    std::array<HarmonicCoefficients,4> baselines{{initial(),initial(),initial(),initial()}};
};
void successfulFrame(AdaptiveMfaSession& session,TraceSolver& solver,Model& model,
                     AdaptiveSessionMode mode,std::size_t index) {
    const auto input=frame(index);
    if(mode!=AdaptiveSessionMode::Initialization)
        model.primary=sourceAction(model.primary,input.coefficient_argument_rotation);
    if(mode==AdaptiveSessionMode::UpdateRates)for(std::size_t j=0;j<kPeriods.size();++j) {
        model.baselines[j]=sourceAction(model.baselines[j],input.coefficient_argument_rotation);
        if(index%kPeriods[j]==0)model.baselines[j]=model.primary;
    }
    const std::size_t before=solver.received.size();
    const std::size_t comparisons=mode==AdaptiveSessionMode::UpdateRates?4:2;
    const auto result=session.processFrame(input);
    if(!result.hasValue())throw std::runtime_error("Synthetic SH Session frame failed");
    check(solver.received.size()==before+comparisons+1,"Exact solve call count");
    const std::string label="mode="+std::to_string(static_cast<int>(mode))+" frame="+std::to_string(index);
    for(std::size_t j=0;j<comparisons;++j) {
        const auto expected=mode==AdaptiveSessionMode::UpdateRates?model.baselines[j]:
            (j==0?initial():model.primary);
        same(solver.received.at(before+j),expected,label+" comparator="+std::to_string(j));
    }
    same(solver.received.at(before+comparisons),model.primary,label+" primary input");
    model.primary.values+=asymmetric().values;
    same(session.primaryCoefficients(),model.primary,label+" primary commit");
    for(std::size_t j=0;j<kPeriods.size();++j)
        same(session.updateRateBaselines()[j],model.baselines[j],label+" baseline commit");
}
void verifySession() {
    for(const auto mode:{AdaptiveSessionMode::Initialization,AdaptiveSessionMode::Comparison,
                         AdaptiveSessionMode::UpdateRates}) {
        TraceSolver solver;AdaptiveMfaSession session(mode,&solver);Model model;
        // 覆盖全部刷新周期和非交换累计旋转，不以常数系数掩盖相位差。
        for(std::size_t index=2;index<=23;++index)successfulFrame(session,solver,model,mode,index);
    }
    for(const std::size_t failed_offset:{2u,5u}) {
        TraceSolver solver;AdaptiveMfaSession session(AdaptiveSessionMode::UpdateRates,&solver);Model model;
        for(std::size_t index=2;index<5;++index)
            successfulFrame(session,solver,model,AdaptiveSessionMode::UpdateRates,index);
        const auto old_primary=session.primaryCoefficients();
        const auto old_baselines=session.updateRateBaselines();
        solver.fail_call=solver.received.size()+failed_offset;
        const auto result=session.processFrame(frame(5));
        check(!result.hasValue()&&result.error().code==ExperimentErrorCode::SolverFailed,
              "Injected source rotation comparison/primary failure");
        same(session.primaryCoefficients(),old_primary,"Rollback primary");
        for(std::size_t j=0;j<kPeriods.size();++j)
            same(session.updateRateBaselines()[j],old_baselines[j],"Rollback baseline");
        solver.fail_call.reset();
        successfulFrame(session,solver,model,AdaptiveSessionMode::UpdateRates,5);
    }
}
} // namespace

int main(int argc,char** argv) {
    try {
        if(argc!=2)throw std::runtime_error("Expected adapter or session mode");
        const std::string mode=argv[1];
        if(mode=="adapter")verifyAdapter();
        else if(mode=="session")verifySession();
        else throw std::runtime_error("Unknown mode");
        std::cout<<std::setprecision(17)<<"Source SH "<<mode<<" checks="<<checks
                 <<" failed="<<failures<<" max_absolute_residual="<<max_residual
                 <<" comparison_allowance="<<kTolerance<<'\n';
        return failures==0?0:1;
    } catch(const std::exception& error) {
        std::cerr<<error.what()<<'\n';return 1;
    }
}
