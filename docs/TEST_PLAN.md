# 测试设计与验收计划（TEST PLAN）

> **测试计划草案 v0.15，未冻结。** 本文件是 66 项 TC→候选测试实体/CTest→AC 的唯一规范性登记；当前对应的 SRS/HLD/LLD 版本与文件 SHA 在 [文档控制矩阵](DOCUMENT_BASELINE_REGISTER.md) 中定位。实际命令、结果和通过数量仅由 [PROGRESS.md](PROGRESS.md) 与 [reviews/](reviews/) 的固定 SHA 证据记录。

> **审批后候选核对材料**：见 [HAR-2026-11-06-001 设计修订与待审材料包](approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md) §2、§3、§7～§9。其中历史上提出六类容差采集格式、582 MAT SHA-256 独立审核和正式 Reader 类型拒绝清单；**当前参考采集/核验路径以 L2/L3/L4 为准，完全不要求 MATLAB/Octave 执行或 L1 Golden**，见 [长期政策](golden/NO_MATLAB_EXECUTION_POLICY.md)。`atol=0.01`/`rtol=0.1` 已由所有者选作**首轮宽松筛查**，不是已冻结的最终验收阈值；测试实体/CTest 仍未执行，H-10/H-11 其他执行门禁未解除。

> **长期约束**：[无需运行 MATLAB 政策](golden/NO_MATLAB_EXECUTION_POLICY.md) 已取消 MATLAB/Octave 运行与 L1 Golden 的一切强制取证前提；现行测试参考为 L2 已存 MAT、L3 独立解析/重实现和 L4 统计。此决定不解除代码/CTest 的实际执行门禁，也不批准数值容差。

> **草案，未冻结。** 测试标识 TC 均为拟定名称；下面的测试文件、CTest 前缀均为未来可能的产物，尚未建立。需求参见 [REQUIREMENTS.md](REQUIREMENTS.md)，实施参见 [PLAN.md](PLAN.md)。已编写并在 Ubuntu 24.04 上编译运行**独立 C++17 matio 数据字典探针**，582/582 MAT 均 `READ_OK`；**未创建完整业务 C++/CTest，也未执行业务级或数值 Golden 测试**。

**实际执行和最终阈值可推迟**：所有需项目所有者授权的 C++ 修改、Bash/CMake/CTest、隔离 MAT 写回读、DEC-005/006、基线/发布记录统一见 [TODO.md](TODO.md)；本测试计划只规定将来测试什么与保留什么证据，不在当前纯文档阶段请求运行权限。

## 1. 验证分层和前提

- 顺序：源文件审计及 MAT 格式核验 → 单元测试 → 合同测试 → 入口集成测试 → L2 最终 MAT/L3 独立逐节点 Golden 比对 → L4 论文统计复现。
- 确定性数值验收：同一固定输入比较逐步计算和误差；随机统计：独立统计有效/失败样本、抽样配置及噪声；不要求 MATLAB 与 C++ 同 seed 的 randn 逐位一致。
- 所有测试须记录 RunId、源 Git SHA、数据文件哈希、实际 Debian/Ubuntu/编译器/依赖版本、命令、参数、随机种子、预期与实测、误差、退出码及日志。
- 每条 M 级需求应有验收条件与至少一个真实测试或经批准的人工核查；任何尚未执行的用例不能宣称通过。

**2026-11-07 已选定策略**：按 [最新项目决定](decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md)，所有新输出路径固定、采用轻量封装的 Result/Status 与 JSONL、独立 Runner 配置及失败/部分结果状态，球谐 Eigen-only 且等待用户上传 1996 原文与 1998 勘误。本轮只修改设计，不运行 MATLAB、Bash、CTest 或写 MAT。现有 TC 编号/数量不变。

## 2. 首批关键测试的具体条件

### 2.1 CLI 契约（TC-F-CLI-001）

候选子命令必须分别核验帮助、必填参数、默认值、正常退出、缺参数、非法参数、输入文件错误、输出覆盖保护。**具体候选语法、参数类型/范围、退出码与诊断字段已写入 [design/cli.md](design/cli.md)**；P0.8/P1.2 仍需批准并以实际 CLI/CTest 验证，不得把文档视为实现。

| 子命令 | 候选必填参数 | 输出/验收 |
|---|---|---|
| inspect-mat | --file PATH | MAT 变量名、rank、dims、type、压缩/错误诊断 |
| run-bias-occluded | --seed UINT32 --level NONNEG_INT | xwerte、ywerte1…6 的 MAT 结果 |
| run-bias-proportional | --seed UINT32 --level NONNEG_INT | 独立完整球面/逐分量噪声结果 |
| run-adaptive-init | 无 | 原始初始化四类误差结果 |
| run-adaptive-comparison | --noise-scale FINITE_DOUBLE | 带噪/无噪的原始五类 MAT 结果 |
| run-adaptive-update-rates | 无 | 1/5/11/21 帧更新频率结果 |
| analyze | --experiment NAME --input-root DIR | 独立读取历史 MAT，计算统计序列 |
| plot（可选） | --experiment NAME --input-root DIR | 图表结果；无图形后端不影响 analyze |

**统一 CLI 输出约定（项目所有者已决定）**：用户配置输出根的 CLI 选项已删除；新结果固定写入仓库根 `output/<RunId>/`。`--run-id ID` 可选，缺省独立 OS 随机生成，不消费实验 RNG；每个目录与文件均独占创建，重复 RunId/路径逃逸/指向 `origin/**` 均拒绝。纯读取/分析命令保持只读；`plot` 只有保存结果时才分配 RunId。真实 C++/CTest/安全负例仍待后续获准实施。

建议退出码类别（尚未批准）：0 成功、2 参数错误、3 MAT/输入 I/O 错误、4 格式/类型/形状错误、5 数值无效、6 输出目录保护、7 可选功能不可用。实施前冻结状态优先级与错误含义。

### 2.2 Bias 噪声（TC-F-BIAS-001、TC-F-BIAS-002）

原脚本三项系数是 [0.6, 2.4, 9.6]。非球面情形按 randn × 系数 × 全场平均光流模长逐分量扰动；完整球面情形按 randn × 系数 × 对应光流分量扰动；两者都要将流重新投影到切向。**不得将系数开平方**。用固定 Gaussian 序列（含零、正负非零值）对每个分量验证，核对六组误差和 xwerte MAT 变量尺寸。

