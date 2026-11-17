#pragma once

#include "egomotion/contracts/interfaces.hpp"

#include <Eigen/Core>

#include <cstddef>
#include <vector>

namespace egomotion::trajectory {

struct TrajectoryPoints {
    Eigen::Matrix<Scalar,Eigen::Dynamic,3,Eigen::ColMajor> points;
};
struct FrameKinematics {
    Vector3 local_translation = Vector3::Zero();
    Vector3 absolute_rotation = Vector3::Zero();
    Vector3 relative_rotation = Vector3::Zero();
    // Source trajektorie_aus_punkten.m leaves the first/last rows at zero.
    // These rows are NOT measurements of zero camera motion.
    bool has_original_matlab_motion = false;
};

class ITrajectoryGenerator {
public:
    virtual ~ITrajectoryGenerator() = default;
    virtual Result<TrajectoryPoints,NumericalErrorCode> circle(
        std::size_t count, Scalar radius, Scalar height) const = 0;
    virtual Result<TrajectoryPoints,NumericalErrorCode> sinusoidal(
        std::size_t count, Scalar amplitude, Scalar length,
        std::size_t periods, Scalar height) const = 0;
};

class TrajectoryGenerator final : public ITrajectoryGenerator {
public:
    Result<TrajectoryPoints,NumericalErrorCode> circle(
        std::size_t count, Scalar radius, Scalar height) const override;

    Result<TrajectoryPoints,NumericalErrorCode> sinusoidal(
        std::size_t count, Scalar amplitude, Scalar length,
        std::size_t periods, Scalar height) const override;
};

class ITrajectoryKinematics {
public:
    virtual ~ITrajectoryKinematics() = default;
    virtual Result<std::vector<FrameKinematics>,NumericalErrorCode> fromPoints(
        const TrajectoryPoints& positions) const = 0;
};

class TrajectoryKinematics final : public ITrajectoryKinematics {
public:
    Result<std::vector<FrameKinematics>,NumericalErrorCode> fromPoints(
        const TrajectoryPoints& positions) const override;
};

// Explicitly preserves the original 1e-11 UNNORMALIZED cross-product
// threshold. Exact antiparallel vectors have no unambiguous rotation axis.
Result<Vector3,NumericalErrorCode> sourceRotationBetween(
    const Vector3& previous_velocity, const Vector3& next_velocity);

// H-02: prefer Sophus SO3d::exp in .cpp, not in public domain headers.
// Rodrigues's original theta < machine epsilon produces exactly I.
Result<Matrix3,NumericalErrorCode> rotationMatrixFromVector(
    const Vector3& rotation_vector);

class ISphereEnvironment {
public:
    virtual ~ISphereEnvironment() = default;
    virtual Result<Eigen::VectorXd,NumericalErrorCode> nearnessInUnitSphere(
        const DirectionSet& directions, const Vector3& camera_position,
        const Vector3& absolute_rotation_vector) const = 0;
};

class SphereEnvironment final : public ISphereEnvironment {
public:
    Result<Eigen::VectorXd,NumericalErrorCode> nearnessInUnitSphere(
        const DirectionSet& directions, const Vector3& camera_position,
        const Vector3& absolute_rotation_vector) const override;
};

}  // namespace egomotion::trajectory
