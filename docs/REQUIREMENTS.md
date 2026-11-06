# Egomotion Estimation with Dense Flow Fields：软件需求规格说明书（SRS）

> **状态：草案 v0.1，未冻结、未验收。** 本文档规定系统的可验证义务；根目录 [PLAN.md](../PLAN.md) 负责实现架构、P0–P10 任务及依赖；[AGENTS.md](../AGENTS.md) 负责开发与 Git 安全规则。需求依据被确认不代表程序已实现。

## 1. 目的、范围与约束

- 目标：将 Strübbe、Stürzl、Egelhaaf 的 MATLAB 数值实验忠实移植为 Debian/Ubuntu 上的独立 C++17 项目。论文 DOI：https://doi.org/10.1371/journal.pone.0128413；MATLAB 资料 DOI：https://doi.org/10.4119/unibi/2736885。
- 原始资料位置：`origin/egomotion_estimation_dense_flow_fields_matlab_scripts/`，永久只读；目录清单为 45 个 `.m`、582 个 `.mat` 和一个原始 `readme`。
- 仅使用 C++17、CMake、Eigen 3、matio/zlib；MAT v7.3/HDF5 仅在真实样本证明需要时讨论。不开发 NPZ/NumPy/Python、MATLAB Runtime、ROS、ESKF、视觉 VO 前端。
- 实验范围：非球面和球面两类 Bias；Adaptive 初始化、带/不带噪声比较、不同更新率；四个数据分析入口和三份插图。

## 2. 干系人、用户与典型用例

| 角色 | 主要需要 |
|---|---|
| 项目所有者/需求批准人 | 忠实算法、结果可复现、批准变更/性能与容差基线 |
| 开发者/AI Agent | 明确范围、输入输出与数据安全边界、可审查的小任务 |
| 数值复现与验收人员 | 用原始 MAT 验证每轮计算、统计及失败记录 |
| 维护者/版权审查人员 | Debian/Ubuntu 构建证据、第三方权利、Issue/PR 可追溯 |

| 用例 ID | 使用场景 | 正常结果 | 异常结果 |
|---|---|---|---|
| UC-01 | Debian/Ubuntu 源外构建 | 得到独立 CLI | 缺依赖时明确报错 |
| UC-02 | inspect MAT | 不修改源数据地显示变量及形状 | 损坏、变量不存在、类型不支持时明确失败 |
| UC-03 | 运行两类 Bias | 新 RunId 下写出兼容 MAT | 不覆盖历史、不吞掉失败 |
| UC-04 | 运行 Adaptive 三类实验 | 各输出及参数与 MATLAB 一致 | 退化或失败样本如实记录 |
| UC-05 | 分析历史 MAT 与绘图 | 与求解器隔离地产生统计和图示 | 发现历史工件差异时标注待归因 |
| UC-06 | 审核需求和复现证据 | 从来源追踪 REQ、AC、TC、P、提交与日志 | 未执行不得标为已通过 |

## 3. 需求编号、优先级、状态

- `REQ-F-*` 为功能，`REQ-D-*` 为来源/数据，`REQ-I-*` 为接口，`REQ-NF-*` 为非功能需求；对应 `AC-*` 是验收条件，`TC-*` 是计划中的测试 ID（**尚未意味着已写测试或通过 CTest**）。
- 优先级 M=必须、S=应该、C=可选、W=本期不做。优先级不等于实施顺序或验证状态。
- 依据状态如“确定”“抽样确认”“待全量审计”“待版权核查”“设计待冻结”“TBD”；所有要求的当前**实施/测试状态均为未实现/未运行**。
- 需求冻结需项目所有者批准具体来源树 SHA、MAT 字典、适用平台、数值容差、性能与版权未决清单，未决项不可假称通过。

## 4. 编号需求及逐项验收条件

### 功能需求