### 2.3 球面采样顺序（TC-F-GEOM-002、TC-F-GEOM-003）

分别对完整八面体 8·4^i、缺损八面体 6·4^i、二十面体 20·4^i 在 i=0、1、2 核对数量、单位范数、缺损面及**相同顺序下的每行三维方向**；不能只比较无序集合或哈希。Golden 输入应由原只读源码面表/递归规则**独立生成并交叉核验 L3**，覆盖 i=0、1、2 全部有序方向；不得由被测 C++ 自算，数值容差经 DEC-005 批准，**无需运行 MATLAB**。

### 2.4 600 点 Adaptive 轨迹输出（TC-F-EXP-002）

原始 agent_in_sphere_adaptive_matched_filter_test.m 采用 stellen=600、anzahliterationen=4、anzahldurchlaeufe=8、帧循环 i=2:599。当前代码明确 save 五个变量：allemittransfehler、allemitrotfehler、alleohnetransfehler、alleohnerotfehler、rottransratio，均保存为同名 MAT 文件。前四项为八次运行平均得到的 600×1 缓冲；rottransratio 仅保留代码在运行循环中最后赋值的 600×1 缓冲，不能擅自平均。with_errors 历史 `alleadap*` 按 [DEC-004](decisions/DEC-004_HISTORICAL_MAT_FIELDS.md) 标为 `HistoricalOnly`、只读保留；新 C++ 结果严格只生成五项当前有效 save 字段，**不以历史来源未明阻塞开发**。

### 2.5 原始 MAT、数值容差和失败判定

**数据字典已通过独立 C++ matio 探针实际扫描**：Ubuntu 24.04 / matio 1.5.26，582/582 源 MAT 全部被逐变量 `Mat_VarRead` 解码，582 个变量均 `READ_OK`，失败 0。原始目录以 `MAT_ACC_RDONLY` 打开；见 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md)。P0.5/0.6 的文档合同、正式业务 Reader/Writer 与负例仍需测试；P10.4 独立复核尚未完成。

对 Original/Modified KvD 的 near、t、ω、终止条件逐轮比对；Adaptive 比对 6×6 系统各子块、右端、旋转、残差与球谐系数更新。 **特别核验：Adaptive oneit 返回的 error 是 MSE，sqrt(error) 在 MATLAB 内部另算但没有返回；测试记录必须分列 error_mse 和仅用于诊断的 rms，不得将二者互作。** `double` 为数值精度要求；**历史** `rtol=1e-10` 不是现行容差候选。已决定的首轮探索筛查为 `atol=0.01`、`rtol=0.1`，另对物理不变量、近零理想残差和离散行为保留严格检查；参阅 [Golden/容差提案](golden/GOLDEN_TOLERANCE_PROPOSAL.md)。须按模块记录绝对/相对误差、条件数及样本有效性，DEC-005 审批前不得作为通过阈值。不对零向量、NaN、Inf 或未定义方向误差计入成功。

**第一轮静态分支判别补充（不新增 TC/CTest）**：TC-F-KVD-001/002 除常数 near 场景，还须以同一固定流场/Franz 初态设计**非恒一近度**，分别比较 Original 的 `⟨μp⟩/⟨μd⟩/⟨μ²(t·d)d⟩` 与 Modified 的 `⟨p⟩/⟨d⟩/⟨μ(t·d)d⟩`，并记录 `ω_new` 使用旧 t 的顺序；参阅 [KvD §9.1](design/kvd.md)。TC-F-ADAPT-001/002 须覆盖正/负/零原始 near 均值，特别在 `s<0` 时归一后均值应为 **−1**（按原先翻 near、再除负 s 公式）；参阅 [Adaptive §10.1](design/adaptive_mfa.md)。这些是源码静态推导的**未来夹具资格条件**，并非已形成完整独立 L3 或通过测试。


### 2.6 已执行 matio 全量探针的证据与限制

| 证据项 | 实测结果 | 当前验收边界 |
|---|---|---|
| 平台与工具 | Ubuntu 24.04，GCC 13，matio 1.5.26；源参数 SHA `48c590ba2564e4bdd293d2cbecd2eef83b291452` | 独立源文件 `src/mat_data_dict_probe.cpp` 已编译运行；非正式 CMake/CTest |
| 读取 | 582/582 MAT、582/582 变量、失败 0 | `Mat_Open(MAT_ACC_RDONLY)` + `Mat_GetDir` + `Mat_VarRead`；未做数据逐元素 L2 原 MAT 值与 L3 独立参考对照 |
| 格式与类别 | 全部 MAT v5 / zlib / MATLAB double、存储 double | 当前数据不需要 HDF5、cell、struct、complex、sparse 适配；新增数据另评审 |
| 形状分布 | 10×1:80，10×3:480，35×1:4，600×1:14，600×4:4 | 真实历史结果 shape；不代表所有未来输出或算法形状 |
| 文件身份 | 582 个文件各有 FNV-1a 64 位指纹 | FNV 非 SHA-256；尚须 P10.4 独立目录/文件身份审核 |
| 未运行的验收 | 严格/兼容读取、类型拒绝负例、写回读、RAII 下异常、原文件哈希比对、C++ 求解器/CTest | 不能用探针的 exit=0 将这些 TC 标为通过 |

详细记录以 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) 的 582 条逐变量表为准，以下的 TC 状态仅表明**相关子能力有实测证据**，而非整个接口合同正式验收。

### 2.7 更新频率实验与四种分析入口（TC-F-EXP-003、TC-F-AN-001）

- 更新率当前脚本：600 点、二十面体等级 2、**1/5/11/21 帧**，输出 600×4 的 allemittransfehler、allemitrotfehler，以及 600×1 的 akrot；历史 many_parameter 额外 `alleadaprotfehler`、`alleadaptransfehler`、`rottransratio` 按 **DEC-004** 标 `HistoricalOnly`，新结果只保存两个 `allemit*` 与 `akrot` 三项；针对新结果进行需要 `rottransratio` 的分析时必须**明确报告缺失而不是构造比值**。
- 初始化分析图：allemittransfehler(1:13,1) 与 allemitrotfehler(:,1) 的数值、索引、对数纵轴语义；数据被读取不代表一定进入图线。
- 比较分析图：x=50:550 的旋转与平移方向角度误差、rottransratio 双纵轴，阈值 0.01，分段规则精确依据原 MATLAB。
- 更新频率分析图：x=50:550、阈值 0.02，四条曲线与不同半窗 2/5/10 的平滑；kurveglaetten 的边缘零值语义不替换。
- 每条曲线的**L2 输入/L3 独立计算参考**应记录原始 source MAT+variable 和 SHA-256、原只读源码中的 1-based 索引规则、x/y 数组、axis/units、分段、窗宽及有效样本状态；**不运行 MATLAB**，只比较图片像素不能算通过。

