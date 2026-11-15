#pragma once

#include "egomotion/experiments/output_contract.hpp"

#include <cstddef>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace egomotion {

struct SourceProvenance {
    std::string repository_commit_sha;
    std::string matlab_source_blob_sha; // source identity, not MATLAB execution
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
               IResultWriter& writer, std::filesystem::path validated_output_root,
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

    // Call only for a COMPLETED in-memory experiment. Validates every
    // intended source-save field before invoking Writer even once.
    // On partial Writer failure, the committed file names remain recorded,
    // and the single-run Result returns failure (never fake Complete).
    Status<ExperimentErrorCode> writeCurrentResults(
        const std::map<std::string,DenseDoubleArray>& outputs);

    // Counts and failure events can be observed by the future Manifest
    // publisher. They are never themselves durable evidence of a file.
    void noteAttempt() noexcept { ++draft_.attempted; }
    void noteCompleted() noexcept { ++draft_.completed; }
    void noteValid() noexcept { ++draft_.valid; }
    void noteFailed() noexcept { ++draft_.failed; }
    void noteUndefined() noexcept { ++draft_.undefined; }
    void recordFailure(Failure<ExperimentErrorCode> failure);
    const RunManifestDraft& manifestDraft() const noexcept { return draft_; }

private:
    RunId run_id_;
    std::unique_ptr<IRandomSource> random_;
    IResultWriter& writer_;
    std::filesystem::path output_root_;
    SourceProvenance provenance_;
    experiments::ExperimentKind kind_;
    RunManifestDraft draft_;
};

}  // namespace egomotion