| ID | 优先级 | 依据 | 必须具备的行为 | 验收条件 ID 与内容 |
|---|---|---|---|---|
| REQ-F-CLI-001 | M | 确定 | 系统应提供 MAT 检查、实验生成和独立结果分析的 CLI 子命令。 | AC-F-CLI-001：验证各命令帮助、参数、正常退出及输入错误的非零退出码。 |
| REQ-F-MAT-001 | M | 抽样确认 | 系统应以 matio/zlib 直接读取原始 MAT v5 压缩文件，不依赖 MATLAB/Python。 | AC-F-MAT-001：对源 MAT 抽样及全部归档进行读取、元数据与数值核验。 |
| REQ-F-MAT-002 | M | 待全量审计 | 系统应枚举 MAT 变量名、类型、rank、dims、复杂数属性并明确拒绝未支持的类型。 | AC-F-MAT-002：数据字典逐文件核验，失败明确可定位。 |
| REQ-F-MAT-003 | M | 确定 | 系统应分别提供指定变量名的严格读取和 getflowfield 兼容的首变量/省略扩展名读取。 | AC-F-MAT-003：分别测试单变量、多变量、缺失变量、后缀处理。 |
| REQ-F-MAT-004 | M | 确定 | 系统应在独立 RunId 输出目录写入变量名与尺寸兼容的 MAT 结果。 | AC-F-MAT-004：写后回读一致，拒绝覆盖原归档/历史 RunId。 |
| REQ-F-GEOM-001 | M | 确定 | 系统应以 N×6 数据表示单位视线和三维切向光流，正确执行原叉积与点积。 | AC-F-GEOM-001：固定矩阵和逐元素计算比较，形状错误可检测。 |
| REQ-F-GEOM-002 | M | 确定 | 系统应按原递归顺序生成 8×4^i 全八面体与 6×4^i 缺损八面体视线。 | AC-F-GEOM-002：小规模样本、面集合、单位范数和顺序一致。 |
| REQ-F-GEOM-003 | M | 确定 | 系统应按原顶点/面表生成 20×4^i 二十面体视线。 | AC-F-GEOM-003：小规模逐元素顺序和数量一致。 |
| REQ-F-FLOW-001 | M | 确定 | 系统应按原符号约定计算平移/旋转及其合成切向光流与平均模长。 | AC-F-FLOW-001：固定输入的三维流场、切向残差、统计量一致。 |
| REQ-F-FLOW-002 | M | 确定 | 系统应使用原距离范围 [1,4]、归一化和随机近度方式生成 Bias 场景。 | AC-F-FLOW-002：固定随机序列下各步距离与近度可比较。 |
| REQ-F-FLOW-003 | M | 确定 | 系统应区分平均流长尺度高斯扰动和逐分量高斯扰动，并投影回切平面。 | AC-F-FLOW-003：各模型用固定 Gaussian 序列逐项核对。 |
| REQ-F-FLOW-004 | M | 确定 | 系统应计算单位球内旋转视线射线交点的近度。 | AC-F-FLOW-004：球心、一般位置、边界与非法输入的数值及状态可核验。 |
| REQ-F-FILTER-001 | M | 确定 | 系统应忠实实现 Franz 三轴匹配滤波的平移、旋转模板响应与归一化。 | AC-F-FILTER-001：逐点响应与最终运动结果固定输入对照。 |
| REQ-F-KVD-001 | M | 确定 | 系统应提供独立 Original KvD 和 Modified KvD 入口，保留各自不同的平移更新公式。 | AC-F-KVD-001：两个版本的 near、t、ω 逐轮对照、差异可定位。 |
| REQ-F-KVD-002 | M | 确定 | 系统应保留 KvD 初始化、更新先后及至少 6 次、最多 200 次和动态/绝对阈值退出规则。 | AC-F-KVD-002：覆盖提前停止、最低次数、上限与异常输入。 |
| REQ-F-ERROR-001 | M | 确定 | 系统应分别实现尺度对齐向量误差与方向角度误差，不能用单一误差代替。 | AC-F-ERROR-001：给定相同真值和估计，两个指标与原脚本各自一致。 |
| REQ-F-BIAS-001 | M | 确定 | 系统应复现非球形视场的 Bias 实验及三组噪声乘数 [0.6,2.4,9.6]。 | AC-F-BIAS-001：输出 xwerte、ywerte1…6 的变量和尺寸一致。 |
| REQ-F-BIAS-002 | M | 确定 | 系统应复现完整球面、逐分量比例噪声的 Bias 实验。 | AC-F-BIAS-002：与非球形场景的采样和噪声存在预期差异。 |
| REQ-F-BIAS-003 | M | 确定 | 系统应支持汇总两种 Bias 实验各 40 份独立运行结果及理论参考曲线。 | AC-F-BIAS-003：跨运行均值及 100/sqrt(N) 曲线与原分析脚本一致。 |
| REQ-F-TRAJ-001 | M | 确定 | 系统应按原参数生成圆轨迹、正弦轨迹和相邻点局部运动/旋转，不将绘图混入数据生成。 | AC-F-TRAJ-001：样本轨迹、旋转方向、端点和角度转换对照。 |
| REQ-F-SH-001 | M | 确定 | 系统应复现 l≤2 的实球谐基函数、3×5 系数、近度恢复与逐项拟合。 | AC-F-SH-001：正负 m、归一化、有效系数位置与数值对照。 |
| REQ-F-SH-002 | M | 待版权核查 | 系统应复现 l≤2 球谐旋转矩阵与系数旋转，保持原基函数顺序及旋转方向。 | AC-F-SH-002：3×3、5×5 旋转块固定样本对照；权利来源先核实。 |
| REQ-F-ADAPT-001 | M | 确定 | 系统应实现 Adaptive MFA 单步 6×6 系统及其平移、旋转、残差、近度和新球谐系数。 | AC-F-ADAPT-001：按阶段比较左右端、运动解、残差及新系数。 |
| REQ-F-ADAPT-002 | M | 确定 | 系统应在连续轨迹中按原顺序旋转系数、计算并提交主状态，同时支持独立对比状态。 | AC-F-ADAPT-002：固定逐帧输入检验更新顺序、运行间隔离。 |
| REQ-F-ADAPT-003 | M | 确定 | 系统应支持每帧重置、继承、1/5/11/21 帧周期刷新策略。 | AC-F-ADAPT-003：固定帧号验证正确触发且不污染主系数。 |
| REQ-F-EXP-001 | M | 确定 | 系统应复现 35 点圆形初始化实验，保存四类方向误差结果。 | AC-F-EXP-001：参数、数组形状、索引与历史 MAT 比较。 |
| REQ-F-EXP-002 | M | 确定 | 系统应复现带/不带噪声 600 点正弦轨迹对比，保留八次运行与原输出。 | AC-F-EXP-002：分情形记录全部运行结果、不忽略失败和缺失。 |
| REQ-F-EXP-003 | M | 确定 | 系统应复现多更新频率实验，并显式登记历史 MAT 与当前脚本输出不一致项。 | AC-F-EXP-003：按当前入口输出与历史独有字段列差异表。 |
| REQ-F-AN-001 | M | 确定 | 系统应复现原 Adaptive 初始化与两种轨迹分析的曲线数值、分段阈值和窗口平滑。 | AC-F-AN-001：阈值 0.01/0.02、窗口 2/5/10 与各图索引核验。 |
| REQ-F-AN-002 | S | 确定 | 系统应将绘图与数值分析隔离，并支持原三份 illustration 的可比较展示。 | AC-F-AN-002：无绘图依赖下数值命令可运行；图示所据数据一致。 |

### 数据需求

