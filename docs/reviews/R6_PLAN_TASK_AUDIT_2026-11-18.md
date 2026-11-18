# R6：PLAN 的98项任务逐项复核

> **接续实测**：`34fe509240167a357afc43da71c588534bee005c`两模式构建无诊断，**SVG ON63/63、OFF62/62 CTest**；先MAT7项，再56/55项，零失败/跳过/禁用。P8.9三具名Runner各自编排，各172962项交错/错误Context/失败后复用与原回归通过，见 [专项记录](R6_ADAPTIVE_ENTRIES_2026-11-18.md)。D2已列明源码职责缺口补足，总体设计验收未签署；此前各专项及Issue证据保留，#1/#2/#3仍开放。首次复核起点保留，相关任务按新证据更新，其它义务不变。

`[AI-EDIT]`，2026-11-18。本表逐项保留 [PLAN](../PLAN.md) 的98个P编号和原义务，核对实际文件、现有测试及缺口；**不修改需求、验收门槛、批准或冻结状态，也不将“有代码/测试通过”改称“任务已验收”**。原始任务全文及REQ/TC/AC/依赖仍以PLAN为准；下列“核对对象”只是定位摘要，不替代它。

起点本地与GitHub branch API均为 `e24e759e796ac18e11d57d56d12683231956f829`，工作区干净，分支 `dev-plan-P0-P1-mat-dictionary-probe`。关联任务“检查R5实现完成度”最新可见用户要求继续检查/实现R6，当前idle；未向其发消息。all-state Issue/PR查询为空。本轮只做源码/已有产物的只读复核及文档编辑，没有新CMake/CTest/程序运行或环境安装。

首次复核所用受测源码 `4c2973b19c148f640613b03309e8756006adb053` 的四份JUnit逐名核对：SVG ON 6+47=53、OFF 6+46=52，全部status=run、无failure/skip/disabled；配置唯一测试差异为 `r06_svg_source_windows`。历史证据不能自动覆盖新源码；首次起点与该SHA仅差文档。原树628文件/582MAT，`origin`树SHA `70ee155131c24a50169346d0ffbaff0c01b1c547`。

## 如何阅读

“实测（有限）”只证明第三列具体程序/输入及结果，不关闭其关联全部AC。“已有材料”只表示真实文件存在且内容已核对。“部分/未完成”列出明确不足。**不计算完成百分比，不以98行有文字等同98项已完成。**

证据组 ANALYZER-SERVICE、ADAPT-SCENE、ADAPT-INJECT、BIAS-INJECT、CLI、HEADLESS、RUNID、STREAM、EXECUTION、DICTIONARY、MAT、SNAPSHOT、IO、CONTRACT、GEOM、KVD、METRIC、STEP、SH、SESSION、RUNNER、ANALYSIS、SVG、ILLUSTRATION、SOURCE、ENV 的实际CTest/文件/日志见 [66项TC索引](R6_TC_EVIDENCE_MAP_2026-11-17.md)。为便于逐行定位，下表使用以下简写及代码锚点：

