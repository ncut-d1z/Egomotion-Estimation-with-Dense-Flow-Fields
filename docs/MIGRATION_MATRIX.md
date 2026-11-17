# 源 MATLAB 文件到当前 C++ 实现的迁移矩阵（MIGRATION_MATRIX）

> **2026-11-18 KvD映射更新 `[AI-EDIT]`**：`df511f9924b75ded66b437fd533ca888918420c1`将共享函数/枚举替换为`KvDIterationEngine`与两种`IKvDTranslationRule`，下表两个入口已同步；两模式CTest为ON56/56、OFF55/55，见 [策略验证](reviews/R6_KVD_STRATEGY_2026-11-18.md)。保留45份源/50个函数范围与既有REQ/TC义务，未冻结或正式验收；下段为首次现行映射复核身份。

> **2026-11-17 事实复核 `[AI-EDIT]`，未冻结/未正式验收。** 检查文档HEAD `b3cc6eda6168f10102dd54e559222dca23c237c5`，生产/测试代码仍为受测 `130a82ba0090e186fcf43bc28bb98c282e68c96c`。本轮把早期候选分配更新为实际符号、文件和有限证据；不修改SRS义务、算法、阈值或设计批准状态。候选类职责仍见 [OBJECT_MODEL §7](OBJECT_MODEL.md)，不要将其中名称当成现有C++ API。

