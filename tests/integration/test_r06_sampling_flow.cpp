#include "../support/direction_fixture.hpp"
#include "egomotion/sampling/directions.hpp"
#include "egomotion/simulation/flow.hpp"

#include <Eigen/Core>
#include <array>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
using namespace egomotion;

void require(bool ok,const char* message) {
    if(!ok)throw std::runtime_error(message);
}
bool near(double a,double b,double tolerance=1e-12) {
    return std::abs(a-b)<=tolerance;
}

struct SequenceRandom final:IRandomSource {
    explicit SequenceRandom(std::vector<double> u={},std::vector<double> n={})
        : uniforms(std::move(u)),normals(std::move(n)) {}
    std::vector<double> uniforms;
    std::vector<double> normals;
    std::size_t uniform_calls=0,normal_calls=0;

    Result<Scalar,NumericalErrorCode> uniform() override {
        const auto i=uniform_calls++;
        if(i>=uniforms.size()) {
            ErrorDetail detail;detail.stage=ErrorStage::Simulation;
            detail.message="Synthetic uniform sequence exhausted";
            return Result<Scalar,NumericalErrorCode>::failure({
                NumericalErrorCode::RandomSourceExhausted,std::move(detail)});
        }
        return Result<Scalar,NumericalErrorCode>::success(uniforms[i]);
    }
    Result<Scalar,NumericalErrorCode> normal() override {
        const auto i=normal_calls++;
        if(i>=normals.size()) {
            ErrorDetail detail;detail.stage=ErrorStage::Simulation;
            detail.message="Synthetic normal sequence exhausted";
            return Result<Scalar,NumericalErrorCode>::failure({
                NumericalErrorCode::RandomSourceExhausted,std::move(detail)});
        }
        return Result<Scalar,NumericalErrorCode>::success(normals[i]);
    }
};

void verifySamplingCountsAndOrder() {
    sampling::OctahedralSampler complete;
    sampling::OccludedOctahedralSampler occluded;
    sampling::IcosahedralSampler icosahedron;
    std::size_t children=1;
    for(int level=0;level<=2;++level,children*=4) {
        auto c=complete.sample(level);
        auto o=occluded.sample(level);
        auto i=icosahedron.sample(level);
        require(c.hasValue()&&o.hasValue()&&i.hasValue(),
                "Three source samplers must accept subdivision levels 0,1,2");
        require(c.value().directions().rows()==static_cast<Eigen::Index>(8*children),
                "Full octahedron direction count differs from 8*4^level");
        require(o.value().directions().rows()==static_cast<Eigen::Index>(6*children),
                "Occluded octahedron direction count differs from 6*4^level");
        require(i.value().directions().rows()==static_cast<Eigen::Index>(20*children),
                "Icosahedron direction count differs from 20*4^level");

        // Source retained faces 2,3,5,6,7,8 (MATLAB 1-based).
        constexpr std::array<std::size_t,6> kept{{1,2,4,5,6,7}};
        for(std::size_t k=0;k<kept.size();++k)
            for(std::size_t j=0;j<children;++j) {
                const auto actual=o.value().directions().row(
                    static_cast<Eigen::Index>(k*children+j));
                const auto expected=c.value().directions().row(
                    static_cast<Eigen::Index>(kept[k]*children+j));
                require((actual-expected).norm()<1e-13,
                        "Occlusion must preserve original depth-first leaf order");
            }
        for(const DirectionSet* directions:
            {&c.value(),&o.value(),&i.value()})
            for(Eigen::Index row=0;row<directions->directions().rows();++row)
                require(near(directions->directions().row(row).norm(),1.0),
                        "Every emitted source direction must be unit length");
    }
    auto initial=complete.sample(0);
    const Vector3 expected=Vector3(1,1,1).normalized();
    require(initial.hasValue()&&
            (initial.value().directions().row(0).transpose()-expected).norm()<1e-13,
            "Original first octahedral face has wrong centre");
    require(!complete.sample(-1).hasValue(),
            "Negative subdivisions must return failure");
}

void verifyGetvektorUniformOrder() {
    SequenceRandom rng({0.5,0.0,0.75,0.25});
    auto first=sampling::randomUnitDirection(rng);
    auto second=sampling::randomUnitDirection(rng);
    require(first.hasValue()&&second.hasValue(),
            "Synthetic getvektor directions must exist");
    require((first.value()-Vector3::UnitX()).norm()<1e-12,
            "Uniform (1/2,0) must produce positive X direction");
    require((second.value()-
             Vector3(0,std::sqrt(3.0)/2.0,0.5)).norm()<1e-12,
            "Uniform (3/4,1/4) must preserve theta-then-phi call order");
    require(rng.uniform_calls==4&&rng.normal_calls==0,
            "Every getvektor must consume exactly two uniforms");
}

