# HAR-2026-11-06-001：审批后的设计修订与待审材料包

> **状态：[AI-DRAFT + HUMAN-APPROVE]；这是2026-11-06审批事项的历史审查包，未冻结、非执行许可、非测试验收。** 根据用户上传的 `HUMAN_APPROVAL_REQUEST.json` 整理，适用于 `ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields` 的 `dev-plan-P0-P1-mat-dictionary-probe` 分支。申请内 `applicationCommit` 与 `scopeIds` 为空，故不能推定任何完整 Git SHA 已签字批准。**不得将未勾选确认的预填参考文字等同于人类已确认意见，也不在仓库上传原始审批 JSON。**

**2026-11-11文档审计适用性说明**：H-01～H-16的JSON决议、当时权限和观察不能追溯改写；下文的“本轮”“当前允许”均指2026-11-06材料形成时点。后续已生效的[永久不运行MATLAB/L1政策](../golden/NO_MATLAB_EXECUTION_POLICY.md)优先于本文件H-09历史采集提案；后续受限SH叶子用户原生3/3证据见[PROGRESS.md](../PROGRESS.md)，不等于整工程代码/TC获批。现行全工程文档整改与未解除的软件/批准阻塞由[AUDIT_PROGRESS.md](../AUDIT_PROGRESS.md)登记。

本材料只补充 **P0/P1 的下一轮审查输入**，不另立或修改 REQ、AC、TC、P 的规范性义务；如未来确需改变任何 SRS/AC/TC/P，则需先另行审批。原实施主计划见 [PLAN.md](../PLAN.md)，项目实际进度见 [PROGRESS.md](../PROGRESS.md)，已有决定与上游替代路径见 [DECISION_IMPLEMENTATION_NOTES.md](DECISION_IMPLEMENTATION_NOTES.md)。

## 1. 决定与权限分层（H-01～H-16）

| 分类 | 项目 | 可在本次计划中落实的范围 | 绝不能据此推断 |
|---|---|---|---|
| **已选定的受限设计路线** | H-02、H-14 | Rodrigues 的实际前向旋转调用选 Sophus SO(3)；SHRotate 选 Don Williamson 原 C++ Ivanic–Ruedenberg 方案 A，保留 l=0/1/2、系数排序、第三方来源 README/版权及适用 LICENSE 告知 | 不等于代码已复制、构建成功、原 MATLAB 及其余第三方内容的许可已获核验，也不等于 DEC-005 已关闭 |
| **已选定的平台信息** | H-04 | 以用户提供的 Ubuntu 24.04.4 VirtualBox/x86_64 `uname -a`/`df`/`free` 输出作为目标环境快照 | 不自动推出编译器版本、设备 CPU 核数、性能阈值、可用性能预算或测试通过 |
| **已选定的数据隔离原则** | H-12 | 新结果规划为仓库根 `output/<RunId>/`；根 `.gitignore` 的 `/output/` 已存在，`origin/**` 保持只读 | H-10 暂缓、`permissions.writeMat=false`，因此当前不准创建或运行写回读/损坏 MAT 负例 |
| **仅供起草** | H-01、H-03、H-05～H-09、H-13、H-15、H-16 | 只允许形成容差/合同/影响/来源/Golden/里程碑/发布候选和静态清单 | 不得填入已批准数值、冻结设计、关闭 DEC、生成真实 Golden、宣布完成 P/TC/AC |
| **暂缓执行** | H-10、H-11 | 保留设计准备与已有的 GitHub 连接器只读静态核查结果 | 不运行生产 C++、CMake、CTest、MATLAB、Bash、MAT 写入/性能测量，不得伪称现场 Bash 已执行 |

`technicalDetails.permissions` 的十项权限 `runCMake`、`runCTest`、`runMatlab`、`writeMat`、`runBash`、`measurePerformance`、`mergePR`、`publish`、`createLicense`、`freezeBaseline` **全部为 false**。即便某个 H 项标为 approved，也只在其明确指定的设计内容与路径内生效。未批准的 draft 不升级为「条件批准」。

## 2. H-01：逐模块数值容差建议的采集与判定模板（DEC-005 仍开放）

