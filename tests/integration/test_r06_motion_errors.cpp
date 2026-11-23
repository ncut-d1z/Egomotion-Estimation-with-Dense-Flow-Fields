// 解析几何参考；不运行 MATLAB，不以被测函数生成参考值。
#include "numerics/source_motion_errors.hpp"

#include <array>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

namespace {
using namespace egomotion;
using namespace egomotion::source_metrics;
using MetricResult = Result<Scalar,NumericalErrorCode>;
// 沿用既有 R6 Bias/Adaptive 回归精度，不代表 DEC-005 正式验收阈值。
constexpr double kRegressionTolerance = 1e-9;
int checks=0,failures=0;
void check(bool pass,const std::string& message) {
    ++checks;
    if (!pass) { ++failures; std::cerr<<message<<'\n'; }
}
void checkFailure(const MetricResult& value,NumericalErrorCode expected,
                  const std::string& label) {
    check(!value.hasValue(),label+": unexpected value");
    if (!value.hasValue()) {
        check(value.error().code==expected,label+": wrong error code");
        check(value.error().detail.stage==ErrorStage::Solve,label+": wrong stage");
        check(!value.error().detail.message.empty(),label+": missing diagnostic");
    }
}
void checkDegrees(const MetricResult& value,double expected,const std::string& label) {
    check(value.hasValue(),label+": unexpected failure");
    if (value.hasValue()) {
        check(std::isfinite(value.value()),label+": non-finite angle");
        check(value.value()>=0&&value.value()<=180,label+": angle outside [0,180]");
        check(std::abs(value.value()-expected)<=kRegressionTolerance,
              label+": differs from independent angle");
    }
}
void geometry(const Vector3& truth,const Vector3& estimate,
              double distance,double degrees,const std::string& label) {
    const Vector3 original_truth=truth,original_estimate=estimate;
    const auto actual_distance=normalizedVectorError(truth,estimate);
    const auto actual_angle=sourceDirectionErrorDegrees(truth,estimate);
    check(actual_distance.hasValue()&&actual_angle.hasValue(),label+": invalid");
    if (actual_distance.hasValue())
        check(std::abs(actual_distance.value()-distance)<=kRegressionTolerance,
              label+": distance differs from analytic geometry");
    if (actual_angle.hasValue())
        check(std::abs(actual_angle.value()-degrees)<=kRegressionTolerance,
              label+": angle differs from analytic geometry");
    if (actual_distance.hasValue()&&actual_angle.hasValue())
        check(std::abs(actual_distance.value()-2*truth.norm()*
              std::sin(std::acos(-1.0)*actual_angle.value()/360))<=kRegressionTolerance,
              label+": chord-angle identity failed");
    check(std::memcmp(truth.data(),original_truth.data(),sizeof(double)*3)==0&&
          std::memcmp(estimate.data(),original_estimate.data(),sizeof(double)*3)==0,
          label+": input bytes changed");
}
void invalidPair(const Vector3& truth,const Vector3& estimate,
                 NumericalErrorCode distance_code,NumericalErrorCode angle_code,
                 const std::string& label) {
    const Vector3 original_truth=truth,original_estimate=estimate;
    checkFailure(normalizedVectorError(truth,estimate),distance_code,label+" distance");
    checkFailure(sourceDirectionErrorDegrees(truth,estimate),angle_code,label+" angle");
    check(std::memcmp(truth.data(),original_truth.data(),sizeof(double)*3)==0&&
          std::memcmp(estimate.data(),original_estimate.data(),sizeof(double)*3)==0,
          label+": input bytes changed");
}
void analyticCases() {
    for (int axis=0;axis<3;++axis) {
        const Vector3 a=Vector3::Unit(axis),b=Vector3::Unit((axis+1)%3);
        for (double sign : {-1.0,1.0}) {
            for (double estimate_scale : {0.5,2.0,16.0}) {
                geometry(sign*a,sign*estimate_scale*a,0,0,"same axis");
                geometry(sign*a,-sign*estimate_scale*a,2,180,"opposite axis");
                geometry(sign*a,sign*estimate_scale*b,std::sqrt(2.0),90,"orthogonal");
                geometry(3*sign*a,sign*estimate_scale*b,3*std::sqrt(2.0),90,
                         "truth scale retained in distance");
                geometry(3*sign*a,-sign*estimate_scale*a,6,180,"scaled opposite");
            }
        }
    }
    // 3-4-5 三角形：单位估计与 x 轴夹角 cos=3/5，弦长 sqrt(4/5)。
    geometry(Vector3::UnitX(),Vector3(3,4,0),std::sqrt(0.8),
             std::acos(0.6)*180/std::acos(-1.0),"Pythagorean direction");
}
void exceptionalCases() {
    const Vector3 x=Vector3::UnitX(),zero=Vector3::Zero();
    using Code=NumericalErrorCode;
    invalidPair(zero,x,Code::ZeroNorm,Code::UndefinedAngle,"zero truth");
    invalidPair(x,zero,Code::ZeroNorm,Code::UndefinedAngle,"zero estimate");
    invalidPair(zero,zero,Code::ZeroNorm,Code::UndefinedAngle,"both zero");
    for (double value : {std::numeric_limits<double>::quiet_NaN(),
                         std::numeric_limits<double>::infinity(),
                         -std::numeric_limits<double>::infinity()}) {
        for (int axis=0;axis<3;++axis) {
            Vector3 bad=x;
            bad[axis]=value;
            invalidPair(bad,x,Code::NonFiniteInput,Code::NonFiniteInput,"nonfinite truth");
            invalidPair(x,bad,Code::NonFiniteInput,Code::NonFiniteInput,"nonfinite estimate");
        }
    }
    // DEC-R6-01 仅改变方向角；Bias 距离原有的平方范数边界继续明确失败。
    checkFailure(normalizedVectorError(1e308*x,x),Code::NonFiniteInput,"truth distance norm overflow");
    checkFailure(normalizedVectorError(x,1e308*x),Code::NonFiniteInput,"estimate distance norm overflow");
    checkFailure(normalizedVectorError(1e-200*x,x),Code::ZeroNorm,"truth distance norm underflow");
    checkFailure(normalizedVectorError(x,1e-200*x),Code::ZeroNorm,"estimate distance norm underflow");
    for (double scale : {1e308,1e-200,std::numeric_limits<double>::max(),
                         std::numeric_limits<double>::denorm_min()}) {
        checkDegrees(sourceDirectionErrorDegrees(scale*x,x),0,"finite extreme truth direction");
        checkDegrees(sourceDirectionErrorDegrees(x,scale*x),0,"finite extreme estimate direction");
        checkDegrees(sourceDirectionErrorDegrees(scale*x,-scale*x),180,"finite extreme opposite direction");
        checkDegrees(sourceDirectionErrorDegrees(scale*x,scale*Vector3::UnitY()),90,
                     "finite extreme perpendicular direction");
    }
    checkFailure(normalizedVectorError(1e154*x,-1e154*x),Code::NonFiniteResult,
                 "finite input norms but distance overflow");
    const auto angle=sourceDirectionErrorDegrees(1e154*x,-1e154*x);
    check(angle.hasValue()&&angle.value()==180,"angle remains defined despite distance overflow");
}
void acosRoundoffCases() {
    // binary64 的 1/sqrt(3) 平方求和会略越过单位端点。
    // 旧表达式仅作诊断；新合同按独立几何断言 0/180，不锁定平台舍入轨迹。
    const Vector3 truth(1,1,1);
    const double unit_component=1/std::sqrt(3.0);
    const double component_square=unit_component*unit_component;
    const double rounded_unit_square=(component_square+component_square)+component_square;
    for (double sign : {-1.0,1.0}) {
        const Vector3 estimate=sign*truth;
        const Vector3 original_truth=truth,original_estimate=estimate;
        const double cosine=sign*rounded_unit_square;
        const std::string label=sign>0?"acos upper roundoff":"acos lower roundoff";
        const auto angle=sourceDirectionErrorDegrees(truth,estimate);
        checkDegrees(angle,sign>0?0:180,label);
        const auto distance=normalizedVectorError(truth,estimate);
        check(distance.hasValue(),label+": distance should remain defined");
        if (distance.hasValue())
            check(std::abs(distance.value()-(sign>0?0:2*std::sqrt(3.0)))<=
                  kRegressionTolerance,label+": analytic distance differs");
        check(std::memcmp(truth.data(),original_truth.data(),sizeof(double)*3)==0&&
              std::memcmp(estimate.data(),original_estimate.data(),sizeof(double)*3)==0,
              label+": input bytes changed");
        std::cout<<label<<" scalar_cosine="<<std::setprecision(17)<<cosine<<'\n';
    }
}

void stableDirectionCases() {
    const double pi=std::acos(-1.0);
    // atan(tiny) 的一阶项在这里已远精确于既有回归余量；不能把小角置零。
    const double tiny=1e-200;
    const auto small=sourceDirectionErrorDegrees(Vector3::UnitX(),Vector3(1,tiny,0));
    checkDegrees(small,tiny*180/pi,"tiny nonzero direction angle");
    if (small.hasValue())
        check(small.value()>0&&std::abs(small.value()/(tiny*180/pi)-1)<=kRegressionTolerance,
              "tiny direction angle lost to squared-norm underflow");
    const double gap=1e-8;
    checkDegrees(sourceDirectionErrorDegrees(Vector3::UnitX(),Vector3(-std::cos(gap),std::sin(gap),0)),
                 180-gap*180/pi,"direction near pi");
    for (double scale : {1e300,1e-300}) {
        checkDegrees(sourceDirectionErrorDegrees(Vector3(3*scale,4*scale,0),Vector3::UnitX()),
                     std::acos(0.6)*180/pi,"scaled Pythagorean direction");
        checkDegrees(sourceDirectionErrorDegrees(Vector3(scale,scale,scale),Vector3(-scale,-scale,-scale)),
                     180,"scaled diagonal opposite direction");
    }
    // 已保存真实 frame14 反例。独立 long double 半弦公式与生产 cross/dot 分离。
    const Vector3 truth(0.105838521997944870479,0.0192068599796313688177,0);
    const Vector3 estimate(0.983929588586116565807,0.178556894867590298936,
                           1.77050023892425639131e-10);
    std::array<long double,3> u{},v{};
    long double truth_squared=0,estimate_squared=0;
    for (int i=0;i<3;++i) {
        u[i]=truth[i];v[i]=estimate[i];
        truth_squared+=u[i]*u[i];estimate_squared+=v[i]*v[i];
    }
    const long double truth_norm=std::sqrt(truth_squared),estimate_norm=std::sqrt(estimate_squared);
    long double difference_squared=0,sum_squared=0;
    for (int i=0;i<3;++i) {
        u[i]/=truth_norm;v[i]/=estimate_norm;
        difference_squared+=(u[i]-v[i])*(u[i]-v[i]);
        sum_squared+=(u[i]+v[i])*(u[i]+v[i]);
    }
    const double independent=static_cast<double>(2*std::atan2(std::sqrt(difference_squared),
        std::sqrt(sum_squared))*180/std::acos(-1.0L));
    const auto actual=sourceDirectionErrorDegrees(truth,estimate);
    checkDegrees(actual,independent,"retained frame14 finite direction");
    if (actual.hasValue())check(actual.value()>0,"frame14 direction was replaced by zero");
}

void rotationCase(const Vector3& truth,const Vector3& estimate,double expected,
                  const std::string& label) {
    const Vector3 original_truth=truth,original_estimate=estimate;
    checkDegrees(rotationGeodesicErrorDegrees(truth,estimate),expected,label);
    checkDegrees(rotationGeodesicErrorDegrees(estimate,truth),expected,label+" symmetric");
    check(std::memcmp(truth.data(),original_truth.data(),sizeof(double)*3)==0&&
          std::memcmp(estimate.data(),original_estimate.data(),sizeof(double)*3)==0,
          label+": rotation inputs changed");
}

void rotationGeodesicCases() {
    const double pi=std::acos(-1.0);
    const Vector3 zero=Vector3::Zero(),x=Vector3::UnitX(),y=Vector3::UnitY(),z=Vector3::UnitZ();
    rotationCase(zero,zero,0,"identity rotations");
    rotationCase(zero,(pi/2)*x,90,"zero rotation has a defined error");
    rotationCase((pi/6)*z,(pi/3)*z,30,"same axis different rotation magnitudes");
    rotationCase((pi/2)*z,(-pi/2)*z,180,"opposite quarter turns");
    rotationCase(zero,2*pi*x,0,"full turn represents identity");
    rotationCase(zero,(5*pi/2)*x,90,"rotation vector wraps in SO(3)");
    // 正交四分之一转的相对四元数标量为 1/2，独立解析测地角 120 度。
    rotationCase((pi/2)*x,(pi/2)*y,120,"noncoaxial quarter turns");
    rotationCase(pi*x,pi*y,180,"noncoaxial half turns");
    const Vector3 a(1.0/3,2.0/3,2.0/3),b(2.0/3,-1.0/3,2.0/3);
    // a dot b=4/9；四元数内积 cos(pi/6)cos(pi/4)+4/9 sin(pi/6)sin(pi/4)。
    const double expected=2*std::acos(std::sqrt(6.0)/4+std::sqrt(2.0)/9)*180/pi;
    rotationCase((pi/3)*a,(pi/2)*b,expected,"general noncoaxial analytic rotations");
    rotationCase((pi/3)*a,(pi/3)*a,0,"identical non-axis rotations");
    const double gap=1e-8;
    rotationCase(zero,(pi-gap)*a,180-gap*180/pi,"rotation just below pi");
    rotationCase(zero,(pi+gap)*a,180-gap*180/pi,"rotation just above pi");
    for (double tiny : {1e-200,std::numeric_limits<double>::denorm_min()}) {
        const auto small=rotationGeodesicErrorDegrees(zero,tiny*x);
        checkDegrees(small,tiny*(180/pi),"tiny nonzero rotation");
        if (small.hasValue())check(small.value()>0,"small rotation was erased by a norm threshold");
    }
    // 单轴极大有限旋转的模长可表示，不应因 theta^2 上溢失败。
    const auto large=rotationGeodesicErrorDegrees(zero,1e308*x);
    check(large.hasValue(),"representable large rotation rejected by squared norm");
    if (large.hasValue())check(std::isfinite(large.value())&&large.value()>=0&&large.value()<=180,
                              "large rotation did not produce a principal angle");
}

void rotationExceptionalCases() {
    using Code=NumericalErrorCode;
    const Vector3 zero=Vector3::Zero();
    for (double value : {std::numeric_limits<double>::quiet_NaN(),
                         std::numeric_limits<double>::infinity(),
                         -std::numeric_limits<double>::infinity()}) {
        for (int axis=0;axis<3;++axis) {
            Vector3 bad=zero;bad[axis]=value;
            checkFailure(rotationGeodesicErrorDegrees(bad,zero),Code::NonFiniteInput,
                         "nonfinite true rotation");
            checkFailure(rotationGeodesicErrorDegrees(zero,bad),Code::NonFiniteInput,
                         "nonfinite estimated rotation");
        }
    }
    const Vector3 beyond_norm=Vector3::Constant(std::numeric_limits<double>::max());
    checkFailure(rotationGeodesicErrorDegrees(beyond_norm,zero),Code::NonFiniteResult,
                 "finite rotation has unrepresentable magnitude");
    checkFailure(rotationGeodesicErrorDegrees(zero,beyond_norm),Code::NonFiniteResult,
                 "finite estimate has unrepresentable magnitude");
}
}  // namespace

int main() {
    analyticCases();
    exceptionalCases();
    acosRoundoffCases();
    stableDirectionCases();
    rotationGeodesicCases();
    rotationExceptionalCases();
    std::cout<<"Motion error checks="<<checks<<" failures="<<failures<<'\n';
    return failures==0?0:1;
}
