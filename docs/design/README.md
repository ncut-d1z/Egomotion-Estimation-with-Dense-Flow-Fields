# 分模块详细设计（LLD）索引、完成度与冻结标准

> **长期证据规则**：[不执行 MATLAB 的项目政策](../golden/NO_MATLAB_EXECUTION_POLICY.md) 已取消原 MATLAB L1 Golden 的强制门禁。LLD 与验收只要求来源清楚的 L2/L3/L4、真实 C++/CTest 和批准容差；**缺少 MATLAB 运行不阻塞冻结**，但不能宣称原脚本的未知异常/RNG/逐位输出已验证。

> **LLD 索引 v0.15（所有规范草案未冻结）。** 仅低阶球谐叶子已有 C++17/Eigen 实现、根 CMake target 和用户提供的固定提交 `8eab5dd`、含解析/相位回归的 `445ed151`、含624四元数/错态的 `601c74de` 三次构建/CTest 均1/1 Passed的用户证据。其它跨模块候选接口/软件目标未实现，`origin/**` 继续只读。

> **2026-11-09 材料更新**：用户已提供IR96原文、IR98勘误及CIGR99全文，已登记附件身份并完成限定范围静态审阅；**不再等待上传**。发现勘误表项冲突、源矩阵转置和实基相位问题，见[论文审计](../reviews/SH_PAPERS_SOURCE_AUDIT_2026-11-09.md)与[独立验证候选](../golden/SH_PAPER_VERIFICATION_CASES.md)。随后用户已**单独开放低阶球谐 C++ 实现**，并在三个固定提交实际运行限定范围构建/CTest；不等于其它模块/H-11 全工程授权或AF-045/046全部验收完成。

## 文档职责

- [REQUIREMENTS.md](../REQUIREMENTS.md)：SRS 中 REQ/AC/TC 与可验证行为的唯一来源。
- [ARCHITECTURE.md](../ARCHITECTURE.md)：HLD 组件边界、target DAG、部署和 ADR 的唯一来源。
- [OBJECT_MODEL.md](../OBJECT_MODEL.md)：类职责、所有权及 45 MATLAB 文件→拟议方法的唯一维护位置。
- [TEST_PLAN.md](../TEST_PLAN.md)：TC→候选测试实体/CTest→AC、L2/L3/L4 参考准则与测试证据**字段模板**的唯一来源；**已执行的结果**只登记于 PROGRESS/reviews。
- [PLAN.md](../PLAN.md)：98 个 P0–P10 最小工作项、任务依赖、风险与里程碑。
- [PROGRESS.md](../PROGRESS.md)：阶段及里程碑、DEC 处理、实际执行结果、人员/批准的**唯一动态登记**。历史证据可在 [reviews/](../reviews/) 中查验；固定 Git SHA 后的变更不能被前一份报告覆盖。
- [TODO.md](../TODO.md)：**仅需人类参与且可推迟**的材料/授权/最终基线事项，逐项记原因与建议处理时间；普通 C++ 私有实现技术细节不作人类待审项。
- [DOCUMENT_BASELINE_REGISTER.md](../DOCUMENT_BASELINE_REGISTER.md)：**选定规范及证据文件的固定版本、状态和 Git blob SHA 登记**（原快照共 36 项，含附件/历史证据，非全部文件）；本轮**全工程文档规范审计**的审计对象、三态和整改进度见 [AUDIT_PROGRESS.md](../AUDIT_PROGRESS.md)，审计方法和评价基准见 [AUDIT.md](../AUDIT.md)；原低阶球谐六项目标单列于 [AUDIT_PROCESS.md](../AUDIT_PROCESS.md)。新增审计对象**不自动取得正式基线资格**；不在每个 LLD 重复维护 SHA 表。
- [contracts.md](contracts.md)：C++17 领域值类型、Result<T,E>、错误分类、资源生命周期与跨模块接口的**统一候选合同**。
- 本目录含**模块LLD、跨模块LLD、专题LLD、理论专题及本索引**，它们不是同一文档类型；只有模块/跨模块LLD的候选签名、错误、生命周期和数据布局需在P0.8以后逐个冻结，数学理论专题与索引不可代替正式LLD。

## LLD 模块覆盖矩阵

