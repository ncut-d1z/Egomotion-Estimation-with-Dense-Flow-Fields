#include "egomotion/trajectory/trajectory.hpp"

#include <sophus/so3.hpp>
#include <Eigen/Geometry>

#include <cmath>
#include <limits>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace egomotion::trajectory {
namespace {
template<class T>
Result<T,NumericalErrorCode> fail(NumericalErrorCode code, std::string message,
                                  std::optional<std::size_t> index = std::nullopt) {
    ErrorDetail detail;
    detail.stage=ErrorStage::Simulation;
    detail.message=std::move(message);
    detail.observation_index=index;
    return Result<T,NumericalErrorCode>::failure({code,std::move(detail)});
}
template<class T>
Result<T,NumericalErrorCode> propagate(const Failure<NumericalErrorCode>& source,
                                       std::string message,std::size_t frame) {
    ErrorDetail outer;
    outer.stage=ErrorStage::Simulation;
    outer.message=std::move(message);
    outer.observation_index=frame; // current frame index, not source ray
    return Result<T,NumericalErrorCode>::failure(
        wrapFailure(source.code,std::move(outer),source));
}
}  // namespace

Result<Vector3,NumericalErrorCode> sourceRotationBetween(
    const Vector3& v1, const Vector3& v2) {
    if (!v1.allFinite() || !v2.allFinite())
        return fail<Vector3>(NumericalErrorCode::NonFiniteInput,
                              "Rotation source vectors are non-finite");
    const double norm1=std::sqrt(v1.dot(v1));
    const double norm2=std::sqrt(v2.dot(v2));
    if (!std::isfinite(norm1) || !std::isfinite(norm2))
        return fail<Vector3>(NumericalErrorCode::NonFiniteInput,
                              "Rotation source vector norm overflow");
    if (norm1==0.0 || norm2==0.0)
        return fail<Vector3>(NumericalErrorCode::ZeroNorm,
                              "Rotation between zero-length directions is undefined");
    Vector3 axis=v1.cross(v2);
    const double cross_norm=std::sqrt(axis.dot(axis));
    const double divisor=norm1*norm2;
    if (!std::isfinite(cross_norm) || !std::isfinite(divisor) || divisor==0.0)
        return fail<Vector3>(NumericalErrorCode::NonFiniteResult,
                              "Rotation cross/norm is not representable");
    const double cosine=v1.dot(v2)/divisor;
    // MATLAB calls acos directly; do not silently clamp a value outside
    // [-1,1] and pretend it is source-equivalent.
    if (!std::isfinite(cosine) || cosine < -1.0 || cosine > 1.0)
        return fail<Vector3>(NumericalErrorCode::InvalidDirection,
                              "Source acos argument outside [-1,1]");
    // Exact antiparallel vectors make the original function return zero
    // despite a pi turn. The explicit non-success avoids a false valid pose.
    if (cross_norm==0.0 && cosine<0.0)
        return fail<Vector3>(NumericalErrorCode::InvalidDirection,
                              "Exact antiparallel directions have ambiguous rotation axis");
    // Preserve the source threshold on the UNNORMALIZED cross magnitude.
    constexpr double source_cross_threshold=1e-11;
    if (cross_norm>source_cross_threshold) axis/=cross_norm;
    const double angle=std::acos(cosine);
    const Vector3 rotvec=axis*angle;
    if (!rotvec.allFinite())
        return fail<Vector3>(NumericalErrorCode::NonFiniteResult,
                              "Rotation vector became non-finite");
    return Result<Vector3,NumericalErrorCode>::success(rotvec);
}

