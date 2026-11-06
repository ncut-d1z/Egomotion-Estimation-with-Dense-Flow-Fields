# 测试设计与验收计划（TEST PLAN）

> **草案，未冻结。** 测试标识 TC 均为拟定名称；下面的测试文件、CTest 前缀均为未来可能的产物，尚未建立。需求参见 [REQUIREMENTS.md](REQUIREMENTS.md)，实施参见 [PLAN.md](PLAN.md)。本轮未写 C++、未构建、未运行测试。

## 1. 验证分层和前提

- 顺序：源文件审计及 MAT 格式核验 → 单元测试 → 合同测试 → 入口集成测试 → 原始 MAT Golden 比对 → 论文统计复现。
- 确定性数值验收：同一固定输入比较逐步计算和误差；随机统计：独立统计有效/失败样本、抽样配置及噪声；不要求 MATLAB 与 C++ 同 seed 的 randn 逐位一致。
- 所有测试须记录 RunId、源 Git SHA、数据文件哈希、实际 Debian/Ubuntu/编译器/依赖版本、命令、参数、随机种子、预期与实测、误差、退出码及日志。
- 每条 M 级需求应有验收条件与至少一个真实测试或经批准的人工核查；任何尚未执行的用例不能宣称通过。

## 2. 首批关键测试的具体条件

### 2.1 CLI 契约（TC-F-CLI-001）

候选子命令必须分别核验帮助、必填参数、默认值、正常退出、缺参数、非法参数、输入文件错误、输出覆盖保护。**完整 CLI 语法和错误码需在 P0.8/P1.2 冻结，以下仅为拟议 schema。**

| 子命令 | 候选必填参数 | 输出/验收 |
|---|---|---|
| inspect-mat | --file PATH | MAT 变量名、rank、dims、type、压缩/错误诊断 |
| run-bias-occluded | --seed INT --level INT --output-root DIR | xwerte、ywerte1…6 的 MAT 结果 |
| run-bias-proportional | --seed INT --level INT --output-root DIR | 独立完整球面/逐分量噪声结果 |
| run-adaptive-init | --output-root DIR | 原始初始化四类误差结果 |
| run-adaptive-comparison | --noise-scale NUMBER --output-root DIR | 带噪/无噪的原始五类 MAT 结果 |
| run-adaptive-update-rates | --output-root DIR | 1/5/11/21 帧更新频率结果 |
| analyze | --experiment NAME --input-root DIR | 独立读取历史 MAT，计算统计序列 |
| plot（可选） | --experiment NAME --input-root DIR | 图表结果；无图形后端不影响 analyze |

建议退出码类别（尚未批准）：0 成功、2 参数错误、3 MAT/输入 I/O 错误、4 格式/类型/形状错误、5 数值无效、6 输出目录保护、7 可选功能不可用。实施前冻结状态优先级与错误含义。

### 2.2 Bias 噪声（TC-F-BIAS-001、TC-F-BIAS-002）

原脚本三项系数是 [0.6, 2.4, 9.6]。非球面情形按 randn × 系数 × 全场平均光流模长逐分量扰动；完整球面情形按 randn × 系数 × 对应光流分量扰动；两者都要将流重新投影到切向。**不得将系数开平方**。用固定 Gaussian 序列（含零、正负非零值）对每个分量验证，核对六组误差和 xwerte MAT 变量尺寸。

### 2.3 球面采样顺序（TC-F-GEOM-002、TC-F-GEOM-003）

分别对完整八面体 8·4^i、缺损八面体 6·4^i、二十面体 20·4^i 在 i=0、1、2 核对数量、单位范数、缺损面及**相同顺序下的每行三维方向**；不能只比较无序集合或哈希。Golden 输入应来自原 MATLAB 原函数而非 C++ 自算输出；逐元素数值容差须实测后冻结。

### 2.4 600 点 Adaptive 轨迹输出（TC-F-EXP-002）

原始 agent_in_sphere_adaptive_matched_filter_test.m 采用 stellen=600、anzahliterationen=4、anzahldurchlaeufe=8、帧循环 i=2:599。当前代码明确 save 五个变量：allemittransfehler、allemitrotfehler、alleohnetransfehler、alleohnerotfehler、rottransratio，均保存为同名 MAT 文件。前四项为八次运行平均得到的 600×1 缓冲；rottransratio 仅保留代码在运行循环中最后赋值的 600×1 缓冲，不能擅自平均。with_errors 历史额外出现的 alleadap* 须作为数据差异调查，不加入虚构的当前生成合同。

### 2.5 原始 MAT、数值容差和失败判定

