# R6 的 66 项 TC 与实际证据

`[AI-EDIT]`　2026-11-22 复核了受测源码 `a83682f2e493170521ec3192e9463e3b4fc132cd`。下表沿用测试计划第 3 节的 66 项 TC 及同后缀 REQ、AC。59 项可以关联到本轮实际执行的 CTest，4 项有静态资料，2 项有本机配置、编译或链接证据，1 项完整实验资源测量延期。正式验收待人类复核。[测试计划](../TEST_PLAN.md) · [需求与验收准则](../REQUIREMENTS.md)

SVG ON 的 79 项与 OFF 的 78 项均通过。先行 MAT 的 8 项在完整套件中再次执行。已将各配置 CMake 登记、JUnit 中逐项执行状态与下表所列测试名作集合核对。仅 `r06_svg_source_windows` 要求启用 SVG。运行目录为 `build/r6-paper-evidence-20261122-vO8IT7bN/02/`，具体命令、两组 JUnit、TC 核对摘要及本地 ELF 证据见运行记录。一个测试可为多条 TC 提供证据。[实施进度](../PROGRESS.md) · [CMake 测试登记和源文件](../../CMakeLists.txt)

## 逐项映射

| TC | 本轮真实 CTest 或材料 | 可核对的结果及适用范围 |
|---|---|---|
| TC-F-CLI-001 | `r01_cli_contract`、`r06_cli_process_contract`、`r06_context_journal_identity` | 真实 inspect 按变量筛选元数据，缺字段及路径身份错误可定位。 |
| TC-F-MAT-001 | `r02_mat_reader`、`r06_historical_mat_readonly_schema`、`r06_mat_independent_storage_values` | 582 个原始 MAT 的 SHA-256 和 33,340 个列主序 double 值与独立表逐一相符。 |
| TC-F-MAT-002 | `r06_dictionary_real_process`、`r06_mat_reader_boundaries`、`r06_mat73_version_rejection` | 归档枚举、形状与类型拒绝、v7.3 数值拒绝及后端故障均有实际用例。无 HDF5 的系统构建另待验证。 |
| TC-F-MAT-003 | `r02_mat_reader`、`r06_mat_raw_payload_boundaries`、`r06_mat_snapshot_identity` | 严格按名、兼容首变量、后缀、重复名及内嵌 NUL 均受测。数值读取绑定到同一份封存内容。 |
| TC-F-MAT-004 | `r05_anchored_io`、`r06_linux_writer_faults`、`r06_masked_mat_writer`、`r06_real_manifest_faults` | 沙箱写回读、字段身份、拒覆盖、掩码、持久化失败和发布确认均有断言。 |
| TC-F-GEOM-001 | `r06_sampling_flow_contract`、`r06_direction_value_invariants`、`r06_flow_value_invariants`、`r06_motion_value_invariants` | 固定方向与切向流、形状、有限性及自拥有数据通过工程检查。 |
| TC-F-GEOM-002 | `r06_directions_all_ordered_l3` | 八面体全视场和缺损视场的 0–2 级方向按源顺序逐行与独立参考比较。 |
| TC-F-GEOM-003 | `r06_directions_all_ordered_l3` | 二十面体 0–2 级的方向数、三分量和行序有独立参考。 |
| TC-F-FLOW-001 | `r06_sampling_flow_contract`、`r06_paper_figure_contract` | 固定平移与旋转的切向流、合成均模长，以及六方向理想流比 1/2。 |
| TC-F-FLOW-002 | `r06_sampling_flow_contract`、`r06_bias_runner_contract` | 固定 uniform 输入检验距离范围、除以 2.5、近度及末项复用。完整 Bias 统计延期。 |
| TC-F-FLOW-003 | `r06_sampling_flow_contract`、`r06_bias_dependency_injection`、`r06_noisy_session_independent_multiframe` | 分别验证两种噪声的消费次序、切平面投影及含噪多帧结果。高斯分布的 L4 资格待验。 |
| TC-F-FLOW-004 | `r06_trajectory_sphere_geometry`、`r06_adaptive_scene_composition` | 球心、一般位置、射线正根及非法交点的解析或故障检查。 |
| TC-F-FILTER-001 | `r06_kvd_independent_l3`、`r06_bias_runner_contract` | Franz 的固定视线模板响应、分母和返回运动与独立参考比较。 |
| TC-F-KVD-001 | `r06_kvd_independent_l3`、`r06_kvd_strategy_composition` | Original 和 Modified 的固定逐轮轨迹、符号翻转、策略选择与更新顺序。 |
| TC-F-KVD-002 | `r06_kvd_stopping_reference` | 最少 6 轮、最多 200 轮、动态与绝对阈值及非有限结果的分支检查。 |
| TC-F-ERROR-001 | `r06_source_motion_errors`、`r06_session_metric_validity`、`r06_paper_figure_contract` | 尺度对齐弦长、稳定方向角、零旋转与 SO(3) 测地角分别有解析或固定输入。历史 Bias 单位仍待来源核实。 |
| TC-F-BIAS-001 | `r06_bias_runner_contract`、`r06_bias_dependency_injection`、`r06_bias_runner_publication` | 遮挡视场三噪声列、求解顺序及有限规模 MAT 发布。两种场景各 40 次的新科学试验延期。 |
| TC-F-BIAS-002 | `r06_bias_runner_contract`、`r06_bias_dependency_injection`、`r06_paper_figure_contract` | 完整视场逐分量噪声、真实方向数与源 xwerte 分别计数。专项使用合成试次检验聚合。 |
| TC-F-BIAS-003 | `r06_bias_archive_independent_curves`、`r06_cli_process_archive`、`r06_paper_figure_contract` | 历史两组各 40 份的 26 条曲线、208 对 x/y 独立重算。专项固定 40 分母，缺一试次的负例受测。 |
| TC-F-TRAJ-001 | `r06_trajectory_sphere_geometry`、`r06_illustrations_independent_geometry`、`r06_adaptive_real_cli` | 逐点公式、端点、局部运动与原轨迹工程入口。轨迹插图每幅 600 点，初始化使用 35 点。 |
| TC-F-SH-001 | `low_order_harmonics`、`low_order_harmonics_archived_golden`、`low_order_harmonics_source_projection`、`r06_harmonic_service_contract`、`r06_source_spherical_angles` | 低阶九基、3×5 系数、拟合重建、源极角约定和独立投影。DEC-005 待审。 |
| TC-F-SH-002 | `r06_sh_argument_action`、`r06_sh_source_phase_reference`、`r06_sh_source_session_action` | R0/R3/R5、m 与相位次序、作用方向及其 Session 路径在固定输入下逐项检查。 |
| TC-F-ADAPT-001 | `r06_adaptive_step_independent_l3`、`r06_adaptive_conditioning_reference`、`r06_adaptive_real_step_delivery_l3` | 单步与合成完整交付的逐节点参考。窄锥前向误差单列诊断，未取得一般病态输入的数值资格。 |
| TC-F-ADAPT-002 | `r06_adaptive_session_transaction`、`r06_session_independent_multiframe`、`r06_session_metric_validity`、`r06_adaptive_entry_isolation` | 系数先旋转再求解，状态隔离。指标缺测可继续，求解或策略失败回滚。 |
| TC-F-ADAPT-003 | `r06_adaptive_periods_atomic_commit`、`r06_coefficient_policy_composition`、`r06_paper_figure_contract` | 归档 1/5/11/21 刷新时刻、对照状态和弃置分支有检查。论文周期另有专项配置与合成用例。 |
| TC-F-EXP-001 | `r06_adaptive_real_cli`、`r06_adaptive_real_step_delivery_l3` | 原 35 点初始化有 33 个有效内帧，9 个 MAT（四个数值、四个计数、一个指标标记）。平移值与历史 L2 对照，历史旋转轴角和新测地角分列。 |
| TC-F-EXP-002 | `r06_adaptive_real_cli`、`r06_adaptive_aggregation_evidence`、`r06_partial_adaptive_output`、`r06_adaptive_real_step_delivery_l3`、`r06_paper_figure_contract` | 原轨迹零噪声 8×600 点，4,784 个有效内帧、11 个 MAT。第 5 轮合成故障的实测计数为尝试 5、完成 4、失败 1、未开始 3。仅七轮有效时，八轮均值按缺测登记。原轨迹 0.1 噪声的全规模比较延期。 |
| TC-F-EXP-003 | `r06_adaptive_real_cli`、`r06_adaptive_periods_atomic_commit`、`r06_partial_analysis` | 归档周期的原 600 点轨迹走完 598 个内帧，7 个 MAT。缺少分析所需的历史流比时返回明确错误。论文周期全量对照延期。 |
| TC-F-AN-001 | `r05_analysis_source_contract`、`r06_analysis_segments_smoothing`、`r06_partial_analysis`、`r06_analysis_finite_cli` | 初始化裁剪、50–550 窗口、阈值、分段平滑和声明缺测掩码均有检查。 |
| TC-F-AN-002 | `r06_analyzer_service_contract`、`r06_illustrations_independent_geometry`、`r06_illustrations_real_cli`、`r06_paper_figure_contract` | 分析与导出隔离。Fig. 3/4 的四个面板都有几何数据，实际 CLI 检查两种格式、论文视角与关闭 SVG 时的拒绝路径。 |
| TC-D-SOURCE-001 | [源文件清单](../SOURCE_INVENTORY.md)、原始 Git 树 | Git 子树与来源清单列出 45 个 .m、582 个 MAT 和 1 份 readme。正式来源基线待签。 |
| TC-D-SOURCE-002 | [来源清单](../SOURCE_INVENTORY.md)、[移植矩阵](../MIGRATION_MATRIX.md) | 源码清单、移植矩阵及引用可追溯至原论文、归档和球谐勘误。人工来源复核仍独立。 |
| TC-D-MAT-001 | `r06_dictionary_real_process`、`r06_dictionary_output_streams`、`r06_mat_independent_storage_values` | 实际枚举、解码、字典输出故障及归档逐文件 SHA。P10.4 的正式复核待办。 |
| TC-D-MAT-002 | `r06_mat_independent_storage_values`、`r05_anchored_io`、`r06_masked_mat_writer` | 首版支持范围内的形状、列序、载荷和字段命名。显式掩码的往返值受测。 |
| TC-D-MAT-003 | `r06_historical_mat_readonly_schema`、`r06_partial_analysis`、`r06_adaptive_real_cli` | 历史额外字段只读。现行生成字段、缺少 rottransratio 的错误及指标版本可辨。 |
| TC-DC-DOMAIN-001 | `r01_contracts`、`r06_result_lifetimes_error_frames`、`r06_flow_value_invariants`、`r06_coefficient_value_invariants`、`r06_direction_value_invariants`、`r06_motion_value_invariants` | Result 生命周期、错误链及四类领域值的形状、有限性和所有权。 |
| TC-DC-IO-001 | `r06_mat_snapshot_identity`、`r06_linux_writer_faults`、`r06_io_symlink_and_runid_safety` | 封存快照、FD 释放、写入故障与路径身份。 |
| TC-DC-RNG-001 | `r06_sampling_flow_contract`、`r06_bias_dependency_injection`、`r06_run_context_interleaving` | 固定序列注入、重置和耗尽，以及两个 RunContext 的随机流隔离。 |
| TC-DC-NOISE-001 | `r06_sampling_flow_contract`、`r06_adaptive_dependency_injection` | 采样和噪声替身可注入。零强度仍按源次序抽样，错误保留上下文。 |
| TC-DC-KVD-001 | `r06_kvd_strategy_composition`、`r06_kvd_independent_l3` | 两个具名估计器共享迭代框架，各自的平移更新有独立参考和替换测试。 |
| TC-DC-ADAPT-001 | `r06_adaptive_step_independent_l3`、`r06_real_session_interleaving`、`r06_coefficient_policy_composition` | 无状态 Step、Session 状态与系数策略分开核对，交错调用有独立状态。 |
| TC-DC-EXPERIMENT-001 | `r06_bias_dependency_injection`、`r06_adaptive_dependency_injection`、`r06_adaptive_scene_composition`、`r06_adaptive_entry_isolation` | Bias 与三个 Adaptive 入口分别检验场景和求解依赖、跨调用隔离、失败后复用。 |
| TC-DC-ANALYSIS-001 | `r06_analyzer_service_contract`、`r06_partial_analysis`、`r06_illustrations_real_cli` | Reader、Analyzer、导出独立。数据模式与可选 SVG 的几何载荷逐字节比较。 |
| TC-NF-PLATFORM-001 | 两套 CMake 配置、编译命令与工具链 | Ubuntu 24.04.5、G++ 16.2.0、C++17 的两套源外 Release 构建。其余发行版和编译器待测。 |
| TC-NF-DEPS-001 | 两套 CLI 的链接命令与 ELF NEEDED | 两套生成的链接规则及 CLI 的 ELF 依赖可核。依赖边界限定于该环境和已构建目标。 |
| TC-NF-SAFETY-001 | `r06_io_symlink_and_runid_safety`、`r06_mat_snapshot_identity`、`r06_real_manifest_faults`、`r06_cli_process_archive` | 原输入运行前后哈希一致，中文路径、拒覆盖、符号链接与持久化失败有回归。 |
| TC-NF-NUM-001 | `r06_kvd_independent_l3`、`r06_adaptive_step_independent_l3`、`r06_session_independent_multiframe`、`r06_adaptive_real_step_delivery_l3` | L2 固定输入与独立 L3 参考已有分模块结果。L4 和正式 DEC-005 门槛待审。 |
| TC-NF-NUM-002 | `r06_source_motion_errors`、`r06_adaptive_conditioning_reference`、`r06_adaptive_zero_rotation_and_solver_failure`、`r06_mat_raw_payload_boundaries` | 真求解失败保留错误。零旋转的轴无定义而测地角可定义。病态样本的前向误差单列。 |
| TC-PROC-TRACE-001 | [测试计划](../TEST_PLAN.md)、[SRS](../REQUIREMENTS.md)、[98 项任务复核](R6_PLAN_TASK_AUDIT_2026-11-18.md) | 对照测试计划和 SRS 逐项复核本表 66 个 TC→REQ/AC 编号，98 项 WBS 与 45 个源文件另有可查索引。正式基线待签。 |
| TC-NF-RNG-001 | `r06_noisy_session_independent_multiframe`、`r06_adaptive_real_cli`、`r06_run_context_interleaving` | 固定注入序列和每个真实进程算法种子分别记录。完整 L4 样本总体延期。 |
| TC-NF-LOG-001 | `r06_execution_record_contract`、`r06_context_journal_identity`、`r06_process_records_lifecycle`、`r06_process_records_real_cli`、`r06_runid_failure_context` | 实际参数、RunId、SHA、输入身份、通道与退出码。故障注入核对开始、结束和输出错误链。 |
| TC-NF-OOP-001 | `r01_contracts`、`r06_result_lifetimes_error_frames`、`r06_kvd_strategy_composition`、`r06_analyzer_service_contract` | 所有权、策略替换、错误寿命与接口隔离有回归。对象模型和公共接口冻结待审。 |
| TC-NF-HEADLESS-001 | `r06_analysis_finite_cli`、`r06_illustrations_real_cli`、`r06_cli_process_contract` | 两种模式都清除显示服务变量。OFF 数值导出成功，SVG 拒绝时无输出目录。 |
| TC-NF-RECOVERY-001 | `r05_manifest_gate`、`r06_real_manifest_faults`、`r06_cli_output_streams`、`r06_cli_output_real_process`、`r06_process_records_lifecycle`、`r06_process_records_real_cli` | 失败 Manifest、结束记录失败、输出通道错误、重复 RunId 与子进程终止。掉电后最终记录仍有边界。 |
| TC-NF-PORTABLE-001 | `r06_io_symlink_and_runid_safety`、`r06_illustrations_real_cli`、`r06_adaptive_real_cli` | 本次 Ubuntu 实际在含中文和空格的路径中读写。 |
| TC-NF-RUNTIME-001 | 延期 | 完整科学实验的 wall、CPU 和峰值 RSS 待测。DEC-006 预算待人类批准。 |
| TC-NF-SESSION-001 | `r06_real_session_interleaving`、`r06_run_context_interleaving`、`r06_adaptive_session_transaction` | 会话及 RunContext 交替、不同实例的 RNG、系数、输出隔离。范围为串行交错。 |
| TC-NF-COPYRIGHT-001 | [MIT 许可](../../LICENSE)、[来源清单](../SOURCE_INVENTORY.md) | 仓库根 MIT、自有范围和实际上游来源告知有记录。 |
| TC-F-CLI-002 | `r06_cli_process_contract`、`r06_execution_record_contract`、`r06_bias_runner_publication` | 两个 Bias 的真实入口、有界生成、参数与种子、RunId 和拒覆盖。 |
| TC-F-CLI-003 | `r06_adaptive_real_cli`、`r06_adaptive_runner_complete_contract`、`r06_adaptive_runner_incomplete_contract`、`r06_adaptive_zero_rotation_and_solver_failure` | 三个具名原轨迹入口实测交付，新指标和失败回滚分别核验。含噪全实验延期。 |
| TC-F-CLI-004 | `r06_cli_process_archive`、`r06_analysis_finite_cli`、`r06_partial_analysis` | 四种分析入口实测。历史裸 NaN 按合同拒绝，有版本且有计数的缺测另走显式分支。 |
| TC-F-CLI-005 | `r06_svg_source_windows`（仅 ON）、`r06_illustrations_real_cli`、`r06_paper_figure_contract` | 专用 SVG 窗口测试仅在 ON 注册。两种配置均覆盖真实 CLI 和论文绘图合同。 |
| TC-I-CLI-001 | `r01_cli_contract`、`r06_cli_process_contract`、`r06_runid_failure_context`、`r06_paper_figure_contract` | 帮助、参数、错误通道、拒绝自定义输出根及 Fig. 5 必需选项。DEC-008 待冻结。 |
| TC-I-MAT-001 | `r02_mat_reader`、`r06_mat_reader_boundaries`、`r06_mat_snapshot_identity`、`r06_masked_mat_writer` | 严格与兼容读、元数据、变量选择、明确类型拒绝及隔离写回读。 |
| TC-I-RESULT-001 | `r06_adaptive_real_step_delivery_l3`、`r06_bias_runner_publication`、`r06_adaptive_real_cli`、`r06_masked_mat_writer` | 合成逐值参考及实际生成结果分别核对字段、形状、存储身份、掩码和根目录。 |
| TC-I-ANALYSIS-001 | `r06_cli_process_archive`、`r06_bias_archive_independent_curves`、`r06_partial_analysis`、`r06_analyzer_service_contract` | 实际 MAT 与有限合成输入的曲线、索引、掩码及来源可追溯。保留历史文件和原有缺字段错误。 |

