// 有限存储与物理单位方向分开；不新增归一化/数值容差。
#include "egomotion/domain/types.hpp"
#include "egomotion/contracts/result.hpp"
#include "egomotion/simulation/flow.hpp"
#include "egomotion/trajectory/trajectory.hpp"

#include <array>
#include <cstring>
#include <iostream>
#include <limits>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace allocation_probe {
bool armed=false;
std::size_t failures=0;
}
extern "C" void* __real_malloc(std::size_t);
extern "C" void* __wrap_malloc(std::size_t size) {
    if(allocation_probe::armed){
        allocation_probe::armed=false;++allocation_probe::failures;return nullptr;
    }
    return __real_malloc(size);
}
namespace {
using namespace egomotion;
std::size_t checks=0;
template<class T,class=void> struct CanTransfer : std::false_type {};
template<class T> struct CanTransfer<T,std::void_t<decltype(DirectionSet::fromOwned(std::declval<T>()))>>
    : std::true_type {};
void require(bool condition,const char* message) {
    ++checks;if(!condition)throw std::runtime_error(message);
}
void exact(const DirectionMatrix& actual,const DirectionMatrix& expected) {
    require(actual.rows()==expected.rows()&&actual.cols()==expected.cols(),"Nx3 shape preserved");
    if(actual.size()!=0)
        require(std::memcmp(actual.data(),expected.data(),static_cast<std::size_t>(actual.size())*sizeof(double))==0,
                "Column-major values preserved bitwise");
}
DirectionSet owned(const DirectionMatrix& values) {
    auto result=DirectionSet::create(values);
    require(result.hasValue(),"Fixed finite storage accepted");
    return std::move(result).value();
}
void compileContracts() {
    static_assert(!std::is_aggregate_v<DirectionSet>);
    static_assert(!std::is_constructible_v<DirectionSet,DirectionMatrix>);
    static_assert(CanTransfer<DirectionMatrix&&>::value);
    static_assert(!CanTransfer<DirectionMatrix&>::value&&!CanTransfer<const DirectionMatrix&&>::value);
    static_assert(!CanTransfer<Eigen::MatrixXd&&>::value&&!CanTransfer<Eigen::Matrix<double,3,2>&&>::value);
    static_assert(!CanTransfer<Eigen::Matrix<double,Eigen::Dynamic,3,Eigen::RowMajor>&&>::value);
    static_assert(DirectionMatrix::RowsAtCompileTime==Eigen::Dynamic&&DirectionMatrix::ColsAtCompileTime==3);
    static_assert(!DirectionMatrix::IsRowMajor);
    static_assert(std::is_same_v<decltype(std::declval<DirectionSet&>().directions()),const DirectionMatrix&>);
    static_assert(!std::is_assignable_v<decltype(std::declval<DirectionSet&>().directions()(0,0)),double>);
    static_assert(std::is_same_v<decltype(std::declval<DirectionSet&&>().directions()),DirectionMatrix>);
    static_assert(std::is_same_v<decltype(std::declval<const DirectionSet&&>().directions()),DirectionMatrix>);
    static_assert(std::is_nothrow_move_constructible_v<DirectionSet>&&std::is_nothrow_move_assignable_v<DirectionSet>);
    static_assert(std::is_copy_constructible_v<DirectionSet>&&std::is_copy_assignable_v<DirectionSet>);
    static_assert(!std::is_default_constructible_v<Result<DirectionSet,NumericalErrorCode>>);
}
void finiteStorage() {
    const DirectionSet empty;
    require(empty.directions().rows()==0&&empty.directions().cols()==3,"Explicit empty Nx3 default");
    for(Eigen::Index count:{0,1,2,7,19})for(double magnitude:{0.0,-0.0,0.125,-2.0,
                    std::numeric_limits<double>::denorm_min(),std::numeric_limits<double>::max()}){
        DirectionMatrix expected(count,3);
        for(Eigen::Index col=0;col<3;++col)for(Eigen::Index row=0;row<count;++row)
            expected(row,col)=(row+col)%2?-magnitude:magnitude;
        Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic,Eigen::RowMajor> input=expected;
        auto result=DirectionSet::create(input);
        require(result.hasValue(),"Finite nonunit, zero, extreme, signed storage accepted");
        auto value=std::move(result).value();input.resize(0,0);exact(value.directions(),expected);
        const auto copied=value;auto moved=std::move(value);
        exact(copied.directions(),expected);exact(moved.directions(),expected);
        require(value.directions().cols()==3&&value.directions().allFinite(),"Moved-from storage remains valid");
        moved=empty;exact(copied.directions(),expected);exact(moved.directions(),empty.directions());
        moved=copied;DirectionSet assigned;assigned=std::move(moved);exact(assigned.directions(),expected);
        assigned=assigned;exact(assigned.directions(),expected);
        const auto from_temporary=DirectionSet(copied).directions();exact(from_temporary,expected);
        const DirectionSet constant=copied;const auto const_owned=std::move(constant).directions();exact(const_owned,expected);
    }
    DirectionMatrix indexed(7,3);
    for(Eigen::Index row=0;row<7;++row)for(Eigen::Index col=0;col<3;++col)indexed(row,col)=10*row+col+0.5;
    const auto value=owned(indexed);
    for(Eigen::Index row=0;row<7;++row)for(Eigen::Index col=0;col<3;++col)
        require(value.directions().data()[7*col+row]==10*row+col+0.5,"MATLAB column-major linear indexing");
}
void rejection() {
    constexpr std::array<std::pair<int,int>,8> shapes{{{0,0},{1,0},{3,1},{3,2},{3,4},{2,7},{1,21},{21,1}}};
    for(const auto& [rows,cols]:shapes){
        const Eigen::MatrixXd invalid=Eigen::MatrixXd::Zero(rows,cols);
        const auto result=DirectionSet::create(invalid);
        require(!result.hasValue()&&result.error().code==NumericalErrorCode::InvalidShape&&
                result.error().detail.stage==ErrorStage::Simulation&&!result.error().detail.observation_index,
                "Shape rejected before fixed-width storage conversion");
    }
    for(double bad:{std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity(),
                    -std::numeric_limits<double>::infinity()})
        for(Eigen::Index row=0;row<7;++row)for(Eigen::Index col=0;col<3;++col){
            DirectionMatrix raw=DirectionMatrix::Zero(7,3);raw(row,col)=bad;
            const auto saved=raw;const auto* address=raw.data();
            const auto copied=DirectionSet::create(raw),taken=DirectionSet::fromOwned(std::move(raw));
            for(const auto* result:{&copied,&taken})
                require(!result->hasValue()&&result->error().code==NumericalErrorCode::NonFiniteInput&&
                        result->error().detail.stage==ErrorStage::Simulation&&
                        result->error().detail.observation_index==static_cast<std::size_t>(row)&&
                        result->error().detail.causes.empty(),"Every invalid component reports its real row");
            exact(raw,saved);require(raw.data()==address,"Failed ownership transfer does not consume input");
        }
    Eigen::MatrixXd wrong=Eigen::MatrixXd::Constant(3,2,std::numeric_limits<double>::quiet_NaN());
    const auto shape=DirectionSet::create(wrong);
    require(!shape.hasValue()&&shape.error().code==NumericalErrorCode::InvalidShape,"Shape before finite values");
    DirectionMatrix multiple=DirectionMatrix::Zero(7,3);multiple(1,2)=std::numeric_limits<double>::quiet_NaN();multiple(5,0)=std::numeric_limits<double>::quiet_NaN();
    const auto first=DirectionSet::create(multiple);
    require(!first.hasValue()&&first.error().detail.observation_index==1,"First invalid row, not column scan order");
}
void ownershipAndAllocation() {
    DirectionMatrix source=DirectionMatrix::Constant(19,3,0.125);const auto reference=source;
    const auto* address=source.data();allocation_probe::armed=true;
    auto taken=DirectionSet::fromOwned(std::move(source));
    const bool no_allocation=allocation_probe::armed;allocation_probe::armed=false;
    require(no_allocation&&taken.hasValue(),"Ownership factory requires no copy allocation");
    require(taken.value().directions().data()==address,"Exact ownership transfer of source buffer");
    require(source.rows()==0&&source.cols()==3,"Successful transfer leaves an empty valid source");
    auto value=std::move(taken).value();
    auto target=owned(DirectionMatrix::Constant(2,3,0.5));const auto prior=target;
    const auto* prior_address=target.directions().data();
    allocation_probe::armed=true;const auto failed=DirectionSet::create(value.directions());
    const bool copy_attempted=!allocation_probe::armed;allocation_probe::armed=false;
    require(copy_attempted&&!failed.hasValue()&&failed.error().code==NumericalErrorCode::ResourceLimitExceeded&&
            failed.error().detail.stage==ErrorStage::Simulation,"Copy allocation failure is a typed failure");
    exact(value.directions(),reference);
    bool threw=false;allocation_probe::armed=true;
    try{target=value;}catch(const std::bad_alloc&){threw=true;}
    const bool assignment_attempted=!allocation_probe::armed;allocation_probe::armed=false;
    require(threw&&assignment_attempted,"Whole-value copy assignment propagates allocation failure");
    exact(target.directions(),prior.directions());exact(value.directions(),reference);
    require(target.directions().data()==prior_address,"Failed assignment retains original buffer");
    target=value;exact(target.directions(),reference);
    const DirectionSet constant=value;const double* volatile copied_address=nullptr;
    threw=false;allocation_probe::armed=true;
    try{const auto copy=std::move(constant).directions();copied_address=copy.data();}
    catch(const std::bad_alloc&){threw=true;}
    const bool getter_attempted=!allocation_probe::armed;allocation_probe::armed=false;
    require(threw&&getter_attempted&&copied_address==nullptr,"Const rvalue getter copies and may report allocation failure");
    exact(constant.directions(),reference);
    allocation_probe::armed=true;auto released=std::move(value).directions();
    const bool getter_no_allocation=allocation_probe::armed;allocation_probe::armed=false;
    require(getter_no_allocation&&released.data()==address,"Rvalue getter transfers owned buffer without allocation");
    exact(released,reference);
    require(value.directions().rows()==0&&value.directions().cols()==3,"Released value remains valid empty storage");
    require(allocation_probe::failures==3,"All three deliberate copy allocation faults were exercised");
}
void physicalBoundaries() {
    const simulation::FlowSynthesizer flow;const trajectory::SphereEnvironment sphere;
    for(double x:{0.0,2.0}){
        DirectionMatrix raw(1,3);raw<<x,0,0;const auto direction=owned(raw);
        const auto ideal=flow.synthesize(direction,Eigen::VectorXd::Ones(1),Vector3::UnitX(),Vector3::Zero());
        const auto near=sphere.nearnessInUnitSphere(direction,Vector3::Zero(),Vector3::Zero());
        require(!ideal.hasValue()&&ideal.error().code==NumericalErrorCode::InvalidDirection&&
                ideal.error().detail.observation_index==0,"Flow retains zero/nonunit direction rejection");
        require(!near.hasValue()&&near.error().code==NumericalErrorCode::InvalidDirection&&
                near.error().detail.observation_index==0,"Sphere retains zero/nonunit direction rejection");
    }
    const auto ideal=flow.synthesize(DirectionSet{},Eigen::VectorXd{},Vector3::UnitX(),Vector3::Zero());
    const auto near=sphere.nearnessInUnitSphere(DirectionSet{},Vector3::Zero(),Vector3::Zero());
    require(!ideal.hasValue()&&ideal.error().code==NumericalErrorCode::InvalidShape,"Flow empty failure unchanged");
    require(!near.hasValue()&&near.error().code==NumericalErrorCode::EmptyInput,"Sphere empty failure unchanged");
}
} // namespace
int main() {
    try {
        compileContracts();finiteStorage();rejection();ownershipAndAllocation();physicalBoundaries();
        std::cout<<"direction value checks="<<checks<<"; finite patterns=30; shape faults=8; component faults=63; allocation faults="
                 <<allocation_probe::failures<<'\n';return 0;
    }catch(const std::exception& error){allocation_probe::armed=false;std::cerr<<"direction value: "<<error.what()<<'\n';return 1;}
}
