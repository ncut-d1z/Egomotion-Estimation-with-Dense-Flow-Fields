#include "egomotion/harmonics/services.hpp"
#include "service_error.hpp"

#include <new>
#include <stdexcept>

namespace egomotion::harmonics {
Result<HarmonicCoefficients,NumericalErrorCode> HarmonicFitter::fitSourceNear(
    const Eigen::VectorXd& near, const FlowField& field) const {
    if (near.size()!=field.observations.rows() || near.size()==0) {
        ErrorDetail error;
        error.stage=ErrorStage::Solve;
        error.message="SH source fitting requires one near per observed ray";
        return Result<HarmonicCoefficients,NumericalErrorCode>::failure(
            {NumericalErrorCode::InvalidShape,std::move(error)});
    }
    try {
        // Original koeff_aus_near.m consumes N×6 and only its first 3 columns.
        // The pre-existing leaf takes an owning dynamic N×K matrix.
        const Eigen::MatrixXd points=field.observations;
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

} // namespace egomotion::harmonics
