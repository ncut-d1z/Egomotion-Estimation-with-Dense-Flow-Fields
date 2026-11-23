#pragma once

#include "egomotion/contracts/result.hpp"

#include <sys/types.h>
#include <cstdint>
#include <filesystem>
#include <string_view>

namespace egomotion::io::detail {
Status<MatErrorCode> validateMat5Payload(
    int fd,off_t offset,std::uint64_t bytes,bool compressed,bool little_endian,
    const std::filesystem::path& path,std::string_view variable);
}  // namespace egomotion::io::detail
