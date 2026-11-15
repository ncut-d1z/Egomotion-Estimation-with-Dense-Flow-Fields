#pragma once

#include "egomotion/contracts/result.hpp"

#include <filesystem>
#include <string_view>
#include <vector>

namespace egomotion {

// R1: declarations only. R2 provides the matio adapter; no MAT file is
// accessed by a base class or the CLI itself.
class IResultReader {
public:
    virtual ~IResultReader() = default;
    virtual Result<std::vector<MatVariableInfo>, MatErrorCode>
    inspect(const std::filesystem::path& source) const = 0;
    virtual Result<DenseDoubleArray, MatErrorCode>
    readNamed(const std::filesystem::path& source,
              std::string_view variable) const = 0;
    virtual Result<DenseDoubleArray, MatErrorCode>
    readFirstCompatible(const std::filesystem::path& source) const = 0;
};

class IResultWriter {
public:
    virtual ~IResultWriter() = default;
    // Domain payload only; the future MAT adapter owns all matio handles.
    virtual Status<MatErrorCode> saveNewVariable(
        const RunId& run_id, std::string_view variable_name,
        const DenseDoubleArray& array) = 0;
};

class IRandomSource {
public:
    virtual ~IRandomSource() = default;
    virtual Result<Scalar, NumericalErrorCode> uniform() = 0;
    virtual Result<Scalar, NumericalErrorCode> normal() = 0;
};

class IDirectionSampler {
public:
    virtual ~IDirectionSampler() = default;
    virtual Result<DirectionSet, NumericalErrorCode> sample(int level) const = 0;
};

class IFlowNoiseModel {
public:
    virtual ~IFlowNoiseModel() = default;
    virtual Result<FlowField, NumericalErrorCode> apply(
        const FlowField& ideal, Scalar coefficient, IRandomSource& random) const = 0;
};

}  // namespace egomotion