**DEC-004 已选的历史字段兼容验收**（参见 [决定原文](decisions/DEC-004_HISTORICAL_MAT_FIELDS.md)）：以 L2 的 `with_errors` 七份文件与 `many_parameter` 六份文件证明历史字段只读存在；以独立 L3 小规模生成案例和未来 C++ 运行验证**新 comparison 仅五类 save、新 update-rates 仅三类 save**，两类历史 `alleadap*` 以及新 update-rates 不保存的 `rottransratio` 必须标 `HistoricalOnly`，不得写入新的 MAT。对新 update-rates 结果运行需要 `rottransratio` 的分析必须**确定性失败并报告 MissingVariable/来源类别**，旧历史目录仍能读取。关联 TC-F-EXP-002/003、TC-D-MAT-003、TC-F-AN-001/002、TC-I-ANALYSIS-001；**没有新增 TC ID，且所有新测试均未执行**。这套处理规则已经确定，不再等待历史额外字段归因。

### 2.8 用户审批 JSON 的数值与执行验证边界（仅规划）

**已补充具体 Golden/容差候选，但仍未测试或批准**：查看 [Golden 参考样例与宽松容差建议](golden/GOLDEN_TOLERANCE_PROPOSAL.md) 和 [12 组解析候选 JSON](golden/analytic_golden_candidates.v1.json)。其中给出 12 组根据原 MATLAB 公式静态推导的固定输入/输出（含有序采样、光流、球谐、90° 转动、误差计算、Adaptive 单步）和第一轮探索性建议 `atol=0.01, rtol=0.1`。**这些是未执行的独立 L3 候选，不代表最终 DEC-005 PASS 阈值**；离散结构、错误状态、球面/旋转不变量另有严格条件。一般 KVD/Adaptive 逐轮及 RNG 测试须补齐 L3 独立固定输入、逐节点与调用次数参考，最终结果/统计使用 L2/L4；不要求 MATLAB 采集，H-10/H-11 暂缓状态不变。

审批记录见 [DECISION_IMPLEMENTATION_NOTES.md](approval/DECISION_IMPLEMENTATION_NOTES.md)。**历史 H-01 在原审批记录中是 `draft`、`referenceConfirmed=false`，并未批准任何数值阈值**；H-03 也只是 CLI/错误/TC→参考数据方案草案，历史 H-09 不再要求 MATLAB 运行。项目所有者**后来另行确定** `atol=0.01`、`rtol=0.1` 仅为**首轮探索性筛查**，不等于 H-01 已签字或 DEC-005 最终模块 PASS 门槛已获批准；不能把被测 C++ 输出当独立真值或将未创建的测试标为通过。

| 将来需要的对照 | 预定输入/观察量 | 当前状态 |
|---|---|---|
| H-02 Sophus 取代 Rodrigues | 原 MATLAB 固定 `ω=0`、小角低于 eps、x/y/z、复合、接近 π；核对 3×3 物理旋转、Rᵀ 局部运动 | **已选路线，未运行** |
| Ivanic–Ruedenberg 原论文/1998 勘误驱动的 Eigen 球谐旋转（H-14 当前路线） | 对原 `SHRotate(r,2)` 的 R0=1、R3 全 9 项、R5 全 25 项及九个有效球谐系数进行独立 L3 对照；核查 m 顺序、相位、轴/基底排列、正向/逆向旋转，并确认生产依赖不含 sdla | **数学/实现路线已定，Don Williamson 仅供代码参考；原论文 PDF 待用户上传，未实现、未编译、未执行 CTest，L3 参考未完整核验** |
| H-12 `output/<RunId>/` 结果及负例 | 原 `origin/**` 前后哈希不变；重复 RunId、软链接越界、异常读写、MAT 写后回读、运行输出排除在 Git 上传之外 | **只完成忽略规则的文档/配置，H-10 暂缓、writeMat=false，未运行** |
| H-04 Ubuntu VM 资源快照 | 用户提供的 `uname/df/free`；将来记录实际 CMake/CTest 和 peak RSS/wall/CPU | **只有环境资料，未批准时间/性能门槛** |

> **H-10=deferred、H-11=deferred**；本次 JSON 的 `runCMake/runCTest/runMatlab/writeMat/runBash/measurePerformance` 全为 false。没有现场命令、运行结果或数值测试可在本轮回填。审阅/格式检查不触发工具执行授权。

**参考术语的唯一解释**：本文件中历史标题/测试名如出现 `Golden`，除明确标明“历史 L1 分类”外，都必须逐个注明为 **L2（原作者现存 MAT）/L3（独立固定输入数值）/L4（统计）**；**L1 原 MATLAB 运行数据永久不作为强制取得/通过条件**。有编号不表示已有测试函数，唯一候选实体矩阵见 §3，真实执行证据只写入 [PROGRESS.md](PROGRESS.md)/[reviews/](reviews/) 的指定 Git SHA 记录。

### 2.9 第一轮新增的可复核测试夹具规范（AF-051，尚未建真实测试）

下表仅把模块 LLD 中新增的固定输入/预期映射到**既有 TC**，不改变 §3 的 66 项编号或将建议的测试文件说成已创建。

