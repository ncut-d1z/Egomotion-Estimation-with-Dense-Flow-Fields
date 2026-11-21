#include "egomotion/harmonics/services.hpp"
#include "service_error.hpp"

namespace egomotion::harmonics {
Result<Scalar,NumericalErrorCode> RealSphericalHarmonics::reconstructNear(
    const HarmonicCoefficients& coeff, const Vector3& direction) const {
    auto converted=low_order::fromMatlabLayout(coeff.values());
    if (!converted.hasValue())
        return detail::bridge<Scalar>(converted.error(),"Near reconstruction layout");
    auto scalar=low_order::reconstruct(converted.value(),direction);
    if (!scalar.hasValue())
        return detail::bridge<Scalar>(scalar.error(),"Near reconstruction");
    return Result<Scalar,NumericalErrorCode>::success(std::move(scalar).value());
}

Result<HarmonicMatrix,NumericalErrorCode> RealSphericalHarmonics::sourceBasis(
    const Vector3& direction) const {
    auto canonical=low_order::basisDirect(direction);
    if(!canonical.hasValue())
        return detail::bridge<HarmonicMatrix>(canonical.error(),"SH source basis");
    // 同场布局变换也是正交的基值置换/相位变换，保留l=0..2、m=-l..l。
    auto source=low_order::toMatlabLayout(canonical.value());
    if(!source.hasValue())
        return detail::bridge<HarmonicMatrix>(source.error(),"SH source basis layout");
    return Result<HarmonicMatrix,NumericalErrorCode>::success(std::move(source).value());
}

} // namespace egomotion::harmonics
