#include "egomotion/estimators/coefficient_update.hpp"
#include "egomotion/harmonics/services.hpp"

#include <utility>

namespace egomotion::estimators {
namespace {
using Output=Result<HarmonicCoefficients,NumericalErrorCode>;
constexpr std::size_t kFirstMatlabFrame=2;
Output fail(NumericalErrorCode code,const char* message) {
    ErrorDetail detail;detail.stage=ErrorStage::Solve;detail.message=message;
    return Output::failure({code,std::move(detail)});
}
Output inherit(const HarmonicCoefficients& primary) {
    if(!primary.values().allFinite())
        return fail(NumericalErrorCode::NonFiniteInput,"Coefficient policy primary is non-finite");
    return Output::success(primary);
}
Output invalidFrame() {
    return fail(NumericalErrorCode::InvalidShape,"Coefficient policy requires MATLAB frame i>=2");
}
} // namespace

Result<HarmonicCoefficients,NumericalErrorCode> ResetCoefficientUpdatePolicy::prepare(
    std::size_t frame,const HarmonicCoefficients&,const HarmonicCoefficients&,
    const Matrix3&) const {
    if(frame<kFirstMatlabFrame)return invalidFrame();
    // 两份源脚本每帧重新构造ohnekoeff。
    return Output::success(HarmonicCoefficients::zeroOrderUnit());
}
Result<HarmonicCoefficients,NumericalErrorCode> InheritCoefficientUpdatePolicy::prepare(
    std::size_t frame,const HarmonicCoefficients& primary,const HarmonicCoefficients&,
    const Matrix3&) const {
    if(frame<kFirstMatlabFrame)return invalidFrame();
    return inherit(primary); // 不再次旋转已由Session旋转的akkoeff。
}
Result<HarmonicCoefficients,NumericalErrorCode> PeriodicCoefficientUpdatePolicy::prepare(
    std::size_t frame,const HarmonicCoefficients& primary,const HarmonicCoefficients& baseline,
    const Matrix3& rotation) const {
    if(frame<kFirstMatlabFrame)return invalidFrame();
    if(period_==0)
        return fail(NumericalErrorCode::InvalidShape,"Coefficient update period must be positive");
    // 原many_parameter先旋转所有旧基线，即使本帧随后刷新也不能跳过。
    const harmonics::HarmonicRotator rotator;
    auto rotated=rotator.rotateSourceCoefficients(baseline,rotation);
    if(!rotated.hasValue())return rotated;
    if(frame%period_==0)return inherit(primary);
    return rotated;
}
} // namespace egomotion::estimators
