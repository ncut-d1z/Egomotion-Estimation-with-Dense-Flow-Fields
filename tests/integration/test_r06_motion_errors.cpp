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
    invalidPair(1e308*x,x,Code::NonFiniteInput,Code::NonFiniteInput,"truth norm overflow");
    invalidPair(x,1e308*x,Code::NonFiniteInput,Code::NonFiniteInput,"estimate norm overflow");
    // 保留现有 double 平方范数的下溢行为，不能以精度升级掩盖边界。
    invalidPair(1e-200*x,x,Code::ZeroNorm,Code::UndefinedAngle,"truth norm underflow");
    invalidPair(x,1e-200*x,Code::ZeroNorm,Code::UndefinedAngle,"estimate norm underflow");
    checkFailure(normalizedVectorError(1e154*x,-1e154*x),Code::NonFiniteResult,
                 "finite input norms but distance overflow");
    const auto angle=sourceDirectionErrorDegrees(1e154*x,-1e154*x);
    check(angle.hasValue()&&angle.value()==180,"angle remains defined despite distance overflow");
}
void acosRoundoffCases() {
    // binary64 的 1/sqrt(3) 平方求和会略越过单位端点。
    // 固定有限输入验证既有 C++ 安全拒绝；不声称 MATLAB 运行时行为相同。
    const Vector3 truth(1,1,1);
    const double unit_component=1/std::sqrt(3.0);
    const double component_square=unit_component*unit_component;
    const double rounded_unit_square=(component_square+component_square)+component_square;
    for (double sign : {-1.0,1.0}) {
        const Vector3 estimate=sign*truth;
        const Vector3 original_truth=truth,original_estimate=estimate;
        const double cosine=sign*rounded_unit_square;
        const std::string label=sign>0?"acos upper roundoff":"acos lower roundoff";
        check(sign>0?cosine>1:cosine< -1,label+": fixed input does not cross endpoint");
        const auto angle=sourceDirectionErrorDegrees(truth,estimate);
        checkFailure(angle,NumericalErrorCode::UndefinedAngle,label);
        if (!angle.hasValue())
            check(angle.error().detail.message=="normrelfehler acos argument outside [-1,1]",
                  label+": wrong failure branch");
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
}  // namespace

int main() {
    analyticCases();
    exceptionalCases();
    acosRoundoffCases();
    std::cout<<"Motion error checks="<<checks<<" failures="<<failures<<'\n';
    return failures==0?0:1;
}
