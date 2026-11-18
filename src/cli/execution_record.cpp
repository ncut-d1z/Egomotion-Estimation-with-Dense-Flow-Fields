#include "execution_record.hpp"
#include "build_identity.hpp"
#include "stream_output.hpp"

#include <cmath>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/utsname.h>
#include <utility>

namespace egomotion::cli {
namespace {
std::string timestamp() {
    const std::time_t now = std::time(nullptr);
    std::tm utc{};
    char text[32]{};
    if (now == std::time_t(-1) || !::gmtime_r(&now, &utc) ||
        !std::strftime(text, sizeof(text), "%Y-%m-%dT%H:%M:%SZ", &utc)) return "null";
    return jsonQuote(text);
}
std::string stringOrNull(const std::string& value) {
    return value.empty() ? "null" : jsonQuote(value);
}
std::string osRelease() {
    // 不source该文件，不枚举环境变量或主机名；只读发行版三个公开字段。
    std::ifstream input("/etc/os-release");
    std::map<std::string, std::string> selected;
    std::string line;
    while (std::getline(input, line)) {
        const auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        const auto key = line.substr(0, pos);
        if (key != "ID" && key != "VERSION_ID" && key != "PRETTY_NAME") continue;
        auto value = line.substr(pos + 1);
        if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') ||
                                 (value.front() == '\'' && value.back() == '\'')))
            value = value.substr(1, value.size() - 2);
        selected[key] = value;
    }
    if (!input.eof()) return "null";
    std::string json = "{";
    for (const auto& item : selected) {
        if (json.size() > 1) json += ',';
        json += jsonQuote(item.first) + ':' + jsonQuote(item.second);
    }
    return json + '}';
}
std::string environment() {
    struct utsname system{};
    const bool identified = ::uname(&system) == 0;
    std::string json = "{\"os_release\":" + osRelease() + ",\"kernel\":";
    if (identified)
        json += "{\"system\":" + jsonQuote(system.sysname) +
            ",\"release\":" + jsonQuote(system.release) +
            ",\"machine\":" + jsonQuote(system.machine) + '}';
    else json += "null";
    json += ",\"build\":{\"compiler_id\":" + stringOrNull(build_identity::compiler_id);
    json += ",\"compiler_version\":" + stringOrNull(build_identity::compiler_version);
    json += ",\"cmake_version\":" + stringOrNull(build_identity::cmake_version);
    json += ",\"configuration\":" + jsonQuote(EGOMOTION_BUILD_CONFIGURATION);
#ifdef __GLIBCXX__
    json += ",\"libstdcxx_build_date\":" + std::to_string(__GLIBCXX__);
#endif
    json += ",\"cpp_standard\":17,\"double_digits\":" + std::to_string(std::numeric_limits<double>::digits);
    json += ",\"svg_enabled\":" + std::string(build_identity::svg_enabled ? "true" : "false");
    json += ",\"dependencies\":{\"eigen\":" + stringOrNull(build_identity::eigen_version);
    json += ",\"matio\":" + stringOrNull(build_identity::matio_version);
    json += ",\"zlib\":" + stringOrNull(build_identity::zlib_version);
    json += ",\"sophus\":" + stringOrNull(build_identity::sophus_version) + "}}}";
    return json;
}
std::string parameters(const Command& command) {
    if (command.kind == CommandKind::InspectMat) {
        const auto& args = std::get<InspectMatArgs>(command.arguments);
        return "{\"file\":" + jsonQuote(args.source.string()) + ",\"variable\":" +
            (args.variable ? jsonQuote(*args.variable) : "null") + '}';
    }
    if (command.kind == CommandKind::Plot || command.kind == CommandKind::Analyze) {
        const auto& args = std::get<AnalysisArgs>(command.arguments);
        const char* kind = nullptr;
        switch (args.kind) {
        case AnalysisKind::Bias: kind = "bias"; break;
        case AnalysisKind::AdaptiveInitialization: kind = "adaptive-init"; break;
        case AnalysisKind::AdaptiveComparison: kind = "adaptive-comparison"; break;
        case AnalysisKind::AdaptiveUpdateRates: kind = "adaptive-update-rates"; break;
        }
        if (!kind) throw std::invalid_argument("Unknown plot analysis kind");
        return "{\"experiment\":" + jsonQuote(kind) + ",\"input_root\":" +
            jsonQuote(args.input_root.string()) + '}';
    }
    if (command.kind == CommandKind::Illustrate) {
        const auto& args = std::get<IllustrationArgs>(command.arguments);
        const auto format = jsonQuote(args.svg ? "svg" : "json");
        if (const auto* harmonic = std::get_if<illustrations::HarmonicConfig>(&args.config))
            return "{\"format\":" + format + ",\"kind\":\"harmonic\",\"degree\":" +
                std::to_string(harmonic->degree) + ",\"order\":" + std::to_string(harmonic->order) + '}';
        return "{\"format\":" + format + ",\"kind\":" + jsonQuote(
            std::holds_alternative<illustrations::CircleConfig>(args.config) ? "circle" : "sinusoidal") + '}';
    }
    if (command.kind == CommandKind::BiasOccluded || command.kind == CommandKind::BiasProportional) {
        const auto& config = std::get<BiasArgs>(command.arguments).config;
        return "{\"seed\":" + std::to_string(config.seed) +
            ",\"maximum_subdivision_level\":" + std::to_string(config.maximum_subdivision_level) + '}';
    }
    if (command.kind == CommandKind::AdaptiveComparison) {
        const double scale = std::get<AdaptiveComparisonArgs>(command.arguments).config.noise_scale;
        if (!std::isfinite(scale)) return "{\"noise_scale\":null,\"noise_scale_is_finite\":false}";
        std::ostringstream value;
        value.imbue(std::locale::classic());
        value.exceptions(std::ios::failbit | std::ios::badbit);
        value << std::setprecision(std::numeric_limits<double>::max_digits10) << scale;
        return "{\"noise_scale\":" + value.str() + '}';
    }
    return "{}"; // Initialization/UpdateRates的Config无数值字段，常量由源码SHA追踪。
}
std::string commandName(CommandKind command) {
    switch (command) {
    case CommandKind::Help: return "help";
    case CommandKind::Version: return "version";
    case CommandKind::InspectMat: return "inspect-mat";
    case CommandKind::Analyze: return "analyze";
    case CommandKind::BiasOccluded: return "run-bias-occluded";
    case CommandKind::BiasProportional: return "run-bias-proportional";
    case CommandKind::AdaptiveInitialization: return "run-adaptive-init";
    case CommandKind::AdaptiveComparison: return "run-adaptive-comparison";
    case CommandKind::AdaptiveUpdateRates: return "run-adaptive-update-rates";
    case CommandKind::Plot: return "plot";
    case CommandKind::Illustrate: return "illustrate";
    default: throw std::invalid_argument("Invocation record requires an output command");
    }
}
std::string invocationPrefix(const Command& command, const std::vector<std::string>* argv,
                              const RunId& id, const std::filesystem::path& output_directory,
                              const std::string& repository_sha, const std::string& source_blob) {
    std::error_code error;
    const auto cwd = std::filesystem::current_path(error);
    const std::string cwd_json = error ? "null" : jsonQuote(cwd.string());
    const auto executable = std::filesystem::read_symlink("/proc/self/exe", error);
    const std::string executable_json = error ? "null" : jsonQuote(executable.string());
    std::string json = "{\"schema\":\"egomotion-invocation-v1\",\"phase\":\"started\"";
    json += ",\"run_id\":" + jsonQuote(id.text) + ",\"started_utc\":" + timestamp();
    json += ",\"repo_sha\":" + jsonQuote(repository_sha);
    json += ",\"source_blob_sha\":" + jsonQuote(source_blob);
    json += ",\"command\":" + jsonQuote(commandName(command.kind));
    json += ",\"arguments_exclude_argv0\":true,\"arguments\":";
    if (argv) {
        json += '[';
        for (std::size_t i = 0; i < argv->size(); ++i) {
            if (i) json += ',';
            json += jsonQuote((*argv)[i]);
        }
        json += ']';
    } else json += "null";
    json += ",\"executable\":" + executable_json + ",\"working_directory\":" + cwd_json;
    json += ",\"output_directory\":" + jsonQuote(output_directory.string());
    json += ",\"effective_config\":" + parameters(command);
    return json;
}
std::string stringArray(const std::vector<std::string>& values) {
    std::string json = "[";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i) json += ',';
        json += jsonQuote(values[i]);
    }
    return json + ']';
}
std::string artifactSuffix() {
    return ",\"record_scope\":\"artifact_export\",\"algorithm_rng\":{\"engine\":null,\"seed\":null,\"seed_origin\":\"not_used\"}"
        ",\"environment\":" + environment() +
        ",\"execution_record\":\"execution.json\",\"calculation_manifest\":null}\n";
}
}  // namespace

