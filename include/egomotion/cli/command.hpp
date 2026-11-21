#pragma once

#include "egomotion/analysis/analyzer.hpp"
#include "egomotion/analysis/plot.hpp"
#include "egomotion/analysis/paper_figures.hpp"
#include "egomotion/experiments/runner.hpp"
#include "egomotion/illustrations/illustration.hpp"
#include "egomotion/cli/invocation_observer.hpp"

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
    Analyze, Plot, Illustrate, PaperFigure
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
struct IllustrationArgs {
    illustrations::Config config;
    bool svg=false;
    std::optional<RunId> run_id;
};
struct PaperArgs { paper::FigureConfig config; std::optional<RunId> run_id; };

using CommandArguments = std::variant<
    std::monostate, InspectMatArgs, BiasArgs, AdaptiveInitArgs,
    AdaptiveComparisonArgs, AdaptiveRatesArgs, AnalysisArgs, IllustrationArgs, PaperArgs>;
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
                                        std::filesystem::path trusted_repo = {},
                                        illustrations::IExporter* illustrator = nullptr,
                                        IInvocationObserver* observer = nullptr)
        : reader_(reader), runner_(runner), analyzer_(analyzer),
          plotter_(plotter), trusted_repo_(std::move(trusted_repo)), illustrator_(illustrator), observer_(observer) {}

    int dispatch(const Command& command, std::ostream& out,
                 std::ostream& err,
                 const std::vector<std::string>* invocation_arguments = nullptr) const;

private:
    IResultReader* reader_;
    IExperimentRunner* runner_;
    IAnalyzer* analyzer_;
    IPlotExporter* plotter_;
    std::filesystem::path trusted_repo_;
    illustrations::IExporter* illustrator_;
    IInvocationObserver* observer_;
};

int run(const std::vector<std::string>& args, std::ostream& out,
        std::ostream& err, const ApplicationCompositionRoot& application);

}  // namespace egomotion::cli