| ID | 优先级 | 依据 | 必须具备的行为 | 验收条件 ID 与内容 |
|---|---|---|---|---|
| REQ-D-SOURCE-001 | M | 目录已核对 | 系统应冻结原始资料的 SHA、45 个 .m、582 个 .mat、readme 和具体文件分类。 | AC-D-SOURCE-001：源码清单与 GitHub 树一致，每项可定位哈希。 |
| REQ-D-SOURCE-002 | M | 待权利核查 | 系统应记录原作者及外部 rodrigues/SHRotate 的实际来源和可再分发范围。 | AC-D-SOURCE-002：核对文件版权头；不擅自补造 LICENSE 结论。 |
| REQ-D-MAT-001 | M | 待全量审计 | 系统应为 582 个 MAT 建立完整的变量、压缩、类型、维度、哈希、兼容状态字典。 | AC-D-MAT-001：所有文件经 matio 解包核验，并列明失败。 |
| REQ-D-MAT-002 | M | 确定 | 系统应保留源 MAT 的列主序、数据类型、变量命名、数值含义和输出形状。 | AC-D-MAT-002：已读字段按元素及真实 shape 比较，变更有记录。 |
| REQ-D-MAT-003 | M | 历史差异待归因 | 系统应区分生成脚本实际保存、分析脚本需要和历史归档额外存在的 MAT 变量。 | AC-D-MAT-003：所有差异有状态和证据，不伪造缺失数据。 |

### 接口需求

| ID | 优先级 | 依据 | 必须具备的行为 | 验收条件 ID 与内容 |
|---|---|---|---|---|
| REQ-I-DOMAIN-001 | M | 设计待冻结 | 系统领域合同应表达流场、运动、球谐系数的语义、维度和显式失败状态。 | AC-I-DOMAIN-001：P0 冻结输入/输出合同，非法形状报错。 |
| REQ-I-IO-001 | M | 设计待冻结 | 系统应使用资源安全的 MAT 读取及独立写入接口，领域计算不直接依赖 matio 指针。 | AC-I-IO-001：RAII/内存寿命、只读/写入权限、替身可验证。 |
| REQ-I-RNG-001 | M | 设计待冻结 | 系统应支持显式 RNG 注入、固定序列替身和逐次调用顺序记录。 | AC-I-RNG-001：同固定序列不改变数学实现即可复现。 |
| REQ-I-NOISE-001 | M | 设计待冻结 | 系统应允许通过窄接口选择采样器、噪声模型和无噪声策略。 | AC-I-NOISE-001：替换实现不修改核心求解器。 |
| REQ-I-KVD-001 | M | 设计待冻结 | 系统应保持两个 KvD 入口和差异化更新规则，不让公共引擎覆盖原始计算顺序。 | AC-I-KVD-001：策略替换对照两条原 MATLAB 路径。 |
| REQ-I-ADAPT-001 | M | 设计待冻结 | 系统应将无状态单步求解与有状态 Session/更新策略分离。 | AC-I-ADAPT-001：单步无 Session 可测，两个 Session 无状态串扰。 |
| REQ-I-EXPERIMENT-001 | M | 设计待冻结 | 系统实验运行器应通过组合注入求解、RNG、采样和写入器，彼此独立。 | AC-I-EXPERIMENT-001：固定替身运行两种 Bias 模式无交叉状态。 |
| REQ-I-ANALYSIS-001 | S | 设计待冻结 | 系统应将 MAT 读取、分析、可选绘图分离，禁止图表后端改变数据。 | AC-I-ANALYSIS-001：禁用绘图时分析值相同。 |

### 非功能需求