std::string processTimestamp() { return timestamp(); }
std::string processEnvironment() { return environment(); }
std::string commandObservation(const Command& command) {
    return "{\"command\":" + jsonQuote(commandName(command.kind)) +
        ",\"effective_config\":" + parameters(command) + '}';
}
std::string analysisObservation(const AnalysisDataset& data) {
    std::string json = "{\"source_blob_sha\":" + jsonQuote(data.source_analysis_blob) + ",\"curves\":[";
    for (std::size_t index = 0; index < data.curves.size(); ++index) {
        if (index) json += ',';
        json += "{\"id\":" + jsonQuote(data.curves[index].name) + ",\"helper_source_blob_shas\":" +
            stringArray(data.curves[index].helper_source_blob_shas) + '}';
    }
    return json + "]}";
}
std::string illustrationObservation(const illustrations::Data& data) {
    return "{\"source_blob_sha\":" + jsonQuote(data.source_script_blob) +
        ",\"helper_source_blob_shas\":" + stringArray(data.helper_source_blobs) + '}';
}

std::string invocationRecord(const Command& command, const std::vector<std::string>* argv,
                             const RunContext& context) {
    const auto& provenance = context.provenance();
    auto json = invocationPrefix(command, argv, context.runId(),
        context.approvedOutputRoot() / context.runId().text,
        provenance.repository_commit_sha, provenance.matlab_source_blob_sha);
    json += ",\"algorithm_rng\":{\"engine\":\"std::mt19937\",\"seed\":";
    json += provenance.rng_seed ? std::to_string(*provenance.rng_seed) : "null";
    json += ",\"seed_origin\":" + jsonQuote(command.kind == CommandKind::BiasOccluded ||
        command.kind == CommandKind::BiasProportional ? "config" : "os_random") + '}';
    json += ",\"inputs\":{\"kind\":\"generated_from_source_and_rng\",\"mat_files\":[]}";
    json += ",\"environment\":" + environment();
    json += ",\"execution_record\":\"execution.json\",\"calculation_manifest\":\"manifest.json\"}\n";
    return json;
}

