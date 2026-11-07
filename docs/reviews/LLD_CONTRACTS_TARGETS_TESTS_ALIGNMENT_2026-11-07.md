# LLD 跨模块合同、target 和测试映射修订记录（2026-11-07）

> **文档审查证据，不是编译/测试或人类冻结。** 本轮依据最新 5 份 LLD 的评审意见，修订 `contracts.md`、`experiments.md`、`harmonics.md`、`mat_io.md`、`sampling_noise.md`，并同步 `cli.md`、`OBJECT_MODEL.md`、`design/README.md`、`TEST_PLAN.md`、`PLAN.md`。原始 `origin/**`、生产 C++ 和 MAT 文件未编辑；没有运行 MATLAB/Octave、Bash、CMake、CTest、生产程序或数值/性能测试。

- 仓库及分支：`ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields` / `dev-plan-P0-P1-mat-dictionary-probe`。
- 修订前的固定基准：`354aea3f32e867bc14f72b5d81c82d4e7e227a74`。
- 当前这一轮**只读静态验证所对应的固定提交**：`2689ac522aaa480d9e66ed993ae1ffc94254065f`（本证据文档及进度引用自身的后续提交不属于该检查点）。
- 长期约束：[无需 MATLAB 运行政策](../golden/NO_MATLAB_EXECUTION_POLICY.md)；参考数据只按实际来源分别记为 L2 原始 MAT、L3 独立固定输入/参考、L4 统计；L1 永久不要求，不恢复成为任何阻塞条件。

## 一、已实施的文档变更（全部仍为候选）

| 文档 | 本轮变更 | 不能据此认定 |
|---|---|---|
| [contracts.md](../design/contracts.md) | 提出 **同一份** C++17 Failure/Result/Status 的 tagged `std::variant` 候选；ErrorDetail 的 stage/RunId/index 等上下文；DataMismatch 的限定触发范围；MatDenseArray **归 io 值对象**，不在 contracts 内重复定义 | 头文件已实现/编译、ABI 已选定、错误枚举和 CLI exit 已获冻结 |
| [experiments.md](../design/experiments.md) | RunContext 的最小候选构造/访问、unique RNG 与 IResultWriter 注入、RunId/来源/输出生命周期；EX-G01～05 按 L2/L3/L4 标来源；Runner 错误范围、`egomotion_experiments` target 与 Bias/Adaptive/Result/Log/Safety 测试映射 | 已创建 Runner、Writer/CTest、明确部分结果提交语义、原 MATLAB 每次随机轨迹已复现 |
| [harmonics.md](../design/harmonics.md) | 明确 HarmonicCoefficients 归 domain、HarmonicRotationBlocks 归 harmonics 的自拥有/复制/移动；L3 的九基/R3(9)/R5(25)/R0；`egomotion_harmonics` target、公私有边界和 `test_harmonics.cpp` | 已引入/编译 Don Williamson SHRotation、适配 sdla、确定完整失败合同、独立数值参考已验收 |
| [sampling_noise.md](../design/sampling_noise.md) | SN-G01～07 明示独立 L3；IdealFlowResult/FlowSynthesisResult 的自拥有与 RNG 不借用；`egomotion_sampling` / `egomotion_simulation` 分层与 `test_sampling.cpp`/`test_flow_simulation.cpp` | 独立 i=1/2 全量有序数值已生成、C++/CTest 已测试、RNG 的 MATLAB 同 seed 流已复现 |
| [mat_io.md](../design/mat_io.md) | §4 标题与 §2.2 统一 L2/L3 及具体 MatDenseArray 归 io；**只有 MAT I/O 适配层**可构建内部 MatDenseArray，Runner 不应直接依赖；其已有 v0.5 双精度自拥有模型、§7.4 多变量、§8 负例、§9 target 保留 | 已创建生产 MAT Reader/Writer、L2 逐值/SHA-256 或写回读安全负例 |
| [cli.md](../design/cli.md) | compat 单变量/多变量的 `AmbiguousVariable` 合同同步 mat_io.md §7.4，且 §3 错误返回与 contracts.md 单一来源保持一致 | 完整 MATLAB fieldnames 顺序被证明或 CLI/诊断实现已经冻结 |
| [OBJECT_MODEL.md](../OBJECT_MODEL.md) | §8 批准证据改为**只读 .m Git blob SHA + L2 原 MAT / L3 独立参考身份** | 已有人类签署或 LLD API 已冻结 |
| [TEST_PLAN.md](../TEST_PLAN.md) | 在原 66 个 TC 内将 TC-DC-RNG-001 / TC-DC-NOISE-001 补充为候选单元测试加人工评审，其他 TC/AC ID 不变；分析曲线参考明确 L2/L3 | 实际 `tests/`、CTest 注册已存在/运行 |
| [PLAN.md](../PLAN.md)、[design/README.md](../design/README.md) | 对 WBS “证据类别”与 TEST_PLAN 单一测试文件映射的关系予以说明；更新 5 份模块 LLD 的候选设计与未决项 | P0～P10 任务状态、里程碑、算法通过计数改变 |

