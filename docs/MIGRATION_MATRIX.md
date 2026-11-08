# 源 MATLAB 文件到 C++ 模块的候选迁移矩阵（MIGRATION_MATRIX）

> **文件级及函数声明级设计分配、未冻结、未编码**。从 [OBJECT_MODEL §7](OBJECT_MODEL.md) 的现行职责表和 [SOURCE_INVENTORY](SOURCE_INVENTORY.md) 的真实 45 个 .m 路径建立；不把“一文件对应一个责任”解释为全部内部函数已审计/移植，也不创建任何生产类或头文件。

- 当前设计采用组合而非一个全能求解器：无状态点积/叉积等保留自由函数；有状态 Adaptive Session 与单步求解分开；MAT I/O 保持独立；原作者 `origin/**` 不写入。
- 物理向量转旋转矩阵已选 Sophus；球谐 l≤2 按 Ivanic–Ruedenberg 1996 原文/1998 勘误，**Eigen-only、禁 sdla**；Don Williamson 仅参考，不以本表宣称论文已读取或递推已实现。
- 源文件 Git blob SHA、字节与 REQ/TC 的单一来源见 [SOURCE_INVENTORY](SOURCE_INVENTORY.md) 和 SRS §9.2，避免在本矩阵复制会频繁变化的源码身份；真实类签名、错误/生命周期、TC→实体见模块 LLD 与 TEST_PLAN。

