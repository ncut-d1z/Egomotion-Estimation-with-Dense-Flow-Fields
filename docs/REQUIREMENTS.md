# Egomotion Estimation with Dense Flow Fields：软件需求规格说明书（SRS）

> **状态：草案 v0.3，未冻结、未验收。** 本文档规定系统的可验证义务；[PLAN.md](PLAN.md) 负责实现架构、P0–P10 任务及依赖；[AGENTS.md](../AGENTS.md) 负责开发与 Git 安全规则。需求依据被确认不代表程序已实现。

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

- 需求类别：`REQ-F-*` 功能、`REQ-D-*` 数据、`REQ-I-*` 可观察/组件边界接口、`REQ-DC-*` 内部设计约束、`REQ-NF-*` 非功能、`REQ-PROC-*` 项目过程质量。实施工作项仍单独使用 P0–P10，不能把 P ID 当需求。
- 对应 `AC-*` 是验收条件、`TC-*` 是**计划中的测试标识**；实际测试文件/CTest 名见 [TEST_PLAN.md](TEST_PLAN.md)，尚未创建的名称均标为拟议。
- 优先级 M=必须、S=应该、C=可选、W=本期排除。M 项目若有必需的未冻结容差/权限/数据合同，应标注“**M（受 DEC 阻塞）**”并保持验收状态“不可判定/未通过”，不能因此放低优先级，也不能冒称通过。
- “确定/待审计/设计待冻结”等仅说明需求依据状态；**MAT 独立探针已在 Ubuntu 上编译并运行，582/582 文件及变量完整读取，详见 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md)**。这一实测证据不表示完整 MAT Reader/Writer、CLI、CTest 或算法已完成。DEC-001/002 对当前归档的格式与类型事实已有证据、尚待基线审查；DEC-005 数值容差、DEC-008 CLI/CTest/Golden 基线继续待批准；**H-02/H-14 已针对 Sophus 和 Don Williamson 的明确替代方案作出选择，H-04 已给出 Ubuntu 24.04.4 的环境容量快照，但未产生性能阈值或软件测试证据**。详细审批边界见 [审批落实记录](approval/DECISION_IMPLEMENTATION_NOTES.md)。
- **ID 迁移兼容表**：`REQ-I-DOMAIN/IO/RNG/NOISE/KVD/ADAPT/EXPERIMENT/ANALYSIS-001` → 对应 `REQ-DC-...-001`；`REQ-NF-TRACE-001` → `REQ-PROC-TRACE-001`；`REQ-NF-THREAD-001` → `REQ-NF-SESSION-001`。其 AC/TC ID 同步更名，旧标识不得用于新提交或验收证据；现有实现/测试均未创建，故无迁移二进制兼容问题。
- 冻结规则：只有确认源树 SHA、Mat 字典、外部接口、逐模块验收容差及阻塞风险后，SRS 才能由项目所有者批准为基线；对仍开放的 Should/TBD 项应有明确批准的处理决定。

## 4. 编号需求及逐项验收条件

### 功能需求

