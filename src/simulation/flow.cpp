#include "egomotion/simulation/flow.hpp"

#include <Eigen/Geometry>

#include <cmath>
#include <new>
#include <optional>
#include <string>
#include <utility>

namespace egomotion::simulation {
namespace {

template<class Value>
Result<Value,NumericalErrorCode> fail(NumericalErrorCode code,
                                     std::string message,
                                     std::optional<std::size_t> index = std::nullopt) {
    ErrorDetail detail;
    detail.stage = ErrorStage::Simulation;
    detail.message = std::move(message);
    detail.observation_index = index;
    return Result<Value,NumericalErrorCode>::failure({code, std::move(detail)});
}
template<class Value>
Result<Value,NumericalErrorCode> propagate(
    const Failure<NumericalErrorCode>& underlying, std::string message,
    std::optional<std::size_t> point = std::nullopt) {
    ErrorDetail context;
    context.stage = ErrorStage::Simulation;
    context.message = std::move(message);
    context.observation_index = point;
    // Preserve the original RNG/synthesis failure as an additional frame.
    return Result<Value,NumericalErrorCode>::failure(
        wrapFailure(underlying.code, std::move(context), underlying));
}

bool unit(const Vector3& d) {
    return d.allFinite() && std::abs(d.squaredNorm()-1.0) <= 1e-8;
}
Vector3 directionOf(const FlowField& flow, Eigen::Index i) {
    return flow.observations.row(i).head<3>().transpose();
}
Vector3 flowOf(const FlowField& flow, Eigen::Index i) {
    return flow.observations.row(i).tail<3>().transpose();
}
double meanFlowMagnitude(const FlowField& flow) {
    double sum=0;
    for (Eigen::Index i=0; i<flow.observations.rows(); ++i)
        sum += flowOf(flow,i).norm();
    return sum/static_cast<double>(flow.observations.rows());
}
Result<FlowField,NumericalErrorCode> noiseApply(const FlowField& ideal,
                                                Scalar coefficient,
                                                IRandomSource& rng,
                                                bool component_wise) {
    if (!std::isfinite(coefficient) || !ideal.observations.allFinite())
        return fail<FlowField>(NumericalErrorCode::NonFiniteInput,
                               "Noise input or coefficient is non-finite");
    if (ideal.observations.rows()==0)
        return fail<FlowField>(NumericalErrorCode::EmptyInput,
                               "Noise field contains no rows");
    for (Eigen::Index i=0;i<ideal.observations.rows();++i)
        if (!unit(directionOf(ideal,i)))
            return fail<FlowField>(NumericalErrorCode::InvalidDirection,
                                   "Noise field direction is not unit length",
                                   static_cast<std::size_t>(i));
    const double magnitude = meanFlowMagnitude(ideal);
    if (!std::isfinite(magnitude))
        return fail<FlowField>(NumericalErrorCode::NonFiniteResult,
                               "Ideal flow mean magnitude overflow");
    try {
        FlowField output{ideal.observations};
        for (Eigen::Index i=0; i<ideal.observations.rows(); ++i) {
            const Vector3 d=directionOf(ideal,i);
            const Vector3 original=flowOf(ideal,i);
            Vector3 noisy=original;
            for (int axis=0;axis<3;++axis) {
                // Preserve source scalar randn order x, then y, then z,
                // regardless of coefficient or source component being zero.
                auto drawn=rng.normal();
                if (!drawn.hasValue())
                    return propagate<FlowField>(
                        drawn.error(), "Noise normal failed at component "+
                            std::to_string(axis), static_cast<std::size_t>(i));
                const double g=drawn.value();
                if (!std::isfinite(g))
                    return fail<FlowField>(NumericalErrorCode::InvalidRandomSample,
                                           "Normal RNG returned non-finite value",
                                           static_cast<std::size_t>(i));
                const double scale=component_wise?original[axis]:magnitude;
                noisy[axis] += g*coefficient*scale;
            }
            noisy -= noisy.dot(d)*d;
            if (!noisy.allFinite())
                return fail<FlowField>(NumericalErrorCode::NonFiniteResult,
                                       "Noise/projection produced non-finite flow",
                                       static_cast<std::size_t>(i));
            output.observations.row(i).tail<3>()=noisy.transpose();
        }
        return Result<FlowField,NumericalErrorCode>::success(std::move(output));
    } catch (const std::bad_alloc&) {
        return fail<FlowField>(NumericalErrorCode::ResourceLimitExceeded,
                               "Cannot allocate noisy flow field");
    }
}
}  // namespace

Result<IdealFlowResult,NumericalErrorCode>
FlowSynthesizer::synthesize(const DirectionSet& directions,
                            const Eigen::VectorXd& nearness,
                            const Vector3& translation, const Vector3& rotation) const {
    const Eigen::Index count=directions.directions.rows();
    if (count==0 || nearness.size()!=count)
        return fail<IdealFlowResult>(NumericalErrorCode::InvalidShape,
                                     "Directions and nearness must have matching nonzero rows");
    if (!translation.allFinite() || !rotation.allFinite())
        return fail<IdealFlowResult>(NumericalErrorCode::NonFiniteInput,
                                     "Translation/rotation contains non-finite values");
    try {
        IdealFlowResult result;
        result.flow.observations.resize(count, 6);
        double total_trans=0,total_rot=0;
        for (Eigen::Index i=0; i<count; ++i) {
            const Vector3 d=directions.directions.row(i).transpose();
            const double near=nearness[i];
            if (!unit(d))
                return fail<IdealFlowResult>(NumericalErrorCode::InvalidDirection,
                                             "Direction is non-unit or non-finite",
                                             static_cast<std::size_t>(i));
            if (!std::isfinite(near) || near<=0)
                return fail<IdealFlowResult>(NumericalErrorCode::InvalidNearness,
                                             "Point nearness must be positive and finite",
                                             static_cast<std::size_t>(i));
            const Vector3 trans_flow=-near*(translation-translation.dot(d)*d);
            const Vector3 rot_flow=-rotation.cross(d);
            const Vector3 p=trans_flow+rot_flow;
            if (!p.allFinite())
                return fail<IdealFlowResult>(NumericalErrorCode::NonFiniteResult,
                                             "Ideal flow overflow",static_cast<std::size_t>(i));
            result.flow.observations.row(i).head<3>()=d.transpose();
            result.flow.observations.row(i).tail<3>()=p.transpose();
            total_trans += trans_flow.norm();
            total_rot += rot_flow.norm();
        }
        result.mean_translational_magnitude=total_trans/static_cast<double>(count);
        result.mean_rotational_magnitude=total_rot/static_cast<double>(count);
        if (!std::isfinite(result.mean_translational_magnitude) ||
            !std::isfinite(result.mean_rotational_magnitude))
            return fail<IdealFlowResult>(NumericalErrorCode::NonFiniteResult,
                                         "Ideal component averages are non-finite");
        return Result<IdealFlowResult,NumericalErrorCode>::success(std::move(result));
    } catch (const std::bad_alloc&) {
        return fail<IdealFlowResult>(NumericalErrorCode::ResourceLimitExceeded,
                                     "Cannot allocate ideal flow field");
    }
}
Result<FlowField,NumericalErrorCode> MeanMagnitudeGaussianNoise::apply(
    const FlowField& ideal, Scalar coefficient, IRandomSource& rng) const {
    return noiseApply(ideal,coefficient,rng,false);
}
Result<FlowField,NumericalErrorCode> ComponentProportionalGaussianNoise::apply(
    const FlowField& ideal, Scalar coefficient, IRandomSource& rng) const {
    return noiseApply(ideal,coefficient,rng,true);
}
Result<FlowField,NumericalErrorCode> NoNoise::apply(
    const FlowField& ideal, Scalar coefficient, IRandomSource&) const {
    if (!std::isfinite(coefficient) || !ideal.observations.allFinite())
        return fail<FlowField>(NumericalErrorCode::NonFiniteInput,
                               "Non-finite NoNoise input or coefficient");
    if (ideal.observations.rows()==0)
        return fail<FlowField>(NumericalErrorCode::EmptyInput,
                               "NoNoise input contains no rows");
    for (Eigen::Index i=0;i<ideal.observations.rows();++i)
        if (!unit(directionOf(ideal,i)))
            return fail<FlowField>(NumericalErrorCode::InvalidDirection,
                                   "NoNoise direction is not unit length",
                                   static_cast<std::size_t>(i));
    try {
        return Result<FlowField,NumericalErrorCode>::success(FlowField{ideal.observations});
    } catch (const std::bad_alloc&) {
        return fail<FlowField>(NumericalErrorCode::ResourceLimitExceeded,
                               "Cannot copy ideal field");
    }
}

Result<FlowSynthesisResult,NumericalErrorCode> generateBiasFlow(
    const BiasFlowParameters& params, const IDirectionSampler& sampler,
    const FlowSynthesizer& synthesizer, const IFlowNoiseModel& noise,
    IRandomSource& rng) {
    if (!std::isfinite(params.noise_coefficient) ||
        !params.truth_translation.allFinite() ||
        !params.truth_rotation.allFinite())
        return fail<FlowSynthesisResult>(NumericalErrorCode::NonFiniteInput,
                                         "Invalid truth parameters or noise scale");
    auto sampled=sampler.sample(params.subdivision_level);
    if (!sampled.hasValue())
        return propagate<FlowSynthesisResult>(
            sampled.error(), "Bias direction sampling failed at level "+
                std::to_string(params.subdivision_level));
    DirectionSet directions=std::move(sampled).value();
    const Eigen::Index count=directions.directions.rows();
    if (count==0)
        return fail<FlowSynthesisResult>(NumericalErrorCode::EmptyInput,
                                         "Bias sampler yielded no directions");
    try {
        Eigen::VectorXd nearness(count);
        for (Eigen::Index i=0;i<count;++i) {
            auto u=rng.uniform();
            if (!u.hasValue())
                return propagate<FlowSynthesisResult>(
                    u.error(), "Bias nearness uniform call failed",
                    static_cast<std::size_t>(i));
            const double sample=u.value();
            if (!std::isfinite(sample) || sample < 0.0 || sample >= 1.0)
                return fail<FlowSynthesisResult>(NumericalErrorCode::InvalidRandomSample,
                                                 "Invalid distance RNG sample",
                                                 static_cast<std::size_t>(i));
            nearness[i]=2.5/(1.0+3.0*sample);
        }
        auto ideal=synthesizer.synthesize(
            directions, nearness, params.truth_translation, params.truth_rotation);
        if (!ideal.hasValue())
            return propagate<FlowSynthesisResult>(
                ideal.error(), "Bias ideal flow synthesis failed");
        IdealFlowResult full=std::move(ideal).value();
        const double mean_total=meanFlowMagnitude(full.flow);
        if (!std::isfinite(mean_total))
            return fail<FlowSynthesisResult>(NumericalErrorCode::NonFiniteResult,
                                             "Bias ideal total flow mean overflow");
        auto perturbed=noise.apply(full.flow,params.noise_coefficient,rng);
        if (!perturbed.hasValue())
            return propagate<FlowSynthesisResult>(
                perturbed.error(), "Bias noise injection failed");
        FlowField noisy=std::move(perturbed).value();
        if (noisy.observations.rows()!=count || !noisy.observations.allFinite() ||
            !(noisy.observations.leftCols<3>().array()==
              full.flow.observations.leftCols<3>().array()).all())
            return fail<FlowSynthesisResult>(NumericalErrorCode::InvalidShape,
                                             "Noise strategy changed source direction columns");
        FlowSynthesisResult output;
        output.near_last=nearness[count-1];
        output.nearness=std::move(nearness);
        output.mean_total_flow_magnitude=mean_total;
        output.mean_translational_magnitude=full.mean_translational_magnitude;
        output.mean_rotational_magnitude=full.mean_rotational_magnitude;
        output.noiseless=std::move(full.flow);
        output.noisy=std::move(noisy);
        return Result<FlowSynthesisResult,NumericalErrorCode>::success(std::move(output));
    } catch (const std::bad_alloc&) {
        return fail<FlowSynthesisResult>(NumericalErrorCode::ResourceLimitExceeded,
                                         "Cannot allocate Bias scene values");
    }
}
}  // namespace egomotion::simulation