| 原源相对路径 | 函数声明数 | 预定 C++ 模块/方法（候选） | 当前状态 |
|---|---:|---|---|
| `adaptive_matched_filter_test/data_analysis/graph_agent_in_sphere_adaptive_matched_filter_many_parameter.m` | 1 | AdaptiveResultAnalyzer::updateRates | 仅设计映射；未实现/未测试 |
| `adaptive_matched_filter_test/data_analysis/graph_agent_in_sphere_adaptive_matched_filter_test.m` | 1 | AdaptiveResultAnalyzer::comparison | 仅设计映射；未实现/未测试 |
| `adaptive_matched_filter_test/data_analysis/graph_agent_in_sphere_initialization_test.m` | 1 | AdaptiveResultAnalyzer::initialization | 仅设计映射；未实现/未测试 |
| `adaptive_matched_filter_test/data_generation/agent_in_sphere_adaptive_matched_filter_many_parameter.m` | 1 | UpdateRateRunner::run | 仅设计映射；未实现/未测试 |
| `adaptive_matched_filter_test/data_generation/agent_in_sphere_adaptive_matched_filter_test.m` | 1 | AdaptiveComparisonRunner::run | 仅设计映射；未实现/未测试 |
| `adaptive_matched_filter_test/data_generation/agent_in_sphere_initialization_test.m` | 1 | InitializationRunner::run | 仅设计映射；未实现/未测试 |
| `bias_test/data_analysis/graphs_bias_test.m` | 1 | BiasResultAnalyzer::aggregate | 仅设计映射；未实现/未测试 |
| `bias_test/data_generation/bias_test_nonspherical.m` | 1 | BiasExperimentRunner::runOccluded | 仅设计映射；未实现/未测试 |
| `bias_test/data_generation/bias_test_spherical_prop_error.m` | 1 | BiasExperimentRunner::runProportional | 仅设计映射；未实现/未测试 |
| `help_functions/drehvektor_aus_zwei_vektoren.m` | 1 | math::rotationVectorBetween (自由函数) | 仅设计映射；未实现/未测试 |
| `help_functions/erzeugeflussausnear.m` | 1 | FlowSynthesizer::synthesize | 仅设计映射；未实现/未测试 |
| `help_functions/erzeugekreuzproduktmatrix.m` | 1 | math::crossMatrix (自由函数) | 仅设计映射；未实现/未测试 |
| `help_functions/erzeugerichtungsvektoren_loch.m` | 1 | OccludedOctahedralSampler::sample | 仅设计映射；未实现/未测试 |
| `help_functions/erzeugerichtungsvektoren.m` | 1 | OctahedralSampler::sample | 仅设计映射；未实现/未测试 |
| `help_functions/findeDreiecke.m` | 1 | sampling::subdivideTriangle (私有自由函数) | 仅设计映射；未实现/未测试 |
| `help_functions/fixed_kugel_symmetric_perfectpoints_randomnear.m` | 1 | BiasExperimentRunner::synthesizeOccludedInput（组合采样、FlowSynthesizer、噪声策略） | 仅设计映射；未实现/未测试 |
| `help_functions/fixed_propfehler_kugel_symmetric_perfectpoints_randomnear.m` | 1 | BiasExperimentRunner::synthesizeProportionalInput（组合采样、FlowSynthesizer、噪声策略） | 仅设计映射；未实现/未测试 |
| `help_functions/franzeigen.m` | 1 | FranzMatchedFilter::estimate | 仅设计映射；未实现/未测试 |
| `help_functions/getflowfield.m` | 1 | MatResultReader::readFirstCompatible | 仅设计映射；未实现/未测试 |
| `help_functions/getnear_in_kugel_Drehung.m` | 1 | SphereEnvironment::nearness | 仅设计映射；未实现/未测试 |
| `help_functions/getvektor.m` | 1 | sampling::randomUnitDirection(IRandomSource&)（无状态自由函数） | 仅设计映射；未实现/未测试 |
| `help_functions/koeff_aus_near.m` | 1 | HarmonicFitter::fit | 仅设计映射；未实现/未测试 |
| `help_functions/kreis_trajektorie.m` | 1 | TrajectoryGenerator::circle | 仅设计映射；未实现/未测试 |
| `help_functions/kreuzprodukt.m` | 1 | math::cross (自由函数) | 仅设计映射；未实现/未测试 |
| `help_functions/kugelflfunc2.m` | 1 | RealSphericalHarmonics::evaluate | 仅设计映射；未实现/未测试 |
| `help_functions/kurveglaetten.m` | 1 | analysis::windowMean (自由函数) | 仅设计映射；未实现/未测试 |
| `help_functions/mue_norm_koenderink_iteration.m` | 1 | ModifiedKvDEstimator::estimate | 仅设计映射；未实现/未测试 |
| `help_functions/near_aus_koeff.m` | 1 | RealSphericalHarmonics::reconstruct | 仅设计映射；未实现/未测试 |
| `help_functions/nonpropfehler.m` | 1 | MeanMagnitudeGaussianNoise::apply | 仅设计映射；未实现/未测试 |
| `help_functions/norm_koenderink_iteration.m` | 1 | OriginalKvDEstimator::estimate | 仅设计映射；未实现/未测试 |
| `help_functions/normrelfehler.m` | 1 | metrics::directionAngleDeg | 仅设计映射；未实现/未测试 |
| `help_functions/oneit_adaptiv_matched_filter.m` | 1 | AdaptiveMfaStepSolver::solve | 仅设计映射；未实现/未测试 |
| `help_functions/richtungsvektoren_icosahedron.m` | 1 | IcosahedralSampler::sample | 仅设计映射；未实现/未测试 |
| `help_functions/rodrigues.m` | 1 | math::rotationVectorToMatrix，内层规划调用 Sophus::SO3d::exp()（仅实际使用的向量→矩阵分支） | 仅设计映射；未实现/未测试 |
| `help_functions/SHRotate.m` | 6 | `egomotion::harmonics::HarmonicRotator::blocks(const Eigen::Matrix3d&) const`；通过 [球谐专题 LLD §3](design/SH_ROTATION_IVANIC_RUEDENBERG.md) 的唯一声明生成 R3/R5（R0=1 不存可变成员）；按 Ivanic–Ruedenberg 1996 原文及 1998 勘误 **纯 Eigen 重写**，Don Williamson 仅供参考、禁止 sdla | 仅设计映射；未实现/未测试 |
| `help_functions/sinuskurve_trajektorie.m` | 1 | TrajectoryGenerator::sine | 仅设计映射；未实现/未测试 |
| `help_functions/skalarprodukt.m` | 1 | math::dot (自由函数) | 仅设计映射；未实现/未测试 |
| `help_functions/trajektorie_aus_punkten.m` | 1 | TrajectoryKinematics::fromPoints | 仅设计映射；未实现/未测试 |
| `help_functions/transfehler_div.m` | 1 | metrics::scaleAlignedVectorError | 仅设计映射；未实现/未测试 |
| `help_functions/transfor_koeff.m` | 1 | `egomotion::harmonics::HarmonicRotator::rotate`；接口声明见 [球谐专题 LLD §3](design/SH_ROTATION_IVANIC_RUEDENBERG.md) | 仅设计映射；未实现/未测试 |
| `help_functions/winkelausvektor.m` | 1 | math::directionToAngles | 仅设计映射；未实现/未测试 |
| `help_functions/zellendefinieren.m` | 1 | FranzMatchedFilter::axisTemplates | 仅设计映射；未实现/未测试 |
| `illustrations/circle_curve_in_the_sphere.m` | 1 | IPlotExporter::drawCircleInSphere (可选) | 仅设计映射；未实现/未测试 |
| `illustrations/sinosodial_curve_in_the_sphere.m` | 1 | IPlotExporter::drawSineInSphere (可选) | 仅设计映射；未实现/未测试 |
| `illustrations/spherical_harmonic_illustration.m` | 1 | IPlotExporter::drawHarmonic (可选) | 仅设计映射；未实现/未测试 |

