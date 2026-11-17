#pragma once

#include "egomotion/contracts/result.hpp"

#include <cmath>
#include <utility>

namespace egomotion::source_metrics {

// 内部共享实现，分别保留原公式；距离与角度不可混用。
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

// normrelfehler.m blob: c9fecc37180ab13d854a144d333a2599e601a9a8
// 不钳制 acos 自变量；未定义的角度仍显式失败。
inline Result<Scalar,NumericalErrorCode> sourceDirectionErrorDegrees(
    const Vector3& truth,const Vector3& estimated) {
    const double reference_norm=std::sqrt(truth.dot(truth));
    const double computed_norm=std::sqrt(estimated.dot(estimated));
    ErrorDetail detail;
    detail.stage=ErrorStage::Solve;
    if (!std::isfinite(reference_norm)||!std::isfinite(computed_norm)) {
        detail.message="normrelfehler input norm is non-finite";
        return Result<Scalar,NumericalErrorCode>::failure(
            {NumericalErrorCode::NonFiniteInput,std::move(detail)});
    }
    if (reference_norm==0.0||computed_norm==0.0) {
        detail.message="normrelfehler angle is undefined for zero vector";
        return Result<Scalar,NumericalErrorCode>::failure(
            {NumericalErrorCode::UndefinedAngle,std::move(detail)});
    }
    const Vector3 u=truth/reference_norm,v=estimated/computed_norm;
    const double cosine=u.dot(v);
    if (!std::isfinite(cosine)||cosine < -1.0||cosine > 1.0) {
        detail.message="normrelfehler acos argument outside [-1,1]";
        return Result<Scalar,NumericalErrorCode>::failure(
            {NumericalErrorCode::UndefinedAngle,std::move(detail)});
    }
    const double pi=std::acos(-1.0);
    const double degrees=std::abs(std::acos(cosine)*(360.0/(pi*2.0)));
    if (!std::isfinite(degrees)) {
        detail.message="normrelfehler output is non-finite";
        return Result<Scalar,NumericalErrorCode>::failure(
            {NumericalErrorCode::NonFiniteResult,std::move(detail)});
    }
    return Result<Scalar,NumericalErrorCode>::success(degrees);
}

}  // namespace egomotion::source_metrics