std::string artifactInvocationRecord(const Command& command, const std::vector<std::string>* argv,
                                      const RunId& id, const std::filesystem::path& output_directory,
                                      const AnalysisDataset& data) {
    auto json = invocationPrefix(command, argv, id, output_directory,
        EGOMOTION_BUILD_SOURCE_SHA, data.source_analysis_blob);
    // 使用Analyzer读取时已绑定的身份，不重新打开路径计算一个可能已变化的摘要。
    json += ",\"inputs\":{\"kind\":\"analyzed_mat_snapshots\",\"curves\":[";
    for (std::size_t i = 0; i < data.curves.size(); ++i) {
        if (i) json += ',';
        const auto& curve = data.curves[i];
        json += "{\"id\":" + jsonQuote(curve.name) + ",\"helper_source_blob_shas\":" +
            stringArray(curve.helper_source_blob_shas) + ",\"mat_files\":[";
        for (std::size_t j = 0; j < curve.inputs.size(); ++j) {
            if (j) json += ',';
            const auto& input = curve.inputs[j];
            json += "{\"path\":" + jsonQuote(input.path.string()) +
                ",\"variable\":" + jsonQuote(input.variable) +
                ",\"role\":" + jsonQuote(input.role) +
                ",\"matlab_run_ordinal\":" + std::to_string(input.matlab_run_ordinal) +
                ",\"mat_sha256\":" + (input.mat_sha256 ? jsonQuote(*input.mat_sha256) : "null") + '}';
        }
        json += "]}";
    }
    return json + "]}" + artifactSuffix();
}

