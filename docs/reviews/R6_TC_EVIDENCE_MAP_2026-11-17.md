# R6：66项 TC 的实际证据映射与剩余工作

`[AI-EDIT]` 事实索引，基于当前Git树、CMake登记、测试源码及实际日志。保留 [TEST_PLAN §3](../TEST_PLAN.md) 的66个TC/REQ/AC和 [PLAN](../PLAN.md) 的98项任务，不替换候选规范、改动阈值或签署验收。下表每个TC仅列一次；其REQ/AC采用相同后缀，正式义务仍以 [REQUIREMENTS](../REQUIREMENTS.md) 为准。

**没有任何一行仅凭CTest数量被宣布正式验收。** “相关实测”只表示下面具体程序/样本已有运行证据。最新受测源码 `0cd5439b7195cd6b7355deb5f7eb951a649be8b1` 为SVG ON59/59、OFF58/58，见 [Adaptive注入证据](R6_ADAPTIVE_INJECTION_2026-11-18.md)；此前各专项回归保留。Issue #1/#2/#3和旧失败日志保持开放供复核。真实Adaptive三入口仍失败、无完整MAT。正式数值/环境/基线资格独立办理，不以尚未签署为理由停止已授权的源码/测试补足。

## 实际测试组及证据入口

以下是便于表格引用的索引，不是新的CTest或TC编号。真实目标/源路径见根 [CMakeLists](../../CMakeLists.txt)；各报告固定SHA、命令、环境、日志和边界，历史提交不能自动覆盖新源码。