| 需求 ID | 优先级 | 依据状态 | 可验证的需求陈述 | 验收条件 |
|---|---|---|---|---|
| REQ-F-ADAPT-001 | M | 确定 | 系统应实现 Adaptive MFA 单步 6×6 系统及其平移、旋转、平均平方残差（MSE）、近度和新球谐系数。 | AC-F-ADAPT-001：固定原 MATLAB 输入逐项比较近度归一化、六维右端、M1–M4 四块、6×6 系统、z、归一化 t、ω、原返回 error=MSE、重算 near、新 3×5 系数；内部 oneit_error=sqrt(error) 未返回，不能把 RMS 当作原 error；逐节点 Golden 与批准容差尚待 DEC-005。 |
| REQ-F-ADAPT-002 | M | 确定 | 系统应在连续轨迹中按原顺序旋转系数、计算并提交主状态，同时支持独立对比状态。 | AC-F-ADAPT-002：固定逐帧输入检验更新顺序、运行间隔离。 |
| REQ-F-ADAPT-003 | M | 确定 | 系统应支持每帧重置、继承、1/5/11/21 帧周期刷新策略。 | AC-F-ADAPT-003：固定帧号验证正确触发且不污染主系数。 |
| REQ-F-AN-001 | M | 确定 | 系统应复现原 Adaptive 初始化与两种轨迹分析的曲线数值、分段阈值和窗口平滑。 | AC-F-AN-001：输出结构至少包含原 MAT 变量名、横轴帧索引、纵轴指标和单位、曲线数组、分段起止索引、平滑窗口及未定义值标记；初始化核验 mittrans 第 1–13 项与 mitrot 全序列；600 点比较图核验 50–550 帧、阈值 0.01、rottransratio 双纵轴；更新率图核验阈值 0.02 和半窗 2/5/10，逐元素对照而非仅比较图像。 |
| REQ-F-AN-002 | S | 确定 | 系统应将绘图与数值分析隔离，并支持原三份 illustration 的可比较展示。 | AC-F-AN-002：无绘图依赖下数值命令可运行；图示所据数据一致。 |
| REQ-F-BIAS-001 | M | 确定 | 系统应复现非球形视场的 Bias 实验及三组噪声乘数 [0.6,2.4,9.6]。 | AC-F-BIAS-001：以固定 Gaussian 序列含 0、1、−1 分量，对 [0.6,2.4,9.6] 每项验证原脚本的 randn × 系数 × 平均光流模长（**不能开平方**）及切平面投影；另核对 xwerte 和 ywerte1…6 变量名、维度。 |
| REQ-F-BIAS-002 | M | 确定 | 系统应复现完整球面、逐分量比例噪声的 Bias 实验。 | AC-F-BIAS-002：同一固定流场及 Gaussian 序列下，逐分量按 randn × 系数 × p_component 扰动并重新投影；其输出与平均模长噪声模型有可计算差别，保存文件名和维度一致。 |
| REQ-F-BIAS-003 | M | 确定 | 系统应支持汇总两种 Bias 实验各 40 份独立运行结果及理论参考曲线。 | AC-F-BIAS-003：跨运行均值及 100/sqrt(N) 曲线与原分析脚本一致。 |
| REQ-F-CLI-001 | M | 确定 | 系统应提供 inspect-mat 命令，对指定 MAT 文件进行只读检查，不调用任何估计器。 | AC-F-CLI-001：依据 [design/cli.md §2–4](design/cli.md) 的 `inspect-mat` 候选合同，使用一个有效压缩 MAT、一个不存在的文件及一个变量名错误样本，核验帮助、strict/compat、变量名/类型/形状、退出码与原文件哈希不变；具体 CLI 参数/错误映射仍待 P0.8/P1.2 批准。 |
| REQ-F-CLI-002 | M | 接口草案 | 系统应分别提供两种 Bias 数据生成命令，保留随机种子、细分等级和结果目录参数。 | AC-F-CLI-002：依据 [design/cli.md §2–4](design/cli.md) 分别检查两种 Bias 命令的合法/非法等级、seed、输出目录及覆盖冲突；生成 xwerte 与 ywerte1…6，并按候选错误映射以非零退出码报告失败；具体范围未批准前不得标为通过。 |
| REQ-F-CLI-003 | M | 接口草案 | 系统应分别提供 Adaptive 初始化、带/不带噪声比较和更新间隔三种实验入口。 | AC-F-CLI-003：依据 [design/cli.md §2–4](design/cli.md) 逐命令核查三种 Adaptive 入口的原 MATLAB 参数、固定默认值、当前脚本输出合同与 1/5/11/21 刷新时刻；非法参数或数值失败不得报告成功。 |
| REQ-F-CLI-004 | M | 接口草案 | 系统应提供独立 analyze 命令，只读取已有 MAT 结果并计算曲线数据而不运行求解器。 | AC-F-CLI-004：依据 [design/cli.md §2–4](design/cli.md) 用历史归档执行四种 analyze 路径，核验不调用求解器、原结果哈希不变及缺失字段的明确错误和非零退出。 |
| REQ-F-CLI-005 | S | 接口草案 | 系统应提供可选 plot 命令，绘图后端缺失时不影响其他数值命令。 | AC-F-CLI-005：依据 [design/cli.md §2–4](design/cli.md) 对同一分析数据核验可选绘图输出与后端缺失状态（候选退出码 7）；禁用绘图不影响 inspect/run/analyze。 |
| REQ-F-ERROR-001 | M | 确定 | 系统应分别实现尺度对齐向量误差与方向角度误差，不能用单一误差代替。 | AC-F-ERROR-001：给定相同真值和估计，两个指标与原脚本各自一致。 |
| REQ-F-EXP-001 | M | 确定 | 系统应复现 35 点圆形初始化实验，保存四类方向误差结果。 | AC-F-EXP-001：参数、数组形状、索引与历史 MAT 比较。 |
| REQ-F-EXP-002 | M | 确定 | 系统应复现带/不带噪声 600 点正弦轨迹对比，保留八次运行与原输出。 | AC-F-EXP-002：分别使用 addederror=0 与批准的非零值，核验 600 点、二十面体等级 4、循环 i=2…599、8 次运行；按当前代码输出 allemittransfehler、allemitrotfehler、alleohnetransfehler、alleohnerotfehler、rottransratio 五个 600×1 MAT；前四项求八次平均，rottransratio 保留最后一次运行值，不伪造历史 alleadap*。 |
| REQ-F-EXP-003 | M | 确定 | 系统应复现多更新频率实验，并显式登记历史 MAT 与当前脚本输出不一致项。 | AC-F-EXP-003：在 600 点正弦轨迹、二十面体等级 2 和刷新周期 [1,5,11,21] 下核验各周期的 600×4 误差矩阵、更新时刻与原脚本实际保存的 allemittransfehler.mat、allemitrotfehler.mat、akrot.mat；对历史归档额外的 alleadaprotfehler.mat、alleadaptransfehler.mat、rottransratio.mat 出具字段级差异记录（文件、变量、shape、生成者、分析依赖、来源状态），未知来源不得伪造。 |
| REQ-F-FILTER-001 | M | 确定 | 系统应忠实实现 Franz 三轴匹配滤波的平移、旋转模板响应与归一化。 | AC-F-FILTER-001：依据 [design/kvd.md §2](design/kvd.md) 核对固定 N×6 输入下的三轴平移/旋转模板向量、每轴响应和平方范数累积、归一化系数及最终 t/ω；零范数退化要有独立状态。逐节点 MATLAB Golden SHA、实测绝对/相对误差及 DEC-005 批准容差齐备后方可判定。 |
| REQ-F-FLOW-001 | M | 确定 | 系统应按原符号约定计算平移/旋转及其合成切向光流与平均模长。 | AC-F-FLOW-001：固定输入的三维流场、切向残差、统计量一致。 |
| REQ-F-FLOW-002 | M | 确定 | 系统应使用原距离范围 [1,4]、归一化和随机近度方式生成 Bias 场景。 | AC-F-FLOW-002：固定随机序列下各步距离与近度可比较。 |
| REQ-F-FLOW-003 | M | 确定 | 系统应区分平均流长尺度高斯扰动和逐分量高斯扰动，并投影回切平面。 | AC-F-FLOW-003：各模型用固定 Gaussian 序列逐项核对。 |
| REQ-F-FLOW-004 | M | 确定 | 系统应计算单位球内旋转视线射线交点的近度。 | AC-F-FLOW-004：球心、一般位置、边界与非法输入的数值及状态可核验。 |
| REQ-F-GEOM-001 | M | 确定 | 系统应以 N×6 数据表示单位视线和三维切向光流，正确执行原叉积与点积。 | AC-F-GEOM-001：固定矩阵和逐元素计算比较，形状错误可检测。 |
| REQ-F-GEOM-002 | M | 确定 | 系统应按原递归顺序生成 8×4^i 全八面体与 6×4^i 缺损八面体视线。 | AC-F-GEOM-002：对 i=0、1、2 使用从原 MATLAB 生成的带来源 SHA 的**有序**黄金方向矩阵，检查 N、单位范数、缺损面和逐行分量；逐元素容差经 DEC-005 冻结，哈希只作文件身份检查。 |
| REQ-F-GEOM-003 | M | 确定 | 系统应按原顶点/面表生成 20×4^i 二十面体视线。 | AC-F-GEOM-003：对 i=0、1、2 使用原 MATLAB 二十面体黄金矩阵逐行比较三维分量、顺序和 N=20·4^i；候选误差不冒充通过，最终容差经 DEC-005 冻结。 |
| REQ-F-KVD-001 | M | 确定 | 系统应提供独立 Original KvD 和 Modified KvD 入口，保留各自不同的平移更新公式。 | AC-F-KVD-001：原始 MATLAB 函数已定位为 `help_functions/norm_koenderink_iteration.m` 与 `help_functions/mue_norm_koenderink_iteration.m`；MAT 数据字典记录的**独立探针运行参数 `source_sha`** 为 `48c590ba2564e4bdd293d2cbecd2eef83b291452`，它**不是 KvD 函数文件 blob SHA，也不是 Golden 生成脚本或参考数值文件的 SHA**；KvD 逐轮 Golden 的固定 N×6 输入、生成/采集脚本 blob SHA 和参考数据 SHA-256 仍待建立、核验。使用同一固定输入，逐轮核验 Franz 初值、重估 near、符号翻转、Original/Modified 各自的平移更新三项均值、旧 t 计算的 ω、新 t、RMS 残差、迭代次数及最终近度尺度恢复；每个量列出维度、最大绝对/相对误差，容差经 DEC-005 批准后方可判定。 |
| REQ-F-KVD-002 | M | 确定 | 系统应保留 KvD 初始化、更新先后及至少 6 次、最多 200 次和动态/绝对阈值退出规则。 | AC-F-KVD-002：覆盖提前停止、最低次数、上限与异常输入。 |
| REQ-F-MAT-001 | M | matio 探针 582/582 读取成功；核心 Reader 数值合同未验收 | 系统应使用 matio/zlib 在运行时解码已支持的原始压缩 MAT v5 数值变量并正确返回数据值，不能依赖 MATLAB/Python。 | AC-F-MAT-001：对每个归档源文件执行实际可解码性检查，在已支持类型上验证元素值、长度及列主序；失败必须计入数据字典但不能冒充成功。 |
| REQ-F-MAT-002 | M | 582 个 MAT 变量元数据已实测；正式接口未实现 | 系统应提供变量枚举与诊断能力，读取 MAT 文件后返回各变量名称、类型、rank、dims、复数标志及不支持原因。 | AC-F-MAT-002：对所有源 MAT 输出元数据报告，按原始文件名准确定位不支持的类别/无效尺寸；其职责是运行时查询而非生成全量字典文档。 |
| REQ-F-MAT-003 | M | 确定 | 系统应分别提供指定变量名的严格读取和 getflowfield 兼容的首变量/省略扩展名读取。 | AC-F-MAT-003：分别测试单变量、多变量、缺失变量、后缀处理。 |
| REQ-F-MAT-004 | M | 确定 | 系统应在独立 RunId 输出目录写入变量名与尺寸兼容的 MAT 结果。 | AC-F-MAT-004：写后回读一致，拒绝覆盖原归档/历史 RunId。 |
| REQ-F-SH-001 | M | 确定 | 系统应复现 l≤2 的实球谐基函数、3×5 系数、近度恢复与逐项拟合。 | AC-F-SH-001：依据 [design/harmonics.md §1–3](design/harmonics.md) 对固定方向逐项核查 l≤2 九个实球谐基函数的正/负 m 相位、归一化、3×5 有效位置、逐项 koeff 拟合与 near 重建；分别记录索引/维度、参考 SHA 和误差。没有源 MATLAB/独立 Golden 及 DEC-005 批准容差时保持未验收。 |
| REQ-F-SH-002 | M | H-02/H-14 已选择 Don Williamson C++ 方案 A；尚待许可保留/Golden/实施 | 系统应复现 l≤2 球谐旋转矩阵与系数旋转，保持原基函数顺序及旋转方向。 | AC-F-SH-002：3×3、5×5 旋转块固定样本对照；权利来源先核实。 |
| REQ-F-TRAJ-001 | M | 确定 | 系统应按原参数生成圆轨迹、正弦轨迹和相邻点局部运动/旋转，不将绘图混入数据生成。 | AC-F-TRAJ-001：样本轨迹、旋转方向、端点和角度转换对照。 |

