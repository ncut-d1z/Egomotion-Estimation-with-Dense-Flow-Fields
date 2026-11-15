#pragma once

#include "egomotion/io/run_directory.hpp"

#include <string_view>

namespace egomotion::io {

// Writes ONLY a previously completed JSON document. It cannot fabricate
// Complete: the application has to validate results and confirmed MAT receipts
// before calling publish(). Never overwrites manifest.json.
class RunManifestPublisher {
public:
    explicit RunManifestPublisher(const RunDirectory& directory):directory_(directory){}
    Status<MatErrorCode> publish(std::string_view json) const;
private:
    const RunDirectory& directory_;
};

}  // namespace egomotion::io