| 索引 | 实际CTest（完整名称） | 源码/证据 |
|---|---|---|
| MAT73 | `r06_mat73_version_rejection` | `tests/integration/test_r06_mat73.cpp`及固定合成v7.3夹具；32场景/308项，3入口在Mat_Open之前拒绝、EINTR/短读/EIO/后端替身/描述符/输入身份，见 [版本证据](R6_MAT73_REJECTION_2026-11-18.md)。无HDF5库实际构建未验证 |
| ADAPT-INJECT | `r06_adaptive_dependency_injection` | `tests/integration/test_r06_adaptive_injection.cpp`；36088项路由/重放、24服务故障及三生产默认/显式装配，见 [证据](R6_ADAPTIVE_INJECTION_2026-11-18.md)；固定Step只作合同替身 |
| BIAS-INJECT | `r06_bias_dependency_injection` | `tests/integration/test_r06_bias_injection.cpp`；668项两模式RNG/求解顺序、精确失败前缀、交错隔离及内存交付检查，见 [注入证据](R6_BIAS_INJECTION_2026-11-18.md)。替身不充当真实MAT或独立求解Golden |
| CLI | `r01_cli_contract`, `r06_cli_process_contract`, `r06_cli_process_archive` | `tests/integration/test_r01_cli.cpp`, `test_r06_cli_process.cmake`；[CLI实测](R6_CLI_PROCESS_VALIDATION_2026-11-17.md) |
| HEADLESS | 两种独立配置的完整CTest，OFF仅不登记`r06_svg_source_windows` | 实际无显示环境/后端缺席CLI、完整数值参考保留；14份有限MAT载荷和3份JSON跨模式相同，CMake/ELF/依赖审计见 [实测](R6_HEADLESS_BUILD_2026-11-17.md) |
| RUNID | `r06_runid_failure_context`（并扩展Manifest、Bias/Adaptive库及实际CLI用例） | `tests/integration/test_r06_runid_diagnostics.cpp`；63项检查/8次装配调用、真实预留与失败Manifest；[身份实测](R6_RUNID_DIAGNOSTICS_2026-11-17.md) |
| STREAM | `r06_cli_output_streams`, `r06_cli_output_real_process` | `tests/integration/test_r06_cli_streams.cpp/.cmake`；每模式现856项/123次调用、11次真实`/dev/full`及默认SIGPIPE后失败审计保存；[原实测](R6_CLI_STREAM_OUTPUT_2026-11-17.md)、[运行记录增补](R6_EXECUTION_RECORDS_2026-11-17.md) |
| EXECUTION | `r06_execution_record_contract`（另扩展CLI、ADAPTIVE-CLI、STREAM及ILLUSTRATION） | `tests/integration/test_r06_execution_records.cpp`, `execution_record_checks.cmake`, `artifact_record_checks.cmake`；每模式394项/38次调用，五生成入口及plot/SVG的真实参数/环境/输入身份/交付状态，独立JSON解析及发布/持久化/流故障，见 [导出记录实测](R6_ARTIFACT_RECORDS_2026-11-18.md) |
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
| SH | `low_order_harmonics`, `low_order_harmonics_archived_golden`, `low_order_harmonics_source_projection`, `r06_sh_argument_action`, `r06_sh_source_phase_reference`, `r06_sh_source_session_action`, `r06_source_spherical_angles` | `tests/unit/test_low_order*.cpp`, `tests/integration/test_r06_sh_adapter.cpp`, `test_r06_source_sh.cpp`；[源相位作用](R6_SH_SOURCE_ACTION_2026-11-17.md)、`tests/unit/test_r06_source_angles.cpp`及 [角度接口](R6_SOURCE_ANGLES_2026-11-18.md) |
| SESSION | `r06_adaptive_session_transaction`, `r06_adaptive_periods_atomic_commit`, `r06_real_session_interleaving`, `r06_coefficient_policy_composition` | `tests/contracts/test_r06_session_contract.cpp`, `test_r06_adaptive_periods.cpp`, `tests/integration/test_r06_session_interleaving.cpp`；三模式原子性/1/5/11/21刷新，真实单步交错9579项见 [隔离实测](R6_INTERLEAVING_2026-11-17.md)；新增`tests/contracts/test_r06_coefficient_policies.cpp`的策略组合/失败/重试48523项见 [策略实测](R6_COEFFICIENT_POLICY_2026-11-18.md) |
| RUNNER | `r06_adaptive_runner_complete_contract`, `r06_adaptive_runner_incomplete_contract`, `r06_adaptive_default_undefined_contract`, `r06_bias_runner_contract`, `r06_bias_runner_publication`, `r06_run_context_interleaving` | `tests/integration/test_r06_adaptive_runner.cpp`, `test_r06_bias_runner.cpp`, `test_r06_run_context_interleaving.cpp`；[Adaptive](R6_ADAPTIVE_RUNNER_CONTRACT_2026-11-17.md)、[Bias](R6_BIAS_RUNNER_VALIDATION_2026-11-17.md)、[RunContext隔离](R6_INTERLEAVING_2026-11-17.md)。Adaptive Runner含单步替身；隔离fixture使用真实RNG/MAT/Manifest，均不等于完整真实求解实验 |
| ANALYSIS | `r05_analysis_source_contract`, `r06_analysis_segments_smoothing`, `r06_analysis_finite_cli`, `r06_bias_archive_independent_curves`（另复用CLI archive） | `tests/integration/test_r05_analysis.cpp`, `test_r06_analysis_segments.cpp`, `test_r06_analysis_finite.cmake`, `r06_analysis_fixture.cpp`, `test_r06_bias_archive.cpp`；[有限输入](R6_ANALYSIS_FINITE_VALIDATION_2026-11-17.md)、[历史Bias及辅助来源](R6_BIAS_ARCHIVE_ANALYSIS_2026-11-17.md) |
| SVG | `r06_svg_source_windows` | `tests/integration/test_r06_svg_windows.cpp`；[视窗实测](R6_SVG_WINDOWS_2026-11-17.md)，状态以该报告为准 |
| ILLUSTRATION | `r06_illustrations_independent_geometry`, `r06_illustrations_real_cli` | `tests/integration/test_r06_illustrations.cpp`, `test_r06_illustrations_cli.cmake`；536225项独立Rodrigues/轨迹/遮罩/画幅检查、每模式29次真实CLI；[原插图与目视](R6_ILLUSTRATIONS_2026-11-17.md)、[新增导出记录及负order拒绝](R6_ARTIFACT_RECORDS_2026-11-18.md) |
| SOURCE | 无独立CTest代表全部人工来源验收 | `SOURCE_INVENTORY.md`, `MIGRATION_MATRIX.md`, `MAT_DATA_SCHEMA.md`, `verification/mat/archive_storage_identity.v1.tsv`, `LICENSE`及原始Git树；[45源现行映射复核](R6_SOURCE_IMPLEMENTATION_MAP_2026-11-17.md)已完成，非全部数学分支/正式验收 |
| ENV | 配置/构建及CLI实测，不是跨发行版资格测试 | [原生环境](R6_NATIVE_VALIDATION_2026-11-17.md)及后续每轮 `environment.txt`、CMake链接图；实际仅Ubuntu24.04.5/GCC16.2.0 |

