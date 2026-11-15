#include "egomotion/estimators/franz_kvd.hpp"

#include <Eigen/Geometry>

#include <cmath>
#include <limits>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace egomotion::estimators {
namespace {
enum class TranslationRule { Original, Modified };

template<class T>
Result<T,NumericalErrorCode> fail(NumericalErrorCode code,
                                  std::string message,
                                  std::optional<std::size_t> index = std::nullopt) {
    ErrorDetail detail;
    detail.stage = ErrorStage::Solve;
    detail.message = std::move(message);
    detail.observation_index = index;
    return Result<T,NumericalErrorCode>::failure({code,std::move(detail)});
}
Vector3 direction(const FlowField& field, Eigen::Index i) {
    return field.observations.row(i).head<3>().transpose();
}
Vector3 velocity(const FlowField& field, Eigen::Index i) {
    return field.observations.row(i).tail<3>().transpose();
}
bool validParameters(const KvDParameters& p) {
    return std::isfinite(p.nearness_denominator_epsilon) &&
           std::isfinite(p.initial_threshold) &&
           std::isfinite(p.absolute_threshold) &&
           std::isfinite(p.accuracy_factor) &&
           p.nearness_denominator_epsilon>0 &&
           p.initial_threshold>0 && p.absolute_threshold>0 &&
           p.accuracy_factor>0 &&
           p.minimum_iterations>0 &&
           p.minimum_iterations<=p.nominal_maximum_iterations;
}

bool shouldContinue(double delta,double threshold,
                    std::size_t k,const KvDParameters& p) {
    // Preserve original MATLAB left-associative && with the trailing || k<6.
    return ((delta>threshold && k<p.nominal_maximum_iterations &&
             delta>p.absolute_threshold) || k<p.minimum_iterations);
}

Result<KvDResult,NumericalErrorCode> estimateKvD(
    const FlowField& field, const KvDParameters& params, TranslationRule rule) {
    if (!validParameters(params))
        return fail<KvDResult>(NumericalErrorCode::NonFiniteInput,
                               "Invalid KvD iteration/epsilon parameters");
    const FranzMatchedFilter franz;
    auto start=franz.estimateWithTrace(field);
    if (!start.hasValue())
        return Result<KvDResult,NumericalErrorCode>::failure(std::move(start).error());
    FranzInitializationTrace initialization=std::move(start).value();
    if (initialization.initial_translation_norm==0)
        return fail<KvDResult>(NumericalErrorCode::ZeroNorm,
                               "Franz translation cannot initialize KvD direction");
    Vector3 translation=initialization.before_translation_normalization.translation/
        initialization.initial_translation_norm;
    Vector3 rotation=initialization.before_translation_normalization.rotation;
    const Eigen::Index count=field.observations.rows();
    const double sample_count=static_cast<double>(count);
    try {
        Eigen::VectorXd near(count), denominators(count);
        KvDResult result;
        result.initial_franz=initialization;
        double previousRms=0;
        double threshold=params.initial_threshold;
        double delta=threshold*10.0;
        std::size_t iteration=0;
        while (shouldContinue(delta,threshold,iteration,params)) {
            ++iteration;
            KvDIterationTrace trace;
            if (params.retain_trace) {
                trace.k=iteration;
                trace.translation_before_sign=translation;
                trace.rotation_before=rotation;
            }
            // Near is computed from the current t/omega BEFORE sign correction.
            double nearSum=0;
            for (Eigen::Index i=0;i<count;++i) {
                const Vector3 d=direction(field,i), p=velocity(field,i);
                const double projection=translation.dot(d);
                const double denom=1.0-projection*projection+
                    params.nearness_denominator_epsilon;
                if (!std::isfinite(denom)||denom<=0)
                    return fail<KvDResult>(NumericalErrorCode::SingularSystem,
                                            "KvD nearness denominator not positive",
                                            static_cast<std::size_t>(i));
                denominators[i]=denom;
                near[i]=-translation.dot(p-d.cross(rotation))/denom;
                if (!std::isfinite(near[i]))
                    return fail<KvDResult>(NumericalErrorCode::NonFiniteResult,
                                            "KvD nearness non-finite",
                                            static_cast<std::size_t>(i));
                nearSum+=near[i];
            }
            if (!std::isfinite(nearSum))
                return fail<KvDResult>(NumericalErrorCode::NonFiniteResult,
                                       "KvD nearness sum non-finite");
            if (params.retain_trace) {
                trace.nearness_denominators=denominators;
                trace.nearness_raw=near;
                trace.nearness_sum_raw=nearSum;
                trace.sign_flipped=nearSum<0;
            }
            if (nearSum<0) {
                near=-near;
                translation=-translation;
            }
            if (params.retain_trace) {
                trace.translation_after_sign=translation;
                trace.nearness_after_sign=near;
            }
            const Vector3 tOld=translation;
            Vector3 a=Vector3::Zero(),b=Vector3::Zero(),c=Vector3::Zero();
            Vector3 rotA=Vector3::Zero(),rotB=Vector3::Zero(),rotC=Vector3::Zero();
            for (Eigen::Index i=0;i<count;++i) {
                const Vector3 d=direction(field,i),p=velocity(field,i);
                const double mu=near[i],projection=tOld.dot(d);
                if (rule==TranslationRule::Original) {
                    a+=mu*p;
                    b+=mu*d;
                    c+=(mu*mu*projection)*d;
                } else {
                    a+=p;
                    b+=d;
                    c+=(mu*projection)*d;
                }
                rotA+=p.cross(d);
                rotB+=mu*d;
                rotC+=rotation.dot(d)*d;
            }
            // Original m-sources use a=a/anzahl (not a*=1/anzahl).
            a/=sample_count; b/=sample_count; c/=sample_count;
            rotA/=sample_count; rotB/=sample_count; rotC/=sample_count;
            const Vector3 rawTranslation=-(a+rotation.cross(b)-c);
            const double length=rawTranslation.norm();
            if (!std::isfinite(length))
                return fail<KvDResult>(NumericalErrorCode::NonFiniteResult,
                                       "KvD translation norm overflow");
            if (length==0)
                return fail<KvDResult>(NumericalErrorCode::ZeroNorm,
                                       "KvD translation update cannot be normalized");
            const Vector3 tNew=rawTranslation/length; // exactly once
            const Vector3 rotNew=rotA+tOld.cross(rotB)+rotC;
            if (!tNew.allFinite()||!rotNew.allFinite())
                return fail<KvDResult>(NumericalErrorCode::NonFiniteResult,
                                       "KvD motion update overflow");
            // The new omega uses the sign-corrected OLD t, before t is committed.
            rotation=rotNew;
            translation=tNew;
            double mse=0;
            if (params.retain_trace)
                trace.predicted_flow_xyz.resize(count,3);
            for (Eigen::Index i=0;i<count;++i) {
                const Vector3 d=direction(field,i),p=velocity(field,i);
                const Vector3 prediction=-near[i]*(translation-translation.dot(d)*d)
                                         -rotation.cross(d);
                const Vector3 diff=p-prediction;
                mse+=std::abs(diff.dot(diff));
                if (params.retain_trace)
                    trace.predicted_flow_xyz.row(i)=prediction.transpose();
            }
            mse/=sample_count;
            const double rms=std::sqrt(mse);
            const double previous=previousRms;
            delta=std::abs(previous-rms);
            previousRms=rms;
            threshold=rms/(params.accuracy_factor*static_cast<double>(count));
            if (!std::isfinite(mse)||!std::isfinite(rms)||
                !std::isfinite(delta)||!std::isfinite(threshold))
                return fail<KvDResult>(NumericalErrorCode::NonFiniteResult,
                                       "KvD residual/threshold overflow");
            if (params.retain_trace) {
                trace.translation_sum_a=a;
                trace.translation_sum_b=b;
                trace.translation_sum_c=c;
                trace.translation_candidate_raw=rawTranslation;
                trace.translation_candidate_norm=length;
                trace.translation_candidate_normalized=tNew;
                trace.rotation_sum_a=rotA;
                trace.rotation_sum_b=rotB;
                trace.rotation_sum_c=rotC;
                trace.rotation_after=rotation;
                trace.translation_after_commit=translation;
                trace.residual_mse=mse;
                trace.residual_rms=rms;
                trace.previous_rms=previous;
                trace.residual_change=delta;
                trace.active_threshold=threshold;
                trace.continue_after_iteration=shouldContinue(delta,threshold,iteration,params);
                result.trace.push_back(std::move(trace));
            }
        }
        // Source returns near from the beginning of the FINAL iteration;
        // it does not recompute near from the last t and omega.
        double source_near_sum=0.0;
        for (Eigen::Index i=0;i<count;++i)
            source_near_sum+=near[i]; // source's explicit for-loop order
        const double nearMean=source_near_sum/sample_count;
        result.motion.translation=translation*nearMean;
        result.motion.rotation=rotation;
        result.final_nearness=std::move(near);
        result.final_nearness_mean=nearMean;
        result.final_translation_direction_before_scale=translation;
        result.iteration_count=iteration;
        if (!result.motion.translation.allFinite()||
            !result.motion.rotation.allFinite()||!std::isfinite(nearMean))
            return fail<KvDResult>(NumericalErrorCode::NonFiniteResult,
                                   "KvD final motion overflow");
        return Result<KvDResult,NumericalErrorCode>::success(std::move(result));
    } catch (const std::bad_alloc&) {
        return fail<KvDResult>(NumericalErrorCode::ResourceLimitExceeded,
                               "Cannot allocate KvD nearness or iteration trace");
    } catch (const std::length_error&) {
        return fail<KvDResult>(NumericalErrorCode::ResourceLimitExceeded,
                               "KvD trace exceeds supported container size");
    }
}
} // namespace

