#include "../support/coefficient_fixture.hpp"
// Source-independent L3 candidate. All fixed 9-direction, M1..M4, RHS,
// solved motion and new coefficients below are copied from:
// verification/adaptive/asymmetric_signed_near_one_step.v1.json
// (independent Wolfram symbolic/numerical result, unapproved DEC-005).
// Negative signed nearness is a non-physical branch test, not positive depth.
#include "egomotion/estimators/adaptive_step.hpp"

#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace {
using namespace egomotion;
using namespace egomotion::estimators;

void require(bool ok,const char* message) {
    if(!ok)throw std::runtime_error(message);
}
void near(double actual,double expected,const char* message,
          double tolerance=2e-8) {
    require(std::isfinite(actual)&&std::isfinite(expected)&&
            std::abs(actual-expected) <= tolerance*(1.0+std::abs(expected)),
            message);
}

struct Evidence {
    bool is_negative;
    const double* coefficients; // 3x5, MATLAB source row layout
    const double* flow;         // 9x3
    double raw_mean;
    double normalized_mean;
    const double* old_near;     // 9
    const double* normalized;   // 9
    const double* m1;           // 3x3 row-major for readability
    const double* m2;
    const double* m3;
    const double* m4;
    const double* rhs;          // 6
    const double* solved;       // 6
    const double* translation;  // 3
    const double* rotation;     // 3
    double mse;
    const double* recomputed;   // 9
    const double* updated;      // 9 (00,1-1,10,11,2-2,2-1,20,21,22)
};