**数据字典已通过独立 C++ matio 探针实际扫描**：Ubuntu 24.04 / matio 1.5.26，582/582 源 MAT 全部被逐变量 `Mat_VarRead` 解码，582 个变量均 `READ_OK`，失败 0。原始目录以 `MAT_ACC_RDONLY` 打开；见 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md)。P0.5/0.6 的文档合同、正式业务 Reader/Writer 与负例仍需测试；P10.4 独立复核尚未完成。

对 Original/Modified KvD 的 near、t、ω、终止条件逐轮比对；Adaptive 比对 6×6 系统各子块、右端、旋转、残差与球谐系数更新。 **特别核验：Adaptive oneit 返回的 error 是 MSE，sqrt(error) 在 MATLAB 内部另算但没有返回；测试记录必须分列 error_mse 和仅用于诊断的 rms，不得将二者互作。** double/1e-10 是先前候选相对容差，尚未冻结；应按模块记录绝对误差、相对误差、条件数和样本有效性，再由维护者批准。不对零向量、NaN、Inf 或未定义方向误差计入成功。

### 2.6 已执行 matio 全量探针的证据与限制

| 证据项 | 实测结果 | 当前验收边界 |
|---|---|---|
| 平台与工具 | Ubuntu 24.04，GCC 13，matio 1.5.26；源参数 SHA `48c590ba2564e4bdd293d2cbecd2eef83b291452` | 独立源文件 `src/mat_data_dict_probe.cpp` 已编译运行；非正式 CMake/CTest |
| 读取 | 582/582 MAT、582/582 变量、失败 0 | `Mat_Open(MAT_ACC_RDONLY)` + `Mat_GetDir` + `Mat_VarRead`；未做数据逐元素 MATLAB Golden |
| 格式与类别 | 全部 MAT v5 / zlib / MATLAB double、存储 double | 当前数据不需要 HDF5、cell、struct、complex、sparse 适配；新增数据另评审 |
| 形状分布 | 10×1:80，10×3:480，35×1:4，600×1:14，600×4:4 | 真实历史结果 shape；不代表所有未来输出或算法形状 |
| 文件身份 | 582 个文件各有 FNV-1a 64 位指纹 | FNV 非 SHA-256；尚须 P10.4 独立目录/文件身份审核 |
| 未运行的验收 | 严格/兼容读取、类型拒绝负例、写回读、RAII 下异常、原文件哈希比对、C++ 求解器/CTest | 不能用探针的 exit=0 将这些 TC 标为通过 |

详细记录以 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) 的 582 条逐变量表为准，以下的 TC 状态仅表明**相关子能力有实测证据**，而非整个接口合同正式验收。

### 2.7 更新频率实验与四种分析入口（TC-F-EXP-003、TC-F-AN-001）

- 更新率当前脚本：600 点、二十面体等级 2、**1/5/11/21 帧**，输出 600×4 的 allemittransfehler、allemitrotfehler，以及 600×1 的 akrot；历史 many_parameter 额外出现 alleadaprotfehler、alleadaptransfehler、rottransratio，需分别记“当前生成/归档存在/分析消费/来源状态”，不补造。
- 初始化分析图：allemittransfehler(1:13,1) 与 allemitrotfehler(:,1) 的数值、索引、对数纵轴语义；数据被读取不代表一定进入图线。
- 比较分析图：x=50:550 的旋转与平移方向角度误差、rottransratio 双纵轴，阈值 0.01，分段规则精确依据原 MATLAB。
- 更新频率分析图：x=50:550、阈值 0.02，四条曲线与不同半窗 2/5/10 的平滑；kurveglaetten 的边缘零值语义不替换。
- 每种曲线的 Golden 结构应记录 source MAT+variable、原 MATLAB 1-based 索引、x/y 数组、axis/units、分段、窗宽和有效样本状态；只比较图像像素不能算通过。

## 3. TC → 实体/CTest → AC 规划矩阵

下列表格共 66 项，与 SRS v0.2 逐项对应。**独立 matio 探针已编写并执行**，但绝大多数拟议测试源文件、测试函数和 CTest 名称尚未创建；探针不是 CTest 注册项。人工审查项与自动测试区分。原始 45 个 MATLAB 源文件映射请参见 [REQUIREMENTS.md §9.2](REQUIREMENTS.md)。

