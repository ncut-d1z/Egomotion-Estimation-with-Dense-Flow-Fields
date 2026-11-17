#pragma once

#include "egomotion/contracts/result.hpp"

#include <filesystem>
#include <string_view>

namespace egomotion::io::detail {
// 业务完整读取的前置校验。fd 与随后 matio 打开的 /proc/self/fd 指向同一 inode。
// 只校验 MAT v5 顶层边界和完整 zlib 流，不替代变量 class/type/shape 检查。
Status<MatErrorCode> validateMat5Container(int fd,const std::filesystem::path& path,
                                        std::string_view variable);
}
