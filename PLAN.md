# Egomotion Estimation with Dense Flow Fields：MATLAB → C++ 源码审计与模块化移植计划

> **本文件为实际 MATLAB 源码审计后的修订计划，不代表任何 C++ 功能已经实现。**
>
> 审计依据：本仓库 main 分支的 origin/egomotion_estimation_dense_flow_fields_matlab_scripts/、AGENTS.md、原始资料自带 readme 和 2015 年论文。原始文件的目录和函数名均已在 GitHub 上核对；MAT 数据的文件头已抽样读取，但全部变量的二进制内容尚未逐个解压、验证。
>
> 本轮工作只修订 PLAN.md，不创建、运行或编译 C++ 代码，不修改 origin、README、AGENTS 或数据文件。

## 0. 文档定位、需求基线与优先级

- **[docs/REQUIREMENTS.md](docs/REQUIREMENTS.md)** 是独立的软件需求规格说明书（SRS），采用 `REQ-F-*`、`REQ-D-*`、`REQ-I-*`、`REQ-DC-*`、`REQ-NF-*`、`REQ-PROC-*` 编号，逐条定义优先级、依据状态、验收 `AC-*` 与计划测试 `TC-*`，并覆盖 P0–P10 全部 98 项任务。
- **本 PLAN.md** 只负责源码审计安排、实施分解、任务依赖、进度和验收顺序；P 工作项不能替代系统需求。
- **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)** 是 HLD 草案，**[docs/OBJECT_MODEL.md](docs/OBJECT_MODEL.md)** 是类职责与生命周期草案，**[docs/design/README.md](docs/design/README.md)** 索引各模块 LLD 草案；三者均未冻结。
- **[docs/TEST_PLAN.md](docs/TEST_PLAN.md)** 规划 TC → 拟议测试实体 → AC，尚未创建 C++ 测试或实际 CTest。
- **[AGENTS.md](AGENTS.md)** 规定 Git 安全、Debian/Ubuntu 支持范围、MAT 原始资料只读保护与 AI Agent 工作要求。
- SRS 目前是 **v0.2 草案、尚未冻结、尚未验收**；需求项“依据已确认”不等于 C++ 已实现或测试通过。P0 对来源/数据/版权的审计结论与软件需求规格经项目所有者批准后才可冻结。所有未实测性能/内存/构建预算与逐模块容差保持 TBD，不得凭空编造。
- 原始 45 个 MATLAB 文件的逐文件追踪见 [SRS §9.2](docs/REQUIREMENTS.md#92-原始-matlab-45-文件逐文件追踪清单已核实代码未移植)。下面每项 P 任务已标注至少一个关联需求 ID；后续修改 PLAN 子任务、SRS 需求、数值验收或 MATLAB 数据契约时须同步更新双向追踪矩阵和相关 AC/TC。

## 1. 目标、不可变约束与完成定义

本项目以忠实移植（Faithful Port）为首要原则，把 Simon Strübbe、Wolfgang Stürzl、Martin Egelhaaf 的 **Egomotion Estimation with Dense Flow Fields: MATLAB Scripts** 移植为可在 **Debian/Ubuntu** 上独立构建和运行的 **C++17** 程序，复现原始数值实验、数据输出与图表分析流程。

| 事项 | 固定决策 |
|---|---|
| C++ / 构建 | C++17、CMake、Eigen 3 |
| MAT 读写 | matio；原始 MAT 只读，新增 MAT 结果写入独立输出目录 |
| 数据格式 | 保留原始 .mat，不开发 .npz/.npy 转换、不引入 cnpy |
| 运行环境 | 仅 Debian/Ubuntu；不依赖 MATLAB Runtime、Python、ROS、OpenCV |
| 数值 | 以 MATLAB 原算法、原参数、原迭代与原始数据为基准；优先 double |
| 目标方法 | franzeigen 匹配滤波基线、Original KvD、Modified KvD、Adaptive MFA 单次更新 |
| 目标实验 | 两类 bias test；Adaptive MFA 初始化、含/不含噪声轨迹对比、更新频率扫描 |
| 图表 | C++ 实验的数值计算与绘图分层；后期复现数据分析及插图，不引入 Python |
| 原文件 | origin/** 只读、不可移动、改名、清理或覆盖 |
| Git | 严格遵守 AGENTS.md；源码实施使用开发分支；不强推、不改写历史 |
| 授权 | 不自动创建 LICENSE，第三方来源需审查版权，特别是 rodrigues.m 和 SHRotate.m |

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

已用 GitHub 直接读取多个代表性 .mat 的 base64 内容和文件头：样本均标识 **MATLAB 5.0 MAT-file**，数据块出现 miCOMPRESSED（类型 15）。所以本项目的 **matio + zlib** 是实际必需路径；**暂不引入 HDF5**，只有在完整清单中查出 MAT v7.3 文件时再依其需求增加。尚未解压检查全部变量的维度和类型，不能将推断的输出结构冒充已经完成的数据校验。

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
7. **第三方来源**：rodrigues.m 标注 Perona/Bouguet 的历史版权及修改；SHRotate.m 说明 MATLAB 实现源于外部 C++ 代码与相关论文。P0 必须保留文件头并单独完成这两项的版权/移植来源核查，不能仅依整个资料页的 ODC-By 声明推定可复制代码。
8. **偏差实验“variance”术语**：脚本注释把 [0.6,2.4,9.6] 称为 variances，但代码把数值直接作为 randn 乘数，并未取方差平方根。忠实模式须执行代码表达式，同时在报告中区分“脚本系数”与统计学方差。
9. **误差不是一种**：transfehler_div 先把估计向量模长缩放到真值模长，再计算向量差；normrelfehler 计算方向夹角（度）。绝不能以统一的角度误差替换 bias test 的六组输出。
10. **已有生成结果和脚本可能不完全同步**：with_errors/ 中包含当前生成脚本不保存的 alleadap* 文件；many_parameter/ 含 rottransratio.mat 与 alleadap*，但当前生成脚本未写出这些文件；many_parameter 的绘图脚本又会读取 rottransratio。需要记录这些为“历史工件与当前生成脚本契约不一致，原因尚未确认”，不得悄悄补造值或删去历史文件。
11. **原代码潜在数值边界**：oneit 使用 real(inv(mgesamt))，并存在零范数、近度均值接近零、逆矩阵病态、acos 输入越界等风险；原 bias 生成函数的 near 输出在循环内作为标量覆写。必须首先忠实记录原行为及可能失败状态，再在单独、显式授权的增强模式中考虑更稳健的数值实现。
12. **MAT 加载兼容性**：getflowfield(filename) 调用 load 后取第一个变量；当前分析脚本经常不写 .mat 扩展名。MAT 访问模块必须支持按明确变量名严格读取，并提供与旧脚本兼容的“无后缀路径、唯一/首个变量”读取策略；两者不可混淆。

## 3. 架构与对象模型引用（详细内容迁至 HLD）

> 本 PLAN 保留**实施约束和 P0–P10 工作任务**，不再承载详细的类职责表、对象 UML、完整文件树或 CMake target 设计。独立设计文档是后续架构审查和 LLD 冻结的输入，不代表任何类已经实现。

### 3.1 模块与依赖边界

系统分为 domain/core_math、mat_io、sampling、flow_simulation、Franz/KvD/Adaptive estimators、harmonics、trajectory、experiments、analysis、可选 plotting、CLI/Composition Root。各模块完整职责和组件依赖图见 [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) §2–4。

### 3.2 数据与接口合同

N×6 球面流场、3×5 实球谐系数、6×6 Adaptive 系统，以及 MATLAB 列主序、原始 MAT 只读、结果目录保护要求均由 [docs/REQUIREMENTS.md](docs/REQUIREMENTS.md) 记录为可验证需求。技术接口的候选分层与物理数据流见 [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)；方法签名需 P0.8 审阅。

### 3.3 面向对象设计原则

采用**值对象 + RAII + 组合/窄接口 + 无状态求解器与有状态会话分离**。无状态点积、叉积和基函数保留普通函数；不为每个 MATLAB .m 机械创建类。详细设计原则和 ADR 见 [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) §3/7。

### 3.4 对象模型与 MATLAB 映射

FlowField、HarmonicCoefficients、MatFileHandle、FranzMatchedFilter、KvDIterationEngine、AdaptiveMfaStepSolver、AdaptiveMfaSession、实验 Runner 的职责与类关系见 [docs/OBJECT_MODEL.md](docs/OBJECT_MODEL.md)；逐文件 MATLAB → REQ → TC 追踪见 [docs/REQUIREMENTS.md](docs/REQUIREMENTS.md) §9.2。当前对象模型仅是草案，不代表已建类/方法。

### 3.5 所有权、生命周期与依赖注入

MAT RAII 不向数学层暴露 matio 指针；每次 RunContext 拥有独立 RNG、输出 Writer 和会话状态；Adaptive 单步求解与跨帧系数状态分离。生命周期、状态图、接口隔离详见 [docs/OBJECT_MODEL.md](docs/OBJECT_MODEL.md) §2–4。

### 3.6 架构与详细设计冻结门槛

P0.7/P0.8 必须核验**全部 45 个 MATLAB 文件的处理决定、MAT 数据字典、类/接口责任及所有权、依赖无环、CLI 和错误码、逐模块测试与数值容差**。架构冻结清单见 [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) §8，LLD 的规范与状态见 [docs/design/README.md](docs/design/README.md)。未核验项不得标记完成。

## 4. 规划中的工程文件结构

目录布局、公开/私有头文件边界、CMake target 候选清单统一见 [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) §4，详细模块的设计草案见 [docs/design/](docs/design/README.md)。核心分层为 include/egomotion 的 domain/contracts/io/math/geometry/simulation/estimators/harmonics/experiments/analysis，与 src/ 对应，另有 tests/unit、tests/contracts、tests/golden、tests/integration、tests/reproduction；命令行 composition root 位于 src/cli。

现有原始资料仍以 origin/egomotion_estimation_dense_flow_fields_matlab_scripts/ 为唯一来源。新结果必须使用独立 results/<RunId>/ 目录，不能写入 origin。以上目录均为**拟议布局**，当前尚未创建。

## 5. 细粒度实施任务（每项均可单独检查并提交）

**新增 OOP 执行要求**：P0 的对象模型与契约评审作为 P1–P10 实施前置条件。每次新增或调整类，都要标明所属模块、构造依赖、公开 API、不可变量、可变状态所有者和测试替身；代码审查按 3.3–3.6 的验收门槛逐项核对。保留原子提交，不把设计重构与算法公式修改混在同一提交。

任务 ID 为相互独立的**最小交付单元**；建议按依赖顺序串行集成，同一阶段内部没有依赖的项可由隔离的 Agent 并行完成。每项至少给出：对应原始 .m、输入/输出契约、修改文件、单元或静态检查、是否满足验收；不准用“整个阶段已完成”代替单项证据。

### P0 — 基线冻结与来源审计（纯文档）

| ID | 子任务 / 验收 |
|---|---|
| P0.1 | 固定本次源代码 commit SHA；清点 45 个 .m、1 个 readme、582 个 .mat，记录 SHA/大小，核验分类，不修改原归档 （需求：REQ-D-SOURCE-001） |
| P0.2 | 对 33 个辅助函数及 12 个入口/绘图文件逐一形成函数签名、被谁调用、是否实际用于实验、输入维度、输出变量清单 （需求：REQ-D-SOURCE-001） |
| P0.3 | 建立两套 bias 与四套 adaptive 归档子目录的结果文件清单；对照 MATLAB 的 save、load 和 getflowfield 调用 （需求：REQ-D-SOURCE-001） |
| P0.4 | 核查 rodrigues.m、SHRotate.m 的第三方来源和可移植范围；记录未能确认的版权问题，不擅自添加 LICENSE （需求：REQ-D-SOURCE-002、REQ-NF-COPYRIGHT-001） |
| P0.5 | 对压缩 MAT 完成全目录文件头、版本、压缩、变量名、类型、形状的只读审计（待 matio 检查工具可用时验证）；缺失项标记未核验 （需求：REQ-F-MAT-002、REQ-D-MAT-001） |
| P0.6 | 将上述信息形成 SOURCE_INVENTORY、MAT_DATA_SCHEMA、MIGRATION_MATRIX，并记录已有旧计划的错误参数及历史结果/当前脚本不一致项 （需求：REQ-D-MAT-001、REQ-PROC-TRACE-001） |
| P0.7 | 复核并完善现有 docs/OBJECT_MODEL.md **草案**：逐个 MATLAB 函数定位到类方法或纯函数；补齐可核验的输入输出、不变量、所有权和寿命，在批准前不标冻结 （需求：REQ-DC-DOMAIN-001、REQ-NF-OOP-001） |
| P0.8 | 复核 docs/ARCHITECTURE.md / docs/OBJECT_MODEL.md 的类图、组件图、依赖与时序；冻结经核验的接口、错误合同和设计决定，未决事项单独记录 （需求：REQ-DC-DOMAIN-001、REQ-NF-OOP-001、REQ-I-CLI-001） |

P0 当前仅完成一部分只读人工审计；以上文档尚未创建。源文件是否存在已确认，不等于 P0.5 的二进制变量检查已完成。

### P1 — 独立构建、CLI 与 MAT 文件访问

| ID | 子任务 / 验收 |
|---|---|
| P1.1 | 建立仅针对 Debian/Ubuntu 的 CMake C++17 工程，声明 Eigen 3、matio、zlib 实际依赖；不编译 origin 下 MATLAB 文件 （需求：REQ-NF-PLATFORM-001、REQ-NF-DEPS-001） |
| P1.2 | 建立 CLI 参数解析、退出码与独立输出路径；允许只执行 inspect 而不加载算法 （需求：REQ-F-CLI-001、REQ-DC-EXPERIMENT-001、REQ-NF-PORTABLE-001、REQ-F-CLI-002、REQ-F-CLI-003、REQ-F-CLI-004、REQ-F-CLI-005、REQ-I-CLI-001） |
| P1.3 | 封装 matio 只读文件句柄和变量句柄的 RAII；错误路径无资源泄漏且不会写源文件 （需求：REQ-F-MAT-001、REQ-DC-IO-001、REQ-NF-SAFETY-001、REQ-I-MAT-001） |
| P1.4 | 通过 MAT v5+zlib 读取数值变量的真实 rank、dims、类型、字节数及列主序内容；不静默转置或单精度升级 （需求：REQ-F-CLI-001、REQ-F-MAT-001、REQ-D-MAT-002） |
| P1.5 | 递进实现真实数据需要的 struct/cell、复数、逻辑或稀疏类型；未要求的类型明确拒绝，禁止伪称全部支持 （需求：REQ-F-MAT-002） |
| P1.6 | 实现明确变量名加载和 getflowfield 兼容读取（首变量/自动 .mat 后缀）；分别测试，不能混淆 （需求：REQ-F-CLI-001、REQ-F-MAT-003、REQ-I-MAT-001） |
| P1.7 | 实现**仅向新结果目录**写入 MATLAB 兼容 MAT 的结果文件，写后读取检查变量名、维度与数值 （需求：REQ-F-MAT-004、REQ-NF-SAFETY-001、REQ-I-MAT-001、REQ-I-RESULT-001） |
| P1.8 | 以原始归档的代表样本为 golden 只读验证，记录源文件哈希、输出形状、误差及实际命令 （需求：REQ-F-MAT-001、REQ-D-MAT-001、REQ-NF-RUNTIME-001） |
| P1.9 | 建立 move-only MatFileHandle/MatVariableHandle 与 MatResultReader、RunMatWriter；验证 RAII、无悬垂 Eigen Map、只读原始数据和拒绝覆盖 origin/历史 RunId （需求：REQ-F-MAT-001、REQ-F-MAT-004、REQ-DC-IO-001、REQ-NF-SAFETY-001） |
| P1.10 | 建立 IResultReader/IResultWriter 的窄接口与内存测试替身，在 composition root 注入；验证 CLI 和求解器均不直接依赖 matio C 指针 （需求：REQ-DC-IO-001） |

### P2 — 通用数学、递归采样与数据类型

| ID | 子任务 / 验收 |
|---|---|
| P2.1 | 统一 setofpoints N×6 与单位视线、切向流、角速度、平移、角度单位的数据合同 （需求：REQ-F-GEOM-001、REQ-DC-DOMAIN-001） |
| P2.2 | 对照 skalarprodukt、kreuzprodukt、erzeugekreuzproduktmatrix 实现向量基础运算与单元测试 （需求：REQ-F-GEOM-001） |
| P2.3 | 按 findeDreiecke 复现递归四分与叶节点顺序、单位化、边界层级；验证 N 随级别增长 （需求：REQ-F-GEOM-002） |
| P2.4 | 按 erzeugerichtungsvektoren 生成完整八面体 8×4^i 点，不改变排列顺序 （需求：REQ-F-GEOM-002） |
| P2.5 | 按 erzeugerichtungsvektoren_loch 生成缺损八面体 6×4^i 点，保留被省略的具体面 （需求：REQ-F-GEOM-002） |
| P2.6 | 按 richtungsvektoren_icosahedron 实现二十面体采样 20×4^i，保留原始顶点/面表 （需求：REQ-F-GEOM-003） |
| P2.7 | 按 getvektor 实现球面随机单位方向，建立显式 RNG 接口并记录固定输入黄金样本 （需求：REQ-DC-RNG-001） |
| P2.8 | 定义 FlowField、MotionEstimate、SolverResult、HarmonicCoefficients 的值语义和不变量；编写维度、复制/移动、生命周期与 MATLAB 列主序合同测试 （需求：REQ-F-GEOM-001、REQ-D-MAT-002） |
| P2.9 | 抽象 IDirectionSampler 并实现三种具体采样器，固定同一层级输出点序；仅在实验组装层选择具体类 （需求：REQ-F-GEOM-003、REQ-DC-NOISE-001） |

### P3 — 球面光流、环境与噪声

| ID | 子任务 / 验收 |
|---|---|
| P3.1 | erzeugeflussausnear：给定视线、近度、t、ω 计算平移/旋转/合成切向流和两种平均模长 （需求：REQ-F-FLOW-001） |
| P3.2 | 固定随机深度生成：按原脚本距离区间 [1,4]、平均值 2.5 归一，并取倒数；保存随机调用顺序 （需求：REQ-F-FLOW-002） |
| P3.3 | fixed_kugel...：非球形视场 + 基于全场平均光流模长的高斯噪声，最后投影回切平面 （需求：REQ-F-FLOW-003） |
| P3.4 | fixed_propfehler...：完整球面 + 基于各光流分量的高斯扰动，最后投影回切平面 （需求：REQ-F-FLOW-003） |
| P3.5 | nonpropfehler：独立实现轨迹实验所用的平均流长尺度高斯噪声；与 P3.3 区分数据来源但可共享底层公式 （需求：REQ-F-FLOW-003） |
| P3.6 | getnear_in_kugel_Drehung：射线旋转、单位球内求交和近度公式，独立测试边界/非法输入 （需求：REQ-F-FLOW-004） |
| P3.7 | 固定 RNG 输入和人工已知光流的确定性参考检查；随机统计再另做验证，不用不同语言的同种子直接逐元素比较 （需求：REQ-DC-RNG-001、REQ-NF-RNG-001） |
| P3.8 | 设计 IRandomSource（均匀/正态）和 FixedSequenceRandomSource、Mt19937RandomSource；确保每 RunId 独占并可验证调用顺序 （需求：REQ-DC-RNG-001） |
| P3.9 | 设计 IFlowNoiseModel 的平均流长、逐分量、无噪声实现，注入模拟器；保持 MATLAB 三处噪声入口的不同参数语义 （需求：REQ-F-FLOW-003、REQ-DC-NOISE-001） |

### P4 — 基线匹配滤波与 KvD 两条路径

| ID | 子任务 / 验收 |
|---|---|
| P4.1 | zellendefinieren：构造三个笛卡尔轴的模板与计数/响应存储 （需求：REQ-F-FILTER-001） |
| P4.2 | franzeigen：逐点累加平移与旋转模板响应、归一化，输出两种 3D 运动估计 （需求：REQ-F-FILTER-001） |
| P4.3 | 抽离 KvD 共享步骤：基线初始化、近度重估、平均近度符号、旋转更新、残差与迭代判定 （需求：REQ-F-KVD-001、REQ-NF-NUM-001） |
| P4.4 | norm_koenderink_iteration：保留含额外 near 因子的**原始平移更新**、更新顺序和尺度恢复 （需求：REQ-F-KVD-001） |
| P4.5 | mue_norm_koenderink_iteration：保留移除额外 near 因子的**修正平移更新**，不依靠一个容易误配的布尔选项隐藏差异 （需求：REQ-F-KVD-001） |
| P4.6 | 精确复现两者至少 6 次迭代、最多 200 次、动态阈值和绝对阈值组合的退出逻辑；输出迭代次数/残差 （需求：REQ-F-KVD-002） |
| P4.7 | transfehler_div 与 normrelfehler 分别实现与测试；特别验证零范数或 acos 越界的定义和状态 （需求：REQ-F-ERROR-001、REQ-NF-NUM-002） |
| P4.8 | 用固定 N×6 流场比较 Franz、两种 KvD 的逐轮中间结果；数值等价检查先于批量实验 （需求：REQ-F-KVD-002、REQ-NF-NUM-001） |
| P4.9 | 实现 KvDIterationEngine + IKvDTranslationRule 的组合以及 OriginalKvDEstimator、ModifiedKvDEstimator 两个明确入口；测试算法差异、状态不串扰和停止条件 （需求：REQ-F-KVD-001、REQ-DC-KVD-001） |
| P4.10 | 将 FranzMatchedFilter 做成可独立实例化、无跨样本状态的求解服务，检查其模板存储私有性及 Run 之间的行为一致性 （需求：REQ-F-FILTER-001） |

### P5 — Bias 实验及 40 目录结果分析

| ID | 子任务 / 验收 |
|---|---|
| P5.1 | bias_test_nonspherical：按实参 corenumber、iterationnumber、savestring 运行，保持 3 个误差系数及 6 组输出 （需求：REQ-F-BIAS-001、REQ-F-CLI-002） |
| P5.2 | bias_test_spherical_prop_error：与 P5.1 共用 runner 接口，保持完整球面和逐分量噪声差异 （需求：REQ-F-BIAS-002、REQ-F-CLI-002） |
| P5.3 | 实现独立 corenumber/seed 的运行管理、实际重复数和结果目录，不把 40 个已存结果目录误作单次重复 （需求：REQ-F-BIAS-001、REQ-NF-RNG-001、REQ-NF-LOG-001、REQ-NF-RECOVERY-001、REQ-F-CLI-002） |
| P5.4 | 为每次运行写 xwerte.mat 和 ywerte1…6.mat；保持 MATLAB 原有矩阵形状、列含义及输出变量名 （需求：REQ-F-BIAS-001、REQ-D-MAT-002、REQ-I-RESULT-001） |
| P5.5 | graphs_bias_test 数值层：从 1…40 读取指定 xwerte、ywerte3…6，计算平均、理论 100/sqrt(N) 参考曲线，保留起始索引/范围 （需求：REQ-F-BIAS-003） |
| P5.6 | 用 occluded_regions 和 proportional_error 的 560 个归档 MAT 对照输出，保留每个独立运行数据与汇总误差，不只看图 （需求：REQ-F-BIAS-003） |
| P5.7 | 把两种 Bias 模式组合为注入采样器、噪声模型、RNG、求解器和 IResultWriter 的 BiasExperimentRunner；测试两模式配置相互隔离和 MAT 结果输出路径权限 （需求：REQ-F-BIAS-001、REQ-DC-EXPERIMENT-001） |

### P6 — 刚体旋转、运动轨迹与误差

| ID | 子任务 / 验收 |
|---|---|
| P6.1 | drehvektor_aus_zwei_vektoren：方向间旋转轴角，处理近零与反向向量情形并记录原始 MATLAB 行为 （需求：REQ-F-TRAJ-001） |
| P6.2 | rodrigues：优先忠实实现实验实际使用的旋转向量→3×3 旋转矩阵分支；未调用的逆向/Jacobian 功能另外登记，不宣称全函数已完整覆盖 （需求：REQ-F-TRAJ-001） |
| P6.3 | kreis_trajektorie：圆轨迹点序列、半径、高度，避免生成函数含有绘图副作用 （需求：REQ-F-TRAJ-001） |
| P6.4 | sinuskurve_trajektorie：正弦轨迹点序列、周期、幅值、长度、高度，保持离散分母和端点语义 （需求：REQ-F-TRAJ-001） |
| P6.5 | trajektorie_aus_punkten：相邻位置推导平移、绝对/相对转动并转到原约定坐标系；保留两端未填写元素 （需求：REQ-F-TRAJ-001） |
| P6.6 | 轨迹、矩阵和位置求交的已知输入黄金检查；确保点始终在单位球内（或明确报错） （需求：REQ-F-TRAJ-001） |
| P6.7 | 通过 TrajectoryGenerator、TrajectoryKinematics、SphereEnvironment 细分职责；验证创建不同轨迹不修改现有 Session 状态，轨迹函数不得直接绘图 （需求：REQ-F-TRAJ-001） |

### P7 — 实球谐与 Adaptive MFA 单步求解

| ID | 子任务 / 验收 |
|---|---|
| P7.1 | winkelausvektor：atan2 方位角与 acos 极角，保持轴约定 （需求：REQ-F-SH-001） |
| P7.2 | kugelflfunc2：按 MATLAB legendre 的约定计算 l≤2 的实球谐，核对负 m 的 sin 与正 m 的 cos （需求：REQ-F-SH-001） |
| P7.3 | near_aus_koeff：以 3×5 系数矩阵恢复离散近度；严格限定有效位置 （需求：REQ-F-SH-001） |
| P7.4 | koeff_aus_near：逐项投影、4π 归一及范数除法，明确其与一般联合最小二乘的差异 （需求：REQ-F-SH-001） |
| P7.5 | SHRotate：仅移植应用所需的 l=0/1/2 系数旋转，并做第三方来源/实现权核查与基函数顺序验证 （需求：REQ-F-SH-002） |
| P7.6 | transfor_koeff：结合 Rodrigues 旋转矩阵和 SHRotate 的 3×3、5×5 块；检验正向/逆向旋转 （需求：REQ-F-SH-002） |
| P7.7 | oneit_adaptiv_matched_filter 的右端向量、四个 3×3 子块与 6×6 联立系统：分别建立可独立对照的数值阶段 （需求：REQ-F-ADAPT-001） |
| P7.8 | 单轮运动向量解析、平移归一化、残差、近度再计算、二阶球谐系数重估；明确状态输入/输出，不混成多帧类 （需求：REQ-F-ADAPT-001） |
| P7.9 | 对照原始 inv/real 的行为和 Eigen 解算差异，记录奇异、零范数及非有限输出，不未经验证更改默认求解结果 （需求：REQ-F-ADAPT-001、REQ-NF-NUM-001、REQ-NF-NUM-002） |
| P7.10 | 将球谐基函数、拟合和旋转分为 RealSphericalHarmonics、HarmonicFitter、HarmonicRotator；类只管理固定约定，不存多帧 mutable 状态 （需求：REQ-F-SH-001） |
| P7.11 | 实现无状态 AdaptiveMfaStepSolver 的明确输入/输出合同，使其可单独用固定 FlowField + 旧 HarmonicCoefficients 测试，且不继承 KvD 求解器 （需求：REQ-F-ADAPT-001、REQ-DC-ADAPT-001） |

### P8 — 三种 Adaptive MFA 实验驱动

| ID | 子任务 / 验收 |
|---|---|
| P8.1 | agent_in_sphere_initialization_test：35 点圆轨迹，对比每帧零阶重置与继承系数，按脚本输出四个误差 MAT （需求：REQ-F-EXP-001、REQ-F-CLI-003） |
| P8.2 | agent_in_sphere_adaptive_matched_filter_test：600 点正弦轨迹、级别 4、8 次重复，保留 addederror 参数和重置/继承两套曲线 （需求：REQ-F-EXP-002、REQ-F-CLI-003） |
| P8.3 | P8.2 中的前一帧 Rodrigues 旋转与系数更新顺序单独实现并测试，不与当前帧求解顺序混淆 （需求：REQ-F-ADAPT-002） |
| P8.4 | P8.2 独立 MAT 输出四种误差及 rottransratio，分别对照 with_errors 和 without_errors （需求：REQ-F-EXP-002、REQ-I-RESULT-001） |
| P8.5 | agent_in_sphere_adaptive_matched_filter_many_parameter：级别 2、更新间隔 **1/5/11/21**、真实更新 akkoeff 与多组冻结/周期刷新 mitkoeff （需求：REQ-F-ADAPT-003、REQ-F-EXP-003、REQ-F-CLI-003） |
| P8.6 | 比较“每步自身求解”和“独立 periodic baseline”的区别，并保存相应 MAT；不得将二者混成一组状态 （需求：REQ-F-ADAPT-003、REQ-F-EXP-003） |
| P8.7 | 对照历史结果时显式列出脚本没有当前写入、但归档已有的 alleadap*、rottransratio 等文件；先检查来源再决定是否需要额外兼容导出，不能伪造数据 （需求：REQ-F-EXP-003、REQ-D-MAT-003、REQ-I-RESULT-001） |
| P8.8 | 引入 AdaptiveMfaSession（唯一主系数所有者）与 ICoefficientUpdatePolicy（重置、继承、周期刷新），以测试替身检验旋转→基线更新→求解→主状态提交的逐帧时序 （需求：REQ-F-ADAPT-002、REQ-DC-ADAPT-001） |
| P8.9 | 为 InitializationRunner、AdaptiveComparisonRunner、UpdateRateRunner 分别提供 RunContext 及独立会话；测试多组更新频率和多次重复运行间不共享可变状态 （需求：REQ-F-ADAPT-002、REQ-DC-ADAPT-001、REQ-NF-LOG-001、REQ-NF-SESSION-001） |

### P9 — 结果分析、绘图与论文插图（与求解器隔离）

| ID | 子任务 / 验收 |
|---|---|
| P9.1 | graph_agent_in_sphere_initialization_test 的数据抽取、对数坐标和不同曲线取样区间 （需求：REQ-F-AN-001、REQ-F-CLI-004、REQ-I-ANALYSIS-001） |
| P9.2 | graph_agent_in_sphere_adaptive_matched_filter_test 的旋转/平移误差、rottransratio 双纵轴以及分段规则，阈值 0.01 （需求：REQ-F-AN-001、REQ-I-ANALYSIS-001） |
| P9.3 | graph_agent_in_sphere_adaptive_matched_filter_many_parameter 的四组曲线、平滑窗口 2/5/10、阈值 0.02、分段区间 （需求：REQ-F-AN-001、REQ-I-ANALYSIS-001） |
| P9.4 | graphs_bias_test 的 40-run 平均和 log-log 曲线、参考 100/sqrt(N)，输入/输出与计算分层 （需求：REQ-F-BIAS-003、REQ-F-CLI-004、REQ-I-ANALYSIS-001） |
| P9.5 | kurveglaetten 的边缘零值和窗口处理按原始行为验证；不自行修改为不同边界策略 （需求：REQ-F-AN-001） |
| P9.6 | 评估可选 C++/gnuplot 等 Debian/Ubuntu 绘图后端，默认不影响无图形运行。导出数值与图像，保证对照曲线的索引、纵横坐标、单位一致 （需求：REQ-F-AN-002、REQ-NF-HEADLESS-001、REQ-F-CLI-005） |
| P9.7 | circle_curve_in_the_sphere、sinosodial_curve_in_the_sphere、spherical_harmonic_illustration 三个展示脚本独立实现/记录视觉等价性；不依赖 MATLAB/Python （需求：REQ-F-AN-002、REQ-F-CLI-005） |
| P9.8 | 用 BiasResultAnalyzer / AdaptiveResultAnalyzer 分离 MAT 读取与数值汇总；可选 IPlotExporter 仅消费分析结果，并用内存数据测试其不影响求解结果 （需求：REQ-F-AN-002、REQ-DC-ANALYSIS-001） |

### P10 — 端到端核验、文档与交付

| ID | 子任务 / 验收 |
|---|---|
| P10.1 | 更新 README：删除旧 NumPy/.npz 迁移承诺，声明原始 MAT 直接访问和实际可运行入口，保留两份 DOI （需求：REQ-PROC-TRACE-001、REQ-NF-COPYRIGHT-001） |
| P10.2 | 形成每个 MATLAB 函数至 C++ 模块的最终追踪表（45/45 有处理决定：已移植、仅展示、无调用但保留、受授权阻塞） （需求：REQ-PROC-TRACE-001） |
| P10.3 | 逐层运行 MAT I/O、数学、采样、基线、KvD、球谐、自适应、实验和画图测试；记录命令、版本、退出码、日志 （需求：REQ-NF-NUM-001、REQ-NF-NUM-002、REQ-PROC-TRACE-001、REQ-NF-RECOVERY-001） |
| P10.4 | 分别以原始归档的 560 个 bias MAT 和 22 个 adaptive MAT 核对结果契约；核对历史产物与当前源码的差异，不强求随机逐样本相等 （需求：REQ-D-MAT-001、REQ-D-MAT-003） |
| P10.5 | 复现论文对应偏差、初始化、轨迹及更新频率实验的数值图表；每组注明实际样本/噪声/状态 （需求：REQ-NF-NUM-001、REQ-NF-RNG-001） |
| P10.6 | 汇总 Debian/Ubuntu 的已执行构建、CTest 和实验结果，生成 NUMERICAL_EQUIVALENCE 与 PROGRESS；保留失败样本和未验证平台 （需求：REQ-NF-PLATFORM-001、REQ-NF-DEPS-001、REQ-NF-LOG-001、REQ-NF-HEADLESS-001、REQ-NF-PORTABLE-001、REQ-NF-RUNTIME-001） |
| P10.7 | 核查第三方版权及 README 声明，不自动添加 LICENSE、不提交未经确认的新增数据副本 （需求：REQ-D-SOURCE-002、REQ-NF-COPYRIGHT-001） |
| P10.8 | 创建或更新对应 Draft PR 供审查；不得自动合并 main，按 AGENTS.md 报告 SHA、状态及未办事项 （需求：REQ-PROC-TRACE-001） |
| P10.9 | 验收 OOP 边界：接口可替换、对象所有权明确、无悬垂 MAT 内存、无跨实验状态泄漏、类方法到全部 MATLAB 函数追踪完整；记录类关系和测试证据 （需求：REQ-PROC-TRACE-001、REQ-NF-OOP-001、REQ-NF-SESSION-001） |
| P10.10 | 检查无过度继承/巨型类/虚假统一求解器；保证纯数学函数不为 OOP 改变求解顺序、精度与实验结果合同 （需求：REQ-PROC-TRACE-001、REQ-NF-OOP-001） |

## 6. 依赖图及并行边界

~~~mermaid
flowchart TD
  P0[P0 源码、对象模型与合同审计] --> P1[P1 RAII MAT I/O、CLI 与装配]
  P0 --> P2[P2 数学与采样]
  P1 --> P5[P5 Bias 实验]
  P2 --> P3[P3 FlowSynthesizer 与噪声策略]
  P2 --> P4[P4 Franz 与 KvD 策略引擎]
  P3 --> P5
  P4 --> P5
  P2 --> P6[P6 轨迹与旋转]
  P2 --> P7[P7 球谐与无状态 Adaptive 单步]
  P6 --> P7
  P3 --> P8[P8 Adaptive 实验]
  P6 --> P8
  P7 --> P8
  P1 --> P8
  P5 --> P9[P9 数据分析与绘图]
  P8 --> P9
  P1 --> P9
  P9 --> P10[P10 全量核验与交付]
~~~

- P0 对象模型和窄接口合约先冻结；P1 MAT I/O、P2 纯数学、P6 轨迹和 P7 球谐基函数准备可在满足依赖后分派到不同**独立工作区/任务分支**，但同一文件与工作区不能由多个 Agent 并发写。P7 的球谐旋转功能依赖 P6 的旋转数学接口；**Adaptive 单步求解不依赖 KvD 的 C++ 实现**，不要因论文中的理论关系而人为耦合 P4 与 P7。
- 每个子任务应是小范围、可验证、独立提交；不得因“同时开发更快”而先把未验证的共享 API 任意修改。变更共同类型或坐标合同先由集成负责人核对。
- 每次编译、运行或实验前，先按 AGENTS.md 完成允许的代码提交/推送远程检查点。不得等待 GitHub Actions。
- **本轮仅修订 PLAN.md；P0～P10 的任何 C++ 文件、构建、CTest、随机仿真和分析程序均尚未实施。**

## 7. 数值等价与科学复现的验收标准

> 本节是**验证实施规划**。正式需求义务、候选与已批准容差、验收编号和测试编号以 [docs/REQUIREMENTS.md](docs/REQUIREMENTS.md) 为需求基线；不得因本节存在文字验收要求就标记相关需求已通过。

1. **结构与对象边界一致**：MAT 变量名、class/type、维度、列主序、数值和压缩格式被正确解释；原始只读/运行输出写入由不同对象管理；MatFileHandle move-only、句柄释放后领域值对象不悬垂。单变量一个文件的旧兼容规则与多变量严格访问规则都有测试。
2. **几何一致**：三种视线采样都满足原始数量公式、单位范数、递归顺序；光流切向满足 d·p≈0，翻转方向、符号、坐标系由固定黄金样本检验。
3. **算法一致**：Original KvD 与 Modified KvD 分别保留不同方程；Franz 3 轴模板、Adaptive 6×6 线性系统以及球谐拟合/旋转都可对照逐步结果。
4. **异常真实**：MAT 损坏、形状不匹配、零模向量、病态求解、非有限数、未定义方向误差必须明确报告，原版异常不应被无声“修复”成通过。
5. **数据与随机分离**：固定输入对照可先使用相对误差 1e−10 作为候选基线（根据计算顺序和条件数调整）；随机实验先比较脚本规定的统计结果，不要求不同 RNG 的同种子样本逐位一致。
6. **实验一致**：bias 的 40 个原始子目录与 Adaptive 的四类数据集彼此独立统计；无噪/含噪、初始化、更新间隔均与对应源码参数一致。
7. **图表一致**：曲线使用同一结果矩阵、定义与索引；图片格式和外观可不同，但曲线数值、相对比较、纵横轴单位不可改变。
8. **状态透明**：生成代码、提交远端、构建、CTest、原始数据回归、论文复现、PR 交付均逐项报告，没有执行的内容不能写成通过。
9. **面向对象合格**：每个持有可变状态的类有唯一所有者；业务运行器不包含数学公式/直接 matio 调用；算法可在固定输入和注入假对象下独立测试；OOP 重构不改变原始数值结果。

## 8. 当前风险、阻塞与决策

| 风险或发现 | 处理决定 |
|---|---|
| 源码由作者在 Linux 环境开发，存在历史 MATLAB API | Debian/Ubuntu 独立 C++17 实现，不加入 MATLAB Runtime；忠实记录 API 语义 |
| 历史 MAT 文件是压缩 MAT v5（目前抽样确认） | matio + zlib 为 P1 实际要求；全量版本/变量校验仍待 P0.5 |
| 原 MATLAB 函数调用有隐式全局随机状态 | RNG 通过实验 runner 显式持有；固定数据与统计层分别对照 |
| 原始脚本的输出集合与归档文件名存在差异 | 建立“实际生成/已有归档/分析依赖”三方清单，未经核验不得补造或删除 |
| 球谐/旋转存在符号、坐标、基函数顺序差异风险 | 分离基础函数、设置固定旋转样本、逐项系数对照，不直接套第三方球谐默认值 |
| rodrigues.m、SHRotate.m 含第三方来源 | 独立版权和技术来源核查，未经确认不直接复制第三方代码段 |
| 既有 README 与最新 MAT-only 决策不一致 | 计划 P10.1 更新；本轮只改 PLAN，不擅改其他文件 |
| 旧计划只设两套实验执行器 | 调整为五类生成入口与四种分析入口，并单独处理三份插图 |
| 原计划频率写成 1/5/10/20 | 明确修正为**1/5/11/21**，完全以当前 MATLAB 文件为准 |
| 原生数值不稳定 | 先记录/复现原始行为；增强方案必须有独立需求和对照证明，不得在忠实模式下暗改 |
| 模块化不等于面向对象 | 增加显式类职责、对象状态/所有权、依赖注入、组合关系、OOP 验收和分阶段任务；禁止为每个函数创建虚类 |
| Adaptive 单步与多帧会话混合 | 无状态 StepSolver + Session + 独立的更新策略；主系数状态仅由 Session 拥有 |
| MAT 指针泄漏到数学层或覆盖原始文件 | move-only RAII + 值对象 + 只读/写入分离；原始 MAT 路径不可作为输出路径 |

## 9. 来源与版本声明

- 原始论文：**Insect-Inspired Self-Motion Estimation with Dense Flow Fields—An Adaptive Matched Filter Approach**，Strübbe / Stürzl / Egelhaaf（2015），https://doi.org/10.1371/journal.pone.0128413
- MATLAB 原始资料：**Egomotion Estimation with Dense Flow Fields: MATLAB Scripts**，https://doi.org/10.4119/unibi/2736885
- 本仓库原始文件目录：origin/egomotion_estimation_dense_flow_fields_matlab_scripts/
- 本项目规则：[AGENTS.md](AGENTS.md)
- **审计深度**：此前已经逐个读取全部 45 个 MATLAB 文件和 readme；通过 GitHub 仓库树核对 582 个 MAT 路径，抽样检查多个 MAT 文件头；本次复核现有计划及 AGENTS，补齐 OOP 设计；尚未通过 matio 解包校验全部 MAT 变量，也没有编译、运行 MATLAB/C++ 或重做实验。
- 本计划只代表后续可执行工作拆分，不视为任何阶段已交付。后续开发须对照实际源码进一步填充函数/数据对照文档，并在产生新事实时以独立提交更新计划。

**最终架构决策：C++17 + Eigen 3 + matio/zlib + CMake；以值对象、RAII、窄接口、策略组合、无状态求解器和有状态实验会话实现面向对象架构；无状态数学公式保留函数形式；保留原始 MAT 文件与完整原始资料目录；按模块移植 45 个 MATLAB 文件所涵盖的数值和分析流程；每个子任务独立验证后集成；目标仅 Debian/Ubuntu。**