## 二、本轮 GitHub 连接器只读静态验证

在固定 SHA `2689ac522aaa480d9e66ed993ae1ffc94254065f` 上，通过 GitHub 连接器读取已提交文档，由 JavaScript 检查：

- SRS **66 REQ** 的编号/AC 对应、TEST_PLAN **66 TC** 与 REQ/AC 对应、PLAN **98 P** 的前置依赖与关联 REQ/TC/AC；范围内无重复/未知引用/依赖环。
- HLD §4.1 的 **26 条现有依赖边**静态无环；**没有新建或测试 CMake target**，如正式接口引入新库依赖，必须同步重审 HLD target/DAG。
- scoped contract checks：Failure/Result/Status 只有一套候选，MatDenseArray 归 io，Runner DataMismatch 与 RunContext，球谐/采样 target 与拟议测试实体，以及 TEST_PLAN 的 TC-DC-RNG/NOISE 候选行；共 9 个限定检查均通过。
- **不等同**在 Debian/Ubuntu 工作树现场运行 `bash scripts/check_docs_traceability.sh`；没有程序输出、数值样本、CTest exit code 或 MATLAB 运行值。

本轮 GitHub 差异检查点起点 `354aea3f...`，所有本轮更改均应限定在 `docs/`，不触及 `origin/**`。真实原始 MAT 文件的全量 SHA-256、L2 逐值、L3 独立多场景样本/统计仍没有本轮实测证据。

## 三、尚需人类决定与未解风险（不自动默认）

| 领域 | 需决定的候选 | 关联门禁 |
|---|---|---|
| Result/Status 与诊断 | `std::variant` alias 还是包装类/tagged union，Failure+ErrorDetail 的公开 ABI；stage 保留稳定 string 还是 enum；嵌套底层 cause/JSONL 转义 | P0.8、DEC-008、contracts/cli |
| RunContext/实验错误 | 输出目录何时独占创建、RunContext 能否移动、配置类型用 variant 还是分入口类型、partial/重试/回滚/失败样本继续策略；DataMismatch 判断与统计分母 | P0.8、DEC-008、实验 LLD |
| 球谐与组件 DAG | Result/Status 的公共头实际由 domain 还是 contracts target 提供；若增加 `egomotion_harmonics→egomotion_contracts` 依赖需同改 HLD DAG；上游 Don Williamson/sdla 复用范围与许可 | P0.8、DEC-003/007、HLD §4 |
| MAT I/O 及 CLI | MatDenseArray double-only 的正式 API、int 原 class/type 的稳定表示、零长处理、compat 多变量顺序未知时拒绝的最终合同、RunId/输出根与无覆盖实现 | P0.8、DEC-008、MAT 及 CLI LLD |
| 数值验收与真实执行 | `atol=0.01, rtol=0.1` 的分模块批准与严格几何/状态例外；L2 SHA-256、L3 独立参考和未来 C++/CTest/错误负例的明确执行许可 | DEC-005、H-10/H-11（MATLAB/Octave **永不列为必需**） |

**结论**：本轮只增加设计可评审程度和跨模块单一来源的明确性；所有新接口、文件、CMake 和数值参考目前**仍是未批准候选或未完成工件**。没有将设计文档更新计为软件任务、TC/AC 或 M0～M5 已通过。
