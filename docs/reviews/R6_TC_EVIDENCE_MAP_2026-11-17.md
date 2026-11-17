# R6：66项 TC 的实际证据映射与剩余工作

`[AI-EDIT]` 事实索引，基于当前Git树、CMake登记、测试源码及实际日志。保留 [TEST_PLAN §3](../TEST_PLAN.md) 的66个TC/REQ/AC和 [PLAN](../PLAN.md) 的98项任务，不替换候选规范、改动阈值或签署验收。下表每个TC仅列一次；其REQ/AC采用相同后缀，正式义务仍以 [REQUIREMENTS](../REQUIREMENTS.md) 为准。

**没有任何一行仅凭CTest数量被宣布正式验收。** “相关实测”只表示下面列出的具体程序/样本已有运行证据，不代表该TC全部条款已满足。最新受测源码 `07f37e2b4db3e34512715c69a805342f4e6e8add` 为40/40；新增conditioning1077项检查通过，见 [专项证据](R6_ADAPTIVE_CONDITIONING_2026-11-17.md)。原39项仍通过，首轮覆盖不足及此前失败保留。正式容差/Golden资格、病态前向误差、环境预算及基线签署仍独立办理；不以这些审批尚未完成为理由停止已授权的源码/测试补足。

## 实际测试组及证据入口

以下是便于表格引用的索引，不是新的CTest或TC编号。真实目标/源路径见根 [CMakeLists](../../CMakeLists.txt)；各报告固定SHA、命令、环境、日志和边界，历史提交不能自动覆盖新源码。

