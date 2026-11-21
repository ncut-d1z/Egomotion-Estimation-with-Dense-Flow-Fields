// 领域运动只检查形状/有限性；数学可观测性仍由原算法判定。
#include "egomotion/domain/types.hpp"
#include "egomotion/contracts/result.hpp"

#include <array>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace {
using namespace egomotion;
std::size_t checks=0;
void require(bool condition,const char* message) {
    ++checks;if(!condition)throw std::runtime_error(message);
}
void exact(const Vector3& actual,const Vector3& expected) {
    require(std::memcmp(actual.data(),expected.data(),3*sizeof(double))==0,"Preserve exact finite components and signed zero");
}
void compileContracts() {
    static_assert(!std::is_aggregate_v<MotionEstimate>);
    static_assert(!std::is_constructible_v<MotionEstimate,Vector3,Vector3>);
    static_assert(std::is_same_v<decltype(std::declval<MotionEstimate&>().translation()),const Vector3&>);
    static_assert(std::is_same_v<decltype(std::declval<MotionEstimate&>().rotation()),const Vector3&>);
    static_assert(!std::is_assignable_v<decltype(std::declval<MotionEstimate&>().translation()[0]),double>);
    static_assert(!std::is_assignable_v<decltype(std::declval<MotionEstimate&>().rotation()[0]),double>);
    static_assert(std::is_same_v<decltype(std::declval<MotionEstimate&&>().translation()),Vector3>);
    static_assert(std::is_same_v<decltype(std::declval<const MotionEstimate&&>().rotation()),Vector3>);
    static_assert(std::is_copy_constructible_v<MotionEstimate>&&std::is_move_constructible_v<MotionEstimate>);
    static_assert(std::is_copy_assignable_v<MotionEstimate>&&std::is_move_assignable_v<MotionEstimate>);
    static_assert(!std::is_default_constructible_v<Result<MotionEstimate,NumericalErrorCode>>);
}
void finiteValues() {
    const double tiny=std::numeric_limits<double>::denorm_min(),large=std::numeric_limits<double>::max();
    const std::array<Vector3,7> values{{Vector3::Zero(),Vector3{-0.0,0.0,-0.0},Vector3::UnitX(),
        Vector3{1,-2,3},Vector3{tiny,-tiny,0},Vector3{large,-large,1},Vector3{0.25,-0.5,0.75}}};
    const MotionEstimate stationary;
    exact(stationary.translation(),Vector3::Zero());exact(stationary.rotation(),Vector3::Zero());
    for(const auto& translation:values)for(const auto& rotation:values){
        Eigen::MatrixXd t=translation,w=rotation;
        auto result=MotionEstimate::create(t,w);
        require(result.hasValue(),"All finite motion magnitudes including stationary are valid domain values");
        auto value=std::move(result).value();
        t.resize(0,0);w.resize(0,0); // 工厂不能借用输入或Eigen临时表达式。
        exact(value.translation(),translation);exact(value.rotation(),rotation);
        const auto copy=value;
        auto moved=std::move(value);
        exact(copy.translation(),translation);exact(copy.rotation(),rotation);
        exact(moved.translation(),translation);exact(moved.rotation(),rotation);
        require(value.translation().allFinite()&&value.rotation().allFinite(),"Moved-from fixed-size value remains finite");
        moved=stationary;
        exact(copy.translation(),translation);exact(copy.rotation(),rotation);
        exact(moved.translation(),Vector3::Zero());exact(moved.rotation(),Vector3::Zero());
        moved=copy;
        MotionEstimate assigned;assigned=std::move(moved);
        exact(assigned.translation(),translation);exact(assigned.rotation(),rotation);
        const Vector3 temporary_t=MotionEstimate(copy).translation();
        const MotionEstimate temporary_w=copy;
        const Vector3 owned_w=std::move(temporary_w).rotation();
        exact(temporary_t,translation);exact(owned_w,rotation);
    }
}
void invalidValues() {
    const Eigen::MatrixXd valid=Vector3{1,2,3};
    constexpr std::array<std::pair<int,int>,8> shapes{{{0,0},{0,1},{3,0},{1,3},{2,1},{4,1},{3,2},{2,3}}};
    for(const auto& [rows,cols]:shapes)for(bool translation:{true,false}){
        const Eigen::MatrixXd bad=Eigen::MatrixXd::Zero(rows,cols);
        const auto result=translation?MotionEstimate::create(bad,valid):MotionEstimate::create(valid,bad);
        require(!result.hasValue()&&result.error().code==NumericalErrorCode::InvalidShape&&
                result.error().detail.stage==ErrorStage::Solve,"Reject shape before fixed-size Eigen conversion");
    }
    for(double invalid:{std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity(),
                        -std::numeric_limits<double>::infinity()})
        for(int component=0;component<3;++component)for(bool translation:{true,false}){
            Eigen::MatrixXd bad=valid;bad(component,0)=invalid;
            const auto result=translation?MotionEstimate::create(bad,valid):MotionEstimate::create(valid,bad);
            require(!result.hasValue()&&result.error().code==NumericalErrorCode::NonFiniteInput&&
                    result.error().detail.stage==ErrorStage::Solve,"Every non-finite component fails without a motion value");
            require(result.error().detail.causes.empty()&&!result.error().detail.observation_index,
                    "Do not invent an observation index or source failure");
        }
    Eigen::MatrixXd wrong=Eigen::MatrixXd::Constant(1,3,std::numeric_limits<double>::quiet_NaN());
    const auto result=MotionEstimate::create(wrong,valid);
    require(!result.hasValue()&&result.error().code==NumericalErrorCode::InvalidShape,"Shape precedes finite check");
}
} // namespace
int main() {
    try {
        compileContracts();finiteValues();invalidValues();
        std::cout<<"motion value checks="<<checks<<"; finite pairs=49; shape faults=16; component faults=18\n";
        return 0;
    } catch(const std::exception& error){std::cerr<<"motion value: "<<error.what()<<'\n';return 1;}
}
