#include "egomotion/contracts/result.hpp"

namespace egomotion {
Result<MotionEstimate,NumericalErrorCode> MotionEstimate::create(
    const Eigen::Ref<const Eigen::MatrixXd>& translation,
    const Eigen::Ref<const Eigen::MatrixXd>& rotation) {
    using Output=Result<MotionEstimate,NumericalErrorCode>;
    if(translation.rows()!=3||translation.cols()!=1||rotation.rows()!=3||rotation.cols()!=1){
        ErrorDetail detail;detail.stage=ErrorStage::Solve;
        detail.message="MotionEstimate requires two 3x1 column vectors";
        return Output::failure({NumericalErrorCode::InvalidShape,std::move(detail)});
    }
    if(!translation.allFinite()||!rotation.allFinite()){
        ErrorDetail detail;detail.stage=ErrorStage::Solve;
        detail.message="MotionEstimate requires finite translation and rotation";
        return Output::failure({NumericalErrorCode::NonFiniteInput,std::move(detail)});
    }
    // 不改尺度/符号/单位，不归一化，也不拒绝有限零向量。
    return Output::success(MotionEstimate(translation,rotation));
}
}  // namespace egomotion