| 索引 | 实际CTest（完整名称） | 源码/证据 |
|---|---|---|
| CLI | `r01_cli_contract`, `r06_cli_process_contract`, `r06_cli_process_archive` | `tests/integration/test_r01_cli.cpp`, `test_r06_cli_process.cmake`；[CLI实测](R6_CLI_PROCESS_VALIDATION_2026-11-17.md) |
| MAT | `r02_mat_reader`, `r06_historical_mat_readonly_schema`, `r06_mat_independent_storage_values`, `r06_mat_reader_boundaries`, `r06_mat_raw_payload_boundaries` | `tests/integration/test_r02_reader.cpp`, `test_r06_archive_readonly.cpp`, `test_r06_mat_independent.cpp`, `test_r06_reader_negative.cpp`, `test_r06_mat_payload.cpp`；[L2](R6_MAT_STORAGE_VALIDATION_2026-11-17.md)、[边界](R6_READER_BOUNDARIES_2026-11-17.md)、[载荷](R6_MAT_PAYLOAD_VALIDATION_2026-11-17.md) |
| IO | `r05_anchored_io`, `r06_io_symlink_and_runid_safety`, `r06_linux_writer_faults`, `r06_real_manifest_faults` | `tests/integration/test_r05_io.cpp`, `test_r06_io_safety.cpp`, `test_r06_io_faults.cpp`；[发布故障](R6_IO_FAULT_VALIDATION_2026-11-17.md) |
| CONTRACT | `r01_contracts`, `r05_manifest_gate`, `r06_result_lifetimes_error_frames` | `tests/contracts/test_r01_contracts.cpp`, `test_r05_manifest.cpp`, `test_r06_result_lifetimes.cpp`；[原生记录](R6_NATIVE_VALIDATION_2026-11-17.md) |
| GEOM | `r06_sampling_flow_contract`, `r06_directions_all_ordered_l3`, `r06_trajectory_sphere_geometry` | `tests/integration/test_r06_sampling_flow.cpp`, `test_r06_direction_rows_l3.cpp`, `test_r06_trajectory_sphere.cpp`；`verification/geometry/` |
| KVD | `r06_kvd_independent_l3`, `r06_kvd_stopping_reference` | `tests/integration/test_r06_kvd_l3.cpp`, `test_r06_kvd_stopping.cpp`；`verification/kvd/`；原逐节点参考及506次独立解析控制流参考，见 [停止分支](R6_MOTION_STOPPING_2026-11-17.md) |
| METRIC | `r06_source_motion_errors` | `tests/integration/test_r06_motion_errors.cpp`；直接调用两个生产内部实现，解析几何/尺度/弦长恒等式/零/非有限/上溢/下溢；[685项实测](R6_MOTION_STOPPING_2026-11-17.md) |
| STEP | `r06_adaptive_step_independent_l3`, `r06_adaptive_conditioning_reference` | `tests/integration/test_r06_adaptive_l3.cpp`, `test_r06_adaptive_conditioning.cpp`；正/负九方向完整单步，另12个解析运动阶段/完整投影拒绝及4个定义域负例，见 [conditioning实测](R6_ADAPTIVE_CONDITIONING_2026-11-17.md) |
| SH | `low_order_harmonics`, `low_order_harmonics_archived_golden`, `low_order_harmonics_source_projection`, `r06_sh_argument_action`, `r06_sh_source_phase_reference`, `r06_sh_source_session_action` | `tests/unit/test_low_order*.cpp`, `tests/integration/test_r06_sh_adapter.cpp`, `test_r06_source_sh.cpp`；[源相位作用](R6_SH_SOURCE_ACTION_2026-11-17.md) |
| SESSION | `r06_adaptive_session_transaction`, `r06_adaptive_periods_atomic_commit` | `tests/contracts/test_r06_session_contract.cpp`, `test_r06_adaptive_periods.cpp`；三模式失败原子性及1/5/11/21刷新 |
| RUNNER | `r06_adaptive_runner_complete_contract`, `r06_adaptive_runner_incomplete_contract`, `r06_adaptive_default_undefined_contract`, `r06_bias_runner_contract`, `r06_bias_runner_publication` | `tests/integration/test_r06_adaptive_runner.cpp`, `test_r06_bias_runner.cpp`；[Adaptive](R6_ADAPTIVE_RUNNER_CONTRACT_2026-11-17.md)、[Bias](R6_BIAS_RUNNER_VALIDATION_2026-11-17.md)。Adaptive含单步替身，不等于完整真实求解实验 |
| ANALYSIS | `r05_analysis_source_contract`, `r06_analysis_segments_smoothing`, `r06_analysis_finite_cli`（另复用CLI archive） | `tests/integration/test_r05_analysis.cpp`, `test_r06_analysis_segments.cpp`, `test_r06_analysis_finite.cmake`, `r06_analysis_fixture.cpp`；[有限输入](R6_ANALYSIS_FINITE_VALIDATION_2026-11-17.md) |
| SVG | `r06_svg_source_windows` | `tests/integration/test_r06_svg_windows.cpp`；[视窗实测](R6_SVG_WINDOWS_2026-11-17.md)，状态以该报告为准 |
| SOURCE | 无独立CTest代表全部人工来源验收 | `SOURCE_INVENTORY.md`, `MIGRATION_MATRIX.md`, `MAT_DATA_SCHEMA.md`, `verification/mat/archive_storage_identity.v1.tsv`, `LICENSE`及原始Git树；MAT实测可提供资料身份的一部分证据 |
| ENV | 配置/构建及CLI实测，不是跨发行版资格测试 | [原生环境](R6_NATIVE_VALIDATION_2026-11-17.md)及后续每轮 `environment.txt`、CMake链接图；实际仅Ubuntu24.04.5/GCC16.2.0 |

## 逐TC映射

“待补足/限定”是当前证据边界，不新增验收义务；所有行仍未正式验收。