| TC 组 | 应准备的独立夹具/重要断言 | 预期证据 |
|---|---|---|
| `TC-F-MAT-003/004`、`TC-NF-SAFETY-001`、`TC-DC-IO-001` | 多变量歧义、固定 `double[2×3]` 列主序、符号链接父目录/并发 RunId/非覆盖、写入失败及独立回读 | 隔离测试目录、源文件不变、具体 MatErrorCode、manifest/log 与逐元素比较；详见 `mat_io.md §8.1` |
| `TC-F-GEOM-002/003`、`TC-NF-RNG-001`、`TC-DC-RNG-001` | 完整/缺损八面体 levels 0/1/2，有序行；固定注入时先 N 次 uniform 后 3N 次 normal；0 噪声仍抽样 | 原源码 SHA、独立方向 L3/固定 RNG 输入、调用数/顺序、非有限与耗尽负例；详见 `sampling_noise.md §7.1` |
| `TC-F-TRAJ-001`、`TC-F-FLOW-004` | 35/600 轨迹、N 分母、端点无有效运动、`R^\top`、小角 epsilon/反向/近 π、球内正根 | 全矩阵独立 L3、状态/维度、球外失败分类；详见 `trajectory.md §6.2` |
| `TC-F-EXP-002/003`、`TC-I-RESULT-001`、`TC-NF-RECOVERY-001` | 8 次四误差平均而 ratio 只留第 8 次、单轮失败/Manifest、周期刷新和当前三个 save | 独立固定八轮和帧号、RunId、保存字段/源只读确认；详见 `experiments.md §8.2` |
| `TC-F-AN-001/002`、`TC-I-ANALYSIS-001` | 50:550 的 501 点、阈值等值不触发；多更新率**旋转逐段平滑且裁剪**、**平移先全长平滑且不分段**；comparison 平移不分段；缺 ratio 明确报错 | 独立 L3 对比段级完整中间数组、绘制 x/y、连续平移 501 点和原值来源；不能以“正确裁剪后的旋转内部值不同于全局平滑”作假断言。详见 `analysis_plotting.md §5.2.1/§8.1` |

**不能自动宣称 PASS**：测试夹具未生成，运行许可 H-10/H-11 未开放，源输入/参考 SHA、逐值误差及 CTest stdout/stderr/exit 尚无本轮执行记录。本表仅完成 AF-051 的测试**设计部分**，AF-051 继续「未整改」。

### 2.10 已可手算但未实际执行的数值/抽样负例（AF-041/047/055）

根据 [AN-G 静态公式审阅](reviews/AN_G_STATIC_FORMULA_REVIEW_2026-11-08.md) 可用于未来独立 L3 的固定条件：

| 原 TC/验证目标 | 固定条件及不可忽略的判据 | 当前实际状态 |
|---|---|---|
| TC-F-GEOM-001 / TC-NF-RNG-001 | `getvektor` 注入两个 uniform `(0.5,0)`，方向严格按 θ/φ 公式算，消耗 2 次；还须覆盖边界输入 0/1、耗尽与非法样本 | 仅静态推导；未运行 |
| TC-F-FLOW-001～004 | `d=e_z, near=2, t=e_x, ω=e_y` 的生成光流 `(-3,0,0)`；另对 `nonpropfehler` 的 **0/+k/−k** 固定正态注入，记录每行 3 次调用与最终切向投影 | 数值点已手算；独立 L3/CTest 缺 |
| TC-F-BIAS-001/002 | `iterationnumber=2` 的原 `xwerte=[6,24,96]`，缺损模式 N=6/24/96，完整球面 N=8/32/128；必须记录实际 N 与显示 x 分别来源 | 仅源表达式/形状复核；未运行 |
| TC-F-SH-001、TC-F-ADAPT-001 | 常数近度 `Y00` 代数与对称方向下 M1～M4/rhs 的关系；另需非对称/近奇异例和 R3/R5 全元素、Session trace | 对称情况仅理论复核，不作数值 PASS |

所有例子不新增 TC ID，不替代现有 66 TC 的文件创建、测试执行、L2/L3/L4 独立证据及 DEC-005/008 正式审批。负有限噪声的 CLI 接收策略仍须批准；原 `.m` 未显式禁止该数值。

### 2.11 分析分段与平滑数据域的区分性测试（AF-075/076，规划）

同一组有 **4 个有效旋转区间**的合成 ratio 与非恒定误差输入，应建立两类独立 L3：

1. 多更新率**旋转图**：分段时读取每个区间切片（列 2/3/4），每段分别调用窗口半宽 2/5/10 的平滑；保存完整的局部零边界和裁剪后 x/y，不允许将源逻辑写成全局预处理。若全局平滑后按 **同样有效内部区间**裁剪，内部均值应**一致**，故不得将差异作为正确性断言。要查的是窗口调用域、完整临时数组、段边界与异常/日志身份。
2. 多更新率**平移图**：四列各以全部 600 行构造数据（前三列平滑半宽 2/5/10，第一列原样），统一绘制 **50:550 共 501 点**、无分段掩码；comparison 实验平移 mit/ohne 亦全区间不分段，旋转图另按阈值分段。必须选有阈值穿越和间隙的 ratio，确保平移线**不会丢掉中间 x**。

沿用 `TC-F-AN-001`、`TC-I-ANALYSIS-001`、`APL-G04/05`，不新增 TC ID/测试 PASS；完整独立参考、Analyzer 和 CTest 尚未建立。旧扫描报告中关于“正确裁剪后的旋转内部值必然不同”的说法已在 [纠偏说明](reviews/AUDIT_ANALYSIS_SEGMENT_SMOOTHING_CORRECTION_2026-11-08.md) 限定，旧扫描仍保留历史原文。

### 2.12 分析错误枚举到 CLI 退出码的合同负例（AF-077，未测试）

根据 [contracts.md §3.1](design/contracts.md) 的**唯一候选**映射设计以下输入与 `stage/exit` 断言（非已经执行）：

| 原 TC | 固定触发条件 | 预期候选返回 | 当前状态 |
|---|---|---|---|
| TC-I-ANALYSIS-001 | 新 update-rates 结果不含历史 `rottransratio`；或指定目录缺实际 MAT | `AnalysisErrorCode::MissingVariable/MissingResult`，`read` / exit 3；记录原 path/variable/source_error_code | 仅计划 |
| TC-I-ANALYSIS-001 | MAT 解码后 shape 不能满足源曲线合同 | `AnalysisErrorCode::ShapeMismatch`，`decode` / exit 4 | 仅计划 |
| TC-F-AN-001、TC-I-ANALYSIS-001 | ratio 形成非法分段、分析值非有限或统计未定义 | `InvalidSegment/NonFiniteInput/UndefinedStatistic`，`analyze` / exit 5 | 仅计划 |
| TC-I-CLI-001、TC-I-ANALYSIS-001 | 非法 CLI `--experiment`；与合法命令内部传给 Analyzer 的 unsupported mode 分开 | CLI 无效枚举 `parse` / exit 2；运行期 `AnalysisErrorCode::UnsupportedExperiment` 为 `analyze` / exit 5 | 仅计划 |

本表不重新定义 enum、增删 66 个 TC 或签署 DEC-008；真实接口、JSONL 转义/底层原因传播与 CTest 待后续授权和证据。

### 2.13 轨迹与近度的符号级候选（AF-048/080，未运行）

