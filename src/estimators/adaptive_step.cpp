#include "egomotion/estimators/adaptive_step.hpp"
#include "egomotion/harmonics/adapter.hpp"

#include <Eigen/Geometry>
#include <Eigen/LU>

#include <cmath>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace egomotion::estimators {
namespace {
template<class T>
Result<T,NumericalErrorCode> fail(NumericalErrorCode code, std::string message,
                                  std::optional<std::size_t> index=std::nullopt) {
    ErrorDetail detail;
    detail.stage=ErrorStage::Solve;
    detail.message=std::move(message);
    detail.observation_index=index;
    return Result<T,NumericalErrorCode>::failure({code,std::move(detail)});
}
template<class T>
Result<T,NumericalErrorCode> propagate(const Failure<NumericalErrorCode>& inner,
                                       std::string message,
                                       std::optional<std::size_t> index=std::nullopt) {
    ErrorDetail outer;
    outer.stage=ErrorStage::Solve;
    outer.message=std::move(message);
    outer.observation_index=index;
    return Result<T,NumericalErrorCode>::failure(
        wrapFailure(inner.code,std::move(outer),inner));
}
Matrix3 crossMatrix(const Vector3& v) {
    Matrix3 m;
    m << 0.0,-v.z(),v.y(),
         v.z(),0.0,-v.x(),
         -v.y(),v.x(),0.0;
    return m;
}
Vector3 direction(const FlowField& f,Eigen::Index i) {
    return f.observations.row(i).head<3>().transpose();
}
Vector3 opticFlow(const FlowField& f,Eigen::Index i) {
    return f.observations.row(i).tail<3>().transpose();
}
}  // namespace

