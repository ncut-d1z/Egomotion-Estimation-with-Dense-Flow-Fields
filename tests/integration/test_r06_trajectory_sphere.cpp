// R6 independent geometric probes for source trajectory and sphere code.
// Tests use analytically known locations/intersections, not the C++ routine
// under test to construct expected values. This is NOT an executed CTest.
#include "egomotion/trajectory/trajectory.hpp"

#include <Eigen/Core>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>

namespace {
using namespace egomotion;
using namespace egomotion::trajectory;

void require(bool yes,const char* why) {
    if(!yes)throw std::runtime_error(why);
}
void close(double got,double expected,const char* why,double tolerance=2e-12) {
    require(std::isfinite(got)&&
            std::abs(got-expected)<=tolerance*(1+std::abs(expected)),why);
}
void checkCircleAndSinusoidal() {
    const TrajectoryGenerator generator;
    const auto circle=generator.circle(35,0.6,0.3);
    require(circle.hasValue()&&circle.value().points.rows()==35,
            "Original 35-point initialization circle missing");
    const double pi=std::acos(-1.0);
    const auto& p=circle.value().points;
    close(p(0,0),0.6,"Circle first x must be radius");
    close(p(0,1),0.0,"Circle first y must be zero");
    close(p(0,2),0.3,"Circle height differs from source");
    close(p(1,0),0.6*std::cos(2*pi/35),"Circle uses wrong angular denominator");
    close(p(1,1),0.6*std::sin(2*pi/35),"Circle angular direction incorrect");
    close(p(34,0),0.6*std::cos(2*pi*34/35),"Circle last x mismatch");
    require(!generator.circle(0,0.6,0.3).hasValue(),
            "Empty circle must return non-success");

    const auto sinusoidal=generator.sinusoidal(600,0.5,1.0,2,0.3);
    require(sinusoidal.hasValue()&&sinusoidal.value().points.rows()==600,
            "Original 600-point two-period sinusoidal trajectory missing");
    const auto& s=sinusoidal.value().points;
    close(s(0,0),-0.5,"Sinusoidal first x must be minus half length");
    close(s(0,1),0,"Sinusoidal starts at zero vertical displacement");
    close(s(75,0),-0.375,"Sinusoidal x uses wrong number of samples");
    close(s(75,1),0.5,"Sinusoidal first peak needs source 2-period formula");
    close(s(599,0),599.0/600.0-0.5,
          "Sinusoidal last point must not reuse first endpoint");
    for(int row: {0,75,150,300,599})
        close(s(row,2),0.3,"Sinusoidal constant height mismatch");
    require(!generator.sinusoidal(0,0.5,1,2,0.3).hasValue(),
            "Empty sinusoidal trajectory must return non-success");
}
void checkRelativeKinematics() {
    TrajectoryPoints line;
    line.points.resize(3,3);
    line.points<<0,0,0,
                 1,0,0,
                 2,0,0;
    const auto motion=TrajectoryKinematics{}.fromPoints(line);
    require(motion.hasValue()&&motion.value().size()==3,
            "Three-point straight-line kinematics failed");
    const auto& frames=motion.value();
    require(!frames[0].has_original_matlab_motion&&
            frames[1].has_original_matlab_motion&&
            !frames[2].has_original_matlab_motion,
            "MATLAB only assigns valid motion to interior trajectory rows");
    require((frames[1].local_translation-Vector3::UnitX()).norm()<1e-12,
            "Identity-attitude straight-line local motion must equal e_x");
    require(frames[1].relative_rotation.norm()<1e-12&&
            frames[1].absolute_rotation.norm()<1e-12,
            "Identical consecutive directions must have zero angular motion");

    auto quarter=sourceRotationBetween(Vector3::UnitX(),Vector3::UnitY());
    require(quarter.hasValue(),"Source perpendicular directional turn failed");
    close(quarter.value().z(),std::acos(-1.0)/2,
          "Perpendicular direction change must produce positive pi/2 about z");
    require(sourceRotationBetween(Vector3::UnitX(),-Vector3::UnitX()).error().code==
            NumericalErrorCode::InvalidDirection,
            "Exact antiparallel source axis must explicitly fail in safe C++");
}
DirectionSet unitAxes() {
    DirectionSet d;
    d.directions.resize(4,3);
    d.directions.row(0)=Vector3::UnitX().transpose();
    d.directions.row(1)=(-Vector3::UnitX()).transpose();
    d.directions.row(2)=Vector3::UnitY().transpose();
    d.directions.row(3)=Vector3::UnitZ().transpose();
    return d;
}
void checkSphereRayDistance() {
    const SphereEnvironment sphere;
    auto axes=unitAxes();
    const auto center=sphere.nearnessInUnitSphere(
        axes,Vector3::Zero(),Vector3::Zero());
    require(center.hasValue()&&center.value().size()==4,
            "Central unit sphere ray distance must be available");
    for(int i=0;i<4;++i)
        close(center.value()[i],1,"Unit sphere central nearness is one");

    const Vector3 camera(0.5,0,0);
    const auto shifted=sphere.nearnessInUnitSphere(axes,camera,Vector3::Zero());
    require(shifted.hasValue(),"Strictly interior camera must intersect sphere");
    close(shifted.value()[0],2,"Outward x ray distance must be 1/2");
    close(shifted.value()[1],2.0/3.0,"Inward -x ray distance must be 3/2");
    close(shifted.value()[2],1/std::sqrt(0.75),
          "Tangential y ray distance must be sqrt(3/4)");
    close(shifted.value()[3],1/std::sqrt(0.75),
          "Tangential z ray distance must be sqrt(3/4)");

    // A local positive-x ray physically rotates to positive-y under Rz(90).
    const double pi=std::acos(-1.0);
    const auto rotated=sphere.nearnessInUnitSphere(
        axes,camera,Vector3(0,0,pi/2));
    require(rotated.hasValue(),"Rotated sphere intersection must succeed");
    close(rotated.value()[0],1/std::sqrt(0.75),
          "Sphere rays must use physical R*local_direction, not R^T");
    require(!sphere.nearnessInUnitSphere(
                axes,Vector3(1,0,0),Vector3::Zero()).hasValue(),
            "Camera on sphere boundary violates strict-inside model");
    require(!sphere.nearnessInUnitSphere(
                axes,Vector3(1.1,0,0),Vector3::Zero()).hasValue(),
            "Camera outside sphere must return explicit non-success");
    axes.directions.row(0)=Vector3(2,0,0).transpose();
    const auto nonunit=sphere.nearnessInUnitSphere(
        axes,Vector3::Zero(),Vector3::Zero());
    require(!nonunit.hasValue()&&
            nonunit.error().code==NumericalErrorCode::InvalidDirection,
            "Non-unit camera sightline must be rejected with its row index");
    require(nonunit.error().detail.observation_index.has_value()&&
            nonunit.error().detail.observation_index.value()==0,
            "First invalid sphere ray must report its real observation index");
}
} // namespace

int main() {
    checkCircleAndSinusoidal();
    checkRelativeKinematics();
    checkSphereRayDistance();
}
