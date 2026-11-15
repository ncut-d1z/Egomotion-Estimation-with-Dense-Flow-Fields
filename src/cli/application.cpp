#include "egomotion/cli/command.hpp"
#include "egomotion/contracts/diagnostics.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <utility>

namespace egomotion::cli {
namespace {
void unavailable(std::ostream& error, std::string_view stage,
                 std::string_view name, int exit_code = 5) {
    error << jsonBackendUnavailable(stage,
        std::string(name) + " backend is not implemented (R0/R1 contract only)",
        exit_code) << '\n';
}
int matExit(MatErrorCode code) {
    switch (code) {
    case MatErrorCode::UnsupportedClassOrType:
    case MatErrorCode::InvalidShape:
    case MatErrorCode::SizeOverflow: return 4;
    case MatErrorCode::OutputPathForbidden:
    case MatErrorCode::OutputExists:
    case MatErrorCode::WriteFailed:
    case MatErrorCode::ReadbackMismatch: return 6;
    default: return 3;
    }
}
}  // namespace

int ApplicationCompositionRoot::dispatch(const Command& command,
                                          std::ostream& out,
                                          std::ostream& err) const {
    switch (command.kind) {
    case CommandKind::Help:
        out << "Usage: egomotion <command> [--option VALUE]\n"
               "Commands: inspect-mat, run-bias-occluded, run-bias-proportional,\n"
               "  run-adaptive-init, run-adaptive-comparison,\n"
               "  run-adaptive-update-rates, analyze, plot, help, version\n";
        return 0;
    case CommandKind::Version:
        out << "egomotion 0.1.0 (R0/R1 interface bootstrap)\n";
        return 0;
    case CommandKind::InspectMat: {
        if (!reader_) {
            unavailable(err, "read", "MAT reader");
            return 5;
        }
        const auto& params = std::get<InspectMatArgs>(command.arguments);
        auto result = reader_->inspect(params.source);
        if (!result.hasValue()) {
            const auto& failure = result.error();
            const int exit_code = matExit(failure.code);
            err << jsonFailure(failure, exit_code) << '\n';
            return exit_code;
        }
        bool found = false;
        for (const auto& variable : result.value()) {
            if (params.variable && *params.variable != variable.name) continue;
            found = true;
            out << "variable=" << variable.name
                << " class=" << variable.original_class_type
                << " type=" << variable.original_data_type
                << " rank=" << variable.dimensions.size() << " dims=";
            for (std::size_t i = 0; i < variable.dimensions.size(); ++i) {
                if (i != 0) out << 'x';
                out << variable.dimensions[i];
            }
            out << " data_bytes=" << variable.data_bytes
                << " compressed=" << variable.is_compressed
                << " complex=" << variable.is_complex
                << " logical=" << variable.is_logical
                << " global=" << variable.is_global << '\n';
        }
        if (params.variable && !found) {
            ErrorDetail detail;
            detail.stage = ErrorStage::Read;
            detail.message = "MAT variable not found";
            detail.source_path = params.source.string();
            detail.variable = *params.variable;
            err << jsonFailure(Failure<MatErrorCode>{MatErrorCode::MissingVariable,
                                                     std::move(detail)}, 3) << '\n';
            return 3;
        }
        return 0;
    }
    case CommandKind::Analyze:
        // No fake dataset: the R5 analyzer is not wired in R0/R1.
        (void)analyzer_;
        unavailable(err, "analyze", "Analyzer");
        return 5;
    case CommandKind::Plot:
        unavailable(err, "plot", "Plot", 7);
        return 7;
    default:
        // Deliberately never create output/<RunId> before R3/R5.
        (void)runner_;
        unavailable(err, "simulation", "Experiment Runner");
        return 5;
    }
}

int run(const std::vector<std::string>& args, std::ostream& out,
        std::ostream& err, const ApplicationCompositionRoot& application) {
    auto parsed = parseCommand(args);
    if (!parsed.hasValue()) {
        err << jsonFailure(parsed.error(), 2) << '\n';
        return 2;
    }
    return application.dispatch(parsed.value(), out, err);
}

}  // namespace egomotion::cli
