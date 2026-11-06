# Egomotion Estimation with Dense Flow Fields：MATLAB → C++ 源码审计与模块化移植计划

> **本文件为实际 MATLAB 源码审计后的修订计划，不代表任何 C++ 功能已经实现。**
>
> 审计依据：本仓库 main 分支的 origin/egomotion_estimation_dense_flow_fields_matlab_scripts/、AGENTS.md、原始资料自带 readme 和 2015 年论文。原始文件的目录和函数名均已在 GitHub 上核对；MAT 数据的文件头已抽样读取，但全部变量的二进制内容尚未逐个解压、验证。
>
> 本轮工作只修订 PLAN.md，不创建、运行或编译 C++ 代码，不修改 origin、README、AGENTS 或数据文件。

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

## 3. 建议的模块边界与数据契约

### 3.1 必须分离的模块

- **mat_io**：原始 MAT 只读打开、压缩变量、类型/维度检查、变量名、MAT 结果独立写出；不依赖任何求解器或绘图。
- **core_types**：方向/光流集合 N×6 的语义访问、参数、随机种子、状态、误差和失败码；不直接持有 MATLAB 文件句柄。
- **math3d**：三维向量、叉积矩阵、Rodrigues 的实验实际使用子集；无实验状态。
- **sampling**：递归三角形、八面体、缺损八面体、二十面体；输出顺序固定。
- **flow_simulation**：正向光流、随机近度、球体内近度、两种明确的噪声模型；通过显式 RNG 注入。
- **estimators/standard_filter**：zellendefinieren + franzeigen；可独立测试。
- **estimators/kvd**：Original/Modified 共用公共运算但保留各自不同的平移更新和停止逻辑。
- **harmonics**：向量→球角、关联勒让德、实球谐、近度重构、逐项系数拟合、球谐旋转；无实验读取。
- **estimators/adaptive**：oneit_adaptiv_matched_filter 的单次迭代；输入旧系数和流场，输出新系数、t、ω、误差与有效性状态。
- **trajectory**：圆轨迹、正弦轨迹、轨迹差分与相机朝向、球内求交；无 MAT I/O。
- **experiments**：只负责原始参数组合、运行调度、状态更新、输出组织；不得重复实现数学公式。
- **analysis**：加载历史/新 MAT 结果、跨 40 运行汇总、数值曲线数据及指标；与图像绘制分离。
- **plotting**：2D、3D 图表导出。后期可评估 Debian/Ubuntu 可用的可选 gnuplot 后端，不将其变成数值核心的必需依赖，也不加入 Python。

### 3.2 接口合同先于实现