### 数据与来源需求

| 需求 ID | 优先级 | 依据状态 | 可验证的需求陈述 | 验收条件 |
|---|---|---|---|---|
| REQ-D-MAT-001 | M | 已有 582/582 的 matio 实测字典；P10.4 审查待完成 | 项目应根据 MAT 读取器对 582 个源 MAT 建立并版本化完整数据字典，记录变量、压缩类型、维度、哈希及解码状态。 | AC-D-MAT-001：全部 582 文件须有**实际解压/变量读取尝试**及逐文件成功、失败或类型不支持的结果；每个变量记录 class/type、rank/dims 与来源 SHA；P0.5/0.6 的审计方案或字典模板不能计为验收，需 P1.8 实测并由 P10.4 复核。 |
| REQ-D-MAT-002 | M | 确定 | 系统应保留源 MAT 的列主序、数据类型、变量命名、数值含义和输出形状。 | AC-D-MAT-002：已读字段按元素及真实 shape 比较，变更有记录。 |
| REQ-D-MAT-003 | M | 历史差异待归因 | 系统应区分生成脚本实际保存、分析脚本需要和历史归档额外存在的 MAT 变量。 | AC-D-MAT-003：所有差异有状态和证据，不伪造缺失数据。 |
| REQ-D-SOURCE-001 | M | 目录已核对 | 系统应冻结原始资料的 SHA、45 个 .m、582 个 .mat、readme 和具体文件分类。 | AC-D-SOURCE-001：源码清单与 GitHub 树一致，每项可定位哈希。 |
| REQ-D-SOURCE-002 | M | 待权利核查 | 系统应记录原作者及外部 rodrigues/SHRotate 的实际来源和可再分发范围。 | AC-D-SOURCE-002：核对文件版权头；不擅自补造 LICENSE 结论。 |

### 外部与组件接口合同

| 需求 ID | 优先级 | 依据状态 | 可验证的需求陈述 | 验收条件 |
|---|---|---|---|---|
| REQ-I-ANALYSIS-001 | M | 待数据字典冻结 | 系统分析接口应接收已有 MAT 结果目录并返回数值曲线/统计结构，而不调用求解器或修改输入。 | AC-I-ANALYSIS-001：使用全部四类分析原入口的结果数据验证输入/输出字段，原始 MAT SHA 无变化。 |
| REQ-I-CLI-001 | M | 待合同冻结 | 系统命令行的命令名、必填/可选参数、类型、合法范围和错误退出行为应具有可读取的确定规格。 | AC-I-CLI-001：以 [design/cli.md §2–4](design/cli.md) 的逐命令参数/类型/范围及 [design/contracts.md §3.1](design/contracts.md) 的错误码→CLI 退出码映射候选为检查依据，逐命令核验必要字段、默认行为、优先级和诊断字段；SRS §6.1 仅是外部接口摘要，详细合同仍须 P0.8/P1.2 批准后冻结。 |
| REQ-I-MAT-001 | M | 历史数据格式已确认；严格/兼容读取及写入接口未验收 | 系统的 MAT 外部接口应区分只读严格/兼容读取和独立输出写入，并保留变量类型/维度等元数据。 | AC-I-MAT-001：使用单/多变量和压缩 MAT 验证路径、变量选择、类型/尺寸、只读/输出保护，不依赖求解器。 |
| REQ-I-RESULT-001 | M | 原作者结果 dtype/shape 已确认；C++ 新结果尚未生成 | 系统实验结果的变量名、尺寸、数据类型、实验标识和输出目录命名应与对应原 MATLAB 结果合同一致。 | AC-I-RESULT-001：按五个数据生成入口分别核验新增 MAT 的变量、形状与历史/当前源码差异表。 |

### 内部设计约束（非外部接口）

| 需求 ID | 优先级 | 依据状态 | 可验证的需求陈述 | 验收条件 |
|---|---|---|---|---|
| REQ-DC-ADAPT-001 | M | 设计待冻结 | 系统应将无状态单步求解与有状态 Session/更新策略分离。 | AC-DC-ADAPT-001：单步无 Session 可测，两个 Session 无状态串扰。 |
| REQ-DC-ANALYSIS-001 | S | 设计待冻结 | 系统应将 MAT 读取、分析、可选绘图分离，禁止图表后端改变数据。 | AC-DC-ANALYSIS-001：禁用绘图时分析值相同。 |
| REQ-DC-DOMAIN-001 | M | 设计待冻结 | 系统领域合同应表达流场、运动、球谐系数的语义、维度和显式失败状态。 | AC-DC-DOMAIN-001：P0 冻结输入/输出合同，非法形状报错。 |
| REQ-DC-EXPERIMENT-001 | M | 设计待冻结 | 系统实验运行器应通过组合注入求解、RNG、采样和写入器，彼此独立。 | AC-DC-EXPERIMENT-001：固定替身运行两种 Bias 模式无交叉状态。 |
| REQ-DC-IO-001 | M | 设计待冻结 | 系统应使用资源安全的 MAT 读取及独立写入接口，领域计算不直接依赖 matio 指针。 | AC-DC-IO-001：RAII/内存寿命、只读/写入权限、替身可验证。 |
| REQ-DC-KVD-001 | M | 设计待冻结 | 系统应保持两个 KvD 入口和差异化更新规则，不让公共引擎覆盖原始计算顺序。 | AC-DC-KVD-001：策略替换对照两条原 MATLAB 路径。 |
| REQ-DC-NOISE-001 | M | 设计待冻结 | 系统应允许通过窄接口选择采样器、噪声模型和无噪声策略。 | AC-DC-NOISE-001：替换实现不修改核心求解器。 |
| REQ-DC-RNG-001 | M | 设计待冻结 | 系统应支持显式 RNG 注入、固定序列替身和逐次调用顺序记录。 | AC-DC-RNG-001：同固定序列不改变数学实现即可复现。 |

### 非功能及质量需求

