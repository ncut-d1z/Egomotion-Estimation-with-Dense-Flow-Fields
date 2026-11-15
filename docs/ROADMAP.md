# C++ 实施路线图

> **原则：先完成全部业务 C++ 代码，再统一构建、测试与验收。** R0–R5 **只实施代码/接口**，期间不安排 CMake 配置、编译、CTest、数学/数值测试或实验运行；测试源码与构建执行也集中在最后的 R6。源码提交和静态审阅不等于通过测试。H-10 编程已授权；H-11 原生运行与 MAT 写入权限仍须单独确认。现行证据见 [PROGRESS](PROGRESS.md)。

## 代码实现（R0 → R5，期间禁止构建或测试）

| 批次 | 从用户入口向下贯通的代码 | 实现边界 |
|---|---|---|
| **R0 外部入口** | CLI 参数解析、五实验入口、`inspect-mat`、`analyze`、可选`plot`及 Composition Root | 错误须明确；未接通的后端不得伪造成功，见 [CLI](design/cli.md) |
| **R1 基础接口** | 自拥有领域值、Result/Status/完整错误链、Reader/Writer/RNG 和实验/分析窄接口 | 按 [contracts](design/contracts.md) 隔离依赖，复用既有低阶球谐叶子 |
| **R2 MAT 读取** | 原`mat_data_dict_probe.cpp`改为纯 CLI；元数据、数值读取统一到`src/io/mat_result_reader.cpp`，报告生成与输入扫描分离 | `inspect`只读结构；`readNamed/readFirstCompatible`返回自拥有二维double列主序。保留旧探针参数/格式，见 [MAT LLD](design/mat_io.md) |
| **R3 Bias 业务** | 三种有序球面方向采样、随机数源、近度/球面光流/两种噪声、Franz、Original/Modified KvD、两种 Bias Runner 与内存结果 | 保留原视线次序、`3N`正态抽样、误差/停止逻辑和当前save字段；见 [sampling](design/sampling_noise.md)、[KvD](design/kvd.md)、[experiments](design/experiments.md) |
| **R4 Adaptive 业务** | 轨迹/球内近度、SH公共适配、单步与Session、三类Adaptive Runner | 失败不提交状态，周期刷新及8次聚合分母不变；见 [Adaptive](design/adaptive_mfa.md)、[trajectory](design/trajectory.md) |
| **R5 数据与交付链** | Writer/Manifest/RunId、四类只读Analyzer、可选绘图适配、完整CLI实际装配 | 只能向固定`output/<RunId>/`写新结果；`origin/**`只读；绘图不阻塞数据分析，见 [analysis](design/analysis_plotting.md) |

**代码批次完成的定义**：源文件、头文件和必要的实现边界完整，真实业务函数不再仅是空壳或无条件`BackendUnavailable`；每批代码可独立审阅并正常提交，但**不进行构建、运行或测试，也不宣称代码正确性已验证**。底层复用数学模块，不为每个`.m`建立同名类。详细98项WBS只见[PLAN](PLAN.md)。

## 最后阶段（R6：仅在 R0–R5 全部代码完成之后）

1. 整理最终 CMake target、依赖和单测/集成/Golden 测试源码，按[HLD](ARCHITECTURE.md)检查依赖图、路径、头文件和测试映射；不提前执行。
2. **待H-11明确授权后**统一配置、构建、执行 CTest 和 CLI/Reader/Writer 安全负例，保留版本/命令/退出码/日志/固定SHA；检查原有SH回归。
3. 使用 L2历史MAT、独立L3参考和必要L4统计做数值复现；**不要求MATLAB/Octave/L1**。最终容差与性能、设计冻结、发布分别走已有延期批准，见 [TEST_PLAN](TEST_PLAN.md) 和 [TODO](TODO.md)。

**状态（2026-11-15）**：R0–R2源码已推送；**R3按本路线图限定的内存计算范围完成源码静态补齐**（失败链/观测索引/原距离浮点次序/容量异常/零真值域），并非编译或数值验证通过。**R4已开始编写轨迹、球内近度、Sophus转换、复用SH叶子、Adaptive单步与Session**，三类实验的完整场景及结果编排仍未完成。R5尚未开始；R3/R4新增C++文件的CMake接线、所有编译/CTest/Golden均留到R6。历史`mat_data_dict_probe`单文件g++命令在R2拆分后不再适用；R6统一通过CMake链接Eigen/matio/zlib。禁止把过去固定SHA的3项低阶SH CTest或旧582/582探针误计为本次新代码测试结果。