| 本表索引 | 真实文件与对应证据 |
|---|---|
| SRC | [SOURCE_INVENTORY](../SOURCE_INVENTORY.md)、[MAT_DATA_SCHEMA](../MAT_DATA_SCHEMA.md)、[MIGRATION_MATRIX](../MIGRATION_MATRIX.md)、[45源复核](R6_SOURCE_IMPLEMENTATION_MAP_2026-11-17.md)；对应SOURCE |
| BUILD | [CMakeLists](../../CMakeLists.txt)、[README](../../README.md)、[无绘图构建](R6_HEADLESS_BUILD_2026-11-17.md)、[最新双模式证据](R6_ADAPTIVE_ENTRIES_2026-11-18.md)；对应ENV/HEADLESS |
| DESIGN | [OBJECT_MODEL](../OBJECT_MODEL.md)、[ARCHITECTURE](../ARCHITECTURE.md)、[contracts](../design/contracts.md)，均为候选；实际 [领域类型](../../include/egomotion/domain/types.hpp)、[窄接口](../../include/egomotion/contracts/interfaces.hpp) |
| GEO | [方向采样](../../src/sampling/directions.cpp)、[随机源](../../src/sampling/random.cpp)；对应GEOM |
| FLOW | [光流及噪声](../../src/simulation/flow.cpp)、[接口](../../include/egomotion/simulation/flow.hpp)；对应GEOM/RUNNER |
| TRAJ | [轨迹](../../src/trajectory/trajectory.cpp)、[运动学](../../src/trajectory/kinematics.cpp)、[球内近度](../../src/trajectory/sphere.cpp)；对应GEOM |
| KVD | [Franz/KvD](../../src/estimators/franz_kvd.cpp)、[公共头](../../include/egomotion/estimators/franz_kvd.hpp)；对应KVD |
| SH | [组件公共头](../../include/egomotion/harmonics/services.hpp)、[基值/重建](../../src/harmonics/real_spherical_harmonics.cpp)、[拟合](../../src/harmonics/harmonic_fitter.cpp)、[旋转适配](../../src/harmonics/harmonic_rotator.cpp)、[低阶叶子](../../include/egomotion/harmonics/low_order.hpp)；对应SH |
| STEP / SESSION | [单步](../../src/estimators/adaptive_step.cpp)、[运动阶段](../../src/estimators/adaptive_motion.hpp)、[会话](../../src/estimators/adaptive_session.cpp)、[会话公共头](../../include/egomotion/estimators/adaptive_session.hpp)；对应同名组 |
| RUN | [生产编排](../../src/experiments/production_runner.cpp)、[Bias](../../src/experiments/bias_runner.cpp)、[Adaptive逐帧](../../src/experiments/adaptive_runner.cpp)、[三入口](../../src/experiments/adaptive_entries.cpp)、[RunContext](../../src/experiments/run_context.cpp)；对应RUNNER |
| MAT / IO | [Reader](../../src/io/mat_result_reader.cpp)、[封存](../../src/io/mat_read_snapshot.cpp)、[Writer](../../src/io/run_mat_writer.cpp)、[目录](../../src/io/run_directory.cpp)；对应MAT/SNAPSHOT/IO |
| ANALYSIS / SVG | [分析分派](../../src/analysis/result_analyzer.cpp)、[Bias数值](../../src/analysis/bias.cpp)、[Adaptive数值](../../src/analysis/adaptive.cpp)、[SVG](../../src/analysis/svg_plot.cpp) |
| CLI / EXECUTION | [参数解析](../../src/cli/command.cpp)、[实际装配](../../src/cli/application.cpp)、[运行记录](../../src/cli/execution_record.cpp) |
| NUM | [数值证据汇总](../NUMERICAL_EQUIVALENCE.md)；区分L2/L3/L4、工程回归和正式资格 |

## 逐项结果