**本次没有来自 MATLAB 与 C++ 同一固定输入的逐值误差样本，故不填写任何 atol/rtol 具体数值。** 对任一有定义参考元素 `m_i` 与候选移植结果 `c_i`，建议分开记录绝对差 `|c_i-m_i|`、相对差（在参考值非零时）以及拟采用的逐元素检验 `|c_i-m_i| <= atol + rtol*|m_i|`。该公式也只是候选；未定义值、相位/轴符号模糊与统计有效性不能用这一条比较掩盖。

| 模块 / JSON tolerances.id | 固定输入与输出核对对象 | 必须收集的误差证据 | 候选 atol/rtol / 决定 |
|---|---|---|---|
| KvD / `kvd` | 同一 N×6 光流，Original/Modified 各轮 `near, t, omega, x`，停止条件与迭代次数 | 每轮元素最大差、尺度/方向分开比较、近零范数/病态/迭代上限分组 | **待实测，DEC-005 未批准** |
| 球谐 / `harmonics` | `l<=2` 的 9 项实基、R0/R3/R5 全元素及旋转前后系数，尤其 SHRotate 25 个 R5 元素 | 轴排列、m 顺序、相位与主动/被动方向判别；正交性仅为辅证 | **待实测，DEC-005 未批准** |
| Adaptive / `adaptive` | 单轮 `near, M1…M4, rhs, b/z, t, omega, error, new_coeff` 和跨帧会话 | 逐节点差、累积漂移、异常提交后后续状态及有效样本分母 | **待实测，DEC-005 未批准** |
| 采样 / `sampling` | 完整/缺损八面体、二十面体，固定 seed 或固定 RNG 序列，所有有序方向行 | 逐行排列、方向范数和 rand/randn 消费次数；不可只比集合 | **待实测，DEC-005 未批准** |
| 轨迹 / `trajectory` | 相同位置序列、转轴及角度、Sophus 前向 `R` 与源 Rodrigues、`R^T` 局部运动 | `omega=0`、微小角、主轴、接近 π，逐元素矩阵误差、符号与坐标系 | **待实测，DEC-005 未批准** |
| MAT/分析 / `matAnalysis` | 原 MAT 类型、尺寸、列主序值、MATLAB 1-based 分段与四类分析曲线 | 精确字段身份、逐值差、有效样本、分段边界与统计指标 | **待实测，DEC-005 未批准** |

比较样本必须共用固定输入与同一索引/状态；按量纲、数值尺度、临界条件分别汇总，不以改变结果值、排除难例或随意放宽阈值达到通过。批准者需要看到独立L2历史MAT、L3逐节点参考与必要的L4统计证据、待测C++结果、差异分布、来源哈希和选择阈值的理由；**不要求运行原MATLAB/Octave或取得L1 Golden**。数值阈值、人类批准人、批准 SHA 与日期保持空缺，不得自动填入。

## 3. H-03：CLI/错误协议及 TC→测试→Golden 待审清单（DEC-008 仍开放）

`docs/design/cli.md` 当前已列出 `inspect-mat`、两类 bias、三类 adaptive、`analyze`、可选 `plot` 的参数与候选退出码；`docs/design/contracts.md §3.1` 是候选错误枚举到退出码的**唯一来源**：正常 0，参数 2，输入/MAT 读取 3，MAT 类型/布局 4，数值/实验 5，输出安全 6，可选绘图不可用 7。**这些是待批草案，不是已实现 CLI。**

| 检查对象 | 现有候选 / 必须明确的选项 | 关联测试 |
|---|---|---|
| 运行输入 | `--seed UINT32`、`--level NONNEG_INT`、`--noise-scale FINITE_DOUBLE`、`--run-id ID`；是否接受负噪声、缺省 RunId、参数重复，均须人审 | TC-F-CLI-002/003、TC-I-CLI-001 |
| MAT 输入 | `--file`、`--variable`、strict/compat；无后缀、首变量顺序与多变量拒绝策略需源 Golden | TC-F-CLI-001、TC-F-MAT-003、TC-I-MAT-001 |
| 结果写出 | `--output-root DIR` 候选与 H-12 固定 `output/` 的关系、RunId 并发新建、符号链接和已存在目录拒绝 | TC-I-RESULT-001、TC-NF-SAFETY-001、TC-F-MAT-004 |
| 诊断序列化 | `error_code/exit_code/stage/message`、path/variable/RunId/样本索引；JSONL 或键值格式尚未决定 | TC-I-CLI-001、TC-NF-LOG-001 |
| 实验与分析 | 输出 MAT 变量名/shape/列主序、analyze 不执行求解、plot 缺席可独立失败 | TC-F-CLI-003/004/005、TC-F-AN-001/002 |