## 逐TC映射

“待补足/限定”是当前证据边界，不新增验收义务；所有行仍未正式验收。

| TC | 相关实测/材料 | 待补足或证据限定 |
|---|---|---|
| TC-F-CLI-001 | CLI、MAT | inspect元数据/缺字段/非法参数已覆盖；逐条CLI合同最终冻结另办 |
| TC-F-MAT-001 | MAT、SNAPSHOT | 582文件/33340值独立逐位及固定SHA已测，返回摘要绑定实际解码的封存字节；不是所有MAT类型兼容承诺 |
| TC-F-MAT-002 | MAT、MAT73、CLI、DICTIONARY | 结构/类型拒绝与原文件枚举、独立探针582解码/报告及输出故障已测；真实v7.3夹具已补，旧错误分类复现/修复、32场景通过，见 [任务复核D5](R6_PLAN_TASK_AUDIT_2026-11-18.md)。正式数据字典复核仍独立 |
| TC-F-MAT-003 | MAT、SNAPSHOT | named/首变量/后缀/多变量及捕获后路径替换、同inode改写已有实测；复制期间可观察变化拒绝，范围限首版类型 |
| TC-F-MAT-004 | IO、RUNNER | 合成写回读、no-overwrite、字段身份及故障链已测；完整五入口实验不在此证据内 |
| TC-F-GEOM-001 | GEOM | 固定uniform顺序及耗尽已有例；不能等同随机统计资格 |
| TC-F-GEOM-002 | GEOM | level0…2全/缺损八面体有序独立参考；最终容差审查另办 |
| TC-F-GEOM-003 | GEOM | level0…2二十面体有序独立参考；最终容差审查另办 |
| TC-F-FLOW-001 | GEOM、RUNNER | 近度/理想流与抵消固定例；需按完整源输入域审查 |
| TC-F-FLOW-002 | GEOM、RUNNER | 均模长噪声、调用数/连续RNG；未完成论文统计复现 |
| TC-F-FLOW-003 | GEOM、RUNNER | 分量噪声/切平面投影与独立固定例；未完成论文统计复现 |
| TC-F-FLOW-004 | GEOM | 球心、一般位置、旋转射线、边界/非法输入已有解析例 |
| TC-F-FILTER-001 | KVD、RUNNER | Franz独立固定输入及奇异响应；覆盖不等于全输入域证明 |
| TC-F-KVD-001 | KVD | 两路径逐轮固定L3；未批准最终数值门槛 |
| TC-F-KVD-002 | KVD | 两算法实际默认200轮上限、单动态/单绝对退出及最少6轮已实测；506次停止行为与独立解析参考一致，限固定输入族 |
| TC-F-ERROR-001 | METRIC、RUNNER、SESSION、ADAPTIVE-CLI | 解析边界及acos双端舍入越界已测；真实初始化第14帧有限非零向量的独立标量cosine=1+2^-52、最后真实Step按位重放已确认。限当前浮点环境，正式数值资格未批准 |
| TC-F-BIAS-001 | GEOM、RUNNER、BIAS-INJECT、CLI | 有界level0…2、seed7/11、三噪声列、实际求解器/发布；另有两次reset/三求解槽/精确失败前缀，非40份level9实验 |
| TC-F-BIAS-002 | GEOM、RUNNER、BIAS-INJECT、CLI | 同上，分量噪声路径及两模式注入隔离独立核对；不是完整统计复现 |
| TC-F-BIAS-003 | ANALYSIS、CLI、SVG | K=2区别性参考、两组40-run全部26曲线/208对x-y独立重算与986来源条目已实测；沿用回归余量，正式数值资格仍未批准 |
| TC-F-TRAJ-001 | GEOM、RUNNER | 独立轨迹/运动/端点例；默认600帧的第151帧UndefinedAngle明确记录 |
| TC-F-SH-001 | SH、STEP | 独立解析九基/布局/拟合与重建，源角度18解析方向及边界；最终Golden/容差资格另办 |
| TC-F-SH-002 | SH | 源相位作用与物理旋转分别实测；不声称MATLAB运行轨迹相同 |
| TC-F-ADAPT-001 | STEP、SH | 正/负近度固定单步及zero_mean_conditioning解析族已接入；六轴/窄锥完整步骤因投影零范数拒绝，运动阶段可独立核对。病态前向误差未正式验收 |
| TC-F-ADAPT-002 | SESSION、SH、RUNNER | 旋转→比较→主提交、失败回滚及两Session三模式交替独立性已有真实单步实测；调度一致性不是独立数值Golden |
| TC-F-ADAPT-003 | SESSION、RUNNER | 1/5/11/21刷新与弃置比较返回值已有替身断言；可替换Reset/Inherit/Periodic策略、所有槽失败/回滚/重试已测，完整真实轨迹未成功 |
| TC-F-EXP-001 | RUNNER、ADAPTIVE-CLI | 35帧编排/字段替身合同；真实入口第14帧acos越界终止、valid12，无完整四MAT/L2对照成功证据 |
| TC-F-EXP-002 | RUNNER、ADAPTIVE-CLI | 固定600帧/8次聚合及失败分母；真实noise0入口第151帧零旋转拒绝、valid149、计划8不变，未完成8轮 |
| TC-F-EXP-003 | RUNNER、ANALYSIS、ADAPTIVE-CLI | 更新率/现行save集合/缺ratio合同；真实入口第151帧零旋转拒绝、valid149，无MAT，未完成完整真实算法实验 |
| TC-F-AN-001 | ANALYSIS、CLI、SNAPSHOT | 32条有限曲线独立逐值与输入SHA完整JSON、初始化区间已测；平滑辅助源Git blob逐曲线记录并验证JSON/SVG。原NaN明确拒绝；生产MAT摘要已绑定，合成/未知来源仍为空 |
| TC-F-AN-002 | CLI、ANALYSIS、SVG、ILLUSTRATION | 三份插图生产入口、独立数值/绘图隔离及画幅已有实测；sphere端点运行细节与MATLAB灯光/像素等价未验证 |
| TC-D-SOURCE-001 | SOURCE、MAT | 628原文件Git树与582MAT固定身份有证据；正式冻结未签署 |
| TC-D-SOURCE-002 | SOURCE、SH | 论文/资料/重写来源可定位；不是所有来源人工验收已签字 |
| TC-D-MAT-001 | MAT、SOURCE、DICTIONARY | 582实际读取/独立值与SHA；新隔离报告已生成并验证原输入不变，未重写已提交字典；P10.4正式复核未完成 |
| TC-D-MAT-002 | MAT、IO | 列主序、类型/形状与值逐位、写回读；只覆盖已选首版范围 |
| TC-D-MAT-003 | MAT、RUNNER、ANALYSIS | CurrentGenerated/HistoricalOnly及缺ratio已有证据；不推测额外字段历史成因 |
| TC-DC-DOMAIN-001 | CONTRACT、SH | 显式Result工厂、move/lifetime/error链；对象模型正式审查另办 |
| TC-DC-IO-001 | MAT、IO、CONTRACT、SNAPSHOT | 自拥有载荷、故障/权限与寿命、封存副本拒写/拒截断及30例fd释放已测；不凭测试数声称全资源状态证明 |
| TC-DC-RNG-001 | GEOM、RUNNER、BIAS-INJECT | RNG注入/顺序/耗尽/seed分离、两次reset/不支持reset/缓存重置已测；正式接口冻结未完成 |
| TC-DC-NOISE-001 | GEOM、RUNNER、ADAPT-INJECT | 噪声接口两路径、Adaptive显式coefficient/零值仍3N抽样、非法success拒绝及错误上下文已有固定测试 |
| TC-DC-KVD-001 | KVD | 两具名入口组合KvDIterationEngine与各自IKvDTranslationRule实现，策略替换/交错复用/完整故障链/翻号后旧运动顺序均已测；两路径停止条件保留独立参考。候选接口未冻结 |
| TC-DC-ADAPT-001 | STEP、SESSION | 单步/Session/系数策略已分离；显式策略替换、所有准备先于求解、借用对象/状态隔离已测，真实Session交错回归保留，候选接口未冻结 |
| TC-DC-EXPERIMENT-001 | RUNNER、BIAS-INJECT、ADAPT-INJECT、CONTRACT | 两RunContext交替RNG/错误链/计数/真实MAT及Manifest隔离已测；Bias求解器和RunContext RNG注入已补并通过替身验证。Adaptive采样/噪声注入与success边界、默认/显式对照已测；轨迹/环境等其它装配仍待核对，见 [98项复核D3](R6_PLAN_TASK_AUDIT_2026-11-18.md)；不等于总体设计验收或全科学实验 |
| TC-DC-ANALYSIS-001 | CLI、ANALYSIS、SVG、ILLUSTRATION | 无后端时纯数值成功、SVG明确不可用；三份插图JSON导出前后不变已实测，候选接口未冻结 |
| TC-NF-PLATFORM-001 | ENV | 仅实际Ubuntu24.04.5，不能推断其它Debian/Ubuntu版本已测 |
| TC-NF-DEPS-001 | ENV、CMake、HEADLESS | 两模式CMake实图无环、ELF/传递依赖已审计；无被禁止运行时/图形库。系统matio间接含HDF5等库，不扩大MAT支持范围；其它环境仍未验证 |
| TC-NF-SAFETY-001 | IO、MAT、CLI、SNAPSHOT | 原SHA不变、RunId/路径/符号链接/拒覆盖及真实故障已有测试；捕获后源改写不改变解码/摘要，不承诺整个目录原子快照 |
| TC-NF-NUM-001 | KVD、STEP、SH、GEOM | 独立固定参考存在；完整资格/DEC-005未批准，不宣称数值验收 |
| TC-NF-NUM-002 | MAT、KVD、STEP、METRIC、RUNNER、SESSION、ADAPTIVE-CLI | 多类明确失败/病态条件已有固定测试；真实初始化cosine越界及另外两入口零旋转拒绝已有实际进程证据。未改变拒绝策略/阈值或宣称完整实验成功 |
| TC-PROC-TRACE-001 | SOURCE、本文及 [98项P任务表](R6_PLAN_TASK_AUDIT_2026-11-18.md) | 66 TC、45源/50声明、98 P逐项实现/证据/缺口均有索引；已有表不等于逐义务关闭，全部分支资格与正式冻结仍未完成 |
| TC-NF-RNG-001 | GEOM、RUNNER | 固定序列/两seed与失败数量保存；L4统计及全样本账本未完成 |
| TC-NF-LOG-001 | CONTRACT、RUNNER、CLI、ADAPTIVE-CLI、RUNID、STREAM、DICTIONARY、EXECUTION | 五生成入口及plot/SVG导出参数/环境/身份和交付记录已测，输入绑定Analyzer快照；记录失败与条件返回码可复核。只读入口/预留前/信号记录边界仍需核对，未关闭整项义务；完整科学结果未生成 |
| TC-NF-OOP-001 | CONTRACT、SH、SESSION | 局部/公共错误桥接与所有权已有用例；总体设计验收未签署 |
| TC-NF-HEADLESS-001 | CLI、ANALYSIS、ENV、HEADLESS | 全新禁用SVG构建47/47通过，9次合法绘图请求明确拒绝且无输出，数值入口/参考全部保留；默认48/48，全部无显示环境。限定运行证据，不是完整科学验收 |
| TC-NF-RECOVERY-001 | IO、CONTRACT、RUNNER、ADAPTIVE-CLI、RUNID、STREAM、DICTIONARY、EXECUTION | 开始记录未确认不调用Runner/导出器，结束记录失败保留已有Manifest/MAT/导出产物和原错误；失败Manifest先于stderr，SIGPIPE保留开始记录和审计、不伪造结束。探针报告write/close失败退出2并保留内容，不声称报告fsync保证；三Adaptive仍Incomplete、无MAT |
| TC-NF-PORTABLE-001 | IO、MAT、CLI | 实际中文/空格路径；目标仅GNU/Linux，不推断其它平台 |
| TC-NF-RUNTIME-001 | 无完整基准 | 未授权完整实验/性能测量，DEC-006预算未批准；编译耗时不能代替实验预算 |
| TC-NF-SESSION-001 | SESSION、RUNNER、BIAS-INJECT、ADAPT-INJECT | 两Session三模式独立/交替重放、两RunContext交替RNG/输出/失败隔离及两Bias模式嵌套交错、同Adaptive Runner重放的主/比较首帧状态重置已测；不承诺单Session多线程，正式验收仍未签署 |
| TC-NF-COPYRIGHT-001 | SOURCE | 根MIT与来源notice存在；不将MIT扩展至原MATLAB/MAT/论文，不以额外普通版权审批阻塞编码 |
| TC-F-CLI-002 | CLI、RUNNER、EXECUTION | 两Bias真实进程、有界输出/seed/RunId及拒覆盖，开始/结束记录独立解析；完整规模未运行 |
| TC-F-CLI-003 | CLI、RUNNER、ADAPTIVE-CLI、EXECUTION | 三入口实际失败链/计数/源身份/无MAT/拒覆盖及运行记录已测；默认初始化因acos越界、另两入口因零旋转不能完成，完整成功产物分支仍未实测 |
| TC-F-CLI-004 | CLI、ANALYSIS | 四分析类型实际进程：历史3组成功/3组NaN拒绝、有限两入口成功；历史Bias全部数值另用真实Reader/Analyzer独立复核，辅助来源在CLI已测，不假称所有归档成功 |
| TC-F-CLI-005 | CLI、SVG、ILLUSTRATION | plot与三份插图实际发布/无后端错误/几何/拒覆盖已测；CLI增量未冻结，不承诺MATLAB像素等价 |
| TC-I-CLI-001 | CLI | 已知子命令帮助、JSONL、拒自定义输出根等；DEC-008冻结未签署 |
| TC-I-MAT-001 | MAT、IO、SNAPSHOT | 严格/兼容读取与安全写回读；数值读取封存副本和metadata不解码均已测，首版类型边界明确 |
| TC-I-RESULT-001 | IO、CONTRACT、RUNNER、CLI、ADAPTIVE-CLI | 当前save合同和真实三个失败审计/固定目录已有实测；五入口完整真实成功不能由替身或失败合同通过覆盖 |
| TC-I-ANALYSIS-001 | ANALYSIS、CLI、SNAPSHOT | 原数据不变/全部分析类型成功或规定失败、历史Bias全曲线及逐曲线平滑来源有证据；986贡献摘要与有限曲线实际读取SHA已核验，原NaN拒绝、未知来源空值保留 |

## 下一步按缺口实施

1. 当前两模式分别完成57/57、56/56复测；所有失败日志及原始数据不变证据保留。不等于完整数值资格、Adaptive实验成功或MATLAB像素等价完成。
2. 三份illustration已有JSON/SVG入口、独立全网格参考和实际CLI/目视证据；保留sphere端点运行细节尚未验证及平面着色的明确边界，不用求解器九项归一化基函数替代插图。
3. 三Adaptive真实进程的失败链已有证据；初始化第14帧acos越界、另外两入口第151帧零旋转均未产出完整MAT，成功路径仍未验证。45源、98 P和本表有限证据已映射，[数值汇总](../NUMERICAL_EQUIVALENCE.md)已建立；Bias注入、真实MAT73负例、plot/SVG导出记录及KvD/Session策略组合已补，继续领域构造不变量/其它策略职责/Adaptive装配合同及只读/预留前记录边界。数值策略变更不能由AI通过改测试自行批准。
4. 完整科学/L4实验、跨发行版资格、资源预算与正式审批不在本轮运行授权内；准备具体材料并在真正到达该边界时请求相应授权。无MATLAB/L1阻塞。
