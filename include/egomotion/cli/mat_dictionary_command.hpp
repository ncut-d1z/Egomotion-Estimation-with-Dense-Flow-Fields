#pragma once

#include <iosfwd>

namespace egomotion::cli {

// 保留历史探针退出码：0 全部读取且报告/状态交付成功，1 数量或读取失败，
// 2 参数、报告或状态交付失败。argv包含程序名；不改变默认SIGPIPE策略。
int runMatDictionaryCommand(int argc, const char* const* argv,
                            std::ostream& diagnostic) noexcept;

}  // namespace egomotion::cli