候选 **TC→测试文件/CTest→Golden** 的单一完整矩阵以 [TEST_PLAN.md §3](../TEST_PLAN.md) 中 **66 项**为准；在此仅列冻结前必须对照的代表项，防止制造第二份相互漂移的「正式矩阵」：

| TC | 未来测试实体（均未创建/未执行） | Golden 身份及主要检查 |
|---|---|---|
| TC-F-CLI-001～005、TC-I-CLI-001 | `tests/integration/test_cli.cpp` / `egomotion.cli.*` | CLI 命令/参数/exit/stderr，MAT 变量合同，已批准退出码 |
| TC-F-MAT-001～004、TC-D-MAT-001～003 | `tests/golden/test_mat_archive.cpp` / `egomotion.mat.*` | 原始 MAT 字典及独立 SHA-256、类型负例、列主序、写回读（待获权限） |
| TC-F-KVD-001/002、TC-DC-KVD-001 | `tests/unit/test_estimators.cpp` / `egomotion.estimators.*` | Original/Modified 逐轮固定 MATLAB near/t/omega |
| TC-F-SH-001/002 | `tests/unit/test_harmonics.cpp` / `egomotion.harmonics.*` | l<=2 实基、R3/R5 逐元素、前后 9 项系数 |
| TC-F-TRAJ-001 | `tests/unit/test_trajectory.cpp` / `egomotion.trajectory.*` | Rodrigues→Sophus 前向旋转与轨迹固定输入 |
| TC-F-ADAPT-001～003、TC-DC-ADAPT-001 | `tests/unit/test_adaptive_mfa.cpp` / `egomotion.adaptive.*` | M1…M4/b/z/step/session 的逐节点及状态 Golden |
| TC-F-AN-001/002 | `tests/golden/test_analysis.cpp` / `egomotion.analysis.*` | 历史 MAT 曲线原值、MATLAB 索引、分段和平滑 |

每条 TC 在可执行前必须补上：真实文件与函数名、CTest 注册名、输入版本、参考数据 SHA-256、完整源 MATLAB blob SHA、已获批准的容差与错误处理、预期/实际失败计数、真实日志。当前仅独立 matio 探针有 582/582 的历史读取证据，不能将其扩展为上表 TC 通过。

## 4. H-05：需求变更影响候选（仅比较，不修改 SRS）

| 受影响 REQ（既有义务） | 已选/拟议的实现差异 | 对应 TC / AC 与 P | 评审状态 |
|---|---|---|---|
| REQ-F-TRAJ-001（保持 Rodrigues 所承担的旋转语义） | H-02 选 Sophus `SO3d::exp`，仅替代实际正向旋转计算，原方向、Rᵀ 和极端角条件不变 | TC-F-TRAJ-001 / AC-F-TRAJ-001；P6.2 | **技术路线已选；数值义务不变** |
| REQ-F-SH-002（l≤2 SHRotate/系数次序等价） | H-02/H-14 选 Don Williamson Ivanic–Ruedenberg 方案 A，增加上游 sdla 依赖/许可审查，不改数学输出 | TC-F-SH-002 / AC-F-SH-002；P7.5～P7.6 | **技术路线已选；Golden 未确认** |
| REQ-D-SOURCE-002、REQ-NF-COPYRIGHT-001（源及版权审计） | 限定第三方来源；复用目录保留来源 README、源文件精确路径及 MIT 条件、所需 LICENSE；其他原 MAT/脚本不因此获许可 | TC-D-SOURCE-002、TC-NF-COPYRIGHT-001 / 对应 AC；P0.4、P10.7 | **限定采用范围已选；实施/发布未授权** |
| REQ-NF-SAFETY-001、REQ-F-MAT-004（原始数据不可被覆盖） | H-12 指定 `output/<RunId>/`，已有 `.gitignore` 排除；不改变 `origin/**` 只读义务 | TC-NF-SAFETY-001、TC-F-MAT-004 / 对应 AC；P1.7、P1.9 | **路径原则已选；写入/负例未授权** |
| REQ-D-MAT-003（忠实处理历史 MAT） | 按 H-07 起草当前脚本保存/历史文件/分析消费三方差异表，不新建虚假历史字段 | TC-D-MAT-003 / AC-D-MAT-003；P8.7、P10.4 | **仅候选归因，未改变需求** |
| REQ-NF-NUM-001（数值等价） | H-01 提议逐模块实测误差/Golden 再定阈值 | TC-NF-NUM-001 / AC-NF-NUM-001；P10.3 | **DEC-005 未批准** |
| REQ-I-CLI-001（可核查 CLI） | H-03 先列参数、退出码、RunId、诊断序列化 | TC-I-CLI-001 / AC-I-CLI-001；P0.8、P1.2 | **DEC-008 未批准** |