原归档45份.m、50个函数声明均保留；源Git blob/字节/REQ/TC的单一身份清单见 [SOURCE_INVENTORY](SOURCE_INVENTORY.md)。下表“测试组”引用 [66 TC证据映射](reviews/R6_TC_EVIDENCE_MAP_2026-11-17.md#实际测试组及证据入口) 的完整CTest名称、测试源码及报告；同组有关联不表示该文件全部输入域已测。首次48/48实测及原MAT不变证据见 [封存读取记录](reviews/R6_MAT_SNAPSHOT_2026-11-17.md)，后续实测按上方固定SHA记录。

- 当前实现以组合复用、Eigen向量运算和内部辅助函数承接源职责，不要求45个同名类或50个公开函数。
- “有对应实现”与“原数值实验完整成功”分开；三Adaptive生成入口仍失败，完整科学/L4、正式容差/资源/基线验收仍未完成。
- 源脚本只读静态审查；没有运行MATLAB/Octave/L1，也不将其列为后续前置条件。

## 45份源文件的现行对应

路径均相对 `origin/egomotion_estimation_dense_flow_fields_matlab_scripts/`；函数数按真实声明行，命名空间以链接源码为准。全部行均未正式验收。

| 原源相对路径 | 函数声明数 | 当前C++符号及文件 | 测试组 | 已有证据与边界 |
|---|---:|---|---|---|
| `adaptive_matched_filter_test/data_analysis/graph_agent_in_sphere_adaptive_matched_filter_many_parameter.m` | 1 | `MatResultAnalyzer::analyze` → `detail::adaptive(UpdateRates)`；[adaptive.cpp](../src/analysis/adaptive.cpp) | ANALYSIS、CLI、SNAPSHOT、SVG | 四段阈值0.02、窗口0/2/5/10与600行先平滑后截取已测；缺ratio/NaN拒绝，历史归档不保证分析成功。 |
| `adaptive_matched_filter_test/data_analysis/graph_agent_in_sphere_adaptive_matched_filter_test.m` | 1 | `MatResultAnalyzer::analyze` → `detail::adaptive(Comparison)`；[adaptive.cpp](../src/analysis/adaptive.cpp) | ANALYSIS、CLI、SNAPSHOT、SVG | 50:550、阈值0.01、四段及双纵轴数据已测；有限合成参考与历史NaN拒绝分别记录。 |
| `adaptive_matched_filter_test/data_analysis/graph_agent_in_sphere_initialization_test.m` | 1 | `MatResultAnalyzer::analyze` → `detail::adaptive(Initialization)`；[adaptive.cpp](../src/analysis/adaptive.cpp) | ANALYSIS、CLI、SNAPSHOT | 读取四字段，绘制mittrans 1:13、mitrot 1:35；历史实际进程成功，非新生成实验成功证据。 |
| `adaptive_matched_filter_test/data_generation/agent_in_sphere_adaptive_matched_filter_many_parameter.m` | 1 | `ProductionExperimentRunner::runAdaptiveUpdateRates` → `AdaptiveExperimentRunner::computeUpdateRates` → `AdaptiveMfaSession::processFrame`；[runner](../src/experiments/adaptive_runner.cpp)、[session](../src/estimators/adaptive_session.cpp) | RUNNER、SESSION、ADAPTIVE-CLI | 1/5/11/21刷新/现行save合同有例；真实第151帧零旋转拒绝、valid149、Incomplete，无完整MAT。 |
| `adaptive_matched_filter_test/data_generation/agent_in_sphere_adaptive_matched_filter_test.m` | 1 | `ProductionExperimentRunner::runAdaptiveComparison` → `AdaptiveExperimentRunner::computeComparison` → `AdaptiveMfaSession::processFrame`；[runner](../src/experiments/adaptive_runner.cpp)、[session](../src/estimators/adaptive_session.cpp) | RUNNER、SESSION、ADAPTIVE-CLI | 600帧/8次聚合/失败不改分母有例；真实noise0第151帧拒绝、valid149，未完成8轮。 |
| `adaptive_matched_filter_test/data_generation/agent_in_sphere_initialization_test.m` | 1 | `ProductionExperimentRunner::runAdaptiveInitialization` → `AdaptiveExperimentRunner::computeInitialization` → `AdaptiveMfaSession::processFrame`；[runner](../src/experiments/adaptive_runner.cpp)、[session](../src/estimators/adaptive_session.cpp) | RUNNER、SESSION、ADAPTIVE-CLI | 35帧编排有替身合同；真实第14帧acos越界、valid12、Incomplete，成功产物分支未实测。 |
| `bias_test/data_analysis/graphs_bias_test.m` | 1 | `MatResultAnalyzer::analyze` → `detail::bias`；[bias.cpp](../src/analysis/bias.cpp) | ANALYSIS、CLI、SNAPSHOT、SVG | 四个y跨run均值、最后x、3:end及100/sqrt(x)；两组40-run的26曲线/208对x-y独立复核，986来源摘要已测。 |
| `bias_test/data_generation/bias_test_nonspherical.m` | 1 | `ProductionExperimentRunner::runBiasOccluded` → `BiasExperimentRunner::compute`；[装配](../src/experiments/production_runner.cpp)、[计算](../src/experiments/bias_runner.cpp) | RUNNER、CLI、GEOM、KVD、METRIC | 缺损采样+均模长噪声；level0…2/seed7、11有界真实求解与发布，未重跑40份level9统计实验。 |
| `bias_test/data_generation/bias_test_spherical_prop_error.m` | 1 | `ProductionExperimentRunner::runBiasProportional` → `BiasExperimentRunner::compute`；[装配](../src/experiments/production_runner.cpp)、[计算](../src/experiments/bias_runner.cpp) | RUNNER、CLI、GEOM、KVD、METRIC | 完整采样+分量噪声；实际8×4^level视线但源xwerte仍6×4^level，有界测试保留该差别；L4未完成。 |
| `help_functions/drehvektor_aus_zwei_vektoren.m` | 1 | `trajectory::sourceRotationBetween`；[kinematics.cpp](../src/trajectory/kinematics.cpp) | GEOM、SH | 未归一叉积阈值1e-11、acos不钳制；精确反向、零/非有限向量明确失败，非所有原异常运行行为等价。 |
| `help_functions/erzeugeflussausnear.m` | 1 | `simulation::FlowSynthesizer::synthesize`；[flow.cpp](../src/simulation/flow.cpp) | GEOM、RUNNER | 切向平移/旋转流、各自均模长及相消固定例已测；限定合法单位方向和正近度。 |
| `help_functions/erzeugekreuzproduktmatrix.m` | 1 | 内部自由函数 `crossMatrix`；[adaptive_step.cpp](../src/estimators/adaptive_step.cpp) | STEP | 叉乘矩阵参与M1…M4；独立单步逐块参考覆盖符号，不新增公开math类。 |
| `help_functions/erzeugerichtungsvektoren_loch.m` | 1 | `sampling::OccludedOctahedralSampler::sample`；[directions.cpp](../src/sampling/directions.cpp) | GEOM | 保留源面2/3/5/6/7/8及深度优先次序；level0…2逐行独立参考。 |
| `help_functions/erzeugerichtungsvektoren.m` | 1 | `sampling::OctahedralSampler::sample`；[directions.cpp](../src/sampling/directions.cpp) | GEOM | 8×4^level、有序叶节点；level0…2逐行独立参考，不据此承诺任意规模资源。 |
| `help_functions/findeDreiecke.m` | 1 | 内部 `sampling::emitFace`（匿名命名空间）及 `normalizeSource`；[directions.cpp](../src/sampling/directions.cpp) | GEOM | 四子面顺序、归一化中点和叶中心；通过三类sampler间接逐行核对，不暴露独立递归API。 |
| `help_functions/fixed_kugel_symmetric_perfectpoints_randomnear.m` | 1 | `simulation::generateBiasFlow` + `OccludedOctahedralSampler` + `MeanMagnitudeGaussianNoise`；[flow.cpp](../src/simulation/flow.cpp) | GEOM、RUNNER | N个uniform后3N个normal、距离1…4/均值2.5、末个near与均流长；源未使用参数未转成假功能。 |
| `help_functions/fixed_propfehler_kugel_symmetric_perfectpoints_randomnear.m` | 1 | `simulation::generateBiasFlow` + `OctahedralSampler` + `ComponentProportionalGaussianNoise`；[flow.cpp](../src/simulation/flow.cpp) | GEOM、RUNNER | 单个xyz分量比例加噪后切平面投影；零系数仍3N抽样；不等于MATLAB RNG同序列。 |
| `help_functions/franzeigen.m` | 1 | `estimators::FranzMatchedFilter::estimate/estimateWithTrace`；[franz_kvd.cpp](../src/estimators/franz_kvd.cpp) | KVD、RUNNER | 三轴平移/旋转响应及各自分母，固定输入独立参考；零分母明确失败。 |
| `help_functions/getflowfield.m` | 1 | `io::MatResultReader::readFirstCompatible`；[mat_result_reader.cpp](../src/io/mat_result_reader.cpp) | MAT、SNAPSHOT、CLI | 后缀/首变量/多变量边界及封存读取已测，首版二维密集实double；非任意MAT结构承诺。 |
| `help_functions/getnear_in_kugel_Drehung.m` | 1 | `trajectory::SphereEnvironment::nearnessInUnitSphere`；[sphere.cpp](../src/trajectory/sphere.cpp) | GEOM | 旋转射线、球内交点/正距离解析参考与负例；不接受球外相机。 |
| `help_functions/getvektor.m` | 1 | `sampling::randomUnitDirection(IRandomSource&)`；[directions.cpp](../src/sampling/directions.cpp) | GEOM、RUNNER | 两次uniform顺序及固定方向，耗尽链；统计分布/L4不由两点样本证明。 |
| `help_functions/koeff_aus_near.m` | 1 | `harmonics::fitSourceNear` → `low_order::projectSourceCoefficientsOrderTwo`；[adapter](../src/harmonics/adapter.cpp)、[projection](../src/harmonics/low_order_source_projection.cpp) | SH、STEP | l≤2逐通道sum(Y·near)/sum(Y²)，保留源缩放顺序；不是联合最小二乘，零通道范数拒绝。 |
| `help_functions/kreis_trajektorie.m` | 1 | `trajectory::TrajectoryGenerator::circle`；[trajectory.cpp](../src/trajectory/trajectory.cpp) | GEOM、ILLUSTRATION | 分母N、圆半径/高度与600点独立例；line绘图副作用移至插图层。 |
| `help_functions/kreuzprodukt.m` | 1 | Eigen `Vector3::cross`，如 [flow.cpp](../src/simulation/flow.cpp)、[franz_kvd.cpp](../src/estimators/franz_kvd.cpp)、[adaptive_step.cpp](../src/estimators/adaptive_step.cpp) | GEOM、KVD、STEP | 三维向量运算内联复用；通过含符号的独立业务参考核对，没有同名包装API。 |
| `help_functions/kugelflfunc2.m` | 1 | `low_order::basisDirect/basisRecurrence` + MATLAB布局相位转换；[low_order_basis.cpp](../src/harmonics/low_order_basis.cpp) | SH | 求解器实际九基l≤2；CS相位/顺序经布局及投影处理，不是任意l,m角参数API。插图非归一化基另实现。 |
| `help_functions/kurveglaetten.m` | 1 | 内部 `analysis::detail::smoothSource`；[adaptive.cpp](../src/analysis/adaptive.cpp) | ANALYSIS、SNAPSHOT | 窗口半宽2/5/10、头尾零、先分段/全列两种调用次序；有限参考逐值，辅助源blob有追踪。 |
| `help_functions/mue_norm_koenderink_iteration.m` | 1 | `estimators::ModifiedKvDEstimator::estimate` → `KvDIterationEngine::estimate` + `ModifiedKvDTranslationRule::compute`；[franz_kvd.cpp](../src/estimators/franz_kvd.cpp) | KVD、RUNNER | 独立规则保留Modified公式；逐轮节点、默认停止控制流及策略替换有实测，不把两算法合并成同公式。 |
| `help_functions/near_aus_koeff.m` | 1 | `harmonics::reconstructNear` → `low_order::reconstruct`；[adapter](../src/harmonics/adapter.cpp)、[basis](../src/harmonics/low_order_basis.cpp) | SH、STEP | 逐方向重建九项，调用者遍历N行；l≤2，浮点求和与角度中间值不宣称逐位同源。 |
| `help_functions/nonpropfehler.m` | 1 | `simulation::MeanMagnitudeGaussianNoise::apply` → 内部 `noiseApply(false)`；[flow.cpp](../src/simulation/flow.cpp) | GEOM、RUNNER | 均总流模长加噪、xyz抽样与切向投影；相消流/零系数仍抽样，NoNoise是独立模式。 |
| `help_functions/norm_koenderink_iteration.m` | 1 | `estimators::OriginalKvDEstimator::estimate` → `KvDIterationEngine::estimate` + `OriginalKvDTranslationRule::compute`；[franz_kvd.cpp](../src/estimators/franz_kvd.cpp) | KVD、RUNNER | 源初始化、near翻号、末轮旧near尺度、最少6/默认最多200轮；506次两路径停止参考及策略组合，正式容差待审。 |
| `help_functions/normrelfehler.m` | 1 | 内部 `source_metrics::sourceDirectionErrorDegrees`；[source_motion_errors.hpp](../src/numerics/source_motion_errors.hpp) | METRIC、SESSION、ADAPTIVE-CLI | 角度单位degree，acos不钳制；零范数/越界拒绝。真实初始化第14帧cosine=1+2^-52已有诊断。 |
| `help_functions/oneit_adaptiv_matched_filter.m` | 1 | `estimators::AdaptiveMfaStepSolver::solve` → 内部 `detail::solveAdaptiveMotion` → `fitSourceNear`；[adaptive_step.cpp](../src/estimators/adaptive_step.cpp) | STEP、SH | 正/负均值、6×6系统、MSE、新near/系数逐节点；12条件数场景仅运动阶段可解，整步零投影拒绝。 |
| `help_functions/richtungsvektoren_icosahedron.m` | 1 | `sampling::IcosahedralSampler::sample`；[directions.cpp](../src/sampling/directions.cpp) | GEOM | 原12顶点/20面/索引和叶顺序；level0…2独立逐行，真实Comparison另用level4。 |
| `help_functions/rodrigues.m` | 1 | `trajectory::rotationMatrixFromVector` → `Sophus::SO3d::exp`；[kinematics.cpp](../src/trajectory/kinematics.cpp) | GEOM、SH | 仅生产实际调用的向量→矩阵及源eps分支；不实现反向矩阵输入、Jacobian或return后示例，见下节。 |
| `help_functions/SHRotate.m` | 6 | `low_order::rotationIvanic` + 内部 `irDegreeOne/irDegreeTwo/auxiliaryP/auxiliaryV`；[low_order_rotation.cpp](../src/harmonics/low_order_rotation.cpp) | SH | 原论文/勘误Eigen重写，仅R0/R3/R5；源组合由phase适配另承接，W的零权重与通用高阶边界见下节。 |
| `help_functions/sinuskurve_trajektorie.m` | 1 | `trajectory::TrajectoryGenerator::sinusoidal`；[trajectory.cpp](../src/trajectory/trajectory.cpp) | GEOM、ILLUSTRATION | 源分母N、不补重复终点；600点独立几何已测，后续角度误差零旋转限制仍存在。 |
| `help_functions/skalarprodukt.m` | 1 | Eigen `Vector3::dot`，如 [flow.cpp](../src/simulation/flow.cpp)、[kinematics.cpp](../src/trajectory/kinematics.cpp)、[source_motion_errors.hpp](../src/numerics/source_motion_errors.hpp) | GEOM、KVD、STEP、METRIC | 三维点积在业务公式中直接使用；独立数值参考非所有浮点归约顺序逐位证明。 |
| `help_functions/trajektorie_aus_punkten.m` | 1 | `trajectory::TrajectoryKinematics::fromPoints`；[kinematics.cpp](../src/trajectory/kinematics.cpp) | GEOM、ADAPTIVE-CLI | 源i=2…N-1、局部转置作用/前帧关系；首尾零并标无效，真实轨迹后续零旋转误差拒绝。 |
| `help_functions/transfehler_div.m` | 1 | 内部 `source_metrics::normalizedVectorError`；[source_motion_errors.hpp](../src/numerics/source_motion_errors.hpp) | METRIC、RUNNER | 先按真值范数缩放估计再取距离；独立弦长/尺度参考，零真值亦显式拒绝计数，不伪称原返回行为相同。 |
| `help_functions/transfor_koeff.m` | 1 | `harmonics::rotateSourceCoefficients` → 相位包裹 `rotateCoefficients`；[adapter.cpp](../src/harmonics/adapter.cpp) | SH、SESSION | 独立源phase参考及Session实际动作；源CS系数直接乘无CS块，不用物理参数旋转替代。 |
| `help_functions/winkelausvektor.m` | 1 | 作用被l≤2笛卡尔基 `low_order::basisDirect/basisRecurrence` 融合；[low_order_basis.cpp](../src/harmonics/low_order_basis.cpp) | SH、STEP | 没有独立theta/phi API；单位方向基值及两极有参考，不声称atan2/acos中间轨迹等价。 |
| `help_functions/zellendefinieren.m` | 1 | `FranzMatchedFilter::estimateWithTrace` 内三轴单位向量循环；[franz_kvd.cpp](../src/estimators/franz_kvd.cpp) | KVD | 替代3×10工作表的三轴累加器；响应/分母逐项有参考，3/轴数=1。 |
| `illustrations/circle_curve_in_the_sphere.m` | 1 | `illustrations::generate(CircleConfig)` + `SvgExporter::exportSvg`；[geometry](../src/illustrations/geometry.cpp)、[svg](../src/illustrations/svg.cpp) | ILLUSTRATION | 600点圆、sphere(50)、z-only遮罩及画幅/实际CLI；未声称sphere端点运行舍入或Phong像素等价。 |
| `illustrations/sinosodial_curve_in_the_sphere.m` | 1 | `illustrations::generate(SinusoidalConfig)` + `SvgExporter::exportSvg`；[geometry](../src/illustrations/geometry.cpp)、[svg](../src/illustrations/svg.cpp) | ILLUSTRATION | 600点正弦与遮罩独立参考；保留原错误圆轨迹标题，视觉边界同上。 |
| `illustrations/spherical_harmonic_illustration.m` | 1 | `illustrations::generate(HarmonicConfig)` + `SvgExporter::exportSvg`；[geometry](../src/illustrations/geometry.cpp)、[svg](../src/illustrations/svg.cpp) | ILLUSTRATION | 未归一化关联勒让德，不限求解器l≤2；degree0…8的45组独立全网格，degree200溢出拒绝，不承诺任意阶资源。 |

## SHRotate的六个声明与实际低阶实现

原源文件 [SHRotate.m](../origin/egomotion_estimation_dense_flow_fields_matlab_scripts/help_functions/SHRotate.m) 为入口加M/U/P/V/W；不能按声明数推断高阶递推已实现。

| 原声明/1-based行 | 当前承接位置 | 有限范围 |
|---|---|---|
| `SHRotate(r,degree):17` | `rotationIvanic` → `irDegreeOne/irDegreeTwo`；R0在系数旋转时直接保留 | 固定l≤2，无通用degree参数；源唯一生产调用 `transfor_koeff(...)` 固定degree=2 |
| `M(l,m,n,R):58` | `irDegreeTwo` 的u/v加权组合 | m,n=-2…2；u=0先分支，避免无效前阶索引 |
| `U(l,m,n,R):87` | 内联为 `auxiliaryP(block,0,m,n)` | 没有单独公开U方法 |
| `P(i,l,a,b,R):91` | `auxiliaryP` | n=-2、2及内部三列；前阶l=1 |
| `V(l,m,n,R):108` | `auxiliaryV` | m=0、正、负及delta分支；注释保留IR98表与公式的符号复核依据 |
| `W(l,m,n,R):127` | **没有W函数**；`irDegreeTwo` 显式说明w恒为0而省略 | l=2时m=0因(1-d)=0，abs(m)=1/2因乘积含0；原M亦仅w≠0才调用W。不是遗漏有效低阶项，也不是高阶W已移植 |

数学重写依据为1996原论文及1998勘误，生产只依赖Eigen、不含sdla。TensorDirect/Choi为现有独立对照路径，实际适配选择Ivanic。原 `transfor_koeff` 的CS系数与无CS旋转块组合由 `rotateSourceCoefficients` 的相位变换承接；`rotateCoefficients` 则具有显式物理参数作用合同，两者不能互换。[源phase及Session实测](reviews/R6_SH_SOURCE_ACTION_2026-11-17.md) 和三个low_order CTest提供有限独立证据；不等于MATLAB运行轨迹逐位一致或任意l资格。

## 调用范围与不应误报为实现完成的分支

1. [rodrigues.m](../origin/egomotion_estimation_dense_flow_fields_matlab_scripts/help_functions/rodrigues.m) 在第177行return之前确有向量/矩阵输入和Jacobian计算。当前业务调用只取旋转向量的单个矩阵输出；选定Sophus实现保留原eps分支。矩阵→向量、Jacobian没有C++对应，也无测试资格；177行后的检查示例不构成生产调用。此处记录既有范围，不新增或删除验收义务。
2. 原 `kugelflfunc2/near_aus_koeff/koeff_aus_near` 可表达不同阶数，但当前求解器HarmonicCoefficients为3×5、九个有效系数，Adaptive固定二阶。其笛卡尔基融合了 `winkelausvektor` 的作用，不提供角度中间输出。插图未归一化勒让德是独立通用阶数路径，不能用其测试证明求解器高阶支持。
3. 原脚本的图形副作用与数据生成分离到分析/插图JSON及可选SVG；不承诺MATLAB灯光、默认图例或逐像素一致。源球面端点舍入边界仍按插图报告记录。
4. 错误策略（零范数、未定义角度、非有限结果、历史缺字段拒绝）已在当前合同和测试显式处理。三Adaptive实际入口未成功；替身的Complete合同与真实失败合同均不能代替完整产物/L2对照。未知RNG运行序列不冒充MATLAB同序列。

## 静态核查与下一步

本轮集合、文件身份、函数数、链接及测试证据核对见 [复核记录](reviews/R6_SOURCE_IMPLEMENTATION_MAP_2026-11-17.md)。**45/45路径有当前实现或明确融合/范围说明，50/50声明有去向说明；这不是50个函数分支全部验证，也不等于45份实验全部成功。** 本轮只更新文档，未重跑CMake/CTest；代码与测试沿用固定130a82的48/48证据。

继续逐项处理 [TC映射](reviews/R6_TC_EVIDENCE_MAP_2026-11-17.md) 的剩余义务，先核实交付依赖/可选绘图与日志、输出合同，再准备真实完整实验、病态数值/资源及正式验收材料；具体授权仍按 [TODO](TODO.md)。不因审批延期停止可独立完成的工作，不通过改门槛把现有失败改成成功。
