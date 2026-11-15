#pragma once

#include <cstddef>
#include <filesystem>
#include <string>

namespace egomotion::io {

struct MatDictionaryOptions {
    std::filesystem::path root;
    std::filesystem::path output;
    std::string source_sha = "not-specified";
    std::size_t expected = 582;
    bool replace = false;
};
struct MatDictionarySummary {
    std::size_t scanned = 0;
    std::size_t succeeded = 0;
    std::size_t failed = 0;
    std::size_t variables = 0;
};

// Uses MatResultReader for all MAT-related information and full decodes.
// The CLI owns argv, error display and exit codes, not scanning or report I/O.
MatDictionarySummary generateMatDictionaryReport(MatDictionaryOptions options);

}  // namespace egomotion::io