**结论**：上述为「已有义务 → 已选技术路线/候选补充 → 影响」的清单，本轮 **不重写任何 REQ、优先级、AC、TC、P 的规范性文字或达标值**。日后若正式要求改变义务，须列明原句、新句、差异理由、具体 SHA、人类批准后再同步全部追踪文档。

## 5. H-06：P0.8 设计冻结准备核对表（不执行冻结）

| 核对项 | 现有可追踪材料 | 缺失证据 / 退出条件 |
|---|---|---|
| 45/45 MATLAB 源职责与移植边界 | SRS §9.2、OBJECT_MODEL、MAT_DATA_SCHEMA、只读 Git 树 | SOURCE_INVENTORY/MIGRATION_MATRIX 完整成稿、P0.1～P0.7 正式评审 |
| 依赖与所有权 | ARCHITECTURE §4.1，静态 HLD 26 条 DAG 边无环；contracts.md 的 Result/RAII/RNG/Session 候选 | CMake 业务 target 实现/真实依赖检查及资源所有权验证，均未执行 |
| CLI 与错误码 | cli.md、contracts.md §3.1、TEST_PLAN §3 | DEC-008 的参数/exit/diagnostics/CTest identity 批准；当前均未冻结 |
| MAT 格式/读写 | 独立 matio 582/582 READ_OK，五种形状，FNV64 字典 | 独立 SHA-256、生产 Reader 拒绝负例、write/readback 的合法授权及证据 |
| 算法/数值 | kvd/harmonics/adaptive/trajectory 的公式 LLD | 独立 MATLAB Golden、逐元素差异、DEC-005 容差和失败策略 |
| 第三方与发布 | H-02/H-14 的 Sophus/Williamson 选项及来源链接 | 精确复用文件清单、上游许可证与依赖闭包、实际复用目录 README/LICENSE；没有根 LICENSE 授权 |
| 平台资源 | H-04 Ubuntu 24.04.4 VirtualBox/x86_64 用户快照；历史探针 GCC 13/matio 1.5.26 | 本机编译器/Sophus/Eigen/CMake 未实测、性能预算无阈值，不能宣布达到要求 |
| DEC/人类签署 | 本材料、审批落实记录、PROGRESS | DEC-001/002/003/004/005/006/007/008 开放项审查、具体 Git SHA/范围/日期/批准人；`freezeBaseline=false` |

**所有核对项只有“证据/待办”的状态，不得勾选已冻结。** H-02/H-14 技术路线和 H-12 路径获选定不替代数值验证或版权再分发记录。静态 HLD 无环不代表真实 CMake 构建无环。

## 6. H-07：历史 MAT、当前 save 与分析入口核对

以下源文件引用均位于 `origin/egomotion_estimation_dense_flow_fields_matlab_scripts/adaptive_matched_filter_test/`，通过本分支 GitHub **源文本静态读取**核对（不是本轮 MATLAB 运行）：