| TC | 相关实测/材料 | 待补足或证据限定 |
|---|---|---|
| TC-F-CLI-001 | CLI、MAT | inspect元数据/缺字段/非法参数已覆盖；逐条CLI合同最终冻结另办 |
| TC-F-MAT-001 | MAT | 582文件/33340值独立逐位及固定SHA已测；不是所有MAT类型兼容承诺 |
| TC-F-MAT-002 | MAT、CLI | 结构/类型拒绝与原文件枚举已测；正式数据字典复核仍独立 |
| TC-F-MAT-003 | MAT | named/首变量/后缀/多变量等边界已有实测，范围限首版类型 |
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
| TC-F-ERROR-001 | METRIC、RUNNER、SESSION | 正交/反向/尺度/任一零/非有限/范数边界直接实测；两指标及弦长恒等式区分。acos舍入越界专例尚缺，正式数值资格未批准 |
| TC-F-BIAS-001 | GEOM、RUNNER、CLI | 有界level0…2、seed7/11、三噪声列、实际求解器/发布；不是40份level9实验 |
| TC-F-BIAS-002 | GEOM、RUNNER、CLI | 同上，分量噪声路径独立核对；不是完整统计复现 |
| TC-F-BIAS-003 | ANALYSIS、CLI、SVG | K=2区别性参考、两个40目录实际读取；历史全曲线数值的独立重算仍需补足 |
| TC-F-TRAJ-001 | GEOM、RUNNER | 独立轨迹/运动/端点例；默认600帧的第151帧UndefinedAngle明确记录 |
| TC-F-SH-001 | SH、STEP | 独立解析九基/布局/拟合与重建；最终Golden/容差资格另办 |
| TC-F-SH-002 | SH | 源相位作用与物理旋转分别实测；不声称MATLAB运行轨迹相同 |
| TC-F-ADAPT-001 | STEP、SH | 正/负近度固定单步及zero_mean_conditioning解析族已接入；六轴/窄锥完整步骤因投影零范数拒绝，运动阶段可独立核对。病态前向误差未正式验收 |
| TC-F-ADAPT-002 | SESSION、SH、RUNNER | 旋转→比较→主提交及失败回滚；仍需两个独立Session交替运行的直接断言 |
| TC-F-ADAPT-003 | SESSION、RUNNER | 1/5/11/21刷新与弃置比较返回值已有替身断言；完整真实轨迹未成功 |
| TC-F-EXP-001 | RUNNER | 初始化35帧编排/字段合同；单步替身结果不能当完整算法实验 |
| TC-F-EXP-002 | RUNNER | 固定600帧/8次聚合及失败分母；原默认无定义角度不能记成功 |
| TC-F-EXP-003 | RUNNER、ANALYSIS | 更新率/现行save集合/缺ratio明确错误；未完成完整真实算法实验 |
| TC-F-AN-001 | ANALYSIS、CLI | 32条有限曲线独立逐值与初始化区间；原NaN明确拒绝，平滑脚本来源身份仍需补齐 |
| TC-F-AN-002 | CLI、ANALYSIS、SVG | 数值/绘图隔离及窗口测试；三份illustration生产展示入口缺失 |
| TC-D-SOURCE-001 | SOURCE、MAT | 628原文件Git树与582MAT固定身份有证据；正式冻结未签署 |
| TC-D-SOURCE-002 | SOURCE、SH | 论文/资料/重写来源可定位；不是所有来源人工验收已签字 |
| TC-D-MAT-001 | MAT、SOURCE | 582实际读取/独立值与SHA；P10.4数据字典正式复核未完成 |
| TC-D-MAT-002 | MAT、IO | 列主序、类型/形状与值逐位、写回读；只覆盖已选首版范围 |
| TC-D-MAT-003 | MAT、RUNNER、ANALYSIS | CurrentGenerated/HistoricalOnly及缺ratio已有证据；不推测额外字段历史成因 |
| TC-DC-DOMAIN-001 | CONTRACT、SH | 显式Result工厂、move/lifetime/error链；对象模型正式审查另办 |
| TC-DC-IO-001 | MAT、IO、CONTRACT | 自拥有载荷、故障/权限与寿命用例；不凭测试数声称全资源状态证明 |
| TC-DC-RNG-001 | GEOM、RUNNER | RNG注入/顺序/耗尽/seed分离；正式接口冻结未完成 |
| TC-DC-NOISE-001 | GEOM、RUNNER | 噪声接口两路径与错误上下文已有固定测试 |
| TC-DC-KVD-001 | KVD | Original/Modified单独入口与差异化trace；两路径实际默认上限及其他退出条件已有独立参考实测 |
| TC-DC-ADAPT-001 | STEP、SESSION | 单步和Session已独立测试；两个Session交替隔离待补 |
| TC-DC-EXPERIMENT-001 | RUNNER、CONTRACT | 注入/编排/Writer/状态已有例；两个RunContext交替状态隔离仍待直接验证 |
| TC-DC-ANALYSIS-001 | CLI、ANALYSIS、SVG | 无后端替身与纯数值/绘图数据不变；三份illustration未完成 |
| TC-NF-PLATFORM-001 | ENV | 仅实际Ubuntu24.04.5，不能推断其它Debian/Ubuntu版本已测 |
| TC-NF-DEPS-001 | ENV、CMake | 当前核心Eigen/matio与可选SVG，无Python/MATLAB/NPZ；完整交付链接审计仍待整理 |
| TC-NF-SAFETY-001 | IO、MAT、CLI | 原SHA不变、RunId/路径/符号链接/拒覆盖及真实故障已有测试 |
| TC-NF-NUM-001 | KVD、STEP、SH、GEOM | 独立固定参考存在；完整资格/DEC-005未批准，不宣称数值验收 |
| TC-NF-NUM-002 | MAT、KVD、STEP、METRIC、RUNNER、SESSION | 多类明确失败、直接误差边界、零/抵消均值/纯旋转/重合视线/投影零范数已实测；acos舍入越界专例待补，未新增病态拒绝阈值 |
| TC-PROC-TRACE-001 | SOURCE及本文 | 66项证据索引已整理；98任务/45源的逐义务关闭与正式冻结仍未完成 |
| TC-NF-RNG-001 | GEOM、RUNNER | 固定序列/两seed与失败数量保存；L4统计及全样本账本未完成 |
| TC-NF-LOG-001 | CONTRACT、RUNNER、CLI | 顶层家族/完整causes/Manifest与执行日志已测；完整科学运行未发生 |
| TC-NF-OOP-001 | CONTRACT、SH、SESSION | 局部/公共错误桥接与所有权已有用例；总体设计验收未签署 |
| TC-NF-HEADLESS-001 | CLI、ANALYSIS、ENV | 当前SVG零外部图形依赖；无后端注入例通过，非独立禁用绘图构建资格 |
| TC-NF-RECOVERY-001 | IO、CONTRACT、RUNNER | Failed/Incomplete/Complete、失败日志/不覆盖、持久化故障已有实测 |
| TC-NF-PORTABLE-001 | IO、MAT、CLI | 实际中文/空格路径；目标仅GNU/Linux，不推断其它平台 |
| TC-NF-RUNTIME-001 | 无完整基准 | 未授权完整实验/性能测量，DEC-006预算未批准；编译耗时不能代替实验预算 |
| TC-NF-SESSION-001 | SESSION、RUNNER部分 | 单实例事务已有测试；两个RunContext交替执行的直接测试缺失，不承诺单Session多线程 |
| TC-NF-COPYRIGHT-001 | SOURCE | 根MIT与来源notice存在；不将MIT扩展至原MATLAB/MAT/论文，不以额外普通版权审批阻塞编码 |
| TC-F-CLI-002 | CLI、RUNNER | 两Bias真实进程、有界输出/seed/RunId及拒覆盖；完整规模未运行 |
| TC-F-CLI-003 | CLI、RUNNER部分 | 帮助/非法参数与Runner内部合同；三Adaptive真实生成进程的成功/失败链尚未全覆盖 |
| TC-F-CLI-004 | CLI、ANALYSIS | 四分析类型实际进程：历史3组成功/3组NaN拒绝、有限两入口成功；不假称所有归档成功 |
| TC-F-CLI-005 | CLI、SVG部分 | plot实际发布/无后端错误/几何；插图入口和完整视觉等价未完成 |
| TC-I-CLI-001 | CLI | 已知子命令帮助、JSONL、拒自定义输出根等；DEC-008冻结未签署 |
| TC-I-MAT-001 | MAT、IO | 严格/兼容读取与安全写回读；首版类型边界明确 |
| TC-I-RESULT-001 | IO、CONTRACT、RUNNER、CLI | 当前save、Manifest/错误链/固定目录已有实测；五入口完整真实实验不能由替身覆盖 |
| TC-I-ANALYSIS-001 | ANALYSIS、CLI | 原数据不变/全部分析类型成功或规定失败有证据；原NaN与未知输入摘要如实保留 |

## 下一步按缺口实施

1. 原绘图视窗、误差/停止分支及conditioning参考接入已完成40/40复测，失败日志及原始数据不变证据保留；不等于完整数值资格、三份illustration或视觉等价完成。
2. 为现有M级义务补两个Session/RunContext交替隔离；误差函数另缺acos舍入越界专例。先读源/现有参考，不能用增加相似正常例代替。
3. 补足原历史Bias分析逐值独立重算、平滑来源身份；审查三份illustration真实数据和展示入口。若需要改动CLI/公共schema合同，先保留候选边界，不自行宣称冻结。
4. 完整科学/L4实验、跨发行版资格、资源预算与正式审批不在本轮运行授权内；准备具体材料并在真正到达该边界时请求相应授权。无MATLAB/L1阻塞。