| TC ID | REQ | 候选测试文件或审核文档 | CTest/审核 | AC | 状态 |
|---|---|---|---|---|---|
| TC-F-CLI-001 | REQ-F-CLI-001 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-F-CLI-001 | 未编写/未运行 |
| TC-F-MAT-001 | REQ-F-MAT-001 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-F-MAT-001 | 独立 matio 探针 582/582 已实测；业务 TC/CTest 及 P10.4 复核未完成 |
| TC-F-MAT-002 | REQ-F-MAT-002 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-F-MAT-002 | 独立 matio 探针 582/582 已实测；业务 TC/CTest 及 P10.4 复核未完成 |
| TC-F-MAT-003 | REQ-F-MAT-003 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-F-MAT-003 | 未编写/未运行 |
| TC-F-MAT-004 | REQ-F-MAT-004 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-F-MAT-004 | 未编写/未运行 |
| TC-F-GEOM-001 | REQ-F-GEOM-001 | tests/unit/test_sampling.cpp | egomotion.geometry.* | AC-F-GEOM-001 | 未编写/未运行 |
| TC-F-GEOM-002 | REQ-F-GEOM-002 | tests/unit/test_sampling.cpp | egomotion.geometry.* | AC-F-GEOM-002 | 未编写/未运行 |
| TC-F-GEOM-003 | REQ-F-GEOM-003 | tests/unit/test_sampling.cpp | egomotion.geometry.* | AC-F-GEOM-003 | 未编写/未运行 |
| TC-F-FLOW-001 | REQ-F-FLOW-001 | tests/unit/test_flow_simulation.cpp | egomotion.flow.* | AC-F-FLOW-001 | 未编写/未运行 |
| TC-F-FLOW-002 | REQ-F-FLOW-002 | tests/unit/test_flow_simulation.cpp | egomotion.flow.* | AC-F-FLOW-002 | 未编写/未运行 |
| TC-F-FLOW-003 | REQ-F-FLOW-003 | tests/unit/test_flow_simulation.cpp | egomotion.flow.* | AC-F-FLOW-003 | 未编写/未运行 |
| TC-F-FLOW-004 | REQ-F-FLOW-004 | tests/unit/test_flow_simulation.cpp | egomotion.flow.* | AC-F-FLOW-004 | 未编写/未运行 |
| TC-F-FILTER-001 | REQ-F-FILTER-001 | tests/unit/test_estimators.cpp | egomotion.estimators.* | AC-F-FILTER-001 | 未编写/未运行 |
| TC-F-KVD-001 | REQ-F-KVD-001 | tests/unit/test_estimators.cpp | egomotion.estimators.* | AC-F-KVD-001 | 未编写/未运行 |
| TC-F-KVD-002 | REQ-F-KVD-002 | tests/unit/test_estimators.cpp | egomotion.estimators.* | AC-F-KVD-002 | 未编写/未运行 |
| TC-F-ERROR-001 | REQ-F-ERROR-001 | tests/unit/test_estimators.cpp | egomotion.estimators.* | AC-F-ERROR-001 | 未编写/未运行 |
| TC-F-BIAS-001 | REQ-F-BIAS-001 | tests/integration/test_bias.cpp | egomotion.bias.* | AC-F-BIAS-001 | 未编写/未运行 |
| TC-F-BIAS-002 | REQ-F-BIAS-002 | tests/integration/test_bias.cpp | egomotion.bias.* | AC-F-BIAS-002 | 未编写/未运行 |
| TC-F-BIAS-003 | REQ-F-BIAS-003 | tests/integration/test_bias.cpp | egomotion.bias.* | AC-F-BIAS-003 | 未编写/未运行 |
| TC-F-TRAJ-001 | REQ-F-TRAJ-001 | tests/unit/test_trajectory.cpp | egomotion.trajectory.* | AC-F-TRAJ-001 | 未编写/未运行 |
| TC-F-SH-001 | REQ-F-SH-001 | tests/unit/test_harmonics.cpp | egomotion.harmonics.* | AC-F-SH-001 | 未编写/未运行 |
| TC-F-SH-002 | REQ-F-SH-002 | tests/unit/test_harmonics.cpp | egomotion.harmonics.* | AC-F-SH-002 | 未编写/未运行 |
| TC-F-ADAPT-001 | REQ-F-ADAPT-001 | tests/unit/test_adaptive_mfa.cpp | egomotion.adaptive.* | AC-F-ADAPT-001 | 未编写/未运行 |
| TC-F-ADAPT-002 | REQ-F-ADAPT-002 | tests/unit/test_adaptive_mfa.cpp | egomotion.adaptive.* | AC-F-ADAPT-002 | 未编写/未运行 |
| TC-F-ADAPT-003 | REQ-F-ADAPT-003 | tests/unit/test_adaptive_mfa.cpp | egomotion.adaptive.* | AC-F-ADAPT-003 | 未编写/未运行 |
| TC-F-EXP-001 | REQ-F-EXP-001 | tests/integration/test_adaptive_experiments.cpp | egomotion.experiments.* | AC-F-EXP-001 | 未编写/未运行 |
| TC-F-EXP-002 | REQ-F-EXP-002 | tests/integration/test_adaptive_experiments.cpp | egomotion.experiments.* | AC-F-EXP-002 | 未编写/未运行 |
| TC-F-EXP-003 | REQ-F-EXP-003 | tests/integration/test_adaptive_experiments.cpp | egomotion.experiments.* | AC-F-EXP-003 | 未编写/未运行 |
| TC-F-AN-001 | REQ-F-AN-001 | tests/golden/test_analysis.cpp | egomotion.analysis.* | AC-F-AN-001 | 未编写/未运行 |
| TC-F-AN-002 | REQ-F-AN-002 | tests/golden/test_analysis.cpp | egomotion.analysis.* | AC-F-AN-002 | 未编写/未运行 |
| TC-D-SOURCE-001 | REQ-D-SOURCE-001 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-D-SOURCE-001 | 未编写/未运行 |
| TC-D-SOURCE-002 | REQ-D-SOURCE-002 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-D-SOURCE-002 | 未编写/未运行 |
| TC-D-MAT-001 | REQ-D-MAT-001 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-D-MAT-001 | 独立 matio 探针 582/582 已实测；业务 TC/CTest 及 P10.4 复核未完成 |
| TC-D-MAT-002 | REQ-D-MAT-002 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-D-MAT-002 | 未编写/未运行 |
| TC-D-MAT-003 | REQ-D-MAT-003 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-D-MAT-003 | 未编写/未运行 |
| TC-DC-DOMAIN-001 | REQ-DC-DOMAIN-001 | docs/OBJECT_MODEL.md / docs/SOURCE_INVENTORY.md | 人工设计/来源评审；部分接口需合同测试 | AC-DC-DOMAIN-001 | 未编写/未运行 |
| TC-DC-IO-001 | REQ-DC-IO-001 | docs/OBJECT_MODEL.md / docs/SOURCE_INVENTORY.md | 人工设计/来源评审；部分接口需合同测试 | AC-DC-IO-001 | 未编写/未运行 |
| TC-DC-RNG-001 | REQ-DC-RNG-001 | docs/OBJECT_MODEL.md / docs/SOURCE_INVENTORY.md | 人工设计/来源评审；部分接口需合同测试 | AC-DC-RNG-001 | 未编写/未运行 |
| TC-DC-NOISE-001 | REQ-DC-NOISE-001 | docs/OBJECT_MODEL.md / docs/SOURCE_INVENTORY.md | 人工设计/来源评审；部分接口需合同测试 | AC-DC-NOISE-001 | 未编写/未运行 |
| TC-DC-KVD-001 | REQ-DC-KVD-001 | tests/unit/test_estimators.cpp | egomotion.estimators.* | AC-DC-KVD-001 | 未编写/未运行 |
| TC-DC-ADAPT-001 | REQ-DC-ADAPT-001 | tests/unit/test_adaptive_mfa.cpp | egomotion.adaptive.* | AC-DC-ADAPT-001 | 未编写/未运行 |
| TC-DC-EXPERIMENT-001 | REQ-DC-EXPERIMENT-001 | tests/integration/test_adaptive_experiments.cpp | egomotion.experiments.* | AC-DC-EXPERIMENT-001 | 未编写/未运行 |
| TC-DC-ANALYSIS-001 | REQ-DC-ANALYSIS-001 | tests/golden/test_analysis.cpp | egomotion.analysis.* | AC-DC-ANALYSIS-001 | 未编写/未运行 |
| TC-NF-PLATFORM-001 | REQ-NF-PLATFORM-001 | tests/integration/test_deployment.cpp | egomotion.deployment.* | AC-NF-PLATFORM-001 | 未编写/未运行 |
| TC-NF-DEPS-001 | REQ-NF-DEPS-001 | tests/integration/test_deployment.cpp | egomotion.deployment.* | AC-NF-DEPS-001 | 未编写/未运行 |
| TC-NF-SAFETY-001 | REQ-NF-SAFETY-001 | tests/contracts/test_output_safety.cpp | egomotion.safety.* | AC-NF-SAFETY-001 | 未编写/未运行 |
| TC-NF-NUM-001 | REQ-NF-NUM-001 | tests/golden/test_numerical_equivalence.cpp | egomotion.numerics.* | AC-NF-NUM-001 | 未编写/未运行 |
| TC-NF-NUM-002 | REQ-NF-NUM-002 | tests/golden/test_numerical_equivalence.cpp | egomotion.numerics.* | AC-NF-NUM-002 | 未编写/未运行 |
| TC-PROC-TRACE-001 | REQ-PROC-TRACE-001 | docs/REQUIREMENTS.md | 人工追踪审查 | AC-PROC-TRACE-001 | 未编写/未运行 |
| TC-NF-RNG-001 | REQ-NF-RNG-001 | tests/golden/test_numerical_equivalence.cpp | egomotion.numerics.* | AC-NF-RNG-001 | 未编写/未运行 |
| TC-NF-LOG-001 | REQ-NF-LOG-001 | tests/contracts/test_runlog.cpp | egomotion.logging.* | AC-NF-LOG-001 | 未编写/未运行 |
| TC-NF-OOP-001 | REQ-NF-OOP-001 | tests/contracts/test_state_isolation.cpp | egomotion.isolation.* | AC-NF-OOP-001 | 未编写/未运行 |
| TC-NF-HEADLESS-001 | REQ-NF-HEADLESS-001 | tests/integration/test_headless.cpp | egomotion.headless.* | AC-NF-HEADLESS-001 | 未编写/未运行 |
| TC-NF-RECOVERY-001 | REQ-NF-RECOVERY-001 | tests/contracts/test_output_safety.cpp | egomotion.safety.* | AC-NF-RECOVERY-001 | 未编写/未运行 |
| TC-NF-PORTABLE-001 | REQ-NF-PORTABLE-001 | tests/integration/test_deployment.cpp | egomotion.deployment.* | AC-NF-PORTABLE-001 | 未编写/未运行 |
| TC-NF-RUNTIME-001 | REQ-NF-RUNTIME-001 | tests/benchmarks/benchmark_original_runs.cpp | 非 CTest 基准 | AC-NF-RUNTIME-001 | 未编写/未运行 |
| TC-NF-SESSION-001 | REQ-NF-SESSION-001 | tests/contracts/test_state_isolation.cpp | egomotion.isolation.* | AC-NF-SESSION-001 | 未编写/未运行 |
| TC-NF-COPYRIGHT-001 | REQ-NF-COPYRIGHT-001 | docs/OBJECT_MODEL.md / docs/SOURCE_INVENTORY.md | 人工设计/来源评审；部分接口需合同测试 | AC-NF-COPYRIGHT-001 | 未编写/未运行 |
| TC-F-CLI-002 | REQ-F-CLI-002 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-F-CLI-002 | 未编写/未运行 |
| TC-F-CLI-003 | REQ-F-CLI-003 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-F-CLI-003 | 未编写/未运行 |
| TC-F-CLI-004 | REQ-F-CLI-004 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-F-CLI-004 | 未编写/未运行 |
| TC-F-CLI-005 | REQ-F-CLI-005 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-F-CLI-005 | 未编写/未运行 |
| TC-I-CLI-001 | REQ-I-CLI-001 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-I-CLI-001 | 未编写/未运行 |
| TC-I-MAT-001 | REQ-I-MAT-001 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-I-MAT-001 | 未编写/未运行 |
| TC-I-RESULT-001 | REQ-I-RESULT-001 | tests/integration/test_adaptive_experiments.cpp | egomotion.experiments.* | AC-I-RESULT-001 | 未编写/未运行 |
| TC-I-ANALYSIS-001 | REQ-I-ANALYSIS-001 | tests/golden/test_analysis.cpp | egomotion.analysis.* | AC-I-ANALYSIS-001 | 未编写/未运行 |

