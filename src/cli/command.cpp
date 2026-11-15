#include "egomotion/cli/command.hpp"

#include <cmath>
#include <cstdint>
#include <limits>
#include <locale>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace egomotion::cli {
namespace {
using Options = std::map<std::string, std::string>;

Result<Command, ExperimentErrorCode> invalid(std::string message) {
    ErrorDetail detail;
    detail.stage = ErrorStage::Parse;
    detail.message = std::move(message);
    return Result<Command, ExperimentErrorCode>::failure(
        {ExperimentErrorCode::InvalidParameters, std::move(detail)});
}
bool unsignedDecimal(std::string_view input, std::uint64_t limit, std::uint64_t& value) {
    if (input.empty()) return false;
    value = 0;
    for (char c : input) {
        if (c < '0' || c > '9') return false;
        const unsigned digit = static_cast<unsigned>(c - '0');
        if (value > (limit - digit) / 10 || digit > limit) return false;
        value = value * 10 + digit;
    }
    return true;
}
bool finiteDecimal(const std::string& input, Scalar& value) {
    if (input.empty()) return false;
    std::istringstream stream(input);
    stream.imbue(std::locale::classic());
    stream >> std::noskipws >> value;
    return static_cast<bool>(stream) &&
           stream.peek() == std::char_traits<char>::eof() &&
           std::isfinite(value);
}
bool safePath(std::string_view name) {
    return !name.empty() && name.find('\0') == std::string_view::npos;
}
bool safeRunId(std::string_view id) {
    if (id.empty() || id.size() > 64) return false;
    for (unsigned char c : id) {
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-')) return false;
    }
    return true;
}
std::optional<RunId> requestedRunId(const Options& options) {
    const auto it = options.find("--run-id");
    return it == options.end() ? std::nullopt
                               : std::optional<RunId>(RunId{it->second});
}
bool validKeys(const Options& options, const std::set<std::string>& allowed) {
    for (const auto& item : options)
        if (allowed.count(item.first) == 0) return false;
    return true;
}
bool has(const Options& options, std::string_view name) {
    return options.find(std::string(name)) != options.end();
}
}  // namespace