| 数据组 | 历史 MAT 中存在 | 当前生成脚本的有效 save | 当前分析脚本的消费 | 差异 / 处理候选 |
|---|---|---|---|---|
| `without_errors/` | 4 种误差 + `rottransratio`，共 5 | `data_generation/agent_in_sphere_adaptive_matched_filter_test.m` 保存这 5 种 | `data_analysis/graph_agent_in_sphere_adaptive_matched_filter_test.m` 读取 `rottransratio` 等 | 与当前主要保存清单一致；不能推断具体历史参数完全相同 |
| `with_errors/` | 上述 5 种 + `alleadaprotfehler`、`alleadaptransfehler`，共 7 | 同一当前生成脚本仍仅 save 5 种，**不** save `alleadap*` | 当前分析入口消费其已声明字段，未证明生成 `alleadap*` | 两份为历史额外工件；来源/生成脚本版本未确认 |
| `many_parameter/` | `allemittransfehler`、`allemitrotfehler`、`alleadaptransfehler`、`alleadaprotfehler` 为 600×4；`akrot`、`rottransratio` 为 600×1；共 6 | `data_generation/agent_in_sphere_adaptive_matched_filter_many_parameter.m` 仅有效 save `allemittransfehler`、`allemitrotfehler`、`akrot`（另外两项 ohne save 为注释） | `data_analysis/graph_agent_in_sphere_adaptive_matched_filter_many_parameter.m` **读取** `rottransratio` | `alleadap*` 与 `rottransratio` 为当前 save 清单之外的历史额外字段；后者是分析脚本直接依赖但当前生成脚本不写的缺口 |

证据定位：上述两个生成脚本分别在末尾连续 `save` 语句、两个分析入口的 `getflowfield([speicherstr 'rottransratio'])`；具体历史变量与 shape 见 [MAT_DATA_SCHEMA.md](../MAT_DATA_SCHEMA.md) 和 [TEST_PLAN.md §5.1](../TEST_PLAN.md)。**原因未定**，可能存在历史不同版本或外部加工，但没有证据不得指定成因，也不直接补造新结果、修改统计分母或覆盖归档。

## 7. H-08：582 MAT 的 SHA-256 独立复核与生产 Reader 拒绝清单（仅计划）

**现有 FNV-1a 64 位是非密码学文件指纹，不可被称为 SHA-256。** 复核计划：从固定 Git commit/树列出 582 个 `origin/**.mat` 的相对路径；将该列表按 UTF-8 路径和确定排序存入审计清单；独立工具对每个原始文件的原始字节流计算 **SHA-256**，为每项记录相对路径、字节大小、SHA-256、Git blob SHA、字典原 FNV64、核查者、环境、命令与完整日志。分别核对 560 个 Bias + 22 个 Adaptive；比对路径集合及 582 个唯一身份，重复/缺失/哈希改变一律记录失败；需要第二独立实现或复核者交叉确认。**本次未计算这些 SHA-256。**

拟定 Reader **拒绝或显式不支持**的样例（正式码按 contracts.md/DEC-008 批准）：损坏或错误 MAT 头、损坏 zlib 压缩、缺失变量、歧义的多变量兼容选择、MAT v7.3/HDF5（当前无必要支持）、MAT class/dtype 非实数 double 的 single/int/uint/logical/char/cell/struct/sparse/object/complex、未知 flags、rank/dims 非合同形状、维度乘积/字节数溢出、与预期变量名不符、数据长度不符。不要将这些类型静默转换为 double；若将来确需支持，另行立需求和测试。负例必须基于隔离副本，且需要 H-10 解除暂缓及相应运行/写入权限，不能直接改坏 `origin/**`。

## 8. H-09：历史 MATLAB 采集提案与现行 L2/L3/L4 参考替代（无需 MATLAB）

此节原是2026-11-06的H-09材料收集方案，现按已生效的[长期政策](../golden/NO_MATLAB_EXECUTION_POLICY.md)明确：**不得将采集脚本、MATLAB运行环境、原脚本逐节点输出或L1当作设计冻结/验收门禁**。下表保留当年的候选ID和测试追踪，仅将必需证据解释为可独立复核的L2归档最终数值、L3解析/符号或非被测实现以及L4统计；**表中的预期输出并非已经取得的参考值**。

