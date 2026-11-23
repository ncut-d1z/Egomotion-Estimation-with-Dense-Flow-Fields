// 只验证系数存储不变量；不把有限性等同可重建/可观测的近度。
#include "egomotion/domain/types.hpp"
#include "egomotion/contracts/result.hpp"

#include <array>
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
void exact(const HarmonicMatrix& actual,const HarmonicMatrix& expected) {
    require(std::memcmp(actual.data(),expected.data(),15*sizeof(double))==0,"All 15 storage values preserved bitwise");
}
void compileContracts() {
    static_assert(!std::is_aggregate_v<HarmonicCoefficients>);
    static_assert(!std::is_constructible_v<HarmonicCoefficients,HarmonicMatrix>);
    static_assert(HarmonicMatrix::RowsAtCompileTime==3&&HarmonicMatrix::ColsAtCompileTime==5);
    static_assert(!HarmonicMatrix::IsRowMajor);
    static_assert(std::is_same_v<decltype(std::declval<HarmonicCoefficients&>().values()),const HarmonicMatrix&>);
    static_assert(!std::is_assignable_v<decltype(std::declval<HarmonicCoefficients&>().values()(0,0)),double>);
    static_assert(std::is_same_v<decltype(std::declval<HarmonicCoefficients&&>().values()),HarmonicMatrix>);
    static_assert(std::is_same_v<decltype(std::declval<const HarmonicCoefficients&&>().values()),HarmonicMatrix>);
    static_assert(std::is_copy_constructible_v<HarmonicCoefficients>&&std::is_move_constructible_v<HarmonicCoefficients>);
    static_assert(std::is_copy_assignable_v<HarmonicCoefficients>&&std::is_move_assignable_v<HarmonicCoefficients>);
    static_assert(!std::is_default_constructible_v<Result<HarmonicCoefficients,NumericalErrorCode>>);
}
void finiteStorage() {
    const HarmonicCoefficients zero;
    exact(zero.values(),HarmonicMatrix::Zero());
    HarmonicMatrix initial=HarmonicMatrix::Zero();initial(0,0)=1;
    exact(HarmonicCoefficients::zeroOrderUnit().values(),initial);
    for(double magnitude:{0.0,-0.0,0.125,-2.0,std::numeric_limits<double>::denorm_min(),
                          std::numeric_limits<double>::max()}){
        HarmonicMatrix expected;
        for(int col=0;col<5;++col)for(int row=0;row<3;++row)
            expected(row,col)=(row+col)%2?-magnitude:magnitude;
        Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic,Eigen::RowMajor> input=expected;
        auto result=HarmonicCoefficients::create(input);
        require(result.hasValue(),"Finite signed/padded storage is a valid domain value");
        auto value=std::move(result).value();input.resize(0,0);
        exact(value.values(),expected);
        const auto copied=value;auto moved=std::move(value);
        exact(copied.values(),expected);exact(moved.values(),expected);
        require(value.values().allFinite(),"Moved-from fixed-size storage remains finite");
        moved=zero;exact(copied.values(),expected);exact(moved.values(),HarmonicMatrix::Zero());
        moved=copied;HarmonicCoefficients assigned;assigned=std::move(moved);
        exact(assigned.values(),expected);
        const HarmonicMatrix owned=HarmonicCoefficients(copied).values();exact(owned,expected);
        const HarmonicCoefficients const_value=copied;
        const auto const_owned=std::move(const_value).values();exact(const_owned,expected);
    }
    // 行主序来源逐项读取为列主序，不转置、不清除六个非零未用位置。
    Eigen::Matrix<double,3,5,Eigen::RowMajor> indexed;
    for(int row=0;row<3;++row)for(int col=0;col<5;++col)indexed(row,col)=10*row+col+0.5;
    const auto result=HarmonicCoefficients::create(indexed);require(result.hasValue(),"Indexed row-major input accepted");
    for(int row=0;row<3;++row)for(int col=0;col<5;++col)
        require(result.value().values().data()[3*col+row]==10*row+col+0.5,"Exact MATLAB column-major linear index");
    require(!result.value().values().topRows<2>().isZero(0),"Finite unused values retained, not canonicalized");
}
void invalidStorage() {
    constexpr std::array<std::pair<int,int>,8> shapes{{{0,0},{0,5},{3,0},{5,3},{2,5},{3,4},{1,15},{15,1}}};
    for(const auto& [rows,cols]:shapes){
        const Eigen::MatrixXd invalid=Eigen::MatrixXd::Zero(rows,cols);
        const auto result=HarmonicCoefficients::create(invalid);
        require(!result.hasValue()&&result.error().code==NumericalErrorCode::InvalidShape&&
                result.error().detail.stage==ErrorStage::Solve,"Shape rejected before fixed-size assignment");
    }
    for(double bad:{std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity(),
                    -std::numeric_limits<double>::infinity()})
        for(int col=0;col<5;++col)for(int row=0;row<3;++row){
            HarmonicMatrix invalid=HarmonicMatrix::Zero();invalid(row,col)=bad;
            const auto result=HarmonicCoefficients::create(invalid);
            require(!result.hasValue()&&result.error().code==NumericalErrorCode::NonFiniteInput&&
                    result.error().detail.stage==ErrorStage::Solve,"Reject every non-finite active or unused component");
            require(result.error().detail.causes.empty()&&!result.error().detail.observation_index,
                    "No invented ray index or lower-level error for domain input");
        }
    const Eigen::MatrixXd invalid=Eigen::MatrixXd::Constant(5,3,std::numeric_limits<double>::quiet_NaN());
    const auto result=HarmonicCoefficients::create(invalid);
    require(!result.hasValue()&&result.error().code==NumericalErrorCode::InvalidShape,"Shape check precedes finite check");
}
} // namespace
int main() {
    try {
        compileContracts();finiteStorage();invalidStorage();
        std::cout<<"coefficient value checks="<<checks<<"; finite patterns=6; shape faults=8; component faults=45\n";
        return 0;
    }catch(const std::exception& error){std::cerr<<"coefficient value: "<<error.what()<<'\n';return 1;}
}