## 4. 非功能指标和验收记录格式

| 指标 | 必须记录的现状 | 目标冻结状态 |
|---|---|---|
| Debian/Ubuntu 构建 | 编译器版本、CMake、依赖、构建命令及结果 | 支持版本范围待实测 |
| 性能与内存 | 完整原始实验的 wall/CPU 时间、峰值 RSS、输入大小及构建耗时 | 阈值、加速比和覆盖率尚未批准，不得凭空填写 |
| 随机与会话隔离 | 运行种子、FixedSequenceRandomSource、交替双 Session 状态 | 单个 Session 的多线程安全不作承诺 |
| 安全与失败恢复 | origin 文件哈希、输出目录检查、RunId 冲突测试、日志/退出码 | 不擅自引入自动重试或新 SLA |

### 4.1 单次测试证据模板

| 字段 | 必需内容 |
|---|---|
| 标识 | REQ、TC、AC、对应 MATLAB 文件路径及源 SHA |
| 环境 | Debian/Ubuntu 版本、编译器、CMake、Eigen/matio/zlib 版本 |
| 执行 | 完整命令/参数、RunId、输入 SHA、随机源/种子、输出目录 |
| 数值 | 预期/实际、绝对/相对误差、容差来源、有效/无效/无定义样本数 |
| 状态 | 未编写、未运行、通过、失败、因依赖阻塞、条件不适用，及证据位置 |

