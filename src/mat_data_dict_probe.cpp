// Historical MAT dictionary command line. All MAT decoding and report
// generation live in src/io/; this file handles argv/exit/status only.
#include "egomotion/io/mat_data_dictionary.hpp"

#include <cstddef>
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace {
egomotion::io::MatDictionaryOptions parse(int argc, char** argv) {
    if (argc < 3)
        throw std::invalid_argument(
            "Usage: mat_data_dict_probe MAT_ROOT REPORT.md "
            "[--source-sha SHA] [--expected-count N] [--replace-generated]");
    egomotion::io::MatDictionaryOptions options;
    options.root = std::filesystem::path(argv[1]);
    options.output = std::filesystem::path(argv[2]);
    for (int i = 3; i < argc; ++i) {
        const std::string flag(argv[i]);
        if (flag == "--source-sha" && i + 1 < argc) {
            options.source_sha = argv[++i];
        } else if (flag == "--expected-count" && i + 1 < argc) {
            const std::string digits(argv[++i]);
            if (digits.empty())
                throw std::invalid_argument("Bad expected count");
            for (char digit : digits)
                if (digit < '0' || digit > '9')
                    throw std::invalid_argument("Bad expected count");
            std::size_t used = 0;
            const auto parsed = std::stoull(digits, &used);
            if (used != digits.size() || parsed == 0 ||
                parsed > std::numeric_limits<std::size_t>::max())
                throw std::invalid_argument("Bad expected count");
            options.expected = static_cast<std::size_t>(parsed);
        } else if (flag == "--replace-generated") {
            options.replace = true;
        } else {
            throw std::invalid_argument("Unknown option: " + flag);
        }
    }
    return options;
}
}  // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse(argc, argv);
        const auto summary = egomotion::io::generateMatDictionaryReport(options);
        std::cerr << "scanned=" << summary.scanned << ", ok=" << summary.succeeded
                  << ", failed=" << summary.failed
                  << ", variables=" << summary.variables << "\n";
        return summary.scanned == options.expected && summary.failed == 0 ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "mat_data_dict_probe: " << error.what() << "\n";
        return 2;
    }
}
