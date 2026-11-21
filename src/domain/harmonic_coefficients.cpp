#include "egomotion/contracts/result.hpp"

namespace egomotion {
Result<HarmonicCoefficients,NumericalErrorCode> HarmonicCoefficients::create(
    const Eigen::Ref<const Eigen::MatrixXd>& values) {
    using Output=Result<HarmonicCoefficients,NumericalErrorCode>;
    if(values.rows()!=3||values.cols()!=5){
        ErrorDetail detail;detail.stage=ErrorStage::Solve;
        detail.message="HarmonicCoefficients requires a 3x5 matrix";
        return Output::failure({NumericalErrorCode::InvalidShape,std::move(detail)});
    }
    if(!values.allFinite()){
        ErrorDetail detail;detail.stage=ErrorStage::Solve;
        detail.message="HarmonicCoefficients requires all 15 storage values to be finite";
        return Output::failure({NumericalErrorCode::NonFiniteInput,std::move(detail)});
    }
    return Output::success(HarmonicCoefficients(values));
}
HarmonicCoefficients HarmonicCoefficients::zeroOrderUnit() noexcept {
    HarmonicMatrix values=HarmonicMatrix::Zero();values(0,0)=1.0;
    return HarmonicCoefficients(values);
}
}  // namespace egomotion