| 参考组（历史候选 ID） | 固定输入 / 来源 | 现行独立参考目标（未生成时仅为候选） | 关联 TC |
|---|---|---|---|
| `G-MAT-01` | 原始 MAT 中选定文件、文件路径/变量名/版本 | dtype/rank/dims、列主序逐值、异常类型 | TC-F-MAT-001～004 |
| `G-KVD-01` | 固定 N×6 球面流场、Franz 初始化、Original/Modified 各自参数 | 每轮 `near,t,omega,error,x`、收敛/失败及原顺序 | TC-F-KVD-001/002 |
| `G-SH-01` | `r=I`、x/y/z 轴旋转、复合/逆旋转；九项固定系数 | 由只读`SHRotate(r,2)`/`transfor_koeff`公式与论文约定独立推导的R0/R3/R5和系数输出 | TC-F-SH-001/002、TC-F-TRAJ-001 |
| `G-AD-01` | `oneit_adaptiv_matched_filter.m` 固定 `altkoeff` 和 N×6 `setofpoints` | near、mean、M1…M4、右端、解、error、更新系数；异常提交时序 | TC-F-ADAPT-001～003 |
| `G-SAMPLE-01` | 固定级别/独立固定随机源；从只读`.m`静态登记`rand/randn`调用顺序 | 有序方向表、流场、噪声逐项与 RNG 调用次数 | TC-F-GEOM-002/003、TC-NF-RNG-001 |
| `G-AN-01` | 历史 MAT，四个原 MATLAB 分析入口，原脚本固定索引 | 数值曲线数组、x 范围、分段、平滑、有效样本数 | TC-F-AN-001/002 |

每组独立参考应按L2/L3/L4实际类型登记`golden_id`、固定输入路径/身份、原`.m` **Git blob SHA**、L2原MAT的SHA-256或L3参考生成器/数学证明与参考文件SHA-256、L4统计定义及样本分母（适用时）、执行者/日期/环境/命令、随机种子和调用顺序、dtype/rank/dims/单位、对应TC/AC、异常/无定义样本、来源版权/分发限制和审签状态；**不要求MATLAB采集脚本、MATLAB版本、L1逐轮数据或MATLAB运行许可**。小型可合法再分发的定值可建议存放 `tests/golden/fixtures/`；其他数据仅保存授权外部存储位置与 SHA-256 引用，不在 GitHub 仓库直接复制。**表内完整参考组尚未取得正式批准的L2/L3/L4验收证据；不要求MATLAB采集脚本或运行，不以被测C++自算输出充当独立参考。** 已独立记录的受限球谐符号检查和用户3/3 CTest只在各自范围有效，不能让其它候选组自动通过。

## 9. H-13：原 MATLAB 失败/状态提交行为的静态证据与待审决策

| 边界 | 已静态观察到的原源码表达式 | 尚不能从静态文本确认的运行行为 | 拟议 C++ 显式失败候选（未批准） |
|---|---|---|---|
| KvD 零向量 | `norm_koenderink_iteration.m` 第 31 行及第 85 行附近直接归一化 `trans/transneu`；`mue_norm_koenderink_iteration.m` 对应直接归一化 | 零范数时具体警告、非有限传播及 MATLAB 版本差异 | `ZeroNorm`/显式失败并留存输入，不能擅自把向量置零当成原结果 |
| KvD 迭代失败 | 原 while 条件同时包含 `x<200` 和最少 `x<6`，循环中按先旧平移求旋转、再提交新平移 | 非有限误差导致比较分支的结果、上限返回了什么、保留哪些近度 | 提案把“达到上限”和“非有限”分开记录，**不更改原停止顺序** |
| Adaptive 近度均值 | `oneit_adaptiv_matched_filter.m` 第 25～28 行先检查 `sum<0` 再 `near=near/sum` | `sum=0`、NaN/Inf 时 MATLAB 输出与后续步行为 | 候选 `ZeroNorm`/`NonFiniteInput`，但必须有独立L3负例和人类审定后才能冻结继续/失败语义 |
| Adaptive 6×6 | 同文件第 90～101 行 `invmgesamt=real(inv(mgesamt))`，随后乘右端并直接归一化 `transneu` | 奇异/病态矩阵是否警告、返回非有限、是否丢弃虚部以及对下游 Session 的影响 | 候选 `SingularSystem`/`IllConditionedSystem`/`NonFiniteResult`，不能未经批准换线性求解规则或报“等价” |
| Adaptive Session | 当前比较入口先进行两类初始化迭代、再以 `oneit` 更新主系数；源 MATLAB 直接赋值 | 一轮出错后是否停、继续沿用无效值、部分提交哪些数组 | 保持独立 StepSolver/Session 的候选分层，**继续/回滚/拒绝写状态必须另行人审** |

