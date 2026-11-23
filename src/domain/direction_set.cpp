#include "egomotion/contracts/result.hpp"

#include <new>

namespace egomotion {
namespace {
Status<NumericalErrorCode> invalid(NumericalErrorCode code,const char* message,
                                  std::optional<std::size_t> row=std::nullopt) {
    ErrorDetail detail;detail.stage=ErrorStage::Simulation;
    detail.message=message;detail.observation_index=row;
    return Status<NumericalErrorCode>::failure({code,std::move(detail)});
}
Status<NumericalErrorCode> validate(const Eigen::Ref<const Eigen::MatrixXd>& directions) {
    if(directions.cols()!=3)
        return invalid(NumericalErrorCode::InvalidShape,"DirectionSet requires an Nx3 matrix");
    for(Eigen::Index row=0;row<directions.rows();++row)
        if(!directions.row(row).allFinite())
            // 公共错误位置沿用C++的0-based观测索引，不改变存储行序。
            return invalid(NumericalErrorCode::NonFiniteInput,"DirectionSet requires finite components",
                           static_cast<std::size_t>(row));
    return Status<NumericalErrorCode>::success();
}
} // namespace

Result<DirectionSet,NumericalErrorCode> DirectionSet::create(
    const Eigen::Ref<const Eigen::MatrixXd>& directions) {
    using Output=Result<DirectionSet,NumericalErrorCode>;
    auto checked=validate(directions);
    if(!checked.hasValue())return Output::failure(std::move(checked).error());
    try {
        DirectionMatrix owned=directions;
        return Output::success(DirectionSet(std::move(owned)));
    }catch(const std::bad_alloc&){
        return Output::failure(invalid(NumericalErrorCode::ResourceLimitExceeded,
            "Cannot allocate owned direction storage").error());
    }catch(const std::length_error&){
        return Output::failure(invalid(NumericalErrorCode::ResourceLimitExceeded,
            "Direction storage exceeds container capacity").error());
    }
}
Result<DirectionSet,NumericalErrorCode> DirectionSet::fromOwned(DirectionMatrix&& directions) {
    using Output=Result<DirectionSet,NumericalErrorCode>;
    auto checked=validate(directions);
    if(!checked.hasValue())return Output::failure(std::move(checked).error());
    return Output::success(DirectionSet(std::move(directions)));
}
DirectionSet& DirectionSet::operator=(const DirectionSet& other) {
    if(this!=&other){
        // 深拷贝失败不得破坏原有形状/有限性；完成后一次交换所有权。
        DirectionMatrix replacement=other.directions_;
        directions_.swap(replacement);
    }
    return *this;
}
} // namespace egomotion