void verifyBiasNearnessAndNoiseCalls() {
    sampling::OccludedOctahedralSampler sampler;
    simulation::FlowSynthesizer synth;
    simulation::MeanMagnitudeGaussianNoise gaussian;
    simulation::BiasFlowParameters parameters;
    parameters.subdivision_level=0;
    parameters.truth_translation=Vector3::UnitX();
    parameters.truth_rotation=Vector3::UnitY();
    parameters.noise_coefficient=0.0;
    SequenceRandom draws({0,0,0,0,0,0.5},std::vector<double>(18,0.0));
    auto result=simulation::generateBiasFlow(
        parameters,sampler,synth,gaussian,draws);
    require(result.hasValue(),"Injected six-ray Bias scene must succeed");
    require(result.value().nearness.size()==6,
            "Bias nearness must retain each source observation");
    for(int n=0;n<5;++n)
        require(near(result.value().nearness[n],2.5),
                "Bias nearness must preserve source 1/((3u+1)/2.5)");
    require(near(result.value().nearness[5],1.0)&&
            near(result.value().near_last,1.0),
            "Historical scalar near is the final draw, not the mean");
    require(draws.uniform_calls==6&&draws.normal_calls==18,
            "Zero-coefficient Gaussian must still consume N + 3N draws");
    require((result.value().noiseless.observations-
             result.value().noisy.observations).norm()<1e-12,
            "Zero Gaussian coefficient must preserve ideal flow");

    simulation::NoNoise no_noise;
    SequenceRandom clean_rng({0,0,0,0,0,0.5});
    auto clean=simulation::generateBiasFlow(
        parameters,sampler,synth,no_noise,clean_rng);
    require(clean.hasValue()&&clean_rng.uniform_calls==6&&
            clean_rng.normal_calls==0,
            "NoNoise must consume no normal samples, unlike zero Gaussian");
}

void verifyFlowCancellationAndComponentNoise() {
    simulation::FlowSynthesizer synthesizer;
    DirectionMatrix values(1,3);values.row(0)=Vector3::UnitZ().transpose();
    const auto direction=test_support::directionValue(values);
    Eigen::VectorXd nearness(1);
    nearness[0]=1;
    auto ideal=synthesizer.synthesize(
        direction,nearness,Vector3::UnitX(),Vector3(0,-1,0));
    require(ideal.hasValue(),"Single-ray cancellation scene must synthesize");
    require(near(ideal.value().mean_translational_magnitude,1)&&
            near(ideal.value().mean_rotational_magnitude,1),
            "Independent ideal component magnitudes must each be one");
    require(ideal.value().flow.observations.row(0).tail<3>().norm()<1e-13,
            "Translated and rotated ideal flow must cancel exactly");

    simulation::MeanMagnitudeGaussianNoise gaussian;
    SequenceRandom normals({}, {1,2,3});
    auto cancelled=gaussian.apply(ideal.value().flow,2.4,normals);
    require(cancelled.hasValue()&&normals.normal_calls==3,
            "Cancelled ideal flow still consumes three Gaussian draws");
    require(cancelled.value().observations.row(0).tail<3>().norm()<1e-13,
            "Gaussian amplitude must use mean TOTAL flow, not component sum");

    FlowField original=ideal.value().flow;
    original.observations.row(0).tail<3>()=Vector3(1,2,0).transpose();
    simulation::ComponentProportionalGaussianNoise component;
    SequenceRandom noise_samples({}, {1,-1,5});
    auto noisy=component.apply(original,0.5,noise_samples);
    require(noisy.hasValue()&&noise_samples.normal_calls==3,
            "Proportional noise must draw XYZ components separately");
    require((noisy.value().observations.row(0).tail<3>().transpose()-
             Vector3(1.5,1,0)).norm()<1e-13,
            "Component noise and tangent projection mismatch fixed reference");

    SequenceRandom exhausted({}, {1});
    auto failed=gaussian.apply(original,0.5,exhausted);
    require(!failed.hasValue()&&
            failed.error().code==NumericalErrorCode::RandomSourceExhausted,
            "RNG exhaustion must return explicit non-success");
    require(!failed.error().detail.causes.empty(),
            "Noise failure must preserve the underlying RNG cause");
}
} // namespace

int main() {
    verifySamplingCountsAndOrder();
    verifyGetvektorUniformOrder();
    verifyBiasNearnessAndNoiseCalls();
    verifyFlowCancellationAndComponentNoise();
}