Result<Matrix3,NumericalErrorCode> rotationMatrixFromVector(const Vector3& rotvec) {
    if (!rotvec.allFinite())
        return fail<Matrix3>(NumericalErrorCode::NonFiniteInput,
                              "SO3 rotation vector is non-finite");
    const double radians=rotvec.norm();
    if (!std::isfinite(radians))
        return fail<Matrix3>(NumericalErrorCode::NonFiniteResult,
                              "SO3 angle magnitude overflow");
    // Original rodrigues.m: norm(rotvec) < MATLAB double eps => identity.
    // H-02 approved Sophus SO3d::exp for all other vectors.
    if (radians<std::numeric_limits<double>::epsilon())
        return Result<Matrix3,NumericalErrorCode>::success(Matrix3::Identity());
    const Matrix3 output=Sophus::SO3d::exp(rotvec).matrix();
    if (!output.allFinite())
        return fail<Matrix3>(NumericalErrorCode::NonFiniteResult,
                              "Sophus SO3 exponential produced non-finite rotation");
    return Result<Matrix3,NumericalErrorCode>::success(output);
}

Result<std::vector<FrameKinematics>,NumericalErrorCode>
TrajectoryKinematics::fromPoints(const TrajectoryPoints& input) const {
    const Eigen::Index count=input.points.rows();
    if (count<3)
        return fail<std::vector<FrameKinematics>>(NumericalErrorCode::InvalidShape,
                                                   "At least three trajectory points are required");
    if (!input.points.allFinite())
        return fail<std::vector<FrameKinematics>>(NumericalErrorCode::NonFiniteInput,
                                                   "Trajectory contains non-finite coordinates");
    try {
        // Source allocates N×3 zero matrices; first and last rows remain 0,
        // but are NOT valid samples, so preserve an explicit validity flag.
        std::vector<FrameKinematics> result(static_cast<std::size_t>(count));
        const Vector3 initial_forward(1.0,0.0,0.0);
        for (Eigen::Index i=1;i<count-1;++i) {
            const Vector3 previous=input.points.row(i-1).transpose();
            const Vector3 current=input.points.row(i).transpose();
            const Vector3 next=input.points.row(i+1).transpose();
            const Vector3 v1=current-previous;
            const Vector3 v2=next-current;
            auto relative=sourceRotationBetween(v1,v2);
            if (!relative.hasValue())
                return propagate<std::vector<FrameKinematics>>(
                    relative.error(),
                    "Source relative rotation is undefined at trajectory point",
                    static_cast<std::size_t>(i));
            auto absolute=sourceRotationBetween(initial_forward,v1);
            if (!absolute.hasValue())
                return propagate<std::vector<FrameKinematics>>(
                    absolute.error(),
                    "Source absolute rotation is undefined at trajectory point",
                    static_cast<std::size_t>(i));
            auto matrix=rotationMatrixFromVector(absolute.value());
            if (!matrix.hasValue())
                return propagate<std::vector<FrameKinematics>>(
                    matrix.error(),
                    "Frame SO3 rotation conversion failed",static_cast<std::size_t>(i));

            FrameKinematics frame;
            frame.absolute_rotation=absolute.value();
            frame.local_translation=matrix.value().transpose()*v2;
            frame.relative_rotation=matrix.value().transpose()*relative.value();
            if (!frame.local_translation.allFinite()||
                !frame.relative_rotation.allFinite())
                return fail<std::vector<FrameKinematics>>(
                    NumericalErrorCode::NonFiniteResult,
                    "Local frame translation/rotation is non-finite",
                    static_cast<std::size_t>(i));
            frame.has_original_matlab_motion=true;
            result[static_cast<std::size_t>(i)]=std::move(frame);
        }
        return Result<std::vector<FrameKinematics>,NumericalErrorCode>::success(
            std::move(result));
    } catch (const std::bad_alloc&) {
        return fail<std::vector<FrameKinematics>>(
            NumericalErrorCode::ResourceLimitExceeded,"Cannot allocate frame kinematics");
    } catch (const std::length_error&) {
        return fail<std::vector<FrameKinematics>>(
            NumericalErrorCode::ResourceLimitExceeded,"Frame count exceeds vector capacity");
    }
}

}  // namespace egomotion::trajectory
