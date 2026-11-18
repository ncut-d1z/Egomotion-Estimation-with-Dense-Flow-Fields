#include "../support/flow_fixture.hpp"
// R6 L3 candidate only: values from independent archived Wolfram traces.
// The source of reference numbers is verification/kvd/*.v1.json, NOT this C++
// estimator. No execution, tolerance approval, or MATLAB runtime is implied.
#include "egomotion/estimators/franz_kvd.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
using namespace egomotion;
using namespace egomotion::estimators;

void check(bool pass,const char* reason) {
    if(!pass)throw std::runtime_error(reason);
}
bool close(double x,double y,double tolerance=1e-9) {
    return std::isfinite(x)&&std::abs(x-y)<=tolerance;
}
template<std::size_t N>
FlowField fixture(const std::array<Vector3,N>& directions,
                  const std::array<Vector3,N>& optical_flow) {
    FlowMatrix f;
    f.resize(static_cast<Eigen::Index>(N),6);
    for(std::size_t i=0;i<N;++i) {
        f.row(static_cast<Eigen::Index>(i)).head<3>()=
            directions[i].transpose();
        f.row(static_cast<Eigen::Index>(i)).tail<3>()=
            optical_flow[i].transpose();
    }
    return test_support::flowValue(f);
}
FlowField axial() {
    const std::array<Vector3,6> d{{
        {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}
    }};
    const std::array<Vector3,6> p{{
        {0,-0.5,0},{0,0.5,0},{-0.5,0,0},
        {-1.5,0,0},{-2,0,0},{-2,0,0}
    }};
    return fixture(d,p);
}
FlowField oblique() {
    const double u=1.0/std::sqrt(2.0);
    const std::array<Vector3,6> d{{
        {u,u,0},{u,-u,0},{-u,u,0},{-u,-u,0},{0,0,1},{0,0,-1}
    }};
    const std::array<Vector3,6> p{{
        {-1,1,0},{-1,-1,0},{-1,-1,0},
        {-1,1,0},{-1,0,0},{-1,0,0}
    }};
    return fixture(d,p);
}
FlowField signedNear() {
    const double c=std::sqrt(99.0)/10.0,s=0.1;
    const std::array<Vector3,8> d{{
        {c,s,0},{c,-s,0},{-c,s,0},{-c,-s,0},
        {0,1,0},{0,-1,0},{0,0,1},{0,0,-1}
    }};
    std::array<Vector3,8> p{};
    for(std::size_t k=0;k<8;++k) {
        const Vector3 tangential=Vector3::UnitX()-d[k].x()*d[k];
        p[k]=(k<4?10.0:-1.0)*tangential;
    }
    return fixture(d,p);
}
struct Expected {
    double translation_raw_x;
    double final_translation_x;
    double final_rotation_z;
    std::size_t n;
    double first_near;
};
void inspect(const KvDResult& actual,const Expected& expected,
             bool first_flip) {
    check(actual.iteration_count==6&&actual.trace.size()==6,
          "Original KvD minimum-iteration and stop rule must yield six iterations");
    check(actual.final_nearness.size()==static_cast<Eigen::Index>(expected.n),
          "Final source nearness must own all observation rows");
    check(close(actual.initial_franz.initial_translation_norm,
                expected.n==8?90.0/101.0:1.5),
          "Franz initial translation norm deviates from independent reference");
    check(close(actual.trace[0].translation_candidate_raw.x(),
                expected.translation_raw_x),
          "First KvD translation raw update differs from independent math");
    check(close(actual.motion.translation().x(),expected.final_translation_x),
          "Final KvD source near-mean scaling differs from independent math");
    check(close(actual.motion.rotation().z(),expected.final_rotation_z),
          "Final KvD rotation deviates from independent math");
    check(close(actual.trace.front().nearness_raw[0],
                expected.first_near),
          "First KvD raw nearness is not the source-order value");
    check(actual.trace.front().sign_flipped==first_flip,
          "First KvD near sign flip differs from independent math");
    for(std::size_t k=0;k<6;++k) {
        const auto& step=actual.trace[k];
        check(step.k==k+1,"Trace MATLAB iteration index must start at one");
        check(step.continue_after_iteration==(k<5),
              "KvD source stop decision may not terminate before iteration six");
        check(step.sign_flipped==(first_flip&&k==0),
              "KvD unexpected later sign flip");
        check(step.nearness_after_sign.size()==static_cast<Eigen::Index>(expected.n),
              "KvD must retain entire per-iteration nearness trace");
        check(step.predicted_flow_xyz.rows()==static_cast<Eigen::Index>(expected.n),
              "KvD trace must retain one predicted flow per direction");
        check(step.residual_mse>=0&&std::isfinite(step.residual_mse)&&
              step.residual_rms>=0&&std::isfinite(step.residual_rms),
              "KvD MSE/RMS must be finite and never confused");
        check(close(step.residual_mse,step.residual_rms*step.residual_rms,1e-16),
              "KvD MSE and RMS must satisfy square relation");
    }
    check(close(actual.motion.translation().x(),
                actual.final_nearness_mean*
                actual.final_translation_direction_before_scale.x()),
          "Source final t must equal last-near mean times direction");
}
void evaluateSix(const FlowField& input,bool oblique_input) {
    FranzMatchedFilter franz;
    auto first=franz.estimateWithTrace(input);
    check(first.hasValue(),"Franz source initialization must be defined");
    check(close(first.value().before_translation_normalization.translation().x(),1.5),
          "Independent six-ray Franz translation is 3/2");
    check(close(first.value().before_translation_normalization.rotation().z(),
                oblique_input?0.0:0.5),
          "Independent six-ray Franz angular motion mismatch");
    check(close(first.value().translation_denominators.x(),4.0)&&
          close(first.value().rotation_denominators.z(),4.0),
          "Independent six-ray Franz matched filter denominators are four");

    KvDParameters cfg;
    cfg.retain_trace=true;
    OriginalKvDEstimator original;
    ModifiedKvDEstimator modified;
    auto a=original.estimate(input,cfg);
    auto b=modified.estimate(input,cfg);
    check(a.hasValue()&&b.hasValue(),
          "Both original and modified six-ray source cases must solve");
    if(oblique_input) {
        inspect(a.value(),{2.999999999999167,1.6666666666663668,0.0,
                           6,1.9999999999996},false);
        inspect(b.value(),{1.6666666666665333,1.6666666666663668,0.0,
                           6,1.9999999999996},false);
    } else {
        inspect(a.value(),{1.6666666666665,0.9999999999999,0.5,
                           6,0.0},false);
        inspect(b.value(),{1.0,0.9999999999999,0.5,
                           6,0.0},false);
        check(close(a.value().trace.front().nearness_raw[2],
                    0.9999999999999),
              "Axial reference near[2] must equal 1/(1+source epsilon)");
    }
    check(std::abs(a.value().trace[0].translation_candidate_raw.x()-
                   b.value().trace[0].translation_candidate_raw.x())>0.2,
          "Original and Modified KvD equations must not collapse to one rule");
}
void evaluateSigned() {
    const FlowField input=signedNear();
    FranzMatchedFilter franz;
    auto f=franz.estimateWithTrace(input);
    check(f.hasValue()&&
          close(f.value().before_translation_normalization.translation().x(),
                90.0/101.0),
          "Signed eight-ray Franz reference translation equals 90/101");
    KvDParameters params;
    params.retain_trace=true;
    auto a=OriginalKvDEstimator{}.estimate(input,params);
    auto b=ModifiedKvDEstimator{}.estimate(input,params);
    check(a.hasValue()&&b.hasValue(),
          "Signed-nearness reference must solve in both KvD modes");
    inspect(a.value(),{-50.49999999900495,-4.49999999995005,0.0,
                       8,-9.9999999999},true);
    inspect(b.value(),{-4.4999999999505,-4.49999999995005,0.0,
                       8,-9.9999999999},true);
    for(const KvDResult* result:{&a.value(),&b.value()}) {
        check(close(result->trace.front().nearness_sum_raw,
                    -35.9999999996004,1e-7),
              "Signed first-round raw near sum must be negative");
        check(close(result->trace.front().nearness_after_sign[0],
                    9.9999999999,1e-7),
              "Signed near must flip before translation update");
        check(close(result->final_nearness_mean,4.49999999995005),
              "Final near scaling must use flipped near mean");
        check(close(result->final_translation_direction_before_scale.x(),-1.0),
              "After source sign flip final direction must be negative X");
    }
    KvDParameters invalid;
    invalid.minimum_iterations=0;
    check(!OriginalKvDEstimator{}.estimate(input,invalid).hasValue(),
          "Invalid iterative configuration must not create success");
}
void degenerateFranz() {
    FlowMatrix f;
    f.resize(3,6);
    f.setZero();
    for(int i=0;i<3;++i)f.row(i).head<3>()=
        Vector3::UnitX().transpose();
    auto bad=FranzMatchedFilter{}.estimateWithTrace(test_support::flowValue(f));
    check(!bad.hasValue()&&bad.error().code==NumericalErrorCode::SingularSystem,
          "Axis-degenerate Franz response denominator requires an explicit failure");
}
} // namespace

int main() {
    evaluateSix(axial(),false);
    evaluateSix(oblique(),true);
    evaluateSigned();
    degenerateFranz();
}