| 模块 | 对应文档 | 原只读 MATLAB 源范围 | 数学/数据约定 | 候选 API | L2/L3/L4 参考/实测 | 冻结状态 |
|---|---|---|---|---|---|---|
| 跨模块 C++17 合同 | [contracts.md](contracts.md) | SRS/OBJECT_MODEL/全部模块 | 值类型、ErrorDetail 稳定字段与 Mat/Numerical/Experiment 错误枚举；**AnalysisErrorCode/PlotErrorCode 均在 analysis_plotting 定义，stage/exit 仅由 contracts §3.1 映射**；MatDenseArray 归 io | **轻量封装 Result/Status（内部 variant）已选定，仍未实现**，DataMismatch 限定范围 | 无真实测试 | 未冻结，P0.8/DEC-008 |
| CLI 参数与错误协议 | [cli.md](cli.md) | 五个实验入口、inspect、analyze、可选 plot | 按入口列出参数/合法范围、退出码与诊断字段 | 候选命令/错误 schema | 未测 | 阻塞 P0.8/P1.2、DEC-008 |
| MAT I/O | [mat_io.md](mat_io.md) | getflowfield、各入口 load/save、582 MAT | **582/582 真实 v5+zlib/double 类型字典已获得**；生产 Reader/Writer、路径安全仍待验收 | 候选签名/错误码 | **独立探针已执行，非 CTest** | DEC-001/002 格式事实已证实，接口未冻结 |
| 基础采样与噪声 | [sampling_noise.md](sampling_noise.md) | findeDreiecke、采样/流/噪声函数 | 数量、顺序、噪声方程、RNG 消耗与自拥有 FlowField/结果；**源 fixed_* 返回 near 是末次标量，C++ nearness 向量仅为新增诊断数据** | **egomotion_sampling/simulation 的接口、头文件边界与 TC/CTest 候选** | L3 独立有序方向/固定 RNG 仍待验证 | 未冻结，P0.8/DEC-008 |
| 几何与轨迹 | [trajectory.md](trajectory.md) | Kreis/Sinus/trajectory、球内近度与旋转 | 已列离散轨迹/求交公式；**Sophus SO(3) 替代 rodrigues.m 已选定**，源脚本只读 | 候选接口 | L3 小角/反向等边界与真实 CTest 未执行 | **设计路线已确定**；实际 Sophus 依赖版本、调用和数值测试尚未验证，不再等待普通版权选型 |
| Franz / KvD | [kvd.md](kvd.md) | zellendefinieren、franzeigen、两种 KvD、误差 | 已列各平移公式、RMS、终止规则 | 候选接口 | 未测 | 阻塞 DEC-005 |
| 实球谐 | [harmonics.md](harmonics.md) + [Ivanic–Ruedenberg 论文与纯 Eigen 方案](SH_ROTATION_IVANIC_RUEDENBERG.md) | kugelflfunc2、near/koeff、SHRotate | 九基直接/递推、三路旋转一般SO(3)已精确验证；受限源koeff_aus_near二阶逐系数投影已在5e522198原生编译/CTest通过，且独立Wolfram七方向精确九系数差全0；IR98冲突已定位 | **现有叶子 `egomotion_harmonics` 只依赖 Eigen3，不依赖尚未实现的 `egomotion_contracts`**；未来正式公共 harmonics→contracts 架构边仍是候选，见专题§5 | GCC16.2/CMake4.3.4/Eigen3.4 下固定 HEAD `8eab5dd`、`445ed151` 和624样本版本 `601c74de` 的球谐旧版CTest均由用户报告1/1通过；最新固定5e522198 3项CTest（含二阶源拟合）3/3通过，广义L2/L3/ABI/DEC-005仍待 | 已实现受限低阶基/旋转/重建及源二阶逐项投影；用户已提供固定5e522198下包括归档和二阶投影在内的**3/3 CTest Passed**，但正式公共ABI、全业务集成和DEC-005仍未冻结 |
| Adaptive 单步/Session | [adaptive_mfa.md](adaptive_mfa.md) | oneit、3 个 Adaptive 入口 | 已列 M1–M4、RHS、MSE 及会话差异 | 候选接口 | 未测 | 阻塞 DEC-005 |
| 实验 Runner | [experiments.md](experiments.md) | 2 Bias + 3 Adaptive 生成入口 | 参数/save、8 次聚合、刷新策略及 L2/L3/L4 证据等级 | **RunContext 候选接口、DataMismatch 边界、egomotion_experiments/TC/CTest** | 新 Runner/完整 L3/CTest 均未实现 | 未冻结，P0.8/DEC-008 |
| 分析与可选绘图 | [analysis_plotting.md](analysis_plotting.md) | 4 分析入口、kurveglaetten、3 插图 | 已列数据/索引/阈值/窗口与 DEC-004 缺字段规则；**历史 Bias xwerte 不变，新增实际 N 对照图不是当前交付项** | 候选接口 | **必需的 L2 原 MAT/L3 独立数值曲线和 CTest 尚未验收** | 可选绘图后端尚未选定，但**不阻塞核心数值分析、设计和验证** |