参见 [trajectory.md §6.3](design/trajectory.md) 的 TR-C01～04：三点局部轨迹 `Rᵀ` 的正负号、严格反向输入 `(e_x,-e_x)` 的原函数**确定**输出零旋转向量（非有效折返），以及球内旋转视线的 `near=4/3` 与反向射线 `near=4/5`。将四组固定输入关联现有 `TC-F-TRAJ-001` 与 `TC-F-FLOW-004`；要求实际 L3/CTest 分别报告源返回/候选安全失败、端点有效性、坐标单位和真实误差。本文只给出静态可手算输入资格，**未生成独立 L3、未测试，也不批准 DEC-005/008**。

### 2.14 AN-G01～03 初始面表行序的静态资格（AF-047/055）

源码面表按 `level=0` 的三顶点单位中心公式，已经形成 [AN-G01/02/03 完整 8/6/20 行符号推导](reviews/AN_G_LEVEL0_FACE_ORDER_STATIC_2026-11-08.md)；关联原 `TC-F-GEOM-002/003` 的初始面顺序及零光流列。**仍不能据此通过 TC 或关闭 AF-047/055**：下一阶段必须覆盖 `level=1/2` 的全量 A/B/C/D 递归顺序、独立数值来源 SHA-256、边界输入及实际采样器/CTest。无 MATLAB/L1 强制门槛。

## 3. TC → 实体/CTest → AC 规划矩阵

下列表格共 66 项，与 SRS **v0.14 草案**逐项对应；MAT I/O 的拟议数据结构、独立负例及测试文件映射详见 [mat_io.md §2、§5、§8–9](design/mat_io.md)。**独立 matio 探针已编写并执行**，但绝大多数拟议测试源文件、测试函数和 CTest 名称尚未创建；探针不是 CTest 注册项。人工审查项与自动测试区分。原始 45 个 MATLAB 源文件映射请参见 [REQUIREMENTS.md §9.2](REQUIREMENTS.md)。

**L2/L3/L4 来源及测试文件唯一索引**：`contracts.md §3/§3.0`、`experiments.md §8.1`、`harmonics.md §4.1`、`sampling_noise.md §8`、`mat_io.md §5/§8` 仅补充对应模块的候选测试细节。本表仍是 66 项 TC 的单一归档，不产生新编号；其 test 文件、CTest 名称与来源证明尚未创建/批准，H-10/H-11 实际执行权限保持暂缓。

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
| TC-DC-DOMAIN-001 | REQ-DC-DOMAIN-001 | 现有：[OBJECT_MODEL.md](OBJECT_MODEL.md)、[REQUIREMENTS.md §9.2](REQUIREMENTS.md) 的 45 源文件追踪；现有：[SOURCE_INVENTORY.md](SOURCE_INVENTORY.md)（45 个源文件的静态 Git 身份；非测试通过） | 人工设计/来源评审；部分接口需合同测试 | AC-DC-DOMAIN-001 | 未编写/未运行 |
| TC-DC-IO-001 | REQ-DC-IO-001 | **候选** tests/contracts/test_mat_raii.cpp；现有：[OBJECT_MODEL.md](OBJECT_MODEL.md)、[MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) 的归档证据；现有：[SOURCE_INVENTORY.md](SOURCE_INVENTORY.md)（45 个源文件的静态 Git 身份；非测试通过） | **候选** egomotion.mat.raii.* ＋人工设计/来源评审 | AC-DC-IO-001 | 未编写/未运行；MatDenseArray 自拥有/移动、ErrorDetail 传播、句柄释放待验证 |
| TC-DC-RNG-001 | REQ-DC-RNG-001 | **候选** tests/unit/test_sampling.cpp ＋ tests/unit/test_flow_simulation.cpp；现有：[OBJECT_MODEL.md](OBJECT_MODEL.md)、[REQUIREMENTS.md §9.2](REQUIREMENTS.md)；现有：[SOURCE_INVENTORY.md](SOURCE_INVENTORY.md)（45 个源文件的静态 Git 身份；非测试通过） | **候选** egomotion.geometry.* / egomotion.flow.* ＋人工设计评审（依 [sampling_noise.md §8](design/sampling_noise.md)） | AC-DC-RNG-001 | 未编写/未运行；固定 RNG 值、调用数/耗尽和上下文待验证 |
| TC-DC-NOISE-001 | REQ-DC-NOISE-001 | **候选** tests/unit/test_flow_simulation.cpp；现有：[OBJECT_MODEL.md](OBJECT_MODEL.md)、[REQUIREMENTS.md §9.2](REQUIREMENTS.md)；现有：[SOURCE_INVENTORY.md](SOURCE_INVENTORY.md)（45 个源文件的静态 Git 身份；非测试通过） | **候选** egomotion.flow.* ＋人工设计评审（依 [sampling_noise.md §8](design/sampling_noise.md)） | AC-DC-NOISE-001 | 未编写/未运行；两噪声模型、错误上下文和样本数待验证 |
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
| TC-NF-COPYRIGHT-001 | REQ-NF-COPYRIGHT-001 | 现有：[OBJECT_MODEL.md](OBJECT_MODEL.md)、[所有者技术决定](decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md)、[LICENSE](../LICENSE)；现有：[SOURCE_INVENTORY.md](SOURCE_INVENTORY.md)（45 个源文件的静态 Git 身份；非测试通过） | 人工设计/来源评审；部分接口需合同测试 | AC-NF-COPYRIGHT-001 | 未编写/未运行 |
| TC-F-CLI-002 | REQ-F-CLI-002 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-F-CLI-002 | 未编写/未运行 |
| TC-F-CLI-003 | REQ-F-CLI-003 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-F-CLI-003 | 未编写/未运行 |
| TC-F-CLI-004 | REQ-F-CLI-004 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-F-CLI-004 | 未编写/未运行 |
| TC-F-CLI-005 | REQ-F-CLI-005 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-F-CLI-005 | 未编写/未运行 |
| TC-I-CLI-001 | REQ-I-CLI-001 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-I-CLI-001 | 未编写/未运行；必须验证已删除的输出根选项被拒绝（exit 2）、固定 `output/<RunId>/`、独立 OS RunId、JSONL、稳定 ErrorStage |
| TC-I-MAT-001 | REQ-I-MAT-001 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-I-MAT-001 | 未编写/未运行 |
| TC-I-RESULT-001 | REQ-I-RESULT-001 | tests/integration/test_adaptive_experiments.cpp | egomotion.experiments.* | AC-I-RESULT-001 | 未编写/未运行 |
| TC-I-ANALYSIS-001 | REQ-I-ANALYSIS-001 | tests/golden/test_analysis.cpp | egomotion.analysis.* | AC-I-ANALYSIS-001 | 未编写/未运行 |

