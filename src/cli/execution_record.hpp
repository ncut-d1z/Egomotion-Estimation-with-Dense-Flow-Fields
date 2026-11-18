#pragma once

#include "egomotion/cli/command.hpp"
#include "egomotion/experiments/run_context.hpp"

#include <string>
#include <vector>

namespace egomotion::cli {

// 进程级记录与应用级记录使用同一套公开环境白名单和UTC格式。
std::string processTimestamp();
std::string processEnvironment();
std::string commandObservation(const Command&);
std::string analysisObservation(const AnalysisDataset&);
std::string illustrationObservation(const illustrations::Data&);

struct ChannelDelivery {
    std::string channel;
    std::string attempted_text;
    bool write_and_flush_succeeded = false;
};
struct CommandDelivery {
    int exit_code = 0;
    std::vector<ChannelDelivery> channels;
};

// argv来自本次cli::run，明确不含argv[0]。直接dispatch时留null，绝不反推伪造。
std::string invocationRecord(const Command&, const std::vector<std::string>* argv,
                             const RunContext&);
std::string artifactInvocationRecord(const Command&, const std::vector<std::string>* argv,
                                      const RunId&, const std::filesystem::path& output_directory,
                                      const AnalysisDataset&);
std::string artifactInvocationRecord(const Command&, const std::vector<std::string>* argv,
                                      const RunId&, const std::filesystem::path& output_directory,
                                      const illustrations::Data&);
std::string executionRecord(const RunId&, const CommandDelivery&, bool invocation_confirmed,
                            bool has_calculation_manifest = true);
// 日志记录每次尝试的全文及完整交付状态，失败时不捏造已送达字节数。
CommandDelivery deliverRecorded(std::string stdout_text,
                                const std::vector<std::string>& diagnostics,
                                int computation_exit, const RunId&,
                                std::ostream& out, std::ostream& err);

}  // namespace egomotion::cli
