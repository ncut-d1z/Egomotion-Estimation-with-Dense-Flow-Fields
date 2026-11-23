#include "egomotion/io/manifest_publisher.hpp"
#include "egomotion/io/run_text_publisher.hpp"

namespace egomotion::io {

Status<MatErrorCode> RunManifestPublisher::publish(std::string_view json) const {
    RunTextPublisher writer(directory_);
    return writer.publishNew("manifest.json",json);
}

}  // namespace egomotion::io
