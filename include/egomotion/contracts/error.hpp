#pragma once

#include "egomotion/domain/types.hpp"

#include <cstddef>
#include <forward_list>
#include <optional>
#include <string>
#include <string_view>

namespace egomotion {

enum class MatErrorCode {
    MissingFile, PermissionDenied, InvalidMatHeader, CorruptCompressedData,
    MissingVariable, AmbiguousVariable, UnsupportedClassOrType,
    InvalidShape, SizeOverflow, OutputPathForbidden, OutputExists,
    WriteFailed, ReadbackMismatch
};
enum class NumericalErrorCode {
    EmptyInput, InvalidShape, NonFiniteInput, ZeroNorm,
    InvalidDirection, InvalidNearness, SingularSystem,
    IllConditionedSystem, NonFiniteResult, UndefinedAngle,
    InvalidSubdivisionLevel, SampleCountOverflow, ResourceLimitExceeded,
    RandomSourceExhausted, InvalidRandomSample
};
enum class ExperimentErrorCode {
    InvalidParameters, SourceUnavailable, DataMismatch,
    SolverFailed, OutputFailed, IncompleteRun
};
enum class ErrorStage {
    Parse, Read, Decode, Output, Solve, Simulation, Analyze, Plot
};

inline constexpr std::string_view stageName(ErrorStage stage) noexcept {
    switch (stage) {
    case ErrorStage::Parse: return "parse";
    case ErrorStage::Read: return "read";
    case ErrorStage::Decode: return "decode";
    case ErrorStage::Output: return "output";
    case ErrorStage::Solve: return "solve";
    case ErrorStage::Simulation: return "simulation";
    case ErrorStage::Analyze: return "analyze";
    case ErrorStage::Plot: return "plot";
    }
    return "unknown";
}

template<class Code> constexpr std::string_view errorDomain() noexcept;
template<> constexpr std::string_view errorDomain<MatErrorCode>() noexcept { return "MatErrorCode"; }
template<> constexpr std::string_view errorDomain<NumericalErrorCode>() noexcept { return "NumericalErrorCode"; }
template<> constexpr std::string_view errorDomain<ExperimentErrorCode>() noexcept { return "ExperimentErrorCode"; }

// Single source of the enum names used in ErrorFrame and CLI JSONL.
inline constexpr std::string_view errorName(MatErrorCode code) noexcept {
    switch (code) {
    case MatErrorCode::MissingFile: return "MissingFile";
    case MatErrorCode::PermissionDenied: return "PermissionDenied";
    case MatErrorCode::InvalidMatHeader: return "InvalidMatHeader";
    case MatErrorCode::CorruptCompressedData: return "CorruptCompressedData";
    case MatErrorCode::MissingVariable: return "MissingVariable";
    case MatErrorCode::AmbiguousVariable: return "AmbiguousVariable";
    case MatErrorCode::UnsupportedClassOrType: return "UnsupportedClassOrType";
    case MatErrorCode::InvalidShape: return "InvalidShape";
    case MatErrorCode::SizeOverflow: return "SizeOverflow";
    case MatErrorCode::OutputPathForbidden: return "OutputPathForbidden";
    case MatErrorCode::OutputExists: return "OutputExists";
    case MatErrorCode::WriteFailed: return "WriteFailed";
    case MatErrorCode::ReadbackMismatch: return "ReadbackMismatch";
    }
    return "Unknown";
}
inline constexpr std::string_view errorName(NumericalErrorCode code) noexcept {
    switch (code) {
    case NumericalErrorCode::EmptyInput: return "EmptyInput";
    case NumericalErrorCode::InvalidShape: return "InvalidShape";
    case NumericalErrorCode::NonFiniteInput: return "NonFiniteInput";
    case NumericalErrorCode::ZeroNorm: return "ZeroNorm";
    case NumericalErrorCode::InvalidDirection: return "InvalidDirection";
    case NumericalErrorCode::InvalidNearness: return "InvalidNearness";
    case NumericalErrorCode::SingularSystem: return "SingularSystem";
    case NumericalErrorCode::IllConditionedSystem: return "IllConditionedSystem";
    case NumericalErrorCode::NonFiniteResult: return "NonFiniteResult";
    case NumericalErrorCode::UndefinedAngle: return "UndefinedAngle";
    case NumericalErrorCode::InvalidSubdivisionLevel: return "InvalidSubdivisionLevel";
    case NumericalErrorCode::SampleCountOverflow: return "SampleCountOverflow";
    case NumericalErrorCode::ResourceLimitExceeded: return "ResourceLimitExceeded";
    case NumericalErrorCode::RandomSourceExhausted: return "RandomSourceExhausted";
    case NumericalErrorCode::InvalidRandomSample: return "InvalidRandomSample";
    }
    return "Unknown";
}
inline constexpr std::string_view errorName(ExperimentErrorCode code) noexcept {
    switch (code) {
    case ExperimentErrorCode::InvalidParameters: return "InvalidParameters";
    case ExperimentErrorCode::SourceUnavailable: return "SourceUnavailable";
    case ExperimentErrorCode::DataMismatch: return "DataMismatch";
    case ExperimentErrorCode::SolverFailed: return "SolverFailed";
    case ExperimentErrorCode::OutputFailed: return "OutputFailed";
    case ExperimentErrorCode::IncompleteRun: return "IncompleteRun";
    }
    return "Unknown";
}

struct ErrorFrame {
    std::string domain;
    std::string code;
    ErrorStage stage = ErrorStage::Solve;
    std::string message;
    std::string source_path;
    std::string variable;
    std::optional<std::size_t> observation_index;
    std::optional<RunId> run_id;
};

struct ErrorDetail {
    std::optional<RunId> run_id;
    ErrorStage stage = ErrorStage::Solve;
    std::string message;
    std::string source_path;
    std::string variable;
    std::optional<std::size_t> observation_index;
    // Front is the nearest inner wrapper; back is the original failure.
    std::forward_list<ErrorFrame> causes;
};

template<class Code>
struct Failure {
    Code code;
    ErrorDetail detail;
};

template<class Code>
ErrorFrame toFrame(const Failure<Code>& failure) {
    return {std::string(errorDomain<Code>()), std::string(errorName(failure.code)),
            failure.detail.stage, failure.detail.message, failure.detail.source_path,
            failure.detail.variable, failure.detail.observation_index, failure.detail.run_id};
}

// Copy first, then prepend the immediate inner error: allocation failure cannot
// truncate the original Failure chain. Every real wrapping layer gets one frame.
template<class OuterCode, class InnerCode>
Failure<OuterCode> wrapFailure(OuterCode code, ErrorDetail outer,
                              const Failure<InnerCode>& inner) {
    auto new_chain = inner.detail.causes;
    new_chain.push_front(toFrame(inner));
    outer.causes = std::move(new_chain);
    return {code, std::move(outer)};
}

}  // namespace egomotion
