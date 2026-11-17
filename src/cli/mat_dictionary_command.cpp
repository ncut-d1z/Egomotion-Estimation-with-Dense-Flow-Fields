#include "egomotion/cli/mat_dictionary_command.hpp"
#include "egomotion/io/mat_data_dictionary.hpp"
#include "stream_output.hpp"

#include <cstddef>
#include <filesystem>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>

namespace egomotion::cli {
namespace {
constexpr int execution_failure_exit = 2;

io::MatDictionaryOptions parse(int argc, const char* const* argv) {
    if (argc < 3)
        throw std::invalid_argument(
            "Usage: mat_data_dict_probe MAT_ROOT REPORT.md "
            "[--source-sha SHA] [--expected-count N] [--replace-generated]");
    io::MatDictionaryOptions options;
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

void reportException(std::ostream& diagnostic, const char* message) noexcept {
    try {
        streams::writeAndFlush(diagnostic,
            std::string("mat_data_dict_probe: ") + message + "\n");
    } catch (...) {
        // 同一诊断通道或分配已失败，不再递归输出；调用方始终返回非零。
    }
}
}  // namespace

int runMatDictionaryCommand(int argc, const char* const* argv,
                            std::ostream& diagnostic) noexcept {
    try {
        const auto options = parse(argc, argv);
        const auto summary = io::generateMatDictionaryReport(options);
        std::ostringstream status;
        status.imbue(std::locale::classic());
        status.exceptions(std::ios::badbit | std::ios::failbit);
        status << "scanned=" << summary.scanned << ", ok=" << summary.succeeded
               << ", failed=" << summary.failed
               << ", variables=" << summary.variables << "\n";
        if (!streams::writeAndFlush(diagnostic, status.str()))
            return execution_failure_exit;
        return summary.scanned == options.expected && summary.failed == 0 ? 0 : 1;
    } catch (const std::exception& error) {
        reportException(diagnostic, error.what());
    } catch (...) {
        reportException(diagnostic, "Unknown failure");
    }
    return execution_failure_exit;
}
}  // namespace egomotion::cli
