// R6 deterministic Adaptive Session tests using a narrow solver seam.
// This verifies the MATLAB 1-based 1/5/11/21 policy, discarded "dumi"
// coefficients, primary update order, and failure atomicity. The fake solver
// is NOT a numerical model, source Golden or replacement for production.
#include "egomotion/estimators/adaptive_session.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
using namespace egomotion;
using namespace egomotion::estimators;

void check(bool ok,const char* why) {
    if(!ok)throw std::runtime_error(why);
}
bool same(double x,double y) {
    return std::isfinite(x)&&std::abs(x-y)<1e-9;
}

struct IncrementSolver final:IAdaptiveStepSolver {
    mutable std::size_t calls=0;
    mutable std::vector<double> received_primary_coefficient;
    std::optional<std::size_t> fail_on_call;

    Result<AdaptiveStepResult,NumericalErrorCode> solve(
        const HarmonicCoefficients& old,const FlowField&) const override {
        ++calls;
        received_primary_coefficient.push_back(old.values(0,0));
        if(fail_on_call&&calls==*fail_on_call) {
            ErrorDetail reason;
            reason.stage=ErrorStage::Solve;
            reason.message="Injected failure at source StepSolver boundary";
            return Result<AdaptiveStepResult,NumericalErrorCode>::failure({
                NumericalErrorCode::SingularSystem,std::move(reason)});
        }
        AdaptiveStepResult output;
        output.motion.translation=Vector3::UnitX();
        output.motion.rotation=Vector3::UnitY();
        output.error_mse=0.0;
        output.coefficients_next=old;
        output.coefficients_next.values(0,0)+=100.0;
        return Result<AdaptiveStepResult,NumericalErrorCode>::success(
            std::move(output));
    }
};