| ID | 优先级 | 依据 | 必须具备的行为 | 验收条件 ID 与内容 |
|---|---|---|---|---|
| REQ-NF-PLATFORM-001 | M | 确定 | 系统应在 Debian/Ubuntu 上以 C++17/CMake 源外构建。 | AC-NF-PLATFORM-001：实际支持的平台/编译器构建并留命令与日志。 |
| REQ-NF-DEPS-001 | M | 确定 | 系统核心运行不得依赖 Python、MATLAB Runtime、NumPy、ROS、OpenCV、Windows 或 NPZ 转换。 | AC-NF-DEPS-001：检查依赖和链接，源代码无禁用路径。 |
| REQ-NF-SAFETY-001 | M | 确定 | 系统应保证 origin/** 所有原文件只读且拒绝覆盖历史 RunId。 | AC-NF-SAFETY-001：前后哈希一致、重复输出默认拒绝。 |
| REQ-NF-NUM-001 | M | 候选容差 | 系统应保留 MATLAB 原公式/失败状态并提供固定输入数值对照与分模块容差。 | AC-NF-NUM-001：double 默认、1e-10 仅作候选相对容差，最终阈值须实测批准。 |
| REQ-NF-NUM-002 | M | 确定 | 系统应显式报告零范数、奇异、非有限、文件损坏、未定义角度；不可误记成功。 | AC-NF-NUM-002：每类失败有非成功状态及对应日志。 |
| REQ-NF-TRACE-001 | M | 确定 | 系统每项正式需求应关联 AC、TC、来源和 P 子任务；所有未运行的测试保持未通过。 | AC-NF-TRACE-001：追踪表覆盖全部 98 个 P 子任务，实际状态与提交一致。 |
| REQ-NF-RNG-001 | M | 确定 | 系统应将固定样本等价验证与随机统计复现分离，并独立记录各 RunId 种子。 | AC-NF-RNG-001：固定序列可重复，统计含有效/失败数量。 |
| REQ-NF-LOG-001 | M | 确定 | 系统应记录 RunId、SHA、环境、命令、参数、随机种子、输入身份、退出码及日志位置。 | AC-NF-LOG-001：每次正式运行记录字段完整，不覆盖失败证据。 |
| REQ-NF-OOP-001 | M | 确定 | 系统应明确所有权，隔离算法公式与实验编排，不创建巨型类或错误的通用求解器。 | AC-NF-OOP-001：对象模型和独立测试核实状态、资源与依赖方向。 |
| REQ-NF-HEADLESS-001 | M | 确定 | 系统应支持无图形环境的检查、运行和分析，绘图模块不得阻断数值。 | AC-NF-HEADLESS-001：禁用绘图时主要数据路径仍可执行。 |
| REQ-NF-RECOVERY-001 | M | 确定 | 系统应保留错误与失败 RunId 原始记录，不覆盖已有结果并可说明原因。 | AC-NF-RECOVERY-001：错误退出日志可追溯，历史未受影响。 |
| REQ-NF-PORTABLE-001 | S | 确定 | 系统应在 Debian/Ubuntu 支持含空格和中文的输入输出路径。 | AC-NF-PORTABLE-001：实际 Unicode/空格路径可读写。 |
| REQ-NF-RUNTIME-001 | S | TBD | 系统应采集完整实验的墙钟、CPU、峰值内存和输入规模，正式运行上限需实测后批准。 | AC-NF-RUNTIME-001：先记录基线，未冻结性能门限不得标记通过。 |
| REQ-NF-THREAD-001 | S | 待实测 | 系统应隔离不同实验上下文的随机状态和会话系数；暂不承诺单个会话线程安全。 | AC-NF-THREAD-001：交替运行两个会话结果互不影响。 |
| REQ-NF-COPYRIGHT-001 | M | 待权利核查 | 系统应保留两份 DOI、第三方来源，不擅自新增项目级 LICENSE。 | AC-NF-COPYRIGHT-001：README 署名可核对，第三方权利障碍记录。 |

## 5. MAT 数据字典与结果契约

- `setofpoints` 是 N×6（前三列单位视线，后三列三维切向光流）；系数布局为 3×5（1+3+5 个有效项）；Adaptive 线性系统为 6×6。MATLAB 的列主序、1-based 索引意义、数值精度不能被静默改变。
- 目录结构事实：`occluded_regions` 40×7=280、`proportional_error` 40×7=280；Adaptive 的 initialization 4、without_errors 5、with_errors 7、many_parameter 6；总计 582 个文件。**完整 MAT 变量字典还未使用 matio 全量解包**。
- 抽样 MAT 文件头提示 MATLAB 5.0 MAT-file 和 miCOMPRESSED，当前需 zlib；此结果不足以断言全部 582 文件同一格式。
- Bias 的 xwerte、ywerte1…6 和 Adaptive 的 allemit*/alleohne*/rottransratio 等应以实际每个文件的变量名、shape 为准。历史归档可能与当前生成脚本输出不一致，须分别登记“当前写出、已有历史、分析需要、来源未明”。
- 噪声参数 `[0.6, 2.4, 9.6]` 在原代码中直接乘以 randn，不能因为注释写 variance 而自行开平方。

## 6. 外部接口与设计分离

| 外部接口 | 输入 | 输出/异常 |
|---|---|---|
| 命令行 | 实验种类、参数、原始数据路径、RunId | 运行结果或结构化失败诊断 |
| MAT 读取 | 路径、严格变量名或兼容模式 | 数组、元数据或明确错误；始终只读原始数据 |
| MAT 结果写入 | 与 origin 不重叠的输出路径、变量名、数组 | 可回读的 MAT 或拒绝覆盖/越界 |
| RNG/采样/噪声 | 种子或固定随机序列、采样级别、模型 | 有明确顺序的流场或失败状态 |
| Franz、KvD、Adaptive | 流场、近度/系数、原参数 | 运动估计、残差、迭代和非成功状态 |
| 分析/绘图 | 历史或新生成 MAT | 统计、曲线、可选图示；不反向改动求解结果 |

**需求与设计不混淆**：PLAN 中的 RAII、FlowField、StepSolver、Session、策略类和组合根是当前拟议实现方式；如果内部设计变更却仍满足这里的输出与验收约束，不应无故改写外部需求。

## 7. 可量化质量条件与待定目标

| 方面 | 已具备的可验收边界 | 不能凭空确定的目标 |
|---|---|---|
| 兼容环境 | 仅 Debian/Ubuntu，源外 CMake/C++17，实际构建记录工具链版本 | 受支持发行版/编译器的完整版本矩阵待实测 |
| 原始数据 | 582 份原始 MAT 均不可修改，已存在结果默认不可覆盖 | 不承诺未经设计的自动恢复 SLA |
| 数值精度 | 默认 double；固定输入记录最大绝对/相对误差、失败/无定义次数 | 1e-10 仅为候选相对容差；分模块正式门限需测量批准 |
| 测试与追踪 | 所有 M 需求有 AC、TC、至少一个 P 子任务；实测通过才算完成 | 行覆盖率、分支覆盖率百分比未获批准，不虚构 |
| 性能/内存 | 记录原实验墙钟、CPU、峰值 RSS、输入规模和运行平台 | 速度上限、加速倍数、内存峰值、构建时间预算均 TBD |
| 日志/恢复 | 留存 RunId、SHA、参数、环境、输入身份、退出码、stderr、历史失败证据 | 不擅自增加自动重试频率和保留天数 |
| 并发 | 每个 RunContext 独立 RNG/系数/结果路径 | 同一 Session 多线程安全非本期承诺 |
| 无界面运行 | 不要求 Python 或绘图环境便能 MAT 检查、实验与数据分析 | 图像像素级复刻不是本期强制要求 |

## 8. 假设、依赖、阻塞与决策人

