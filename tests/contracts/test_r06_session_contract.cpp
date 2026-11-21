#include "../support/motion_fixture.hpp"
#include "../support/coefficient_fixture.hpp"
#include "egomotion/estimators/adaptive_session.hpp"
#include "egomotion/experiments/adaptive_runner.hpp"
#include "egomotion/sampling/directions.hpp"
#include "egomotion/simulation/flow.hpp"

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
struct MetricSolver final:estimators::IAdaptiveStepSolver {
    mutable std::size_t calls=0;
    std::size_t fail_on_call=0;
    bool zero_estimated_translation=false;
    Result<estimators::AdaptiveStepResult,NumericalErrorCode> solve(
        const HarmonicCoefficients& old,const FlowField&) const override {
        if(++calls==fail_on_call) {
            ErrorDetail detail;detail.stage=ErrorStage::Solve;detail.message="Primary solve fault after metric gaps";
            return Result<estimators::AdaptiveStepResult,NumericalErrorCode>::failure(
                {NumericalErrorCode::SingularSystem,std::move(detail)});
        }
        estimators::AdaptiveStepResult result;
        Vector3 translation=Vector3::UnitX();
        if(zero_estimated_translation)translation.setZero();
        result.motion=test_support::motionValue(translation,Vector3::Zero());
        auto next=old.values();next(0,0)+=1;
        result.coefficients_next=test_support::coefficientValue(next);
        result.trace.coefficients_next=result.coefficients_next;
        return Result<estimators::AdaptiveStepResult,NumericalErrorCode>::success(std::move(result));
    }
};
void verifyMetricValiditySeparateFromSolve(estimators::AdaptiveSessionMode mode) {
    const std::size_t comparisons=mode==estimators::AdaptiveSessionMode::UpdateRates?4:2;
    MetricSolver solver;
    estimators::AdaptiveMfaSession session(mode,&solver);
    estimators::AdaptiveFrameInput frame;
    frame.matlab_frame_index=2;
    frame.truth=test_support::motionValue(Vector3::Zero(),Vector3::Zero());
    const auto before=session.primaryCoefficients().values();
    // 比较求解成功而方向无定义之后，主求解真正失败仍须整帧回滚。
    solver.fail_on_call=comparisons+1;
    const auto failed=session.processFrame(frame);
    require(!failed.hasValue()&&failed.error().code==ExperimentErrorCode::SolverFailed,
            "A primary solve failure remains fatal despite earlier metric gaps");
    require((session.primaryCoefficients().values()-before).norm()==0,
            "A true solve failure must not commit state");
    solver.fail_on_call=0;
    const auto partial=session.processFrame(frame);
    require(partial.hasValue()&&solver.calls==2*(comparisons+1),
            "All comparisons and the primary must run with missing translation metrics");
    require(partial.value().comparison_translation_error_deg.size()==comparisons,
            "Missing metric slots must retain their source indices");
    for(std::size_t slot=0;slot<comparisons;++slot) {
        const auto& translation=partial.value().comparison_translation_error_deg[slot];
        const auto& rotation=partial.value().comparison_rotation_error_deg[slot];
        require(!translation.hasValue()&&
                (translation.error().code==NumericalErrorCode::UndefinedAngle||
                 translation.error().code==NumericalErrorCode::ZeroNorm),
                "Zero translation direction remains explicitly missing");
        require(rotation.hasValue()&&rotation.value()==0,
                "Two zero rotation vectors represent identical valid rotations");
    }
    require(session.primaryCoefficients().values()(0,0)==before(0,0)+1,
            "Successful solving must commit despite missing translation metrics");
    require(!session.processFrame(frame).hasValue(),"Metric-gap frame advances the source clock exactly once");
    frame.matlab_frame_index=3;
    frame.truth=test_support::motionValue(Vector3::UnitX(),Vector3::UnitY());
    const auto next=session.processFrame(frame);
    require(next.hasValue(),"Frame following a metric gap must continue");
    for(const auto& rotation:next.value().comparison_rotation_error_deg)
        require(rotation.hasValue()&&std::abs(rotation.value()-180/std::acos(-1.0))<1e-9,
                "Zero estimated rotation versus one-radian truth has a finite geodesic error");
    solver.zero_estimated_translation=true;
    frame.matlab_frame_index=4;
    const auto zero_estimate=session.processFrame(frame);
    require(zero_estimate.hasValue(),"A comparison metric gap must not suppress the primary solve");
    for(const auto& translation:zero_estimate.value().comparison_translation_error_deg)
        require(!translation.hasValue(),"A zero estimated translation is not a valid zero-angle metric");
}
void verifySessionRollback(estimators::AdaptiveSessionMode mode) {
    estimators::AdaptiveMfaSession session(mode);
    const auto initial=session.primaryCoefficients().values();
    const auto initial_baselines=session.updateRateBaselines();
    require(initial(0,0)==1.0,
            "Source Adaptive initial coefficient akkoeff(1,1) must be one");

    estimators::AdaptiveFrameInput frame;
    frame.matlab_frame_index=3;
    frame.source_coefficient_rotation=Matrix3::Identity();
    frame.observations=FlowField{};
    frame.truth=test_support::motionValue(Vector3::UnitX(),Vector3::UnitY());
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
    require((session.primaryCoefficients().values()-initial).norm()==0.0,
            "Failed frame must not commit primary harmonic coefficients");
    const auto& final_baselines=session.updateRateBaselines();
    for(std::size_t k=0;k<initial_baselines.size();++k)
        require((final_baselines[k].values()-initial_baselines[k].values()).norm()==0,
                "Failed frame must not commit periodic baseline coefficients");

    frame.matlab_frame_index=3;
    require(!session.processFrame(frame).hasValue(),
            "A failed frame must not advance the expected source frame index");
}
void verifyInvalidComparisonHasNoRngSideEffect() {
    sampling::IcosahedralSampler sampler;
    simulation::MeanMagnitudeGaussianNoise noise;
    trajectory::TrajectoryGenerator generator;
    trajectory::TrajectoryKinematics kinematics;
    trajectory::SphereEnvironment environment;
    experiments::AdaptiveExperimentRunner runner({sampler,noise},{generator,kinematics,environment});
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
    for(auto mode:{estimators::AdaptiveSessionMode::Initialization,
                   estimators::AdaptiveSessionMode::Comparison,
                   estimators::AdaptiveSessionMode::UpdateRates})
        verifyMetricValiditySeparateFromSolve(mode);
}