| 需求 ID | 优先级 | 依据状态 | 可验证的需求陈述 | 验收条件 |
|---|---|---|---|---|
| REQ-NF-COPYRIGHT-001 | M | H-02 已限定第三方替代来源；源码再分发与声明仍须核查 | 系统应保留两份 DOI、第三方来源，不擅自新增项目级 LICENSE。 | AC-NF-COPYRIGHT-001：README 署名可核对，第三方权利障碍记录。 |
| REQ-NF-DEPS-001 | M | 确定 | 系统核心运行不得依赖 Python、MATLAB Runtime、NumPy、ROS、OpenCV、Windows 或 NPZ 转换。 | AC-NF-DEPS-001：检查依赖和链接，源代码无禁用路径。 |
| REQ-NF-HEADLESS-001 | M | 确定 | 系统应支持无图形环境的检查、运行和分析，绘图模块不得阻断数值。 | AC-NF-HEADLESS-001：禁用绘图时主要数据路径仍可执行。 |
| REQ-NF-LOG-001 | M | 确定 | 系统应记录 RunId、SHA、环境、命令、参数、随机种子、输入身份、退出码及日志位置。 | AC-NF-LOG-001：每次正式运行记录字段完整，不覆盖失败证据。 |
| REQ-NF-NUM-001 | M | M（受 DEC-005 阻塞） | 系统应保留 MATLAB 原公式/失败状态并提供固定输入数值对照与分模块容差。 | AC-NF-NUM-001：必须采用 double、保留原始计算及失败行为；对固定输入记录各模块实际绝对/相对误差、尺度、条件数；原候选 1e-10 不构成通过阈值，待 DEC-005 批准逐模块容差才可验收。 |
| REQ-NF-NUM-002 | M | 确定 | 系统应显式报告零范数、奇异、非有限、文件损坏、未定义角度；不可误记成功。 | AC-NF-NUM-002：每类失败有非成功状态及对应日志。 |
| REQ-NF-OOP-001 | M | 确定 | 系统应明确所有权，隔离算法公式与实验编排，不创建巨型类或错误的通用求解器。 | AC-NF-OOP-001：对象模型和独立测试核实状态、资源与依赖方向。 |
| REQ-NF-PLATFORM-001 | M | 确定 | 系统应在 Debian/Ubuntu 上以 C++17/CMake 源外构建。 | AC-NF-PLATFORM-001：实际支持的平台/编译器构建并留命令与日志。 |
| REQ-NF-PORTABLE-001 | S | 确定 | 系统应在 Debian/Ubuntu 支持含空格和中文的输入输出路径。 | AC-NF-PORTABLE-001：实际 Unicode/空格路径可读写。 |
| REQ-NF-RECOVERY-001 | M | 确定 | 系统应保留错误与失败 RunId 原始记录，不覆盖已有结果并可说明原因。 | AC-NF-RECOVERY-001：错误退出日志可追溯，历史未受影响。 |
| REQ-NF-RNG-001 | M | 确定 | 系统应将固定样本等价验证与随机统计复现分离，并独立记录各 RunId 种子。 | AC-NF-RNG-001：固定序列可重复，统计含有效/失败数量。 |
| REQ-NF-RUNTIME-001 | S | S／TBD（受 DEC-006 阻塞） | 系统应采集完整实验的墙钟、CPU、峰值内存和输入规模，正式运行上限需实测后批准。 | AC-NF-RUNTIME-001：可先记录完整实验 wall/CPU 时间、峰值 RSS、输入大小、编译用时和环境；加速倍数/上限等要求未冻结前不得宣称性能验收通过。 |
| REQ-NF-SAFETY-001 | M | 确定 | 系统应保证 origin/** 所有原文件只读且拒绝覆盖历史 RunId。 | AC-NF-SAFETY-001：前后哈希一致、重复输出默认拒绝。 |
| REQ-NF-SESSION-001 | S | 待实测 | 系统应隔离不同 RunContext 的随机状态、系数及输出写入，暂不承诺同一 Session 的多线程安全。 | AC-NF-SESSION-001：使用两个 RunContext 交替运行固定输入，结果独立一致；单 Session 并发调用明确列为不支持/未定义，不标为已验证。 |

### 过程质量与追踪要求

| 需求 ID | 优先级 | 依据状态 | 可验证的需求陈述 | 验收条件 |
|---|---|---|---|---|
| REQ-PROC-TRACE-001 | M | 确定 | 项目的需求管理过程应为每条正式要求维护 AC、TC、来源、PLAN 任务及实测状态，并不得将未运行的用例记为通过。 | AC-PROC-TRACE-001：全部 P0–P10 的 98 项任务都有 REQ 关联、每个必须需求有 AC/TC，45 个源文件均追踪；测试文件/CTest 未生成的条目必须标注拟议而非真实。 |

### 4.1 MAT 审计任务与需求验收的先后关系

`REQ-D-MAT-001 / AC-D-MAT-001` 要求的不是“写好审计方案”，而是**对 582 个源 MAT 完成实际解压、变量读取和字典记录**。为避免 P0 在没有 C++ matio Reader 时不合理地被 P1 阻塞，将交付责任区分为：

| 步骤 | 对应任务 | 产物/证据 | AC-D-MAT-001 判定 |
|---|---|---|---|
| 审计规则准备 | P0.5 | 文件身份/头部抽样、字段定义、失败分类、批量检查流程；仅文档/静态审计 | **不能通过** |
| 字典模板与来源清单 | P0.6 | MAT_DATA_SCHEMA 字段表、待检文件列表与已知/未知状态 | **不能通过** |
| 完整实际解码 | P1.8 中独立 `src/mat_data_dict_probe.cpp` 探针（尚非完整生产 Reader） | **Ubuntu 24.04 / matio 1.5.26：582/582 文件、582 变量均 `READ_OK`、失败 0；每文件版本/class/type/dims/FNV64 已写入 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md)** | 真实全量读取证据已取得；核心库接口和 P10.4 独立复核仍未验收 |
| 独立复核和归档差异确认 | P10.4 | 560 份 Bias + 22 份 Adaptive 的读取与历史/当前源码契约复核，失败未被隐藏 | 审查全部证据、确认通过或如实标阻塞 |

该阶段已有**实际全量解码证据**，不是 P0 的模拟报告。`AC-D-MAT-001` 仍需核对数据字典与原资料身份、失败样本口径及 P10.4 审查记录后才能标记最终通过；不能用这份读档证据替代损坏输入负例、按值索引校验、原算法数值对照或新 MAT 写回读。

## 5. MAT 数据字典与结果契约

- `setofpoints` 是 N×6（前三列单位视线，后三列三维切向光流）；系数布局为 3×5（1+3+5 个有效项）；Adaptive 线性系统为 6×6。MATLAB 的列主序、1-based 索引意义、数值精度不能被静默改变。
- **已实测的归档数据合同**：`occluded_regions` 40×7=280、`proportional_error` 40×7=280；Adaptive 的 initialization 4、without_errors 5、with_errors 7、many_parameter 6，共 **582 个文件，582 个变量**。matio 1.5.26 实际读取均成功：**582/582 MAT v5、582/582 MATLAB class=double、data_type=double、zlib 压缩，flags 为 `-`，失败 0**。该批源文件无 MAT v7.3、cell、struct、sparse、complex、single 或整数存储变量；所以**不需要为现存 582 文件引入 HDF5 或其他 MAT class 读取支持**。详见 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md)。
- **变量尺寸（真实 MAT 内容，不再是由 MATLAB `zeros` 推测）**：`10×3` 480 个、`10×1` 80 个、`35×1` 4 个、`600×1` 14 个、`600×4` 4 个。582/582 的变量名与同名 `.mat` 文件 basename 一致，且 `data_bytes = 8 × 元素数`。该证据只覆盖原作者已保存的历史结果，不证明新 C++ 算法或输出的数值正确。
- Bias 的 xwerte、ywerte1…6 和 Adaptive 的 allemit*/alleohne*/rottransratio 等应以实际每个文件的变量名、shape 为准。历史归档可能与当前生成脚本输出不一致，须分别登记“当前写出、已有历史、分析需要、来源未明”。
- 噪声参数 `[0.6, 2.4, 9.6]` 在原代码中直接乘以 randn，不能因为注释写 variance 而自行开平方。

## 6. 外部接口合同（不包含内部 OOP 类的实现承诺）

| 外部接口 | 输入 | 输出/异常 |
|---|---|---|
| 命令行 | 实验种类、参数、原始数据路径、RunId | 运行结果或结构化失败诊断 |
| MAT 读取 | 路径、严格变量名或兼容模式 | 数组、元数据或明确错误；始终只读原始数据 |
| MAT 结果写入 | 与 origin 不重叠的输出路径、变量名、数组 | 可回读的 MAT 或拒绝覆盖/越界 |
| RNG/采样/噪声 | 种子或固定随机序列、采样级别、模型 | 有明确顺序的流场或失败状态 |
| Franz、KvD、Adaptive | 流场、近度/系数、原参数 | 运动估计、残差、迭代和非成功状态 |
| 分析/绘图 | 历史或新生成 MAT | 统计、曲线、可选图示；不反向改动求解结果 |

**需求与设计不混淆**：PLAN 中的 RAII、FlowField、StepSolver、Session、策略类和组合根是当前拟议实现方式；如果内部设计变更却仍满足这里的输出与验收约束，不应无故改写外部需求。


### 6.1 拟议 CLI 参数 schema 与退出码（待 P0.8/P1.2 冻结）

