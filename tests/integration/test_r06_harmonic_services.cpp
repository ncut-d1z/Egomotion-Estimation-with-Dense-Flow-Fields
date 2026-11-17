// P7.10：固定源约定的三个无状态服务；独立多项式/精确投影参考，不运行MATLAB。
#include "egomotion/harmonics/services.hpp"
#include "egomotion/harmonics/adapter.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace {
using namespace egomotion;
using namespace egomotion::harmonics;
static_assert(std::is_empty_v<RealSphericalHarmonics> && std::is_empty_v<HarmonicFitter> &&
              std::is_empty_v<HarmonicRotator>,"No per-session state in harmonic services");
static_assert(!std::is_polymorphic_v<RealSphericalHarmonics> &&
              !std::is_polymorphic_v<HarmonicFitter> && !std::is_polymorphic_v<HarmonicRotator>,
              "Fixed mathematical conventions need no inheritance");
constexpr double kTolerance=4096.0*std::numeric_limits<double>::epsilon();
std::size_t checks=0;double maximum_error=0;
void require(bool condition,const char* message) {
    ++checks;if(!condition)throw std::runtime_error(message);
}
void close(double actual,double expected) {
    const double error=std::abs(actual-expected);maximum_error=std::max(maximum_error,error);
    require(std::isfinite(actual)&&std::isfinite(expected)&&error<=kTolerance,"Independent harmonic reference");
}
void same(const HarmonicMatrix& a,const HarmonicMatrix& b) {
    require((a.array()==b.array()).all(),"Legacy function and service preserve bits");
}
HarmonicMatrix referenceBasis(const Vector3& v) {
    // 原kugelflfunc2的CS相位，l=0..2、m=-l..l；独立Cartesian多项式。
    const double pi=std::acos(-1.0),x=v.x(),y=v.y(),z=v.z();
    const double one=std::sqrt(3/(4*pi)),two=std::sqrt(15/(4*pi));
    HarmonicMatrix b=HarmonicMatrix::Zero();
    b(0,0)=1/std::sqrt(4*pi);
    b(1,0)=-one*y;b(1,1)=one*z;b(1,2)=-one*x;
    b(2,0)=two*x*y;b(2,1)=-two*y*z;b(2,2)=std::sqrt(5/(16*pi))*(3*z*z-1);
    b(2,3)=-two*x*z;b(2,4)=std::sqrt(15/(16*pi))*(x*x-y*y);
    return b;
}
std::vector<Vector3> directions() {
    return {Vector3::UnitX(),Vector3::UnitY(),Vector3::UnitZ(),
        -Vector3::UnitX(),-Vector3::UnitY(),-Vector3::UnitZ(),
        Vector3(3.0/5,4.0/5,0),Vector3(0,3.0/5,4.0/5),
        Vector3(4.0/5,0,3.0/5),Vector3(1.0/3,2.0/3,2.0/3)};
}
HarmonicCoefficients coefficients() {
    HarmonicCoefficients c;
    c.values<<2,0,0,0,0, 0.125,-0.25,0.375,0,0, -0.125,0.25,0.5,-0.375,0.625;
    return c;
}
void basisAndReconstruction() {
    const RealSphericalHarmonics service;const auto c=coefficients();
    for(const auto& direction:directions()) {
        const auto expected=referenceBasis(direction),basis=service.sourceBasis(direction).value();
        for(int l=0;l<3;++l)for(int m=0;m<5;++m)close(basis(l,m),expected(l,m));
        const auto reconstructed=service.reconstructNear(c,direction);
        require(reconstructed.hasValue(),"Finite source reconstruction");
        close(reconstructed.value(),(c.values.array()*expected.array()).sum());
        require(reconstructed.value()==reconstructNear(c,direction).value(),"Reconstruction forwarding");
        for(int l=0;l<3;++l)for(int m=0;m<2*l+1;++m) {
            HarmonicCoefficients channel{HarmonicMatrix::Zero()};channel.values(l,m)=1;
            close(service.reconstructNear(channel,direction).value(),expected(l,m));
        }
    }
    // 无效六项只要求有限、不参与重建；不得暗改为非零填充拒绝。
    auto padding=c;for(int l=0;l<2;++l)for(int m=2*l+1;m<5;++m)padding.values(l,m)=123;
    require(service.reconstructNear(padding,Vector3::UnitX()).value()==
            service.reconstructNear(c,Vector3::UnitX()).value(),"Finite unused padding ignored");
}
HarmonicMatrix projectionReference() {
    // verification/harmonics/source_projection_reference.v1.json的独立精确七方向参考。
    const double pi=std::acos(-1.0),p=std::sqrt(pi),p3=std::sqrt(3*pi),
                 p5=std::sqrt(5*pi),p15=std::sqrt(15*pi);
    HarmonicMatrix r=HarmonicMatrix::Zero();r(0,0)=37*p/14;
    r(1,0)=-19*p3/55;r(1,1)=361*p3/220;r(1,2)=-154*p3/95;
    r(2,0)=4935*p15/7082;r(2,1)=-1695*p15/5416;r(2,2)=25530*p5/44797;
    r(2,3)=-3075*p15/3541;r(2,4)=4360*p15/15349;return r;
}
FlowField projectionSamples() {
    FlowField field{FlowMatrix::Zero(7,6)};
    field.observations.leftCols<3>()<<1,0,0,0,1,0,0,0,1,3.0/5,4.0/5,0,
        0,3.0/5,4.0/5,4.0/5,0,3.0/5,1.0/3,2.0/3,2.0/3;
    return field;
}
Eigen::VectorXd nearness() {
    Eigen::VectorXd values(7);values<<2,-1,4,0.5,-3,1.25,5.5;return values;
}
void fittingAndOwnership() {
    const HarmonicFitter fitter;const auto expected=projectionReference();
    auto input=projectionSamples();auto near=nearness();const auto original=input;
    const auto fitted=fitter.fitSourceNear(near,input);
    require(fitted.hasValue(),"Independent seven-direction source fit");
    for(int l=0;l<3;++l)for(int m=0;m<5;++m)close(fitted.value().values(l,m),expected(l,m));
    same(fitted.value().values,fitSourceNear(near,input).value().values);
    // 原函数只消费视线列，即使尾部非有限也不改变此服务的投影合同。
    input.observations(0,5)=std::numeric_limits<double>::infinity();
    same(fitter.fitSourceNear(near,input).value().values,fitted.value().values);
    input.observations.setZero();near.setZero();
    same(fitted.value().values,fitter.fitSourceNear(nearness(),original).value().values);
    const auto other=fitter.fitSourceNear(-nearness(),original);
    require(other.hasValue(),"Negative near projection remains allowed");
    same(fitter.fitSourceNear(nearness(),original).value().values,fitted.value().values);
}
void rotationSemantics() {
    const HarmonicRotator rotator;const RealSphericalHarmonics basis;const auto old=coefficients();
    Matrix3 x,y,mixed;x<<1,0,0,0,0,-1,0,1,0;y<<0,0,1,0,1,0,-1,0,0;
    mixed<<1,8,4,8,1,-4,-4,4,-7;mixed/=9.0;
    const std::array<Matrix3,5> rotations{{Matrix3::Identity(),x,y,mixed,mixed.transpose()}};
    for(const auto& r:rotations) {
        const auto physical=rotator.rotateCoefficients(old,r);
        const auto source=rotator.rotateSourceCoefficients(old,r);
        require(physical.hasValue()&&source.hasValue(),"Both explicit rotation conventions");
        same(physical.value().values,rotateCoefficients(old,r).value().values);
        same(source.value().values,rotateSourceCoefficients(old,r).value().values);
        for(const auto& d:directions())close(basis.reconstructNear(physical.value(),d).value(),
            (old.values.array()*referenceBasis(r*d).array()).sum());
        const auto inverse=rotator.rotateCoefficients(physical.value(),r.transpose());
        require(inverse.hasValue(),"Inverse physical rotation");
        for(int l=0;l<3;++l)for(int m=0;m<5;++m)close(inverse.value().values(l,m),old.values(l,m));
    }
    // 独立判别mu=2+z与Rx(pi/2)：物理2+y，原系数动作2-y。
    HarmonicCoefficients c{HarmonicMatrix::Zero()};const double pi=std::acos(-1.0);
    c.values(0,0)=2*std::sqrt(4*pi);c.values(1,1)=std::sqrt(4*pi/3);
    close(basis.reconstructNear(rotator.rotateCoefficients(c,x).value(),Vector3::UnitY()).value(),3);
    close(basis.reconstructNear(rotator.rotateSourceCoefficients(c,x).value(),Vector3::UnitY()).value(),1);
}
template<class T> void error(const Result<T,NumericalErrorCode>& result,NumericalErrorCode code,
                            const char* leaf) {
    require(!result.hasValue()&&result.error().code==code,"Public numerical error unchanged");
    require(result.error().detail.stage==ErrorStage::Solve,"Solve stage preserved");
    if(leaf) {
        const auto& chain=result.error().detail.causes;
        require(!chain.empty(),"Original leaf error frame retained");
        require(chain.front().domain=="LowOrderHarmonicsErrorCode"&&chain.front().code==leaf&&
                chain.front().stage==ErrorStage::Solve&&!chain.front().message.empty(),"Leaf family/code/message preserved");
    }else require(result.error().detail.causes.empty(),"Adapter shape failure does not invent a leaf");
}
void boundaries() {
    const RealSphericalHarmonics basis;const HarmonicFitter fitter;const HarmonicRotator rotator;
    const auto c=coefficients();auto samples=projectionSamples();auto near=nearness();
    error(basis.sourceBasis(Vector3::Zero()),NumericalErrorCode::InvalidDirection,"NotUnitDirection");
    Vector3 nan=Vector3::UnitX();nan.x()=std::numeric_limits<double>::quiet_NaN();
    error(basis.sourceBasis(nan),NumericalErrorCode::NonFiniteInput,"NonFiniteInput");
    auto invalid=c;invalid.values(0,4)=std::numeric_limits<double>::infinity();
    error(basis.reconstructNear(invalid,Vector3::UnitX()),NumericalErrorCode::NonFiniteInput,"NonFiniteInput");
    error(fitter.fitSourceNear(Eigen::VectorXd::Zero(6),samples),NumericalErrorCode::InvalidShape,nullptr);
    error(fitter.fitSourceNear(Eigen::VectorXd{},FlowField{}),NumericalErrorCode::InvalidShape,nullptr);
    near[0]=std::numeric_limits<double>::quiet_NaN();
    error(fitter.fitSourceNear(near,samples),NumericalErrorCode::NonFiniteInput,"NonFiniteInput");
    samples.observations(0,0)=2;
    error(fitter.fitSourceNear(nearness(),samples),NumericalErrorCode::InvalidDirection,"NotUnitDirection");
    FlowField axes{FlowMatrix::Zero(3,6)};axes.observations.leftCols<3>().setIdentity();
    error(fitter.fitSourceNear(Eigen::VectorXd::Ones(3),axes),NumericalErrorCode::SingularSystem,"ZeroProjectionNorm");
    Matrix3 reflection=Matrix3::Identity();reflection(2,2)=-1;
    error(rotator.rotateCoefficients(c,reflection),NumericalErrorCode::InvalidDirection,"NotProperRotation");
    error(rotator.rotateSourceCoefficients(c,reflection),NumericalErrorCode::InvalidDirection,"NotProperRotation");
    error(rotator.rotateSourceCoefficients(invalid,Matrix3::Identity()),NumericalErrorCode::NonFiniteInput,"NonFiniteInput");
    // 失败不改变可复用的无状态服务，也不污染独立返回值。
    same(fitter.fitSourceNear(nearness(),projectionSamples()).value().values,
         fitSourceNear(nearness(),projectionSamples()).value().values);
}
} // namespace
int main() {
    try {
        basisAndReconstruction();fittingAndOwnership();rotationSemantics();boundaries();
        std::cout<<"harmonic service checks="<<checks<<"; max_abs="<<std::setprecision(17)<<maximum_error<<'\n';
        return 0;
    }catch(const std::exception& e){std::cerr<<"harmonic services: "<<e.what()<<'\n';return 1;}
}