static constexpr double directions[] = {
    1.0,
    0.0,
    0.0,
    0.0,
    1.0,
    0.0,
    0.0,
    0.0,
    1.0,
    -1.0,
    0.0,
    0.0,
    0.0,
    -1.0,
    0.0,
    0.0,
    0.0,
    -1.0,
    0.59999999999999998,
    0.80000000000000004,
    0.0,
    0.0,
    0.59999999999999998,
    0.80000000000000004,
    0.80000000000000004,
    0.0,
    0.59999999999999998
};
static constexpr double pos_coeff[] = {
    3.5449077018110322,
    0.0,
    0.0,
    0.0,
    0.0,
    0.51166335397324425,
    0.40933068317859539,
    -0.68221780529765896,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0
};
static constexpr double pos_flow[] = {
    0.0,
    -0.090909090909090912,
    -0.14285714285714285,
    -0.65909090909090906,
    0.0,
    -0.20000000000000001,
    -1.0571428571428572,
    0.20000000000000001,
    0.0,
    0.0,
    0.090909090909090912,
    0.14285714285714285,
    -1.3409090909090908,
    0.0,
    0.20000000000000001,
    -0.94285714285714284,
    -0.20000000000000001,
    0.0,
    -0.56727272727272726,
    0.42545454545454547,
    -0.24571428571428572,
    -0.84116883116883112,
    0.16000000000000000,
    -0.12000000000000000,
    -0.41348571428571429,
    0.047272727272727272,
    0.55131428571428576
};
static constexpr double pos_raw[] = {
    1.3333333333333333,
    0.75000000000000000,
    1.2000000000000000,
    0.66666666666666663,
    1.2500000000000000,
    0.80000000000000004,
    1.0,
    1.0100000000000000,
    1.3866666666666667
};
static constexpr double pos_normalized[] = {
    1.2770485987938984,
    0.71833983682156788,
    1.1493437389145087,
    0.63852429939694932,
    1.1972330613692799,
    0.76622915927633917,
    0.95778644909542388,
    0.96736431358637820,
    1.3281305427456545
};
static constexpr double pos_m1[] = {
    0.65440227030862008,
    -0.051081943951755943,
    -0.070833628946434912,
    -0.051081943951755943,
    0.68035473572188720,
    -0.051592763391273500,
    -0.070833628946434912,
    -0.051592763391273500,
    0.66524299396949271
};
static constexpr double pos_m2[] = {
    0.0,
    0.15555555555555556,
    -0.15555555555555556,
    -0.15555555555555556,
    0.0,
    0.15555555555555556,
    0.15555555555555556,
    -0.15555555555555556,
    0.0
};
static constexpr double pos_m3[] = {
    0.0,
    -0.21709826179496275,
    0.096417169208939335,
    0.21709826179496275,
    0.0,
    -0.25285562256119193,
    -0.096417169208939335,
    0.25285562256119193,
    0.0
};
static constexpr double pos_m4[] = {
    0.66666666666666663,
    -0.053333333333333330,
    -0.053333333333333330,
    -0.053333333333333330,
    0.66666666666666663,
    -0.053333333333333330,
    -0.053333333333333330,
    -0.053333333333333330,
    0.66666666666666663
};
static constexpr double pos_rhs[] = {
    0.64688080808080806,
    -0.070303030303030298,
    -0.020622222222222222,
    0.13610389610389609,
    0.11591341991341991,
    -0.043108225108225109
};
static constexpr double pos_solved[] = {
    1.0440740740740742,
    0.0,
    0.0,
    0.20000000000000001,
    -0.14285714285714285,
    0.090909090909090912
};
static constexpr double pos_translation[] = {
    1.0,
    0.0,
    0.0
};
static constexpr double pos_rotation[] = {
    0.20000000000000001,
    -0.14285714285714285,
    0.090909090909090912
};
static constexpr double pos_recomputed[] = {
    0.0,
    0.74999999999250000,
    1.1999999999880000,
    0.0,
    1.2499999999875000,
    0.79999999999200000,
    0.99999999998437505,
    1.0099999999899001,
    1.3866666666281482
};
static constexpr double pos_updated[] = {
    2.9133889593362294,
    -0.61808933159042811,
    1.3917243227832163,
    -1.1661376351613801,
    1.9068567351030574,
    -1.9259253024649212,
    0.46695057349789582,
    -2.6441746726441058,
    -0.69619431901279982
};
static constexpr double neg_coeff[] = {
    -3.5449077018110322,
    0.0,
    0.0,
    0.0,
    0.0,
    -0.51166335397324425,
    -0.40933068317859539,
    0.68221780529765896,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0
};
static constexpr double neg_flow[] = {
    0.0,
    -0.090909090909090912,
    -0.14285714285714285,
    0.84090909090909094,
    0.0,
    -0.20000000000000001,
    1.3428571428571430,
    0.20000000000000001,
    0.0,
    0.0,
    0.090909090909090912,
    0.14285714285714285,
    1.1590909090909092,
    0.0,
    0.20000000000000001,
    0.65714285714285714,
    -0.20000000000000001,
    0.0,
    0.71272727272727276,
    -0.53454545454545455,
    -0.24571428571428572,
    1.1788311688311688,
    0.16000000000000000,
    -0.12000000000000000,
    0.58491428571428572,
    0.047272727272727272,
    -0.77988571428571429
};
static constexpr double neg_raw[] = {
    -1.3333333333333333,
    -0.75000000000000000,
    -1.2000000000000000,
    -0.66666666666666663,
    -1.2500000000000000,
    -0.80000000000000004,
    -1.0,
    -1.0100000000000000,
    -1.3866666666666667
};
static constexpr double neg_normalized[] = {
    -1.2770485987938984,
    -0.71833983682156788,
    -1.1493437389145087,
    -0.63852429939694932,
    -1.1972330613692799,
    -0.76622915927633917,
    -0.95778644909542388,
    -0.96736431358637820,
    -1.3281305427456545
};
static constexpr double neg_m1[] = {
    1.3455977296913799,
    0.051081943951755943,
    0.070833628946434912,
    0.051081943951755943,
    1.3196452642781129,
    0.051592763391273500,
    0.070833628946434912,
    0.051592763391273500,
    1.3347570060305074
};
static constexpr double neg_m2[] = {
    0.0,
    0.15555555555555556,
    -0.15555555555555556,
    -0.15555555555555556,
    0.0,
    0.15555555555555556,
    0.15555555555555556,
    -0.15555555555555556,
    0.0
};
static constexpr double neg_m3[] = {
    0.0,
    0.21709826179496275,
    -0.096417169208939335,
    -0.21709826179496275,
    0.0,
    0.25285562256119193,
    0.096417169208939335,
    -0.25285562256119193,
    0.0
};
static constexpr double neg_m4[] = {
    0.66666666666666663,
    -0.053333333333333330,
    -0.053333333333333330,
    -0.053333333333333330,
    0.66666666666666663,
    -0.053333333333333330,
    -0.053333333333333330,
    -0.053333333333333330,
    0.66666666666666663
};
static constexpr double neg_rhs[] = {
    -0.71960808080808081,
    0.036363636363636362,
    0.12728888888888890,
    0.13610389610389609,
    -0.33741991341991340,
    0.15822510822510824
};
static constexpr double neg_solved[] = {
    -0.43359877990021728,
    0.030913089863413411,
    0.024971542276776469,
    0.17006415524255841,
    -0.62114147415510879,
    0.27568585009435687
};
static constexpr double neg_translation[] = {
    -0.99582645820650428,
    0.070996677615159820,
    0.057350997406309438
};
static constexpr double neg_rotation[] = {
    0.17006415524255841,
    -0.62114147415510879,
    0.27568585009435687
};
static constexpr double neg_recomputed[] = {
    -4.8679771318788863,
    0.56744131250179419,
    0.71894291434017954,
    4.8679771318788863,
    1.4343014447732256,
    1.2792824388033777,
    0.70833131065665667,
    0.51773372989052791,
    0.51507333908465291
};
static constexpr double neg_updated[] = {
    2.2612991792774464,
    -0.0071258701662767132,
    0.11112745632137397,
    6.0709858486251980,
    1.3506863304311263,
    -0.98724404984720604,
    0.41288036211613593,
    -0.98217106572093771,
    -0.81539442975417609
};

