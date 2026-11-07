# Egomotion Estimation with Dense Flow Fields：MATLAB → C++ 源码审计与模块化移植计划

> **WBS 草案 v0.1，未形成承诺基线。** 98 项 P 任务的证据要求与可执行进度以 [PROGRESS.md](PROGRESS.md) 为唯一滚动登记；本文件不保存动态 PASS/失败计数。

> **HAR-2026-11-06-001 的可审查补充**：用户审批 JSON 的 H-01～H-16 已拆成 [审批后的设计修订与待审材料包](approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md)，包含 H-05 需求影响、H-06 冻结准备、H-07 历史 MAT 差异、H-08 SHA-256、H-09 Golden、H-13 异常、H-15 里程碑与 H-16 Draft PR 清单。该补充不是 SRS/LLD 冻结或 H-10 解除暂缓，P0～P10 的既有 REQ/TC/AC 义务与任务状态不变。

> **本文件为实际 MATLAB 源码审计后的修订计划，不代表任何 C++ 功能已经实现。**
>
> 审计依据：本仓库 main 分支的 origin/egomotion_estimation_dense_flow_fields_matlab_scripts/、AGENTS.md、原始资料自带 readme 和 2015 年论文。原始文件的目录和函数名均已在 GitHub 上核对；MAT 变量已由独立 C++17 matio 探针在 Ubuntu 24.04 上完整读取：582 个文件、582 个变量全部 `READ_OK`，见 [docs/MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md)。这不等于 MATLAB/C++ 算法数值等价。
>
> 本文件仅用于规划和跟踪，不构成启动 C++ 开发的授权。后续修改以 AGENTS.md 的工作区、提交、源数据保护及用户授权要求为准；本文档中出现的测试与运行入口均不能视为已实现。

## 0. 文档定位、需求基线与优先级

