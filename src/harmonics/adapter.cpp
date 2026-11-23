#include "egomotion/harmonics/adapter.hpp"

namespace egomotion::harmonics {

// 保留已公开自由函数的源码兼容；公式/错误映射只由对应服务实现拥有。
Result<Scalar,NumericalErrorCode> reconstructNear(
    const HarmonicCoefficients& coefficients,const Vector3& direction) {
    return RealSphericalHarmonics{}.reconstructNear(coefficients,direction);
}
Result<HarmonicCoefficients,NumericalErrorCode> fitSourceNear(
    const Eigen::VectorXd& nearness,const FlowField& observations) {
    return HarmonicFitter{}.fitSourceNear(nearness,observations);
}
Result<HarmonicCoefficients,NumericalErrorCode> rotateCoefficients(
    const HarmonicCoefficients& coefficients,const Matrix3& argument_rotation) {
    return HarmonicRotator{}.rotateCoefficients(coefficients,argument_rotation);
}
Result<HarmonicCoefficients,NumericalErrorCode> rotateSourceCoefficients(
    const HarmonicCoefficients& coefficients,const Matrix3& source_rotation) {
    return HarmonicRotator{}.rotateSourceCoefficients(coefficients,source_rotation);
}

} // namespace egomotion::harmonics