| P | 核对对象 | 实际承接/证据 | 证据状态 | 尚未关闭的边界 |
|---|---|---|---|---|
| P0.1 | 源树与身份 | SRC、MAT：628文件、45.m/582MAT/1readme，Git树和固定摘要 | 已有材料/实测 | 人工来源基线仍未签署；Git blob与独立文件SHA-256分别标识。 |
| P0.2 | 函数及调用清单 | SRC：45路径、50函数声明现行去向；含SHRotate六声明 | 已有材料 | 静态签名/输入输出与业务承接可追溯；不代表原MATLAB运行轨迹或每个未调用分支已移植。 |
| P0.3 | 六套归档结果与save/load | SRC、MAT、RUN、ANALYSIS：560 Bias+22 Adaptive；当前/历史字段分离 | 已有材料/实测 | 历史额外字段成因不推断，DEC-004选择不重开。 |
| P0.4 | MIT及重写来源 | SRC、SH、BUILD：根LICENSE、论文/勘误来源、Eigen-only | 已有材料/实测 | 不将根MIT授予原数据；普通版权复核不阻塞编码。 |
| P0.5 | MAT字典与失败分类 | MAT、DICTIONARY：582真实扫描、独立值/摘要及类型负例 | 已有材料/实测 | 不是全MAT类型兼容，P10.4人工复核仍独立。 |
| P0.6 | 三份追踪文档 | SRC、MAT：SOURCE_INVENTORY/MAT_DATA_SCHEMA/MIGRATION_MATRIX | 已有材料 | 本表补98任务；正式最终追踪/验收不因文档存在而完成。 |
| P0.7 | 对象模型 | DESIGN、SRC：候选所有权与实际符号映射 | 部分 | 设计中若干具体类/接口与实现不同，见下方D1–D4，不称候选对象图已经实现。 |
| P0.8 | 最小接口与错误合同评审 | DESIGN、CONTRACT、CLI：Result/错误链/入口已有实测 | 部分 | DEC-008及HLD/LLD冻结未批准；不据测试通过代签。 |
| P1.1 | C++17与依赖工程 | BUILD：CMake两配置及ELF/链接图，Ubuntu24.04.5 | 实测（有限） | 其它发行版未验证；Sophus相关target和Eigen-only SH已追踪，无sdla。 |
| P1.2 | CLI与独立输出 | CLI、IO：解析/退出码/只读inspect、五实验及分析绘图 | 实测（有限） | inspect分派不调用求解器，不声称是独立最小二进制；schema冻结及日志缺口另列。 |
| P1.3 | matio RAII | MAT、IO、CONTRACT：句柄/封存对象/失败释放 | 实测（有限） | 负例和fd计数是有限资源证据，不证明所有资源耗尽路径。 |
| P1.4 | MAT维度/类型/列主序 | MAT：582文件/33340值独立按位对照及94载荷场景 | 实测（有限） | 首版v5/zlib二维密集实double，允许mxDOUBLE物理miINT32，不静默single升级。 |
| P1.5 | 非目标类型拒绝 | MAT、MAT73：Reader29场景、原始标签负例及版本32场景 | 实测（有限） | cell/struct/sparse/complex/logical/高维/v4及真实v7.3拒绝已有例；见D5，实际无HDF5工具链和正式资格未验证。 |
| P1.6 | named与首变量读取 | MAT、SNAPSHOT：后缀、多变量、首变量与命名读取 | 实测（有限） | 保留两入口语义；不把任意未知MAT结构称兼容。 |
| P1.7 | 新MAT写回读 | IO、RUN：七Bias文件和合成Adaptive产物、安全发布/回读 | 实测（有限） | 只在隔离沙箱；三个真实Adaptive无完整输出。 |
| P1.8 | 探针与生产Reader分开核对 | DICTIONARY、MAT、SNAPSHOT：两者均实际执行 | 部分 | 存储复核已做；性能预算与人工字典验收仍未完成。 |
| P1.9 | move-only句柄与输出安全 | IO、MAT、CONTRACT：符号链接、拒覆盖、发布故障/寿命 | 实测（有限） | 实际RunDirectory锚定固定根，未声称无限资源或全部竞态证明。 |
| P1.10 | 窄Reader/Writer接口 | CONTRACT、IO、CLI：IResultReader/IResultWriter及内存替身 | 实测（有限） | 公共ABI未冻结；matio C句柄不进入求解器接口。 |
| P2.1 | N×6与数学量合同 | GEO、CONTRACT：double/列主序领域值、单位和边界检查 | 部分 | 领域值是aggregate，构造时不自动验证所有不变量；见D1。 |
| P2.2 | 向量基础运算 | GEO、KVD、STEP：Eigen dot/cross与内部叉乘矩阵 | 实测（有限） | 融合入业务公式，无机械同名包装类；独立固定公式覆盖符号。 |
| P2.3 | 递归四分 | GEO：emitFace/normalizeSource，独立有序JSON | 实测（有限） | level0…2全行；任意深度资源和全部浮点输入不由此证明。 |
| P2.4 | 完整八面体 | GEO：OctahedralSampler，8×4^level | 实测（有限） | 同层点序和原face顺序已核。 |
| P2.5 | 缺损八面体 | GEO：OccludedOctahedralSampler，保留源面2/3/5/6/7/8 | 实测（有限） | 6×4^level；level0…2逐行，不将面集互换。 |
| P2.6 | 二十面体 | GEO：IcosahedralSampler，源顶点/面表 | 实测（有限） | 20×4^level，独立逐行到level2；实际Comparison使用level4但未完整成功。 |
| P2.7 | 随机单位方向 | GEO、RUN：两uniform固定序列/耗尽 | 实测（有限） | 不证明不同语言同seed序列或L4随机分布。 |
| P2.8 | 值语义与不变量 | CONTRACT、MAT、SH：拥有载荷、复制移动/寿命与布局 | 部分 | 实际返回类型含KvDResult/AdaptiveStepResult，无统一SolverResult；构造不变量见D1。 |
| P2.9 | 可替换方向采样 | GEO、RUN：IDirectionSampler三实现，Bias及Adaptive均显式注入 | 实测（有限） | Adaptive级别2/4/2及默认/显式装配逐位输入已测，正式对象边界验收仍独立。 |
| P3.1 | 理想切向光流 | FLOW：idealFlow、平移/旋转/合成和均流长 | 实测（有限） | 固定解析输入；不以单族等同全域数学证明。 |
| P3.2 | 固定随机深度 | FLOW、RUN：距离1…4、平均2.5、倒数与抽样顺序 | 实测（有限） | uniform/normal消费序保留；MATLAB RNG内部实现未验证。 |
| P3.3 | 缺损视场均模长噪声 | FLOW、GEO、RUN：generateBiasFlow+MeanMagnitudeGaussianNoise | 实测（有限） | N uniform后3N normal，末端切向投影。 |
| P3.4 | 球面分量噪声 | FLOW、GEO、RUN：ComponentProportionalGaussianNoise | 实测（有限） | 各分量尺度和零系数仍抽样已测，与均模长路径分开。 |
| P3.5 | 轨迹nonpropfehler | FLOW、ADAPTIVE-CLI：独立来源参数、共享均流长公式 | 实测（有限） | 实际noise0调用路径有证据；完整含噪轨迹/L4未运行。 |
| P3.6 | 单位球旋转射线求交 | TRAJ、GEO：SphereEnvironment | 实测（有限） | 球内/边界/非单位/非法输入有固定解析例。 |
| P3.7 | 固定RNG与统计分离 | FLOW、RUN：固定序列及人工光流 | 部分 | 确定性参考已有，随机统计/L4未完成。 |
| P3.8 | 显式随机源与独占 | CONTRACT、RUN：FixedSequence/Mt19937、RunContext | 实测（有限） | 两RunContext交替状态已测，不承诺单对象并发。 |
| P3.9 | 三种噪声策略 | FLOW：IFlowNoiseModel/MeanMagnitude/Component/NoNoise | 实测（有限） | 参数语义及抽样调用已核；实际注入边界以公共头为准。 |
| P4.1 | Franz三轴模板 | KVD：FranzMatchedFilter内三轴单位向量累加 | 实测（有限） | 没有源3×10工作表API，响应/分母由trace核对。 |
| P4.2 | Franz响应归一 | KVD：estimateWithTrace独立L3 | 实测（有限） | 零分母明确失败，不伪造估计。 |
| P4.3 | KvD共享迭代 | KVD：KvDIterationEngine借用IKvDTranslationRule | 实测（有限） | near翻号、旧运动更新、唯一归一化、残差/停止及最终缩放保留；原独立逐节点/停止参考及策略调用顺序均通过。 |
| P4.4 | Original更新 | KVD：OriginalKvDEstimator | 实测（有限） | 额外near因子、顺序及最终尺度独立逐轮核对。 |
| P4.5 | Modified更新 | KVD：ModifiedKvDEstimator与ModifiedKvDTranslationRule | 实测（有限） | 独立具名入口和平移规则，逐轮节点保留Modified公式。 |
| P4.6 | 6…200停止 | KVD：506次独立标量参考、75080检查 | 实测（有限） | 轮数/继续标志精确；原门限未变，输入族限定。 |
| P4.7 | 两种误差函数 | METRIC：normalizedVectorError/sourceDirectionErrorDegrees | 实测（有限） | 703检查；零范数/acos域外拒绝，不等同MATLAB异常行为。 |
| P4.8 | 逐轮独立L3 | KVD：已有verification/kvd参考与停止解析族 | 实测（有限） | 最终DEC-005及完整科学资格未批准。 |
| P4.9 | 两入口与策略组合 | KVD：两具名Estimator各自组合引擎与Original/Modified规则 | 实测（有限） | 可替换IKvDTranslationRule及两入口差异/交错隔离/故障透传已测2712项；停止控制仍有独立参考。候选接口未冻结，不代签结构验收，见D2。 |
| P4.10 | Franz服务状态隔离 | KVD、RUN：无成员模板状态、const estimate | 实测（有限） | 有限重复/交替调用与源码可复核；不是全线程安全证明。 |
| P5.1 | 缺损Bias入口 | RUN、CLI：三噪声列、有限level0…2、真实求解 | 部分 | 完整原规模/40目录重生成未运行。 |
| P5.2 | 球面比例Bias入口 | RUN、CLI：独立采样/噪声装配 | 部分 | 完整规模/L4未运行；有界两模式已有结果。 |
| P5.3 | seed与运行管理 | RUNID、RUN、EXECUTION：独立seed/RunId/计数/日志 | 部分 | 五生成入口及plot/SVG导出记录已测；只读入口/分配前错误的产品记录边界仍待核对。 |
| P5.4 | 七MAT输出 | IO、RUN、CLI：xwerte及ywerte1…6变量/形状/值 | 实测（有限） | 拒覆盖与发布故障已测，限有界生成。 |
| P5.5 | 40目录数值分析 | ANALYSIS：两组26曲线/208点独立复核 | 实测（有限） | 按源取末轮x、使用ywerte3…6，不修改历史128横值。 |
| P5.6 | 560历史Bias对照 | MAT、ANALYSIS：560存储值，400实际分析输入 | 部分 | 160个ywerte1/2不贡献源曲线；未完成新40-run科学对照。 |
| P5.7 | Bias组合与依赖替换 | RUN、BIAS-INJECT：采样/噪声/三求解槽/RNG注入、RunContext借用Writer | 实测（有限） | 668项替身/调用/失败前缀/交错隔离及既有真实回归通过；见D3，不代签正式设计验收。 |
| P6.1 | 方向旋转向量 | TRAJ：rotationVectorBetween与源eps/退化边界 | 实测（有限） | 近零、反向向量的显式状态；未采集原运行轨迹。 |
| P6.2 | Sophus向量转矩阵 | TRAJ、BUILD：SO3d::exp及原小角分支 | 实测（有限） | 仅原实际向量→矩阵用途；逆向/Jacobian按既定范围未移植。 |
| P6.3 | 圆轨迹 | TRAJ、ILLUSTRATION：circle纯数据 | 实测（有限） | 原N分母、半径/高度；绘图另层。 |
| P6.4 | 正弦轨迹 | TRAJ、ILLUSTRATION：sinusoidal纯数据 | 实测（有限） | 周期/幅值/长度/高度及不重复终点已核。 |
| P6.5 | 局部运动与端点 | TRAJ：TrajectoryKinematics::fromPoints | 实测（有限） | 首尾未填状态显式；源R转置和前帧关系独立核对。 |
| P6.6 | 已知轨迹/球内输入 | TRAJ、GEO：解析参考与非法输入 | 实测（有限） | 不等于完整Adaptive运动误差成功。 |
| P6.7 | 三类职责与状态 | TRAJ、SESSION、ILLUSTRATION、ADAPT-SCENE | 实测（有限） | 轨迹/运动学/球环境分别有窄接口；原数学函数体不变，无绘图副作用。三入口装配及八轮Session重置已有合成合同证据。 |
| P7.1 | 角约定 | SH：sourceAnglesFromDirection独立返回弧度极角/方位角，原Cartesian基路径保留 | 实测（有限） | 18解析方向及1137项检查；单位输入、acos越界与C++带符号零边界明确，不声称MATLAB运行时全域等价；见D2。 |
| P7.2 | l≤2实基 | SH：direct/recurrence和MATLAB布局 | 实测（有限） | 九基/CS相位/正负m已核，非通用任意阶求解器。 |
| P7.3 | 近度重建 | SH：reconstructNear、3×5有效布局 | 实测（有限） | 填充与非法输入拒绝；九项场值固定参考。 |
| P7.4 | 逐项投影 | SH、STEP：fitSourceNear/source projection | 实测（有限） | sum(Y·near)/sum(Y²)及归一顺序；零通道拒绝，不冒充联合最小二乘。 |
| P7.5 | IR低阶旋转 | SH、BUILD：原论文/勘误、Eigen-only R0/R3/R5 | 实测（有限） | 材料已提供，无重复上传阻塞；高阶W未实现且低阶权重恒零。 |
| P7.6 | 源系数动作 | SH、SESSION：rotateSourceCoefficients与物理接口分离 | 实测（有限） | R5全项/布局/逆组合有独立参考；不凭物理旋转代替源组合，正式容差未签署。 |
| P7.7 | Adaptive矩阵阶段 | STEP：solveAdaptiveMotion、M1…M4/rhs | 实测（有限） | 九方向正负均值与12解析条件场景，矩阵和求解结果分开。 |
| P7.8 | 完整单步输出 | STEP：归一运动/MSE/near/coeff | 实测（有限） | 固定九方向成功；零投影/未定义路径拒绝，非完整多帧资格。 |
| P7.9 | Eigen与源inv/real差异 | STEP、METRIC：有限/病态/奇异诊断 | 部分 | 病态前向误差未正式验收；不能由小后向残差推定运动准确。 |
| P7.10 | 球谐职责分离 | SH：RealSphericalHarmonics/HarmonicFitter/HarmonicRotator三个固定约定组件 | 实测（有限） | 无多帧mutable状态；实际单步/Session/策略已调用，旧自由函数仅兼容转发。482项独立参考/边界及原回归通过；不代签公共API/数值验收。 |
| P7.11 | 无状态Step接口 | STEP、SESSION：IAdaptiveStepSolver/AdaptiveMfaStepSolver | 实测（有限） | 可固定输入与替身注入，未继承KvD；正式对象合同仍未冻结。 |
| P8.1 | 35帧初始化 | RUN、ADAPTIVE-CLI：真实入口与Complete编排替身 | 部分 | 真实帧14失败、valid12、无四MAT；替身99次Step不代表真实成功。 |
| P8.2 | 600帧8次对比 | RUN、ADAPTIVE-CLI：配置/聚合/真实noise0 | 部分 | 真实帧151失败、valid149、completed0；未完成8次或含噪实验。 |
| P8.3 | 前帧旋转时序 | SH、SESSION：三模式、主/比较/提交、失败回滚 | 实测（有限） | 源动作与非交换旋转已测，完整真实轨迹未成功。 |
| P8.4 | Comparison五MAT | RUN：固定8组600行聚合/ratio取末轮 | 部分 | 替身/合成发布合同已有，with/without-errors真实新产物对照未完成。 |
| P8.5 | 更新率1/5/11/21 | SESSION、RUN：冻结基线、旋转与周期刷新 | 部分 | 状态规则有证据；真实帧151失败，无完整曲线。 |
| P8.6 | 独立baseline与主状态 | SESSION：比较返回dumi弃置、主akkoeff提交 | 实测（有限） | 真实Step短序列及替身周期规则已有；完整实验保存仍缺。 |
| P8.7 | 历史额外字段 | MAT、RUN、ANALYSIS：HistoricalOnly与缺ratio拒绝 | 实测（有限） | 不伪造当前Runner字段，不猜测历史成因。 |
| P8.8 | Session与更新策略 | SESSION：唯一持久状态、ICoefficientUpdatePolicy及Reset/Inherit/Periodic | 实测（有限） | 策略可替换、全部准备先于求解、失败原子性/同帧重试/隔离48523项通过；默认源周期/旋转与真实Session回归保留，候选接口未冻结。 |
| P8.9 | 独立Runner上下文 | RUN、SESSION、ADAPT-SCENE、ADAPT-ENTRY：三具名入口独立Config/Context，逐次/逐轮新Session | 实测（有限） | 172962项具名Runner交错/12错误Context/非法Config/第五轮失败后重试，八轮固定分母及末轮ratio保持；原88121项场景保留。真实完整科学成功、记录全部边界及正式验收未完成。 |
| P9.1 | 初始化抽取/范围 | ANALYSIS、SVG、CLI：历史与固定数值 | 实测（有限） | 对数轴与取样区间已核，不声称全部MATLAB视觉细节相同。 |
| P9.2 | 轨迹Comparison分析 | ANALYSIS、SVG：ratio分段阈值0.01、双轴 | 实测（有限） | 历史NaN明确失败，有限多项式参考不能代替历史成功。 |
| P9.3 | UpdateRates分析 | ANALYSIS、SVG：阈值0.02、四组/平滑 | 实测（有限） | 2/5/10与分段顺序已核，历史NaN/缺字段仍拒绝。 |
| P9.4 | Bias统计与图层 | ANALYSIS、SVG：40-run、100/sqrt(N)、log-log | 实测（有限） | 历史全部曲线独立复核；不等于新统计复现。 |
| P9.5 | 源平滑边界 | ANALYSIS、SNAPSHOT：smoothSource、零边界/窗口 | 实测（有限） | 3460完整平滑值及辅助源blob已核，不修改边界策略。 |
| P9.6 | 可选绘图隔离 | HEADLESS、SVG、CLI：ON/OFF构建、9次后端缺席 | 实测（有限） | OFF只少SVG视窗CTest，所有数值参考保留；其他平台/后端未验证。 |
| P9.7 | 三份展示脚本 | ILLUSTRATION：独立几何/JSON/SVG及实际目视 | 实测（有限） | sphere端点运行舍入、MATLAB光照/像素等价未验证。 |
| P9.8 | 分析与导出分层 | ANALYSIS、ANALYZER-SERVICE、SVG：BiasResultAnalyzer/AdaptiveResultAnalyzer、分派入口及IPlotExporter | 实测（有限） | 两类组件拥有原数值逻辑，借用Reader/自拥有结果；727项读取顺序/40故障/导出不改值及历史独立回归通过，不代签整体分析/对象验收。 |
| P10.1 | README及直接MAT入口 | SRC、CLI、BUILD：README现行命令/两DOI/构建说明 | 已有材料 | 实际运行范围和失败限制明确；不是全平台支持承诺。 |
| P10.2 | 45源最终追踪 | SRC：45路径/50声明现行映射 | 部分 | 已定位实现与非使用范围；最终人工验收/全部分支资格未完成。 |
| P10.3 | 逐层构建测试 | BUILD及全部TC证据组：ON57/OFF56 | 实测（有限） | 每次固定SHA/日志，未把失败合同算实验成功；全部失败历史保留。 |
| P10.4 | 560+22结果合同 | MAT、ANALYSIS、RUN：582独立值、历史字段/NaN | 部分 | 数据与合同复核已有；三个Adaptive新完整产物及正式人工复核缺失。 |
| P10.5 | 论文数值图表复现 | NUM、RUN、ANALYSIS | 未完成 | 历史曲线分析/有限回归不等于完整Bias、初始化、轨迹、更新频率及L4。 |
| P10.6 | 证据汇总及平台 | NUM、PROGRESS、BUILD：本轮新增NUMERICAL_EQUIVALENCE | 部分 | 实际仅一个Ubuntu/GCC环境；完整实验/性能/资源预算仍缺。 |
| P10.7 | 来源notice与输出保护 | SRC、BUILD、IO：根MIT/来源与.gitignore | 已有材料/实测 | 不重授原作者许可，不上传output/MAT；正式发布单独授权。 |
| P10.8 | Draft PR交付 | Issue #1/#2/#3开放，尚无PR | 未完成 | 尚未创建Draft PR；不合并/发布，后续准备具体范围与证据后办理。 |
| P10.9 | OOP边界验收 | CONTRACT、IO、SESSION、RUN、SRC | 部分 | 所有权/替换/隔离有有限实测；D1–D4未核销，不代签总体设计验收。 |
| P10.10 | 避免过度继承/统一求解器 | KVD、STEP、SH、ANALYSIS：真实类及自由函数 | 部分 | 未引入统一虚假求解基类；与候选结构的差别需逐义务收敛，不能仅凭简洁或通过数宣布完成。 |

