# R6 Linux 发布故障验证（2026-11-17）

**[HUMAN-EXECUTE/APPROVE] 已获代码修改、构建/CTest 授权；本检查点尚未运行。** 接续 `8292d9d26b57ffcd2f9b37ca4b57c51c3733ab57`，分支 `dev-plan-P0-P1-mat-dictionary-probe`，当前本地/远端已核实一致、Issue/PR 全状态查询为空。上一批次 `84c8662` 的21项通过属于已执行证据，本批新增不能自动继承 PASS。

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

提交/正常推送/远端 SHA 核实后执行全工程构建及23项CTest。命令、环境、输入哈希、stdout/stderr、退出码和结果待真实执行后补充；不运行 MATLAB/Octave、完整科学实验、依赖安装或自动合并发布。
