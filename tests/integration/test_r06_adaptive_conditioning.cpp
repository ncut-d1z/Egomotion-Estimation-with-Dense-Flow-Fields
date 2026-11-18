// 参考：verification/adaptive/zero_mean_conditioning_evidence.v1.json
// blob be9aa64cb454fb434b407814f8cfa1310a7c90b7；独立 Wolfram 解析式，非 MATLAB 执行。
#include "egomotion/estimators/adaptive_step.hpp"
#include "estimators/adaptive_motion.hpp"

#include <Eigen/LU>
#include <Eigen/SVD>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {
using namespace egomotion;
using namespace egomotion::estimators;
using Triple=std::array<double,3>;
using ReferenceVector=std::array<long double,6>;
using ReferenceMatrix=std::array<ReferenceVector,6>;
// 沿用 test_r06_adaptive_l3.cpp；不是新增 DEC-005/病态判定门限。
constexpr long double kRegressionAllowance=2e-8L;
constexpr double kNearDenominatorEpsilon=1e-11;
const double kPi=std::acos(-1.0);
int checks=0,failures=0;
void check(bool pass,const std::string& why) {
    ++checks;
    if (!pass) { ++failures; std::cerr<<why<<'\n'; }
}
void near(long double actual,long double expected,const std::string& why) {
    check(std::isfinite(actual)&&std::isfinite(expected)&&
          std::abs(actual-expected)<=kRegressionAllowance*(1+std::abs(expected)),why);
}
struct Fixture {
    std::string name;
    std::vector<Triple> directions;
    std::vector<double> raw_near;
    HarmonicCoefficients coefficients;
    Triple translation{{1,0,0}},rotation{{1.0/5,-1.0/7,1.0/11}};
    double mean=1;
    ReferenceMatrix matrix{};
    ReferenceVector rhs{},solution{},singular_values{};
    long double condition=1;
    bool check_forward_solution=true;
};
HarmonicCoefficients coefficients(double a,double b) {
    HarmonicCoefficients result;
    result.values.setZero();
    result.values(0,0)=a*std::sqrt(4*kPi);
    result.values(1,0)=-b*std::sqrt(4*kPi/3);
    return result;
}
FlowField flow(const Fixture& fixture) {
    FlowField result;
    result.observations.resize(static_cast<Eigen::Index>(fixture.directions.size()),6);
    for (std::size_t row=0;row<fixture.directions.size();++row) {
        const auto& d=fixture.directions[row];
        double projection=0;
        for (std::size_t i=0;i<3;++i) projection+=fixture.translation[i]*d[i];
        for (std::size_t i=0;i<3;++i) {
            const double cross=fixture.rotation[(i+1)%3]*d[(i+2)%3]-
                               fixture.rotation[(i+2)%3]*d[(i+1)%3];
            result.observations(static_cast<Eigen::Index>(row),static_cast<Eigen::Index>(i))=d[i];
            result.observations(static_cast<Eigen::Index>(row),static_cast<Eigen::Index>(i+3))=
                -fixture.raw_near[row]*(fixture.translation[i]-projection*d[i])-cross;
        }
    }
    return result;
}
Fixture axes(double a,double b,bool no_rotation=false) {
    Fixture f;
    f.name="axes a="+std::to_string(a)+" b="+std::to_string(b);
    f.directions={{{1,0,0}},{{-1,0,0}},{{0,1,0}},{{0,-1,0}},{{0,0,1}},{{0,0,-1}}};
    f.coefficients=coefficients(a,b);
    f.mean=a;
    if (no_rotation) f.rotation={{0,0,0}};
    for (const auto& d:f.directions) f.raw_near.push_back(a+b*d[1]);
    for (std::size_t i=0;i<6;++i) f.matrix[i][i]=2.0L/3;
    const long double shear=static_cast<long double>(b)/(3.0L*a);
    f.matrix[3][2]=shear;
    f.matrix[5][0]=-shear;
    f.rhs[0]=2.0L*a/3;
    f.solution[0]=a;
    for (std::size_t i=0;i<3;++i) {
        f.rhs[i+3]=2.0L*f.rotation[i]/3;
        f.solution[i+3]=f.rotation[i];
    }
    f.rhs[5]-=static_cast<long double>(b)/3;
    const long double gamma=static_cast<long double>(b)/(2.0L*a);
    const long double factor=(std::sqrt(gamma*gamma+4)+gamma)/2;
    f.singular_values={{2*factor/3,2*factor/3,2.0L/3,2.0L/3,2/(3*factor),2/(3*factor)}};
    f.condition=factor*factor;
    return f;
}
Fixture cone(double h) {
    Fixture f;
    f.name="cone h="+std::to_string(h);
    const double q=2*h/(1+h*h),c=(1-h*h)/(1+h*h);
    f.directions={{{q,0,c}},{{-q,0,c}},{{0,q,c}},{{0,-q,c}}};
    f.raw_near.assign(4,1);
    f.coefficients=coefficients(1,0);
    // 独立解析式以提升后的 h 计算，不从生产 trace 或生成视线反推参考矩阵。
    const long double hh=h,denom=(1+hh*hh)*(1+hh*hh);
    const long double cc=(1-hh*hh)/(1+hh*hh);
    const long double axial=4*hh*hh/denom,transverse=(1+cc*cc)/2;
    for (std::size_t i=0;i<6;++i) f.matrix[i][i]=i%3==2?axial:transverse;
    f.matrix[0][4]=cc; f.matrix[1][3]=-cc;
    f.matrix[3][1]=-cc; f.matrix[4][0]=cc;
    f.rhs={{transverse+cc*f.rotation[1],-cc*f.rotation[0],0,
            transverse*f.rotation[0],cc+transverse*f.rotation[1],axial*f.rotation[2]}};
    f.solution={{1,0,0,f.rotation[0],f.rotation[1],f.rotation[2]}};
    f.singular_values={{2/denom,2/denom,axial,axial,2*hh*hh*hh*hh/denom,2*hh*hh*hh*hh/denom}};
    f.condition=1/(hh*hh*hh*hh);
    // 窄锥前向误差只记录，不以普通回归 allowance 冒充病态解的正式数值资格。
    f.check_forward_solution=false;
    return f;
}
void inspectSystem(const Fixture& f,const detail::AdaptiveMotionResult& result) {
    const auto& t=result.trace;
    near(t.original_near_mean/f.mean,1,f.name+": relative raw mean");
    for (std::size_t row=0;row<f.raw_near.size();++row) {
        const auto i=static_cast<Eigen::Index>(row);
        near(t.near_raw[i],f.raw_near[row],f.name+": raw near");
        near(t.near_after_sign[i],f.raw_near[row],f.name+": positive-mean sign step");
        near(t.near_normalized[i],static_cast<long double>(f.raw_near[row])/f.mean,
             f.name+": normalized near");
    }
    for (std::size_t r=0;r<6;++r) {
        near(t.rhs[static_cast<Eigen::Index>(r)],f.rhs[r],f.name+": independent RHS");
        for (std::size_t c=0;c<6;++c)
            near(t.system_matrix(static_cast<Eigen::Index>(r),static_cast<Eigen::Index>(c)),
                 f.matrix[r][c],f.name+": independent matrix entry");
        if (f.check_forward_solution)
            near(t.solved_motion[static_cast<Eigen::Index>(r)],f.solution[r],f.name+": raw solution");
    }
    const Eigen::JacobiSVD<Matrix6> svd(t.system_matrix);
    for (std::size_t i=0;i<6;++i)
        near(svd.singularValues()[static_cast<Eigen::Index>(i)],f.singular_values[i],
             f.name+": independent singular-value scale");
    long double residual=0,matrix_norm=0,rhs_norm=0,solution_norm=0,forward=0;
    for (std::size_t r=0;r<6;++r) {
        long double product=0,row_norm=0;
        for (std::size_t c=0;c<6;++c) {
            const long double entry=t.system_matrix(static_cast<Eigen::Index>(r),static_cast<Eigen::Index>(c));
            product+=entry*t.solved_motion[static_cast<Eigen::Index>(c)];
            row_norm+=std::abs(entry);
        }
        residual=std::max(residual,std::abs(product-t.rhs[static_cast<Eigen::Index>(r)]));
        matrix_norm=std::max(matrix_norm,row_norm);
        rhs_norm=std::max(rhs_norm,std::abs(static_cast<long double>(t.rhs[static_cast<Eigen::Index>(r)])));
        solution_norm=std::max(solution_norm,std::abs(static_cast<long double>(t.solved_motion[static_cast<Eigen::Index>(r)])));
        forward=std::max(forward,std::abs(t.solved_motion[static_cast<Eigen::Index>(r)]-f.solution[r]));
    }
    const long double backward=residual/(matrix_norm*solution_norm+rhs_norm);
    near(backward,0,f.name+": backward residual");
    std::cout<<f.name<<" mean="<<t.original_near_mean
             <<" condition_formula="<<f.condition
             <<" condition_double_svd="<<svd.singularValues()[0]/svd.singularValues()[5]
             <<" determinant="<<t.system_matrix.determinant()<<" raw_translation_norm="<<t.translation_raw.norm()
             <<" forward_max="<<forward<<" backward="<<backward<<" mse="<<result.error_mse
             <<" forward_asserted="<<f.check_forward_solution<<'\n';
    std::cout<<"matrix\n"<<t.system_matrix<<"\nrhs "<<t.rhs.transpose()
             <<"\nsolution "<<t.solved_motion.transpose()<<"\nsingular_values "
             <<svd.singularValues().transpose()<<"\nraw_near "<<t.near_raw.transpose()
             <<"\nnormalized_near "<<t.near_normalized.transpose()<<'\n';
}
void verify(const Fixture& fixture) {
    const FlowField field=flow(fixture),saved_field=field;
    const HarmonicCoefficients saved_coeff=fixture.coefficients;
    const auto actual=detail::solveAdaptiveMotion(fixture.coefficients,field);
    check(actual.hasValue(),fixture.name+": unexpected failure");
    check(std::memcmp(field.observations.data(),saved_field.observations.data(),
                      static_cast<std::size_t>(field.observations.size())*sizeof(double))==0&&
          std::memcmp(fixture.coefficients.values.data(),saved_coeff.values.data(),15*sizeof(double))==0,
          fixture.name+": input changed");
    if (!actual.hasValue()) { std::cerr<<actual.error().detail.message<<'\n'; return; }
    const auto& result=actual.value();
    inspectSystem(fixture,result);
    near(result.motion.translation().norm(),1,fixture.name+": unit translation");
    check(result.motion.translation().allFinite()&&result.motion.rotation().allFinite()&&
          result.trace.recomputed_near.allFinite()&&std::isfinite(result.error_mse),
          fixture.name+": nonfinite result");
    check(result.error_mse>=0,fixture.name+": negative MSE");
    if (fixture.check_forward_solution) {
        for (std::size_t i=0;i<3;++i) {
            near(result.motion.translation()[static_cast<Eigen::Index>(i)],fixture.translation[i],
                 fixture.name+": normalized translation");
            near(result.motion.rotation()[static_cast<Eigen::Index>(i)],fixture.rotation[i],fixture.name+": rotation");
        }
        near(result.error_mse,0,fixture.name+": ideal-flow MSE");
        for (std::size_t i=0;i<6;++i)
            near(result.trace.recomputed_near[static_cast<Eigen::Index>(i)],
                 i<2?0:fixture.raw_near[i]/(1+kNearDenominatorEpsilon),fixture.name+": recomputed near");
    }
    // 所有固定场的 xy 源通道均为零：运动阶段成功不等于完整投影成功。
    const auto complete=AdaptiveMfaStepSolver{}.solve(fixture.coefficients,field);
    check(!complete.hasValue(),fixture.name+": full step must reject zero projection norm");
    if (!complete.hasValue()) {
        check(complete.error().code==NumericalErrorCode::SingularSystem&&
              complete.error().detail.stage==ErrorStage::Solve,fixture.name+": projection classification");
        bool projection_cause=false;
        for (const auto& cause:complete.error().detail.causes)
            projection_cause|=cause.domain=="LowOrderHarmonicsErrorCode"&&cause.code=="ZeroProjectionNorm";
        check(projection_cause,fixture.name+": missing projection cause");
    }
}
void negativeCases() {
    auto zero=axes(1,0);
    zero.coefficients=coefficients(0,0);
    auto cancellation=zero;
    cancellation.coefficients=coefficients(0,1);
    auto rotation=axes(1,0);
    rotation.translation={{0,0,0}};
    auto collapsed=cone(0.1);
    collapsed.directions.assign(4,Triple{{0,0,1}});
    const std::array<Fixture,4> inputs{{zero,cancellation,rotation,collapsed}};
    const std::array<NumericalErrorCode,4> expected{{NumericalErrorCode::InvalidNearness,
        NumericalErrorCode::InvalidNearness,NumericalErrorCode::ZeroNorm,NumericalErrorCode::SingularSystem}};
    for (std::size_t i=0;i<inputs.size();++i) {
        const auto actual=AdaptiveMfaStepSolver{}.solve(inputs[i].coefficients,flow(inputs[i]));
        check(!actual.hasValue(),"Undefined family unexpectedly succeeded");
        if (!actual.hasValue()) {
            check(actual.error().code==expected[i],"Undefined family error classification");
            check(actual.error().detail.stage==ErrorStage::Solve,"Undefined family error stage");
            std::cout<<"negative_case="<<i<<" code="<<errorName(actual.error().code)
                     <<" message="<<actual.error().detail.message<<'\n';
        }
    }
}
}  // namespace

int main() {
    std::cout<<std::setprecision(18);
    for (double a:{1.0,0.1,0.001,0.000001}) verify(axes(a,0));
    auto tiny=axes(1e-100,0,true); tiny.name="uniform positive a=1e-100 omega=0"; verify(tiny);
    for (double a:{1.0,0.1,0.001,0.000001}) verify(axes(a,1));
    for (double h:{0.1,0.01,0.001}) verify(cone(h));
    negativeCases();
    std::cout<<"Adaptive conditioning checks="<<checks<<" failures="<<failures<<'\n';
    return failures==0?0:1;
}
