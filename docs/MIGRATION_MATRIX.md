# 源 MATLAB 文件到 C++ 模块的候选迁移矩阵（MIGRATION_MATRIX）

> **文件级设计分配、未冻结、未编码**。从 [OBJECT_MODEL §7](OBJECT_MODEL.md) 的现行职责表和 [SOURCE_INVENTORY](SOURCE_INVENTORY.md) 的真实 45 个 .m 路径建立；不把“一文件对应一个责任”解释为全部内部函数已审计/移植，也不创建任何生产类或头文件。

- 当前设计采用组合而非一个全能求解器：无状态点积/叉积等保留自由函数；有状态 Adaptive Session 与单步求解分开；MAT I/O 保持独立；原作者 `origin/**` 不写入。
- 物理向量转旋转矩阵已选 Sophus；球谐 l≤2 按 Ivanic–Ruedenberg 1996 原文/1998 勘误，**Eigen-only、禁 sdla**；Don Williamson 仅参考，不以本表宣称论文已读取或递推已实现。
- 源文件 Git blob SHA、字节与 REQ/TC 的单一来源见 [SOURCE_INVENTORY](SOURCE_INVENTORY.md) 和 SRS §9.2，避免在本矩阵复制会频繁变化的源码身份；真实类签名、错误/生命周期、TC→实体见模块 LLD 与 TEST_PLAN。