## 尚待执行与审查

Fig. 1 的新 40 次双场景实验、Fig. 5 的含噪比较及两套更新周期的全规模论文对照、L4 统计、DEC-005/006/008 及正式验收继续按计划办理。工程测试的三项原轨迹入口产生了 Complete 记录，实际有效内帧数及指标定义见本轮进度。

## 2026-11-18 历史测试组及证据入口

以下索引保留各项实测首次登记时的源码与执行状态。该表中的未运行和失败描述仅适用于所链接的历史提交。

| 索引 | 实际CTest（完整名称） | 源码/证据 |
|---|---|---|
| FLOW-VALUE | `r06_flow_value_invariants` | `tests/contracts/test_r06_flow_value.cpp`；1245项30有限组合/8形状/126非有限位置/寿命/3分配故障/移交编译约束，见 [证据](R6_FLOW_VALUE_2026-11-18.md)；不新增物理门槛 |
| DIR-VALUE | `r06_direction_value_invariants` | `tests/contracts/test_r06_direction_value.cpp`；901项30有限组合/8形状/63非有限分量/值寿命/3分配故障与移交编译约束，见 [证据](R6_DIRECTION_VALUE_2026-11-18.md)；不新增单位容差 |
| COEFF-VALUE | `r06_coefficient_value_invariants` | `tests/contracts/test_r06_coefficient_value.cpp`；178项6组有限存储/8形状负例/45非有限分量/逐位保存和寿命及编译约束，见 [证据](R6_COEFFICIENT_VALUE_2026-11-18.md)；有限填充允许，不证明近度有效 |
| MOTION-VALUE | `r06_motion_value_invariants` | `tests/contracts/test_r06_motion_value.cpp`；839项49有限输入对/16形状负例/18非有限分量/值复制移动寿命及编译只读约束，见 [证据](R6_MOTION_VALUE_2026-11-18.md)；不归一化/不判可观测性，不覆盖其他领域值 |
| MAT73 | `r06_mat73_version_rejection` | `tests/integration/test_r06_mat73.cpp`及固定合成v7.3夹具；32场景/308项，3入口在Mat_Open之前拒绝、EINTR/短读/EIO/后端替身/描述符/输入身份，见 [版本证据](R6_MAT73_REJECTION_2026-11-18.md)。无HDF5库实际构建未验证 |
| ADAPT-ENTRY | `r06_adaptive_entry_isolation` | `tests/integration/test_r06_adaptive_scene.cpp`的entry-isolation分支；172962项三具名Runner六次交错、12错误Context、非法Config及第五轮失败后同Runner复用，见 [证据](R6_ADAPTIVE_ENTRIES_2026-11-18.md)；固定Step及内存发布只证明组合合同 |
| ADAPT-SCENE | `r06_adaptive_scene_composition` | `tests/integration/test_r06_adaptive_scene.cpp`；88121项三入口完整合成场景、Comparison八轮聚合/状态重置、54服务故障及第5轮失败前缀，见 [证据](R6_ADAPTIVE_SCENE_2026-11-18.md)；Step及Writer/Publisher为替身，不证明真实科学实验或MAT持久化 |
| ADAPT-INJECT | `r06_adaptive_dependency_injection` | `tests/integration/test_r06_adaptive_injection.cpp`；原36088项路由/重放、24服务故障及三生产默认/显式装配，见 [证据](R6_ADAPTIVE_INJECTION_2026-11-18.md)；新增三入口方向构造错误行索引断言后为36091项，见 [本轮](R6_DIRECTION_VALUE_2026-11-18.md)；固定Step只作合同替身；本轮非有限噪声改在工厂拒绝，保留Runner行号且总检查仍36091项，见 [接续](R6_FLOW_VALUE_2026-11-18.md) |
| BIAS-INJECT | `r06_bias_dependency_injection` | `tests/integration/test_r06_bias_injection.cpp`；668项两模式RNG/求解顺序、精确失败前缀、交错隔离及内存交付检查，见 [注入证据](R6_BIAS_INJECTION_2026-11-18.md)。替身不充当真实MAT或独立求解Golden |
| CLI | `r01_cli_contract`, `r06_cli_process_contract`, `r06_cli_process_archive` | `tests/integration/test_r01_cli.cpp`, `test_r06_cli_process.cmake`；[CLI实测](R6_CLI_PROCESS_VALIDATION_2026-11-17.md) |
| HEADLESS | 两种独立配置的完整CTest，OFF仅不登记`r06_svg_source_windows` | 实际无显示环境/后端缺席CLI、完整数值参考保留；14份有限MAT载荷和3份JSON跨模式相同，CMake/ELF/依赖审计见 [实测](R6_HEADLESS_BUILD_2026-11-17.md) |
| RUNID | `r06_runid_failure_context`（并扩展Manifest、Bias/Adaptive库及实际CLI用例） | `tests/integration/test_r06_runid_diagnostics.cpp`；63项检查/8次装配调用、真实预留与失败Manifest；[身份实测](R6_RUNID_DIAGNOSTICS_2026-11-17.md) |
| STREAM | `r06_cli_output_streams`, `r06_cli_output_real_process` | `tests/integration/test_r06_cli_streams.cpp/.cmake`；每模式现856项/123次调用、11次真实`/dev/full`及默认SIGPIPE后失败审计保存；[原实测](R6_CLI_STREAM_OUTPUT_2026-11-17.md)、[运行记录增补](R6_EXECUTION_RECORDS_2026-11-17.md) |
| EXECUTION | `r06_execution_record_contract`（另扩展CLI、ADAPTIVE-CLI、STREAM及ILLUSTRATION） | `tests/integration/test_r06_execution_records.cpp`, `execution_record_checks.cmake`, `artifact_record_checks.cmake`；每模式394项/38次调用，五生成入口及plot/SVG的真实参数/环境/输入身份/交付状态，独立JSON解析及发布/持久化/流故障，见 [导出记录实测](R6_ARTIFACT_RECORDS_2026-11-18.md) |
| PROCESS-RECORD | `r06_process_records_lifecycle`, `r06_process_records_real_cli` | 各464项/26次受监控调用、29次真实CLI；原通道/信号/权限边界及新上下文返回确认，见 [最新证据](R6_CONTEXT_JOURNAL_2026-11-18.md) |
| CONTEXT | `r06_context_journal_identity`（另扩展PROCESS-RECORD） | `tests/integration/test_r06_context_journal.cpp`；224项日志短写/失败、MAT路径/inode竞态、只读元数据、阻止下游及实际RunId错误身份；[证据](R6_CONTEXT_JOURNAL_2026-11-18.md) |
| DICTIONARY | `r06_dictionary_output_streams`, `r06_dictionary_real_process` | 每模式165项/40次入口、17次真实进程；582原MAT、状态/报告write/close故障、历史参数/报告与输入保护；[实测](R6_DICTIONARY_OUTPUT_2026-11-17.md) |
| ADAPTIVE-CLI | `r06_adaptive_real_cli` | `tests/integration/test_r06_adaptive_cli.cmake`、`r06_adaptive_init_trace.cpp`；6次实际CLI和1次真实Step记录进程，三种生成均Incomplete，无MAT，见 [实测](R6_ADAPTIVE_CLI_2026-11-17.md)。`r06_adaptive_cli_values.cpp`仅编译，成功产物/L2分支未执行 |
| MAT | `r02_mat_reader`, `r06_historical_mat_readonly_schema`, `r06_mat_independent_storage_values`, `r06_mat_reader_boundaries`, `r06_mat_raw_payload_boundaries` | `tests/integration/test_r02_reader.cpp`, `test_r06_archive_readonly.cpp`, `test_r06_mat_independent.cpp`, `test_r06_reader_negative.cpp`, `test_r06_mat_payload.cpp`；[L2](R6_MAT_STORAGE_VALIDATION_2026-11-17.md)、[边界](R6_READER_BOUNDARIES_2026-11-17.md)、[载荷](R6_MAT_PAYLOAD_VALIDATION_2026-11-17.md) |
| SNAPSHOT | `r06_mat_snapshot_identity`（另扩展MAT、ANALYSIS、CLI的输入SHA检查） | `tests/integration/test_r06_mat_snapshot.cpp`；256检查/30次完整读取及1次metadata；582固定SHA、986贡献摘要及32条有限曲线JSON，见 [封存实测](R6_MAT_SNAPSHOT_2026-11-17.md) |
| IO | `r05_anchored_io`, `r06_io_symlink_and_runid_safety`, `r06_linux_writer_faults`, `r06_real_manifest_faults` | `tests/integration/test_r05_io.cpp`, `test_r06_io_safety.cpp`, `test_r06_io_faults.cpp`；[发布故障](R6_IO_FAULT_VALIDATION_2026-11-17.md) |
| CONTRACT | `r01_contracts`, `r05_manifest_gate`, `r06_result_lifetimes_error_frames` | `tests/contracts/test_r01_contracts.cpp`, `test_r05_manifest.cpp`, `test_r06_result_lifetimes.cpp`；[原生记录](R6_NATIVE_VALIDATION_2026-11-17.md) |
| GEOM | `r06_sampling_flow_contract`, `r06_directions_all_ordered_l3`, `r06_trajectory_sphere_geometry` | `tests/integration/test_r06_sampling_flow.cpp`, `test_r06_direction_rows_l3.cpp`, `test_r06_trajectory_sphere.cpp`；`verification/geometry/` |
| KVD | `r06_kvd_independent_l3`, `r06_kvd_stopping_reference`, `r06_kvd_strategy_composition` | `tests/integration/test_r06_kvd_l3.cpp`, `test_r06_kvd_stopping.cpp`, `test_r06_kvd_strategy.cpp`；`verification/kvd/`；原逐节点参考及506次独立解析控制流参考见 [停止分支](R6_MOTION_STOPPING_2026-11-17.md)，新增2712项标量/策略替换/顺序/故障/隔离检查见 [策略组合](R6_KVD_STRATEGY_2026-11-18.md) |
| METRIC | `r06_source_motion_errors` | `tests/integration/test_r06_motion_errors.cpp`；直接调用两个生产内部实现，解析几何/尺度/弦长恒等式/零/非有限/上溢/下溢，含acos双端舍入越界共703项；[基础实测](R6_MOTION_STOPPING_2026-11-17.md)、[端点补充](R6_INTERLEAVING_2026-11-17.md#最终复测) |
| STEP | `r06_adaptive_step_independent_l3`, `r06_adaptive_conditioning_reference` | `tests/integration/test_r06_adaptive_l3.cpp`, `test_r06_adaptive_conditioning.cpp`；正/负九方向完整单步，另12个解析运动阶段/完整投影拒绝及4个定义域负例，见 [conditioning实测](R6_ADAPTIVE_CONDITIONING_2026-11-17.md) |
| SH | `low_order_harmonics`, `low_order_harmonics_archived_golden`, `low_order_harmonics_source_projection`, `r06_sh_argument_action`, `r06_sh_source_phase_reference`, `r06_sh_source_session_action`, `r06_source_spherical_angles`, `r06_harmonic_service_contract` | `tests/unit/test_low_order*.cpp`, `tests/integration/test_r06_sh_adapter.cpp`, `test_r06_source_sh.cpp`；[源相位作用](R6_SH_SOURCE_ACTION_2026-11-17.md)、`tests/unit/test_r06_source_angles.cpp`及 [角度接口](R6_SOURCE_ANGLES_2026-11-18.md)；`tests/integration/test_r06_harmonic_services.cpp`的原482项三组件独立参考/边界见 [职责分离](R6_HARMONIC_SERVICES_2026-11-18.md)；系数非法输入迁到工厂/原始叶子后为484项，见 [更新](R6_COEFFICIENT_VALUE_2026-11-18.md)；本轮方向入口/前缀边界后494项，见 [流场接续](R6_FLOW_VALUE_2026-11-18.md) |
| SESSION | `r06_adaptive_session_transaction`, `r06_adaptive_periods_atomic_commit`, `r06_real_session_interleaving`, `r06_coefficient_policy_composition` | `tests/contracts/test_r06_session_contract.cpp`, `test_r06_adaptive_periods.cpp`, `tests/integration/test_r06_session_interleaving.cpp`；三模式原子性/1/5/11/21刷新，真实单步交错9579项见 [隔离实测](R6_INTERLEAVING_2026-11-17.md)；新增`tests/contracts/test_r06_coefficient_policies.cpp`的策略组合/失败/重试48523项见 [策略实测](R6_COEFFICIENT_POLICY_2026-11-18.md) |
| SESSION-L3 | `r06_session_independent_multiframe` | `tests/integration/test_r06_session_l3.cpp`与标准库标量参考；299236项/252帧/24回滚，真实Step逐节点及演进状态独立对照，见 [证据](R6_SESSION_MULTIFRAME_L3_2026-11-18.md)；不替代原轨迹/完整addederror/八轮实验 |
| NOISY-SESSION | `r06_noisy_session_independent_multiframe` | 同一测试的`--noise`分支与独立标量噪声参考，290923项/252帧/9072固定normal/24回滚；[证据](R6_NOISY_SESSION_L3_2026-11-18.md)，不是原轨迹/L4或完整8轮 |
| DELIVERY-L3 | `r06_adaptive_real_step_delivery_l3` | `tests/integration/test_r06_adaptive_delivery_l3.cpp`；3676721项/17441真实Step/12MAT/8540值，合成35/600帧及8轮独立数值交付，见 [证据](R6_REAL_STEP_DELIVERY_L3_2026-11-18.md)；非原场景科学成功 |
| RUNNER | `r06_adaptive_runner_complete_contract`, `r06_adaptive_runner_incomplete_contract`, `r06_adaptive_default_undefined_contract`, `r06_bias_runner_contract`, `r06_bias_runner_publication`, `r06_run_context_interleaving` | `tests/integration/test_r06_adaptive_runner.cpp`, `test_r06_bias_runner.cpp`, `test_r06_run_context_interleaving.cpp`；[Adaptive](R6_ADAPTIVE_RUNNER_CONTRACT_2026-11-17.md)、[Bias](R6_BIAS_RUNNER_VALIDATION_2026-11-17.md)、[RunContext隔离](R6_INTERLEAVING_2026-11-17.md)。Adaptive Runner含单步替身；隔离fixture使用真实RNG/MAT/Manifest，均不等于完整真实求解实验 |
| ANALYZER-SERVICE | `r06_analyzer_service_contract` | `tests/integration/test_r06_analyzer_services.cpp`；727项四入口两次交替/40读取故障/自拥有结果/导出失败不改值，见 [职责分离](R6_ANALYZER_SERVICES_2026-11-18.md)；内存Reader/Exporter不冒称真实MAT或独立全数值参考 |
| ANALYSIS | `r05_analysis_source_contract`, `r06_analysis_segments_smoothing`, `r06_analysis_finite_cli`, `r06_bias_archive_independent_curves`（另复用CLI archive） | `tests/integration/test_r05_analysis.cpp`, `test_r06_analysis_segments.cpp`, `test_r06_analysis_finite.cmake`, `r06_analysis_fixture.cpp`, `test_r06_bias_archive.cpp`；[有限输入](R6_ANALYSIS_FINITE_VALIDATION_2026-11-17.md)、[历史Bias及辅助来源](R6_BIAS_ARCHIVE_ANALYSIS_2026-11-17.md) |
| SVG | `r06_svg_source_windows` | `tests/integration/test_r06_svg_windows.cpp`；[视窗实测](R6_SVG_WINDOWS_2026-11-17.md)，状态以该报告为准 |
| ILLUSTRATION | `r06_illustrations_independent_geometry`, `r06_illustrations_real_cli` | `tests/integration/test_r06_illustrations.cpp`, `test_r06_illustrations_cli.cmake`；536225项独立Rodrigues/轨迹/遮罩/画幅检查、每模式29次真实CLI；[原插图与目视](R6_ILLUSTRATIONS_2026-11-17.md)、[新增导出记录及负order拒绝](R6_ARTIFACT_RECORDS_2026-11-18.md) |
| SOURCE | 无独立CTest代表全部人工来源验收 | `SOURCE_INVENTORY.md`, `MIGRATION_MATRIX.md`, `MAT_DATA_SCHEMA.md`, `verification/mat/archive_storage_identity.v1.tsv`, `LICENSE`及原始Git树；[45源现行映射复核](R6_SOURCE_IMPLEMENTATION_MAP_2026-11-17.md)已完成，非全部数学分支/正式验收 |
| ENV | 配置/构建及CLI实测，不是跨发行版资格测试 | [原生环境](R6_NATIVE_VALIDATION_2026-11-17.md)及后续每轮 `environment.txt`、CMake链接图；实际仅Ubuntu24.04.5/GCC16.2.0 |
