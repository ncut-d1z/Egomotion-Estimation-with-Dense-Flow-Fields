#pragma once

#include "egomotion/contracts/result.hpp"

#include <string>

namespace egomotion::io {

// SHA-256 of the exact open inode: never hash a filename that can be
// exchanged between validation and publication. Implements FIPS-180-4
// SHA-256 without introducing an external crypto/runtime dependency.
Result<std::string,MatErrorCode> sha256Fd(int fd);

}  // namespace egomotion::io