Result<AdaptiveStepResult,NumericalErrorCode> AdaptiveMfaStepSolver::solve(
    const HarmonicCoefficients& old, const FlowField& observations) const {
    const Eigen::Index count=observations.observations.rows();
    if (count==0)
        return fail<AdaptiveStepResult>(NumericalErrorCode::EmptyInput,
                                         "Adaptive step requires at least one flow observation");
    if (!old.values.allFinite()||!observations.observations.allFinite())
        return fail<AdaptiveStepResult>(NumericalErrorCode::NonFiniteInput,
                                         "Adaptive coefficients/flow must be finite");
    for (Eigen::Index i=0;i<count;++i) {
        const Vector3 d=direction(observations,i);
        if (std::abs(d.squaredNorm()-1.0)>1e-8)
            return fail<AdaptiveStepResult>(NumericalErrorCode::InvalidDirection,
                                             "Adaptive observation direction is not unit length",
                                             static_cast<std::size_t>(i));
    }
    try {
        AdaptiveStepResult result;
        auto& trace=result.trace;
        trace.near_raw.resize(count);
        double raw_sum=0.0;
        for (Eigen::Index i=0;i<count;++i) {
            const Vector3 d=direction(observations,i);
            auto near=harmonics::reconstructNear(old,d);
            if (!near.hasValue())
                return propagate<AdaptiveStepResult>(
                    near.error(),"Adaptive near_aus_koeff failed",
                    static_cast<std::size_t>(i));
            trace.near_raw[i]=near.value();
            raw_sum+=near.value();
        }
        const double invCount=1.0/static_cast<double>(count);
        const double mean=raw_sum*invCount;
        if (!std::isfinite(mean)||mean==0.0)
            return fail<AdaptiveStepResult>(NumericalErrorCode::InvalidNearness,
                                             "Adaptive initial nearness mean is zero or non-finite");
        trace.original_near_mean=mean;
        // Source: save s; if s<0: near=-near; then near=near/s.
        // For s<0 this gives raw/abs(s), NOT post-flip/abs(s).
        trace.near_after_sign=trace.near_raw;
        if (mean<0.0) trace.near_after_sign=-trace.near_after_sign;
        trace.near_normalized=trace.near_after_sign/mean;
        if (!trace.near_normalized.allFinite())
            return fail<AdaptiveStepResult>(NumericalErrorCode::NonFiniteResult,
                                             "Adaptive normalized nearness is non-finite");
        Vector3 sum_direction=Vector3::Zero();
        Vector3 sum_near_direction=Vector3::Zero();
        Matrix3 sum_outer=Matrix3::Zero(),sum_near_outer=Matrix3::Zero();
        Vector3 rhs_translation=Vector3::Zero(),rhs_rotation=Vector3::Zero();
        for (Eigen::Index i=0;i<count;++i) {
            const Vector3 d=direction(observations,i),p=opticFlow(observations,i);
            const double mu=trace.near_normalized[i];
            const Matrix3 outer=d*d.transpose();
            sum_direction+=d;
            sum_near_direction+=mu*d;
            sum_outer+=outer;
            sum_near_outer+=mu*outer;
            rhs_translation-=p;
            rhs_rotation+=p.cross(d);
        }
        trace.m1=Matrix3::Identity()-sum_near_outer*invCount;
        trace.m4=Matrix3::Identity()-sum_outer*invCount;
        trace.m2=crossMatrix(-sum_direction*invCount);
        trace.m3=crossMatrix(sum_near_direction*invCount);
        trace.rhs.head<3>()=rhs_translation*invCount;
        trace.rhs.tail<3>()=rhs_rotation*invCount;
        trace.system_matrix.topLeftCorner<3,3>()=trace.m1;
        trace.system_matrix.topRightCorner<3,3>()=trace.m2;
        trace.system_matrix.bottomLeftCorner<3,3>()=trace.m3;
        trace.system_matrix.bottomRightCorner<3,3>()=trace.m4;
        if (!trace.system_matrix.allFinite()||!trace.rhs.allFinite())
            return fail<AdaptiveStepResult>(NumericalErrorCode::NonFiniteResult,
                                             "Adaptive 6x6 system assembly is non-finite");

        // Original: real(inv(M))*b. FullPivLU handles a GENERAL, potentially
        // nonsymmetric matrix; never replace with symmetric LDLT.
        const Eigen::FullPivLU<Matrix6> lu(trace.system_matrix);
        if (!lu.isInvertible())
            return fail<AdaptiveStepResult>(NumericalErrorCode::SingularSystem,
                                             "Adaptive full-pivot 6x6 system is singular");
        trace.solved_motion=lu.solve(trace.rhs);
        trace.translation_raw=trace.solved_motion.head<3>();
        const double translation_norm=trace.translation_raw.norm();
        if (!std::isfinite(translation_norm)||!trace.solved_motion.allFinite())
            return fail<AdaptiveStepResult>(NumericalErrorCode::NonFiniteResult,
                                             "Adaptive motion solve produced non-finite values");
        if (translation_norm==0)
            return fail<AdaptiveStepResult>(NumericalErrorCode::ZeroNorm,
                                             "Adaptive raw translation has zero norm");
        result.motion.translation=trace.translation_raw/translation_norm;
        result.motion.rotation=trace.solved_motion.tail<3>();
        double error_sum=0;
        trace.recomputed_near.resize(count);
        for (Eigen::Index i=0;i<count;++i) {
            const Vector3 d=direction(observations,i),p=opticFlow(observations,i);
            const double mu=trace.near_normalized[i];
            const Vector3 predicted=-mu*mean*
                (result.motion.translation-result.motion.translation.dot(d)*d)
                -result.motion.rotation.cross(d);
            error_sum+=std::abs((p-predicted).squaredNorm());
            const double proj=result.motion.translation.dot(d);
            constexpr double adaptive_denominator_epsilon=1e-11;
            const double denominator=1.0-proj*proj+adaptive_denominator_epsilon;
            if (!std::isfinite(denominator)||denominator<=0.0)
                return fail<AdaptiveStepResult>(NumericalErrorCode::SingularSystem,
                                                 "Adaptive new near denominator is invalid",
                                                 static_cast<std::size_t>(i));
            trace.recomputed_near[i]=
                -result.motion.translation.dot(p-d.cross(result.motion.rotation))/
                denominator;
            if (!std::isfinite(trace.recomputed_near[i]))
                return fail<AdaptiveStepResult>(NumericalErrorCode::NonFiniteResult,
                                                 "Adaptive recomputed near is non-finite",
                                                 static_cast<std::size_t>(i));
        }
        result.error_mse=error_sum*invCount;
        trace.error_mse=result.error_mse;
        if (!std::isfinite(result.error_mse))
            return fail<AdaptiveStepResult>(NumericalErrorCode::NonFiniteResult,
                                             "Adaptive MSE is non-finite");
        auto fitted=harmonics::fitSourceNear(trace.recomputed_near,observations);
        if (!fitted.hasValue())
            return propagate<AdaptiveStepResult>(
                fitted.error(),"Adaptive koeff_aus_near projection failed");
        result.coefficients_next=std::move(fitted).value();
        trace.coefficients_next=result.coefficients_next;
        return Result<AdaptiveStepResult,NumericalErrorCode>::success(
            std::move(result));
    } catch (const std::bad_alloc&) {
        return fail<AdaptiveStepResult>(NumericalErrorCode::ResourceLimitExceeded,
                                         "Adaptive step allocation failed");
    } catch (const std::length_error&) {
        return fail<AdaptiveStepResult>(NumericalErrorCode::ResourceLimitExceeded,
                                         "Adaptive step exceeds container capacity");
    }
}

}  // namespace egomotion::estimators