逐命令选项类型、合法范围、错误优先级、RunId/诊断输出的明确候选见 [design/cli.md](design/cli.md)。本节保持外部需求概览，**CLI 细节仍待批准，不等于已实现或已冻结**。

| 入口 | 必填或关键实参 | 可核验的输出 |
|---|---|---|
| inspect-mat | --file PATH；可选 --variable NAME、--mode strict/compat | 不写输入文件，显示 MAT 变量与错误诊断 |
| run-bias-occluded / run-bias-proportional | --seed INTEGER、--level INTEGER、--output-root DIR | 对应两个实验的 xwerte 与六个 ywerte |
| run-adaptive-init | --output-root DIR | 四类初始化误差 |
| run-adaptive-comparison | --noise-scale NUMBER、--output-root DIR | 前四种方向误差加 rottransratio 五类 MAT |
| run-adaptive-update-rates | --output-root DIR | 1/5/11/21 更新周期结果 |
| analyze | --experiment NAME、--input-root DIR | 四类分析之一的数值序列，不重跑求解 |
| plot（S 级） | --experiment NAME、--input-root DIR | 可选图示；无后端不妨碍数值命令 |

建议退出码表（尚未冻结）：0 成功；2 参数错误；3 文件/MAT 输入错误；4 MAT 类型或形状错误；5 数值状态失败；6 输出安全冲突；7 可选功能不可用。参数合法范围、错误优先级、默认值及输出 schema 在 P0.8/P1.2 以 [TEST_PLAN.md](TEST_PLAN.md) 中 TC-F-CLI-* 的测试说明确认，不能将这份候选表当作已有 CLI。

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
| DEC-001 | 582 份现有归档的 MAT 格式和 HDF5 必要性 | **格式事实已证实**：582/582 MAT v5 + zlib；现有数据无需 MAT v7.3/HDF5 读取能力。项目支持范围仍待所有者批准 |
| DEC-002 | 现有 MAT 变量 class/data_type/dims | **类型与形状事实已证实**：582 个 double/double；`10×1` 80、`10×3` 480、`35×1` 4、`600×1` 14、`600×4` 4；无其他 class，数据字典待独立复核 |
| DEC-003 | rodrigues.m 和 SHRotate.m 中第三方代码许可/可再分发范围 | **限定替代方向已由 H-02 批准**：Sophus `SO3d::exp` 取代 Rodrigues；Don Williamson 的 C++ `IvanicSHRotate` 取代 SHRotate；上游 MIT 形式许可文本已找到；第三方源码纳入、上游版权声明/README/LICENSE 的具体保存、间接依赖及原 MATLAB 自身版权不自动获得完整验收 |
| DEC-004 | 历史 MAT 有而当前源脚本未保存的字段 | H-07 **只允许先起草调查**，归因和统计处理未批准 |
| DEC-005 | 固定输入容差及统计噪声验收方法 | H-01 **draft**：仅准备逐项误差和 atol/rtol 提案；数值阈值、Golden 与最终验收标准均未批准 |
| DEC-006 | Debian/Ubuntu 版本范围、CPU/内存与构建时间预算 | H-04 **确认开发主机快照**：Ubuntu 24.04.4 VirtualBox/x86_64，物理内存总 8127380 KiB、当时 available 4908696 KiB、Swap 4194300 KiB；历史独立探针曾在 Ubuntu 24.04/GCC 13/matio 1.5.26 成功。**未有真实全工程构建、Debian 验证或可据以判通过的 wall/CPU/RSS/构建时间阈值** |
| DEC-007 | 原 MATLAB 文件第三方代码的再分发权利与归档使用边界 | H-02 **只针对 Sophus/Don Williamson 选定来源**；将来实际复用 Don C++ 文件时在其目录提供上游来源 README 和适用 MIT LICENSE/版权声明。整个原 MATLAB/归档的许可范围及 H-16 发布权限**未获整体批准** |
| DEC-008 | TC 与测试文件、CTest 名称及 Golden 文件的实际对应关系 | H-03 仅**draft**：先整理 `cli.md`/`contracts.md` 与 TC→测试/Golden 候选，不冻结；H-10/H-11 暂缓运行 |

**H-12 的实施目录决定**：后续新生成的数据放在仓库根 `output/<RunId>/`，`.gitignore` 已规划排除 `/output/`，以避免新数据被误提交；`origin/**` 保持只读。H-12 属于路径和隔离策略，**不改变**上表中 REQ/AC 的验收义务，也不因它批准而允许运行实际写回读（H-10 暂缓、`permissions.writeMat=false`）。

## 9. 双向追踪矩阵：来源 → REQ → PLAN → TC/AC

下表的完整业务 C++ 设计仍以 PLAN 的 P 子任务指向拟议模块；**已存在独立 `src/mat_data_dict_probe.cpp` 及其正式读取结果报告，但通用生产 Reader/Writer、测试函数与 CTest 注册名尚未建立**。对应测试实体规划见 [TEST_PLAN.md](TEST_PLAN.md)；探针验证仅归于 MAT 数据字典子能力，不能使其余 REQ 自动通过。