**本目录当前实际有14份Markdown文件**（经固定分支目录列表核对）：**1份本索引**（README.md）、**1份跨模块LLD**（contracts.md）、**9份模块LLD**（cli.md、mat_io.md、sampling_noise.md、trajectory.md、kvd.md、harmonics.md、adaptive_mfa.md、experiments.md、analysis_plotting.md）、**1份球谐专题LLD**（SH_ROTATION_IVANIC_RUEDENBERG.md）以及**2份纯数学理论专题**（SH_DISCRETE_REFIT_CONSISTENCY.md、ADAPTIVE_MEAN_ZERO_CONDITIONING.md）。上方覆盖矩阵仍为**10个模块/合同行**，harmonics和球谐专题LLD共用一行；**14个目录文件≠11份LLD≠10个覆盖行**。理论专题不能冒充模块LLD的接口/错误/生命周期/测试验收；文档存在也不等于实现/批准。

**术语约定**：此目录仍可能保留历史测试编号中的 Golden 字样；现行数值参考明确为 **L2 作者已存 MAT、L3 独立解析/非被测实现、L4 统计**，不是已取消强制获取的L1。候选值须给来源等级、独立生成器/推导和数据SHA，不可凭名称推断已通过。

## 模块 LLD 的标准模板与通过条件

每份 LLD 在进入正式编码前应具有：

1. **来源**：实际MATLAB函数完整路径、blob SHA、调用关系及第三方来源。
2. **精确合同**：C++17签名、所有权/异常/复制移动/线程边界、单位和shape。
3. **算法**：逐步公式/伪代码、固定顺序、数值阈值、索引转换，原版异常独立记录。
4. **资源**：所有者/销毁/可写边界，源MAT只读、新目录非覆盖、RNG/Session不共享。
5. **测试**：REQ→AC→TC→实际测试文件/函数/CTest，独立参考身份、容差、正常/异常日志。
6. **构建**：target、头文件与真实依赖/工具链版本，不形成反向依赖。
7. **批准**：有权限者批准具体SHA、范围、日期、开放风险和DEC处理。

### 进度状态字典

- 草案已创建：已有文字，不据此计算软件完成率。
- 待核验：来源、数据、公式、接口或参考不足。
- 可评审：材料准备且待决事项明确，并非通过。
- 已冻结：审阅人批准具体SHA/合同/参考/容差/例外。
- 已实现与已测试：分别需要代码与实际执行证据，独立于设计冻结。

## 设计冻结所需证据的分类（实时状态见 PROGRESS）

需要接口/错误与所有权的可执行定义、真实CMake依赖、L2只读逐值/SHA-256、独立L3、适用L4、真实CTest/异常负例、最终DEC-005及签署。实际取得时间/分母/失败数由[PROGRESS](../PROGRESS.md)与[TEST_PLAN](../TEST_PLAN.md)登记。本索引不重复动态PASS计数。已选MIT、纯Eigen/禁sdla、DEC-004、固定根、L1非必需不重新要求决策。

## LLD 证据就绪度与源 MATLAB 核查重点

| 文档 | 已文档化检查点 | 尚缺的冻结证据 |
|---|---|---|
| contracts.md | FlowField N×6、3×5系数、统一Failure/Result/Status、ErrorDetail阶段、MatDenseArray归io、DataMismatch边界 | 实际类型/ABI、协议批准、线程/异常测试 |
| mat_io.md | v0.10候选自拥有double、严格/有界compat、RunContext/Writer、target/负例映射；历史格式shape已测 | 正式接口、L2逐值/SHA、L3负例、Reader/Writer、回读安全CTest/P10.4 |
| cli.md | 参数/类型、固定根、独立OS RunId、ErrorStage/JSONL | 真实CLI、负例/转义及CTest |
| kvd.md | 两种平移/旋转、循环条件、trace和两组六方向静态候选 | 独立逐轮L3、一般/病态/停止边界、容差与CTest |
| harmonics.md | 九实基、IR/Choi与张量一般SO(3)精确证明；叶子低阶C++/测试及GCC16首次CTest已有 | 完整全业务适配仍缺；旧增强/624压力回归已由用户实测，新增AN-G归档及源二阶逐项投影的两项CTest仍待运行；一般公共HarmonicFitter/ABI、正式DEC-005/008与批准未完成；**不再缺论文或首次原生编译** |
| adaptive_mfa.md | M1～M4/rhs/MSE、入口专属Session时序 | 逐节点L3、失败不提交真实测试、最终容差 |
| sampling_noise.md | 有序采样、RNG/噪声、所有权、target/TC | levels0/1/2全量独立参考与实际测试 |
| trajectory.md | 圆/正弦、方向奇异性、Rᵀ、正根、Sophus路线 | 锁定依赖/调用、边界参考与CTest |
| experiments.md | 五入口、save/历史字段、RunContext、Manifest、聚合 | 真实Writer/Runner、失败提交、新结果/CTest |
| analysis_plotting.md | 四分析、分段/平滑、插图专用公式与源标题差异 | 独立曲线L3、分析/图形代码/CTest；后端不阻塞数值 |

