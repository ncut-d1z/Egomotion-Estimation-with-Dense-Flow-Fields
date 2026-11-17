#include "egomotion/estimators/adaptive_session.hpp"
#include "egomotion/experiments/adaptive_runner.hpp"

#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
using namespace egomotion;
void require(bool ok,const char* message) {
    if(!ok)throw std::runtime_error(message);
}
struct CountRandom final:IRandomSource {
    std::size_t uniform_calls=0;
    std::size_t normal_calls=0;
    Result<Scalar,NumericalErrorCode> uniform() override {
        ++uniform_calls;
        return Result<Scalar,NumericalErrorCode>::success(0.5);
    }
    Result<Scalar,NumericalErrorCode> normal() override {
        ++normal_calls;
        return Result<Scalar,NumericalErrorCode>::success(0.0);
    }
};
void verifySessionRollback(estimators::AdaptiveSessionMode mode) {
    estimators::AdaptiveMfaSession session(mode);
    const auto initial=session.primaryCoefficients().values;
    const auto initial_baselines=session.updateRateBaselines();
    require(initial(0,0)==1.0,
            "Source Adaptive initial coefficient akkoeff(1,1) must be one");

    estimators::AdaptiveFrameInput frame;
    frame.matlab_frame_index=3;
    frame.source_coefficient_rotation=Matrix3::Identity();
    frame.observations.observations.resize(0,6);
    frame.truth.translation=Vector3::UnitX();
    frame.truth.rotation=Vector3::UnitY();
    auto out_of_order=session.processFrame(frame);
    require(!out_of_order.hasValue()&&
            out_of_order.error().code==ExperimentErrorCode::InvalidParameters,
            "Source frame three must not precede frame two");

    frame.matlab_frame_index=2;
    frame.source_coefficient_rotation(0,0)=
        std::numeric_limits<double>::quiet_NaN();
    auto nonfinite=session.processFrame(frame);
    require(!nonfinite.hasValue()&&
            nonfinite.error().code==ExperimentErrorCode::InvalidParameters,
            "Non-finite coefficient-argument rotation must be rejected");

    // Rotation and periodic refresh are temporary until every required
    // comparison AND the primary solve succeed. Empty flow forces failure.
    frame.source_coefficient_rotation=Matrix3::Identity();
    auto failed=session.processFrame(frame);
    require(!failed.hasValue()&&
            failed.error().code==ExperimentErrorCode::SolverFailed,
            "Empty source flow must fail the required comparison");
    bool has_numerical_root=false;
    for(const auto& cause:failed.error().detail.causes)
        has_numerical_root|=(cause.domain=="NumericalErrorCode"&&
                             cause.code=="EmptyInput");
    require(has_numerical_root,
            "Session failure must retain underlying numerical error frame");
    require((session.primaryCoefficients().values-initial).norm()==0.0,
            "Failed frame must not commit primary harmonic coefficients");
    const auto& final_baselines=session.updateRateBaselines();
    for(std::size_t k=0;k<initial_baselines.size();++k)
        require((final_baselines[k].values-initial_baselines[k].values).norm()==0,
                "Failed frame must not commit periodic baseline coefficients");

    frame.matlab_frame_index=3;
    require(!session.processFrame(frame).hasValue(),
            "A failed frame must not advance the expected source frame index");
}
void verifyInvalidComparisonHasNoRngSideEffect() {
    experiments::AdaptiveExperimentRunner runner;
    CountRandom random;
    AdaptiveComparisonConfig bad{
        std::numeric_limits<double>::quiet_NaN()};
    auto failed=runner.computeComparison(bad,random);
    require(!failed.hasValue()&&
            failed.error().code==ExperimentErrorCode::InvalidParameters,
            "NaN Comparison noise must fail at the configuration gate");
    require(random.uniform_calls==0&&random.normal_calls==0,
            "Invalid Comparison must not consume scientific RNG samples");
}
} // namespace

int main() {
    verifySessionRollback(estimators::AdaptiveSessionMode::Initialization);
    verifySessionRollback(estimators::AdaptiveSessionMode::Comparison);
    verifySessionRollback(estimators::AdaptiveSessionMode::UpdateRates);
    verifyInvalidComparisonHasNoRngSideEffect();
}
