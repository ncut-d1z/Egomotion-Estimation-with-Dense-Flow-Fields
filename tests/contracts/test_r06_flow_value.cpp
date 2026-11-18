// 有限观测存储与物理使用条件分开；不新增归一化/数值容差。
#include "egomotion/domain/types.hpp"
#include "egomotion/contracts/result.hpp"
#include "egomotion/simulation/flow.hpp"
#include "egomotion/estimators/franz_kvd.hpp"
#include "egomotion/estimators/adaptive_step.hpp"

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
template<class T> struct CanTransfer<T,std::void_t<decltype(FlowField::fromOwned(std::declval<T>()))>>
    : std::true_type {};
void require(bool condition,const char* message) {
    ++checks;if(!condition)throw std::runtime_error(message);
}
void exact(const FlowMatrix& actual,const FlowMatrix& expected) {
    require(actual.rows()==expected.rows()&&actual.cols()==expected.cols(),"Nx6 shape preserved");
    if(actual.size()!=0)
        require(std::memcmp(actual.data(),expected.data(),static_cast<std::size_t>(actual.size())*sizeof(double))==0,
                "Column-major values preserved bitwise");
}
FlowField owned(const FlowMatrix& values) {
    auto result=FlowField::create(values);
    require(result.hasValue(),"Fixed finite storage accepted");
    return std::move(result).value();
}
void compileContracts() {
    static_assert(!std::is_aggregate_v<FlowField>);
    static_assert(!std::is_constructible_v<FlowField,FlowMatrix>);
    static_assert(CanTransfer<FlowMatrix&&>::value);
    static_assert(!CanTransfer<FlowMatrix&>::value&&!CanTransfer<const FlowMatrix&&>::value);
    static_assert(!CanTransfer<Eigen::MatrixXd&&>::value&&!CanTransfer<Eigen::Matrix<double,3,2>&&>::value);
    static_assert(!CanTransfer<Eigen::Matrix<double,Eigen::Dynamic,6,Eigen::RowMajor>&&>::value);
    static_assert(FlowMatrix::RowsAtCompileTime==Eigen::Dynamic&&FlowMatrix::ColsAtCompileTime==6);
    static_assert(!FlowMatrix::IsRowMajor);
    static_assert(std::is_same_v<decltype(std::declval<FlowField&>().observations()),const FlowMatrix&>);
    static_assert(!std::is_assignable_v<decltype(std::declval<FlowField&>().observations()(0,0)),double>);
    static_assert(std::is_same_v<decltype(std::declval<FlowField&&>().observations()),FlowMatrix>);
    static_assert(std::is_same_v<decltype(std::declval<const FlowField&&>().observations()),FlowMatrix>);
    static_assert(std::is_nothrow_move_constructible_v<FlowField>&&std::is_nothrow_move_assignable_v<FlowField>);
    static_assert(std::is_copy_constructible_v<FlowField>&&std::is_copy_assignable_v<FlowField>);
    static_assert(!std::is_default_constructible_v<Result<FlowField,NumericalErrorCode>>);
}
void finiteStorage() {
    const FlowField empty;
    require(empty.observations().rows()==0&&empty.observations().cols()==6,"Explicit empty Nx6 default");
    for(Eigen::Index count:{0,1,2,7,19})for(double magnitude:{0.0,-0.0,0.125,-2.0,
                    std::numeric_limits<double>::denorm_min(),std::numeric_limits<double>::max()}){
        FlowMatrix expected(count,6);
        for(Eigen::Index col=0;col<6;++col)for(Eigen::Index row=0;row<count;++row)
            expected(row,col)=(row+col)%2?-magnitude:magnitude;
        Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic,Eigen::RowMajor> input=expected;
        auto result=FlowField::create(input);
        require(result.hasValue(),"Finite nonunit, zero, extreme, signed storage accepted");
        auto value=std::move(result).value();input.resize(0,0);exact(value.observations(),expected);
        const auto copied=value;auto moved=std::move(value);
        exact(copied.observations(),expected);exact(moved.observations(),expected);
        require(value.observations().cols()==6&&value.observations().allFinite(),"Moved-from storage remains valid");
        moved=empty;exact(copied.observations(),expected);exact(moved.observations(),empty.observations());
        moved=copied;FlowField assigned;assigned=std::move(moved);exact(assigned.observations(),expected);
        assigned=assigned;exact(assigned.observations(),expected);
        const auto from_temporary=FlowField(copied).observations();exact(from_temporary,expected);
        const FlowField constant=copied;const auto const_owned=std::move(constant).observations();exact(const_owned,expected);
    }
    FlowMatrix indexed(7,6);
    for(Eigen::Index row=0;row<7;++row)for(Eigen::Index col=0;col<6;++col)indexed(row,col)=10*row+col+0.5;
    const auto value=owned(indexed);
    for(Eigen::Index row=0;row<7;++row)for(Eigen::Index col=0;col<6;++col)
        require(value.observations().data()[7*col+row]==10*row+col+0.5,"MATLAB column-major linear indexing");
}
void rejection() {
    constexpr std::array<std::pair<int,int>,8> shapes{{{0,0},{1,0},{3,1},{3,3},{3,5},{2,7},{1,42},{42,1}}};
    for(const auto& [rows,cols]:shapes){
        const Eigen::MatrixXd invalid=Eigen::MatrixXd::Zero(rows,cols);
        const auto result=FlowField::create(invalid);
        require(!result.hasValue()&&result.error().code==NumericalErrorCode::InvalidShape&&
                result.error().detail.stage==ErrorStage::Simulation&&!result.error().detail.observation_index,
                "Shape rejected before fixed-width storage conversion");
    }
    for(double bad:{std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity(),
                    -std::numeric_limits<double>::infinity()})
        for(Eigen::Index row=0;row<7;++row)for(Eigen::Index col=0;col<6;++col){
            FlowMatrix raw=FlowMatrix::Zero(7,6);raw(row,col)=bad;
            const auto saved=raw;const auto* address=raw.data();
            const auto copied=FlowField::create(raw),taken=FlowField::fromOwned(std::move(raw));
            for(const auto* result:{&copied,&taken})
                require(!result->hasValue()&&result->error().code==NumericalErrorCode::NonFiniteInput&&
                        result->error().detail.stage==ErrorStage::Simulation&&
                        result->error().detail.observation_index==static_cast<std::size_t>(row)&&
                        result->error().detail.causes.empty(),"Every invalid component reports its real row");
            exact(raw,saved);require(raw.data()==address,"Failed ownership transfer does not consume input");
        }
    Eigen::MatrixXd wrong=Eigen::MatrixXd::Constant(3,2,std::numeric_limits<double>::quiet_NaN());
    const auto shape=FlowField::create(wrong);
    require(!shape.hasValue()&&shape.error().code==NumericalErrorCode::InvalidShape,"Shape before finite values");
    FlowMatrix multiple=FlowMatrix::Zero(7,6);multiple(1,5)=std::numeric_limits<double>::quiet_NaN();multiple(5,0)=std::numeric_limits<double>::quiet_NaN();
    const auto first=FlowField::create(multiple);
    require(!first.hasValue()&&first.error().detail.observation_index==1,"First invalid row, not column scan order");
}
void ownershipAndAllocation() {
    FlowMatrix source=FlowMatrix::Constant(19,6,0.125);const auto reference=source;
    const auto* address=source.data();allocation_probe::armed=true;
    auto taken=FlowField::fromOwned(std::move(source));
    const bool no_allocation=allocation_probe::armed;allocation_probe::armed=false;
    require(no_allocation&&taken.hasValue(),"Ownership factory requires no copy allocation");
    require(taken.value().observations().data()==address,"Exact ownership transfer of source buffer");
    require(source.rows()==0&&source.cols()==6,"Successful transfer leaves an empty valid source");
    auto value=std::move(taken).value();
    auto target=owned(FlowMatrix::Constant(2,6,0.5));const auto prior=target;
    const auto* prior_address=target.observations().data();
    allocation_probe::armed=true;const auto failed=FlowField::create(value.observations());
    const bool copy_attempted=!allocation_probe::armed;allocation_probe::armed=false;
    require(copy_attempted&&!failed.hasValue()&&failed.error().code==NumericalErrorCode::ResourceLimitExceeded&&
            failed.error().detail.stage==ErrorStage::Simulation,"Copy allocation failure is a typed failure");
    exact(value.observations(),reference);
    bool threw=false;allocation_probe::armed=true;
    try{target=value;}catch(const std::bad_alloc&){threw=true;}
    const bool assignment_attempted=!allocation_probe::armed;allocation_probe::armed=false;
    require(threw&&assignment_attempted,"Whole-value copy assignment propagates allocation failure");
    exact(target.observations(),prior.observations());exact(value.observations(),reference);
    require(target.observations().data()==prior_address,"Failed assignment retains original buffer");
    target=value;exact(target.observations(),reference);
    const FlowField constant=value;const double* volatile copied_address=nullptr;
    threw=false;allocation_probe::armed=true;
    try{const auto copy=std::move(constant).observations();copied_address=copy.data();}
    catch(const std::bad_alloc&){threw=true;}
    const bool getter_attempted=!allocation_probe::armed;allocation_probe::armed=false;
    require(threw&&getter_attempted&&copied_address==nullptr,"Const rvalue getter copies and may report allocation failure");
    exact(constant.observations(),reference);
    allocation_probe::armed=true;auto released=std::move(value).observations();
    const bool getter_no_allocation=allocation_probe::armed;allocation_probe::armed=false;
    require(getter_no_allocation&&released.data()==address,"Rvalue getter transfers owned buffer without allocation");
    exact(released,reference);
    require(value.observations().rows()==0&&value.observations().cols()==6,"Released value remains valid empty storage");
    require(allocation_probe::failures==3,"All three deliberate copy allocation faults were exercised");
}
class UnusedRandom final:public IRandomSource {
public:
    std::size_t calls=0;
    Result<Scalar,NumericalErrorCode> uniform() override {++calls;return Result<Scalar,NumericalErrorCode>::success(0);}
    Result<Scalar,NumericalErrorCode> normal() override {++calls;return Result<Scalar,NumericalErrorCode>::success(0);}
};
void physicalBoundaries() {
    const simulation::NoNoise no_noise;UnusedRandom rng;
    const estimators::FranzMatchedFilter franz;
    const estimators::AdaptiveMfaStepSolver adaptive;
    for(double x:{0.0,2.0}){
        FlowMatrix raw=FlowMatrix::Zero(1,6);raw(0,0)=x;const auto field=owned(raw);
        const auto noise=no_noise.apply(field,0,rng);
        const auto first=franz.estimateWithTrace(field);
        const auto step=adaptive.solve(HarmonicCoefficients::zeroOrderUnit(),field);
        require(!noise.hasValue()&&noise.error().code==NumericalErrorCode::InvalidDirection&&
                noise.error().detail.observation_index==0,"Noise retains zero/nonunit direction rejection");
        require(!first.hasValue()&&first.error().code==NumericalErrorCode::InvalidDirection&&
                first.error().detail.observation_index==0,"Franz retains zero/nonunit direction rejection");
        require(!step.hasValue()&&step.error().code==NumericalErrorCode::InvalidDirection&&
                step.error().detail.observation_index==0,"Adaptive retains zero/nonunit direction rejection");
    }
    const auto noise=no_noise.apply(FlowField{},0,rng);
    const auto first=franz.estimateWithTrace(FlowField{});
    const auto step=adaptive.solve(HarmonicCoefficients::zeroOrderUnit(),FlowField{});
    require(!noise.hasValue()&&noise.error().code==NumericalErrorCode::EmptyInput,"Noise empty failure unchanged");
    require(!first.hasValue()&&first.error().code==NumericalErrorCode::EmptyInput,"Franz empty failure unchanged");
    require(!step.hasValue()&&step.error().code==NumericalErrorCode::EmptyInput,"Adaptive empty failure unchanged");
    FlowMatrix radial(1,6);radial<<1,0,0,2,3,4;
    const auto stored=owned(radial);const auto unchanged=no_noise.apply(stored,0,rng);
    require(unchanged.hasValue(),"NoNoise retains finite flow without adding tangency rejection");
    exact(unchanged.value().observations(),radial);
    require(rng.calls==0,"NoNoise checks/copy require no random draws");
}
} // namespace
int main() {
    try {
        compileContracts();finiteStorage();rejection();ownershipAndAllocation();physicalBoundaries();
        std::cout<<"flow value checks="<<checks<<"; finite patterns=30; shape faults=8; component faults=126; allocation faults="
                 <<allocation_probe::failures<<'\n';return 0;
    }catch(const std::exception& error){allocation_probe::armed=false;std::cerr<<"flow value: "<<error.what()<<'\n';return 1;}
}