### 4.2 测试优先级与最低资源配置（计划，未测）

| 级别 | 测试集合 | 触发时机与资源 | 失败处理 |
|---|---|---|---|
| T0（必须、快速） | FlowField 形状、叉积/球面采样 i=0/1、固定随机序列、最小 Franz/KvD/Adaptive 单步 | 每个相关模块提交后；Debian/Ubuntu 一台开发环境即可；具体 CPU/内存/耗时未知 | 阻止将相应模块标为验证通过，保存最小输入和日志 |
| T1（必须、数据合同） | matio/zlib 元数据、只读保护、结果新目录写回读、异常输入/路径冲突 | P1 MAT Reader/Writer 可用后；只读原始 MAT 与独立写目录 | 保留所有失败文件和原因，绝不修改 origin |
| T2（必须、全量） | **582 个 MAT、582 变量的全量读取/字典**及 45 个 .m 的功能归属 | 独立 matio 探针在 Ubuntu 已实际执行、全部 READ_OK；P1 业务 Reader 与 P10.4 复核尚待实施 | 582/582 无读取失败是**已获得的子项证据**，不可扩展宣称所有类型负例、原 MATLAB Golden 或项目整体验收通过 |
| T3（必须、数值 Golden） | KvD 逐轮、SH 旋转块、Adaptive M1–M4 和 MSE | 固定 MATLAB/独立参考数据以及 DEC-005 容差批准后 | 未取得参考/容差时明确阻塞，不能用 C++ 自算结果替代 |
| T4（必须、实验复现） | 两种 Bias、三种 Adaptive、四类分析及原论文曲线 | P5/P8/P9/P10 入口实际可执行后；性能资源按首轮实测安排 | 保留统计分母、失败/无定义样本、运行命令和真实耗时 |
| T5（应该、附加） | 可选绘图、额外编译器/CPU 组合、扩大性能测试 | 在必须需求有证据后，按项目所有者授权分期 | 明示未执行，不阻塞未要求的功能 |