| MATLAB/规则来源 | REQ | PLAN 工作项 | 计划 TC | 对应 AC | 当前状态 |
|---|---|---|---|---|---|
| MATLAB five generation entries | REQ-F-CLI-001 | P1.2, P1.4, P1.6 | TC-F-CLI-001 | AC-F-CLI-001 | 未实现/未运行 |
| 582 MAT files | REQ-F-MAT-001 | P1.3, P1.4, P1.8, P1.9 | TC-F-MAT-001 | AC-F-MAT-001 | matio 探针已验证历史归档读取；其他合同待验收 |
| MAT archive | REQ-F-MAT-002 | P0.5, P1.5 | TC-F-MAT-002 | AC-F-MAT-002 | matio 探针已验证历史归档读取；其他合同待验收 |
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
| origin/**/produced_data/** | REQ-D-MAT-001 | P0.5, P0.6, P1.8, P10.4 | TC-D-MAT-001 | AC-D-MAT-001 | matio 探针已验证历史归档读取；其他合同待验收 |
| getflowfield.m; save calls | REQ-D-MAT-002 | P1.4, P2.8, P5.4 | TC-D-MAT-002 | AC-D-MAT-002 | 未实现/未运行 |
| with_errors; many_parameter | REQ-D-MAT-003 | P8.7, P10.4 | TC-D-MAT-003 | AC-D-MAT-003 | 未实现/未运行 |
| PLAN §3 | REQ-DC-DOMAIN-001 | P0.7, P0.8, P2.1 | TC-DC-DOMAIN-001 | AC-DC-DOMAIN-001 | 未实现/未运行 |
| PLAN §3 | REQ-DC-IO-001 | P1.3, P1.9, P1.10 | TC-DC-IO-001 | AC-DC-IO-001 | 未实现/未运行 |
| getvektor.m; RandStream | REQ-DC-RNG-001 | P2.7, P3.7, P3.8 | TC-DC-RNG-001 | AC-DC-RNG-001 | 未实现/未运行 |
| sampling/noise .m | REQ-DC-NOISE-001 | P2.9, P3.9 | TC-DC-NOISE-001 | AC-DC-NOISE-001 | 未实现/未运行 |
| two KvD solvers | REQ-DC-KVD-001 | P4.9 | TC-DC-KVD-001 | AC-DC-KVD-001 | 未实现/未运行 |
| oneit*.m; adaptive generation | REQ-DC-ADAPT-001 | P7.11, P8.8, P8.9 | TC-DC-ADAPT-001 | AC-DC-ADAPT-001 | 未实现/未运行 |
| PLAN §3 | REQ-DC-EXPERIMENT-001 | P1.2, P5.7 | TC-DC-EXPERIMENT-001 | AC-DC-EXPERIMENT-001 | 未实现/未运行 |
| data_analysis/*.m | REQ-DC-ANALYSIS-001 | P9.8 | TC-DC-ANALYSIS-001 | AC-DC-ANALYSIS-001 | 未实现/未运行 |
| AGENTS.md | REQ-NF-PLATFORM-001 | P1.1, P10.6 | TC-NF-PLATFORM-001 | AC-NF-PLATFORM-001 | 未实现/未运行 |
| AGENTS.md | REQ-NF-DEPS-001 | P1.1, P10.6 | TC-NF-DEPS-001 | AC-NF-DEPS-001 | 未实现/未运行 |
| AGENTS.md | REQ-NF-SAFETY-001 | P1.3, P1.7, P1.9 | TC-NF-SAFETY-001 | AC-NF-SAFETY-001 | 未实现/未运行 |
| PLAN §7 | REQ-NF-NUM-001 | P4.3, P4.8, P7.9, P10.3, P10.5 | TC-NF-NUM-001 | AC-NF-NUM-001 | 未实现/未运行 |
| AGENTS.md | REQ-NF-NUM-002 | P4.7, P7.9, P10.3 | TC-NF-NUM-002 | AC-NF-NUM-002 | 未实现/未运行 |
| AGENTS.md; PLAN.md | REQ-PROC-TRACE-001 | P0.6, P10.1, P10.2, P10.3, P10.8, P10.9, P10.10 | TC-PROC-TRACE-001 | AC-PROC-TRACE-001 | 未实现/未运行 |
| RandStream usage | REQ-NF-RNG-001 | P3.7, P5.3, P10.5 | TC-NF-RNG-001 | AC-NF-RNG-001 | 未实现/未运行 |
| AGENTS.md | REQ-NF-LOG-001 | P5.3, P8.9, P10.6 | TC-NF-LOG-001 | AC-NF-LOG-001 | 未实现/未运行 |
| PLAN §3.3–3.6 | REQ-NF-OOP-001 | P0.7, P0.8, P10.9, P10.10 | TC-NF-OOP-001 | AC-NF-OOP-001 | 未实现/未运行 |
| PLAN §9 | REQ-NF-HEADLESS-001 | P9.6, P10.6 | TC-NF-HEADLESS-001 | AC-NF-HEADLESS-001 | 未实现/未运行 |
| AGENTS.md | REQ-NF-RECOVERY-001 | P5.3, P10.3 | TC-NF-RECOVERY-001 | AC-NF-RECOVERY-001 | 未实现/未运行 |
| AGENTS.md | REQ-NF-PORTABLE-001 | P1.2, P10.6 | TC-NF-PORTABLE-001 | AC-NF-PORTABLE-001 | 未实现/未运行 |
| PLAN; experiments | REQ-NF-RUNTIME-001 | P1.8, P10.6 | TC-NF-RUNTIME-001 | AC-NF-RUNTIME-001 | 未实现/未运行 |
| PLAN §3 | REQ-NF-SESSION-001 | P8.9, P10.9 | TC-NF-SESSION-001 | AC-NF-SESSION-001 | 未实现/未运行 |
| README.md; original headers | REQ-NF-COPYRIGHT-001 | P0.4, P10.1, P10.7 | TC-NF-COPYRIGHT-001 | AC-NF-COPYRIGHT-001 | 未实现/未运行 |
| bias_test/data_generation/*.m | REQ-F-CLI-002 | P1.2, P5.1, P5.2, P5.3 | TC-F-CLI-002 | AC-F-CLI-002 | 未实现/未运行 |
| adaptive_matched_filter_test/data_generation/*.m | REQ-F-CLI-003 | P1.2, P8.1, P8.2, P8.5 | TC-F-CLI-003 | AC-F-CLI-003 | 未实现/未运行 |
| bias_test/data_analysis/*.m; adaptive_matched_filter_test/data_analysis/*.m | REQ-F-CLI-004 | P1.2, P9.1, P9.4 | TC-F-CLI-004 | AC-F-CLI-004 | 未实现/未运行 |
| data_analysis/*.m; illustrations/*.m | REQ-F-CLI-005 | P1.2, P9.6, P9.7 | TC-F-CLI-005 | AC-F-CLI-005 | 未实现/未运行 |
| CLI/应用入口 | REQ-I-CLI-001 | P0.8, P1.2 | TC-I-CLI-001 | AC-I-CLI-001 | 未实现/未运行 |
| getflowfield.m; MAT archive | REQ-I-MAT-001 | P1.3, P1.6, P1.7 | TC-I-MAT-001 | AC-I-MAT-001 | 未实现/未运行 |
| data_generation/*; produced_data/* | REQ-I-RESULT-001 | P1.7, P5.4, P8.4, P8.7 | TC-I-RESULT-001 | AC-I-RESULT-001 | 未实现/未运行 |
| data_analysis/*.m | REQ-I-ANALYSIS-001 | P9.1, P9.2, P9.3, P9.4 | TC-I-ANALYSIS-001 | AC-I-ANALYSIS-001 | 未实现/未运行 |

### 9.1 反向追踪：98 个 P 子任务 → REQ

| PLAN 工作项 | 关联需求 |
|---|---|
| P0.1 | REQ-D-SOURCE-001 |
| P0.2 | REQ-D-SOURCE-001 |
| P0.3 | REQ-D-SOURCE-001 |
| P0.4 | REQ-D-SOURCE-002, REQ-NF-COPYRIGHT-001 |
| P0.5 | REQ-F-MAT-002, REQ-D-MAT-001 |
| P0.6 | REQ-D-MAT-001, REQ-PROC-TRACE-001 |
| P0.7 | REQ-DC-DOMAIN-001, REQ-NF-OOP-001 |
| P0.8 | REQ-DC-DOMAIN-001, REQ-NF-OOP-001, REQ-I-CLI-001 |
| P1.1 | REQ-NF-PLATFORM-001, REQ-NF-DEPS-001 |
| P1.2 | REQ-F-CLI-001, REQ-DC-EXPERIMENT-001, REQ-NF-PORTABLE-001, REQ-F-CLI-002, REQ-F-CLI-003, REQ-F-CLI-004, REQ-F-CLI-005, REQ-I-CLI-001 |
| P1.3 | REQ-F-MAT-001, REQ-DC-IO-001, REQ-NF-SAFETY-001, REQ-I-MAT-001 |
| P1.4 | REQ-F-CLI-001, REQ-F-MAT-001, REQ-D-MAT-002 |
| P1.5 | REQ-F-MAT-002 |
| P1.6 | REQ-F-CLI-001, REQ-F-MAT-003, REQ-I-MAT-001 |
| P1.7 | REQ-F-MAT-004, REQ-NF-SAFETY-001, REQ-I-MAT-001, REQ-I-RESULT-001 |
| P1.8 | REQ-F-MAT-001, REQ-D-MAT-001, REQ-NF-RUNTIME-001 |
| P1.9 | REQ-F-MAT-001, REQ-F-MAT-004, REQ-DC-IO-001, REQ-NF-SAFETY-001 |
| P1.10 | REQ-DC-IO-001 |
| P2.1 | REQ-F-GEOM-001, REQ-DC-DOMAIN-001 |
| P2.2 | REQ-F-GEOM-001 |
| P2.3 | REQ-F-GEOM-002 |
| P2.4 | REQ-F-GEOM-002 |
| P2.5 | REQ-F-GEOM-002 |
| P2.6 | REQ-F-GEOM-003 |
| P2.7 | REQ-DC-RNG-001 |
| P2.8 | REQ-F-GEOM-001, REQ-D-MAT-002 |
| P2.9 | REQ-F-GEOM-003, REQ-DC-NOISE-001 |
| P3.1 | REQ-F-FLOW-001 |
| P3.2 | REQ-F-FLOW-002 |
| P3.3 | REQ-F-FLOW-003 |
| P3.4 | REQ-F-FLOW-003 |
| P3.5 | REQ-F-FLOW-003 |
| P3.6 | REQ-F-FLOW-004 |
| P3.7 | REQ-DC-RNG-001, REQ-NF-RNG-001 |
| P3.8 | REQ-DC-RNG-001 |
| P3.9 | REQ-F-FLOW-003, REQ-DC-NOISE-001 |
| P4.1 | REQ-F-FILTER-001 |
| P4.2 | REQ-F-FILTER-001 |
| P4.3 | REQ-F-KVD-001, REQ-NF-NUM-001 |
| P4.4 | REQ-F-KVD-001 |
| P4.5 | REQ-F-KVD-001 |
| P4.6 | REQ-F-KVD-002 |
| P4.7 | REQ-F-ERROR-001, REQ-NF-NUM-002 |
| P4.8 | REQ-F-KVD-002, REQ-NF-NUM-001 |
| P4.9 | REQ-F-KVD-001, REQ-DC-KVD-001 |
| P4.10 | REQ-F-FILTER-001 |
| P5.1 | REQ-F-BIAS-001, REQ-F-CLI-002 |
| P5.2 | REQ-F-BIAS-002, REQ-F-CLI-002 |
| P5.3 | REQ-F-BIAS-001, REQ-NF-RNG-001, REQ-NF-LOG-001, REQ-NF-RECOVERY-001, REQ-F-CLI-002 |
| P5.4 | REQ-F-BIAS-001, REQ-D-MAT-002, REQ-I-RESULT-001 |
| P5.5 | REQ-F-BIAS-003 |
| P5.6 | REQ-F-BIAS-003 |
| P5.7 | REQ-F-BIAS-001, REQ-DC-EXPERIMENT-001 |
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
| P7.11 | REQ-F-ADAPT-001, REQ-DC-ADAPT-001 |
| P8.1 | REQ-F-EXP-001, REQ-F-CLI-003 |
| P8.2 | REQ-F-EXP-002, REQ-F-CLI-003 |
| P8.3 | REQ-F-ADAPT-002 |
| P8.4 | REQ-F-EXP-002, REQ-I-RESULT-001 |
| P8.5 | REQ-F-ADAPT-003, REQ-F-EXP-003, REQ-F-CLI-003 |
| P8.6 | REQ-F-ADAPT-003, REQ-F-EXP-003 |
| P8.7 | REQ-F-EXP-003, REQ-D-MAT-003, REQ-I-RESULT-001 |
| P8.8 | REQ-F-ADAPT-002, REQ-DC-ADAPT-001 |
| P8.9 | REQ-F-ADAPT-002, REQ-DC-ADAPT-001, REQ-NF-LOG-001, REQ-NF-SESSION-001 |
| P9.1 | REQ-F-AN-001, REQ-F-CLI-004, REQ-I-ANALYSIS-001 |
| P9.2 | REQ-F-AN-001, REQ-I-ANALYSIS-001 |
| P9.3 | REQ-F-AN-001, REQ-I-ANALYSIS-001 |
| P9.4 | REQ-F-BIAS-003, REQ-F-CLI-004, REQ-I-ANALYSIS-001 |
| P9.5 | REQ-F-AN-001 |
| P9.6 | REQ-F-AN-002, REQ-NF-HEADLESS-001, REQ-F-CLI-005 |
| P9.7 | REQ-F-AN-002, REQ-F-CLI-005 |
| P9.8 | REQ-F-AN-002, REQ-DC-ANALYSIS-001 |
| P10.1 | REQ-PROC-TRACE-001, REQ-NF-COPYRIGHT-001 |
| P10.2 | REQ-PROC-TRACE-001 |
| P10.3 | REQ-NF-NUM-001, REQ-NF-NUM-002, REQ-PROC-TRACE-001, REQ-NF-RECOVERY-001 |
| P10.4 | REQ-D-MAT-001, REQ-D-MAT-003 |
| P10.5 | REQ-NF-NUM-001, REQ-NF-RNG-001 |
| P10.6 | REQ-NF-PLATFORM-001, REQ-NF-DEPS-001, REQ-NF-LOG-001, REQ-NF-HEADLESS-001, REQ-NF-PORTABLE-001, REQ-NF-RUNTIME-001 |
| P10.7 | REQ-D-SOURCE-002, REQ-NF-COPYRIGHT-001 |
| P10.8 | REQ-PROC-TRACE-001 |
| P10.9 | REQ-PROC-TRACE-001, REQ-NF-OOP-001, REQ-NF-SESSION-001 |
| P10.10 | REQ-PROC-TRACE-001, REQ-NF-OOP-001 |

### 9.2 原始 MATLAB 45 文件逐文件追踪（清单已核实，代码未移植）

每行列出在原始 `origin/egomotion_estimation_dense_flow_fields_matlab_scripts/` 下的**真实相对路径**。相关 TC ID 与 REQ 的后缀一一对应，详见 TEST_PLAN；部分文件出现在多个原实验中，列出首要需求及直接相关需求。文件存在不代表 C++ 已移植或 MATLAB 已运行。

| 原始 MATLAB 文件 | 对应 REQ | 主要计划 TC | 源码状态/实现状态 |
|---|---|---|---|
| `adaptive_matched_filter_test/data_analysis/graph_agent_in_sphere_adaptive_matched_filter_many_parameter.m` | REQ-F-AN-001, REQ-F-CLI-004 | TC-F-AN-001 | 已定位 / 未移植、未测试 |
| `adaptive_matched_filter_test/data_analysis/graph_agent_in_sphere_adaptive_matched_filter_test.m` | REQ-F-AN-001, REQ-F-CLI-004 | TC-F-AN-001 | 已定位 / 未移植、未测试 |
| `adaptive_matched_filter_test/data_analysis/graph_agent_in_sphere_initialization_test.m` | REQ-F-AN-001, REQ-F-CLI-004 | TC-F-AN-001 | 已定位 / 未移植、未测试 |
| `adaptive_matched_filter_test/data_generation/agent_in_sphere_adaptive_matched_filter_many_parameter.m` | REQ-F-EXP-003, REQ-F-CLI-003 | TC-F-EXP-003 | 已定位 / 未移植、未测试 |
| `adaptive_matched_filter_test/data_generation/agent_in_sphere_adaptive_matched_filter_test.m` | REQ-F-EXP-002, REQ-F-CLI-003 | TC-F-EXP-002 | 已定位 / 未移植、未测试 |
| `adaptive_matched_filter_test/data_generation/agent_in_sphere_initialization_test.m` | REQ-F-EXP-001, REQ-F-CLI-003 | TC-F-EXP-001 | 已定位 / 未移植、未测试 |
| `bias_test/data_analysis/graphs_bias_test.m` | REQ-F-BIAS-003, REQ-F-CLI-004 | TC-F-BIAS-003 | 已定位 / 未移植、未测试 |
| `bias_test/data_generation/bias_test_nonspherical.m` | REQ-F-BIAS-001, REQ-F-CLI-002 | TC-F-BIAS-001 | 已定位 / 未移植、未测试 |
| `bias_test/data_generation/bias_test_spherical_prop_error.m` | REQ-F-BIAS-002, REQ-F-CLI-002 | TC-F-BIAS-002 | 已定位 / 未移植、未测试 |
| `help_functions/SHRotate.m` | REQ-F-SH-002 | TC-F-SH-002 | 已定位 / 未移植、未测试 |
| `help_functions/drehvektor_aus_zwei_vektoren.m` | REQ-F-TRAJ-001 | TC-F-TRAJ-001 | 已定位 / 未移植、未测试 |
| `help_functions/erzeugeflussausnear.m` | REQ-F-FLOW-001 | TC-F-FLOW-001 | 已定位 / 未移植、未测试 |
| `help_functions/erzeugekreuzproduktmatrix.m` | REQ-F-GEOM-001 | TC-F-GEOM-001 | 已定位 / 未移植、未测试 |
| `help_functions/erzeugerichtungsvektoren.m` | REQ-F-GEOM-002 | TC-F-GEOM-002 | 已定位 / 未移植、未测试 |
| `help_functions/erzeugerichtungsvektoren_loch.m` | REQ-F-GEOM-002 | TC-F-GEOM-002 | 已定位 / 未移植、未测试 |
| `help_functions/findeDreiecke.m` | REQ-F-GEOM-002, REQ-F-GEOM-003 | TC-F-GEOM-002 | 已定位 / 未移植、未测试 |
| `help_functions/fixed_kugel_symmetric_perfectpoints_randomnear.m` | REQ-F-FLOW-002, REQ-F-FLOW-003 | TC-F-FLOW-002 | 已定位 / 未移植、未测试 |
| `help_functions/fixed_propfehler_kugel_symmetric_perfectpoints_randomnear.m` | REQ-F-FLOW-002, REQ-F-FLOW-003 | TC-F-FLOW-002 | 已定位 / 未移植、未测试 |
| `help_functions/franzeigen.m` | REQ-F-FILTER-001 | TC-F-FILTER-001 | 已定位 / 未移植、未测试 |
| `help_functions/getflowfield.m` | REQ-F-MAT-003, REQ-I-MAT-001 | TC-F-MAT-003 | 已定位 / 未移植、未测试 |
| `help_functions/getnear_in_kugel_Drehung.m` | REQ-F-FLOW-004 | TC-F-FLOW-004 | 已定位 / 未移植、未测试 |
| `help_functions/getvektor.m` | REQ-DC-RNG-001 | TC-DC-RNG-001 | 已定位 / 未移植、未测试 |
| `help_functions/koeff_aus_near.m` | REQ-F-SH-001 | TC-F-SH-001 | 已定位 / 未移植、未测试 |
| `help_functions/kreis_trajektorie.m` | REQ-F-TRAJ-001 | TC-F-TRAJ-001 | 已定位 / 未移植、未测试 |
| `help_functions/kreuzprodukt.m` | REQ-F-GEOM-001 | TC-F-GEOM-001 | 已定位 / 未移植、未测试 |
| `help_functions/kugelflfunc2.m` | REQ-F-SH-001 | TC-F-SH-001 | 已定位 / 未移植、未测试 |
| `help_functions/kurveglaetten.m` | REQ-F-AN-001 | TC-F-AN-001 | 已定位 / 未移植、未测试 |
| `help_functions/mue_norm_koenderink_iteration.m` | REQ-F-KVD-001 | TC-F-KVD-001 | 已定位 / 未移植、未测试 |
| `help_functions/near_aus_koeff.m` | REQ-F-SH-001 | TC-F-SH-001 | 已定位 / 未移植、未测试 |
| `help_functions/nonpropfehler.m` | REQ-F-FLOW-003 | TC-F-FLOW-003 | 已定位 / 未移植、未测试 |
| `help_functions/norm_koenderink_iteration.m` | REQ-F-KVD-001 | TC-F-KVD-001 | 已定位 / 未移植、未测试 |
| `help_functions/normrelfehler.m` | REQ-F-ERROR-001 | TC-F-ERROR-001 | 已定位 / 未移植、未测试 |
| `help_functions/oneit_adaptiv_matched_filter.m` | REQ-F-ADAPT-001 | TC-F-ADAPT-001 | 已定位 / 未移植、未测试 |
| `help_functions/richtungsvektoren_icosahedron.m` | REQ-F-GEOM-003 | TC-F-GEOM-003 | 已定位 / 未移植、未测试 |
| `help_functions/rodrigues.m` | REQ-F-TRAJ-001, REQ-F-SH-002 | TC-F-TRAJ-001 | 已定位 / 未移植、未测试 |
| `help_functions/sinuskurve_trajektorie.m` | REQ-F-TRAJ-001 | TC-F-TRAJ-001 | 已定位 / 未移植、未测试 |
| `help_functions/skalarprodukt.m` | REQ-F-GEOM-001 | TC-F-GEOM-001 | 已定位 / 未移植、未测试 |
| `help_functions/trajektorie_aus_punkten.m` | REQ-F-TRAJ-001 | TC-F-TRAJ-001 | 已定位 / 未移植、未测试 |
| `help_functions/transfehler_div.m` | REQ-F-ERROR-001 | TC-F-ERROR-001 | 已定位 / 未移植、未测试 |
| `help_functions/transfor_koeff.m` | REQ-F-SH-002 | TC-F-SH-002 | 已定位 / 未移植、未测试 |
| `help_functions/winkelausvektor.m` | REQ-F-SH-001 | TC-F-SH-001 | 已定位 / 未移植、未测试 |
| `help_functions/zellendefinieren.m` | REQ-F-FILTER-001 | TC-F-FILTER-001 | 已定位 / 未移植、未测试 |
| `illustrations/circle_curve_in_the_sphere.m` | REQ-F-AN-002 | TC-F-AN-002 | 已定位 / 未移植、未测试 |
| `illustrations/sinosodial_curve_in_the_sphere.m` | REQ-F-AN-002 | TC-F-AN-002 | 已定位 / 未移植、未测试 |
| `illustrations/spherical_harmonic_illustration.m` | REQ-F-AN-002 | TC-F-AN-002 | 已定位 / 未移植、未测试 |

### 9.3 实际测试实体追踪

[TEST_PLAN.md](TEST_PLAN.md) 中给出全部需求的候选测试源文件、CTest 前缀与人工验收方式；这些候选实体尚未创建，真实源文件路径、测试函数、Golden SHA、执行命令与通过状态须由 P1–P10 逐步回填。不允许用 TC 编号存在代替测试已实现。

### 9.4 待冻结的参数化验收合同（增强说明）

- **TC-F-KVD-001** 的具体 MATLAB 固定输入来源、可验证的逐轮中间量字典、误差容差必须在 [TEST_PLAN.md](TEST_PLAN.md) 和 [design/kvd.md](design/kvd.md) 共同确认。单独重绘论文图像不构成对 KvD 数值等价的证明。
- **TC-F-EXP-003** 的当前 MATLAB 输出文件（3 个）与历史归档（6 个）保留分开的契约；差异表字段采用 文件路径、MAT 变量名、rank/dims、当前生成脚本是否写入、历史文件是否存在、分析脚本是否消费、归因状态、证据 SHA。
- **TC-F-AN-001** 的输入曲线每个点必须能回溯到 MAT 变量和 MATLAB 索引，输出的结构化数组需要携带 axis/unit/segment/smoothing 元数据。绘图配色和 PNG 像素级一致不是验收依据。
- 上述 AC 对象状态均为“规格进一步明确，尚未验证”；任何无法找到原 MATLAB Golden、未批准 DEC-005 或缺实际测试的条目保持阻塞/未通过。

### 9.5 matio 读取证据与未完成需求的边界

- **已运行**：`src/mat_data_dict_probe.cpp` 在 Ubuntu 24.04、matio 1.5.26 上运行；正式字典记录源参数 SHA `48c590ba2564e4bdd293d2cbecd2eef83b291452`，582 个文件/变量均 `READ_OK`，状态见 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md)。
- **已核对**：MAT class/dtype、rank/dims、压缩状态、变量名/文件名及逐文件物理长度一致（当前全部为 double），这不是固定输入浮点数值与 MATLAB 逐元素一致的证明。正式报告采用 FNV-1a 64 位指纹用于变更检测，**不是 SHA-256**。
- **仍未验证**：正式业务 Reader 的严格/兼容加载、读取后生命周期、错误负例、原始数据前后加密哈希、RunMatWriter、历史数据与 MATLAB save 的语义对应、Golden 和数值容差。项目 SRS/HLD/LLD 均保留“草案/待批准”状态。

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

本文档为**需求草案，不是已冻结基线**。原始 MAT 的格式、变量及形状已经通过独立 matio 全量探针读取确认；但版权许可、业务接口、Golden、数值容差和算法/实验复现仍待完成与批准，不得将扫描器成功视为软件整体验收成功。

### 草案修订记录

| 版本 | 内容 | 基线状态 |
|---|---|---|
| v0.2 | REQ/AC/TC 分层与 98 项 P 任务追踪；数据合同和 CLI 候选 | 未冻结 |
| v0.3 | 明确 582/582 独立 matio 读取的已验证范围；配套静态追踪检查器 `scripts/check_docs_traceability.sh`；不改变任何必须需求的验收门槛 | 未冻结；仍待项目所有者批准 |
