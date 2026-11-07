# 分模块详细设计（LLD）索引、完成度与冻结标准

> **长期证据规则**：[不执行 MATLAB 的项目政策](../golden/NO_MATLAB_EXECUTION_POLICY.md) 已取消原 MATLAB L1 Golden 的强制门禁。LLD 与验收只要求来源清楚的 L2/L3/L4、真实 C++/CTest 和批准容差；**缺少 MATLAB 运行不阻塞冻结**，但不能宣称原脚本的未知异常/RNG/逐位输出已验证。

> **LLD 索引 v0.3（所有模块草案、未冻结；未建立生产 C++/CTest 证据）。** 索引中的文件已实际创建，但其中的候选 C++17 接口、数学步骤、测试路径和错误枚举没有对应已实现代码。源 MATLAB 文件和已保存 MAT 仍由 origin/** 只读保存。

## 文档职责

- [REQUIREMENTS.md](../REQUIREMENTS.md)：SRS 中 REQ/AC/TC 与可验证行为的唯一来源。
- [ARCHITECTURE.md](../ARCHITECTURE.md)：HLD 组件边界、target DAG、部署和 ADR 的唯一来源。
- [OBJECT_MODEL.md](../OBJECT_MODEL.md)：类职责、所有权及 45 MATLAB 文件→拟议方法的唯一维护位置。
- [TEST_PLAN.md](../TEST_PLAN.md)：TC→候选测试实体/CTest→AC、L2/L3/L4 参考准则与测试证据**字段模板**的唯一来源；**已执行的结果**只登记于 PROGRESS/reviews。
- [PLAN.md](../PLAN.md)：98 个 P0–P10 最小工作项、任务依赖、风险与里程碑。
- [PROGRESS.md](../PROGRESS.md)：阶段及里程碑、DEC 处理、实际执行结果、人员/批准的**唯一动态登记**。历史证据可在 [reviews/](../reviews/) 中查验；固定 Git SHA 后的变更不能被前一份报告覆盖。
- [TODO.md](../TODO.md)：**仅需人类参与且可推迟**的材料/授权/最终基线事项，逐项记原因与建议处理时间；普通 C++ 私有实现技术细节不作人类待审项。
- [DOCUMENT_BASELINE_REGISTER.md](../DOCUMENT_BASELINE_REGISTER.md)：**全部当前规范和过程文档的类型/版本/状态、适用范围、来源 SHA 与未冻结门禁的统一索引**；不在每个 LLD 重复维护当前状态表。
- [contracts.md](contracts.md)：C++17 领域值类型、Result<T,E>、错误分类、资源生命周期与跨模块接口的**统一候选合同**。
- 本目录每份文档是**对应模块的 LLD 草案**，具体签名、公式、错误处理、数据布局应在 P0.8 以后逐个确认。

## LLD 模块覆盖矩阵

| 模块 | 对应文档 | 原只读 MATLAB 源范围 | 数学/数据约定 | 候选 API | L2/L3/L4 参考/实测 | 冻结状态 |
|---|---|---|---|---|---|---|
| 跨模块 C++17 合同 | [contracts.md](contracts.md) | SRS/OBJECT_MODEL/全部模块 | 值类型、ErrorDetail 稳定字段与唯一 Mat/Num/Experiment 错误码；MatDenseArray 归 io | **轻量封装 Result/Status（内部 variant）已选定，仍未实现**，DataMismatch 限定范围 | 无真实测试 | 未冻结，P0.8/DEC-008 |
| CLI 参数与错误协议 | [cli.md](cli.md) | 五个实验入口、inspect、analyze、可选 plot | 按入口列出参数/合法范围、退出码与诊断字段 | 候选命令/错误 schema | 未测 | 阻塞 P0.8/P1.2、DEC-008 |
| MAT I/O | [mat_io.md](mat_io.md) | getflowfield、各入口 load/save、582 MAT | **582/582 真实 v5+zlib/double 类型字典已获得**；生产 Reader/Writer、路径安全仍待验收 | 候选签名/错误码 | **独立探针已执行，非 CTest** | DEC-001/002 格式事实已证实，接口未冻结 |
| 基础采样与噪声 | [sampling_noise.md](sampling_noise.md) | findeDreiecke、采样/流/噪声函数 | 数量、顺序、噪声方程、RNG 消耗与自拥有 FlowField/结果 | **egomotion_sampling/simulation 的接口、头文件边界与 TC/CTest 候选** | L3 独立有序方向/固定 RNG 仍待验证 | 未冻结，P0.8/DEC-008 |
| 几何与轨迹 | [trajectory.md](trajectory.md) | Kreis/Sinus/trajectory、球内近度与旋转 | 已列离散轨迹/求交公式；**Sophus SO(3) 替代 rodrigues.m 已选定**，源脚本只读 | 候选接口 | L3 小角/反向等边界与真实 CTest 未执行 | **设计路线已确定**；实际 Sophus 依赖版本、调用和数值测试尚未验证，不再等待普通版权选型 |
| Franz / KvD | [kvd.md](kvd.md) | zellendefinieren、franzeigen、两种 KvD、误差 | 已列各平移公式、RMS、终止规则 | 候选接口 | 未测 | 阻塞 DEC-005 |
| 实球谐 | [harmonics.md](harmonics.md) + [Ivanic–Ruedenberg 原文/勘误与纯 Eigen 方案](SH_ROTATION_IVANIC_RUEDENBERG.md) | kugelflfunc2、near/koeff、SHRotate | 九项索引/公式，原文 1996＋勘误 1998 为必读，Eigen-only 且禁止 sdla | **egomotion_harmonics 显式依赖 egomotion_contracts**；[专题 LLD §3](SH_ROTATION_IVANIC_RUEDENBERG.md) 已给出预定公共/私有文件、命名空间、类/成员/静态方法**声明**，无任何实现/测试 | 待用户上传论文、完成 L3 全元素与 CTest | 数学方向已定；实现/DEC-005 未冻结 |
| Adaptive 单步/Session | [adaptive_mfa.md](adaptive_mfa.md) | oneit、3 个 Adaptive 入口 | 已列 M1–M4、RHS、MSE 及会话差异 | 候选接口 | 未测 | 阻塞 DEC-005 |
| 实验 Runner | [experiments.md](experiments.md) | 2 Bias + 3 Adaptive 生成入口 | 参数/save、8 次聚合、刷新策略及 L2/L3/L4 证据等级 | **RunContext 候选接口、DataMismatch 边界、egomotion_experiments/TC/CTest** | 新 Runner/完整 L3/CTest 均未实现 | 未冻结，P0.8/DEC-008 |
| 分析与可选绘图 | [analysis_plotting.md](analysis_plotting.md) | 4 分析入口、kurveglaetten、3 插图 | 已列数据/索引/阈值/窗口与 DEC-004 缺字段规则 | 候选接口 | **必需的 L2 原 MAT/L3 独立数值曲线和 CTest 尚未验收** | 可选绘图后端尚未选定，但**不阻塞核心数值分析、设计和验证** |

覆盖范围是**10 份文档：1 份跨模块合同（contracts.md）和 9 份模块 LLD（含 cli.md）**的存在性，不是代码/接口实现比例。无论数值公式写得多完整，缺少 Golden 和批准容差就不能把相应模块标为已完成。

**术语约定**：此目录仍可能保留历史测试编号中的 `Golden` 字样；如无特殊说明，现行数值参考明确指向 **L2 作者已存 MAT、L3 独立解析/非被测实现、L4 统计**，而不是已经取消强制获取的 **L1 MATLAB 运行输出**。任何候选值均须给出来源等级、独立生成器和数据 SHA，不可凭名称推断已通过。

## 模块 LLD 的标准模板与通过条件

每份 LLD 在进入正式编码前应具有：

1. **来源**：每个实际 MATLAB 函数的完整路径、原始 blob SHA、函数调用关系和可能的第三方来源。
2. **精确合同**：候选 C++17 类/方法最终签名、所有权/异常/复制移动/线程边界、输入输出单位与 shape。
3. **算法**：逐步公式或伪代码、固定顺序、数值阈值、源 MATLAB 与 C++ 索引差异；对原版明显异常单独记录。
4. **资源**：谁创建、谁销毁、谁可写；原始 MAT 不可写、输出新目录不可覆盖、RNG/Session 不跨运行共享。
5. **测试**：REQ→AC→TC→实际测试文件/函数/CTest、Golden 输入输出 SHA、批准容差、正常/异常路径和真实测试日志。
6. **构建**：target、公开/私有头文件、依赖库/编译器版本，避免逆向依赖。
7. **批准**：项目所有者/维护者批准、Git SHA、日期、开放风险及 DEC 处理决定。

### 进度状态字典

- **草案已创建**：已有文字设计，但不能据此创建“完成”百分比。
- **待核验**：来源、MAT 字典、算法细节、接口或 Golden 缺失。
- **可评审**：必填字段已准备且所有待决事项明确；尚不表示通过。
- **已冻结**：审阅人批准具体 SHA、合同、Golden、容差和开放例外。
- **已实现/已测试**：分别需要代码提交及实际执行证据，是与设计冻结独立的两个状态。

## 设计冻结所需证据的分类（实时状态见 PROGRESS）

本索引只列**必需证据类型**：接口/错误合同与所有权的可执行定义、CMake target 的真实依赖、L2 只读逐值与 SHA-256、L3 独立固定输入数值、L4 统计（适用时）、真实 CTest/异常负例、最终 DEC-005 模块阈值及适用的验收签署。**实际是否已取得及执行时间、分母、失败数统一参见 [PROGRESS.md](../PROGRESS.md) 和 [TEST_PLAN.md](../TEST_PLAN.md)**，此处不复制运行计数或假称已通过。用户已确定 MIT、纯 Eigen/禁 sdla、DEC-004、固定输出根、L1 不必需等设计方向，**不再把这些旧技术选择反复列为待审批阻塞**。


## LLD 证据就绪度与源 MATLAB 核查重点

| 文档 | 已文档化的可执行设计检查点 | 尚缺的冻结证据 |
|---|---|---|
| contracts.md | FlowField N×6、3×5 近度系数、统一 Failure/Result/Status、ErrorDetail 允许阶段、MatDenseArray 的 io 归属与 DataMismatch 边界 | 实际类型/ABI、错误码和诊断协议批准、线程/异常合同测试 |
| mat_io.md | MAT v5+zlib/double 类型与形状已实测；**v0.6 候选**已细化 MatDenseArray 的二维 double 自拥有值、严格/有界 compat 多变量规则、RunContext/Writer、egomotion_matio target 和负例/TC 测试文件映射；与 contracts 窄接口分离 | **仍缺**正式接口冻结、L2 逐值/SHA-256、L3 独立负例、Reader/Writer、写入/回读、安全 CTest 与 P10.4 复核 |
| cli.md | 命令、参数/类型、固定输出根、独立 OS RunId、ErrorStage/JSONL 已选定 | 实际 CLI/CTest 仍不存在，参数负例和 JSONL 转义需测试 |
| kvd.md | 原始/修正平移与旋转公式、最低/最大迭代、逐轮 trace 字段 | 独立 L3 逐轮 Golden、病态/停止边界、正式容差与 CTest |
| harmonics.md | 九实基公式、3×5、R3/R5 全元素、HarmonicRotationBlocks 归属、私有递推 target 与测试映射 | 论文原文/勘误待上传、Eigen-only 递推未实现、独立 L3 逐元素参考与最终容差/CTest |
| adaptive_mfa.md | M1–M4、rhs、MSE 返回、Frame/Session 更新时序差异 | 逐节点 L3 参考、已确定的不提交失败状态策略仍需真实 CTest 验证、最终容差 |
| sampling_noise.md | 三种采样与噪声公式、固定 uniform/normal 消耗及值对象所有权、sampling/simulation target 和测试文件候选 | i=0/1/2 全量独立 L3 参考、RNG/噪声 TC/CTest、真实实现 |
| trajectory.md | 圆/正弦公式、方向→旋转奇异性、Rᵀ 局部运动、球内求交；H-02 指定 Sophus 替代 Rodrigues | Sophus 实际依赖/版本和调用、小角 Golden、零模/反向边界 |
| experiments.md | 五入口参数/输出、历史 MAT shape 与原 xwerte、RunContext 所有权、L2/L3/L4 分类、DataMismatch/target/CTest | **DEC-004 已选定历史字段 HistoricalOnly/当前仅存现行 save**；仍缺 RunContext/Writer 具体实现、错误提交测试、新结果/CTest |
| analysis_plotting.md | 四分析入口与历史 MAT 的真实 shape、1-based 索引、分段阈值、半窗平滑 | **仍缺**独立 L3 数值曲线、C++ 分析/CTest；可选绘图后端不是核心算法阻塞 |

### 评审与完成状态的强制区分

- “接口/算法已文档化”只表示具备静态评审材料；不等于签名可编译、Golden 已存在或数值结果正确。
- “MATLAB 原代码具有某种边界行为”应标明它来自**源文件的静态阅读**还是实际 MATLAB 数值执行，不能混淆。
- MATLAB 原始函数、原始 MAT 文件和版权 notice 一律不在文档维护任务中更改；尤其不能通过改动已有 MAT 获得“通过”。
- 模块签名只有在 SRS/HLD、[contracts.md](contracts.md)、源 MATLAB、CLI/error schema、真实 MAT 数据合同之间一致且项目所有者批准后才能冻结。批准之前不应称这些草案为“编码基线”。

## 本期未完成的先决工作

1. **已完成的独立子能力**：582/582 MAT 已通过 matio 解压/变量元数据检查；**尚待** P1 生产 Reader/Writer 按值加载、错误负例及 P10.4 独立结果审核。
2. **球谐设计约束已确定**：物理旋转使用 Sophus，实球谐递推须依据 1996 原论文和 1998 勘误**纯 Eigen 重写**，Don Williamson 仅代码参考且**禁用 sdla**；用户尚待上传论文 PDF。参见 [专题 LLD](SH_ROTATION_IVANIC_RUEDENBERG.md)；普通版权核查不再是研发阻塞。
3. **接口设计已选择，尚待实现核验**：Result/Status 使用轻量封装+内部 variant，ErrorStage/JSONL、固定输出根/RunId 已选；P0.8/DEC-008 的头文件自包含、异常传播、实际 CTest 身份与兼容边界仍待记录。
4. **数值/性能必须留实证**：L3 独立参考、L2 全量值、必要 L4 统计及真实误差记录仍是验收证据；首轮 0.01/0.1 只是筛查，最终 DEC-005/006 的通过阈值需实测。MATLAB/Octave/L1 不要求。
5. **DEC-004 已选定**：新 Runner 仅写当前有效 save，历史额外字段标 HistoricalOnly 并只读保留；新分析缺必需字段报错，不等待归因。相应真实 Runner/Analyzer/CTest 和证据仍未完成，见 [DEC-004](../decisions/DEC-004_HISTORICAL_MAT_FIELDS.md)。

**当前模块 LLD 均未冻结**：独立 C++17 matio 扫描器确已编译运行，但无生产业务模块、CTest、逐元素 Golden 或算法数值实验。不能将数据字典扫描通过扩大为整项目或 LLD 验收通过。

## Golden 来源等级的跨模块统一索引（不运行 MATLAB）

以 [长期无需 MATLAB 执行政策](../golden/NO_MATLAB_EXECUTION_POLICY.md) 为当前规则，[L2/L3/L4 参考来源及 AC 对应](../golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md) 为模块核对清单。**L1 原 MATLAB 运行证据永久退出必需清单**，不得再因为“未运行 .m”或“缺少 MATLAB 逐节点 Golden”而阻塞签名评审、设计冻结、代码实现或验收。作者 582 MAT 为 L2 最终值来源（只读逐值/SHA-256 复核待做）；AN-G01～12 等解析候选属于 L3，**尚需独立复核与扩展**；L4 统计尚待实际分析。模块真实接口、异常、依赖、许可、DEC-005/008、CMake/CTest 与正式签署要求不变。

- KvD/Franz、Adaptive：用 L3 **独立逐节点/逐轮数学参考**（包含一般/边界输入），以及实际 C++ trace 验证。
- 采样、RNG：用 i=0/1/2 的全部有序 L3 方向、固定注入 uniform/normal 与准确调用次数；不要求复现原 MATLAB 特定种子的具体随机流。
- 球谐/轨迹：用独立 R0/R3/R5 全元素、系数、几何和逆变换参考。
- 实验、分析与 MAT I/O：L2 历史 MAT 原值，独立重算 L3 曲线/聚合/刷新以及 L4 统计。原 MATLAB 不执行。

[TEST_PLAN.md §3](../TEST_PLAN.md) 仍是完整 TC→测试实体→AC 的唯一候选矩阵；本索引不等于任何 TC/AC 已通过。