**资源责任分工**：项目所有者批准需要运行的全量试验和数据使用范围；开发负责人提供测试入口与提交 SHA；数值验证负责人准备有来源的 Golden；维护/集成负责人核对目标 Debian/Ubuntu 工具链和 CTest 证据。当前没有经确认的设备型号、执行预算或时限，因此不编造 CPU 核心数、RAM 容量、分钟数、覆盖率及测试费用。

## 5. 原始 MAT 数据管理及 Golden 策略

### 5.1 测试数据来源清单

| 原数据分组 | 当前文件数量 | 角色 | 全量核验 | 开发期代表性样本 |
|---|---:|---|---|---|
| bias_test/produced_data/occluded_regions | 280（40×7） | 非球形 Bias 统计基准 | **matio 280/280 READ_OK**；xwerte 40×10×1，ywerte 240×10×3，class/type 均 double；独立复核待做 | 编号 1、中间编号、40 |
| bias_test/produced_data/proportional_error | 280（40×7） | 球形比例噪声 Bias 统计基准 | **matio 280/280 READ_OK**；xwerte 40×10×1，ywerte 240×10×3，double/double；独立复核待做 | 编号 1、中间编号、40 |
| adaptive_matched_filter_test/produced_data/initialization | 4 | 初始化已保存结果 | **matio 4/4 READ_OK**；全部 double[35×1] | 4 个文件 |
| adaptive_matched_filter_test/produced_data/without_errors | 5 | 无噪声对比 | **matio 5/5 READ_OK**；全部 double[600×1] | 5 个文件 |
| adaptive_matched_filter_test/produced_data/with_errors | 7 | 带噪声对比及历史额外字段 | **matio 7/7 READ_OK**；全部 double[600×1]，含两份历史独有 alleadap* | 7 个文件 |
| adaptive_matched_filter_test/produced_data/many_parameter | 6 | 多更新率与历史额外字段 | **matio 6/6 READ_OK**；四个 double[600×4]、两个 double[600×1] | 6 个文件 |
| **合计** | **582** | 原作者静态参考数据 | **matio 已读 582/582（582 变量、0 失败）；独立复核未完成** | 实际全量结果见 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) |

