# C++ 实施路线图

> **2026-11-17 最新执行授权**：用户已明确允许生产代码修改、构建/CTest 和方案文档修改。现进入 R6 原生验证及失败修复，旧批次的运行暂缓只作历史记录；每次运行前仍执行远程检查点规则。实际命令/结果见 [PROGRESS](PROGRESS.md)，本次授权不等于数值验收或冻结。

> **最新结果**：`c42f12f0be89646fd39b93dbb61a79a0ee28e05f` 增量构建无诊断、35/35 CTest通过；首次干净构建为 `84c86624954406b31e8a82d5e145c7f6ae30ee93`。新增57次真实CLI调用，修复子命令帮助；六组历史输入中3组分析/绘图成功，3组原始NaN按合同拒绝，见 [CLI验证](reviews/R6_CLI_PROCESS_VALIDATION_2026-11-17.md)。原始MAT/源SH/有界Bias/Adaptive合同及安全回归仍通过。下一项为有限合成MAT下Comparison/UpdateRates的真实CLI成功路径及非线性独立平滑参考；原始NaN限制、剩余L3/L4、完整科学实验、资源资格及正式验收待完成。后文旧未编译/0项通过均属历史SHA，R6未结案。

> **原则：先完成全部业务 C++ 代码，再统一构建、测试与验收。** R0–R5 **只实施代码/接口**，期间不安排 CMake 配置、编译、CTest、数学/数值测试或实验运行；测试源码与构建执行也集中在最后的 R6。源码提交和静态审阅不等于通过测试。H-10 编程已授权；H-11 原生运行与 MAT 写入权限仍须单独确认。现行证据见 [PROGRESS](PROGRESS.md)。

## 代码实现（R0 → R5，期间禁止构建或测试）

> **后续静态复核（对象 `eeb30135`）**：R5 MAT Writer 的 const 输入不符合 matio 1.5.26 的 `void*` 接口，需重开该项源码修复；现有21项CTest不能证明暂存文件发布后故障或独立L2逐值已覆盖。具体证据与候选修正见 [R6接续复核](reviews/R6_READINESS_eeb30135.md)。本文后面的“代码实现完成”保留为历史批次判断，不代表当前缺陷已修复或工程已通过原生验证。

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

**状态（2026-11-16，源码静态核查）**：R0–R5业务C++实现范围已具备源码级闭合条件；**R5代码实现完成（未原生验证）**，不能把此前R5早期记录当成当前HEAD状态。实有`RunDirectory`独占RunId与dirfd锚定、MAT v5 Writer及回读/SHA-256/no-overwrite、`RunTextPublisher`、Complete/Failed/Incomplete Manifest、五实验Runner真实入口、四类只读Analyzer、结构化JSON、可选SVG，以及CLI Composition Root装配。R6已为这些源文件接入CMake，保留原有3项SH及R01/R02/R05测试，并新增3项**尚未执行**的合成合同测试（采样与光流/RNG、Adaptive Session失败原子性、Analysis分段与平滑），当前CTest登记共12项。**本分支R0–R6新源码没有原生构建、CTest、MAT读写或数值验收通过证据**；严格未更改`origin/**`，不据旧582/582探针或固定SHA的SH 3/3推断本HEAD通过。后续需H-11独立授权方可执行R6统一构建/CTest/安全负例、L2历史MAT、独立L3和必要L4统计验证；特别核SHRotate源组合/物理球谐作用、DEC-005/006容差与性能、基线批准和发布。真实测试若发现失败应重开对应代码批次修复。

## R6 本轮代码与测试源码补充（2026-11-16，仅写源码）

**复核结论：R6 尚未完成，不进入 R7。** 原路线图只定义 R0–R6；在 R6 测试源码准备、批准及真实执行闭环之前，不擅自设立 R7。当前 CTest **仅登记 16 项，执行证据 0 项（对于当前 R0–R6 新代码）**；由此前登记12项增加4项，均尚未编译、运行：

| 新R6登记 | 实际源码 | 边界 |
|---|---|---|
| `r06_kvd_independent_l3` | `tests/integration/test_r06_kvd_l3.cpp` | 固定6方向两组及8方向负均值、Franz分母/响应、Original/Modified近度/翻号/迭代停止/末次尺度；从 `verification/kvd/*.v1.json` 的独立数学参考选取数值，L3候选未批准 |
| `r06_adaptive_step_independent_l3` | `tests/integration/test_r06_adaptive_l3.cpp` | 非对称9方向正/负均值、旧系数、M1–M4、6×6 rhs/解、MSE、新near/九系数；参考 `verification/adaptive/asymmetric_signed_near_one_step.v1.json`，未批准 |
| `r06_historical_mat_readonly_schema` | `tests/integration/test_r06_archive_readonly.cpp` | 原582 MAT的只读metadata、单变量double解码及5类shape频数；计算每文件SHA-256但**没有独立预批准SHA进行比对，不是L2逐值Golden** |
| `r06_sh_argument_action` | `tests/integration/test_r06_sh_adapter.cpp` | AN-G08物理90°SO(3)、低阶SH对参数R/Rᵀ的反向鉴别、旋转-逆旋转往返；**不宣称源SHRotate逐项等价** |

同时修复 `tests/integration/test_r01_cli.cpp` 中R0占位错误消息/退出码的过时断言，增加**纯数值 analyze 能成功且可选 SVG 缺席时 plot 单独失败**的替身断言。旧3项球谐CTest与其他R0/R2/R5/R6既有登记保留，未修改任何 `origin/**`。

