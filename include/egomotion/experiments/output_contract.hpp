#pragma once

#include "egomotion/contracts/interfaces.hpp"

#include <map>
#include <string>
#include <vector>

namespace egomotion::experiments {

enum class ExperimentKind {
    BiasOccluded, BiasProportional,
    AdaptiveInitialization, AdaptiveComparison, AdaptiveUpdateRates
};

// Exact source save fields per entry, in source save order. HistoricalOnly
// archive fields are deliberately absent and MUST NOT be auto-generated.
std::vector<std::string> currentSaveFields(ExperimentKind kind);

// Verify the COMPLETE memory batch before the first IResultWriter call.
// A Writer has no ExperimentKind argument; this gate belongs above MAT I/O.
Status<ExperimentErrorCode> validateCurrentOutput(
    ExperimentKind kind, const std::map<std::string,DenseDoubleArray>& fields);

// Adaptive方案B专用：掩码为1只能是有限值，为0只能是NaN；不放宽旧入口。
Status<ExperimentErrorCode> validateMaskedAdaptiveOutput(
    ExperimentKind kind,const std::map<std::string,DenseDoubleArray>& fields,
    const std::map<std::string,std::vector<unsigned char>>& defined);

}  // namespace egomotion::experiments
