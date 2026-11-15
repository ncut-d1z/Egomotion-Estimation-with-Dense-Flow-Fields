#include "egomotion/trajectory/trajectory.hpp"

#include <cmath>
#include <limits>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>

namespace egomotion::trajectory {
namespace {
Result<TrajectoryPoints,NumericalErrorCode> fail(
    NumericalErrorCode code, std::string message) {
    ErrorDetail error;
    error.stage=ErrorStage::Simulation;
    error.message=std::move(message);
    return Result<TrajectoryPoints,NumericalErrorCode>::failure(
        {code,std::move(error)});
}
bool countFitsEigen(std::size_t count) {
    return count<=static_cast<std::size_t>(std::numeric_limits<Eigen::Index>::max());
}
}

Result<TrajectoryPoints,NumericalErrorCode> TrajectoryGenerator::circle(
    std::size_t count, Scalar radius, Scalar height) const {
    if (count==0) return fail(NumericalErrorCode::EmptyInput,
                              "A circle trajectory needs at least one position");
    if (!countFitsEigen(count))
        return fail(NumericalErrorCode::SampleCountOverflow,
                    "Circle trajectory count overflows Eigen::Index");
    if (!std::isfinite(radius)||!std::isfinite(height))
        return fail(NumericalErrorCode::NonFiniteInput,
                    "Circle radius/height must be finite");
    try {
        TrajectoryPoints out;
        out.points.resize(static_cast<Eigen::Index>(count),3);
        const double pi=std::acos(-1.0);
        for (std::size_t j=0;j<count;++j) {
            // Source: w=2*pi*(i-1)/stellen; r=kugelradius*parameter, radius=1.
            const double angle=2.0*pi*static_cast<double>(j)/
                static_cast<double>(count);
            const Eigen::Index row=static_cast<Eigen::Index>(j);
            out.points(row,0)=radius*std::cos(angle);
            out.points(row,1)=radius*std::sin(angle);
            out.points(row,2)=height;
        }
        if (!out.points.allFinite())
            return fail(NumericalErrorCode::NonFiniteResult,
                        "Circle trajectory contains non-finite coordinates");
        return Result<TrajectoryPoints,NumericalErrorCode>::success(std::move(out));
    } catch (const std::bad_alloc&) {
        return fail(NumericalErrorCode::ResourceLimitExceeded,
                    "Circle trajectory allocation failed");
    } catch (const std::length_error&) {
        return fail(NumericalErrorCode::ResourceLimitExceeded,
                    "Circle trajectory exceeds container size");
    }
}

Result<TrajectoryPoints,NumericalErrorCode> TrajectoryGenerator::sinusoidal(
    std::size_t count, Scalar amplitude, Scalar length,
    std::size_t periods, Scalar height) const {
    if (count==0) return fail(NumericalErrorCode::EmptyInput,
                              "A sinusoidal trajectory needs at least one position");
    if (!countFitsEigen(count))
        return fail(NumericalErrorCode::SampleCountOverflow,
                    "Sinusoidal trajectory count overflows Eigen::Index");
    if (!std::isfinite(amplitude)||!std::isfinite(length)||!std::isfinite(height))
        return fail(NumericalErrorCode::NonFiniteInput,
                    "Sinusoidal trajectory input must be finite");
    try {
        TrajectoryPoints out;
        out.points.resize(static_cast<Eigen::Index>(count),3);
        const double pi=std::acos(-1.0);
        for (std::size_t j=0;j<count;++j) {
            // Source denominator is stellen, not stellen-1; do not add an
            // artificial duplicate last point.
            const double angle=2.0*pi*static_cast<double>(j)*
                static_cast<double>(periods)/static_cast<double>(count);
            const Eigen::Index row=static_cast<Eigen::Index>(j);
            out.points(row,0)=length*static_cast<double>(j)/
                static_cast<double>(count)-length/2.0;
            out.points(row,1)=amplitude*std::sin(angle);
            out.points(row,2)=height;
        }
        if (!out.points.allFinite())
            return fail(NumericalErrorCode::NonFiniteResult,
                        "Sinusoidal trajectory contains non-finite coordinates");
        return Result<TrajectoryPoints,NumericalErrorCode>::success(std::move(out));
    } catch (const std::bad_alloc&) {
        return fail(NumericalErrorCode::ResourceLimitExceeded,
                    "Sinusoidal trajectory allocation failed");
    } catch (const std::length_error&) {
        return fail(NumericalErrorCode::ResourceLimitExceeded,
                    "Sinusoidal trajectory exceeds container size");
    }
}

}  // namespace egomotion::trajectory