**仍未完成的R6义务**：实际 Debian/Ubuntu CMake配置/编译/CTest/异常负例与日志（H-11未授权）；真正跨目录L2逐值独立数值对照/预期SHA、完整L3逐节点与源SHRotate动作差异限定验证、原始实验与R4多帧/8-run行为、必要L4统计及错误修复、最终DEC-005/006审批与工程冻结发布。以上不得以“已增加测试源码”标为已验证通过；真实执行如发现错误须重新打开相应代码批次。**R6代码/测试准备继续进行，R7未定义。**

## R6 源码复核第二批（2026-11-16，不执行原生测试）

**最新状态：R6仍未结案，CTest登记由16项增加为20项，当前分支新增R0–R6代码原生测试通过证据仍为0项。** 本轮直接补足四个可定位的代码/测试缺口：

| R6新增或改造 | 实际代码 | 判据与证据边界 |
|---|---|---|
| 全行球面采样参考 | `tests/integration/test_r06_direction_rows_l3.cpp` | 读取已有独立 `verification/geometry/source_direction_levels_0_2.v1.json`，比较完整/缺损八面体及二十面体的0–2级**714行、2142坐标**，保持face和A/B/C/D源顺序，不再仅靠样本数与签名 |
| 轨迹与球内近度 | `tests/integration/test_r06_trajectory_sphere.cpp` | 圆35帧与正弦600帧的原N分母、连续三点局部运动与两端无效帧、SO(3)转角、球心/偏心近度、R乘本地视线、边界/非单位视线非成功 |
| Adaptive刷新成功/回滚 | `IAdaptiveStepSolver` 窄接口 + `AdaptiveMfaSession` 借用注入（默认真实求解器不变）+ `tests/contracts/test_r06_adaptive_periods.cpp` | MATLAB 1-based帧2–22，周期1/5/11/21在**主求解前**刷新；比较的`dumi`更新不提交；period5比较失败和主求解失败均原子回滚、可在原帧重试；Session禁复制/移动以保护内部指针 |
| Linux输出安全负例 | `tests/integration/test_r06_io_safety.cpp` | build目录隔离：重复RunId、非法名字、`origin`/输出根/仓库根符号链接拒绝；不操作真实`origin/**`或用户结果 |

`CMakeLists.txt`登记四个新CTest且修复先前两条路径中错误转义的 `${CMAKE_CURRENT_SOURCE_DIR}`（若不修复将无法正确展开源目录）；`build.sh`改为真实**全工程R6**范围及`build/r06-full`默认目录，加入`EGOMOTION_H11_APPROVED=YES`显式执行门禁，防止误把全工程构建当旧低阶SH构建。该开关只防误操作，**不是代替真实H-11审批**。本轮全部修改为源文件与静态复核，没有执行构建、CTest、MAT读写或数值实验。

**结案仍需**：H-11明确批准后才可原生配置/编译/CTest并逐项修复；完成真实MAT Reader/Writer安全竞态与独立文件SHA、历史L2逐值、R3/R4完整runner与必要L4统计、源SHRotate相位/参数动作限定、DEC-005/006最终数值与性能资格。新增20项“注册”不等于20项“通过”；未增设R7，也未改变历史源数据。

## R6 第三批：发布安全与合同缺口（2026-11-16，静态源码）

**R6尚未完成**。本轮在原20项CTest之外增加`r06_result_lifetimes_error_frames`，因此**现有登记21项，当前HEAD没有原生CTest执行或PASS证据**。代码实修：

1. `src/io/run_mat_writer.cpp`：为独占`.r5_*.mat-tmp`暂存文件增加RAII清理守卫，所有预发布错误路径都只删除本次生成的暂存名；成功链接最终MAT后，验证暂存硬链接移除以及目录fsync，最后才将receipt标记为`durably_confirmed`。最终文件已发布但清理/持久化不确定时维持非成功，保留既有文件及未确认receipt供`Incomplete`审计，**绝不覆盖最终结果**。
2. `src/io/run_text_publisher.cpp`：对`.text_*.tmp`作同样的独占暂存文件清理；最终链接发布后若清理/目录fsync不成功，则明确`ReadbackMismatch`，不假报成功。
3. `tests/integration/test_r05_io.cpp`：扩展真实Linux沙箱Writer正例与重复写负例，在独立RunId目录确认没有孤立的MAT/文本暂存文件。
4. `tests/contracts/test_r05_manifest.cpp`：扩展无效MAT SHA-256、未核实的40位构建源SHA、完整Writer receipts但Manifest发布失败、Failed/Incomplete持久化与完整底层错误帧的测试。缺数据或验证未通过，禁止产生`Complete`。
5. `tests/contracts/test_r06_result_lifetimes.cpp`：新增`Result<T,Code>`和`Status<Code>`的右值自拥有提取、临时对象析构后生命周期、move-only值、三层错误链/JSONL身份测试。CMake新增单独CTest，不改变已冻结公共错误码。

**边界**：这些是源码、CTest准备和静态审阅；没有H-11许可，未执行配置、编译、CTest、MAT读写、数值实验，未修改`origin/**`。R6继续需要真实原生执行/失败修复、L2独立SHA与逐值、剩余L3/L4实验与SHRotate限定核查、DEC-005/006和冻结签署；不能用注册21项替代通过21项。R7仍无已批准定义。
