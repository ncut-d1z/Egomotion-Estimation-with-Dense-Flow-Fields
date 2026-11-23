# R6 失败边界接续：Session、Manifest 与实验异常（2026-11-20）

`[HUMAN-EXECUTE/APPROVE]`。所有者继续要求代码实现；上一条“推迟构建、测试”仍有效。本批从 Draft [PR #6](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/pull/6) 的 `9c91bbc408a315b2386876c2aa2dca958637d05d` 接续，保持 `dev-plan-P0-P1-mat-dictionary-probe`。**只修改生产源码、必要回归源码和状态文档；未执行配置、编译、编译器语法检查、CTest、测试程序、CLI、MAT I/O 或数值实验。**

前批聚合、平滑、SVG 和指标来源修复见 [代码接续记录](R6_CODE_CONTINUATION_2026-11-20.md)。本批同样没有运行结果；更早固定 `594a1b11...` / `2ed5c2d...` 的通过记录不能外推到新源码。

## 三项静态反例与实际修复

| 边界 | 起点源码中的确定缺口 | 本批实现 |
|---|---|---|
| Session 接受成功 Step | 可注入求解器返回成功 Result，motion/coefficients 有限，但 `error_mse=Inf` 或 trace 含 NaN/Inf；Session 原来只核对成功标志，因此比较路径可继续计算指标，主路径可提交系数并推进帧号。现行 Adaptive LLD 已要求必需计算节点有限。 | 比较/主求解成功后、指标计算或状态提交前检查 MSE 和已提供 trace 数值，发现非有限值即按 `NonFiniteResult@Solve` 包装为 `SolverFailed@Simulation`；保留字段名、比较槽/主路径和原 MATLAB 帧号。 |
| 发布后未确认产物的失败状态 | Writer 首次最终链接成功但后续确认失败，保留 `durably_confirmed=false` 回执；在尚未登记内存完成计数时，`publishFailureRecord` 原按确认文件列表产生 `Failed`，JSON 却按实际回执产生 `Incomplete`。公开 RunContext 允许先写结果再登记完成，因此该顺序可达。 | 读取一份实际 `FileEvidence`，同时用于内存状态和 Manifest 序列化。有未确认产物即 `Incomplete`，回执仍未确认；发布前失败且没有部分进度仍为 `Failed`。不将未确认文件当作成功。 |
| 实验标准异常漏记 RunId/结束审计 | `invocation.json` 已确认后，Runner 的 `std::exception` 原会越过失败 Manifest 和 `execution.json`；`cli::run` 的全局回退虽返回 6，却丢失已知 RunId，直接 `dispatch` 则外抛。 | 仅在 `invokeExperiment` 周围捕获标准异常，附实际 RunId 和原 `what()`，沿既有失败记录与通道交付流程处理；保留原 `OutputFailed@Output` / 退出码 6，不猜测数值错误类型或虚构原因链。 |

这些是接口与状态边界的静态反例，不表示默认生产 Step 已实测产生非有限成功载荷，也不表示本批重现了磁盘故障或运行时异常。

## 实现边界

- Session 只增加有限性检查，不要求替身补齐空 trace，不增加 shape、一致性重算、正值或数值容差门槛；MotionEstimate/HarmonicCoefficients 继续使用既有工厂不变量。纯指标缺测仍可以继续，坏求解载荷属于真实失败，不能提交状态。
- RunContext 在取得回执前仍保留当时已知的失败状态，获取回执失败时继续返回明确失败，不声称完成审计。失败审计保持禁止覆盖和禁止转 Complete 的规则。
- CLI 保持正常 Result 失败的原错误链和 5/6 分类；标准异常沿用原回退 6。已有 Complete Manifest、已有结束记录不能被此路径覆盖。局部捕获不承诺持续内存耗尽、信号终止或再次记录失败时仍能完整持久化。
- DEC-R6-01～04、固定八轮分母、末轮 ratio、原科学采样、求解公式及全部延期决定保持。不新增审批事项，不恢复构建或测试。

## 文件及待执行回归

| 实现 | 回归源码 | 已有登记与本批待验证内容 |
|---|---|---|
| [adaptive_session.cpp](../../src/estimators/adaptive_session.cpp)、[adaptive_step.hpp](../../include/egomotion/estimators/adaptive_step.hpp) | [test_r06_session_metric_validity.cpp](../../tests/contracts/test_r06_session_metric_validity.cpp) | `r06_session_metric_validity`：三模式、比较/主路径、四类非有限载荷的 24 个新增情景，检查停止位置、状态字节保持、时钟和同帧重试；只写源码。 |
| [run_context.cpp](../../src/experiments/run_context.cpp) | [test_r05_manifest.cpp](../../tests/contracts/test_r05_manifest.cpp) | `r05_manifest_gate`：两 Bias kind × 首次发布前/后失败，检查 Failed/Incomplete 一致、未确认回执、RunId/错误链和禁止转 Complete；使用纯内存替身。 |
| [application.cpp](../../src/cli/application.cpp) | [test_r06_execution_records.cpp](../../tests/integration/test_r06_execution_records.cpp) | `r06_execution_record_contract`：CLI/直接分派 × 无进度/已有有效及缺测计数，检查异常不外抛、真实 RunId、转义消息、失败 Manifest 和完整结束记录。 |

三个测试名称均已登记，前两项也已列入有限子集脚本，本批无需修改 CMake 或脚本。**登记和断言数量不表示已经执行或通过。**

## 本批静态核对与剩余工作

已核对实际工作树、现行 AGENTS/PLAN/ROADMAP、PR 身份及全部状态 Issue；仍是既有 #1～#5，没有创建/关闭 Issue 或发送评论。三个逻辑修改在隔离工作树中处理，汇总时逐项审阅补丁和既有调用/序列化约定，`git diff --check` 仅作文本检查。Analyzer 的 marker/count、缺测传播与 JSON 路径本批静态检查未确认新问题，未为扩大改动而新增限制。

原始 `origin/**` 树保持 `70ee155131c24a50169346d0ffbaff0c01b1c547`；没有安装依赖、改写历史、强推、转 Ready、合并或发布。状态为 **三项具体源码缺口已修复并提交，构建/测试延期，R6 总体验收未完成**。目前没有需要所有者立即干预的新事项；恢复验证后应针对新 SHA 执行上述回归及相关全工程检查，不能继承旧受测 SHA 的结论。
