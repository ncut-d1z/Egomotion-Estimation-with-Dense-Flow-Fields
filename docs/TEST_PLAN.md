# 测试设计与验收计划（TEST PLAN）

> **草案，未冻结。** 测试标识 TC 均为拟定名称；下面的测试文件、CTest 前缀均为未来可能的产物，尚未建立。需求参见 [REQUIREMENTS.md](REQUIREMENTS.md)，实施参见 [PLAN.md](../PLAN.md)。本轮未写 C++、未构建、未运行测试。

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

582 份原始 MAT 应逐文件只读验证文件哈希、解压、变量名、dtype、shape 和完整读取结果；两个 Bias 目录合计 560 份，Adaptive 四个目录合计 22 份。记录失败与未支持类型的精确数量。

对 Original/Modified KvD 的 near、t、ω、终止条件逐轮比对；Adaptive 比对 6×6 系统各子块、右端、旋转、残差与球谐系数更新。double/1e-10 是先前候选相对容差，尚未冻结；应按模块记录绝对误差、相对误差、条件数和样本有效性，再由维护者批准。不对零向量、NaN、Inf 或未定义方向误差计入成功。

## 3. TC → 实体/CTest → AC 规划矩阵

下列表格共 66 项，与 SRS v0.2 逐项对应。源文件、测试函数、CTest 名称全部只是**拟议实体**，不代表已经创建或执行；人工审查项与自动测试区分。原始 45 个 MATLAB 源文件映射请参见 [REQUIREMENTS.md §9.2](REQUIREMENTS.md)。

| TC ID | REQ | 候选测试文件或审核文档 | CTest/审核 | AC | 状态 |
|---|---|---|---|---|---|
| TC-F-CLI-001 | REQ-F-CLI-001 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-F-CLI-001 | 未编写/未运行 |
| TC-F-MAT-001 | REQ-F-MAT-001 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-F-MAT-001 | 未编写/未运行 |
| TC-F-MAT-002 | REQ-F-MAT-002 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-F-MAT-002 | 未编写/未运行 |
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
| TC-D-MAT-001 | REQ-D-MAT-001 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-D-MAT-001 | 未编写/未运行 |
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

## 5. 变更与冻结条件

任何更改 MATLAB 对应公式、输出变量、MAT 读取边界、随机数处理、数值容差或实验输入都必须更新 REQ/TC/AC 对照并重新生成对照证据；不能通过跳过失败样本、修改阈值或者仅重新绘图来宣布结果通过。真实构建与 CTest 开始之前须遵守 AGENTS.md 的远程检查点规则。

**当前状态：66 个 TC 均未创建真实测试函数或 CTest，不能计为通过。**