现行必须确认的证据：固定输入/源文件身份、从原静态控制流得到的状态更新顺序、独立L3定值与异常/非有限判别、未来真实C++的失败传播与Session前后状态。原MATLAB版本、stdout/warnings和真实运行的非有限传播**在没有原运行时只可写未知，且永久不作必需门禁**；不能仅据C++稳健性偏好宣称与作者原运行无条件等价。此节是**源码行为静态观察 + 执行候选**，非真实 NaN/Inf/奇异测试结果。

## 10. H-15：M0–M5 退出证据与资源清单（不承诺负责人/工期/性能）

| 里程碑 | 应收集的退出证据 | 当前允许的准备工作 / 阻塞 |
|---|---|---|
| M0 源与设计 | 45 文件逐项责任，582 文件身份，需求/AC/TC/P 映射，LLD/依赖，第三方权利与未决 DEC；人类审定具体 SHA | 可起草核对表；SRS/HLD/LLD 未冻结 |
| M1 MAT 与构建 | 生产 Reader/Writer、合法新结果写回读、无源覆盖负例、真实 CMake/CTest 日志 | 只读探针已有 582/582，其他均需解除 H-10/H-12 写入限制 |
| M2 Bias | 两类 Bias 的数值程序、固定采样和噪声 Golden、保存/分析数据与测试日志 | 合同可起草；生产程序与数值测试未授权 |
| M3 Adaptive | 完整 l≤2 球谐块、KvD/Adaptive 逐节点 Golden、Session 更新序列与真实 CTest | 方案 A 已选，依赖/权利/Golden 缺失 |
| M4 分析与曲线 | 四类分析入口的独立数值结果、历史字段差异处理、统计曲线 Golden | 可静态核对 MAT；绘图后端/数值测试未授权 |
| M5 可审核交付 | 所有相关 REQ/TC/AC 验收证据、环境与资源真实测量、版权告知、Draft PR、未决 DEC 列表 | 不得自动标记通过，不得合并、发布或创建项目根 LICENSE |

**资源来源**：H-04 的用户虚拟机快照记录 Linux x86_64、内存总量 8,127,380 KiB、当时 available 4,908,696 KiB、Swap 4,194,300 KiB、根分区可用 65,347,856 KiB。CPU 核数、编译器的该主机现测版本、项目壁钟/CPU/RSS、工作日程和负责人均**没有**由 JSON 具体给出；全部保持 TBD。不要把 `free` 的容量当成获批准的性能上限。

## 11. H-16：Draft PR / 发布前检查模板（仍不授权操作）

- [ ] 固定需审核的分支、HEAD commit SHA、差异文件清单、原始 `origin/**` 未修改证据；禁止强推、替换历史。
- [ ] 逐项附 REQ/AC/TC/P 映射、各项实际测试文件/CTest 运行证据、失败和未运行计数；没有则明确「缺失」。
- [ ] 原始 MAT 文件 SHA-256 独立复核、Golden 来源及批准容差、NaN/Inf/奇异与写保护负例（在授权后）归档。
- [ ] 只纳入 H-02/H-14 指定且核验的第三方文件，复用目录 README 注明上游路径与精确版本，保留适用 LICENSE/版权声明；项目根 LICENSE **不得**因本清单自动创建。
- [ ] 将 `output/` 真实生成结果排除 Git 跟踪并检查 `git diff --name-only`/暂存文件，不提交未许可数据或敏感执行记录。
- [ ] PR 保持 **Draft**；记录仍开放的 DEC、未知数值、当前工具环境与真实日志；合并/发布必须再次获人类明确批准。

## 12. 下一轮人类审批需要给出的具体答复

优先顺序建议：**(1)** H-05/H-06 的需求影响与接口边界审查 → **(2)** H-07/H-08 的 MAT 来源/身份与拒绝范围 → **(3)** H-09 的独立L2/L3/L4参考身份、生成/存储及批准规则 → **(4)** H-01/DEC-005 的实测误差与容差签署 → **(5)** H-03/DEC-008 的 CLI、错误码、CTest identity 冻结 → **(6)** 另行明确 H-10/H-11、MAT 写入和构建/测试的逐项执行权限。此顺序是**候选工作安排**，不能替代实际批准或重新写入已上传 JSON 的决议。

**停止条件**：若相应权限位仍为 false、源许可/Golden/作用 SHA 不确定，保持「未批准/未执行/未知」，继续文档审查，不将任何 P、TC、AC 或 DEC 直接关闭。
