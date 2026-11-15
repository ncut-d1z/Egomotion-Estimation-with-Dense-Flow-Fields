#pragma once

#include "egomotion/contracts/interfaces.hpp"

#include <Eigen/Core>

namespace egomotion::simulation {

struct IdealFlowResult {
    FlowField flow;
    Scalar mean_translational_magnitude = 0;
    Scalar mean_rotational_magnitude = 0;
};
struct FlowSynthesisResult {
    FlowField noisy;
    FlowField noiseless;
    Eigen::VectorXd nearness;
    Scalar near_last = 0;
    Scalar mean_total_flow_magnitude = 0;
    Scalar mean_translational_magnitude = 0;
    Scalar mean_rotational_magnitude = 0;
};
struct BiasFlowParameters {
    int subdivision_level = 0;
    Vector3 truth_translation = Vector3::Zero();
    Vector3 truth_rotation = Vector3::Zero();
    Scalar noise_coefficient = 0;
};

class FlowSynthesizer {
public:
    Result<IdealFlowResult,NumericalErrorCode> synthesize(
        const DirectionSet& directions, const Eigen::VectorXd& nearness,
        const Vector3& translation, const Vector3& rotation) const;
};

// The selected model is injected by the caller. No combined two-model mode.
class MeanMagnitudeGaussianNoise final : public IFlowNoiseModel {
public:
    Result<FlowField,NumericalErrorCode> apply(
        const FlowField& ideal, Scalar coefficient, IRandomSource& rng) const override;
};
class ComponentProportionalGaussianNoise final : public IFlowNoiseModel {
public:
    Result<FlowField,NumericalErrorCode> apply(
        const FlowField& ideal, Scalar coefficient, IRandomSource& rng) const override;
};
// Distinct from MeanMagnitudeGaussianNoise(coefficient=0), which still draws 3N samples.
class NoNoise final : public IFlowNoiseModel {
public:
    Result<FlowField,NumericalErrorCode> apply(
        const FlowField& ideal, Scalar coefficient, IRandomSource& rng) const override;
};

Result<FlowSynthesisResult,NumericalErrorCode> generateBiasFlow(
    const BiasFlowParameters& parameters, const IDirectionSampler& sampler,
    const FlowSynthesizer& synthesizer, const IFlowNoiseModel& noise,
    IRandomSource& rng);

}  // namespace egomotion::simulation
