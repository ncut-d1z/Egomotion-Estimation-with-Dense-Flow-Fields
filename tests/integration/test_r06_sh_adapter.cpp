#include "../support/coefficient_fixture.hpp"
// R6 pure-visual rotation action and SH adapter invariants (NOT a claim
// that source SHRotate.m uses the same phase/action convention).
// Physical rotation reference is AN-G08; distinguish R versus R^T explicitly.
#include "egomotion/harmonics/adapter.hpp"
#include "egomotion/trajectory/trajectory.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace {
using namespace egomotion;
void require(bool ok,const char* message) {
    if(!ok)throw std::runtime_error(message);
}
bool close(double a,double b,double tol=2e-10) {
    return std::isfinite(a)&&std::isfinite(b)&&
           std::abs(a-b)<tol*(1+std::abs(b));
}
HarmonicCoefficients testCoefficients() {
    HarmonicMatrix c;
    c.setZero();
    c(0,0)=3.544907701811032;
    c(1,0)=0.37;
    c(1,1)=-0.21;
    c(1,2)=0.83;
    c(2,0)=-0.14;
    c(2,1)=0.42;
    c(2,2)=0.75;
    c(2,3)=-0.51;
    c(2,4)=0.23;
    return test_support::coefficientValue(c);
}
void verifyArgumentAction(const Matrix3& rotation) {
    const auto old=testCoefficients();
    auto forward=harmonics::rotateCoefficients(old,rotation);
    require(forward.hasValue(),"SH adapter must rotate finite low-order coefficients");

    const std::vector<Vector3> samples{
        Vector3::UnitX(),Vector3::UnitY(),Vector3::UnitZ(),
        -Vector3::UnitX(),-Vector3::UnitY(),-Vector3::UnitZ(),
        Vector3(1,2,3).normalized(),Vector3(2,-1,3).normalized(),
        Vector3(-3,4,1).normalized()
    };
    double transpose_discriminator=0.0;
    for(const auto& d:samples) {
        const auto expected=harmonics::reconstructNear(old,rotation*d);
        const auto actual=harmonics::reconstructNear(forward.value(),d);
        const auto wrong=harmonics::reconstructNear(old,rotation.transpose()*d);
        require(expected.hasValue()&&actual.hasValue()&&wrong.hasValue(),
                "SH reconstruction of source and rotated sightlines must be defined");
        require(close(actual.value(),expected.value()),
                "Low-order harmonic rotation must act as near_new(d)=near_old(R*d)");
        transpose_discriminator=std::max(
            transpose_discriminator,std::abs(expected.value()-wrong.value()));
    }
    require(transpose_discriminator>1e-3,
            "Chosen asymmetric SH fixture must discriminate R from R transpose");
    auto undone=harmonics::rotateCoefficients(
        forward.value(),rotation.transpose());
    require(undone.hasValue(),"SH inverse argument rotation must be defined");
    require((undone.value().values()-old.values()).norm()<2e-9,
            "R then R transpose must recover the complete nine source coefficients");
}
} // namespace

int main() {
    const double pi=std::acos(-1.0);
    auto matrix=trajectory::rotationMatrixFromVector(Vector3(0,0,pi/2.0));
    require(matrix.hasValue(),"Sophus physical quarter-turn must exist");
    Matrix3 expected;
    expected<<0,-1,0,1,0,0,0,0,1; // independent AN-G08 SO(3)
    require((matrix.value()-expected).norm()<1e-12,
            "Physical Rodrigues/Sophus quarter-turn disagrees with independent R_z");
    verifyArgumentAction(matrix.value());
    verifyArgumentAction(matrix.value().transpose());

    auto identity=harmonics::rotateCoefficients(
        testCoefficients(),Matrix3::Identity());
    require(identity.hasValue()&&
            (identity.value().values()-testCoefficients().values()).norm()<2e-10,
            "Identity SO(3) must not change any source coefficient");
    auto antiparallel=trajectory::sourceRotationBetween(
        Vector3::UnitX(),-Vector3::UnitX());
    require(!antiparallel.hasValue()&&
            antiparallel.error().code==NumericalErrorCode::InvalidDirection,
            "Source ambiguous exact anti-parallel case must not fake zero rotation");
}
