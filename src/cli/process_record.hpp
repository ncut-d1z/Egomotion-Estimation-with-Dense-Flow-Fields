#pragma once

#include <filesystem>
#include <iosfwd>

namespace egomotion::cli {
class IInvocationObserver;
using RecordedApplication = int (*)(int, char**, IInvocationObserver*);

// 仅用于独立、单线程的进程入口；回调在fork子进程中运行同一构建的应用。
// receipt属于记录器，子进程的stdout/stderr仅写各自原始字节日志。
int recordProcess(int argc, char** argv, RecordedApplication application,
                  const std::filesystem::path& trusted_root,
                  std::ostream& receipt, std::ostream& errors);

}  // namespace egomotion::cli