AdaptiveFrameInput input(std::size_t matlab_index) {
    AdaptiveFrameInput frame;
    frame.matlab_frame_index=matlab_index;
    frame.source_coefficient_rotation=Matrix3::Identity();
    frame.truth.translation=Vector3::UnitX();
    frame.truth.rotation=Vector3::UnitY();
    // Deliberately empty: the injected step consumes coefficients only.
    // No scientific data or claimed trajectory is generated here.
    frame.observations.observations.resize(0,6);
    return frame;
}
constexpr std::array<std::size_t,4> periods{{1,5,11,21}};
double expectedBaseline(std::size_t frame,std::size_t period) {
    const std::size_t refreshed_at=(frame/period)*period;
    if(refreshed_at<2)return 1.0;
    return 1.0+100.0*static_cast<double>(refreshed_at-2);
}
void verifyOneSuccessfulFrame(AdaptiveMfaSession& session,
                              IncrementSolver& fake,std::size_t frame) {
    const std::size_t prior_calls=fake.calls;
    const double main_before=1.0+100.0*static_cast<double>(frame-2);
    auto observed=session.processFrame(input(frame));
    check(observed.hasValue(),"Synthetic deterministic Session frame failed");
    check(fake.calls-prior_calls==5,
          "UpdateRates must perform exactly four comparisons and one primary solve");
    check(observed.value().comparison_motion.size()==4&&
          observed.value().comparison_translation_error_deg.size()==4&&
          observed.value().comparison_rotation_error_deg.size()==4,
          "UpdateRates must return four comparable motion/angle results");
    for(std::size_t j=0;j<4;++j){
        const double expected=expectedBaseline(frame,periods[j]);
        check(same(fake.received_primary_coefficient[prior_calls+j],expected),
              "Period baseline solver input uses wrong MATLAB frame or refresh order");
        check(same(session.updateRateBaselines()[j].values(0,0),expected),
              "Comparison dumi result was committed or source refresh order changed");
        check(same(observed.value().comparison_translation_error_deg[j],0.0)&&
              same(observed.value().comparison_rotation_error_deg[j],0.0),
              "Synthetic identical truth/estimate directions must have zero angular error");
    }
    check(same(fake.received_primary_coefficient[prior_calls+4],main_before),
          "Primary must run using the current pre-commit main coefficients");
    check(same(session.primaryCoefficients().values(0,0),main_before+100.0),
          "Only primary StepSolver may commit the new main coefficients");
}
void verifyAllPeriods() {
    IncrementSolver fake;
    AdaptiveMfaSession session(AdaptiveSessionMode::UpdateRates,&fake);
    for(std::size_t i=2;i<=22;++i)
        verifyOneSuccessfulFrame(session,fake,i);
    check(fake.calls==105,"Twenty-one frames must invoke exactly 105 StepSolver calls");
    check(same(session.updateRateBaselines()[1].values(0,0),1801.0),
          "At frame22 period5 must still contain its frame20 pre-primary snapshot");
    check(same(session.updateRateBaselines()[2].values(0,0),2001.0),
          "At frame22 period11 must refresh from frame22 pre-primary main");
    check(same(session.updateRateBaselines()[3].values(0,0),1901.0),
          "At frame22 period21 must retain frame21 pre-primary snapshot");
}
void unchanged(const AdaptiveMfaSession& session,
               const HarmonicMatrix& before,
               const std::array<HarmonicCoefficients,4>& baselines) {
    check((session.primaryCoefficients().values-before).norm()==0,
          "Failed source frame changed primary coefficients");
    for(std::size_t j=0;j<4;++j)
        check((session.updateRateBaselines()[j].values-
               baselines[j].values).norm()==0,
              "Failed source frame partially rotated/refreshed a baseline");
}
void verifyFailureInjection() {
    IncrementSolver fake;
    AdaptiveMfaSession session(AdaptiveSessionMode::UpdateRates,&fake);
    for(std::size_t i=2;i<=4;++i)
        verifyOneSuccessfulFrame(session,fake,i);

    const auto before=session.primaryCoefficients().values;
    const auto baselines=session.updateRateBaselines();
    fake.fail_on_call=fake.calls+2; // period5 comparison at MATLAB i=5
    auto failed=session.processFrame(input(5));
    check(!failed.hasValue()&&
          failed.error().code==ExperimentErrorCode::SolverFailed,
          "Injected period5 comparison must abort whole frame");
    bool nested=false;
    for(const auto& frame:failed.error().detail.causes)
        nested|=frame.domain=="NumericalErrorCode"&&frame.code=="SingularSystem";
    check(nested,"Injected failure must preserve original numerical ErrorFrame");
    unchanged(session,before,baselines);
    fake.fail_on_call.reset();
    // Since frame5 was not committed, it must still be the expected frame.
    auto valid=session.processFrame(input(5));
    check(valid.hasValue(),"Retry at same MATLAB frame must succeed after failure");
    check(same(session.updateRateBaselines()[1].values(0,0),301.0),
          "Retry frame5 must refresh period5 from pre-primary main");

    const auto before_primary=session.primaryCoefficients().values;
    const auto baselines_primary=session.updateRateBaselines();
    fake.fail_on_call=fake.calls+5; // after four successful comparisons
    auto primary_failed=session.processFrame(input(6));
    check(!primary_failed.hasValue(),
          "Injected primary failure must abort frame after comparisons");
    unchanged(session,before_primary,baselines_primary);
    fake.fail_on_call.reset();
    auto retry=session.processFrame(input(6));
    check(retry.hasValue(),"After primary failure original frame6 remains retryable");
    check(same(session.primaryCoefficients().values(0,0),501.0),
          "Primary commit after retry must advance exactly one source iteration");
}
void verifyOtherModeComparisonOrder(AdaptiveSessionMode mode) {
    IncrementSolver fake;
    AdaptiveMfaSession session(mode,&fake);
    for(std::size_t i=2;i<=4;++i) {
        const std::size_t before=fake.calls;
        auto frame=session.processFrame(input(i));
        check(frame.hasValue(),"Injected initialization/comparison frame must succeed");
        check(fake.calls-before==3,
              "Initialization/Comparison must each perform two comparisons and one primary");
        check(same(fake.received_primary_coefficient[before],1.0),
              "Source ohne comparator must always use constant initial coefficients");
        const double initial=1.0+100.0*static_cast<double>(i-2);
        check(same(fake.received_primary_coefficient[before+1],initial)&&
              same(fake.received_primary_coefficient[before+2],initial),
              "Source mit and primary input must use current main coefficients");
        check(same(session.primaryCoefficients().values(0,0),initial+100.0),
              "Source primary coefficients must advance after both comparisons");
    }
}
} // namespace

int main() {
    verifyAllPeriods();
    verifyFailureInjection();
    verifyOtherModeComparisonOrder(AdaptiveSessionMode::Initialization);
    verifyOtherModeComparisonOrder(AdaptiveSessionMode::Comparison);
}