| ID | 待决问题 | 状态与决定方 |
|---|---|---|
| DEC-001 | 所有 MAT 是否均为 v5 压缩；是否需要 HDF5 | 仅抽样；由 P0/P1 审计后交项目所有者批准 |
| DEC-002 | MAT 内各变量的真实数据类型、尺寸、稀疏/复数边界 | 待 582 文件全量数据字典 |
| DEC-003 | rodrigues.m 和 SHRotate.m 中第三方代码许可/可再分发范围 | 待项目所有者及权利审查 |
| DEC-004 | 历史 MAT 有而当前源脚本未保存的字段 | 已观察，待出处调查与处理决定 |
| DEC-005 | 固定输入容差及统计噪声验收方法 | 有候选值，待数值验证人员与项目所有者冻结 |
| DEC-006 | Debian/Ubuntu 版本范围、CPU/内存与构建时间预算 | TBD，先做基线实测再批准 |

## 9. 需求追踪矩阵：MATLAB 来源 → REQ → PLAN → TC/AC

以下源文件名以 `origin/egomotion_estimation_dense_flow_fields_matlab_scripts/` 为基准；实际 C++ 类/文件及测试位置后续实施后补入。TC 均为**规划标识**，不是已运行的证据。

| 来源（原 MATLAB/规则） | 需求 | PLAN 子任务 | 测试 ID | 验收 ID | 状态 |
|---|---|---|---|---|---|
| MATLAB five generation entries | REQ-F-CLI-001 | P1.2, P5.1, P5.2, P8.1, P8.2, P8.5, P9.6 | TC-F-CLI-001 | AC-F-CLI-001 | 未实现/未运行 |
| 582 MAT files | REQ-F-MAT-001 | P1.3, P1.4, P1.8, P1.9 | TC-F-MAT-001 | AC-F-MAT-001 | 未实现/未运行 |
| MAT archive | REQ-F-MAT-002 | P0.5, P1.5 | TC-F-MAT-002 | AC-F-MAT-002 | 未实现/未运行 |
| help_functions/getflowfield.m | REQ-F-MAT-003 | P1.6 | TC-F-MAT-003 | AC-F-MAT-003 | 未实现/未运行 |
| data_generation/* | REQ-F-MAT-004 | P1.7, P1.9 | TC-F-MAT-004 | AC-F-MAT-004 | 未实现/未运行 |
| skalarprodukt.m; kreuzprodukt.m; erzeugeflussausnear.m | REQ-F-GEOM-001 | P2.1, P2.2, P2.8 | TC-F-GEOM-001 | AC-F-GEOM-001 | 未实现/未运行 |
| findeDreiecke.m; erzeugerichtungsvektoren*.m | REQ-F-GEOM-002 | P2.3, P2.4, P2.5 | TC-F-GEOM-002 | AC-F-GEOM-002 | 未实现/未运行 |
| richtungsvektoren_icosahedron.m | REQ-F-GEOM-003 | P2.6, P2.9 | TC-F-GEOM-003 | AC-F-GEOM-003 | 未实现/未运行 |
| erzeugeflussausnear.m | REQ-F-FLOW-001 | P3.1 | TC-F-FLOW-001 | AC-F-FLOW-001 | 未实现/未运行 |
| fixed_*_randomnear.m | REQ-F-FLOW-002 | P3.2 | TC-F-FLOW-002 | AC-F-FLOW-002 | 未实现/未运行 |
| nonpropfehler.m; fixed_*_randomnear.m | REQ-F-FLOW-003 | P3.3, P3.4, P3.5, P3.9 | TC-F-FLOW-003 | AC-F-FLOW-003 | 未实现/未运行 |
| getnear_in_kugel_Drehung.m | REQ-F-FLOW-004 | P3.6 | TC-F-FLOW-004 | AC-F-FLOW-004 | 未实现/未运行 |
| zellendefinieren.m; franzeigen.m | REQ-F-FILTER-001 | P4.1, P4.2, P4.10 | TC-F-FILTER-001 | AC-F-FILTER-001 | 未实现/未运行 |
| norm_koenderink_iteration.m; mue_norm_koenderink_iteration.m | REQ-F-KVD-001 | P4.3, P4.4, P4.5, P4.9 | TC-F-KVD-001 | AC-F-KVD-001 | 未实现/未运行 |
| two KvD .m files | REQ-F-KVD-002 | P4.6, P4.8 | TC-F-KVD-002 | AC-F-KVD-002 | 未实现/未运行 |
| transfehler_div.m; normrelfehler.m | REQ-F-ERROR-001 | P4.7 | TC-F-ERROR-001 | AC-F-ERROR-001 | 未实现/未运行 |
| bias_test_nonspherical.m | REQ-F-BIAS-001 | P5.1, P5.3, P5.4, P5.7 | TC-F-BIAS-001 | AC-F-BIAS-001 | 未实现/未运行 |
| bias_test_spherical_prop_error.m | REQ-F-BIAS-002 | P5.2 | TC-F-BIAS-002 | AC-F-BIAS-002 | 未实现/未运行 |
| graphs_bias_test.m; 560 MAT | REQ-F-BIAS-003 | P5.5, P5.6, P9.4 | TC-F-BIAS-003 | AC-F-BIAS-003 | 未实现/未运行 |
| kreis_trajektorie.m; sinuskurve_trajektorie.m; trajektorie_aus_punkten.m | REQ-F-TRAJ-001 | P6.1, P6.2, P6.3, P6.4, P6.5, P6.6, P6.7 | TC-F-TRAJ-001 | AC-F-TRAJ-001 | 未实现/未运行 |
| kugelflfunc2.m; near_aus_koeff.m; koeff_aus_near.m | REQ-F-SH-001 | P7.1, P7.2, P7.3, P7.4, P7.10 | TC-F-SH-001 | AC-F-SH-001 | 未实现/未运行 |
| SHRotate.m; transfor_koeff.m | REQ-F-SH-002 | P7.5, P7.6 | TC-F-SH-002 | AC-F-SH-002 | 未实现/未运行 |
| oneit_adaptiv_matched_filter.m | REQ-F-ADAPT-001 | P7.7, P7.8, P7.9, P7.11 | TC-F-ADAPT-001 | AC-F-ADAPT-001 | 未实现/未运行 |
| adaptive_matched_filter_test/data_generation/* | REQ-F-ADAPT-002 | P8.3, P8.8, P8.9 | TC-F-ADAPT-002 | AC-F-ADAPT-002 | 未实现/未运行 |
| agent_in_sphere_adaptive_matched_filter_many_parameter.m | REQ-F-ADAPT-003 | P8.5, P8.6 | TC-F-ADAPT-003 | AC-F-ADAPT-003 | 未实现/未运行 |
| agent_in_sphere_initialization_test.m | REQ-F-EXP-001 | P8.1 | TC-F-EXP-001 | AC-F-EXP-001 | 未实现/未运行 |
| agent_in_sphere_adaptive_matched_filter_test.m | REQ-F-EXP-002 | P8.2, P8.4 | TC-F-EXP-002 | AC-F-EXP-002 | 未实现/未运行 |
| agent_in_sphere_adaptive_matched_filter_many_parameter.m | REQ-F-EXP-003 | P8.5, P8.6, P8.7 | TC-F-EXP-003 | AC-F-EXP-003 | 未实现/未运行 |
| adaptive_matched_filter_test/data_analysis/*.m | REQ-F-AN-001 | P9.1, P9.2, P9.3, P9.5 | TC-F-AN-001 | AC-F-AN-001 | 未实现/未运行 |
| illustrations/*.m | REQ-F-AN-002 | P9.6, P9.7, P9.8 | TC-F-AN-002 | AC-F-AN-002 | 未实现/未运行 |
| origin/** | REQ-D-SOURCE-001 | P0.1, P0.2, P0.3 | TC-D-SOURCE-001 | AC-D-SOURCE-001 | 未实现/未运行 |
| rodrigues.m; SHRotate.m | REQ-D-SOURCE-002 | P0.4, P10.7 | TC-D-SOURCE-002 | AC-D-SOURCE-002 | 未实现/未运行 |
| origin/**/produced_data/** | REQ-D-MAT-001 | P0.5, P0.6, P1.8, P10.4 | TC-D-MAT-001 | AC-D-MAT-001 | 未实现/未运行 |
| getflowfield.m; save calls | REQ-D-MAT-002 | P1.4, P2.8, P5.4 | TC-D-MAT-002 | AC-D-MAT-002 | 未实现/未运行 |
| with_errors; many_parameter | REQ-D-MAT-003 | P8.7, P10.4 | TC-D-MAT-003 | AC-D-MAT-003 | 未实现/未运行 |
| PLAN §3 | REQ-I-DOMAIN-001 | P0.7, P0.8, P2.1 | TC-I-DOMAIN-001 | AC-I-DOMAIN-001 | 未实现/未运行 |
| PLAN §3 | REQ-I-IO-001 | P1.3, P1.9, P1.10 | TC-I-IO-001 | AC-I-IO-001 | 未实现/未运行 |
| getvektor.m; RandStream | REQ-I-RNG-001 | P2.7, P3.7, P3.8 | TC-I-RNG-001 | AC-I-RNG-001 | 未实现/未运行 |
| sampling/noise .m | REQ-I-NOISE-001 | P2.9, P3.9 | TC-I-NOISE-001 | AC-I-NOISE-001 | 未实现/未运行 |
| two KvD solvers | REQ-I-KVD-001 | P4.9 | TC-I-KVD-001 | AC-I-KVD-001 | 未实现/未运行 |
| oneit*.m; adaptive generation | REQ-I-ADAPT-001 | P7.11, P8.8, P8.9 | TC-I-ADAPT-001 | AC-I-ADAPT-001 | 未实现/未运行 |
| PLAN §3 | REQ-I-EXPERIMENT-001 | P1.2, P5.7 | TC-I-EXPERIMENT-001 | AC-I-EXPERIMENT-001 | 未实现/未运行 |
| data_analysis/*.m | REQ-I-ANALYSIS-001 | P9.8 | TC-I-ANALYSIS-001 | AC-I-ANALYSIS-001 | 未实现/未运行 |
| AGENTS.md | REQ-NF-PLATFORM-001 | P1.1, P10.6 | TC-NF-PLATFORM-001 | AC-NF-PLATFORM-001 | 未实现/未运行 |
| AGENTS.md | REQ-NF-DEPS-001 | P1.1, P10.6 | TC-NF-DEPS-001 | AC-NF-DEPS-001 | 未实现/未运行 |
| AGENTS.md | REQ-NF-SAFETY-001 | P1.3, P1.7, P1.9 | TC-NF-SAFETY-001 | AC-NF-SAFETY-001 | 未实现/未运行 |
| PLAN §7 | REQ-NF-NUM-001 | P4.3, P4.8, P7.9, P10.3, P10.5 | TC-NF-NUM-001 | AC-NF-NUM-001 | 未实现/未运行 |
| AGENTS.md | REQ-NF-NUM-002 | P4.7, P7.9, P10.3 | TC-NF-NUM-002 | AC-NF-NUM-002 | 未实现/未运行 |
| AGENTS.md; PLAN.md | REQ-NF-TRACE-001 | P0.6, P10.1, P10.2, P10.3, P10.8, P10.9, P10.10 | TC-NF-TRACE-001 | AC-NF-TRACE-001 | 未实现/未运行 |
| RandStream usage | REQ-NF-RNG-001 | P3.7, P5.3, P10.5 | TC-NF-RNG-001 | AC-NF-RNG-001 | 未实现/未运行 |
| AGENTS.md | REQ-NF-LOG-001 | P5.3, P8.9, P10.6 | TC-NF-LOG-001 | AC-NF-LOG-001 | 未实现/未运行 |
| PLAN §3.3–3.6 | REQ-NF-OOP-001 | P0.7, P0.8, P10.9, P10.10 | TC-NF-OOP-001 | AC-NF-OOP-001 | 未实现/未运行 |
| PLAN §9 | REQ-NF-HEADLESS-001 | P9.6, P10.6 | TC-NF-HEADLESS-001 | AC-NF-HEADLESS-001 | 未实现/未运行 |
| AGENTS.md | REQ-NF-RECOVERY-001 | P5.3, P10.3 | TC-NF-RECOVERY-001 | AC-NF-RECOVERY-001 | 未实现/未运行 |
| AGENTS.md | REQ-NF-PORTABLE-001 | P1.2, P10.6 | TC-NF-PORTABLE-001 | AC-NF-PORTABLE-001 | 未实现/未运行 |
| PLAN; experiments | REQ-NF-RUNTIME-001 | P1.8, P10.6 | TC-NF-RUNTIME-001 | AC-NF-RUNTIME-001 | 未实现/未运行 |
| PLAN §3 | REQ-NF-THREAD-001 | P8.9, P10.9 | TC-NF-THREAD-001 | AC-NF-THREAD-001 | 未实现/未运行 |
| README.md; original headers | REQ-NF-COPYRIGHT-001 | P0.4, P10.1, P10.7 | TC-NF-COPYRIGHT-001 | AC-NF-COPYRIGHT-001 | 未实现/未运行 |

### 9.1 反向追踪：全部 P 子任务 → 需求

本表覆盖原计划 P0–P10 的全部 **98** 项子任务；新子任务必须同步登记需求映射。

| P 子任务 | 关联需求 |
|---|---|
| P0.1 | REQ-D-SOURCE-001 |
| P0.2 | REQ-D-SOURCE-001 |
| P0.3 | REQ-D-SOURCE-001 |
| P0.4 | REQ-D-SOURCE-002, REQ-NF-COPYRIGHT-001 |
| P0.5 | REQ-F-MAT-002, REQ-D-MAT-001 |
| P0.6 | REQ-D-MAT-001, REQ-NF-TRACE-001 |
| P0.7 | REQ-I-DOMAIN-001, REQ-NF-OOP-001 |
| P0.8 | REQ-I-DOMAIN-001, REQ-NF-OOP-001 |
| P1.1 | REQ-NF-PLATFORM-001, REQ-NF-DEPS-001 |
| P1.2 | REQ-F-CLI-001, REQ-I-EXPERIMENT-001, REQ-NF-PORTABLE-001 |
| P1.3 | REQ-F-MAT-001, REQ-I-IO-001, REQ-NF-SAFETY-001 |
| P1.4 | REQ-F-MAT-001, REQ-D-MAT-002 |
| P1.5 | REQ-F-MAT-002 |
| P1.6 | REQ-F-MAT-003 |
| P1.7 | REQ-F-MAT-004, REQ-NF-SAFETY-001 |
| P1.8 | REQ-F-MAT-001, REQ-D-MAT-001, REQ-NF-RUNTIME-001 |
| P1.9 | REQ-F-MAT-001, REQ-F-MAT-004, REQ-I-IO-001, REQ-NF-SAFETY-001 |
| P1.10 | REQ-I-IO-001 |
| P2.1 | REQ-F-GEOM-001, REQ-I-DOMAIN-001 |
| P2.2 | REQ-F-GEOM-001 |
| P2.3 | REQ-F-GEOM-002 |
| P2.4 | REQ-F-GEOM-002 |
| P2.5 | REQ-F-GEOM-002 |
| P2.6 | REQ-F-GEOM-003 |
| P2.7 | REQ-I-RNG-001 |
| P2.8 | REQ-F-GEOM-001, REQ-D-MAT-002 |
| P2.9 | REQ-F-GEOM-003, REQ-I-NOISE-001 |
| P3.1 | REQ-F-FLOW-001 |
| P3.2 | REQ-F-FLOW-002 |
| P3.3 | REQ-F-FLOW-003 |
| P3.4 | REQ-F-FLOW-003 |
| P3.5 | REQ-F-FLOW-003 |
| P3.6 | REQ-F-FLOW-004 |
| P3.7 | REQ-I-RNG-001, REQ-NF-RNG-001 |
| P3.8 | REQ-I-RNG-001 |
| P3.9 | REQ-F-FLOW-003, REQ-I-NOISE-001 |
| P4.1 | REQ-F-FILTER-001 |
| P4.2 | REQ-F-FILTER-001 |
| P4.3 | REQ-F-KVD-001, REQ-NF-NUM-001 |
| P4.4 | REQ-F-KVD-001 |
| P4.5 | REQ-F-KVD-001 |
| P4.6 | REQ-F-KVD-002 |
| P4.7 | REQ-F-ERROR-001, REQ-NF-NUM-002 |
| P4.8 | REQ-F-KVD-002, REQ-NF-NUM-001 |
| P4.9 | REQ-F-KVD-001, REQ-I-KVD-001 |
| P4.10 | REQ-F-FILTER-001 |
| P5.1 | REQ-F-CLI-001, REQ-F-BIAS-001 |
| P5.2 | REQ-F-CLI-001, REQ-F-BIAS-002 |
| P5.3 | REQ-F-BIAS-001, REQ-NF-RNG-001, REQ-NF-LOG-001, REQ-NF-RECOVERY-001 |
| P5.4 | REQ-F-BIAS-001, REQ-D-MAT-002 |
| P5.5 | REQ-F-BIAS-003 |
| P5.6 | REQ-F-BIAS-003 |
| P5.7 | REQ-F-BIAS-001, REQ-I-EXPERIMENT-001 |
| P6.1 | REQ-F-TRAJ-001 |
| P6.2 | REQ-F-TRAJ-001 |
| P6.3 | REQ-F-TRAJ-001 |
| P6.4 | REQ-F-TRAJ-001 |
| P6.5 | REQ-F-TRAJ-001 |
| P6.6 | REQ-F-TRAJ-001 |
| P6.7 | REQ-F-TRAJ-001 |
| P7.1 | REQ-F-SH-001 |
| P7.2 | REQ-F-SH-001 |
| P7.3 | REQ-F-SH-001 |
| P7.4 | REQ-F-SH-001 |
| P7.5 | REQ-F-SH-002 |
| P7.6 | REQ-F-SH-002 |
| P7.7 | REQ-F-ADAPT-001 |
| P7.8 | REQ-F-ADAPT-001 |
| P7.9 | REQ-F-ADAPT-001, REQ-NF-NUM-001, REQ-NF-NUM-002 |
| P7.10 | REQ-F-SH-001 |
| P7.11 | REQ-F-ADAPT-001, REQ-I-ADAPT-001 |
| P8.1 | REQ-F-CLI-001, REQ-F-EXP-001 |
| P8.2 | REQ-F-CLI-001, REQ-F-EXP-002 |
| P8.3 | REQ-F-ADAPT-002 |
| P8.4 | REQ-F-EXP-002 |
| P8.5 | REQ-F-CLI-001, REQ-F-ADAPT-003, REQ-F-EXP-003 |
| P8.6 | REQ-F-ADAPT-003, REQ-F-EXP-003 |
| P8.7 | REQ-F-EXP-003, REQ-D-MAT-003 |
| P8.8 | REQ-F-ADAPT-002, REQ-I-ADAPT-001 |
| P8.9 | REQ-F-ADAPT-002, REQ-I-ADAPT-001, REQ-NF-LOG-001, REQ-NF-THREAD-001 |
| P9.1 | REQ-F-AN-001 |
| P9.2 | REQ-F-AN-001 |
| P9.3 | REQ-F-AN-001 |
| P9.4 | REQ-F-BIAS-003 |
| P9.5 | REQ-F-AN-001 |
| P9.6 | REQ-F-CLI-001, REQ-F-AN-002, REQ-NF-HEADLESS-001 |
| P9.7 | REQ-F-AN-002 |
| P9.8 | REQ-F-AN-002, REQ-I-ANALYSIS-001 |
| P10.1 | REQ-NF-TRACE-001, REQ-NF-COPYRIGHT-001 |
| P10.2 | REQ-NF-TRACE-001 |
| P10.3 | REQ-NF-NUM-001, REQ-NF-NUM-002, REQ-NF-TRACE-001, REQ-NF-RECOVERY-001 |
| P10.4 | REQ-D-MAT-001, REQ-D-MAT-003 |
| P10.5 | REQ-NF-NUM-001, REQ-NF-RNG-001 |
| P10.6 | REQ-NF-PLATFORM-001, REQ-NF-DEPS-001, REQ-NF-LOG-001, REQ-NF-HEADLESS-001, REQ-NF-PORTABLE-001, REQ-NF-RUNTIME-001 |
| P10.7 | REQ-D-SOURCE-002, REQ-NF-COPYRIGHT-001 |
| P10.8 | REQ-NF-TRACE-001 |
| P10.9 | REQ-NF-TRACE-001, REQ-NF-OOP-001, REQ-NF-THREAD-001 |
| P10.10 | REQ-NF-TRACE-001, REQ-NF-OOP-001 |

## 10. 需求变更与基线管理

1. 需求提出者记录变更的 REQ/AC/TC ID、源码依据、问题与期望行为。
2. 审核对源 MATLAB、MAT 字典、C++ 模块、P 任务、数值验收、数据安全、版权及 Debian/Ubuntu 依赖的影响；不直接以更改门限把失败称为通过。
3. 经项目所有者批准后，以新增提交更新 SRS 和相应 PLAN 任务；不得改写 Git 历史或直接改动 origin。
4. 冻结基线时保存源树 SHA、MAT 审计版本、批准的指标、仍然开放的 DEC 项和关联测试证据。

## 11. 术语表

| 术语 | 含义 |
|---|---|
| KvD | Koenderink–van Doorn 估计，原始/修正版平移更新公式不同 |
| MFA / Franz | 匹配滤波自运动估计基线（franzeigen） |
| Adaptive MFA | 利用球谐近度模型和状态更新的匹配滤波估计 |
| setofpoints | N×6，三列视线与三列三维切向光流 |
| near | 近度，通常为距离倒数；归一化以实际 MATLAB 入口为准 |
| akkoeff | 主轨迹球谐系数状态 |
| mitkoeff / ohnekoeff | 继承/定期刷新与每帧重置的对比系数状态 |
| rottransratio | 平均旋转光流模长与平移光流模长之比 |
| Golden MAT | 原作者已保存、只读的参考实验数据 |
| RunId | 每轮独立运行的可追踪标识 |
| SRS | 描述可验收系统义务的需求规格说明书，不等同于实施计划 |

## 12. 当前确认程度

本文档为**需求草案，不是已冻结基线**；源码/目录检查不等于 C++ 功能实现，也不等于 MATLAB/C++ 数值测试通过。下阶段先完成 P0 的 MAT 字典、第三方版权、验收参数核验，再由维护者批准正式需求基线。