- **[docs/REQUIREMENTS.md](REQUIREMENTS.md)** 是独立的软件需求规格说明书（SRS），采用 `REQ-F-*`、`REQ-D-*`、`REQ-I-*`、`REQ-DC-*`、`REQ-NF-*`、`REQ-PROC-*` 编号，逐条定义优先级、依据状态、验收 `AC-*` 与计划测试 `TC-*`，并覆盖 P0–P10 全部 98 项任务。
- **本 PLAN.md** 只负责源码审计安排、实施分解、任务依赖、进度和验收顺序；P 工作项不能替代系统需求。
- **[docs/ARCHITECTURE.md](ARCHITECTURE.md)** 是 HLD 草案，**[docs/OBJECT_MODEL.md](OBJECT_MODEL.md)** 是类职责与生命周期草案，**[docs/design/README.md](design/README.md)** 索引各模块 LLD 草案；三者均未冻结。
- **[docs/TEST_PLAN.md](TEST_PLAN.md)** 规划 TC → 拟议测试实体 → AC，尚未创建 C++ 测试或实际 CTest。
- **[docs/PROGRESS.md](PROGRESS.md)** 是实施任务、里程碑、角色、实际验证与未决决定的滚动证据登记；初次静态核查见 [2026-11-06 审查记录](reviews/STATIC_TRACEABILITY_2026-11-06.md)，审查意见后的跨文档更正及连接器复核见 [一致性修订记录](reviews/DOC_CONSISTENCY_RECONCILIATION_2026-11-06.md)；二者均不代表 H-11 的 Bash 现场执行。
- **[AGENTS.md](../AGENTS.md)** 规定 Git 安全、Debian/Ubuntu 支持范围、MAT 原始资料只读保护与 AI Agent 工作要求。
- SRS 目前是 **v0.6 草案、尚未冻结、尚未验收**；需求项“依据已确认”不等于 C++ 已实现或测试通过。P0 对来源/数据/版权的审计结论与软件需求规格经项目所有者批准后才可冻结。所有未实测性能/内存/构建预算与逐模块容差保持 TBD，不得凭空编造。
- 原始 45 个 MATLAB 文件的逐文件追踪见 [SRS §9.2](REQUIREMENTS.md#92-原始-matlab-45-文件逐文件追踪清单已核实代码未移植)。下面每项 P 任务已标注至少一个关联需求 ID；后续修改 PLAN 子任务、SRS 需求、数值验收或 MATLAB 数据契约时须同步更新双向追踪矩阵和相关 AC/TC。

## 1. 目标、不可变约束与完成定义

**长期政策**：[无需 MATLAB 执行](golden/NO_MATLAB_EXECUTION_POLICY.md) 已由项目所有者明确选定：原 `.m` 仅只读审查，**禁止将运行 MATLAB/Octave 或取得 L1 视为任何 P 工作项、里程碑、基线或验收的前置条件**；应以 L2/L3/L4 独立参考、实际 C++ 测试及其明确覆盖范围为准。H-10/H-11 的其他真实执行暂缓仍有效。

本项目以忠实移植（Faithful Port）为首要原则，把 Simon Strübbe、Wolfgang Stürzl、Martin Egelhaaf 的 **Egomotion Estimation with Dense Flow Fields: MATLAB Scripts** 移植为可在 **Debian/Ubuntu** 上独立构建和运行的 **C++17** 程序，复现原始数值实验、数据输出与图表分析流程。

| 事项 | 固定决策 |
|---|---|
| C++ / 构建 | C++17、CMake、Eigen 3；**H-02 已选用 Sophus SO(3)** 为 Rodrigues 实际调用的替代依赖；H-14 方案 A 采用 Don Williamson C++ SHRotation 递推适配（当前未纳入代码） |
| MAT 读写 | matio；原始 MAT 只读，未来新 MAT 结果仅写仓库根 `output/<RunId>/` 隔离目录（H-12 已选定，已以 `/output/` 规则忽略上传）；**H-10 仍暂缓代码与执行** |
| 数据格式 | 保留原始 .mat，不开发 .npz/.npy 转换、不引入 cnpy |
| 运行环境 | 仅 Debian/Ubuntu；**H-04 选定 Ubuntu 24.04.4 VirtualBox/x86_64 开发环境快照**（可用容量已记录，未批准性能通过阈值）；不依赖 MATLAB Runtime、Python、ROS、OpenCV |
| 数值 | 以 MATLAB 原算法、原参数、原迭代与原始数据为基准；优先 double |
| 目标方法 | franzeigen 匹配滤波基线、Original KvD、Modified KvD、Adaptive MFA 单次更新 |
| 目标实验 | 两类 bias test；Adaptive MFA 初始化、含/不含噪声轨迹对比、更新频率扫描 |
| 图表 | C++ 实验的数值计算与绘图分层；后期复现数据分析及插图，不引入 Python |
| 原文件 | origin/** 只读、不可移动、改名、清理或覆盖 |
| Git | 严格遵守 AGENTS.md；源码实施使用开发分支；不强推、不改写历史 |
| 项目许可证与来源 | **项目所有者直接指定 MIT**，仓库根 [LICENSE](../LICENSE) 已创建；球谐选择原始 Ivanic–Ruedenberg 1996 论文及 1998 勘误作为必读数学依据，Don Williamson 仅供参考，正式递推必须使用 Eigen 且不能依赖 sdla；普通版权审核不再作为开发阻塞，只保留实际上游文件的已有许可告知 |

**完成定义**：所有被原始实验实际调用的数值计算、数据生成、MAT 结果读取、统计分析均有可追溯的 C++ 对应实现；在给定固定输入下能对照原作者 MAT 参考结果；未实现或未核验的功能显式报告。原始 MATLAB 文件本身不做修改。绘图输出的视觉风格不要求像素级一致，但所用数据、坐标、变换和曲线区间必须可核对。

**反目标**：不开发图像前端、LK 光流、ESKF、VO、外部三维地图或新的鲁棒优化器；不把非 MATLAB 原算法误写为“原论文复现”。不为让结果“好看”而修改原始阈值、噪声、随机调用顺序或删除失败样本。

## 2. 源码清单与实际实验边界（已核对）

### 2.1 资料结构

原始目录中共 **45 个 .m、582 个 .mat、1 个 readme**。

| 目录 | MATLAB 文件数 | 角色 |
|---|---:|---|
| bias_test/data_generation | 2 | 两种噪声与视场下的偏差实验 |
| bias_test/data_analysis | 1 | 汇总多个独立运行目录并绘制 bias 曲线 |
| adaptive_matched_filter_test/data_generation | 3 | 初始化、带/不带噪声轨迹、更新频率实验 |
| adaptive_matched_filter_test/data_analysis | 3 | 各实验的结果读取与曲线展示 |
| help_functions | 33 | 数学、采样、模拟、KvD、匹配滤波、球谐和轨迹 |
| illustrations | 3 | 球谐与球内两种轨迹插图 |
| produced_data 及其子目录 | 582 个 .mat | 作者保存的数值**结果**，优先作为只读回归基准 |

归档自带 readme 明确列出三类内容：Bias-test、Adaptive matched filter test、Illustrations；不能把该资料简化为“两个 C++ 实验程序”。**这 582 个 MAT 主要是已生成实验结果，不是脚本开始数值求解前必需的观测输入**：数据生成脚本自身产生视线、近度、运动和流场；分析脚本再加载 produced_data。因而必须把“重新生成”与“读取已有参考结果”拆开实现。

原始数据数量核对：
- bias_test/produced_data/occluded_regions/：40 个编号目录，每目录 xwerte.mat、ywerte1.mat…ywerte6.mat，共 280 个。
- bias_test/produced_data/proportional_error/：40 个编号目录、相同七类文件，共 280 个。
- adaptive_matched_filter_test/produced_data/initialization/：4 个。
- adaptive_matched_filter_test/produced_data/without_errors/：5 个。
- adaptive_matched_filter_test/produced_data/with_errors/：7 个。
- adaptive_matched_filter_test/produced_data/many_parameter/：6 个。
- 合计 280 + 280 + 4 + 5 + 7 + 6 = **582**。

**matio 全量读取实测**：[docs/MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) 显示 582/582 MAT v5 + zlib，582 个变量全部 MATLAB class=double、data_type=double、单文件一个同名变量，均 `READ_OK`、失败 0。真实维度分布：10×1（80）、10×3（480）、35×1（4）、600×1（14）、600×4（4）。本批归档无需 HDF5，也不存在需读取的 cell/struct/sparse/complex 类型。独立扫描器不是正式业务 Reader/Writer，未验证列主序值拷贝、异常负例或算法数值等价。

### 2.2 源文件对应的功能清单（计划中的一对一追踪依据）

| 原始 .m 文件（省略共同 help_functions/ 前缀） | 预定 C++ 职责 |
|---|---|
| skalarprodukt.m、kreuzprodukt.m、erzeugekreuzproduktmatrix.m | 三维向量运算与叉乘矩阵 |
| getvektor.m | 单位球面随机方向及随机数供给 |
| findeDreiecke.m、erzeugerichtungsvektoren.m、erzeugerichtungsvektoren_loch.m、richtungsvektoren_icosahedron.m | 八面体/缺损八面体/二十面体递归采样，保持叶节点顺序 |
| erzeugeflussausnear.m、fixed_kugel_symmetric_perfectpoints_randomnear.m、fixed_propfehler_kugel_symmetric_perfectpoints_randomnear.m、nonpropfehler.m | 连续球面光流生成、两类噪声及模拟数据 |
| franzeigen.m、zellendefinieren.m | 三轴基线匹配滤波，平移/旋转两类模板响应 |
| norm_koenderink_iteration.m | Original KvD 交替迭代 |
| mue_norm_koenderink_iteration.m | Modified KvD 交替迭代 |
| normrelfehler.m、transfehler_div.m | 不同定义的误差指标：角度误差/尺度对齐向量误差 |
| winkelausvektor.m、kugelflfunc2.m、near_aus_koeff.m、koeff_aus_near.m | 球面角、实球谐、近度展开及系数拟合 |
| SHRotate.m、transfor_koeff.m | 实球谐 l=1、l=2 系数旋转 |
| oneit_adaptiv_matched_filter.m | Adaptive MFA 单轮 6×6 系统、旋转和平移、近度与新系数 |
| rodrigues.m、drehvektor_aus_zwei_vektoren.m | 旋转向量/矩阵及方向间旋转 |
| kreis_trajektorie.m、sinuskurve_trajektorie.m、trajektorie_aus_punkten.m | 球内圆轨迹、正弦轨迹和局部自运动参数 |
| getnear_in_kugel_Drehung.m | 球内旋转后的射线到球面距离/近度 |
| getflowfield.m | 读取 MAT 文件第一个变量的原始兼容行为 |
| kurveglaetten.m | 数据分析阶段的窗口平滑 |

其余 12 个 .m 为 2+1 个 bias 实验及分析脚本、3+3 个 adaptive 实验及分析脚本、3 个 illustration 脚本；见下一节。共 33 + 12 = 45，均列入范围。具体 C++ 文件名只代表新项目模块，不假定与 MATLAB 文件逐一同名。

### 2.3 真正的实验入口与结果契约

| MATLAB 入口 / 对应分析 | 输入参数与重要行为 | 历史结果 |
|---|---|---|
| bias_test_nonspherical(corenumber, iterationnumber, savestring) / graphs_bias_test | 六面缺损八面体，N=6×4^i；高斯误差按**整个流场平均模长**缩放；输出 xwerte 和 ywerte1…6 | occluded_regions/1…40 |
| bias_test_spherical_prop_error(corenumber, iterationnumber, savestring) / graphs_bias_test | 完整八面体，N=8×4^i；噪声按**当前光流每个分量**缩放；同样六列误差统计 | proportional_error/1…40 |
| agent_in_sphere_initialization_test(savestring) / graph_agent_in_sphere_initialization_test | 圆轨迹，35 个位置，二十面体采样级别 2，对比每步重新用常数近度初始化与继承前状态 | initialization/ |
| agent_in_sphere_adaptive_matched_filter_test(addederror, savestring) / graph_agent_in_sphere_adaptive_matched_filter_test | 600 个位置正弦轨迹，二十面体级别 4，8 次运行，可加不按分量比例的噪声，比较自适应与每帧重置 | with_errors/、without_errors/ |
| agent_in_sphere_adaptive_matched_filter_many_parameter(savestring) / graph_agent_in_sphere_adaptive_matched_filter_many_parameter | 600 位置正弦轨迹，级别 2，更新间隔 **1、5、11、21** 帧；无该入口内的随机加噪调用 | many_parameter/ |

必须按照源码而非旧计划复制参数：原计划的更新间隔“1、5、10、20”**有误**，正确值为 **1、5、11、21**。各实验的采样级别、轨迹参数、重复次数不同，不应抽取一个虚构的“通用实验默认参数”覆盖所有入口。Bias-test 的 MATLAB 脚本实际写有 durchlaeufe=1，40 个结果编号目录是独立运行的集合，不能把单目录内重复次数和集合汇总次数混淆。

### 2.4 审计已发现的原版细节与待核验差异

1. **函数的数据约定**：setofpoints 是 N×6 矩阵，每行前 3 项为单位视线 d，后 3 项为三维切向光流 p。生成时使用 p = −near·(t−(t·d)d)−(ω×d)。不能把它当二维像素光流，或任意更换光流、角速度符号。
2. **Franz 基线**：zellendefinieren 产生的是 **3 行、每行 10 项的模板表**，在 franzeigen 内对三轴分别累积平移/旋转响应；这不是六个可互换的独立观测点。须保存响应的求和顺序和归一化。
3. **两种 KvD**：两个函数用 franzeigen 初始化、平移方向归一化、重估近度，并在循环中使用不同的平移方程；原始退出逻辑包含**至少 6 次迭代**、动态误差阈值、200 次上限及绝对阈值。向量更新的先后顺序对迭代结果有影响。
4. **近度符号、尺度**：KvD 使用近度和的符号翻转，并在最后按平均近度缩放平移；Adaptive 单轮先由球谐系数恢复近度、做均值归一化，再经 6×6 联立系统恢复运动，最后重算近度和系数。不能合并为同一无状态求解器。
5. **球谐表示**：kugelflfunc2 使用 MATLAB legendre 与指定的实球谐 sin/cos 规则；系数以 **3×5** 矩阵存储，其中有效分量数为 1+3+5=9，初始系数 (1,1)=1；8 个独立的非零阶环境参数与完整存储不矛盾。拟合 koeff_aus_near 是按每个基函数分别投影/归一的具体计算，并非可任意替换的通用最小二乘拟合。
6. **球谐旋转**：transfor_koeff 使用 SHRotate(r,2) 给出的 3×3 与 5×5 块，并保持零阶项。需校验旋转主动/被动方向及真实基函数顺序，不可直接替换为其他球谐库默认约定。
7. **来源与实现数学依据**：原作者材料及第三方代码保留原出处与现有文件头，项目新代码采用根 MIT。球谐 Eigen 独立重写必须在用户上传 1996 原论文和 1998 勘误后逐式核对；普通版权复核不作为开发/构建的前置门禁，复制已许可的上游文件时保留其既有版权/许可告知即可。
8. **偏差实验“variance”术语**：脚本注释把 [0.6,2.4,9.6] 称为 variances，但代码把数值直接作为 randn 乘数，并未取方差平方根。忠实模式须执行代码表达式，同时在报告中区分“脚本系数”与统计学方差。
9. **误差不是一种**：transfehler_div 先把估计向量模长缩放到真值模长，再计算向量差；normrelfehler 计算方向夹角（度）。绝不能以统一的角度误差替换 bias test 的六组输出。
10. **已有生成结果和脚本可能不完全同步**：with_errors/ 中包含当前生成脚本不保存的 alleadap* 文件；many_parameter/ 含 rottransratio.mat 与 alleadap*，但当前生成脚本未写出这些文件；many_parameter 的绘图脚本又会读取 rottransratio。根据 [DEC-004 最新决定](decisions/DEC-004_HISTORICAL_MAT_FIELDS.md)，**新 Runner 只生成当前有效 save 字段**；历史 MAT 完整只读，额外字段列为 `HistoricalOnly`；原因不明不阻塞开发，缺失分析必需变量时明确失败，不补造或删去历史。
11. **原代码潜在数值边界**：oneit 使用 real(inv(mgesamt))，并存在零范数、近度均值接近零、逆矩阵病态、acos 输入越界等风险；原 bias 生成函数的 near 输出在循环内作为标量覆写。必须首先忠实记录原行为及可能失败状态，再在单独、显式授权的增强模式中考虑更稳健的数值实现。
12. **MAT 加载兼容性**：getflowfield(filename) 调用 load 后取第一个变量；当前分析脚本经常不写 .mat 扩展名。MAT 访问模块必须支持按明确变量名严格读取，并提供与旧脚本兼容的“无后缀路径、唯一/首个变量”读取策略；两者不可混淆。

## 3. 架构与对象模型引用（详细内容迁至 HLD）

> 本 PLAN 保留**实施约束和 P0–P10 工作任务**，不再承载详细的类职责表、对象 UML、完整文件树或 CMake target 设计。独立设计文档是后续架构审查和 LLD 冻结的输入，不代表任何类已经实现。

### 3.1 模块与依赖边界

系统分为 domain/core_math、mat_io、sampling、flow_simulation、Franz/KvD/Adaptive estimators、harmonics、trajectory、experiments、analysis、可选 plotting、CLI/Composition Root。各模块完整职责和组件依赖图见 [docs/ARCHITECTURE.md](ARCHITECTURE.md) §2–4。

### 3.2 数据与接口合同

N×6 球面流场、3×5 实球谐系数、6×6 Adaptive 系统，以及 MATLAB 列主序、原始 MAT 只读、结果目录保护要求均由 [docs/REQUIREMENTS.md](REQUIREMENTS.md) 记录为可验证需求。技术接口的候选分层与物理数据流见 [docs/ARCHITECTURE.md](ARCHITECTURE.md)；方法签名需 P0.8 审阅。

### 3.3 面向对象设计原则

采用**值对象 + RAII + 组合/窄接口 + 无状态求解器与有状态会话分离**。无状态点积、叉积和基函数保留普通函数；不为每个 MATLAB .m 机械创建类。设计原则参见 [docs/ARCHITECTURE.md §3](ARCHITECTURE.md)，架构决策记录参见其 [§8 ADR](ARCHITECTURE.md)；两者均为未冻结 HLD 草案。

### 3.4 对象模型与 MATLAB 映射

FlowField、HarmonicCoefficients、MatFileHandle、FranzMatchedFilter、KvDIterationEngine、AdaptiveMfaStepSolver、AdaptiveMfaSession、实验 Runner 的职责与类关系见 [docs/OBJECT_MODEL.md](OBJECT_MODEL.md)；逐文件 MATLAB → REQ → TC 追踪见 [docs/REQUIREMENTS.md](REQUIREMENTS.md) §9.2。当前对象模型仅是草案，不代表已建类/方法。

### 3.5 所有权、生命周期与依赖注入

MAT RAII 不向数学层暴露 matio 指针；每次 RunContext 拥有独立 RNG、输出 Writer 和会话状态；Adaptive 单步求解与跨帧系数状态分离。生命周期、状态图、接口隔离详见 [docs/OBJECT_MODEL.md](OBJECT_MODEL.md) §2–4。

### 3.6 架构与详细设计冻结门槛

P0.7/P0.8 必须核验**全部 45 个 MATLAB 文件的处理决定、MAT 数据字典、类/接口责任及所有权、依赖无环、CLI 和错误码、逐模块测试与数值容差**。架构冻结清单见 [docs/ARCHITECTURE.md](ARCHITECTURE.md) §9（§8 为 ADR），LLD 的规范与状态见 [docs/design/README.md](design/README.md)。未核验项不得标记完成。

## 4. 规划中的工程文件结构

目录布局、公开/私有头文件边界、CMake target 候选清单统一见 [docs/ARCHITECTURE.md](ARCHITECTURE.md) §4，详细模块的设计草案见 [docs/design/](design/README.md)。核心分层为 include/egomotion 的 domain/contracts/io/math/geometry/simulation/estimators/harmonics/experiments/analysis，与 src/ 对应，另有 tests/unit、tests/contracts、tests/golden、tests/integration、tests/reproduction；命令行 composition root 位于 src/cli。

现有原始资料仍以 `origin/egomotion_estimation_dense_flow_fields_matlab_scripts/` 为只读参照。**新输出根固定为仓库根 `output/`，其下每次独占创建 `output/<RunId>/`；CLI 已删除自定义输出根选项**。`--run-id` 可选，无参数使用独立 OS 随机源生成身份（与算法 RNG 分离），重复 ID/目标拒绝覆盖。`.gitignore` 保留 `/output/` 排除。项目 MIT [LICENSE](../LICENSE) 已添加；Don Williamson 仅为算法参考，SH Rotation 通过 Eigen 重写完全移除 sdla，参考源/勘误见 [专门 LLD](design/SH_ROTATION_IVANIC_RUEDENBERG.md)。本轮不创建生产结果或运行测试。

## 5. 细粒度实施任务（每项均可单独检查并提交）

**WBS 证据列的性质**：本表的 98 个 P 工作项在「证据要求」中使用类别描述，**不表示**候选测试文件/CTest 已创建或分配了完成时间；每个 TC→唯一拟议测试实体/CTest→AC 的对照以 [TEST_PLAN.md §3](TEST_PLAN.md) 为唯一登记来源。关于 LLD 的具体候选路径：P2/P3 参照 [sampling_noise.md §8](design/sampling_noise.md) 的 `test_sampling.cpp`、`test_flow_simulation.cpp`；P5/P8 参照 [experiments.md §8.1](design/experiments.md) 的 `test_bias.cpp`、`test_adaptive_experiments.cpp`；P7 球谐参照 [harmonics.md §4.1](design/harmonics.md) 的 `test_harmonics.cpp`；P1 MAT/RAII 参照 [mat_io.md §5/§8](design/mat_io.md) 的 `test_mat_archive.cpp` 和候选 `test_mat_raii.cpp`。相关实体、CTest 名与调用/环境仍**未批准或运行**，本段不改变任何 P 的 REQ/TC/AC、依赖或状态。


**本节是 WBS，不代表 C++ 工作已经开始。** 每个 P 任务显式关联 REQ→TC→AC，证据列只描述应收集的证据类别；初始状态均未验收。设计草案/源码可读取并不等价于完成 P 任务。所有完整测试数据与执行记录统一按 [TEST_PLAN.md](TEST_PLAN.md) 管理。

### P0 — 基线冻结与来源审计（原始资料/设计准备）

| ID | 单一子任务与验收要点 | 前置 P | REQ | TC / AC | 证据要求与当前状态 |
|---|---|---|---|---|---|
| P0.1 | 固定本次源代码 commit SHA；清点 45 个 .m、1 个 readme、582 个 .mat，记录 SHA/大小，核验分类，不修改原归档 | — | REQ-D-SOURCE-001 | TC-D-SOURCE-001 / AC-D-SOURCE-001 | 文档、源文件哈希、评审记录；部分审计/草案；未验收 |
| P0.2 | 对 33 个辅助函数及 12 个入口/绘图文件逐一形成函数签名、被谁调用、是否实际用于实验、输入维度、输出变量清单 | P0.1 | REQ-D-SOURCE-001 | TC-D-SOURCE-001 / AC-D-SOURCE-001 | 文档、源文件哈希、评审记录；部分审计/草案；未验收 |
| P0.3 | 建立两套 bias 与四套 adaptive 归档子目录的结果文件清单；对照 MATLAB 的 save、load 和 getflowfield 调用 | P0.1 | REQ-D-SOURCE-001 | TC-D-SOURCE-001 / AC-D-SOURCE-001 | 文档、源文件哈希、评审记录；部分审计/草案；未验收 |
| P0.4 | 已按项目所有者最新决定创建项目 MIT 根 LICENSE；记录 Sophus、Don Williamson 参考实现与 Ivanic–Ruedenberg 1996 原论文/1998 勘误来源；计划用 Eigen 重写 R5，完全去除 sdla，不以普通版权审查阻塞代码编写 | P0.1 | REQ-D-SOURCE-002、REQ-NF-COPYRIGHT-001 | TC-D-SOURCE-002、TC-NF-COPYRIGHT-001 / AC-D-SOURCE-002、AC-NF-COPYRIGHT-001 | 文档、源文件哈希、评审记录；部分审计/草案；未验收 |
| P0.5 | 维护原始 MAT 文件身份、元数据字段及扫描失败分类方案；**MAT_DATA_SCHEMA 已由真实 matio 全量探针生成**。本 P0 审计任务与 AC-D-MAT-001 的完整实测/复核验收分开记录 | P0.1 | REQ-F-MAT-002、REQ-D-MAT-001 | TC-F-MAT-002、TC-D-MAT-001 / AC-F-MAT-002、AC-D-MAT-001 | 文档、源文件哈希、评审记录；部分审计/草案；未验收 |
| P0.6 | 维护 SOURCE_INVENTORY、**真实 MAT_DATA_SCHEMA**、MIGRATION_MATRIX 的追踪与来源；将已实测文件字段与尚需数值/授权/错误合同确认的内容区分。SOURCE_INVENTORY、MIGRATION_MATRIX 尚未完成，不自动视为全项验收 | P0.2、P0.3、P0.5 | REQ-D-MAT-001、REQ-PROC-TRACE-001 | TC-D-MAT-001、TC-PROC-TRACE-001 / AC-D-MAT-001、AC-PROC-TRACE-001 | 文档、源文件哈希、评审记录；部分审计/草案；未验收 |
| P0.7 | 复核并完善已有 docs/OBJECT_MODEL.md **草案**，覆盖 45 个 MATLAB 文件→候选方法及对象所有权，不把已有文件视为任务完成 | P0.2 | REQ-DC-DOMAIN-001、REQ-NF-OOP-001 | TC-DC-DOMAIN-001、TC-NF-OOP-001 / AC-DC-DOMAIN-001、AC-NF-OOP-001 | 文档、源文件哈希、评审记录；部分审计/草案；未验收 |
| P0.8 | 评审 HLD/对象模型/LLD 的最小接口与错误合同，明确当前已冻结范围和未决 DEC；不得将独立探针已成功解码 582/582 误写为生产 Reader/Writer、错误合同、数值 Golden 或架构已冻结 | P0.6、P0.7 | REQ-DC-DOMAIN-001、REQ-NF-OOP-001、REQ-I-CLI-001 | TC-DC-DOMAIN-001、TC-NF-OOP-001、TC-I-CLI-001 / AC-DC-DOMAIN-001、AC-NF-OOP-001、AC-I-CLI-001 | 文档、源文件哈希、评审记录；部分审计/草案；未验收 |

**P0/P1 数据审计现状**：源文件与归档已核对，MAT_DATA_SCHEMA 是**582 文件全量真实 matio 数据字典**；SRS、HLD、OBJECT_MODEL、TEST_PLAN、LLD 仍为未冻结草案，SOURCE_INVENTORY 与 MIGRATION_MATRIX 的全量内容未完成。P0 的文档合同、独立 P1 探针执行、生产 Reader/Writer 和 P10.4 审核均为独立门禁，不以扫描器成功替代整阶段验收。

### P1 — 独立构建、CLI 与 MAT 文件访问

> **已有局部实测**：独立 `src/mat_data_dict_probe.cpp` 已在 Ubuntu 24.04/GCC 13/matio 1.5.26 上编译运行；582 个 MAT 成功解码。完整 CMake 工程、业务 CLI、Reader/Writer、错误路径、写回读和 CTest 尚未验证。

| ID | 单一子任务与验收要点 | 前置 P | REQ | TC / AC | 证据要求与当前状态 |
|---|---|---|---|---|---|
| P1.1 | 待 H-10 解除暂缓后建立 Debian/Ubuntu CMake C++17 工程，声明 Eigen 3、matio、zlib 和 H-02 选定的 Sophus（只在相关 target 私有使用）；拟议 SHRotation C++ 适配同样须经版权保留和依赖核查；不编译 origin 下 MATLAB 文件 | P0.8 | REQ-NF-PLATFORM-001、REQ-NF-DEPS-001 | TC-NF-PLATFORM-001、TC-NF-DEPS-001 / AC-NF-PLATFORM-001、AC-NF-DEPS-001 | 源外构建/IO 合同、测试日志；未实施/未验收 |
| P1.2 | 依据 docs/design/cli.md 候选并经 P0.8 批准后建立 CLI 参数解析、退出码与独立输出路径；允许只执行 inspect 而不加载算法 | P1.1 | REQ-F-CLI-001、REQ-DC-EXPERIMENT-001、REQ-NF-PORTABLE-001、REQ-F-CLI-002、REQ-F-CLI-003、REQ-F-CLI-004、REQ-F-CLI-005、REQ-I-CLI-001 | TC-F-CLI-001、TC-DC-EXPERIMENT-001、TC-NF-PORTABLE-001、TC-F-CLI-002、TC-F-CLI-003、TC-F-CLI-004、TC-F-CLI-005、TC-I-CLI-001 / AC-F-CLI-001、AC-DC-EXPERIMENT-001、AC-NF-PORTABLE-001、AC-F-CLI-002、AC-F-CLI-003、AC-F-CLI-004、AC-F-CLI-005、AC-I-CLI-001 | 源外构建/IO 合同、测试日志；未实施/未验收 |
| P1.3 | 封装 matio 只读文件句柄和变量句柄的 RAII；错误路径无资源泄漏且不会写源文件 | P1.1 | REQ-F-MAT-001、REQ-DC-IO-001、REQ-NF-SAFETY-001、REQ-I-MAT-001 | TC-F-MAT-001、TC-DC-IO-001、TC-NF-SAFETY-001、TC-I-MAT-001 / AC-F-MAT-001、AC-DC-IO-001、AC-NF-SAFETY-001、AC-I-MAT-001 | 源外构建/IO 合同、测试日志；未实施/未验收 |
| P1.4 | 通过 MAT v5+zlib 读取数值变量的真实 rank、dims、类型、字节数及列主序内容；不静默转置或单精度升级 | P1.3 | REQ-F-CLI-001、REQ-F-MAT-001、REQ-D-MAT-002 | TC-F-CLI-001、TC-F-MAT-001、TC-D-MAT-002 / AC-F-CLI-001、AC-F-MAT-001、AC-D-MAT-002 | 源外构建/IO 合同、测试日志；未实施/未验收 |
| P1.5 | 按已核实的源 MAT v5/zlib + double/double + 五种二维形状确定实际读取范围；对非目标 cell/struct/complex/sparse/MAT v7.3 实现清晰的 UnsupportedClassOrType 拒绝和负例测试，避免无需求的扩展 | P1.4 | REQ-F-MAT-002 | TC-F-MAT-002 / AC-F-MAT-002 | 源外构建/IO 合同、测试日志；未实施/未验收 |
| P1.6 | 实现明确变量名加载和 getflowfield 兼容读取（首变量/自动 .mat 后缀）；分别测试，不能混淆 | P1.4 | REQ-F-CLI-001、REQ-F-MAT-003、REQ-I-MAT-001 | TC-F-CLI-001、TC-F-MAT-003、TC-I-MAT-001 / AC-F-CLI-001、AC-F-MAT-003、AC-I-MAT-001 | 源外构建/IO 合同、测试日志；未实施/未验收 |
| P1.7 | 待重新授权后**仅向 `output/<RunId>/` 新结果目录**写入 MATLAB 兼容 MAT 的结果文件，写后读取检查变量名、维度与数值；H-10 暂缓且 permissions.writeMat=false，本轮不得运行 | P1.3 | REQ-F-MAT-004、REQ-NF-SAFETY-001、REQ-I-MAT-001、REQ-I-RESULT-001 | TC-F-MAT-004、TC-NF-SAFETY-001、TC-I-MAT-001、TC-I-RESULT-001 / AC-F-MAT-004、AC-NF-SAFETY-001、AC-I-MAT-001、AC-I-RESULT-001 | 源外构建/IO 合同、测试日志；未实施/未验收 |
| P1.8 | **已有独立 matio 探针的 582/582 `READ_OK` 正式字典**（文件/变量/class/dtype/dims/压缩/FNV64）；后续核对业务 Reader 的类型/索引/复制/异常处理及独立数据复核，P10.4 完成审查后再判定 AC-D-MAT-001；不得把探针成功算作 Reader 完成 | P1.4 | REQ-F-MAT-001、REQ-D-MAT-001、REQ-NF-RUNTIME-001 | TC-F-MAT-001、TC-D-MAT-001、TC-NF-RUNTIME-001 / AC-F-MAT-001、AC-D-MAT-001、AC-NF-RUNTIME-001 | **扫描子任务已实测；生产 Reader/复核/性能未验收** |
| P1.9 | 待授权后建立 move-only MatFileHandle/MatVariableHandle 与 MatResultReader、RunMatWriter；以 `output/<RunId>/`/测试副本验证 RAII、无悬垂 Eigen Map、原始数据只读及拒绝覆盖；H-12 仅确认策略，当前无真实执行权限 | P1.3、P1.7 | REQ-F-MAT-001、REQ-F-MAT-004、REQ-DC-IO-001、REQ-NF-SAFETY-001 | TC-F-MAT-001、TC-F-MAT-004、TC-DC-IO-001、TC-NF-SAFETY-001 / AC-F-MAT-001、AC-F-MAT-004、AC-DC-IO-001、AC-NF-SAFETY-001 | 源外构建/IO 合同、测试日志；未实施/未验收 |
| P1.10 | 建立 IResultReader/IResultWriter 的窄接口与内存测试替身，在 composition root 注入；验证 CLI 和求解器均不直接依赖 matio C 指针 | P1.6、P1.7 | REQ-DC-IO-001 | TC-DC-IO-001 / AC-DC-IO-001 | 源外构建/IO 合同、测试日志；未实施/未验收 |

### P2 — 通用数学、递归采样与数据类型

| ID | 单一子任务与验收要点 | 前置 P | REQ | TC / AC | 证据要求与当前状态 |
|---|---|---|---|---|---|
| P2.1 | 统一 setofpoints N×6 与单位视线、切向流、角速度、平移、角度单位的数据合同 | P0.8 | REQ-F-GEOM-001、REQ-DC-DOMAIN-001 | TC-F-GEOM-001、TC-DC-DOMAIN-001 / AC-F-GEOM-001、AC-DC-DOMAIN-001 | 单元 Golden、采样顺序；未实施/未验收 |
| P2.2 | 对照 skalarprodukt、kreuzprodukt、erzeugekreuzproduktmatrix 实现向量基础运算与单元测试 | P2.1 | REQ-F-GEOM-001 | TC-F-GEOM-001 / AC-F-GEOM-001 | 单元 Golden、采样顺序；未实施/未验收 |
| P2.3 | 按 findeDreiecke 复现递归四分与叶节点顺序、单位化、边界层级；验证 N 随级别增长 | P2.2 | REQ-F-GEOM-002 | TC-F-GEOM-002 / AC-F-GEOM-002 | 单元 Golden、采样顺序；未实施/未验收 |
| P2.4 | 按 erzeugerichtungsvektoren 生成完整八面体 8×4^i 点，不改变排列顺序 | P2.3 | REQ-F-GEOM-002 | TC-F-GEOM-002 / AC-F-GEOM-002 | 单元 Golden、采样顺序；未实施/未验收 |
| P2.5 | 按 erzeugerichtungsvektoren_loch 生成缺损八面体 6×4^i 点，保留被省略的具体面 | P2.3 | REQ-F-GEOM-002 | TC-F-GEOM-002 / AC-F-GEOM-002 | 单元 Golden、采样顺序；未实施/未验收 |
| P2.6 | 按 richtungsvektoren_icosahedron 实现二十面体采样 20×4^i，保留原始顶点/面表 | P2.3 | REQ-F-GEOM-003 | TC-F-GEOM-003 / AC-F-GEOM-003 | 单元 Golden、采样顺序；未实施/未验收 |
| P2.7 | 按 getvektor 实现球面随机单位方向，建立显式 RNG 接口并记录固定输入黄金样本 | P2.1 | REQ-DC-RNG-001 | TC-DC-RNG-001 / AC-DC-RNG-001 | 单元 Golden、采样顺序；未实施/未验收 |
| P2.8 | 定义 FlowField、MotionEstimate、SolverResult、HarmonicCoefficients 的值语义和不变量；编写维度、复制/移动、生命周期与 MATLAB 列主序合同测试 | P2.1 | REQ-F-GEOM-001、REQ-D-MAT-002 | TC-F-GEOM-001、TC-D-MAT-002 / AC-F-GEOM-001、AC-D-MAT-002 | 单元 Golden、采样顺序；未实施/未验收 |
| P2.9 | 抽象 IDirectionSampler 并实现三种具体采样器，固定同一层级输出点序；仅在实验组装层选择具体类 | P2.4、P2.5、P2.6 | REQ-F-GEOM-003、REQ-DC-NOISE-001 | TC-F-GEOM-003、TC-DC-NOISE-001 / AC-F-GEOM-003、AC-DC-NOISE-001 | 单元 Golden、采样顺序；未实施/未验收 |

### P3 — 球面光流、环境与噪声

| ID | 单一子任务与验收要点 | 前置 P | REQ | TC / AC | 证据要求与当前状态 |
|---|---|---|---|---|---|
| P3.1 | erzeugeflussausnear：给定视线、近度、t、ω 计算平移/旋转/合成切向流和两种平均模长 | P2.2、P2.8 | REQ-F-FLOW-001 | TC-F-FLOW-001 / AC-F-FLOW-001 | 固定 RNG、切向误差；未实施/未验收 |
| P3.2 | 固定随机深度生成：按原脚本距离区间 [1,4]、平均值 2.5 归一，并取倒数；保存随机调用顺序 | P2.7 | REQ-F-FLOW-002 | TC-F-FLOW-002 / AC-F-FLOW-002 | 固定 RNG、切向误差；未实施/未验收 |
| P3.3 | fixed_kugel...：非球形视场 + 基于全场平均光流模长的高斯噪声，最后投影回切平面 | P3.1、P3.2、P2.5 | REQ-F-FLOW-003 | TC-F-FLOW-003 / AC-F-FLOW-003 | 固定 RNG、切向误差；未实施/未验收 |
| P3.4 | fixed_propfehler...：完整球面 + 基于各光流分量的高斯扰动，最后投影回切平面 | P3.1、P3.2、P2.4 | REQ-F-FLOW-003 | TC-F-FLOW-003 / AC-F-FLOW-003 | 固定 RNG、切向误差；未实施/未验收 |
| P3.5 | nonpropfehler：独立实现轨迹实验所用的平均流长尺度高斯噪声；与 P3.3 区分数据来源但可共享底层公式 | P3.1、P2.7 | REQ-F-FLOW-003 | TC-F-FLOW-003 / AC-F-FLOW-003 | 固定 RNG、切向误差；未实施/未验收 |
| P3.6 | getnear_in_kugel_Drehung：射线旋转、单位球内求交和近度公式，独立测试边界/非法输入 | P6.2、P2.2 | REQ-F-FLOW-004 | TC-F-FLOW-004 / AC-F-FLOW-004 | 固定 RNG、切向误差；未实施/未验收 |
| P3.7 | 固定 RNG 输入和人工已知光流的确定性参考检查；随机统计再另做验证，不用不同语言的同种子直接逐元素比较 | P3.1、P3.3、P3.4 | REQ-DC-RNG-001、REQ-NF-RNG-001 | TC-DC-RNG-001、TC-NF-RNG-001 / AC-DC-RNG-001、AC-NF-RNG-001 | 固定 RNG、切向误差；未实施/未验收 |
| P3.8 | 设计 IRandomSource（均匀/正态）和 FixedSequenceRandomSource、Mt19937RandomSource；确保每 RunId 独占并可验证调用顺序 | P2.7 | REQ-DC-RNG-001 | TC-DC-RNG-001 / AC-DC-RNG-001 | 固定 RNG、切向误差；未实施/未验收 |
| P3.9 | 设计 IFlowNoiseModel 的平均流长、逐分量、无噪声实现，注入模拟器；保持 MATLAB 三处噪声入口的不同参数语义 | P3.3、P3.4、P3.5 | REQ-F-FLOW-003、REQ-DC-NOISE-001 | TC-F-FLOW-003、TC-DC-NOISE-001 / AC-F-FLOW-003、AC-DC-NOISE-001 | 固定 RNG、切向误差；未实施/未验收 |

### P4 — 基线匹配滤波与 KvD 两条路径

| ID | 单一子任务与验收要点 | 前置 P | REQ | TC / AC | 证据要求与当前状态 |
|---|---|---|---|---|---|
| P4.1 | zellendefinieren：构造三个笛卡尔轴的模板与计数/响应存储 | P2.2 | REQ-F-FILTER-001 | TC-F-FILTER-001 / AC-F-FILTER-001 | 逐轮近度/t/ω、残差；未实施/未验收 |
| P4.2 | franzeigen：逐点累加平移与旋转模板响应、归一化，输出两种 3D 运动估计 | P4.1 | REQ-F-FILTER-001 | TC-F-FILTER-001 / AC-F-FILTER-001 | 逐轮近度/t/ω、残差；未实施/未验收 |
| P4.3 | 抽离 KvD 共享步骤：基线初始化、近度重估、平均近度符号、旋转更新、残差与迭代判定 | P4.2 | REQ-F-KVD-001、REQ-NF-NUM-001 | TC-F-KVD-001、TC-NF-NUM-001 / AC-F-KVD-001、AC-NF-NUM-001 | 逐轮近度/t/ω、残差；未实施/未验收 |
| P4.4 | norm_koenderink_iteration：保留含额外 near 因子的**原始平移更新**、更新顺序和尺度恢复 | P4.3 | REQ-F-KVD-001 | TC-F-KVD-001 / AC-F-KVD-001 | 逐轮近度/t/ω、残差；未实施/未验收 |
| P4.5 | mue_norm_koenderink_iteration：保留移除额外 near 因子的**修正平移更新**，不依靠一个容易误配的布尔选项隐藏差异 | P4.3 | REQ-F-KVD-001 | TC-F-KVD-001 / AC-F-KVD-001 | 逐轮近度/t/ω、残差；未实施/未验收 |
| P4.6 | 精确复现两者至少 6 次迭代、最多 200 次、动态阈值和绝对阈值组合的退出逻辑；输出迭代次数/残差 | P4.4、P4.5 | REQ-F-KVD-002 | TC-F-KVD-002 / AC-F-KVD-002 | 逐轮近度/t/ω、残差；未实施/未验收 |
| P4.7 | transfehler_div 与 normrelfehler 分别实现与测试；特别验证零范数或 acos 越界的定义和状态 | P2.2 | REQ-F-ERROR-001、REQ-NF-NUM-002 | TC-F-ERROR-001、TC-NF-NUM-002 / AC-F-ERROR-001、AC-NF-NUM-002 | 逐轮近度/t/ω、残差；未实施/未验收 |
| P4.8 | 用固定 N×6 流场，依据源静态公式建立**独立 L3 逐轮近度/运动/残差/停止**参考，并与 Franz、两种 KvD 的 C++ trace 比较；不要求运行 MATLAB | P4.6、P4.7 | REQ-F-KVD-002、REQ-NF-NUM-001 | TC-F-KVD-002、TC-NF-NUM-001 / AC-F-KVD-002、AC-NF-NUM-001 | 逐轮近度/t/ω、残差；未实施/未验收 |
| P4.9 | 实现 KvDIterationEngine + IKvDTranslationRule 的组合以及 OriginalKvDEstimator、ModifiedKvDEstimator 两个明确入口；测试算法差异、状态不串扰和停止条件 | P4.4、P4.5、P4.6 | REQ-F-KVD-001、REQ-DC-KVD-001 | TC-F-KVD-001、TC-DC-KVD-001 / AC-F-KVD-001、AC-DC-KVD-001 | 逐轮近度/t/ω、残差；未实施/未验收 |
| P4.10 | 将 FranzMatchedFilter 做成可独立实例化、无跨样本状态的求解服务，检查其模板存储私有性及 Run 之间的行为一致性 | P4.2 | REQ-F-FILTER-001 | TC-F-FILTER-001 / AC-F-FILTER-001 | 逐轮近度/t/ω、残差；未实施/未验收 |

### P5 — Bias 实验及 40 目录结果分析

| ID | 单一子任务与验收要点 | 前置 P | REQ | TC / AC | 证据要求与当前状态 |
|---|---|---|---|---|---|
| P5.1 | bias_test_nonspherical：按实参 corenumber、iterationnumber、savestring 运行，保持 3 个误差系数及 6 组输出 | P3.3、P4.9、P1.7 | REQ-F-BIAS-001、REQ-F-CLI-002 | TC-F-BIAS-001、TC-F-CLI-002 / AC-F-BIAS-001、AC-F-CLI-002 | MAT 写回读、Bias 统计；未实施/未验收 |
| P5.2 | bias_test_spherical_prop_error：与 P5.1 共用 runner 接口，保持完整球面和逐分量噪声差异 | P3.4、P4.9、P1.7 | REQ-F-BIAS-002、REQ-F-CLI-002 | TC-F-BIAS-002、TC-F-CLI-002 / AC-F-BIAS-002、AC-F-CLI-002 | MAT 写回读、Bias 统计；未实施/未验收 |
| P5.3 | 实现独立 corenumber/seed 的运行管理、实际重复数和结果目录，不把 40 个已存结果目录误作单次重复 | P1.2、P3.8 | REQ-F-BIAS-001、REQ-NF-RNG-001、REQ-NF-LOG-001、REQ-NF-RECOVERY-001、REQ-F-CLI-002 | TC-F-BIAS-001、TC-NF-RNG-001、TC-NF-LOG-001、TC-NF-RECOVERY-001、TC-F-CLI-002 / AC-F-BIAS-001、AC-NF-RNG-001、AC-NF-LOG-001、AC-NF-RECOVERY-001、AC-F-CLI-002 | MAT 写回读、Bias 统计；未实施/未验收 |
| P5.4 | 为每次运行写 xwerte.mat 和 ywerte1…6.mat；保持 MATLAB 原有矩阵形状、列含义及输出变量名 | P1.7、P5.1、P5.2 | REQ-F-BIAS-001、REQ-D-MAT-002、REQ-I-RESULT-001 | TC-F-BIAS-001、TC-D-MAT-002、TC-I-RESULT-001 / AC-F-BIAS-001、AC-D-MAT-002、AC-I-RESULT-001 | MAT 写回读、Bias 统计；未实施/未验收 |
| P5.5 | graphs_bias_test 数值层：从 1…40 读取指定 xwerte、ywerte3…6，计算平均、理论 100/sqrt(N) 参考曲线，保留起始索引/范围 | P1.6、P1.8 | REQ-F-BIAS-003 | TC-F-BIAS-003 / AC-F-BIAS-003 | MAT 写回读、Bias 统计；未实施/未验收 |
| P5.6 | 用 occluded_regions 和 proportional_error 的 560 个归档 MAT 对照输出，保留每个独立运行数据与汇总误差，不只看图 | P5.5、P5.4 | REQ-F-BIAS-003 | TC-F-BIAS-003 / AC-F-BIAS-003 | MAT 写回读、Bias 统计；未实施/未验收 |
| P5.7 | 把两种 Bias 模式组合为注入采样器、噪声模型、RNG、求解器和 IResultWriter 的 BiasExperimentRunner；测试两模式配置相互隔离和 MAT 结果输出路径权限 | P5.1、P5.2、P5.3 | REQ-F-BIAS-001、REQ-DC-EXPERIMENT-001 | TC-F-BIAS-001、TC-DC-EXPERIMENT-001 / AC-F-BIAS-001、AC-DC-EXPERIMENT-001 | MAT 写回读、Bias 统计；未实施/未验收 |

### P6 — 刚体旋转、运动轨迹与误差

| ID | 单一子任务与验收要点 | 前置 P | REQ | TC / AC | 证据要求与当前状态 |
|---|---|---|---|---|---|
| P6.1 | drehvektor_aus_zwei_vektoren：方向间旋转轴角，处理近零与反向向量情形并记录原始 MATLAB 行为 | P2.2 | REQ-F-TRAJ-001 | TC-F-TRAJ-001 / AC-F-TRAJ-001 | 轨迹/旋转黄金向量；未实施/未验收 |
| P6.2 | **H-02 指定 Sophus `SO3d::exp` 替代 rodrigues** 实际使用的旋转向量→3×3 分支，保留原小角 `eps` 分支、Rᵀ 局部坐标语义并准备独立 L3 固定输入旋转参考（不执行 MATLAB）；未用的逆向/Jacobian 不机械移植；H-10 暂缓实际实现 | P2.2 | REQ-F-TRAJ-001 | TC-F-TRAJ-001 / AC-F-TRAJ-001 | 轨迹/旋转黄金向量；未实施/未验收 |
| P6.3 | kreis_trajektorie：圆轨迹点序列、半径、高度，避免生成函数含有绘图副作用 | P2.1 | REQ-F-TRAJ-001 | TC-F-TRAJ-001 / AC-F-TRAJ-001 | 轨迹/旋转黄金向量；未实施/未验收 |
| P6.4 | sinuskurve_trajektorie：正弦轨迹点序列、周期、幅值、长度、高度，保持离散分母和端点语义 | P2.1 | REQ-F-TRAJ-001 | TC-F-TRAJ-001 / AC-F-TRAJ-001 | 轨迹/旋转黄金向量；未实施/未验收 |
| P6.5 | trajektorie_aus_punkten：相邻位置推导平移、绝对/相对转动并转到原约定坐标系；保留两端未填写元素 | P6.1、P6.2、P6.3、P6.4 | REQ-F-TRAJ-001 | TC-F-TRAJ-001 / AC-F-TRAJ-001 | 轨迹/旋转黄金向量；未实施/未验收 |
| P6.6 | 轨迹、矩阵和位置求交的已知输入黄金检查；确保点始终在单位球内（或明确报错） | P6.5、P3.6 | REQ-F-TRAJ-001 | TC-F-TRAJ-001 / AC-F-TRAJ-001 | 轨迹/旋转黄金向量；未实施/未验收 |
| P6.7 | 通过 TrajectoryGenerator、TrajectoryKinematics、SphereEnvironment 细分职责；验证创建不同轨迹不修改现有 Session 状态，轨迹函数不得直接绘图 | P6.5 | REQ-F-TRAJ-001 | TC-F-TRAJ-001 / AC-F-TRAJ-001 | 轨迹/旋转黄金向量；未实施/未验收 |

### P7 — 实球谐与 Adaptive MFA 单步求解

| ID | 单一子任务与验收要点 | 前置 P | REQ | TC / AC | 证据要求与当前状态 |
|---|---|---|---|---|---|
| P7.1 | winkelausvektor：atan2 方位角与 acos 极角，保持轴约定 | P2.2 | REQ-F-SH-001 | TC-F-SH-001 / AC-F-SH-001 | 球谐与 6×6 中间量；未实施/未验收 |
| P7.2 | kugelflfunc2：按 MATLAB legendre 的约定计算 l≤2 的实球谐，核对负 m 的 sin 与正 m 的 cos | P7.1 | REQ-F-SH-001 | TC-F-SH-001 / AC-F-SH-001 | 球谐与 6×6 中间量；未实施/未验收 |
| P7.3 | near_aus_koeff：以 3×5 系数矩阵恢复离散近度；严格限定有效位置 | P7.2 | REQ-F-SH-001 | TC-F-SH-001 / AC-F-SH-001 | 球谐与 6×6 中间量；未实施/未验收 |
| P7.4 | koeff_aus_near：逐项投影、4π 归一及范数除法，明确其与一般联合最小二乘的差异 | P7.2 | REQ-F-SH-001 | TC-F-SH-001 / AC-F-SH-001 | 球谐与 6×6 中间量；未实施/未验收 |
| P7.5 | **所有者确定：待上传 Ivanic–Ruedenberg 1996 原论文与 1998 勘误后，参考 Don Williamson 的递推，以 Eigen 完全重写 l=0/1/2 的 R0/R3/R5；禁止 sdla include/link/vendor/间接依赖**，普通版权审查不再阻塞开发；本轮不实施代码 | P7.2、P6.2 | REQ-F-SH-002 | TC-F-SH-002 / AC-F-SH-002 | 球谐与 6×6 中间量；未实施/未验收 |
| P7.6 | transfor_koeff：结合 **Sophus 输出的 3×3 物理旋转**与 **Don Williamson/ Ivanic 生成的 3×3、5×5 球谐块**，校准 R1 轴置换、R5 的 25 项、m 顺序及正向/逆向旋转；独立 L3 全元素 Golden/批准容差先于数值等价认定 | P7.5 | REQ-F-SH-002 | TC-F-SH-002 / AC-F-SH-002 | 球谐与 6×6 中间量；未实施/未验收 |
| P7.7 | oneit_adaptiv_matched_filter 的右端向量、四个 3×3 子块与 6×6 联立系统：分别建立可独立对照的数值阶段 | P7.3、P2.2 | REQ-F-ADAPT-001 | TC-F-ADAPT-001 / AC-F-ADAPT-001 | 球谐与 6×6 中间量；未实施/未验收 |
| P7.8 | 单轮运动向量解析、平移归一化、残差、近度再计算、二阶球谐系数重估；明确状态输入/输出，不混成多帧类 | P7.4、P7.7 | REQ-F-ADAPT-001 | TC-F-ADAPT-001 / AC-F-ADAPT-001 | 球谐与 6×6 中间量；未实施/未验收 |
| P7.9 | 对照原始 inv/real 的行为和 Eigen 解算差异，记录奇异、零范数及非有限输出，不未经验证更改默认求解结果 | P7.8 | REQ-F-ADAPT-001、REQ-NF-NUM-001、REQ-NF-NUM-002 | TC-F-ADAPT-001、TC-NF-NUM-001、TC-NF-NUM-002 / AC-F-ADAPT-001、AC-NF-NUM-001、AC-NF-NUM-002 | 球谐与 6×6 中间量；未实施/未验收 |
| P7.10 | 将球谐基函数、拟合和旋转分为 RealSphericalHarmonics、HarmonicFitter、HarmonicRotator；类只管理固定约定，不存多帧 mutable 状态 | P7.2、P7.4、P7.6 | REQ-F-SH-001 | TC-F-SH-001 / AC-F-SH-001 | 球谐与 6×6 中间量；未实施/未验收 |
| P7.11 | 实现无状态 AdaptiveMfaStepSolver 的明确输入/输出合同，使其可单独用固定 FlowField + 旧 HarmonicCoefficients 测试，且不继承 KvD 求解器 | P7.8 | REQ-F-ADAPT-001、REQ-DC-ADAPT-001 | TC-F-ADAPT-001、TC-DC-ADAPT-001 / AC-F-ADAPT-001、AC-DC-ADAPT-001 | 球谐与 6×6 中间量；未实施/未验收 |

### P8 — 三种 Adaptive MFA 实验驱动

| ID | 单一子任务与验收要点 | 前置 P | REQ | TC / AC | 证据要求与当前状态 |
|---|---|---|---|---|---|
| P8.1 | agent_in_sphere_initialization_test：35 点圆轨迹，对比每帧零阶重置与继承系数，按脚本输出四个误差 MAT | P7.11、P6.5、P3.1、P1.7 | REQ-F-EXP-001、REQ-F-CLI-003 | TC-F-EXP-001、TC-F-CLI-003 / AC-F-EXP-001、AC-F-CLI-003 | 逐帧会话及输出契约；未实施/未验收 |
| P8.2 | agent_in_sphere_adaptive_matched_filter_test：600 点正弦轨迹、级别 4、8 次重复，保留 addederror 参数和重置/继承两套曲线 | P8.3、P3.5、P1.7 | REQ-F-EXP-002、REQ-F-CLI-003 | TC-F-EXP-002、TC-F-CLI-003 / AC-F-EXP-002、AC-F-CLI-003 | 逐帧会话及输出契约；未实施/未验收 |
| P8.3 | P8.2 中的前一帧 Rodrigues 旋转与系数更新顺序单独实现并测试，不与当前帧求解顺序混淆 | P8.8、P7.6 | REQ-F-ADAPT-002 | TC-F-ADAPT-002 / AC-F-ADAPT-002 | 逐帧会话及输出契约；未实施/未验收 |
| P8.4 | P8.2 独立 MAT 输出四种误差及 rottransratio，分别对照 with_errors 和 without_errors | P8.2、P1.7 | REQ-F-EXP-002、REQ-I-RESULT-001 | TC-F-EXP-002、TC-I-RESULT-001 / AC-F-EXP-002、AC-I-RESULT-001 | 逐帧会话及输出契约；未实施/未验收 |
| P8.5 | agent_in_sphere_adaptive_matched_filter_many_parameter：级别 2、更新间隔 **1/5/11/21**、真实更新 akkoeff 与多组冻结/周期刷新 mitkoeff | P8.8、P7.11、P6.5 | REQ-F-ADAPT-003、REQ-F-EXP-003、REQ-F-CLI-003 | TC-F-ADAPT-003、TC-F-EXP-003、TC-F-CLI-003 / AC-F-ADAPT-003、AC-F-EXP-003、AC-F-CLI-003 | 逐帧会话及输出契约；未实施/未验收 |
| P8.6 | 比较“每步自身求解”和“独立 periodic baseline”的区别，并保存相应 MAT；不得将二者混成一组状态 | P8.5 | REQ-F-ADAPT-003、REQ-F-EXP-003 | TC-F-ADAPT-003、TC-F-EXP-003 / AC-F-ADAPT-003、AC-F-EXP-003 | 逐帧会话及输出契约；未实施/未验收 |
| P8.7 | 按已确定 DEC-004，在历史字段映射中将当前不保存但归档已有的 alleadap*、rottransratio 标 `HistoricalOnly`；**当前 C++ Runner 不新增兼容输出**，历史文件只读；新结果进行需要缺失比值的分析时明确报告错误，不等待归因 | P8.4、P8.6、P1.8 | REQ-F-EXP-003、REQ-D-MAT-003、REQ-I-RESULT-001 | TC-F-EXP-003、TC-D-MAT-003、TC-I-RESULT-001 / AC-F-EXP-003、AC-D-MAT-003、AC-I-RESULT-001 | 逐帧会话及输出契约；未实施/未验收 |
| P8.8 | 引入 AdaptiveMfaSession（唯一主系数所有者）与 ICoefficientUpdatePolicy（重置、继承、周期刷新），以测试替身检验旋转→基线更新→求解→主状态提交的逐帧时序 | P7.11、P7.6 | REQ-F-ADAPT-002、REQ-DC-ADAPT-001 | TC-F-ADAPT-002、TC-DC-ADAPT-001 / AC-F-ADAPT-002、AC-DC-ADAPT-001 | 逐帧会话及输出契约；未实施/未验收 |
| P8.9 | 为 InitializationRunner、AdaptiveComparisonRunner、UpdateRateRunner 分别提供 RunContext 及独立会话；测试多组更新频率和多次重复运行间不共享可变状态 | P8.1、P8.2、P8.5 | REQ-F-ADAPT-002、REQ-DC-ADAPT-001、REQ-NF-LOG-001、REQ-NF-SESSION-001 | TC-F-ADAPT-002、TC-DC-ADAPT-001、TC-NF-LOG-001、TC-NF-SESSION-001 / AC-F-ADAPT-002、AC-DC-ADAPT-001、AC-NF-LOG-001、AC-NF-SESSION-001 | 逐帧会话及输出契约；未实施/未验收 |

### P9 — 结果分析、绘图与论文插图

| ID | 单一子任务与验收要点 | 前置 P | REQ | TC / AC | 证据要求与当前状态 |
|---|---|---|---|---|---|
| P9.1 | graph_agent_in_sphere_initialization_test 的数据抽取、对数坐标和不同曲线取样区间 | P1.6、P1.8 | REQ-F-AN-001、REQ-F-CLI-004、REQ-I-ANALYSIS-001 | TC-F-AN-001、TC-F-CLI-004、TC-I-ANALYSIS-001 / AC-F-AN-001、AC-F-CLI-004、AC-I-ANALYSIS-001 | 历史 MAT 曲线/展示；未实施/未验收 |
| P9.2 | graph_agent_in_sphere_adaptive_matched_filter_test 的旋转/平移误差、rottransratio 双纵轴以及分段规则，阈值 0.01 | P1.6、P1.8 | REQ-F-AN-001、REQ-I-ANALYSIS-001 | TC-F-AN-001、TC-I-ANALYSIS-001 / AC-F-AN-001、AC-I-ANALYSIS-001 | 历史 MAT 曲线/展示；未实施/未验收 |
| P9.3 | graph_agent_in_sphere_adaptive_matched_filter_many_parameter 的四组曲线、平滑窗口 2/5/10、阈值 0.02、分段区间 | P1.6、P1.8 | REQ-F-AN-001、REQ-I-ANALYSIS-001 | TC-F-AN-001、TC-I-ANALYSIS-001 / AC-F-AN-001、AC-I-ANALYSIS-001 | 历史 MAT 曲线/展示；未实施/未验收 |
| P9.4 | graphs_bias_test 的 40-run 平均和 log-log 曲线、参考 100/sqrt(N)，输入/输出与计算分层 | P5.5 | REQ-F-BIAS-003、REQ-F-CLI-004、REQ-I-ANALYSIS-001 | TC-F-BIAS-003、TC-F-CLI-004、TC-I-ANALYSIS-001 / AC-F-BIAS-003、AC-F-CLI-004、AC-I-ANALYSIS-001 | 历史 MAT 曲线/展示；未实施/未验收 |
| P9.5 | kurveglaetten 的边缘零值和窗口处理按原始行为验证；不自行修改为不同边界策略 | P2.2 | REQ-F-AN-001 | TC-F-AN-001 / AC-F-AN-001 | 历史 MAT 曲线/展示；未实施/未验收 |
| P9.6 | 评估可选 C++/gnuplot 等 Debian/Ubuntu 绘图后端，默认不影响无图形运行。导出数值与图像，保证对照曲线的索引、纵横坐标、单位一致 | P9.1、P9.2、P9.3、P9.4 | REQ-F-AN-002、REQ-NF-HEADLESS-001、REQ-F-CLI-005 | TC-F-AN-002、TC-NF-HEADLESS-001、TC-F-CLI-005 / AC-F-AN-002、AC-NF-HEADLESS-001、AC-F-CLI-005 | 历史 MAT 曲线/展示；未实施/未验收 |
| P9.7 | circle_curve_in_the_sphere、sinosodial_curve_in_the_sphere、spherical_harmonic_illustration 三个展示脚本独立实现/记录视觉等价性；不依赖 MATLAB/Python | P6.3、P6.4、P7.2、P9.6 | REQ-F-AN-002、REQ-F-CLI-005 | TC-F-AN-002、TC-F-CLI-005 / AC-F-AN-002、AC-F-CLI-005 | 历史 MAT 曲线/展示；未实施/未验收 |
| P9.8 | 用 BiasResultAnalyzer / AdaptiveResultAnalyzer 分离 MAT 读取与数值汇总；可选 IPlotExporter 仅消费分析结果，并用内存数据测试其不影响求解结果 | P9.1、P9.2、P9.3、P9.4 | REQ-F-AN-002、REQ-DC-ANALYSIS-001 | TC-F-AN-002、TC-DC-ANALYSIS-001 / AC-F-AN-002、AC-DC-ANALYSIS-001 | 历史 MAT 曲线/展示；未实施/未验收 |

### P10 — 端到端核验、文档与交付

| ID | 单一子任务与验收要点 | 前置 P | REQ | TC / AC | 证据要求与当前状态 |
|---|---|---|---|---|---|
| P10.1 | 更新 README：删除旧 NumPy/.npz 迁移承诺，声明原始 MAT 直接访问和实际可运行入口，保留两份 DOI | P1.2 | REQ-PROC-TRACE-001、REQ-NF-COPYRIGHT-001 | TC-PROC-TRACE-001、TC-NF-COPYRIGHT-001 / AC-PROC-TRACE-001、AC-NF-COPYRIGHT-001 | 需求验收、统计及 PR 记录；未实施/未验收 |
| P10.2 | 形成每个 MATLAB 函数至 C++ 模块的最终追踪表（45/45 有处理决定：已移植、仅展示、无调用但保留、受授权阻塞） | P0.2、P8.9、P9.8 | REQ-PROC-TRACE-001 | TC-PROC-TRACE-001 / AC-PROC-TRACE-001 | 需求验收、统计及 PR 记录；未实施/未验收 |
| P10.3 | 逐层运行 MAT I/O、数学、采样、基线、KvD、球谐、自适应、实验和画图测试；记录命令、版本、退出码、日志 | P1.8、P4.8、P7.9、P8.9 | REQ-NF-NUM-001、REQ-NF-NUM-002、REQ-PROC-TRACE-001、REQ-NF-RECOVERY-001 | TC-NF-NUM-001、TC-NF-NUM-002、TC-PROC-TRACE-001、TC-NF-RECOVERY-001 / AC-NF-NUM-001、AC-NF-NUM-002、AC-PROC-TRACE-001、AC-NF-RECOVERY-001 | 需求验收、统计及 PR 记录；未实施/未验收 |
| P10.4 | 分别以原始归档的 560 个 bias MAT 和 22 个 adaptive MAT 核对结果契约；核对历史产物与当前源码的差异，不强求随机逐样本相等 | P1.8、P5.6、P8.7 | REQ-D-MAT-001、REQ-D-MAT-003 | TC-D-MAT-001、TC-D-MAT-003 / AC-D-MAT-001、AC-D-MAT-003 | 需求验收、统计及 PR 记录；未实施/未验收 |
| P10.5 | 复现论文对应偏差、初始化、轨迹及更新频率实验的数值图表；每组注明实际样本/噪声/状态 | P9.8、P10.3、P10.4 | REQ-NF-NUM-001、REQ-NF-RNG-001 | TC-NF-NUM-001、TC-NF-RNG-001 / AC-NF-NUM-001、AC-NF-RNG-001 | 需求验收、统计及 PR 记录；未实施/未验收 |
| P10.6 | 汇总 Debian/Ubuntu 的已执行构建、CTest 和实验结果，生成 NUMERICAL_EQUIVALENCE 与 PROGRESS；保留失败样本和未验证平台 | P10.5、P1.1 | REQ-NF-PLATFORM-001、REQ-NF-DEPS-001、REQ-NF-LOG-001、REQ-NF-HEADLESS-001、REQ-NF-PORTABLE-001、REQ-NF-RUNTIME-001 | TC-NF-PLATFORM-001、TC-NF-DEPS-001、TC-NF-LOG-001、TC-NF-HEADLESS-001、TC-NF-PORTABLE-001、TC-NF-RUNTIME-001 / AC-NF-PLATFORM-001、AC-NF-DEPS-001、AC-NF-LOG-001、AC-NF-HEADLESS-001、AC-NF-PORTABLE-001、AC-NF-RUNTIME-001 | 需求验收、统计及 PR 记录；未实施/未验收 |
| P10.7 | 项目根 MIT 已创建；为实际引用上游文件保留必要原 notice/来源 SHA，不将根 MIT 扩展到原始归档数据；不提交 `output/` 生成结果；无需另行版权签字作为正常开发门禁 | P0.4、P10.1 | REQ-D-SOURCE-002、REQ-NF-COPYRIGHT-001 | TC-D-SOURCE-002、TC-NF-COPYRIGHT-001 / AC-D-SOURCE-002、AC-NF-COPYRIGHT-001 | 需求验收、统计及 PR 记录；未实施/未验收 |
| P10.8 | 创建或更新对应 Draft PR 供审查；不得自动合并 main，按 AGENTS.md 报告 SHA、状态及未办事项 | P10.6、P10.7 | REQ-PROC-TRACE-001 | TC-PROC-TRACE-001 / AC-PROC-TRACE-001 | 需求验收、统计及 PR 记录；未实施/未验收 |
| P10.9 | 验收 OOP 边界：接口可替换、对象所有权明确、无悬垂 MAT 内存、无跨实验状态泄漏、类方法到全部 MATLAB 函数追踪完整；记录类关系和测试证据 | P10.3、P10.4 | REQ-PROC-TRACE-001、REQ-NF-OOP-001、REQ-NF-SESSION-001 | TC-PROC-TRACE-001、TC-NF-OOP-001、TC-NF-SESSION-001 / AC-PROC-TRACE-001、AC-NF-OOP-001、AC-NF-SESSION-001 | 需求验收、统计及 PR 记录；未实施/未验收 |
| P10.10 | 检查无过度继承/巨型类/虚假统一求解器；保证纯数学函数不为 OOP 改变求解顺序、精度与实验结果合同 | P10.9 | REQ-PROC-TRACE-001、REQ-NF-OOP-001 | TC-PROC-TRACE-001、TC-NF-OOP-001 / AC-PROC-TRACE-001、AC-NF-OOP-001 | 需求验收、统计及 PR 记录；未实施/未验收 |

## 6. 子任务依赖、并行、里程碑与进出准则

**审批适用的执行门禁**：上传 JSON H-10 为 `deferred`、H-11 为 `deferred`，全部运行/构建/写入/合并权限位均为 false；本阶段只能更新计划、LLD 和 `.gitignore`，不能编译、运行脚本、**执行** Golden 数值测试、合并 PR 或按已完成数计入 P 项。完整逐项解释见 [审批落实记录](approval/DECISION_IMPLEMENTATION_NOTES.md)。

本计划每个 P 子任务的**直接前置依赖**已在 §5 的表格中列出，共 98 个节点。当前依赖检查依据：每个前置 ID 存在，且图中没有环；这是一份静态的规划图检查，不等于已有 C++ 构建或实现。阶段编号并非实际严格串行顺序，例如 P3.6 的几何求交依赖 P6.2 的 Rodrigues 数学，允许跨阶段依赖。

### 6.1 关键子任务依赖（摘要）

~~~mermaid
flowchart LR
  A["P0.1 原始清单"] --> B["P0.7 对象合同"]
  B --> C["P0.8 设计评审"]
  C --> D["P1.1 CMake 工程"]
  D --> E["P1.3 MAT RAII"]
  E --> F["P1.4 MAT 压缩读取"]
  F --> G["P1.8 582 文件解码/字典"]
  C --> H["P2.2 数学/向量"]
  H --> I["P4.2 Franz"]
  I --> J["P4.3 KvD 公共步骤"]
  J --> K["P4.4/5 两种平移公式"]
  H --> L["P7.2 实球谐"]
  L --> M["P7.7 6×6 块"]
  M --> N["P7.8 Adaptive 单步"]
  N --> O["P8.8 Session"]
  O --> P["P8.2/5 轨迹实验"]
  G --> Q["P10.4 MAT 结果核对"]
  P --> Q
  K --> R["P5.1/2 Bias 实验"]
  R --> Q
  Q --> S["P10.5 论文复现"]
  S --> T["P10.8 交付审查"]
~~~

模块/target 的 DAG 与接口依赖另由 [ARCHITECTURE.md](ARCHITECTURE.md) §4 维护，本节只负责 P 子任务依赖。多个 Agent 并行实施必须分别使用独立工作区及开发分支；严禁抢占共享 HEAD/暂存区或覆盖其他任务产物。

### 6.2 里程碑与交付物

| 里程碑 | 交付范围 | 进入条件 | 退出条件和证据 |
|---|---|---|---|
| M0 源资料与设计评审门禁 | P0.1–P0.8 的可评审资料、SRS/HLD/LLD 草案 | 原资料目录可只读访问、AGENTS 已核对 | 45 文件来源清单、**已生成的 582/582 MAT 全量 matio 字典及探针证据**、最小接口待批准范围和开放 DEC；**生产 Reader/Writer、异常负例、原始资料身份审查及 P10.4 独立复核尚未完成，M0 尚未正式验收** |
| M1 MAT 只读/新结果写入闭环 | P1 + P2 基础值类型 | M0 最小合同审查、开发分支远程检查点 | **582/582 MAT 的独立探针读取及类型/shape 字典已有证据**；尚需生产 Reader/Writer、原始只读/结果写入路径保护、异常负例、值校验、写后回读的真实日志及 P10.4 独立复核 |
| M2 Bias 计算与统计 | P2–P5 的共享数学、噪声、Franz、KvD、Bias | M1 + 相关数学/采样合同 | Original/Modified 每轮对照和两个 40-run 统计，含失败样本 |
| M3 Adaptive 数值闭环 | P6–P8 轨迹、球谐和自适应 Session | 所需旋转/球谐代码及基准可用，版权障碍已处理 | 35/600 点与 1/5/11/21 更新率实验，合同及错误数有证据 |
| M4 图表和原文复现 | P9–P10 | M2/M3 必须输出可校验 MAT | 四种分析、三幅插图与论文相关曲线数据复现；容差与例外批准 |
| M5 可审查交付 | P10 交付、需求/设计/测试映射 | M4 证据已收集或明确批准阻塞 | 真实 SHA/命令/环境/失败日志与 Draft PR 可复核，不自动合并 |

任务没有固定工时承诺；里程碑也不代表已经开始。预算、性能、人员分工应以项目所有者确认的资源和首次实测为依据。**M0–M5 的当前状态、可用证据、尚缺退出条件在 [PROGRESS.md §3](PROGRESS.md) 更新；没有被批准的日期/预算保持 TBD，不得替代承诺。**

### 6.3 角色与职责

以下是**角色定义，不是人事任命**。角色的当前签署与证据责任状态见 [PROGRESS.md §4](PROGRESS.md)。

| 角色 | 需要承担的职责 | 是否已指定具体人员 |
|---|---|---|
| 项目所有者/需求批准人 | SRS/接口/DEC、权利范围及验收例外批准 | 角色已确定，具体审核记录待实际发生 |
| 源码/数据审计负责人 | 原 45 个文件、MAT 数据字典、版权线索、脚本—结果差异 | 尚未正式分派 |
| C++ 模块开发负责人 | 所属 P 子任务、接口、代码提交与实测记录 | 尚未正式分派 |
| 数值验证负责人 | Golden 数据、容差提案、对照误差、复现实验 | 尚未正式分派 |
| 集成/维护负责人 | 分支与工作区隔离、无环依赖、PR、最终证据审查 | 尚未正式分派 |

不为尚未分派的工作编造个人姓名或已获批准的时间表。

### 6.4 任务状态、完成度与阻塞口径

状态使用：**未开始 / 进行中（有证据）/ 代码已提交未验证 / 已通过验证 / 因 DEC 阻塞 / 已验收**。文档已创建是“有审查素材”，不是“代码已完成”或“已验收”。

- **完成百分比**按“经批准并有实际证据的 P 子任务数 / 98”计算；单项不因撰写草案而自动记作完成。必要时分别报审计/设计完成度与代码/测试完成度，不混用。
- 对 M 级需求，数值容差、实际数据或版权待决导致的不可判定状态不能统计为验收通过。
- 任何阶段退出需附该阶段实际 REQ/TC/AC/测试实体/输出 SHA 与失败数；未经真实构建的任务不写“编译成功”。
- 当前可见的文档提交不代表 98 个 P 任务已验收。**当前进行中状态、里程碑缺口和 E-DATA/E-DOC 证据索引已登记在 [docs/PROGRESS.md](PROGRESS.md)**；后续必须随真实代码、测试、批准记录持续回填，不得将已存在的登记表等同于 98 项任务已验收。

> **Golden 取证与里程碑口径（长期决策）**：以 [无需 MATLAB 执行政策](golden/NO_MATLAB_EXECUTION_POLICY.md) 为当前依据。L1 原 MATLAB 逐节点、RNG 和异常运行记录**永久退出必需证据/里程碑门禁**；L2 作者 582 MAT 最终结果、L3 独立固定输入逐节点解析/重实现、L4 统计是现行可行参考来源。P4/P7/P8/P9/P10 不得以缺 L1 为阻塞理由，但独立 L3 尚需完整覆盖、真实 C++/CTest 和 DEC-005/008 仍未批准，故不能宣称任务或 M0–M5 已验收。未经许可不运行 MATLAB/Octave 或任何受 H-10/H-11 限制的真实命令。

**设计决定已明确，避免重复审批**：Result/Status 采用轻量封装（内含 variant）、ErrorStage 稳定枚举、JSONL；实验每入口独立配置，依赖状态异常不提交、连续序列不能安全恢复则终止、八次聚合缺样本不能改除数、Manifest 区分 Complete/Incomplete/Failed；生产 C++ 实施应等用户上传球谐原文/勘误再启动相关递推。阶段一改代码、二隔离 CMake/CTest、三验收/合并分别授权，不能因选择方案视为已经运行/通过。

## 7. 数值等价与科学复现的验收标准

> 本节是**验证实施规划**。正式需求义务、候选与已批准容差、验收编号和测试编号以 [docs/REQUIREMENTS.md](REQUIREMENTS.md) 为需求基线；不得因本节存在文字验收要求就标记相关需求已通过。

1. **结构与对象边界一致**：MAT 变量名、class/type、维度、列主序、数值和压缩格式被正确解释；原始只读/运行输出写入由不同对象管理；MatFileHandle move-only、句柄释放后领域值对象不悬垂。单变量一个文件的旧兼容规则与多变量严格访问规则都有测试。
2. **几何一致**：三种视线采样都满足原始数量公式、单位范数、递归顺序；光流切向满足 d·p≈0，翻转方向、符号、坐标系由固定黄金样本检验。
3. **算法一致**：Original KvD 与 Modified KvD 分别保留不同方程；Franz 3 轴模板、Adaptive 6×6 线性系统以及球谐拟合/旋转都可对照逐步结果。
4. **异常真实**：MAT 损坏、形状不匹配、零模向量、病态求解、非有限数、未定义方向误差必须明确报告，原版异常不应被无声“修复”成通过。
5. **数据与随机分离**：历史 `rtol=1e-10` 方案不再作为现行候选；固定输入**已选定首轮宽松筛查** `atol=0.01`、`rtol=0.1`，但点数/次序/索引/状态与物理不变量另设严格门禁，最终分模块 PASS 门槛仍待 DEC-005 审定，见 [容差提案](golden/GOLDEN_TOLERANCE_PROPOSAL.md)。随机实验优先用 L4 对照脚本规定的统计结果，不要求不同 RNG 在同种子下逐位一致；独立 L3 参考用于逐节点核验，**无需 MATLAB 运行**。
6. **实验一致**：bias 的 40 个原始子目录与 Adaptive 的四类数据集彼此独立统计；无噪/含噪、初始化、更新间隔均与对应源码参数一致。
7. **图表一致**：曲线使用同一结果矩阵、定义与索引；图片格式和外观可不同，但曲线数值、相对比较、纵横轴单位不可改变。
8. **状态透明**：生成代码、提交远端、构建、CTest、原始数据回归、论文复现、PR 交付均逐项报告，没有执行的内容不能写成通过。
9. **面向对象合格**：每个持有可变状态的类有唯一所有者；业务运行器不包含数学公式/直接 matio 调用；算法可在固定输入和注入假对象下独立测试；OOP 重构不改变原始数值结果。

## 8. 实施风险、监测与 DEC 单一来源

需求与待决事项的**唯一正式 DEC 编号、状态和批准条件**见 [REQUIREMENTS.md](REQUIREMENTS.md) §8；本节只列实施风险及应对工作，不复制另一套“已经批准”的决策表。

| 风险 ID | 对应 DEC/需求 | 风险信号 | 实施响应、责任角色与状态 |
|---|---|---|---|
| R-01 | DEC-001/002，REQ-D-MAT-001 | 现存归档已 582/582 读取成功，但新增 MAT 或正式 Reader 可能出现未支持类型/形状 | 当前数据边界已经确认；P1.5/1.9 做拒绝负例与 Reader 合同，P10.4 独立复核；未全部验收 |
| R-02 | DEC-003/007，REQ-D-SOURCE-002 | 球谐 Eigen 重写的数学/勘误/相位未验证，原引用代码包含 sdla 类型 | 等待用户上传 1996 原文+1998 勘误，逐式对齐、以 Eigen 实现并对 L3 全元素验证；上游实际复用文件保留必要来源告知，**普通版权复核不阻塞研发** |
| R-03 | DEC-004，REQ-D-MAT-003 | 历史 MAT 与当前 save 列表不一致 | **处理规则已由所有者选定**：当前只写当前 save，历史字段 HistoricalOnly、只读保留；分析缺字段返回错误，来源未知不阻塞研发；仍待 Runner/Analyzer 与 CTest 实施 |
| R-04 | DEC-005，REQ-NF-NUM-001 | MATLAB/Eigen 计算顺序与奇异边界导致误差 | P4.8/P7.9 固定输入逐轮对照，容差按条件数批准；未冻结 |
| R-05 | DEC-006，REQ-NF-RUNTIME-001 | 原配置资源消耗大、内存或构建版本不兼容 | P10.6 首测 wall/CPU/RSS、Debian/Ubuntu 版本；未测 |
| R-06 | REQ-DC-*、REQ-NF-OOP-001 | 状态、I/O 与计算模块耦合 | P0.7/P0.8 对象/target DAG 审查，独立测试；未冻结 |
| R-07 | REQ-NF-SAFETY-001 | 结果路径与 origin 重叠或误覆盖旧结果 | P1.9 独占新 RunId 输出、符号链接负例；未测试 |
| R-08 | REQ-NF-RNG-001 | MATLAB/C++ 同 seed 不是同样随机数 | P3.7、P10.5 固定输入和统计复现分开；已知风险，未验证 |

风险发生后应记录对应 P 任务、证据 SHA、影响需求和已采取措施；真正的新缺陷按 AGENTS.md 检索 Issue 后处理，外部临时网络错误不创建项目 Issue。

## 9. 来源与版本声明

- 原始论文：**Insect-Inspired Self-Motion Estimation with Dense Flow Fields—An Adaptive Matched Filter Approach**，Strübbe / Stürzl / Egelhaaf（2015），https://doi.org/10.1371/journal.pone.0128413
- MATLAB 原始资料：**Egomotion Estimation with Dense Flow Fields: MATLAB Scripts**，https://doi.org/10.4119/unibi/2736885
- 本仓库原始文件目录：origin/egomotion_estimation_dense_flow_fields_matlab_scripts/
- 本项目规则：[AGENTS.md](../AGENTS.md)
- **审计深度**：45 份 MATLAB 文件与原始目录已核对；独立 matio C++ 探针已在 Ubuntu 24.04 编译运行并逐文件读取 **582/582 MAT、582/582 变量，失败 0**；来源及字段见 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md)。**尚未**运行原 MATLAB 求解器、编译完整 C++ 业务项目、建立 CTest、复现数值算法或审查完整第三方版权。
- 本计划只代表后续可执行工作拆分，不视为任何阶段已交付。后续开发须对照实际源码进一步填充函数/数据对照文档，并在产生新事实时以独立提交更新计划。

**最终架构决策：C++17 + Eigen 3 + matio/zlib + CMake；以值对象、RAII、窄接口、策略组合、无状态求解器和有状态实验会话实现面向对象架构；无状态数学公式保留函数形式；保留原始 MAT 文件与完整原始资料目录；按模块移植 45 个 MATLAB 文件所涵盖的数值和分析流程；每个子任务独立验证后集成；目标仅 Debian/Ubuntu。**
