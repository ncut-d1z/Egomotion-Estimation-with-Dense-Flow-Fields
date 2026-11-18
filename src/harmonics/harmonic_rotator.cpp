#include "egomotion/harmonics/services.hpp"
#include "service_error.hpp"

namespace egomotion::harmonics {
namespace {
Result<HarmonicCoefficients,NumericalErrorCode> toggleSourcePhase(const HarmonicCoefficients& input) {
    HarmonicMatrix coefficients=input.values();
    // S1=diag(-1,1,-1)，S2=diag(1,-1,1,-1,1)，m=-l..l。
    coefficients(1,0)=-coefficients(1,0);
    coefficients(1,2)=-coefficients(1,2);
    coefficients(2,1)=-coefficients(2,1);
    coefficients(2,3)=-coefficients(2,3);
    return HarmonicCoefficients::create(coefficients);
}
} // namespace

Result<HarmonicCoefficients,NumericalErrorCode> HarmonicRotator::rotateCoefficients(
    const HarmonicCoefficients& coeff, const Matrix3& argument_rotation) const {
    auto converted=low_order::fromMatlabLayout(coeff.values());
    if (!converted.hasValue())
        return detail::bridge<HarmonicCoefficients>(converted.error(),"SH rotation layout");
    auto rotated=low_order::rotate(converted.value(),argument_rotation,
                                    low_order::RotationMethod::Ivanic1996);
    if (!rotated.hasValue())
        return detail::bridge<HarmonicCoefficients>(rotated.error(),"SH rotation");
    auto returned=low_order::toMatlabLayout(rotated.value());
    if (!returned.hasValue())
        return detail::bridge<HarmonicCoefficients>(returned.error(),"SH output layout");
    return HarmonicCoefficients::create(returned.value());
}

Result<HarmonicCoefficients,NumericalErrorCode> HarmonicRotator::rotateSourceCoefficients(
    const HarmonicCoefficients& coeff,const Matrix3& source_rotation) const {
    // 物理源CS动作 D_CS=S R_source S，故 S D_CS S=R_source。
    // 复用已按原论文/勘误独立实现的Eigen递推；不复制上游SHRotate代码。
    // 这只恢复原transfor_koeff的组合行为，不更改叶子的物理合同。
    auto phased=toggleSourcePhase(coeff);
    if(!phased.hasValue())return phased;
    auto rotated=rotateCoefficients(phased.value(),source_rotation);
    if(!rotated.hasValue())return rotated;
    return toggleSourcePhase(rotated.value());
}

} // namespace egomotion::harmonics