**来源资料状态（AF-063）**：当前可使用 [SRS 的 45 个 .m 路径/REQ/TC 映射](REQUIREMENTS.md)、[对象模型](OBJECT_MODEL.md)、[MAT 数据字典](MAT_DATA_SCHEMA.md) 和已选技术决定；P0.6 的 [SOURCE_INVENTORY](SOURCE_INVENTORY.md) / [MIGRATION_MATRIX](MIGRATION_MATRIX.md) **已有文件级静态草案并与 45 个原源路径对应**；但这不构成逐函数审计、人工来源批准、软件测试或 AC 通过。上述五行保留原 TC/REQ/AC 编号，真实测试文件、来源验收与 CTest 仍未执行。

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
| T3（必须、L3 独立数值参考） | KvD 逐轮、SH 旋转块、Adaptive M1–M4 和 MSE | **独立 L3 固定输入逐节点参考**与批准的 DEC-005 容差；无需 MATLAB | 仅独立参考缺失或未经批准容差时阻塞；不能用被测 C++ 自算结果替代 |
| T4（必须、实验复现） | 两种 Bias、三种 Adaptive、四类分析及原论文曲线 | P5/P8/P9/P10 入口实际可执行后；性能资源按首轮实测安排 | 保留统计分母、失败/无定义样本、运行命令和真实耗时 |
| T5（应该、附加） | 可选绘图、额外编译器/CPU 组合、扩大性能测试 | 在必须需求有证据后，按项目所有者授权分期 | 明示未执行，不阻塞未要求的功能 |

**资源责任分工**：项目所有者批准需要运行的全量试验和数据使用范围；开发负责人提供测试入口与提交 SHA；数值验证负责人准备有来源的 Golden；维护/集成负责人核对目标 Debian/Ubuntu 工具链和 CTest 证据。当前没有经确认的设备型号、执行预算或时限，因此不编造 CPU 核心数、RAM 容量、分钟数、覆盖率及测试费用。

## 5. 原始 MAT、L2/L3/L4 参考数据管理与来源策略

### 5.1 测试数据来源清单

| 原数据分组 | 当前文件数量 | 角色 | 全量核验 | 开发期代表性样本 |
|---|---:|---|---|---|
| bias_test/produced_data/occluded_regions | 280（40×7） | 非球形 Bias 统计基准 | **matio 280/280 READ_OK**；xwerte **40 个 10×1 MAT 数组**，ywerte **240 个 10×3 MAT 数组**，class/type 均 double；独立复核待做 | 编号 1、中间编号、40 |
| bias_test/produced_data/proportional_error | 280（40×7） | 球形比例噪声 Bias 统计基准 | **matio 280/280 READ_OK**；xwerte **40 个 10×1 MAT 数组**，ywerte **240 个 10×3 MAT 数组**，double/double；独立复核待做 | 编号 1、中间编号、40 |
| adaptive_matched_filter_test/produced_data/initialization | 4 | 初始化已保存结果 | **matio 4/4 READ_OK**；全部 double[35×1] | 4 个文件 |
| adaptive_matched_filter_test/produced_data/without_errors | 5 | 无噪声对比 | **matio 5/5 READ_OK**；全部 double[600×1] | 5 个文件 |
| adaptive_matched_filter_test/produced_data/with_errors | 7 | 带噪声对比及历史额外字段 | **matio 7/7 READ_OK**；全部 double[600×1]，其中 `alleadaprotfehler` 与 `alleadaptransfehler` 为历史专属 **`HistoricalOnly`**（只读；新 Runner 不保存） | 7 个文件 |
| adaptive_matched_filter_test/produced_data/many_parameter | 6 | 多更新率与历史额外字段 | **matio 6/6 READ_OK**；四个 double[600×4]、两个 double[600×1] | 6 个文件 |
| **合计** | **582** | 原作者静态参考数据 | **matio 已读 582/582（582 变量、0 失败）；独立复核未完成** | 实际全量结果见 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) |