std::string artifactInvocationRecord(const Command& command, const std::vector<std::string>* argv,
                                      const RunId& id, const std::filesystem::path& output_directory,
                                      const illustrations::Data& data) {
    auto json = invocationPrefix(command, argv, id, output_directory,
        EGOMOTION_BUILD_SOURCE_SHA, data.source_script_blob);
    json += ",\"inputs\":{\"kind\":\"generated_from_source_and_config\",\"mat_files\":[],"
        "\"helper_source_blob_shas\":" + stringArray(data.helper_source_blobs) + '}';
    return json + artifactSuffix();
}

std::string executionRecord(const RunId& id, const CommandDelivery& delivery,
                            bool invocation_confirmed, bool has_calculation_manifest) {
    std::string json = "{\"schema\":\"egomotion-execution-v1\",\"run_id\":" + jsonQuote(id.text);
    json += ",\"finished_utc\":" + timestamp();
    json += ",\"invocation_confirmed\":" + std::string(invocation_confirmed ? "true" : "false");
    // 记录本身也可能发布失败。该条件字段不把随后返回6冒称为已存的0/5。
    json += ",\"exit_code_if_record_published\":" + std::to_string(delivery.exit_code);
    json += ",\"exit_code_on_record_failure\":6,\"invocation_record\":\"invocation.json\"";
    json += ",\"calculation_manifest\":";
    json += has_calculation_manifest ? "\"manifest.json\"" : "null";
    json += ",\"channel_attempts\":[";
    bool first = true;
    for (const auto& channel : delivery.channels) {
        if (!first) json += ',';
        first = false;
        json += "{\"channel\":" + jsonQuote(channel.channel) +
            ",\"attempted_text\":" + jsonQuote(channel.attempted_text) +
            ",\"write_and_flush_succeeded\":" + (channel.write_and_flush_succeeded ? "true" : "false") + '}';
    }
    return json + "]}\n";
}

CommandDelivery deliverRecorded(std::string stdout_text, const std::vector<std::string>& diagnostics,
                                int computation_exit, const RunId& id,
                                std::ostream& out, std::ostream& err) {
    CommandDelivery delivery;
    delivery.exit_code = computation_exit;
    delivery.channels.reserve(diagnostics.size() + 2);
    const auto attempt = [&](const char* channel, std::string text, std::ostream& target) {
        delivery.channels.push_back({channel, std::move(text), false});
        auto& entry = delivery.channels.back();
        entry.write_and_flush_succeeded = streams::writeAndFlush(target, entry.attempted_text);
        if (!entry.write_and_flush_succeeded) delivery.exit_code = 6;
        return entry.write_and_flush_succeeded;
    };
    if (!stdout_text.empty() && !attempt("stdout", std::move(stdout_text), out)) {
        ErrorDetail detail; detail.stage = ErrorStage::Output; detail.run_id = id;
        detail.message = "CLI stdout write or flush failed";
        attempt("stderr", jsonFailure(Failure<ExperimentErrorCode>{ExperimentErrorCode::OutputFailed,
                std::move(detail)}, 6) + '\n', err);
    }
    for (const auto& diagnostic : diagnostics) attempt("stderr", diagnostic, err);
    return delivery;
}
}  // namespace egomotion::cli
