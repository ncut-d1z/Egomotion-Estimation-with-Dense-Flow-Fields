#include "egomotion/harmonics/services.hpp"
#include "service_error.hpp"

#include <new>
#include <stdexcept>

namespace egomotion::harmonics {
namespace {
Result<HarmonicCoefficients,NumericalErrorCode> fitDirections(
    const Eigen::VectorXd& near,const Eigen::Ref<const Eigen::MatrixXd>& directions) {
    if (near.size()!=directions.rows() || near.size()==0) {
        ErrorDetail error;
        error.stage=ErrorStage::Solve;
        error.message="SH source fitting requires one near per observed ray";
        return Result<HarmonicCoefficients,NumericalErrorCode>::failure(
            {NumericalErrorCode::InvalidShape,std::move(error)});
    }
    try {
        // 原koeff_aus_near仅消费前三列；不要求无关光流列有效或存在。
        const Eigen::MatrixXd points=directions;
        auto fitted=low_order::projectSourceCoefficientsOrderTwo(near,points);
        if (!fitted.hasValue())
            return detail::bridge<HarmonicCoefficients>(fitted.error(),"SH source fit");
        return HarmonicCoefficients::create(fitted.value());
    } catch (const std::bad_alloc&) {
        return detail::resourceFailure<HarmonicCoefficients>();
    } catch (const std::length_error&) {
        return detail::resourceFailure<HarmonicCoefficients>();
    }
}
} // namespace

Result<HarmonicCoefficients,NumericalErrorCode> HarmonicFitter::fitSourceNear(
    const Eigen::VectorXd& near,const FlowField& field) const {
    return fitDirections(near,field.observations().leftCols<3>());
}

Result<HarmonicCoefficients,NumericalErrorCode> HarmonicFitter::fitSourceNear(
    const Eigen::VectorXd& near,const DirectionSet& directions) const {
    return fitDirections(near,directions.directions());
}

} // namespace egomotion::harmonics