void verify(const Evidence& evidence) {
    HarmonicMatrix raw_old;
    for(int r=0;r<3;++r)
        for(int c=0;c<5;++c)
            raw_old(r,c)=evidence.coefficients[5*r+c];
    const auto old=test_support::coefficientValue(raw_old);
    FlowField field;
    field.observations.resize(9,6);
    for(int row=0;row<9;++row)
        for(int col=0;col<3;++col){
            field.observations(row,col)=directions[3*row+col];
            field.observations(row,3+col)=evidence.flow[3*row+col];
        }

    auto result=AdaptiveMfaStepSolver{}.solve(old,field);
    require(result.hasValue(),
            "Independent nine-direction Adaptive candidate must solve");
    const auto& output=result.value();
    const auto& t=output.trace;
    require(t.near_raw.size()==9&&t.near_after_sign.size()==9&&
            t.near_normalized.size()==9&&t.recomputed_near.size()==9,
            "Adaptive trace must retain each signed near and reconstructed near");
    near(t.original_near_mean,evidence.raw_mean,
         "Source raw nearness mean differs from independent reference");
    double normalized_sum=0;
    for(int i=0;i<9;++i){
        near(t.near_raw[i],evidence.old_near[i],
             "Old source CS nearness reconstruction mismatch");
        near(t.near_normalized[i],evidence.normalized[i],
             "Sign-flip/old-mean near normalization mismatch");
        // The sign-corrected buffer is positive in both fixtures.
        near(t.near_after_sign[i],std::abs(evidence.old_near[i]),
             "Source sign-flip buffer differs from reference");
        normalized_sum+=t.near_normalized[i];
        near(t.recomputed_near[i],evidence.recomputed[i],
             "Adaptive near_new differs from independent epsilon-regularized L3");
    }
    near(normalized_sum/9.0,evidence.normalized_mean,
         "Negative-mean source branch must retain normalized mean -1");
    for(int r=0;r<3;++r)
        for(int c=0;c<3;++c) {
            const int i=3*r+c;
            near(t.m1(r,c),evidence.m1[i],"Adaptive M1 entry mismatch");
            near(t.m2(r,c),evidence.m2[i],"Adaptive M2 cross block mismatch");
            near(t.m3(r,c),evidence.m3[i],"Adaptive M3 cross block mismatch");
            near(t.m4(r,c),evidence.m4[i],"Adaptive M4 entry mismatch");
        }
    for(int k=0;k<6;++k){
        near(t.rhs[k],evidence.rhs[k],
             "Adaptive 6x6 right-hand side differs from independent L3");
        near(t.solved_motion[k],evidence.solved[k],
             "Adaptive general non-symmetric 6x6 solution mismatch");
    }
    for(int k=0;k<3;++k){
        near(output.motion.translation()[k],evidence.translation[k],
             "Adaptive normalized translation mismatch");
        near(output.motion.rotation()[k],evidence.rotation[k],
             "Adaptive estimated rotation mismatch");
    }
    near(output.error_mse,evidence.mse,
         "Adaptive original return must be MSE, not its square root");
    near(t.error_mse,evidence.mse,"Adaptive trace MSE differs from return");
    require(output.error_mse>=0&&std::isfinite(output.error_mse),
            "Adaptive MSE must be finite and non-negative");
    const int offsets[3]={0,1,4};
    for(int l=0;l<3;++l)
        for(int c=0;c<2*l+1;++c)
            near(output.coefficients_next.values()(l,c),
                 evidence.updated[offsets[l]+c],
                 "New nine source harmonic coefficients differ from independent L3");
    if(evidence.is_negative) {
        near(evidence.raw_mean,-2819.0/2700.0,
             "Negative branch reference must preserve signed source mean");
        require(output.error_mse>0.1,
                "Negative signed-near fixture must not falsely claim zero MSE");
    } else {
        near(evidence.raw_mean,2819.0/2700.0,
             "Positive branch reference must preserve source mean");
        near(output.error_mse,0.0,
             "Positive, nonuniform-near analytical motion must be exact");
    }
}
} // namespace

int main() {
    const Evidence positive{
        false,pos_coeff,pos_flow,
        1.0440740740740742,1.0,pos_raw,pos_normalized,
        pos_m1,pos_m2,pos_m3,pos_m4,pos_rhs,pos_solved,
        pos_translation,pos_rotation,0.0,pos_recomputed,pos_updated};
    const Evidence negative{
        true,neg_coeff,neg_flow,
        -1.0440740740740742,-1.0,neg_raw,neg_normalized,
        neg_m1,neg_m2,neg_m3,neg_m4,neg_rhs,neg_solved,
        neg_translation,neg_rotation,0.21140062257948003,
        neg_recomputed,neg_updated};
    verify(positive);
    verify(negative);
}
