#include "egomotion/contracts/result.hpp"

#include <new>
#include <stdexcept>

namespace egomotion {
namespace {
Status<NumericalErrorCode> invalid(NumericalErrorCode code,const char* message,
                                  std::optional<std::size_t> row=std::nullopt) {
    ErrorDetail detail;detail.stage=ErrorStage::Simulation;
    detail.message=message;detail.observation_index=row;
    return Status<NumericalErrorCode>::failure({code,std::move(detail)});
}
Status<NumericalErrorCode> validate(const Eigen::Ref<const Eigen::MatrixXd>& observations) {
    if(observations.cols()!=6)
        return invalid(NumericalErrorCode::InvalidShape,"FlowField requires an Nx6 matrix");
    for(Eigen::Index row=0;row<observations.rows();++row)
        if(!observations.row(row).allFinite())
            // 公共错误位置沿用C++的0-based观测索引，不改变存储行序。
            return invalid(NumericalErrorCode::NonFiniteInput,"FlowField requires finite components",
                           static_cast<std::size_t>(row));
    return Status<NumericalErrorCode>::success();
}
} // namespace

Result<FlowField,NumericalErrorCode> FlowField::create(
    const Eigen::Ref<const Eigen::MatrixXd>& observations) {
    using Output=Result<FlowField,NumericalErrorCode>;
    auto checked=validate(observations);
    if(!checked.hasValue())return Output::failure(std::move(checked).error());
    try {
        FlowMatrix owned=observations;
        return Output::success(FlowField(std::move(owned)));
    }catch(const std::bad_alloc&){
        return Output::failure(invalid(NumericalErrorCode::ResourceLimitExceeded,
            "Cannot allocate owned flow storage").error());
    }catch(const std::length_error&){
        return Output::failure(invalid(NumericalErrorCode::ResourceLimitExceeded,
            "Flow storage exceeds container capacity").error());
    }
}
Result<FlowField,NumericalErrorCode> FlowField::fromOwned(FlowMatrix&& observations) {
    using Output=Result<FlowField,NumericalErrorCode>;
    auto checked=validate(observations);
    if(!checked.hasValue())return Output::failure(std::move(checked).error());
    return Output::success(FlowField(std::move(observations)));
}
FlowField& FlowField::operator=(const FlowField& other) {
    if(this!=&other){
        // 深拷贝失败不得破坏原有形状/有限性；完成后一次交换所有权。
        FlowMatrix replacement=other.observations_;
        observations_.swap(replacement);
    }
    return *this;
}
} // namespace egomotion
