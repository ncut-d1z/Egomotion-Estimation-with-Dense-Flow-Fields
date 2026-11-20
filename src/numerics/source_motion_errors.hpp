#pragma once

#include "egomotion/contracts/result.hpp"

#include <Eigen/Geometry>

#include <cmath>
#include <utility>

namespace egomotion::source_metrics {

// Bias 距离保留原公式；方向角和旋转测地角采用 DEC-R6-01/02 的所有者决定。
// transfehler_div.m blob: 89708055b98e0b6651990c08dc3850393ca8acad
inline Result<Scalar,NumericalErrorCode> normalizedVectorError(
    const Vector3& truth, const Vector3& estimate) {
    const double nt=truth.norm(), ne=estimate.norm();
    if (!std::isfinite(nt)||!std::isfinite(ne)) {
        ErrorDetail detail;
        detail.stage=ErrorStage::Solve;
        detail.message="Source transfehler_div input norm is non-finite";
        return Result<Scalar,NumericalErrorCode>::failure(
            {NumericalErrorCode::NonFiniteInput,std::move(detail)});
    }
    // transfehler_div yields mathematical zero for zero truth and nonzero
    // estimate, but that is NOT a valid error statistic; never count it.
    if (nt==0 || ne==0) {
        ErrorDetail detail;
        detail.stage=ErrorStage::Solve;
        detail.message="Source transfehler_div has undefined zero vector norm";
        return Result<Scalar,NumericalErrorCode>::failure(
            {NumericalErrorCode::ZeroNorm,std::move(detail)});
    }
    const double scaled=(truth-estimate*(nt/ne)).norm();
    if (!std::isfinite(scaled)) {
        ErrorDetail detail;
        detail.stage=ErrorStage::Solve;
        detail.message="Source-normalized motion error is non-finite";
        return Result<Scalar,NumericalErrorCode>::failure(
            {NumericalErrorCode::NonFiniteResult,std::move(detail)});
    }
    return Result<Scalar,NumericalErrorCode>::success(scaled);
}

namespace metric_detail {
// 调用者先验证有限且非零。最大分量缩放使范数计算落在 [1,sqrt(3)]，
// 因而极大/极小的有限输入不会因平方范数上溢/下溢丢失有效方向。
inline Vector3 scaledUnitDirection(const Vector3& vector,double maximum) {
    // 明确逐分量相除；不先形成 1/maximum，次正规缩放因子也不会倒数上溢。
    const Vector3 scaled(vector.x()/maximum,vector.y()/maximum,vector.z()/maximum);
    return scaled/std::hypot(scaled.x(),scaled.y(),scaled.z());
}

inline Result<Matrix3,NumericalErrorCode> metricRotationMatrix(const Vector3& rotation) {
    ErrorDetail detail;
    detail.stage=ErrorStage::Solve;
    if (!rotation.allFinite()) {
        detail.message="SO(3) error input rotation vector is non-finite";
        return Result<Matrix3,NumericalErrorCode>::failure(
            {NumericalErrorCode::NonFiniteInput,std::move(detail)});
    }
    const double maximum=rotation.cwiseAbs().maxCoeff();
    if (maximum==0.0)
        return Result<Matrix3,NumericalErrorCode>::success(Matrix3::Identity());
    const Vector3 scaled(rotation.x()/maximum,rotation.y()/maximum,rotation.z()/maximum);
    const double scaled_norm=std::hypot(scaled.x(),scaled.y(),scaled.z());
    const double radians=maximum*scaled_norm;
    if (!std::isfinite(radians)) {
        detail.message="SO(3) error rotation magnitude is not representable";
        return Result<Matrix3,NumericalErrorCode>::failure(
            {NumericalErrorCode::NonFiniteResult,std::move(detail)});
    }
    // AngleAxis 的 Rodrigues 矩阵就是 Exp([rotation]_x)，不改变物理轨迹实现。
    const Matrix3 matrix=Eigen::AngleAxisd(radians,scaled/scaled_norm).toRotationMatrix();
    if (!matrix.allFinite()) {
        detail.message="SO(3) error exponential produced a non-finite matrix";
        return Result<Matrix3,NumericalErrorCode>::failure(
            {NumericalErrorCode::NonFiniteResult,std::move(detail)});
    }
    return Result<Matrix3,NumericalErrorCode>::success(matrix);
}
}  // namespace metric_detail

// 保留调用名称兼容性；DEC-R6-01 明确批准以稳定 atan2 替代原
// normrelfehler.m 的 abs(acos(c))，不声称域外/舍入行为仍与源表达式相同。
// 原文件 blob: c9fecc37180ab13d854a144d333a2599e601a9a8。
// 仅零向量没有方向；非零有限向量不因 acos 端点舍入失败。
inline Result<Scalar,NumericalErrorCode> sourceDirectionErrorDegrees(
    const Vector3& truth,const Vector3& estimated) {
    ErrorDetail detail;
    detail.stage=ErrorStage::Solve;
    if (!truth.allFinite()||!estimated.allFinite()) {
        detail.message="Direction-angle input is non-finite";
        return Result<Scalar,NumericalErrorCode>::failure(
            {NumericalErrorCode::NonFiniteInput,std::move(detail)});
    }
    const double truth_maximum=truth.cwiseAbs().maxCoeff();
    const double estimated_maximum=estimated.cwiseAbs().maxCoeff();
    if (truth_maximum==0.0||estimated_maximum==0.0) {
        detail.message="Direction angle is undefined for zero vector";
        return Result<Scalar,NumericalErrorCode>::failure(
            {NumericalErrorCode::UndefinedAngle,std::move(detail)});
    }
    const Vector3 u=metric_detail::scaledUnitDirection(truth,truth_maximum);
    const Vector3 v=metric_detail::scaledUnitDirection(estimated,estimated_maximum);
    const Vector3 cross=u.cross(v);
    const double sine=std::hypot(cross.x(),cross.y(),cross.z());
    const double cosine=u.dot(v);
    const double pi=std::acos(-1.0);
    const double degrees=std::atan2(sine,cosine)*(180.0/pi);
    if (!std::isfinite(sine)||!std::isfinite(cosine)||!std::isfinite(degrees)) {
        detail.message="Direction-angle output is non-finite";
        return Result<Scalar,NumericalErrorCode>::failure(
            {NumericalErrorCode::NonFiniteResult,std::move(detail)});
    }
    return Result<Scalar,NumericalErrorCode>::success(degrees);
}

// DEC-R6-02：输入是同一帧、同一坐标系的弧度旋转向量，不是无量纲旋转轴。
// 零旋转对应单位阵。测地角由 Exp(truth)^T Exp(estimated) 决定，范围 [0,180] 度。
inline Result<Scalar,NumericalErrorCode> rotationGeodesicErrorDegrees(
    const Vector3& truth,const Vector3& estimated) {
    auto reference=metric_detail::metricRotationMatrix(truth);
    if (!reference.hasValue())
        return Result<Scalar,NumericalErrorCode>::failure(std::move(reference).error());
    auto computed=metric_detail::metricRotationMatrix(estimated);
    if (!computed.hasValue())
        return Result<Scalar,NumericalErrorCode>::failure(std::move(computed).error());
    const Matrix3 relative=reference.value().transpose()*computed.value();
    const Vector3 skew(relative(2,1)-relative(1,2),
                       relative(0,2)-relative(2,0),
                       relative(1,0)-relative(0,1));
    // 对 SO(3)，skew 范数/2=|sin(theta)|，(trace-1)/2=cos(theta)。
    // atan2 同时避免 theta 接近 0/pi 时 acos 的端点敏感性，不增加裁剪门限。
    const double sine=0.5*std::hypot(skew.x(),skew.y(),skew.z());
    const double cosine=0.5*(relative.trace()-1.0);
    const double degrees=std::atan2(sine,cosine)*(180.0/std::acos(-1.0));
    if (!relative.allFinite()||!std::isfinite(degrees)) {
        ErrorDetail detail;
        detail.stage=ErrorStage::Solve;
        detail.message="SO(3) geodesic error is non-finite";
        return Result<Scalar,NumericalErrorCode>::failure(
            {NumericalErrorCode::NonFiniteResult,std::move(detail)});
    }
    return Result<Scalar,NumericalErrorCode>::success(degrees);
}

}  // namespace egomotion::source_metrics
