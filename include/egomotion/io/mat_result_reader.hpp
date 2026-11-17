#pragma once

#include "egomotion/contracts/interfaces.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace egomotion::io {

enum class ScanMode { MetadataOnly, VerifyDecoded };

struct MatScanVariable {
    MatVariableInfo info;
    // MetadataOnly never asks matio for the numeric payload.
    // VerifyDecoded preserves the historical v5 dictionary READ_OK meaning;
    // unsupported v7.3 is rejected before any backend opens its payload.
    std::string status;
};
struct MatFileScan {
    std::string version;
    std::vector<MatScanVariable> variables;
};

class MatResultReader final : public IResultReader {
public:
    Result<std::vector<MatVariableInfo>, MatErrorCode>
    inspect(const std::filesystem::path& path) const override;

    Result<DenseDoubleArray, MatErrorCode>
    readNamed(const std::filesystem::path& path,
              std::string_view variable) const override;

    Result<DenseDoubleArray, MatErrorCode>
    readFirstCompatible(const std::filesystem::path& path) const override;

    // Probe-only inspection uses ONE MAT handle for directory and full read.
    // The public IResultReader remains metadata/value only and matio-free.
    Result<MatFileScan, MatErrorCode>
    scanFile(const std::filesystem::path& path, ScanMode mode) const;

    static std::string libraryVersion();
};

}  // namespace egomotion::io