## 函数声明级补充（AF-072：50 定义、仅静态声明）

对来源目录下 **45 个 `.m` 逐文件只读查看 `function` 声明**：44 份文件各有一个与文件名对应的主函数，`help_functions/SHRotate.m` 则有 **1 个入口 + 5 个文件内子函数**，合计 **50 个函数定义**。表格的「函数声明数」严格按实际代码声明行计数，不推断函数分支、输出已被验证，更不能折算 C++ 类/接口数。

| 原源文件与行（1-based） | 声明 | 迁移审计责任（未实现） |
|---|---|---|
| `SHRotate.m:17` | `SHRotate(r,degree)` | `HarmonicRotator::blocks` 的设计主入口；R0/R3/R5 |
| `SHRotate.m:58` | `M(l,m,n,R)` | band 递推的 U/V/W 加权组合，检查 u/v/w 边界和零系数条件 |
| `SHRotate.m:87` | `U(l,m,n,R)` | 从 P 构造 U 分量，保持源索引语义 |
| `SHRotate.m:91` | `P(i,l,a,b,R)` | 根据 b=±l 或内部 m 选择前一阶旋转块，检查边界行列索引 |
| `SHRotate.m:108` | `V(l,m,n,R)` | m=0、m>0、m<0 的分段公式和符号 |
| `SHRotate.m:127` | `W(l,m,n,R)` | m≠0 的两路分段及 P 符号 |

这些内部子函数可在将来以 **私有辅助函数/内部实现**承担，不要求增加五个公开 C++ 方法；实际实现必须依据 Ivanic–Ruedenberg **1996 原文＋1998 勘误**，纯 Eigen 独立重写、禁 sdla，禁止以源 MATLAB 对 Don Williamson 的转述替代论文逐式审查。独立 L3 的 R3(9)/R5(25)、相位/方向、近零/复合旋转和真实 CTest 全部未完成。另见 [球谐 LLD](design/harmonics.md)、[专题声明](design/SH_ROTATION_IVANIC_RUEDENBERG.md)。

**一个例外**：`help_functions/rodrigues.m` 的 `return;` 位于第 **177 行**；其后的 Jacobian/随机旋转检查例子不再构成新的函数声明，且不是调用这个函数时会自动运行的生产路径。不得把这些示例当成项目原算法的运行条件或已经通过的测试。对原函数实际使用的“向量→矩阵”分支仍以已选 Sophus `SO3d::exp` 方案及原 epsilon 判据为准。

## 后续需要完成但**不由本矩阵宣称已完成**

- 已列出 45 个文件中全部 **50 个函数声明**；仍须逐项审查各函数内部全部数学分支、调用图、错误/非有限路径、状态、输出及 MAT 读写。声明清点≠数学等价或实现完成。
- HLD/LLD 以真实可编译 C++17 接口、Result/Status、RAII、RNG 注入、Session 状态、Writer 路径安全逐模块冻结；实际代码、CMake target 和 CTest 均不存在完整业务验收证据。
- 使用 L2 原作者 MAT（只读逐值及独立 SHA-256）、L3 独立逐节点/一般与退化输入、必要 L4 统计；不要求运行 MATLAB/Octave 或采集 L1。DEC-005/006/008、H-10/H-11 及人类签署仍按 [TODO](TODO.md) 处理。

**文件级静态核对：45/45；声明级静态清点：50/50（含 SHRotate 内部 5 个）；数学逐式/实际代码/CTest 验收：未进行。**
