#pragma once

#include "egomotion/io/run_directory.hpp"

#include <map>
#include <string>

namespace egomotion::io {

struct MatFileReceipt {
    std::string variable;
    std::string filename;
    std::string sha256; // SHA-256 of the successfully published MAT inode
    std::size_t bytes=0;
    bool durably_confirmed=false; // false after link but before directory fsync
};

// Lifetime: the RunDirectory must outlive this Writer and its RunContext.
// Only the controlled run-directory FD is used to create and publish files.
class RunMatWriter final : public IResultWriter {
public:
    explicit RunMatWriter(const RunDirectory& directory):directory_(directory) {}
    Status<MatErrorCode> saveNewVariable(
        const RunId& run_id,std::string_view name,
        const DenseDoubleArray& value) override;
    const std::map<std::string,MatFileReceipt>& receipts() const noexcept {
        return receipts_;
    }
private:
    const RunDirectory& directory_;
    std::map<std::string,MatFileReceipt> receipts_;
};

}  // namespace egomotion::io
