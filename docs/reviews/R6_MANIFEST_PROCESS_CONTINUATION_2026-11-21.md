# R6 完成状态与子进程回收接续（2026-11-21）

`[HUMAN-EXECUTE/APPROVE]`。按所有者“继续代码实现，推迟构建、测试”，从 Draft [PR #6](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/pull/6) 的 `5060783e06214bb697d556de81f64789750736df` 继续。前批见 [启动异常与 MAT 扫描记录](R6_STARTUP_MAT_SCAN_CONTINUATION_2026-11-21.md)。**本批仅生产源码、必要回归源码和状态文档；未配置、未编译、未做语法编译、未执行测试、CLI、MAT I/O 或数值实验。**

## 1. Bias 的最终 Complete 门禁必须要求有效样本

[RunContext::noteCompletedMemory](../../src/experiments/run_context.cpp) 已明确拒绝 `valid_samples == 0`，正常 `finishBias()` 按三种噪声系数与实际 subdivision level 数登记正有效计数（`3 * level_count`）。但公开 `noteAttempt()`、`noteCompleted()` 可令重复次数满足完成条件而 `valid` 仍为零；准备全套合法 MAT 输出及确认回执后，旧 [makeCompleteManifestJson](../../src/experiments/manifest_json.cpp) 未复核此条件，可以序列化并发布 `Complete`。这是静态可达的接口组合，本轮没有执行该反例。

本批只在 v1 最终门禁补上 `draft.valid == 0` 拒绝，统一已有完成条件。失败保留既有 `OutputFailed @ Output` 和 RunId；检查早于 Publisher 调用及生命周期封存，不修改计数、错误记录或 MAT 回执。

[test_r05_manifest.cpp](../../tests/contracts/test_r05_manifest.cpp) 在既有 `r05_manifest_gate` 中增加两种 Bias 的生命周期回归源码：先验证 `noteCompletedMemory(1,0)` 失败，再以公开进度入口和完整输出回执构造绕过路径，要求最终发布拒绝且状态快照/Writer 调用/Publisher 输出不变；补记三个有效样本后要求同一批回执可正常完成，无须重写 MAT。**两种是源码场景数量，未运行或通过。**

`publishCompleteOutcome()` 仍拒绝 Adaptive 类型，后者经独立的 v2 `makeAdaptiveManifestJson()` 处理；本批没有把 Bias 的非零条件加到方案 B。计算完成但指标缺测的 Incomplete、固定八轮分母、末轮 ratio、原值/原因/有效次数均保持。

## 2. 记录器必须保留对子进程 wait 状态的观察权

[process_record.hpp](../../src/cli/process_record.hpp) 限定为独立、单线程进程入口；[process_record.cpp](../../src/cli/process_record.cpp) 通过 `waitpid` 记录实际正常退出或信号终止，并在尚未回收时按 PID 转发信号、在父侧失败时清理本次子进程。

Linux [wait(2) 的 NOTES](https://man7.org/linux/man-pages/man2/wait.2.html) 明确区分默认 SIGCHLD 与显式 `SIG_IGN`：显式忽略或 `SA_NOCLDWAIT` 会自动回收已终止子进程，使后续等待不能取得退出状态。另有继承的自定义 SIGCHLD handler 可能先执行 `waitpid`。由此静态推导：原代码可能失去实际终止状态，且“尚未由记录器 wait 的 PID 不会复用”这一转发/清理前提不再成立。**本轮没有运行信号或 PID 复用实验，不声称观测到实际误发信号。**

本批在既有 Signals 保护范围中补齐：

- 创建 Directory、发布开始记录及 fork 之前检查 SIGCHLD；显式 `SIG_IGN` 或 `SA_NOCLDWAIT` 沿用 `OutputFailed / Output / exit 6`，不启动应用、不生成成功收据或本次记录目录。阻塞信号不能阻止这两种内核自动回收模式，因此单独检查。
- 父进程阻塞集增加 SIGCHLD，保留原 disposition，防止自定义 handler 先取走退出状态；signalfd 及转发集合仍只有 INT/TERM/HUP/QUIT。
- 应用子进程恢复调用前 mask，父侧正常或异常返回同样恢复；SIGPIPE 对故障诊断的已有保护持续有效。Linux [signal(7)](https://man7.org/linux/man-pages/man7/signal.7.html) 的 mask/pending 语义支持这一限定实现；没有引入多线程嵌入支持或 pidfd 依赖。

[test_r06_process_records.cpp](../../tests/integration/test_r06_process_records.cpp) 复用 `r06_process_records_lifecycle`，新增 **6 种调用场景的源码**：

| 场景 | 源码断言 |
|---|---|
| 显式 SIG_IGN、SA_NOCLDWAIT（2 种） | 返回6及原错误域，未 fork、未进入应用、未创建记录目录；原 action/flags/mask 保持。 |
| 自定义回收 handler，原 SIGCHLD mask 阻塞/未阻塞 × 正常退出137/TERM转发（4 种） | fork 前父侧已阻塞 SIGCHLD，handler 未回收本次子进程；应用看见原 action/mask，finish 区分正常退出与信号终止，转发记录准确。 |

fork 前阻塞状态通过现有 `__wrap_fork` 直接观察，不依赖调度或睡眠碰到竞争窗口；公共 `invoke()` 同时对既有失败场景增加父侧 SIGCHLD 恢复断言。**6 是准备的调用数量，不是实际执行或通过数量。** 没有修改 CMake 或新增测试目标。

## 3. 限定静态复核与交付状态

两项生产修改及其回归源码均经独立源码复核。另核对 Bias 连续目录/末轮 x 值、Adaptive 严格缺测与八轮聚合、JSON/SVG 有效性及分段、插图与轨迹的本轮已读路径，未确认其它具体实现缺口；没有因为继续实现的要求而改动已有数值公式、阈值或支持范围。此结论只覆盖实际读过的路径，不是全工程正确性证明。

实际检查限于源码交叉审阅、Git 状态/差异与 `git diff --check`。原始 `origin/**` 树保持 `70ee155131c24a50169346d0ffbaff0c01b1c547`；历史分支、工作树与失败证据保留，没有运行仓库验证脚本或安装依赖。

DEC-R6-01～04 及其它延期项保持，**没有新增需要所有者立即决定或干预的事项**。后续构建/测试、完整科学实验、独立 L3 的更广覆盖与必要 L4、数值/性能及跨环境资格、正式冻结/验收和发布继续延期；旧固定 SHA 的通过记录不外推到本批。当前已列明功能已有源码实现，仍须修复确认的具体错误并在恢复验证授权后取得证据；PR #6 保持 Draft，R6 总体验收仍未完成。
