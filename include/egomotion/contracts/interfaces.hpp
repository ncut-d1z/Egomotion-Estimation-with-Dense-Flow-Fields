#pragma once

#include "egomotion/contracts/result.hpp"

#include <cstdint>
#include <filesystem>
#include <string_view>
#include <utility>
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
    // Read-only receipts. These are NOT a claim that a Manifest was saved.
    virtual std::vector<FileEvidence> fileEvidence() const = 0;
};

// Atomic, exclusive, single-file text publication within an already
// reserved RunId directory; never accepts a user-supplied path.
class ITextPublisher {
public:
    virtual ~ITextPublisher() = default;
    virtual Status<MatErrorCode> publishNew(
        std::string_view filename,std::string_view data)=0;
};

class IRandomSource {
public:
    virtual ~IRandomSource() = default;
    virtual Result<Scalar, NumericalErrorCode> uniform() = 0;
    virtual Result<Scalar, NumericalErrorCode> normal() = 0;
    // Bias源脚本在真值前及真值后各重置一次；其它调用者不应隐式重置。
    // 不可重置的流须明确失败，不能悄悄继续消费或换用另一个具体RNG。
    virtual Status<NumericalErrorCode> reseed(std::uint32_t) {
        ErrorDetail detail; detail.stage=ErrorStage::Simulation;
        detail.message="Injected random source does not support reseeding";
        return Status<NumericalErrorCode>::failure(
            {NumericalErrorCode::InvalidRandomSample,std::move(detail)});
    }
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
