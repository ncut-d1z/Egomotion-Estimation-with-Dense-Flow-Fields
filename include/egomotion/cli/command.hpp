#pragma once

#include "egomotion/analysis/analyzer.hpp"
#include "egomotion/analysis/plot.hpp"
#include "egomotion/experiments/runner.hpp"

#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <utility>

namespace egomotion::cli {

enum class CommandKind {
    Help, Version, InspectMat,
    BiasOccluded, BiasProportional,
    AdaptiveInitialization, AdaptiveComparison, AdaptiveUpdateRates,
    Analyze, Plot
};

struct InspectMatArgs {
    std::filesystem::path source;
    std::optional<std::string> variable;
};
struct BiasArgs {
    BiasExperimentConfig config;
    std::optional<RunId> run_id;
};
struct AdaptiveInitArgs { std::optional<RunId> run_id; };
struct AdaptiveComparisonArgs {
    AdaptiveComparisonConfig config;
    std::optional<RunId> run_id;
};
struct AdaptiveRatesArgs { std::optional<RunId> run_id; };
struct AnalysisArgs {
    AnalysisKind kind;
    std::filesystem::path input_root;
    std::optional<RunId> run_id;
};

using CommandArguments = std::variant<
    std::monostate, InspectMatArgs, BiasArgs, AdaptiveInitArgs,
    AdaptiveComparisonArgs, AdaptiveRatesArgs, AnalysisArgs>;
struct Command {
    CommandKind kind;
    CommandArguments arguments;
};

// Strings exclude argv[0]. Invalid syntax never triggers I/O or random draws.
Result<Command, ExperimentErrorCode> parseCommand(
    const std::vector<std::string>& arguments);

// Composition root does not own injected services; their lifetime is external.
class ApplicationCompositionRoot {
public:
    explicit ApplicationCompositionRoot(IResultReader* reader = nullptr,
                                        IExperimentRunner* runner = nullptr,
                                        IAnalyzer* analyzer = nullptr,
                                        IPlotExporter* plotter = nullptr,
                                        std::filesystem::path trusted_repo = {})
        : reader_(reader), runner_(runner), analyzer_(analyzer),
          plotter_(plotter), trusted_repo_(std::move(trusted_repo)) {}

    int dispatch(const Command& command, std::ostream& out,
                 std::ostream& err) const;

private:
    IResultReader* reader_;
    IExperimentRunner* runner_;
    IAnalyzer* analyzer_;
    IPlotExporter* plotter_;
    std::filesystem::path trusted_repo_;
};

int run(const std::vector<std::string>& args, std::ostream& out,
        std::ostream& err, const ApplicationCompositionRoot& application);

}  // namespace egomotion::cli
