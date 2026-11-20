#pragma once

#include "egomotion/experiments/output_contract.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace egomotion {

namespace experiments { struct AdaptiveRunData; }

struct SourceProvenance {
    std::string repository_commit_sha;
    std::string matlab_source_blob_sha; // source identity, not MATLAB execution
    std::optional<std::uint32_t> rng_seed; // exact injected C++ seed, if applicable
};

// Unpersisted audit state ONLY. This is NOT a published Manifest.
struct RunManifestDraft {
    RunStatus status=RunStatus::Failed; // never imply Complete before manifest
    std::size_t attempted=0;
    std::size_t completed=0;
    std::size_t valid=0;
    std::size_t failed=0;
    std::size_t undefined=0;
    std::size_t expected_repetitions=0;
    std::vector<std::string> files_writer_confirmed;
    std::vector<Failure<ExperimentErrorCode>> errors;
};

// RunContext manages per-run state, never file path construction or MAT APIs.
// Composition Root owns and validates output/<RunId>/ dir identity before
// creating this object; concrete Writer performs its own path checks.
// It CANNOT certify that a Manifest has been durably published.
class RunContext {
public:
    RunContext(RunId run_id, std::unique_ptr<IRandomSource> random,
               IResultWriter& writer,ITextPublisher& manifest_publisher,
               std::filesystem::path validated_output_root,
               SourceProvenance provenance, experiments::ExperimentKind kind,
               std::size_t expected_repetitions);

    RunContext(const RunContext&)=delete;
    RunContext& operator=(const RunContext&)=delete;
    RunContext(RunContext&&)=delete;
    RunContext& operator=(RunContext&&)=delete;

    const RunId& runId() const noexcept { return run_id_; }
    IRandomSource& randomSource() noexcept { return *random_; }
    IResultWriter& resultWriter() noexcept { return writer_; }
    const SourceProvenance& provenance() const noexcept { return provenance_; }
    const std::filesystem::path& approvedOutputRoot() const noexcept {
        return output_root_;
    }
    experiments::ExperimentKind experimentKind() const noexcept { return kind_; }

    // Bias only: call for a COMPLETED in-memory experiment. Validates every
    // intended source-save field before invoking Writer even once.
    // On partial Writer failure, the committed file names remain recorded,
    // and the single-run Result returns failure (never fake Complete).
    Status<ExperimentErrorCode> writeCurrentResults(
        const std::map<std::string,DenseDoubleArray>& outputs);

    // Adaptive v2：先发布指标政策与同形状有效次数，再发布带显式缺测掩码的值。
    // 部分统计仍须完成全部求解轮次；求解失败不能借此提交部分平均。
    Status<ExperimentErrorCode> writeAdaptiveResults(
        const experiments::AdaptiveRunData& outputs);

    // Bias only, after all necessary source repetitions succeeded.
    Status<ExperimentErrorCode> noteCompletedMemory(
        std::size_t repetitions,std::size_t valid_samples);
    Status<ExperimentErrorCode> noteAdaptiveCompletedMemory(
        const experiments::AdaptiveRunData& outputs);

    // Bias v1 publication validates exact receipts and publishes manifest.json
    // durably before returning success. Adaptive uses its explicit v2 gateway.
    Result<RunOutcome,ExperimentErrorCode> publishCompleteOutcome();
    Result<RunOutcome,ExperimentErrorCode> publishAdaptiveOutcome();

    // A failed or incomplete run still attempts to persist a truthful
    // Failed/Incomplete audit. Failure to persist is reported separately.
    Status<ExperimentErrorCode> publishFailureRecord(
        Failure<ExperimentErrorCode> original_failure);

    // Counts and failure events can be observed by the future Manifest
    // publisher. They are never themselves durable evidence of a file.
    void noteAttempt() noexcept { ++draft_.attempted; }
    void noteCompleted() noexcept { ++draft_.completed; }
    void noteValid() noexcept { ++draft_.valid; }
    void noteFailed() noexcept { ++draft_.failed; }
    void noteUndefined() noexcept { ++draft_.undefined; }
    void recordFailure(Failure<ExperimentErrorCode> failure);
    const RunManifestDraft& manifestDraft() const noexcept { return draft_; }
    const std::vector<std::string>& expectedOutputNames() const noexcept {
        return expected_output_names_;
    }
    const std::string& adaptiveMetricValidityJson() const noexcept {
        return adaptive_metric_validity_json_;
    }
    bool adaptiveComputationComplete() const noexcept;
    bool adaptiveStatisticsComplete() const noexcept {
        return adaptiveComputationComplete()&&adaptive_statistics_complete_;
    }

private:
    RunId run_id_;
    std::unique_ptr<IRandomSource> random_;
    IResultWriter& writer_;
    ITextPublisher& manifest_publisher_;
    std::filesystem::path output_root_;
    SourceProvenance provenance_;
    experiments::ExperimentKind kind_;
    RunManifestDraft draft_;
    std::vector<std::string> expected_output_names_;
    std::string adaptive_metric_validity_json_;
    std::size_t adaptive_expected_repetitions_=0;
    std::size_t adaptive_valid_frames_=0;
    std::size_t adaptive_undefined_frames_=0;
    bool adaptive_memory_noted_=false;
    bool adaptive_statistics_complete_=false;
    bool write_started_=false;
    bool manifest_published_=false;
    Status<ExperimentErrorCode> retainAdaptiveEvidence(
        const experiments::AdaptiveRunData& outputs,std::string serialized);
};

}  // namespace egomotion