**数据保护**：origin/** 作为只读输入；测试只产生新目录或测试专属副本；不得向原目录写入、覆盖、删除或以 git add -f 强制提交忽略内容。样本选择和目录哈希应随源仓库提交 SHA 固定。

### 5.2 L2/L3/L4 参考数据的形成、保管与追溯

1. **现有 Golden**：作者归档中真正存在的 MAT 结果，可作为变量合同与统计结果参考，但没有每轮中间量时不能冒称提供了逐步算法真值。
2. **待生成的数值 Golden**：Franz 模板项、Original/Modified KvD 的 near/t/ω、Adaptive 的 M1…M4/b/z/error_mse、新系数、球谐块旋转。**必须以只读源公式、独立解析或独立非被测实现生成/复核 L3 参考**；不需要执行 MATLAB/Octave 或采集 L1；不能以被测 C++ 程序自己的结果作为真值。
3. **身份字段**：源 .m Git blob SHA、源树 SHA、工具及版本、原公式的固定输入和独立注入 RNG 序列、变量名/shape/dtype、参考数值文件 SHA-256、许可证/可存储位置。
4. **存放位置**：小型、可再分发的定值 Golden 在满足现有上游告知后可放 tests/golden/fixtures；受限或大型资料放在维护者批准的独立持久位置，仓库只保存路径映射和哈希，不擅自复制/再许可。
5. **版本更新**：变更输入、脚本 SHA、近度定义、索引或容差时，应生成新的 Golden 版本并保存历史，不静默覆盖旧结果。
6. **样本统计**：每次用例记录文件总数、成功解码、格式不支持、损坏、缺失、统计有效/失败/未定义，不能只报告成功部分。

### 5.3 数据合同变更与试验数据基准

MAT 全量字典和来源证据**已经生成**：见 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) 和 [design/mat_io.md](design/mat_io.md)。在 Ubuntu 24.04、matio 1.5.26 环境下，独立探针对原始归档 582/582 MAT、582/582 变量执行读取，均为 `READ_OK`，失败 0，这是**已取得的特定数据读取事实**。但它不代表生产 Reader/Writer、MAT 字节与 MATLAB 逐值 Golden、异常负例、数据来源独立核查或 P10.4 已通过；不得由此扩大推断到其他 MAT 格式和未来输入。

### 5.4 L2/L3/L4 独立证据及 TC 剩余缺项（无需 MATLAB）

以 [长期无需 MATLAB 运行政策](golden/NO_MATLAB_EXECUTION_POLICY.md) 为准。L1 原 MATLAB 运行证据**永久退出必需证据与阻塞条件**；L2 为原作者 582 MAT 最终输出，L3 为独立解析/独立重实现逐节点/逐轮参考，L4 为统计对照。AN-G01～12 是未实际测试的 L3 候选，**不等于现有全部参考就绪**。

| 重点 TC | 采用的可行参考 | 剩余缺项（不包括 L1） |
|---|---|---|
| TC-F-FILTER-001、TC-F-KVD-001/002 | L3 独立逐轴/逐轮 near、旧/新 t、ω、a/b/c、RMS、阈值、退出次数；C++ trace | 多组 L3 生成与核验、容差批准、真实 C++/CTest |
| TC-F-ADAPT-001/002/003、TC-DC-ADAPT-001 | L3 独立 M1～M4/b/z/MSE/系数、连续帧状态；L2 最终误差 | 一般场景、退化输入、Session 状态和 C++/CTest |
| TC-F-GEOM-002/003、TC-F-FLOW-002/003、TC-DC-RNG-001、TC-NF-RNG-001 | L3 i=0/1/2 **全部有序方向**、固定 uniform/normal 和调用数；L4 统计 | i=1/2 L3 参考、独立 RNG/噪声测试、CTest |
| TC-F-SH-001/002、TC-F-TRAJ-001 | L3 R0/R3/R5 全元素、系数与几何旋转、轨迹解析值 | 多轴/复合/非平凡角独立参考、版权/适配、CTest |
| TC-F-EXP-001/002/003 | L3 固定两帧/8 次聚合/刷新规则；L2 历史 MAT、L4 统计 | 一般输入参考、Runner、CTest、L4 |
| TC-F-AN-001、TC-I-ANALYSIS-001 | 从 L2 只读 MAT 独立重算为 L3 begin/end/zone/平滑/曲线 | 原始值逐元素导出、独立分析结果、CTest |

无论哪项仍缺 C++ 实施、实际测试、参考值或 DEC-005 批准时均不可标记 PASS。宽松 `atol=0.01,rtol=0.1` 已确认用于**首轮探索筛查**（正式验收另定）；对形状、顺序、RNG 消耗、状态和几何不变量仍严格检查。**不再因无法运行 MATLAB 而阻塞任何 TC/AC。**

## 6. Debian/Ubuntu 环境与配置矩阵

| 环境组 | 发行版 / CPU 架构 | 编译器 | Eigen / matio / zlib | 当前状态 |
|---|---|---|---|---|
| LINUX-01 | **H-04 用户提供 Ubuntu 24.04.4 VirtualBox / x86_64 快照**（内核标识 7.0.0-38-generic） | **历史独立 matio 探针记录 GCC 13**；用户本次 JSON 没提供编译器版本新实测 | matio 探针历史记录 **1.5.26**；Sophus、Eigen、完整 CMake/CTest 未在该审批快照中测得 | **环境已选**；用户 `free` 总内存 8127380 KiB、available 4908696 KiB、Swap 4194300 KiB，根分区当时可用 65347856 KiB；**非项目通过预算**，业务构建/CTest 未执行 |
| LINUX-02 | Debian stable，具体版本待确认 / x86_64 | GCC 或 Clang，具体版本待记录 | 依赖版本待记录 | **未构建** |
| LINUX-03（可选） | Debian/Ubuntu 的另一受支持编译器或 CPU 架构 | 由维护者决定 | 兼容性待核 | 未安排 |

- 不将 Windows 或交叉编译视为本项目要求；不凭空规定编译器/Eigen/matio 的最低版本。
- 每组将来仍须记录 uname、/etc/os-release、编译器版本、CMake 版本、Eigen/Sophus/matio/zlib 的实际版本、CMake 选项、build SHA、CTest 名称与结果；**H-04 用户提供的某时刻容量并不是可自动分配的 CPU/RAM 预算或通过阈值**，未执行不能在平台矩阵打勾。
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
| 原 MATLAB L1 取证不可行 | 无需取证 MATLAB 运行；按只读静态源生成 L3 独立逐节点参考并用 L2/L4 做输出/统计核验，披露原 MATLAB 未可证的运行行为；不得由被测 C++ 自验替代 |
| 运行资源消耗或测试超时 | 保存命令及失败证据，先进行小规模单元回归；完整论文实验要单独安排，不因小规模通过而虚称全量 |
| 未经批准的绝对/相对误差门限 | 保留候选及测量结果，项目所有者确认后冻结；历史失败不删 |

## 8. 测试阶段进入/退出准则

| 阶段 | 进入准则 | 退出准则 |
|---|---|---|
| 测试策划基线 | SRS 编号与来源、风险和 P 任务映射已核对 | TC/AC、数据管理、环境矩阵、未决 DEC 和审查人明确；**不要求软件已运行** |
| 单元测试 | P0.8 所需接口/数值合同评审、相应代码已提交推送 | 对应 TC 实际执行并保存正常/异常证据；未通过保持开放 |
| MAT/集成测试 | P1 有最小 MAT 读写能力、原始输入受保护 | 相关源 MAT 全量读取/实际字段校验，输出安全/回读测试真实通过 |
| Golden 数值测试 | 固定输入的**独立 L3 参考**与批准容差可用（不要求 MATLAB/Octave） | 逐轮变量和公式通过判据，失败/退化样本单列 |
| 论文复现 | 各入口和输出合同真实可执行、统计流程可用 | 每类实验的有效/失败数量、误差、曲线数据、差异说明和完整证据 |
| 最终交付 | M 级要求及批准的 S 级范围有可追踪状态 | 项目所有者批准例外与未决项，PR/文档/测试/提交链路完整 |

**状态区分**：独立 matio 探针在 Ubuntu 上确已编译运行且 582/582 读取成功；但**没有**由本项目注册/运行 CTest，也未完成正式 Reader/Writer、负例、逐元素 Golden 或论文复现。其余阶段的进入/退出门槛不得因这一项成功而视为满足。

### 8.1 文档追踪静态一致性检查（不等同于执行测试）

**A. 规范要求：待人类授权的 Bash 现场执行（本项未执行）。** 需要完整的 Debian/Ubuntu 仓库检出环境、获准运行和可保存的真实日志。在 REQ、P、TC、AC 或 MATLAB 源文件映射变化后，应使用下列命令执行现场审查：

~~~bash
bash scripts/check_docs_traceability.sh
~~~

该脚本只依赖 Debian/Ubuntu 的 Bash、awk、find、sed、sort 和 git；核验下述文档级集合，不编译 C++、不注册或运行 CTest，也不判定算法数值等价。**上面的命令是待执行的操作规程，绝非本次已运行命令。** 现场运行须按 [AGENTS.md §10](../AGENTS.md) 与 [人类授权申请 H-11](approval/HUMAN_APPROVAL_REQUEST.html) 获得适用授权，并保留 stdout、stderr、实际退出码、HEAD SHA、系统/工具版本及执行人。

1. 解析 REQUIREMENTS §4 的全部 REQ ID，每个 ID 唯一且有对应的 `AC-`；解析 §9 的正向追踪，不允许不存在的需求 ID 或空 P/TC/AC。
2. 解析 TEST_PLAN §3 的 TC ID，检查 TC 集合严格等于 `{ "TC-" + REQ[4:] }`；对每个 TC 校验其引用的 REQ 和 AC 也完全匹配。人工核查类型不必注册 CTest，但必须有真实证据路径。
3. 解析 PLAN §5 的全部 P ID 与依赖，检查无重复、无未知前置、无有向环；每个 P 的 REQ、TC、AC 集合必须成对一一对应，任何孤立 REQ/P 都需要解释。
4. 解析 `origin/egomotion_estimation_dense_flow_fields_matlab_scripts/**/*.m` 与 SRS §9.2 的路径集合，确保两边完全一致；`OBJECT_MODEL.md` 的对象职责映射也应覆盖全部源文件，未移植标注不等于完成。
5. 数据文件数量、版本和哈希应来自真实 Git 树，不从对话旧表复制；只有实际完整解码后才检查 AC-D-MAT-001 的值域/类型/维度合同。
6. 将每轮静态审查报告记录为：源 Git SHA、REQ/P/TC/AC 和 .m 文件数量、唯一性、漏项、环数量、被审阅文件清单。只读检查成功不应标为 CTest、Golden 或需求已通过。

**B. 已执行与未执行证据（不得合并统计）。**

| 证据类别 | 当前状态 | 可核查的来源与未覆盖范围 |
|---|---|---|
| GitHub 连接器＋JavaScript 只读静态断言 | **已执行（此前的连接器静态检查）** | [2026-11-06 追踪审查记录](reviews/STATIC_TRACEABILITY_2026-11-06.md)：按固定 Git SHA 核对 66 REQ、66 TC、98 P、45 `.m`、26 条 HLD 边，未发现缺项/环；**没有执行 Bash** |
| 审查意见后的 GitHub 连接器只读复核 | **已执行（非 Bash、非 CTest）** | [跨文档一致性复核记录](reviews/DOC_CONSISTENCY_RECONCILIATION_2026-11-06.md)：固定提交 `5472c756b983fe002ef6e32143c96b6c6798cde9`，核对 66/66/98、45 `.m` 双向映射、26 条 HLD 静态边与源 `origin` 子树 SHA 未变；**未执行现场 Bash** |
| `bash scripts/check_docs_traceability.sh` 完整检出现场运行 | **待授权／未执行** | [H-11](approval/HUMAN_APPROVAL_REQUEST.html)；目前无现场 stdout、stderr、真实退出码和执行环境记录，不得写为 PASS |
| 正式 CMake 业务 targets 的构建、链接与 CTest | **未建立／未执行** | 静态 DAG 无环不能证明真实构建成功；审批与执行记录见 H-10 / [PROGRESS.md](PROGRESS.md) |
| 独立 matio 原始归档读取探针 | **此前已实测** | [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) 已记录 582/582 READ_OK；**不代表**本次重新运行探针、生产 Reader/Writer、异常负例或数值 Golden 通过 |

**实际现场执行的补录要求**：新建独立证据记录，附所执行命令、真实 HEAD SHA、工具链、stdin（若有）、stdout/stderr 路径、exit code、时间戳和执行人；经人类审阅后再同步 [PROGRESS.md](PROGRESS.md)，不能以本表的“应执行”替代“已执行”。

**当前文档级检查口径**：66 条 REQ、66 项 TC、98 个 P 工作项、45 个 .m 映射是版本化的预期数量；检查器同时验证 ID 唯一性、配对关系、真实路径集合、P 依赖环和 ARCHITECTURE §4.1 target DAG 的**静态环**，不只比对计数。变更基线数量时须同步评审脚本期望值；**静态无环不等于真实 CMake target 已创建或编译通过**。

**本轮实际执行状态**：用户审批 JSON 的 H-11=deferred，因此未运行上文的 Bash 现场脚本；H-10=deferred，因此未运行新的 CMake 配置/编译、CTest、MATLAB、MAT 写回读、负例或性能测量。原来的 582/582 matio 探针属于**早前独立实测**，不在本轮重跑。审批详情与阶段状态参阅 [审批落实记录](approval/DECISION_IMPLEMENTATION_NOTES.md) 和 [PROGRESS.md](PROGRESS.md)。

## 9. 变更与冻结条件

任何更改 MATLAB 对应公式、输出变量、MAT 读取边界、随机数处理、数值容差或实验输入都必须更新 REQ/TC/AC 对照并重新生成对照证据；不能通过跳过失败样本、修改阈值或者仅重新绘图来宣布结果通过。真实构建与 CTest 开始之前须遵守 AGENTS.md 的远程检查点规则。

**当前状态：66 个 TC 均未创建真实测试函数或 CTest，不能计为通过。**

**v0.8 文档修订（AF-063）**：五个 TC 行的尚未创建来源清单与现有可用文档分开列示，不修改 TC/AC/REQ、Golden 数值、授权或任何 PASS 状态。


**v0.11 历史文件身份索引补充（P0.6）**：已新增 45 个 `.m` 的 [SOURCE_INVENTORY](SOURCE_INVENTORY.md) 与 [MIGRATION_MATRIX](MIGRATION_MATRIX.md)；改变的只是五个 TC 行可引用的**已存在静态来源**，TC/REQ/AC 名称、66 个 TC 测试函数未执行事实及 H-10/H-11 仍不变。