| 原源相对路径 | 预定 C++ 模块/方法（候选） | 当前状态 |
|---|---|---|
| `adaptive_matched_filter_test/data_analysis/graph_agent_in_sphere_adaptive_matched_filter_many_parameter.m` | AdaptiveResultAnalyzer::updateRates | 仅设计映射；未实现/未测试 |
| `adaptive_matched_filter_test/data_analysis/graph_agent_in_sphere_adaptive_matched_filter_test.m` | AdaptiveResultAnalyzer::comparison | 仅设计映射；未实现/未测试 |
| `adaptive_matched_filter_test/data_analysis/graph_agent_in_sphere_initialization_test.m` | AdaptiveResultAnalyzer::initialization | 仅设计映射；未实现/未测试 |
| `adaptive_matched_filter_test/data_generation/agent_in_sphere_adaptive_matched_filter_many_parameter.m` | UpdateRateRunner::run | 仅设计映射；未实现/未测试 |
| `adaptive_matched_filter_test/data_generation/agent_in_sphere_adaptive_matched_filter_test.m` | AdaptiveComparisonRunner::run | 仅设计映射；未实现/未测试 |
| `adaptive_matched_filter_test/data_generation/agent_in_sphere_initialization_test.m` | InitializationRunner::run | 仅设计映射；未实现/未测试 |
| `bias_test/data_analysis/graphs_bias_test.m` | BiasResultAnalyzer::aggregate | 仅设计映射；未实现/未测试 |
| `bias_test/data_generation/bias_test_nonspherical.m` | BiasExperimentRunner::runOccluded | 仅设计映射；未实现/未测试 |
| `bias_test/data_generation/bias_test_spherical_prop_error.m` | BiasExperimentRunner::runProportional | 仅设计映射；未实现/未测试 |
| `help_functions/drehvektor_aus_zwei_vektoren.m` | math::rotationVectorBetween (自由函数) | 仅设计映射；未实现/未测试 |
| `help_functions/erzeugeflussausnear.m` | FlowSynthesizer::synthesize | 仅设计映射；未实现/未测试 |
| `help_functions/erzeugekreuzproduktmatrix.m` | math::crossMatrix (自由函数) | 仅设计映射；未实现/未测试 |
| `help_functions/erzeugerichtungsvektoren_loch.m` | OccludedOctahedralSampler::sample | 仅设计映射；未实现/未测试 |
| `help_functions/erzeugerichtungsvektoren.m` | OctahedralSampler::sample | 仅设计映射；未实现/未测试 |
| `help_functions/findeDreiecke.m` | sampling::subdivideTriangle (私有自由函数) | 仅设计映射；未实现/未测试 |
| `help_functions/fixed_kugel_symmetric_perfectpoints_randomnear.m` | BiasExperimentRunner::synthesizeOccludedInput（组合采样、FlowSynthesizer、噪声策略） | 仅设计映射；未实现/未测试 |
| `help_functions/fixed_propfehler_kugel_symmetric_perfectpoints_randomnear.m` | BiasExperimentRunner::synthesizeProportionalInput（组合采样、FlowSynthesizer、噪声策略） | 仅设计映射；未实现/未测试 |
| `help_functions/franzeigen.m` | FranzMatchedFilter::estimate | 仅设计映射；未实现/未测试 |
| `help_functions/getflowfield.m` | MatResultReader::readFirstCompatible | 仅设计映射；未实现/未测试 |
| `help_functions/getnear_in_kugel_Drehung.m` | SphereEnvironment::nearness | 仅设计映射；未实现/未测试 |
| `help_functions/getvektor.m` | sampling::randomUnitDirection(IRandomSource&)（无状态自由函数） | 仅设计映射；未实现/未测试 |
| `help_functions/koeff_aus_near.m` | HarmonicFitter::fit | 仅设计映射；未实现/未测试 |
| `help_functions/kreis_trajektorie.m` | TrajectoryGenerator::circle | 仅设计映射；未实现/未测试 |
| `help_functions/kreuzprodukt.m` | math::cross (自由函数) | 仅设计映射；未实现/未测试 |
| `help_functions/kugelflfunc2.m` | RealSphericalHarmonics::evaluate | 仅设计映射；未实现/未测试 |
| `help_functions/kurveglaetten.m` | analysis::windowMean (自由函数) | 仅设计映射；未实现/未测试 |
| `help_functions/mue_norm_koenderink_iteration.m` | ModifiedKvDEstimator::estimate | 仅设计映射；未实现/未测试 |
| `help_functions/near_aus_koeff.m` | RealSphericalHarmonics::reconstruct | 仅设计映射；未实现/未测试 |
| `help_functions/nonpropfehler.m` | MeanMagnitudeGaussianNoise::apply | 仅设计映射；未实现/未测试 |
| `help_functions/norm_koenderink_iteration.m` | OriginalKvDEstimator::estimate | 仅设计映射；未实现/未测试 |
| `help_functions/normrelfehler.m` | metrics::directionAngleDeg | 仅设计映射；未实现/未测试 |
| `help_functions/oneit_adaptiv_matched_filter.m` | AdaptiveMfaStepSolver::solve | 仅设计映射；未实现/未测试 |
| `help_functions/richtungsvektoren_icosahedron.m` | IcosahedralSampler::sample | 仅设计映射；未实现/未测试 |
| `help_functions/rodrigues.m` | math::rotationVectorToMatrix，内层规划调用 Sophus::SO3d::exp()（仅实际使用的向量→矩阵分支） | 仅设计映射；未实现/未测试 |
| `help_functions/SHRotate.m` | `egomotion::harmonics::HarmonicRotator::blocks(const Eigen::Matrix3d&) const`；通过 [球谐专题 LLD §3](design/SH_ROTATION_IVANIC_RUEDENBERG.md) 的唯一声明生成 R3/R5（R0=1 不存可变成员）；按 Ivanic–Ruedenberg 1996 原文及 1998 勘误 **纯 Eigen 重写**，Don Williamson 仅供参考、禁止 sdla | 仅设计映射；未实现/未测试 |
| `help_functions/sinuskurve_trajektorie.m` | TrajectoryGenerator::sine | 仅设计映射；未实现/未测试 |
| `help_functions/skalarprodukt.m` | math::dot (自由函数) | 仅设计映射；未实现/未测试 |
| `help_functions/trajektorie_aus_punkten.m` | TrajectoryKinematics::fromPoints | 仅设计映射；未实现/未测试 |
| `help_functions/transfehler_div.m` | metrics::scaleAlignedVectorError | 仅设计映射；未实现/未测试 |
| `help_functions/transfor_koeff.m` | `egomotion::harmonics::HarmonicRotator::rotate`；接口声明见 [球谐专题 LLD §3](design/SH_ROTATION_IVANIC_RUEDENBERG.md) | 仅设计映射；未实现/未测试 |
| `help_functions/winkelausvektor.m` | math::directionToAngles | 仅设计映射；未实现/未测试 |
| `help_functions/zellendefinieren.m` | FranzMatchedFilter::axisTemplates | 仅设计映射；未实现/未测试 |
| `illustrations/circle_curve_in_the_sphere.m` | IPlotExporter::drawCircleInSphere (可选) | 仅设计映射；未实现/未测试 |
| `illustrations/sinosodial_curve_in_the_sphere.m` | IPlotExporter::drawSineInSphere (可选) | 仅设计映射；未实现/未测试 |
| `illustrations/spherical_harmonic_illustration.m` | IPlotExporter::drawHarmonic (可选) | 仅设计映射；未实现/未测试 |

## 后续需要完成但**不由本矩阵宣称已完成**

- 逐份审查 45 个文件中的全部局部/辅助可调用函数、数学分支与跨文件调用；记录源码顺序、错误/非有限分支、状态、输出、MAT 读写。当前表只覆盖每文件首要职责。
- HLD/LLD 以真实可编译 C++17 接口、Result/Status、RAII、RNG 注入、Session 状态、Writer 路径安全逐模块冻结；实际代码、CMake target 和 CTest 均不存在完整业务验收证据。
- 使用 L2 原作者 MAT（只读逐值及独立 SHA-256）、L3 独立逐节点/一般与退化输入、必要 L4 统计；不要求运行 MATLAB/Octave 或采集 L1。DEC-005/006/008、H-10/H-11 及人类签署仍按 [TODO](TODO.md) 处理。

**文件级静态核对：45/45 无缺漏；模块实现验收：未进行。**