**数据保护**：origin/** 作为只读输入；测试只产生新目录或测试专属副本；不得向原目录写入、覆盖、删除或以 git add -f 强制提交忽略内容。样本选择和目录哈希应随源仓库提交 SHA 固定。

### 5.2 Golden 文件的产生、保管与追溯

1. **现有 Golden**：作者归档中真正存在的 MAT 结果，可作为变量合同与统计结果参考，但没有每轮中间量时不能冒称提供了逐步算法真值。
2. **待生成的数值 Golden**：Franz 模板项、Original/Modified KvD 的 near/t/ω、Adaptive 的 M1…M4/b/z/error_mse、新系数、球谐块旋转。必须在合法且经授权的 MATLAB/原脚本执行环境捕获中间量，或使用经独立审核的固定解析解，不能以被测 C++ 程序自己的结果作为真值。
3. **身份字段**：源 .m Git blob SHA、源树 SHA、工具及版本、MATLAB 输入 seed 与固定数据、变量名/shape/dtype、参考数值文件 SHA-256、许可证/可存储位置。
4. **存放位置**：小型、可再分发的定值 Golden 在经版权核验后可放 tests/golden/fixtures；受限或大型资料放在维护者批准的独立持久位置，仓库只保存路径映射和哈希，不擅自复制/再许可。
5. **版本更新**：变更输入、脚本 SHA、近度定义、索引或容差时，应生成新的 Golden 版本并保存历史，不静默覆盖旧结果。
6. **样本统计**：每次用例记录文件总数、成功解码、格式不支持、损坏、缺失、统计有效/失败/未定义，不能只报告成功部分。

### 5.3 数据合同变更与试验数据基准

MAT 全量字典的字段定义和状态见 [design/mat_io.md](design/mat_io.md) 与 SRS 中 DEC-001/002。验收前需生成 docs/MAT_DATA_SCHEMA.md 及来源证据。测试文档中的“582 份 MAT 均可用”仍是**目标而非事实**。

## 6. Debian/Ubuntu 环境与配置矩阵

| 环境组 | 发行版 / CPU 架构 | 编译器 | Eigen / matio / zlib | 当前状态 |
|---|---|---|---|---|
| LINUX-01 | **Ubuntu 24.04 / x86_64（用户提供的 VirtualBox 环境）** | **GCC 13**（已编译独立探针） | matio **1.5.26**、zlib1g-dev 已安装；Eigen/CMake 业务构建未验证 | **仅 matio 探针编译/运行成功**；非完整工程及 CTest |
| LINUX-02 | Debian stable，具体版本待确认 / x86_64 | GCC 或 Clang，具体版本待记录 | 依赖版本待记录 | **未构建** |
| LINUX-03（可选） | Debian/Ubuntu 的另一受支持编译器或 CPU 架构 | 由维护者决定 | 兼容性待核 | 未安排 |

- 不将 Windows 或交叉编译视为本项目要求；不凭空规定编译器/Eigen/matio 的最低版本。
- 每组记录 uname、/etc/os-release、编译器版本、CMake 版本、依赖实际版本、CMake 选项、build SHA、CTest 名称与结果；未执行不能在平台矩阵打勾。
- 依赖安装、构建及数值测试必须遵守 [AGENTS.md](../AGENTS.md) 的远程检查点和 Git 安全规则。

## 7. 非功能测试方法与风险

| 测试领域 | 实施方法（计划） | 判定与不确定性 |
|---|---|---|
| 性能 | 选定原论文全量配置，固定 CPU/线程、seed、输入规模，记录墙钟（单调时钟）、CPU user+system、样本数与负载，必要时多次重复并报告分布 | 先形成基线，**没有经批准的时间/加速比阈值就不宣称性能通过** |
| 内存 | 使用 Debian/Ubuntu 可获得的峰值 RSS 记录方式，分别测 MAT 解码、最大采样、600 点实验与结果输出 | 先报告 peak RSS、输入数据量和分配行为，阈值 DEC-006 待定 |
| 构建时间 | 同一源 SHA、工具链/构建类型下记录源外配置、编译、链接时间；区分冷构建和增量构建 | 尚未设构建耗时门限 |
| 恢复/安全 | 对**测试副本**制造无效文件、权限不足、输出 RunId 冲突和中断情况，检查输出不覆盖、错误定位、历史日志保留 | 不允许破坏 origin 原始文件；正式自动恢复 SLA 未定义 |
| 会话隔离 | 交替运行两份不同 seed/刷新频率的 RunContext，并分别独立复算 | 结果与各独立运行一致；**不承诺单个 Session 多线程安全** |
| 可重复数值 | 独立固定输入与 Golden 的每步误差、统计有效率、无定义数目 | DEC-005 逐模块容差未冻结，不能判定 M 级通过 |
| 头less | 不安装/禁用可选绘图后端，分别运行 inspect、实验、analyze | 数值路径不应因绘图库缺失而不可用 |

### 7.1 测试策划风险

| 风险 | 缓解措施 / 状态 |
|---|---|
| 当前数据类型已明确；未来扩展可能带来其他 MAT class/shape | 当前归档 582/582 为 MAT v5 + zlib + double；仅需针对未来未知类型提供明确拒绝和负例，业务 Reader 未验收 |
| MATLAB 随机数与 C++ 难以逐样本一致 | 使用固定输入 Golden 验证算法，单独验证统计分布与运行种子 |
| 当前生成脚本和历史 MAT 字段不一致 | “当前源码/历史归档/分析依赖”三方差异表，不伪造字段 |
| 无 MATLAB 运行环境或缺权利授权，难以生成中间量 Golden | 标为阻塞，先完成不依赖该 Golden 的读取/纯数学测试；不能以 C++ 自验代替 |
| 运行资源消耗或测试超时 | 保存命令及失败证据，先进行小规模单元回归；完整论文实验要单独安排，不因小规模通过而虚称全量 |
| 未经批准的绝对/相对误差门限 | 保留候选及测量结果，项目所有者确认后冻结；历史失败不删 |

## 8. 测试阶段进入/退出准则

| 阶段 | 进入准则 | 退出准则 |
|---|---|---|
| 测试策划基线 | SRS 编号与来源、风险和 P 任务映射已核对 | TC/AC、数据管理、环境矩阵、未决 DEC 和审查人明确；**不要求软件已运行** |
| 单元测试 | P0.8 所需接口/数值合同评审、相应代码已提交推送 | 对应 TC 实际执行并保存正常/异常证据；未通过保持开放 |
| MAT/集成测试 | P1 有最小 MAT 读写能力、原始输入受保护 | 相关源 MAT 全量读取/实际字段校验，输出安全/回读测试真实通过 |
| Golden 数值测试 | 固定 MATLAB/独立参考与批准容差可用 | 逐轮变量和公式通过判据，失败/退化样本单列 |
| 论文复现 | 各入口和输出合同真实可执行、统计流程可用 | 每类实验的有效/失败数量、误差、曲线数据、差异说明和完整证据 |
| 最终交付 | M 级要求及批准的 S 级范围有可追踪状态 | 项目所有者批准例外与未决项，PR/文档/测试/提交链路完整 |

**状态区分**：独立 matio 探针在 Ubuntu 上确已编译运行且 582/582 读取成功；但**没有**由本项目注册/运行 CTest，也未完成正式 Reader/Writer、负例、逐元素 Golden 或论文复现。其余阶段的进入/退出门槛不得因这一项成功而视为满足。

### 8.1 文档追踪静态一致性检查（不等同于执行测试）

每次 REQ、P、TC、AC 或源 MATLAB 文件映射改变后，审查者应对 GitHub main/目标分支的**实际当前文件内容**做以下集合检查（可由后续经授权的工具实现，当前没有提交可执行脚本）：

1. 解析 REQUIREMENTS §4 的全部 REQ ID，每个 ID 唯一且有对应的 `AC-`；解析 §9 的正向追踪，不允许不存在的需求 ID 或空 P/TC/AC。
2. 解析 TEST_PLAN §3 的 TC ID，检查 TC 集合严格等于 `{ "TC-" + REQ[4:] }`；对每个 TC 校验其引用的 REQ 和 AC 也完全匹配。人工核查类型不必注册 CTest，但必须有真实证据路径。
3. 解析 PLAN §5 的全部 P ID 与依赖，检查无重复、无未知前置、无有向环；每个 P 的 REQ、TC、AC 集合必须成对一一对应，任何孤立 REQ/P 都需要解释。
4. 解析 `origin/egomotion_estimation_dense_flow_fields_matlab_scripts/**/*.m` 与 SRS §9.2 的路径集合，确保两边完全一致；`OBJECT_MODEL.md` 的对象职责映射也应覆盖全部源文件，未移植标注不等于完成。
5. 数据文件数量、版本和哈希应来自真实 Git 树，不从对话旧表复制；只有实际完整解码后才检查 AC-D-MAT-001 的值域/类型/维度合同。
6. 将每轮静态审查报告记录为：源 Git SHA、REQ/P/TC/AC 和 .m 文件数量、唯一性、漏项、环数量、被审阅文件清单。只读检查成功不应标为 CTest、Golden 或需求已通过。

**当前文档级检查口径**：66 条 REQ、66 项 TC、98 个 P 工作项、45 个 .m 映射是预期集合数量；正式验收以**ID 集合完全匹配**为准，不仅对比计数相同。

## 9. 变更与冻结条件

任何更改 MATLAB 对应公式、输出变量、MAT 读取边界、随机数处理、数值容差或实验输入都必须更新 REQ/TC/AC 对照并重新生成对照证据；不能通过跳过失败样本、修改阈值或者仅重新绘图来宣布结果通过。真实构建与 CTest 开始之前须遵守 AGENTS.md 的远程检查点规则。

**当前状态：66 个 TC 均未创建真实测试函数或 CTest，不能计为通过。**