### 评审与完成状态的强制区分

接口/算法已文档化不等于可编译或数值正确。原MATLAB行为必须区分静态阅读与运行事实；源函数/MAT/notice不在文档任务中更改。模块只有SRS/HLD/contracts/源码/CLI/真实数据合同一致且获得批准才冻结，不能把草案称编码基线。

## AF-059：逐模块 LLD 冻结前核对清单（静态审阅，非批准）

| 单一合同归属与模块 | 候选入口 | 尚缺证据 |
|---|---|---|
| [contracts](contracts.md) | §2～4公共值/错误；MatDenseArray归MAT I/O；分析错误exit归§3.1 | 真实公共头、variant/ABI/线程/错误传播与DEC-008 |
| [CLI](cli.md) | §2～4命令/RunId/JSONL；映射引用contracts | RunId/负噪声/转义/优先级测试与签署 |
| [MAT I/O](mat_io.md) | §2/§7自拥有数据/RAII/Reader/Writer，不暴露matio C指针 | L2/L3、真实安全/回读/异常测试 |
| [采样/噪声](sampling_noise.md) | §3/§6注入接口、near_last与新向量分离 | 有序L3、消耗/非法样本测试 |
| [轨迹](trajectory.md) | §5～6轨迹/局部运动/Sophus | 零角/反向/近π/求交参考与CTest |
| [Franz/KvD](kvd.md) | §7～9候选接口、trace及固定输入，不依赖matio | 完整逐轮参考、失败分类、编译/DEC-005 |
| [实球谐](harmonics.md)及[专题](SH_ROTATION_IVANIC_RUEDENBERG.md) | harmonics §2～3；专题§3约定、§4数学、§5未来公共声明；现有 `low_order.hpp` 是局部叶子 API | 一般SO(3)符号与低阶叶子首次CTest已有；未完成正式公共API/ABI、以后新回归重测、完整集成和最终批准 |
| [Adaptive](adaptive_mfa.md) | §8～9StepSolver/Session和原子提交 | 6×6/刷新/异常的独立L3与真实CTest |
| [实验](experiments.md) | §6～8独立Config/RunContext/RunOutcome、Writer注入，不直连matio | 五Runner/save/Manifest/聚合与替身测试 |
| [分析/绘图](analysis_plotting.md) | §2/§6～8自拥有只读分析、可选Exporter | L2→L3曲线、分段/缺字段负例、C++/CTest |

上表只定位候选签名、所有者、target和TC索引，不自动冻结、生成代码或关闭原AF-037～051；动态证据仅见PROGRESS。

## 本期未完成的先决工作

1. 历史582/582 MAT解码/元数据已有证据；生产按值Reader/Writer、负例和P10.4仍缺。
2. **球谐材料已齐**：IR96/IR98/CIGR99已读并登记，主要剩余问题是勘误表项冲突、源码转置/实基相位和独立全元素验证；路线保持Sophus物理旋转、Eigen-only球谐、禁sdla，普通版权选型不阻塞。
3. Result/Status、阶段/JSONL和固定根已选；实际头自包含/异常/CTest身份仍缺。
4. L2逐值、独立L3、适用L4、真实误差/性能及DEC-005/006仍缺，0.01/0.1只筛查。
5. DEC-004已选：新save不补造HistoricalOnly，缺分析必需字段显式失败；真实实现/测试仍缺。

**全部LLD仍未冻结**。论文材料与静态推导、旧matio探针都不能扩大为项目验收。

## Golden 来源等级统一索引

以[无需MATLAB政策](../golden/NO_MATLAB_EXECUTION_POLICY.md)和[L2/L3/L4矩阵](../golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md)为准。作者582 MAT为L2来源，独立逐值/SHA尚缺；AN-G01～12和SH-PAPER均为候选，不是已经执行的参考；L4待实测。KvD/Adaptive需逐节点/逐轮独立参考，采样需全量有序输入与固定RNG消耗，球谐/轨迹需全矩阵与场重建，实验/分析需真实L2曲线及独立聚合参考。

[TEST_PLAN §3](../TEST_PLAN.md)仍维护66个TC。新[SH-PAPER](../golden/SH_PAPER_VERIFICATION_CASES.md)只在已有TC-F-SH-001/002与数值失败TC下细化10个候选用例，不新增TC，不改变运行权限、容差或验收状态。
