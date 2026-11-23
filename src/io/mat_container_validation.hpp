#pragma once

#include "egomotion/contracts/result.hpp"

#include <filesystem>
#include <string_view>

namespace egomotion::io::detail {
// 业务完整读取的前置校验。fd 与随后 matio 打开的 /proc/self/fd 指向同一 inode。
// 校验顶层/内部边界、完整zlib及数值载荷长度，业务class/rank等合同仍由Reader检查。
Status<MatErrorCode> validateMat5Container(int fd,const std::filesystem::path& path,
                                        std::string_view variable);
}