Result<FranzInitializationTrace,NumericalErrorCode>
FranzMatchedFilter::estimateWithTrace(const FlowField& field) const {
    if (field.observations.rows()==0)
        return fail<FranzInitializationTrace>(NumericalErrorCode::EmptyInput,
                                              "Franz requires a nonempty flow field");
    if (!field.observations.allFinite())
        return fail<FranzInitializationTrace>(NumericalErrorCode::NonFiniteInput,
                                              "Franz flow contains non-finite values");
    for (Eigen::Index i=0;i<field.observations.rows();++i)
        if (std::abs(direction(field,i).squaredNorm()-1.0)>1e-8)
            return fail<FranzInitializationTrace>(NumericalErrorCode::InvalidDirection,
                                                  "Franz requires unit direction vectors",
                                                  static_cast<std::size_t>(i));
    FranzInitializationTrace trace;
    MotionEstimate motion;
    motion.translation.setZero();
    motion.rotation.setZero();
    for (int axis=0;axis<3;++axis) {
        Vector3 e=Vector3::Zero();
        e[axis]=1.0;
        double tResponse=0,tDenom=0,rResponse=0,rDenom=0;
        for (Eigen::Index i=0;i<field.observations.rows();++i) {
            const Vector3 d=direction(field,i),p=velocity(field,i);
            const Vector3 qt=-(e-e.dot(d)*d);
            const Vector3 qr=-e.cross(d);
            tResponse+=qt.dot(p);
            tDenom+=qt.squaredNorm();
            rResponse+=qr.dot(p);
            rDenom+=qr.squaredNorm();
        }
        if (!std::isfinite(tResponse)||!std::isfinite(rResponse)||
            !std::isfinite(tDenom)||!std::isfinite(rDenom))
            return fail<FranzInitializationTrace>(NumericalErrorCode::NonFiniteResult,
                                                  "Franz matched-filter sums overflow",
                                                  static_cast<std::size_t>(axis));
        if (!(tDenom>0) || !(rDenom>0))
            return fail<FranzInitializationTrace>(NumericalErrorCode::SingularSystem,
                                                  "Franz matched-filter axis denominator is zero",
                                                  static_cast<std::size_t>(axis));
        trace.translation_responses[axis]=tResponse;
        trace.translation_denominators[axis]=tDenom;
        trace.rotation_responses[axis]=rResponse;
        trace.rotation_denominators[axis]=rDenom;
        motion.translation[axis]=tResponse/tDenom;
        motion.rotation[axis]=rResponse/rDenom;
    }
    // zellendefinieren has exactly three axes: original scale 3/anzahlzellen=1.
    if (!motion.translation.allFinite()||!motion.rotation.allFinite())
        return fail<FranzInitializationTrace>(NumericalErrorCode::NonFiniteResult,
                                              "Franz initialized non-finite motion");
    trace.before_translation_normalization=motion;
    trace.initial_translation_norm=motion.translation.norm();
    if (!std::isfinite(trace.initial_translation_norm))
        return fail<FranzInitializationTrace>(NumericalErrorCode::NonFiniteResult,
                                              "Franz translation norm overflow");
    return Result<FranzInitializationTrace,NumericalErrorCode>::success(std::move(trace));
}

Result<MotionEstimate,NumericalErrorCode>
FranzMatchedFilter::estimate(const FlowField& field) const {
    auto trace=estimateWithTrace(field);
    if (!trace.hasValue())
        return Result<MotionEstimate,NumericalErrorCode>::failure(std::move(trace).error());
    return Result<MotionEstimate,NumericalErrorCode>::success(
        std::move(trace).value().before_translation_normalization);
}
Result<KvDResult,NumericalErrorCode>
OriginalKvDEstimator::estimate(const FlowField& field,
                               const KvDParameters& params) const {
    return estimateKvD(field,params,TranslationRule::Original);
}
Result<KvDResult,NumericalErrorCode>
ModifiedKvDEstimator::estimate(const FlowField& field,
                               const KvDParameters& params) const {
    return estimateKvD(field,params,TranslationRule::Modified);
}
} // namespace egomotion::estimators