## 设计与代码的具体差别

下列差别是本次源码审计事实，**不是AI批准的新设计**。P任务中明确的结构义务不能因算法回归通过而静默删除；是否通过最小改动补齐结构或修订候选设计，需保持原职责、可替换性、生命周期和算法次序。未冻结的候选类名称也不能被报告成已经存在的生产API。

- **D1：构造不变量（P2.1/P2.8/P10.9）**。实际FlowField、MotionEstimate、HarmonicCoefficients是Eigen成员的aggregate；当前在算法入口校验，并无“构造时验证全部维度/有限性/有效填充”的统一保证。已有寿命/失败测试不证明任意默认构造对象有效。DEC-008-03的工厂/不可变决定针对Result/Status，不能扩称已批准所有领域值同样改造；继续落实候选公共边界的实际不变量，不能用文档改词抹平义务。
- **D2：策略和纯数学职责（P4.9/P7.1/P7.10/P8.8/P8.9/P9.8）**。`df511f9`已补KvD引擎/平移规则，`5318cd8`已补Session的ICoefficientUpdatePolicy及Reset/Inherit/Periodic默认组合和显式注入；48523项策略检查及既有真实Session/独立数值回归通过，见 [Session证据](R6_COEFFICIENT_POLICY_2026-11-18.md)，P4.9/P8.8结构缺口已补。`cce0bf2`另补P7.1的独立源角度接口，1137项解析/边界检查见 [角度证据](R6_SOURCE_ANGLES_2026-11-18.md)，不改变已有Cartesian基路径。`a19d2d8`已补P7.10的RealSphericalHarmonics/HarmonicFitter/HarmonicRotator三个固定约定组件，实际求解/Session/策略接入，482项独立参考/边界和原回归见 [球谐职责证据](R6_HARMONIC_SERVICES_2026-11-18.md)。`03f7f81`再补P9.8的BiasResultAnalyzer/AdaptiveResultAnalyzer，MatResultAnalyzer只分派；727项入口/故障/隔离及历史回归见 [分析职责证据](R6_ANALYZER_SERVICES_2026-11-18.md)。`34fe509`补足P8.9三具名入口的Config/Context编排与独立会话，172962项交错/故障恢复及旧回归见 [入口证据](R6_ADAPTIVE_ENTRIES_2026-11-18.md)。D2列明的源码职责缺口已补，总体对象与公共API仍未冻结/正式验收。
- **D3：编排依赖注入（P5.7/P10.9）**。首次发现Bias在compute内创建Franz/Original/Modified及RNG，与SRS附录A的**REQ-DC-EXPERIMENT-001“通过组合注入求解、RNG、采样和写入器”**存在差距。`909fd57`已补BiasEstimators三槽及RunContext RNG注入，保留采样/噪声/Writer接口、源两次reseed和求解顺序；[668项测试及真实回归](R6_BIAS_INJECTION_2026-11-18.md)验证该Bias缺口。`0cd5439`再补AdaptiveSampling的采样/噪声显式借用及Production默认拥有服务，RNG沿用RunContext；噪声success先核行数/方向/有限性。36088项组合/故障/重放及真实回归见 [Adaptive证据](R6_ADAPTIVE_INJECTION_2026-11-18.md)。`74dbccd`再补AdaptiveScene的轨迹/运动学/球环境三接口及Production默认/外部装配；原数学函数体不变，轨迹/运动学返回值在下游调用前检查。三入口完整合成场景、八轮聚合、54个服务故障及第5轮失败前缀共88121项见 [场景证据](R6_ADAPTIVE_SCENE_2026-11-18.md)。D3目前列明的装配源码缺口已补，总体OOP验收未关闭；FlowSynthesizer仍为无状态公式服务，不为凑类名添加无用途包装。
- **D4：正式运行记录边界（P5.3/P8.9/P10.6）**。五生成入口已有开始/结束记录；`00d1742`补齐plot/SVG在分配RunId后的导出记录，394项/38次调用及真实进程检查通过，见 [证据](R6_ARTIFACT_RECORDS_2026-11-18.md)。来源摘要沿用读取快照，未使用的RNG/计算Manifest明确为null；记录失败保留已有产物。inspect/analyze/JSON illustration继续只读，解析/分析/几何/RunId预留失败及信号终止仍依赖外部证据；不将这一有限补足等同整个REQ-NF-LOG验收。
- **D5：MAT v7.3真实拒绝用例已补（P1.5）**。`f1fb7d3`生成并直接matio逐位核验真实合成夹具，`0d73ed1`复现后端先打开的错误分类（32场景中29失败），[Issue #1](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/issues/1)登记后在`f1a7ab2`前移封存头预检。32场景/308项及两模式全量CTest通过，详见 [证据](R6_MAT73_REJECTION_2026-11-18.md)。v7.3拒绝不调用Mat_Open；普通构建/测试读取固定夹具，不新增HDF5依赖。后端不可用由确定性替身验证，实际无HDF5工具链未验证，元数据能力不扩大；不代签正式数据资格。

## 后续次序与权限边界

1. D3已列明的Bias/Adaptive装配、D5真实版本负例、D4的已分配RunId导出记录及D2的KvD/Session策略、P7.1角度、P7.10球谐、P9.8分析和P8.9入口职责已补。继续D1构造不变量和只读/预留前记录范围，保留原数值公式、输入只读和检查点顺序。P10.8仍缺可审查Draft PR，后续以实际范围与测试证据准备，不合并或发布。
2. 三个Adaptive真实失败（初始化帧14acos越界，另外两入口帧151零旋转）及完整Bias/L4仍未解决；不可修改门限、跳帧或改分母以获得成功。具体结果与未执行成功分支见NUM。
3. 容差、资源、跨环境、基线及发布的正式批准仍按TODO的既定延期状态处理；本表不擅自解锁、催办或代签。**没有MATLAB/Octave/L1阻塞。**
4. 本次新增NUMERICAL_EQUIVALENCE补齐了P10.6缺失的汇总文件；P10.6以及R6整体仍未完成。

## 本轮静态复核结果

按实际PLAN表提取的98个P与本表集合、顺序和唯一性一致，各阶段数量为8/10/9/9/10/7/7/11/9/8/10。两份新增报告65处相对文件引用均存在；用Git跟踪清单补足被搜索忽略规则隐藏的README，未把首次搜索漏报当作文件缺失。66项TC索引保持原编号集合。

CMake展开contract/archive两实例后得到53个唯一测试名，与已保存ON两份JUnit逐名一致；OFF只少`r06_svg_source_windows`，两套均全为status=run且失败/跳过/禁用为0。另核原始Git树及工作区无`origin/**`差异。以上是本地Git/文件及既有产物静态检查，不是运行 `scripts/check_docs_traceability.sh`，也不产生新的CTest通过记录。
