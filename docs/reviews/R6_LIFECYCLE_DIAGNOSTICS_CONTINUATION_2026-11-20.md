# R6 发布封存与错误身份接续（2026-11-20）

`[HUMAN-EXECUTE/APPROVE]`。所有者要求继续代码实现，既有“推迟构建、测试”继续有效。本批从 Draft [PR #6](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/pull/6) 的 `c89e7a1270e05685c23328e2a9802a7d7ee9d6f0` 接续，沿用 `dev-plan-P0-P1-mat-dictionary-probe`。**只修改生产源码、必要回归源码与状态文档；没有执行 CMake 配置、编译、编译器语法检查、CTest、测试程序、CLI、MAT I/O 或数值实验。**

前批 Session、Manifest 状态与实验异常修复见 [失败边界接续](R6_FAILURE_BOUNDARY_CONTINUATION_2026-11-20.md)；更早聚合与分析修改见 [代码接续](R6_CODE_CONTINUATION_2026-11-20.md)。这些批次的新代码均不能继承旧固定 SHA 的通过结论。

## 三项静态反例与修复

| 边界 | 基线源码中的问题 | 本批实现 |
|---|---|---|
| RunContext 确认发布后的生命周期 | Bias 在首次写 MAT 前失败并成功发布 Failed/Incomplete Manifest，`write_started_` 仍为 false；再次调用 `writeCurrentResults()` 可以写出 MAT 并改变内存状态。完成计数、五个进度计数与 `recordFailure()` 也可以修改已经确认发布的快照。 | Bias 写入和完成入口增加 `manifest_published_` 门禁，分别返回带 RunId 的 `OutputFailed@Output`、`IncompleteRun@Simulation`。既有 void/noexcept 计数方法及 `recordFailure()` 在封存后不再修改审计状态，不更改接口签名。 |
| MAT 读取失败的请求身份 | `readNamed(path,"value")` 已知请求变量，但无效路径、无法打开、非法请求名及空 MAT 目录的四处失败构造未传 `variable`。现有 MAT LLD 已要求错误包含文件、变量与阶段。 | 将原始请求变量传入这些失败记录；空字符串与含 NUL 的非法请求也保留其准确字节。不更改错误码、阶段、成功读取路径或类型支持集合。 |
| 有限轨迹向量的模长溢出 | `sourceRotationBetween()` 先确认输入全部有限，随后 `sqrt(v.dot(v))` 溢出却返回 `NonFiniteInput`。有限 `v=(DBL_MAX,0,0)` 是直接静态反例；同文件旋转矩阵转换对模长溢出已使用 `NonFiniteResult`。 | 仅将该分支归类为 `NonFiniteResult`。真正含 Inf/NaN 的直接输入继续返回 `NonFiniteInput`；外层运动学保持轨迹点索引与完整原因链。 |

以上依据来自源码可达路径与接口不变量，没有执行故障注入或数值复现。本批不声称默认科学输入曾触发这些边界。

## 封存与诊断的具体边界

- 封存仅在 `manifest_published_ == true`，即 Publisher 确认发布后生效。Complete、Failed 以及方案 B 的 Incomplete 都保持快照；仅指标缺测、但全部求解完成的交付规则不变。
- 如果 Manifest 已链接但持久化未确认，仍返回原失败且保持未确认标志；既有失败审计尝试和 Publisher 非覆盖协议不变。本批没有把“存在文件”改成“发布成功”。
- 封存保证针对 RunContext 自身的计数、证据、写入与发布方法。Composition Root 持有的 Writer、公开借用 Writer/RNG 接口保持既有装配合同，不声称本地标志可以封锁外部直接调用。
- Reader 修复落实已有诊断身份；没有增加新的 MAT 名字规则、支持类型或文件版本。
- 轨迹修改不改变点差、点积、叉积、开方、`acos` 或 Rodrigues 求值顺序，也不改变原 `1e-11` 分支和 epsilon 阈值。DEC-R6-01 的稳定指标角仍与源轨迹转换保持各自既有范围。
- DEC-R6-01～04、固定八轮分母、末轮 ratio、部分有效数据方案 B 与全部延期决定继续有效。

## 文件与待执行回归

| 实现文件 | 已有回归目标与新增源码覆盖 |
|---|---|
| [run_context.hpp](../../include/egomotion/experiments/run_context.hpp)、[run_context.cpp](../../src/experiments/run_context.cpp) | `r05_manifest_gate`：两种 Bias × Complete/Incomplete/Failed 六种纯内存情景，核对迟到写入、完成计数、进度/错误、重复发布的拒绝或无副作用，以及诊断 RunId/阶段和快照不变；源码见 [test_r05_manifest.cpp](../../tests/contracts/test_r05_manifest.cpp)。 |
| 同上 | `r06_partial_adaptive_output`：在既有完整及部分有效发布情景中，核对迟到调用不改变指标有效性、次数、输出身份、回执或 Manifest 内容；源码见 [test_r06_partial_adaptive_output.cpp](../../tests/integration/test_r06_partial_adaptive_output.cpp)。 |
| [mat_result_reader.cpp](../../src/io/mat_result_reader.cpp) | `r06_mat_reader_boundaries`：缺文件、空/含 NUL 路径、非法请求变量、截断/损坏 MAT 与空 MAT 目录的来源身份断言；源码见 [test_r06_reader_negative.cpp](../../tests/integration/test_r06_reader_negative.cpp)。 |
| [kinematics.cpp](../../src/trajectory/kinematics.cpp) | `r06_trajectory_sphere_geometry`：两个参数位置的有限大向量溢出、真正 Inf 输入，以及 `fromPoints` 的 Simulation 阶段、0-based 点索引 1 和内层原因；源码见 [test_r06_trajectory_sphere.cpp](../../tests/integration/test_r06_trajectory_sphere.cpp)。 |

四个测试目标原已登记，本批无需修改 CMake；前两项已在既有有限子集脚本中。**这里只准备回归源码，没有执行这些断言；登记与场景数量不表示测试通过。**

## 静态核对与后续范围

核对了实际工作树、现行 AGENTS/PLAN/ROADMAP、PR 与全部状态 Issue；仍沿用 #1～#5，没有创建/关闭 Issue 或发送评论。三项实现先在独立工作树提交，再逐项审阅并整合；`git diff --check` 仅用于文本差异检查。原始 `origin/**` 树保持 `70ee155131c24a50169346d0ffbaff0c01b1c547`，没有依赖安装、历史改写、强推、转 Ready、合并或发布。

Reader 静态检查另发现 raw 名字含 NUL 时私有载荷选名与 C 字符串接口可能存在差异，但尚未确认 matio 后端是否接受该输入。此项仅为待核实观察，没有按已证实缺陷实施新规则，也无需当前人类决定；待后续取得后端源码或恢复验证时再确认。其它已查看的流场成功载荷、噪声方向保持、RNG 错误链和 Adaptive 证据保留路径，本批未确认需要额外修改的具体缺口。

当前结论为：**本批三项具体源码问题已修复并提交，新代码未构建/未测试；R6 总体验收未完成。** 没有需要所有者立即介入的新事项。恢复验证后应针对新的固定 SHA 执行以上回归与相关工程验证；科学复现、最终资格、冻结及发布继续按所有者决定延期。