- 统一规范 setofpoints 行结构为 direction (3) + tangent_flow (3)，明确 **N×6、列主序、右手系、坐标系、角速度符号**；独立的 Eigen 数据访问层对外可提供结构化读取，不必把 N×6 传染到所有模块。
- ExperimentConfig 以各原始入口的实参和实际常量为起点；**每个实验有独立默认值**，共享的只是明确有物理意义的可复用字段。若使用配置模板，遵守 AGENTS.md 对 configs/README.md 的要求。
- RNG 由实验运行器显式创建并传入模拟器。原程序使用 RandStream mt19937ar 和 rand/randn；现代 C++ 同 seed 不保证 randn 样本逐项一致。因此定义：固定输入数值对照与随机分布/统计复现分别验收；不得虚报同种子逐位相等。
- 核心数值 API 返回明确结果/状态，不把 NaN、Inf、零范数或奇异矩阵结果偷偷替换成 0 或伪造成功。原版失败行为需与后续安全防护区分记录。
- 实验输出采用新结果根目录和 RunId；原始 origin/** 永远只读。当前 MATLAB 的 save 输出 **一个变量一个 MAT 文件**，首版兼容输出应优先保留变量名、矩阵形状和路径约定；已存历史文件不允许被重新生成结果覆盖。
- 读取或写入 MAT 的图表程序不得改变任何求解结果；绘图不可用时数值实验和 MAT 回归仍能独立完成。

## 4. 新项目目录结构（依据源码重新设计）

下列为**计划**，目前尚未创建。避免再复制一套 data/original/：已有原始文件以 origin/egomotion_estimation_dense_flow_fields_matlab_scripts/ 为唯一来源。生成结果应位于受保护的独立运行目录。

~~~text
Egomotion-Estimation-with-Dense-Flow-Fields/
├── AGENTS.md
├── PLAN.md
├── README.md
├── .gitignore
├── CMakeLists.txt
├── CMakePresets.json
├── origin/egomotion_estimation_dense_flow_fields_matlab_scripts/  # 永远保留
├── include/egomotion/
│   ├── core/types.hpp
│   ├── core/parameters.hpp
│   ├── io/mat_reader.hpp
│   ├── io/mat_writer.hpp
│   ├── math/vector_ops.hpp
│   ├── math/rotations.hpp
│   ├── geometry/sampling.hpp
│   ├── geometry/flow_field.hpp
│   ├── simulation/noise.hpp
│   ├── simulation/sphere_scene.hpp
│   ├── simulation/trajectory.hpp
│   ├── estimators/franz_filter.hpp
│   ├── estimators/kvd.hpp
│   ├── harmonics/real_spherical_harmonics.hpp
│   ├── harmonics/coefficients.hpp
│   ├── harmonics/rotation.hpp
│   ├── estimators/adaptive_mfa.hpp
│   ├── experiments/bias_test.hpp
│   ├── experiments/sphere_tests.hpp
│   └── analysis/metrics.hpp
├── src/                          # 与 include 的模块目录相对应
│   ├── core/
│   ├── io/
│   ├── math/
│   ├── geometry/
│   ├── simulation/
│   ├── estimators/
│   ├── harmonics/
│   ├── experiments/
│   ├── analysis/
│   ├── plotting/                 # 可选、晚期启用
│   └── cli/main.cpp
├── tests/
│   ├── unit/                     # 各函数独立、无数据生成测试
│   ├── golden/                   # 原始 MAT 只读对照
│   ├── integration/              # 入口与 MAT 结果闭环
│   └── reproduction/             # 原论文统计与图表数据
├── docs/
│   ├── SOURCE_INVENTORY.md
│   ├── MAT_DATA_SCHEMA.md
│   ├── MIGRATION_MATRIX.md
│   ├── NUMERICAL_EQUIVALENCE.md
│   └── PROGRESS.md
└── results/                      # 本地生成、不得覆盖 origin
~~~

建议一个主 CLI（例如 egomotion）提供 **inspect-mat、run-bias-occluded、run-bias-proportional、run-adaptive-init、run-adaptive-comparison、run-adaptive-update-rates、analyze、plot** 等子命令；实验数值计算与分析/绘图可以分别执行。子命令是规划中的接口名，不代表已实现。结果路径必须可配置，不能依赖执行时的当前工作目录。

## 5. 细粒度实施任务（每项均可单独检查并提交）

任务 ID 为相互独立的**最小交付单元**；建议按依赖顺序串行集成，同一阶段内部没有依赖的项可由隔离的 Agent 并行完成。每项至少给出：对应原始 .m、输入/输出契约、修改文件、单元或静态检查、是否满足验收；不准用“整个阶段已完成”代替单项证据。

### P0 — 基线冻结与来源审计（纯文档）

| ID | 子任务 / 验收 |
|---|---|
| P0.1 | 固定本次源代码 commit SHA；清点 45 个 .m、1 个 readme、582 个 .mat，记录 SHA/大小，核验分类，不修改原归档 |
| P0.2 | 对 33 个辅助函数及 12 个入口/绘图文件逐一形成函数签名、被谁调用、是否实际用于实验、输入维度、输出变量清单 |
| P0.3 | 建立两套 bias 与四套 adaptive 归档子目录的结果文件清单；对照 MATLAB 的 save、load 和 getflowfield 调用 |
| P0.4 | 核查 rodrigues.m、SHRotate.m 的第三方来源和可移植范围；记录未能确认的版权问题，不擅自添加 LICENSE |
| P0.5 | 对压缩 MAT 完成全目录文件头、版本、压缩、变量名、类型、形状的只读审计（待 matio 检查工具可用时验证）；缺失项标记未核验 |
| P0.6 | 将上述信息形成 SOURCE_INVENTORY、MAT_DATA_SCHEMA、MIGRATION_MATRIX，并记录已有旧计划的错误参数及历史结果/当前脚本不一致项 |

P0 当前仅完成一部分只读人工审计；以上文档尚未创建。源文件是否存在已确认，不等于 P0.5 的二进制变量检查已完成。

### P1 — 独立构建、CLI 与 MAT 文件访问

| ID | 子任务 / 验收 |
|---|---|
| P1.1 | 建立仅针对 Debian/Ubuntu 的 CMake C++17 工程，声明 Eigen 3、matio、zlib 实际依赖；不编译 origin 下 MATLAB 文件 |
| P1.2 | 建立 CLI 参数解析、退出码与独立输出路径；允许只执行 inspect 而不加载算法 |
| P1.3 | 封装 matio 只读文件句柄和变量句柄的 RAII；错误路径无资源泄漏且不会写源文件 |
| P1.4 | 通过 MAT v5+zlib 读取数值变量的真实 rank、dims、类型、字节数及列主序内容；不静默转置或单精度升级 |
| P1.5 | 递进实现真实数据需要的 struct/cell、复数、逻辑或稀疏类型；未要求的类型明确拒绝，禁止伪称全部支持 |
| P1.6 | 实现明确变量名加载和 getflowfield 兼容读取（首变量/自动 .mat 后缀）；分别测试，不能混淆 |
| P1.7 | 实现**仅向新结果目录**写入 MATLAB 兼容 MAT 的结果文件，写后读取检查变量名、维度与数值 |
| P1.8 | 以原始归档的代表样本为 golden 只读验证，记录源文件哈希、输出形状、误差及实际命令 |

### P2 — 通用数学、递归采样与数据类型

| ID | 子任务 / 验收 |
|---|---|
| P2.1 | 统一 setofpoints N×6 与单位视线、切向流、角速度、平移、角度单位的数据合同 |
| P2.2 | 对照 skalarprodukt、kreuzprodukt、erzeugekreuzproduktmatrix 实现向量基础运算与单元测试 |
| P2.3 | 按 findeDreiecke 复现递归四分与叶节点顺序、单位化、边界层级；验证 N 随级别增长 |
| P2.4 | 按 erzeugerichtungsvektoren 生成完整八面体 8×4^i 点，不改变排列顺序 |
| P2.5 | 按 erzeugerichtungsvektoren_loch 生成缺损八面体 6×4^i 点，保留被省略的具体面 |
| P2.6 | 按 richtungsvektoren_icosahedron 实现二十面体采样 20×4^i，保留原始顶点/面表 |
| P2.7 | 按 getvektor 实现球面随机单位方向，建立显式 RNG 接口并记录固定输入黄金样本 |

### P3 — 球面光流、环境与噪声

| ID | 子任务 / 验收 |
|---|---|
| P3.1 | erzeugeflussausnear：给定视线、近度、t、ω 计算平移/旋转/合成切向流和两种平均模长 |
| P3.2 | 固定随机深度生成：按原脚本距离区间 [1,4]、平均值 2.5 归一，并取倒数；保存随机调用顺序 |
| P3.3 | fixed_kugel...：非球形视场 + 基于全场平均光流模长的高斯噪声，最后投影回切平面 |
| P3.4 | fixed_propfehler...：完整球面 + 基于各光流分量的高斯扰动，最后投影回切平面 |
| P3.5 | nonpropfehler：独立实现轨迹实验所用的平均流长尺度高斯噪声；与 P3.3 区分数据来源但可共享底层公式 |
| P3.6 | getnear_in_kugel_Drehung：射线旋转、单位球内求交和近度公式，独立测试边界/非法输入 |
| P3.7 | 固定 RNG 输入和人工已知光流的确定性参考检查；随机统计再另做验证，不用不同语言的同种子直接逐元素比较 |

### P4 — 基线匹配滤波与 KvD 两条路径

| ID | 子任务 / 验收 |
|---|---|
| P4.1 | zellendefinieren：构造三个笛卡尔轴的模板与计数/响应存储 |
| P4.2 | franzeigen：逐点累加平移与旋转模板响应、归一化，输出两种 3D 运动估计 |
| P4.3 | 抽离 KvD 共享步骤：基线初始化、近度重估、平均近度符号、旋转更新、残差与迭代判定 |
| P4.4 | norm_koenderink_iteration：保留含额外 near 因子的**原始平移更新**、更新顺序和尺度恢复 |
| P4.5 | mue_norm_koenderink_iteration：保留移除额外 near 因子的**修正平移更新**，不依靠一个容易误配的布尔选项隐藏差异 |
| P4.6 | 精确复现两者至少 6 次迭代、最多 200 次、动态阈值和绝对阈值组合的退出逻辑；输出迭代次数/残差 |
| P4.7 | transfehler_div 与 normrelfehler 分别实现与测试；特别验证零范数或 acos 越界的定义和状态 |
| P4.8 | 用固定 N×6 流场比较 Franz、两种 KvD 的逐轮中间结果；数值等价检查先于批量实验 |

### P5 — Bias 实验及 40 目录结果分析

| ID | 子任务 / 验收 |
|---|---|
| P5.1 | bias_test_nonspherical：按实参 corenumber、iterationnumber、savestring 运行，保持 3 个误差系数及 6 组输出 |
| P5.2 | bias_test_spherical_prop_error：与 P5.1 共用 runner 接口，保持完整球面和逐分量噪声差异 |
| P5.3 | 实现独立 corenumber/seed 的运行管理、实际重复数和结果目录，不把 40 个已存结果目录误作单次重复 |
| P5.4 | 为每次运行写 xwerte.mat 和 ywerte1…6.mat；保持 MATLAB 原有矩阵形状、列含义及输出变量名 |
| P5.5 | graphs_bias_test 数值层：从 1…40 读取指定 xwerte、ywerte3…6，计算平均、理论 100/sqrt(N) 参考曲线，保留起始索引/范围 |
| P5.6 | 用 occluded_regions 和 proportional_error 的 560 个归档 MAT 对照输出，保留每个独立运行数据与汇总误差，不只看图 |

### P6 — 刚体旋转、运动轨迹与误差

| ID | 子任务 / 验收 |
|---|---|
| P6.1 | drehvektor_aus_zwei_vektoren：方向间旋转轴角，处理近零与反向向量情形并记录原始 MATLAB 行为 |
| P6.2 | rodrigues：优先忠实实现实验实际使用的旋转向量→3×3 旋转矩阵分支；未调用的逆向/Jacobian 功能另外登记，不宣称全函数已完整覆盖 |
| P6.3 | kreis_trajektorie：圆轨迹点序列、半径、高度，避免生成函数含有绘图副作用 |
| P6.4 | sinuskurve_trajektorie：正弦轨迹点序列、周期、幅值、长度、高度，保持离散分母和端点语义 |
| P6.5 | trajektorie_aus_punkten：相邻位置推导平移、绝对/相对转动并转到原约定坐标系；保留两端未填写元素 |
| P6.6 | 轨迹、矩阵和位置求交的已知输入黄金检查；确保点始终在单位球内（或明确报错） |

### P7 — 实球谐与 Adaptive MFA 单步求解

| ID | 子任务 / 验收 |
|---|---|
| P7.1 | winkelausvektor：atan2 方位角与 acos 极角，保持轴约定 |
| P7.2 | kugelflfunc2：按 MATLAB legendre 的约定计算 l≤2 的实球谐，核对负 m 的 sin 与正 m 的 cos |
| P7.3 | near_aus_koeff：以 3×5 系数矩阵恢复离散近度；严格限定有效位置 |
| P7.4 | koeff_aus_near：逐项投影、4π 归一及范数除法，明确其与一般联合最小二乘的差异 |
| P7.5 | SHRotate：仅移植应用所需的 l=0/1/2 系数旋转，并做第三方来源/实现权核查与基函数顺序验证 |
| P7.6 | transfor_koeff：结合 Rodrigues 旋转矩阵和 SHRotate 的 3×3、5×5 块；检验正向/逆向旋转 |
| P7.7 | oneit_adaptiv_matched_filter 的右端向量、四个 3×3 子块与 6×6 联立系统：分别建立可独立对照的数值阶段 |
| P7.8 | 单轮运动向量解析、平移归一化、残差、近度再计算、二阶球谐系数重估；明确状态输入/输出，不混成多帧类 |
| P7.9 | 对照原始 inv/real 的行为和 Eigen 解算差异，记录奇异、零范数及非有限输出，不未经验证更改默认求解结果 |

### P8 — 三种 Adaptive MFA 实验驱动

| ID | 子任务 / 验收 |
|---|---|
| P8.1 | agent_in_sphere_initialization_test：35 点圆轨迹，对比每帧零阶重置与继承系数，按脚本输出四个误差 MAT |
| P8.2 | agent_in_sphere_adaptive_matched_filter_test：600 点正弦轨迹、级别 4、8 次重复，保留 addederror 参数和重置/继承两套曲线 |
| P8.3 | P8.2 中的前一帧 Rodrigues 旋转与系数更新顺序单独实现并测试，不与当前帧求解顺序混淆 |
| P8.4 | P8.2 独立 MAT 输出四种误差及 rottransratio，分别对照 with_errors 和 without_errors |
| P8.5 | agent_in_sphere_adaptive_matched_filter_many_parameter：级别 2、更新间隔 **1/5/11/21**、真实更新 akkoeff 与多组冻结/周期刷新 mitkoeff |
| P8.6 | 比较“每步自身求解”和“独立 periodic baseline”的区别，并保存相应 MAT；不得将二者混成一组状态 |
| P8.7 | 对照历史结果时显式列出脚本没有当前写入、但归档已有的 alleadap*、rottransratio 等文件；先检查来源再决定是否需要额外兼容导出，不能伪造数据 |

### P9 — 结果分析、绘图与论文插图（与求解器隔离）

| ID | 子任务 / 验收 |
|---|---|
| P9.1 | graph_agent_in_sphere_initialization_test 的数据抽取、对数坐标和不同曲线取样区间 |
| P9.2 | graph_agent_in_sphere_adaptive_matched_filter_test 的旋转/平移误差、rottransratio 双纵轴以及分段规则，阈值 0.01 |
| P9.3 | graph_agent_in_sphere_adaptive_matched_filter_many_parameter 的四组曲线、平滑窗口 2/5/10、阈值 0.02、分段区间 |
| P9.4 | graphs_bias_test 的 40-run 平均和 log-log 曲线、参考 100/sqrt(N)，输入/输出与计算分层 |
| P9.5 | kurveglaetten 的边缘零值和窗口处理按原始行为验证；不自行修改为不同边界策略 |
| P9.6 | 评估可选 C++/gnuplot 等 Debian/Ubuntu 绘图后端，默认不影响无图形运行。导出数值与图像，保证对照曲线的索引、纵横坐标、单位一致 |
| P9.7 | circle_curve_in_the_sphere、sinosodial_curve_in_the_sphere、spherical_harmonic_illustration 三个展示脚本独立实现/记录视觉等价性；不依赖 MATLAB/Python |

### P10 — 端到端核验、文档与交付

| ID | 子任务 / 验收 |
|---|---|
| P10.1 | 更新 README：删除旧 NumPy/.npz 迁移承诺，声明原始 MAT 直接访问和实际可运行入口，保留两份 DOI |
| P10.2 | 形成每个 MATLAB 函数至 C++ 模块的最终追踪表（45/45 有处理决定：已移植、仅展示、无调用但保留、受授权阻塞） |
| P10.3 | 逐层运行 MAT I/O、数学、采样、基线、KvD、球谐、自适应、实验和画图测试；记录命令、版本、退出码、日志 |
| P10.4 | 分别以原始归档的 560 个 bias MAT 和 22 个 adaptive MAT 核对结果契约；核对历史产物与当前源码的差异，不强求随机逐样本相等 |
| P10.5 | 复现论文对应偏差、初始化、轨迹及更新频率实验的数值图表；每组注明实际样本/噪声/状态 |
| P10.6 | 汇总 Debian/Ubuntu 的已执行构建、CTest 和实验结果，生成 NUMERICAL_EQUIVALENCE 与 PROGRESS；保留失败样本和未验证平台 |
| P10.7 | 核查第三方版权及 README 声明，不自动添加 LICENSE、不提交未经确认的新增数据副本 |
| P10.8 | 创建或更新对应 Draft PR 供审查；不得自动合并 main，按 AGENTS.md 报告 SHA、状态及未办事项 |

## 6. 依赖图及并行边界

~~~mermaid
flowchart TD
  P0[P0 源码与资料审计] --> P1[P1 MAT I/O 与 CLI]
  P0 --> P2[P2 数学与采样]
  P1 --> P5[P5 Bias 实验]
  P2 --> P3[P3 光流与噪声]
  P2 --> P4[P4 Franz 与 KvD]
  P3 --> P5
  P4 --> P5
  P2 --> P6[P6 轨迹与旋转]
  P2 --> P7[P7 球谐与 Adaptive 单步]
  P4 --> P7
  P3 --> P8[P8 Adaptive 实验]
  P6 --> P8
  P7 --> P8
  P1 --> P8
  P5 --> P9[P9 数据分析与绘图]
  P8 --> P9
  P1 --> P9
  P9 --> P10[P10 全量核验与交付]
~~~

- P1 的 MAT I/O、P2 纯数学、P6 轨迹和 P7 基函数准备可在满足依赖后分派到不同**独立工作区/任务分支**，但同一文件与工作区不能由多个 Agent 并发写。
- 每个子任务应是小范围、可验证、独立提交；不得因“同时开发更快”而先把未验证的共享 API 任意修改。变更共同类型或坐标合同先由集成负责人核对。
- 每次编译、运行或实验前，先按 AGENTS.md 完成允许的代码提交/推送远程检查点。不得等待 GitHub Actions。
- **本轮仅修订 PLAN.md；P0～P10 的任何 C++ 文件、构建、CTest、随机仿真和分析程序均尚未实施。**

## 7. 数值等价与科学复现的验收标准

1. **结构一致**：MAT 变量名、class/type、维度、列主序、数值和压缩格式被正确解释。单变量一个文件的旧兼容规则与多变量严格访问规则都有测试。
2. **几何一致**：三种视线采样都满足原始数量公式、单位范数、递归顺序；光流切向满足 d·p≈0，翻转方向、符号、坐标系由固定黄金样本检验。
3. **算法一致**：Original KvD 与 Modified KvD 分别保留不同方程；Franz 3 轴模板、Adaptive 6×6 线性系统以及球谐拟合/旋转都可对照逐步结果。
4. **异常真实**：MAT 损坏、形状不匹配、零模向量、病态求解、非有限数、未定义方向误差必须明确报告，原版异常不应被无声“修复”成通过。
5. **数据与随机分离**：固定输入对照可先使用相对误差 1e−10 作为候选基线（根据计算顺序和条件数调整）；随机实验先比较脚本规定的统计结果，不要求不同 RNG 的同种子样本逐位一致。
6. **实验一致**：bias 的 40 个原始子目录与 Adaptive 的四类数据集彼此独立统计；无噪/含噪、初始化、更新间隔均与对应源码参数一致。
7. **图表一致**：曲线使用同一结果矩阵、定义与索引；图片格式和外观可不同，但曲线数值、相对比较、纵横轴单位不可改变。
8. **状态透明**：生成代码、提交远端、构建、CTest、原始数据回归、论文复现、PR 交付均逐项报告，没有执行的内容不能写成通过。

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

## 9. 来源与版本声明

- 原始论文：**Insect-Inspired Self-Motion Estimation with Dense Flow Fields—An Adaptive Matched Filter Approach**，Strübbe / Stürzl / Egelhaaf（2015），https://doi.org/10.1371/journal.pone.0128413
- MATLAB 原始资料：**Egomotion Estimation with Dense Flow Fields: MATLAB Scripts**，https://doi.org/10.4119/unibi/2736885
- 本仓库原始文件目录：origin/egomotion_estimation_dense_flow_fields_matlab_scripts/
- 本项目规则：[AGENTS.md](AGENTS.md)
- **审计深度**：已经逐个读取全部 45 个 MATLAB 文件和 readme；通过 GitHub 仓库树核对 582 个 MAT 路径，抽样检查多个 MAT 文件头；尚未通过 matio 解包校验全部 MAT 变量，也没有编译、运行 MATLAB/C++ 或重做实验。
- 本计划只代表后续可执行工作拆分，不视为任何阶段已交付。后续开发须对照实际源码进一步填充函数/数据对照文档，并在产生新事实时以独立提交更新计划。

**最终架构决策：C++17 + Eigen 3 + matio/zlib + CMake；保留原始 MAT 文件与完整原始资料目录；按模块移植 45 个 MATLAB 文件所涵盖的数值和分析流程；每个子任务独立验证后集成；目标仅 Debian/Ubuntu。**
