# R6 Linux 发布故障验证（2026-11-17）

**[HUMAN-EXECUTE/APPROVE] 已实际通过：23/23 CTest。** 源码 SHA 为 `1e15b9c32342c96e2e553325a8870c0a276b31a4`，接续 `8292d9d26b57ffcd2f9b37ca4b57c51c3733ab57`，分支 `dev-plan-P0-P1-mat-dictionary-probe`；运行前核实远端 SHA，Issue/PR 全状态查询为空。上一批次21项与本批新增2项均已重新执行。

## 实现范围与判据

关联 PLAN P1.7/P1.9、TC-F-MAT-004、TC-NF-SAFETY-001、TC-I-RESULT-001、TC-DC-EXPERIMENT-001。本批落实 [前次候选故障场景](R6_READINESS_eeb30135.md#63-真正io故障场景待源码方案批准和独立执行授权) 的 Linux 发布路径部分，不新增或放宽验收义务、数值阈值、公共接口或 CLI 参数。

- `tests/support/io_fault_injection.{hpp,cpp}` 用 GNU ld [`--wrap`](https://sourceware.org/binutils/docs/ld/Options.html#index-_002d_002dwrap_003dsymbol) 包装测试可执行程序的 `openat/write/fsync/linkat/unlinkat/fstatat` 未解析引用。只对本例 RunDirectory 的目录 fd、独占暂存 fd、名字与调用阶段触发；不安装注入工具，不改变生产库/CLI 行为，不包装动态 libmatio 内部调用。
- `tests/integration/test_r06_io_faults.cpp` 调用真实 RunDirectory、MAT/Text Writer、Reader、SHA-256 和 RunContext。每例独立 OS 随机 RunId，写入 build 内含中文/空格的合成沙箱；真实 `origin/**` 不写入。打印场景、RunId，失败返回1，保留生成文件与日志。
- 新增 `r06_linux_writer_faults`（23场景）及 `r06_real_manifest_faults`（6场景），CTest总数由21变23；两进程使用不同沙箱。无静态全局生产故障开关。

| 场景 | 真实观察要求 |
|---|---|
| 正常 Text/MAT | 文件与两次目录 fsync、一次暂存 unlink 均完成；成功；MAT receipt 确认 |
| 暂存 open、文件 fsync、link 失败 | 无本次最终名、无 MAT receipt；可用的暂存清理完成；明确失败 |
| Text write EIO/0 | 无最终名，明确 WriteFailed；暂存清理完成 |
| Text write EINTR/短写 | 命中一次注入后继续完整写出；最终字节完全一致 |
| link EEXIST 竞争 | 在真正 link 前创建竞争文件；inode/字节保留；返回 OutputExists，MAT 待发布 receipt 撤回 |
| MAT 发布后身份检查失败 | 最终 MAT 保留且可逐位读回；receipt 未确认；ReadbackMismatch |
| 第一次/第二次目录 fsync 失败 | 保留最终文件；前者由守卫清理暂存，后者暂存已移除；均非成功 |
| 显式 unlink 失败、析构重试成功 | 精确两次 unlink；最终文件保留、暂存消失，但仍非成功 |
| unlink 持续失败 | 精确两次失败；保留最终名及同 inode 的暂存硬链接，link count=2；receipt 未确认 |
| 真实 RunContext 正常/发布前失败/发布后失败 | 只有确认成功可返回 Complete；发布前失败恢复后可写 Incomplete；发布后失败导致失败记录独立 OutputExists，原 Complete 字节保留且内存不提交 Complete |

每例精确核对故障命中次数、文件/目录同步及清理次数，防止测试未实际拦截调用却虚假通过。已发布 MAT 的维度、列主序逐位值（含负零）、SHA-256/文件字节数必须与其真实 receipt 相符。Manifest 使用合成35帧数组和测试来源标记，不是完整科学实验结果。

## 证据边界与剩余工作

这是受控系统调用返回值注入，不能模拟断电、真实磁盘介质损坏或保证崩溃后的持久状态；不是“所有 Linux I/O 故障已覆盖”。matio 内部关闭/解码失败、发布路径竞态的全部时序、原始 MAT packed tag/损坏负例、独立 L2/L3/L4 和完整 Runner 验证仍待推进。

单个磁盘 Manifest 在目录持久化未确认时仍可能含 Complete 文本。本批验证运行中的返回值、内存状态和错误链，不替人类选择离线消费者的最终资格/恢复协议，也不覆盖或删除该文件伪造恢复成功。现有 DEC-008 设计问题保持开放。

## 执行记录

RunId：`r6-io-faults-20261117-sm6Ovszn`，工作目录为仓库根。证据位于 `build/r6-io-faults-20261117-sm6Ovszn/evidence/`，含逐步 `.meta/.stdout/.stderr`、JUnit、Testing 快照、工具链、随机源及输入 SHA-256。复用 `build/r6-20261117-KzjFHPDQ/cmake/` 缓存并重新配置 SHA，**不是新的干净构建**。

环境仍为 Ubuntu 24.04.5/GCC16.2/CMake4.3.4/Eigen3.4/Sophus1.24.6/matio1.5.26/zlib1.3，C++17 Release；无依赖安装。配置参数 `-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`；构建 `cmake --build <build-dir> --parallel 4 -- -k`。CTest 使用 `-C Release --verbose --no-tests=error --output-junit <evidence>.xml`，先 `--parallel 2 -R '^(r02_mat_reader|r06_historical_mat_readonly_schema)$'`，再 `--parallel 4 -E` 同一表达式，覆盖全部23个不同名称。

| 步骤 | 开始/结束（UTC+08:00） | 退出码 | 结果 |
|---|---|---:|---|
| configure | 15:39:51 / 15:39:52 | 0 | 新SHA正确 |
| build | 15:39:52 / 15:40:19 | 0 | 全目标成功，无编译诊断 |
| MAT CTest | 15:40:19 / 15:40:19 | 0 | 2/2 Passed，0.48秒 |
| 其余 CTest | 15:40:19 / 15:40:20 | 0 | 21/21 Passed，0.85秒 |
| 输入 SHA-256 | 15:40:20 完成 | 0 | 运行前后全部一致 |

新增测试实际输出 `Writer fault scenarios=23`、`Manifest fault scenarios=6`；每例 RunId、精确注入计数和断言已执行，无跳过。持续 unlink 失败的暂存文件保留在沙箱，属于预期审计证据，不自动删除。全部步骤 stderr 为空。`ctest-remaining.stdout` SHA-256：`379a9bf0fc8bb8ceee8f15b64d951eab126c657831fa59bded9dc00ba99f38e2`。

没有执行 MATLAB/Octave、完整科学实验或合并发布，也没有修改任何科学算法/数值门限。生产 I/O 此批不需要修正；新增证据支持现有错误/回执行为，但上述剩余边界仍未完成。
