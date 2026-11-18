#include "egomotion/trajectory/trajectory.hpp"

#include <cmath>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace egomotion::trajectory {
namespace {
Result<Eigen::VectorXd,NumericalErrorCode> fail(
    NumericalErrorCode code, std::string message,
    std::optional<std::size_t> index=std::nullopt) {
    ErrorDetail detail;
    detail.stage=ErrorStage::Simulation;
    detail.message=std::move(message);
    detail.observation_index=index;
    return Result<Eigen::VectorXd,NumericalErrorCode>::failure(
        {code,std::move(detail)});
}
bool unit(const Vector3& direction) {
    return direction.allFinite() &&
           std::abs(direction.squaredNorm()-1.0)<=1e-8;
}
}  // namespace

Result<Eigen::VectorXd,NumericalErrorCode> SphereEnvironment::nearnessInUnitSphere(
    const DirectionSet& directions, const Vector3& camera_position,
    const Vector3& absolute_rotation_vector) const {
    if (directions.directions().rows()==0)
        return fail(NumericalErrorCode::EmptyInput,"Sphere requires unit rays");
    if (!camera_position.allFinite()||!absolute_rotation_vector.allFinite())
        return fail(NumericalErrorCode::NonFiniteInput,
                    "Camera position or rotation is non-finite");
    const double position_square=camera_position.dot(camera_position);
    // A valid positive forward ray distance is not sufficient: a camera
    // exactly on or outside the unit sphere violates the source model.
    if (!std::isfinite(position_square)||position_square>=1.0)
        return fail(NumericalErrorCode::InvalidNearness,
                    "Camera position must lie strictly inside the unit sphere");
    auto orientation=rotationMatrixFromVector(absolute_rotation_vector);
    if (!orientation.hasValue()) {
        ErrorDetail outer;
        outer.stage=ErrorStage::Simulation;
        outer.message="Cannot rotate sphere camera rays";
        return Result<Eigen::VectorXd,NumericalErrorCode>::failure(
            wrapFailure(orientation.error().code,std::move(outer),
                        orientation.error()));
    }
    try {
        Eigen::VectorXd nearness(directions.directions().rows());
        for (Eigen::Index i=0;i<directions.directions().rows();++i) {
            const Vector3 d=directions.directions().row(i).transpose();
            if (!unit(d))
                return fail(NumericalErrorCode::InvalidDirection,
                            "Sphere ray is not unit length",static_cast<std::size_t>(i));
            const Vector3 n=orientation.value()*d;
            const double b=n.dot(camera_position);
            const double delta=b*b-position_square+1.0;
            if (!std::isfinite(delta)||delta<0.0)
                return fail(NumericalErrorCode::InvalidNearness,
                            "Sphere discriminant is invalid",static_cast<std::size_t>(i));
            const double distance=-b+std::sqrt(delta);
            const double near=1.0/distance;
            if (!std::isfinite(distance)||distance<=0.0||!std::isfinite(near))
                return fail(NumericalErrorCode::InvalidNearness,
                            "Forward sphere intersection has no positive distance",
                            static_cast<std::size_t>(i));
            nearness[i]=near;
        }
        return Result<Eigen::VectorXd,NumericalErrorCode>::success(std::move(nearness));
    } catch (const std::bad_alloc&) {
        return fail(NumericalErrorCode::ResourceLimitExceeded,
                    "Cannot allocate sphere nearness");
    } catch (const std::length_error&) {
        return fail(NumericalErrorCode::ResourceLimitExceeded,
                    "Sphere nearness exceeds vector capacity");
    }
}

}  // namespace egomotion::trajectory