Result<Command, ExperimentErrorCode> parseCommand(
    const std::vector<std::string>& args) {
    if (args.empty() || (args.size() == 1 &&
        (args[0] == "help" || args[0] == "--help" || args[0] == "-h"))) {
        return Result<Command, ExperimentErrorCode>::success(
            {CommandKind::Help, std::monostate{}});
    }
    if (args.size() == 1 &&
        (args[0] == "version" || args[0] == "--version")) {
        return Result<Command, ExperimentErrorCode>::success(
            {CommandKind::Version, std::monostate{}});
    }
    const std::string& name = args.front();
    CommandKind kind;
    if (name == "inspect-mat") kind = CommandKind::InspectMat;
    else if (name == "run-bias-occluded") kind = CommandKind::BiasOccluded;
    else if (name == "run-bias-proportional") kind = CommandKind::BiasProportional;
    else if (name == "run-adaptive-init") kind = CommandKind::AdaptiveInitialization;
    else if (name == "run-adaptive-comparison") kind = CommandKind::AdaptiveComparison;
    else if (name == "run-adaptive-update-rates") kind = CommandKind::AdaptiveUpdateRates;
    else if (name == "analyze") kind = CommandKind::Analyze;
    else if (name == "plot") kind = CommandKind::Plot;
    else return invalid("Unknown command: " + name);

    Options options;
    if (args.size() % 2 == 0) return invalid("Option requires a value");
    for (std::size_t i = 1; i < args.size(); i += 2) {
        if (args[i].size() < 3 || args[i].compare(0, 2, "--") != 0 ||
            args[i + 1].compare(0, 2, "--") == 0)
            return invalid("Expected --option VALUE pair");
        if (!options.emplace(args[i], args[i + 1]).second)
            return invalid("Duplicate option: " + args[i]);
    }

    if (has(options, "--run-id") && !safeRunId(options.at("--run-id")))
        return invalid("Invalid RunId: use 1-64 ASCII letters, digits, _ or -");
    const auto id = requestedRunId(options);
    if (kind == CommandKind::InspectMat) {
        if (!validKeys(options, {"--file", "--variable"}) ||
            !has(options, "--file") || !safePath(options.at("--file")) ||
            (has(options, "--variable") && !safePath(options.at("--variable"))))
            return invalid("inspect-mat requires --file PATH; only --variable NAME is optional");
        InspectMatArgs value{options.at("--file"), std::nullopt};
        if (has(options, "--variable")) value.variable = options.at("--variable");
        return Result<Command, ExperimentErrorCode>::success(
            {kind, std::move(value)});
    }
    if (kind == CommandKind::BiasOccluded || kind == CommandKind::BiasProportional) {
        if (!validKeys(options, {"--seed", "--level", "--run-id"}) ||
            !has(options, "--seed") || !has(options, "--level"))
            return invalid("Bias requires --seed UINT32 and --level NONNEG_INT");
        std::uint64_t seed = 0, level = 0;
        if (!unsignedDecimal(options.at("--seed"), UINT32_MAX, seed) ||
            !unsignedDecimal(options.at("--level"), INT32_MAX, level))
            return invalid("Invalid --seed or --level decimal range");
        return Result<Command, ExperimentErrorCode>::success(
            {kind, BiasArgs{{static_cast<std::uint32_t>(seed),
                              static_cast<int>(level)}, id}});
    }
    if (kind == CommandKind::AdaptiveInitialization ||
        kind == CommandKind::AdaptiveUpdateRates) {
        if (!validKeys(options, {"--run-id"}))
            return invalid("Only --run-id ID is supported for this experiment");
        if (kind == CommandKind::AdaptiveInitialization)
            return Result<Command, ExperimentErrorCode>::success(
                {kind, AdaptiveInitArgs{id}});
        return Result<Command, ExperimentErrorCode>::success(
            {kind, AdaptiveRatesArgs{id}});
    }
    if (kind == CommandKind::AdaptiveComparison) {
        if (!validKeys(options, {"--noise-scale", "--run-id"}) ||
            !has(options, "--noise-scale"))
            return invalid("Comparison requires --noise-scale FINITE_DOUBLE");
        Scalar noise_scale = 0;
        if (!finiteDecimal(options.at("--noise-scale"), noise_scale))
            return invalid("Invalid finite --noise-scale");
        return Result<Command, ExperimentErrorCode>::success(
            {kind, AdaptiveComparisonArgs{{noise_scale}, id}});
    }
    if (!validKeys(options, kind == CommandKind::Plot
                          ? std::set<std::string>{"--experiment", "--input-root", "--run-id"}
                          : std::set<std::string>{"--experiment", "--input-root"}) ||
        !has(options, "--experiment") || !has(options, "--input-root") ||
        !safePath(options.at("--input-root")))
        return invalid("analyze/plot requires --experiment KIND and --input-root DIR");
    const std::string& exp = options.at("--experiment");
    AnalysisKind analysis_kind;
    if (exp == "bias") analysis_kind = AnalysisKind::Bias;
    else if (exp == "adaptive-init") analysis_kind = AnalysisKind::AdaptiveInitialization;
    else if (exp == "adaptive-comparison") analysis_kind = AnalysisKind::AdaptiveComparison;
    else if (exp == "adaptive-update-rates") analysis_kind = AnalysisKind::AdaptiveUpdateRates;
    else return invalid("Invalid --experiment value: " + exp);
    return Result<Command, ExperimentErrorCode>::success(
        {kind, AnalysisArgs{analysis_kind, options.at("--input-root"), id}});
}

}  // namespace egomotion::cli
