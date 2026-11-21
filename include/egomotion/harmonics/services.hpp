#pragma once

#include "egomotion/contracts/result.hpp"

namespace egomotion::harmonics {

// 固定l<=2、源MATLAB/CS的3x5布局；无效六项填零，不使用实验或会话状态。
class RealSphericalHarmonics final {
public:
    Result<HarmonicMatrix,NumericalErrorCode> sourceBasis(const Vector3& direction) const;
    Result<Scalar,NumericalErrorCode> reconstructNear(
        const HarmonicCoefficients& coefficients,const Vector3& direction) const;
};

// 原koeff_aus_near逐通道投影与4pi顺序；只使用N×6的前三列，不是联合最小二乘。
class HarmonicFitter final {
public:
    Result<HarmonicCoefficients,NumericalErrorCode> fitSourceNear(
        const Eigen::VectorXd& nearness,const FlowField& observations) const;
    // 只需方向时无需构造光流；原N×K源矩阵应先显式提取前三列。
    Result<HarmonicCoefficients,NumericalErrorCode> fitSourceNear(
        const Eigen::VectorXd& nearness,const DirectionSet& directions) const;
};

class HarmonicRotator final {
public:
    // 物理场作用mu_new(d)=mu_old(A*d)，调用方明确选择A或A^T。
    Result<HarmonicCoefficients,NumericalErrorCode> rotateCoefficients(
        const HarmonicCoefficients& coefficients,const Matrix3& argument_rotation) const;
    // 原transfor_koeff系数动作：无CS旋转块直接乘源CS系数，与上式分开命名。
    Result<HarmonicCoefficients,NumericalErrorCode> rotateSourceCoefficients(
        const HarmonicCoefficients& coefficients,const Matrix3& source_rotation) const;
};

} // namespace egomotion::harmonics
