# 低阶球谐三路线一致性专项审计（AUDIT）

> **当前唯一有效审计范围，由用户明确指定。** 本文件及 [AUDIT_PROCESS.md](AUDIT_PROCESS.md) 是专项工作计划，不是整个 MATLAB→C++ 移植工程的全面审计计划。本轮只修订这两个文件中的目标、规则与跟踪口径；**不开展任何具体审计、推导、符号计算、代码修改或测试**。

## 1. 唯一审计目的与范围硬边界

**唯一目的**：核对论文 *Insect-Inspired Self-Motion Estimation with Dense Flow Fields—An Adaptive Matched Filter Approach* 实际依赖的**低阶实球谐函数**计算公式及其附带 MATLAB 实现，确认三条相互独立的数学路线结果一致；发现差异时定位到准确的公式、相位、下标、旋转约定和 MATLAB 源码位置。

**只允许以下三条路线相互核对**：

| 路线 | 核对对象 | 明确的结论目标 |
|---|---|---|
| A：直接定义 | 从球谐函数定义与实基转换直接求 `l=0,1,2` 的九个实球谐函数及其旋转作用 | 独立解析基准，非以被测递推结果反推参考 |
| B：实球谐递推 | Ivanic–Ruedenberg 1996 算法、1998 勘误及原 MATLAB `SHRotate.m` 的 `M/U/P/V/W` | 求得的 `l=0/1/2` 旋转块与 A 的定义结果一致 |
| C：复球谐递推 | Choi 等 1999 年的复球谐递推，经复基→实基转换后得到实系数旋转 | 转换后与 A、B 的 `l=0/1/2` 结果一致 |

**必须核对的 MATLAB 内容**：`origin/egomotion_estimation_dense_flow_fields_matlab_scripts/help_functions/kugelflfunc2.m` 的 `legendre`（含 Condon–Shortley 相位）、归一化、`m` 符号分支；`SHRotate.m` 及文件内部 `M/U/P/V/W` 的递推、系数排列、矩阵转置/方向。只在确定这些关系时引用论文的定义、勘误及三条公式；允许给出明确约定变换，但不得把约定不同直接判作算法错误。

**仅限阶数 `l=0,1,2`**：九个实基、(R_0)（1×1）、(R_1)（3×3）、(R_2)（5×5）；必要时核对一般 (SO(3)) 旋转和明确的独立特例，不能用某个 z 轴90度特例代替一般证明。历史符号验证和原生球谐测试可复用为证据，但其适用范围必须明确。

### 1.1 明确禁止扩大研究与整改范围

**以下内容一律不属于本专项，禁止审计、推导、生成新的参考/测试，禁止借“数学上值得研究”名义重新加入**：

- KvD、Franz 运动估计、Adaptive MFA 的 M1～M4、6×6运动矩阵、near 正/负/零均值、Session、状态提交、收敛性、退化可观测性。
- 球面采样/噪声/RNG、有序采样点扩展、MAT/CLI/Runner/实验/分析绘图、资源性能和全工程 SRS/HLD/66TC/98P/批准治理。
- `koeff_aus_near.m` / `near_aus_koeff.m` 的**离散重新拟合、保正、幂等、最小二乘或逆问题**；这些即使使用球谐基也不是三路线基函数/旋转递推一致性的必要证明。
- 任意高阶球谐数值稳定性、全 IEEE 754 边界、算法替代或工程重构，除非用户随后单独明确授权。

**范围判断规则**：每个新问题必须能直接映射到 A/B/C 中的某个 `l<=2` 基函数或旋转公式，或上述两份 MATLAB 文件的确切行/函数；否则不得登记为专项目标，不得创建审计工件，不得扩大文件清单。用户再次扩大授权之前，本专项的范围变更为**禁止**。

### 1.2 执行规则与本轮限制

- 本轮仅修改 `docs/AUDIT.md` 和 `docs/AUDIT_PROCESS.md`；本轮**不实际进行上述 A/B/C 核验**，不调用 Wolfram 执行新证明，不修改其他文件、不写 C++、不运行 CMake/CTest/MATLAB。
- 后续如用户允许具体审计，优先**复用已有论文、原 MATLAB、Wolfram与独立参考**；仅在三路线结论存在准确未闭合差异时才补充最少量符号计算。不得为了增加已审计文件数量而创建无关证明或新台账。
- MATLAB 文件保持只读；MATLAB/Octave 运行不列为必要前提。符号计算、公式审计、数值专项 CTest 是不同证据，不能互相冒充。
- 文献、论文印刷/勘误问题、实基相位、坐标/旋转约定须明确区分；不得暗改原论文或 MATLAB 算法。
- 使用 [AUDIT_PROCESS](AUDIT_PROCESS.md) 的**六个专项目标**作为结案唯一依据。每一目标只能在其完整公式/代码对应及结论明确后标“已整改”；未核验的继续“未整改”。不能把原97个跨模块整改目标的任何一项作为本专项阻塞。
- **第二轮规则**：只有六个专项整改目标全部明确结案，才可对这13个专项文件进行限定范围的第二次复核；不得自动跳转到整个项目的“第二轮全面审计”。
- 此处的“已审计”标记继承此前限定历史审核记录，**不代表本轮实际开展核验或三路线总目标已通过**。

## 2. 专项唯一审计对象与文件状态

| 仓库根相对路径 | 状态 | 本专项唯一关联 |
|---|---|---|
| `docs/AUDIT.md` | 已审计 | 本专项范围、禁止扩张、只读/符号及结案规则；本轮仅修订范围 |
| `docs/AUDIT_PROCESS.md` | 已审计 | 六项唯一专项整改目标、非专项历史隔离；本轮仅修订目标 |
| `docs/design/harmonics.md` | 正在审计 | 仅九个实球谐基值、相位/排列和与 MATLAB 一致性的既有论证 |
| `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 正在审计 | 仅 Ivanic–Ruedenberg 的 R0/R1/R2、1998 勘误及 M/U/P/V/W 递推公式 |
| `docs/golden/SH_PAPER_VERIFICATION_CASES.md` | 已审计 | 复用已有的低阶三路线验证候选；不新增外围 Golden |
| `docs/reviews/SH_PAPERS_SOURCE_AUDIT_2026-11-09.md` | 已审计 | 历史论文、勘误与原 MATLAB 来源核查；复查目标仅限球谐 |
| `docs/reviews/LOW_ORDER_SH_THREE_ROUTE_VERIFICATION_2026-11-09.md` | 已审计 | 历史三路线证明证据；本轮仅定义结案核对任务 |
| `verification/harmonics/verify_low_order_basis.wl` | 已审计 | 直接定义九实基和低阶解析恒等式的历史符号输入 |
| `verification/harmonics/verify_low_order_rotations.wl` | 已审计 | 低阶一般SO(3)三路线旋转块的历史精确符号输入 |
| `verification/harmonics/verify_low_order_layout.wl` | 已审计 | 复/实与 MATLAB 系数排列、相位及旋转作用约定的历史验证 |
| `verification/harmonics/verification_results_2026-11-09.json` | 已审计 | 固定历史 Wolfram 检查数据；不冒充本轮重新执行 |
| `origin/egomotion_estimation_dense_flow_fields_matlab_scripts/help_functions/kugelflfunc2.m` | 空白 | 专项待核：MATLAB legendre 相位、归一化与全部九个函数 |
| `origin/egomotion_estimation_dense_flow_fields_matlab_scripts/help_functions/SHRotate.m` | 空白 | 专项待核：degree=0/1/2、M/U/P/V/W、转置方向和实系数作用 |

**专项现行文件口径：13个唯一文件＝9已审计＋2正在审计＋2空白。** 其中两份原 MATLAB 文件标空白，表示下一轮才进行针对三路线的一致性审阅；不据此断言其源码错误。此表仅含范围直接相关文件，不引入全工程登记或新增模块。

## 3. 审计完成标准

最终报告必须清楚给出：九个 `Y_{lm}` 的 A/B/C 对照结果、`R_0/R_1/R_2` 一般旋转一致性、复实基转换/索引和相位映射、`kugelflfunc2.m` 及 `SHRotate.m`（M/U/P/V/W）的逐式核对、论文勘误或源码差异的明确裁定、已证范围及未证边界。**允许明确发现错误或不足，禁止为了宣称一致而忽略反例。** 不要求全工程实现、对其它算法的证明或新平台部署。

本轮的完成标准仅为：**两份范围文件内容一致、仅两文件发生修改、专项与历史计数不混用**；不是三路线审计工作的数学结案。

## 附录 A：原全工程审计表（历史存档，不属于本专项）

以下原100个对象及其原有状态，取自本轮开始前固定提交 [`c38b4b6`](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/c38b4b63307c5f3febf2bee0aac95b565f44a46c/docs/AUDIT.md)。**下表仅供追溯，绝不可与上面的13个专项对象合并计数，也不得据此继续发起全工程审计。**这些历史状态没有被宣布完成或取消；本轮只是将其从现行专项的工作范围移出。

保留本轮起点 `21ad0c34f3c43fd1bbf33c847406dfd302e239ea` 的97个路径和各自状态；新增3份离散SH重新拟合理论工件，均仅在声明的数学复核范围内标为已审计。现有32份在审文件不因新增数学证明而自动关闭。用户已在固定`5e522198`完成SH 3/3原生测试；本轮新数学反例不抹去该来源忠实性运行证据，也不扩大为该版本生产资格。

| 仓库根相对路径 | 状态 | 核对重点 |
|---|---|---|
| `docs/AUDIT.md` | 已审计 | AU规则、唯一文件三态、现行与历史统计分开 |
| `docs/AUDIT_PROCESS.md` | 已审计 | 唯一AF、完整目标、状态及数学子义务证据 |
| `docs/REQUIREMENTS.md` | 正在审计 | 58正文+8附录REQ、AC/DEC与正式冻结；AF-037 |
| `docs/PLAN.md` | 正在审计 | 98P静态追踪已核；真实DEC-006/性能仍缺；AF-061 |
| `docs/TEST_PLAN.md` | 正在审计 | SH 5e522198原生3/3已记录；数学专题映射既有SH/Adaptive TC，软件及正式阈值仍缺；AF-051 |
| `docs/design/README.md` | 已审计 | LLD索引限定静态范围，非全软件通过 |
| `docs/DOCUMENT_BASELINE_REGISTER.md` | 正在审计 | 固定36项历史身份与实时集合分离；正式签署AF-058 |
| `docs/PROGRESS.md` | 正在审计 | 用户SH原生及独立数学结果分SHA记录；全工程/资源/批准仍缺；AF-053 |
| `docs/ARCHITECTURE.md` | 正在审计 | 27条设计边、真实部署与质量场景；AF-038 |
| `docs/OBJECT_MODEL.md` | 正在审计 | 生命周期、45.m职责与批准；叶子不冒充公共类；AF-039 |
| `docs/design/contracts.md` | 正在审计 | 公共ABI/异常AF-040，本轮不实施代码 |
| `docs/design/cli.md` | 正在审计 | 参数、RunId、JSONL与真实负例；AF-041 |
| `docs/design/mat_io.md` | 正在审计 | Reader/Writer、RAII、TOCTOU、安全与L2；AF-042 |
| `docs/design/kvd.md` | 正在审计 | 36轮独立数学trace和首翻号已有；一般/病态/200轮、正式DEC-005和生产CTest仍缺；AF-043 |
| `docs/design/adaptive_mfa.md` | 正在审计 | 均值/条件数参考已在独立专题；新增C_new的离散往返/保正及末端分母专题；Session、软件负例与批准仍缺；AF-044 |
| `docs/design/harmonics.md` | 正在审计 | 原定义/旋转/投影有数学及原生证据；本轮离散逐项拟合的精确逆/保正前提见新专题，不等同基函数或C++实现错误；AF-045 |
| `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 正在审计 | 公共接口/声明适配及冻结仍缺；AF-046 |
| `docs/design/sampling_noise.md` | 正在审计 | 714行有序核查完成；新增离散采样与SH阶数相容性反例，生产采样/噪声CTest、异常/RNG仍缺；AF-047 |
| `docs/design/trajectory.md` | 正在审计 | 源反向/TR-C候选，完整独立参考/CTest仍缺；AF-048 |
| `docs/design/experiments.md` | 正在审计 | 五入口/save/Manifest/聚合，真实Runner仍缺；AF-049 |
| `docs/design/analysis_plotting.md` | 正在审计 | 曲线、分段/平滑、历史字段，完整L3/CTest仍缺；AF-050 |
| `docs/decisions/DEC-004_HISTORICAL_MAT_FIELDS.md` | 已审计 | 已选来源规则，不重复审批 |
| `docs/decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md` | 已审计 | 保留当时所有者决定 |
| `docs/golden/NO_MATLAB_EXECUTION_POLICY.md` | 已审计 | L1永久非必需 |
| `docs/golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | 正在审计 | 逐AC数学/软件/批准分层；新离散反例不替代正式阈值；AF-060 |
| `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | 正在审计 | 精确等式及高精度SVD不等于最终double容差；AF-054 |
| `docs/golden/analytic_golden_candidates.v1.json` | 正在审计 | 原AN-G01～12保持DRAFT，完整来源/软件/批准未齐；AF-055 |
| `docs/TODO.md` | 正在审计 | 用户既有SH实测与本轮禁止C++分开；其它授权/批准仍缺；AF-057 |
| `docs/MAT_DATA_SCHEMA.md` | 正在审计 | 582 MAT独立SHA-256/逐值仍缺；AF-056 |
| `docs/SOURCE_INVENTORY.md` | 已审计 | 45源文件静态身份，不等于软件验收 |
| `docs/MIGRATION_MATRIX.md` | 已审计 | 45文件/50函数历史映射；叶子与公共适配分开 |
| `docs/approval/DECISION_IMPLEMENTATION_NOTES.md` | 已审计 | 历史审批不回写 |
| `docs/approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md` | 已审计 | 历史审批包不回写 |
| `docs/reviews/STATIC_TRACEABILITY_2026-11-06.md` | 已审计 | 固定历史静态记录 |
| `docs/reviews/DOC_CONSISTENCY_RECONCILIATION_2026-11-06.md` | 已审计 | 固定历史静态记录 |
| `docs/reviews/LLD_MAT_IO_AND_CONSISTENCY_2026-11-07.md` | 已审计 | 固定历史静态记录 |
| `docs/reviews/LLD_CONTRACTS_TARGETS_TESTS_ALIGNMENT_2026-11-07.md` | 已审计 | 固定历史静态记录 |
| `docs/reviews/OWNER_TECH_DECISIONS_ALIGNMENT_2026-11-07.md` | 已审计 | 固定历史决定对齐 |
| `docs/reviews/DOCUMENT_REVIEW_AND_SH_DECLARATIONS_2026-11-07.md` | 已审计 | 歷史声明级记录 |
| `docs/reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md` | 已审计 | 历史全量扫描，不冒充当前HEAD |
| `docs/reviews/AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md` | 已审计 | 历史整改证据 |
| `docs/reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md` | 已审计 | 历史整改证据 |
| `docs/reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md` | 已审计 | 历史增量扫描 |
| `docs/reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md` | 已审计 | 历史整改证据 |
| `docs/reviews/AUDIT_SEVEN_MATH_DOCS_SCAN_2026-11-08.md` | 已审计 | 历史数学文档扫描 |
| `docs/reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md` | 已审计 | 历史限定修订 |
| `docs/reviews/AUDIT_FOUR_POLICY_DRIFTS_SCAN_2026-11-08.md` | 已审计 | 历史决策漂移扫描 |
| `docs/reviews/AN_G_STATIC_FORMULA_REVIEW_2026-11-08.md` | 已审计 | 历史静态代数，非正式验收 |
| `docs/reviews/AUDIT_FUNCTION_DECLARATIONS_AND_LEDGER_COMPACT_2026-11-08.md` | 已审计 | 45.m/50函数及治理压缩历史 |
| `docs/reviews/AN_G08_Z90_STATIC_CONVENTION_2026-11-08.md` | 已审计 | z90限定特例 |
| `docs/reviews/AUDIT_ANALYSIS_SEGMENT_SMOOTHING_CORRECTION_2026-11-08.md` | 已审计 | 旧窗口差异断言修正 |
| `docs/reviews/AUDIT_ANALYSIS_ERROR_MAPPING_SCAN_2026-11-08.md` | 已审计 | AF-077发现证据 |
| `docs/reviews/AN_G_LEVEL0_FACE_ORDER_STATIC_2026-11-08.md` | 已审计 | 初始面有序符号推导 |
| `docs/reviews/AN_G_RECURSION_INDEX_STATIC_2026-11-08.md` | 已审计 | A/B/C/D索引历史证明 |
| `docs/reviews/AUDIT_BIAS_XWERTE_LAST_RUN_STATIC_2026-11-08.md` | 已审计 | Bias末轮x/均值y审查 |
| `AGENTS.md` | 已审计 | 既有规则服从最新本轮理论限定 |
| `README.md` | 已审计 | 原项目范围；当前证据归PROGRESS |
| `LICENSE` | 已审计 | MIT与上游作品边界 |
| `.gitignore` | 已审计 | 不绕过输出/构建排除 |
| `CMakeLists.txt` | 已审计 | 5e522198下原3项SH配置已实测；本轮未修改/执行 |
| `build.sh` | 已审计 | 5e522198一次verbose运行已实测；本轮未修改/执行 |
| `scripts/check_docs_traceability.sh` | 已审计 | 既有源码静态范围；本轮未运行 |
| `docs/reviews/SH_PAPERS_SOURCE_AUDIT_2026-11-09.md` | 已审计 | 三PDF/勘误/相位固定记录 |
| `docs/golden/SH_PAPER_VERIFICATION_CASES.md` | 已审计 | 25公式/10用例设计，后续实测另记 |
| `docs/decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md` | 已审计 | 既往仅SH授权；本轮最新指令禁止任何C++实现 |
| `include/egomotion/harmonics/low_order.hpp` | 正在审计 | 局部API已有构建实证，公共ABI未冻结 |
| `src/harmonics/low_order_detail.hpp` | 正在审计 | 校验/舍入边界未全面覆盖 |
| `src/harmonics/low_order_basis.cpp` | 正在审计 | 旧固定SHA实测，完整异常边界未完 |
| `src/harmonics/low_order_rotation.cpp` | 正在审计 | 一般数学及旧SHA实测，公共接口/边界未完 |
| `src/harmonics/low_order_choi.cpp` | 正在审计 | 复递推一般证明及旧SHA实测，全面边界未完 |
| `src/harmonics/low_order_source_projection.cpp` | 正在审计 | 5e522198专项76检查通过，非完整公共Fitter/ABI |
| `tests/unit/test_low_order_harmonics.cpp` | 正在审计 | 旧SHA 53,532检查/624四元数实测，正式容差/更广输入仍缺 |
| `tests/unit/test_low_order_archived_golden.cpp` | 正在审计 | 5e522198归档176检查通过，完整L3/批准仍缺 |
| `tests/unit/test_low_order_source_projection.cpp` | 正在审计 | 5e522198投影76检查通过，公共API/其它场景仍缺 |
| `verification/geometry/verify_source_direction_levels.wl` | 已审计 | 独立9组行序加权签名历史证据 |
| `verification/geometry/verify_full_source_direction_rows.wl` | 已审计 | 714行/2142坐标独立逐值核对，最大差1.11e-16 |
| `verification/geometry/full_ordered_row_verification.v1.json` | 已审计 | 全行结果、Git blob及SHA-256，非正式Golden |
| `verification/adaptive/verify_AN_G12_symmetric_one_step.wl` | 已审计 | 对称20方向独立数学单步 |
| `verification/adaptive/AN_G12_symmetric_one_step_evidence.v1.json` | 已审计 | AN-G12数值及正则后近度非严格1 |
| `verification/adaptive/verify_asymmetric_signed_near_one_step.wl` | 已审计 | 九方向非对称正/负均值独立数学单步，非生产CTest |
| `verification/adaptive/asymmetric_signed_near_one_step.v1.json` | 已审计 | 12/12数学检查、源身份及2I差异，非正式Golden |
| `verification/kvd/verify_six_direction_exact_trace.wl` | 已审计 | 精确Wolfram四条六轮轨迹、14/14检查 |
| `verification/kvd/six_direction_exact_trace.v1.json` | 已审计 | 24轮独立数学数据及源身份 |
| `verification/kvd/verify_near_sign_flip_exact_trace.wl` | 已审计 | 八方向首次翻号两条六轮轨迹、11/11检查 |
| `verification/kvd/near_sign_flip_exact_trace.v1.json` | 已审计 | 12轮控制流数学数据，非正距离物理例 |
| `verification/geometry/source_direction_levels_0_2.v1.json` | 已审计 | 固定714行与独立Wolfram逐元素核对、身份已核 |
| `verification/harmonics/verify_low_order_basis.wl` | 已审计 | 九基/递推/Gram/Laplacian/dyadic证明 |
| `verification/harmonics/verify_low_order_rotations.wl` | 已审计 | 一般四元数SO(3)、两递推各34项精确零差 |
| `verification/harmonics/verify_low_order_layout.wl` | 已审计 | 任意系数同场性、往返和旧相位反例 |
| `verification/harmonics/verify_archived_sh_candidates.wl` | 已审计 | AN-G06/07/08限定符号证明 |
| `verification/harmonics/verify_source_projection_order2.wl` | 已审计 | 七方向投影精确九系数和非零交叉Gram |
| `verification/harmonics/verification_results_2026-11-09.json` | 已审计 | 当时Wolfram输出/固定输入及运行限制 |
| `verification/harmonics/source_projection_reference.v1.json` | 已审计 | 固定七方向数学参考，原历史状态不回写 |
| `docs/reviews/LOW_ORDER_SH_THREE_ROUTE_VERIFICATION_2026-11-09.md` | 已审计 | 三路一般证明、论文印刷差异、实际运行边界 |
| `verification/adaptive/verify_zero_mean_conditioning_families.wl` | 已审计 | 前轮25项精确恒等式/谱及70位SVD检查通过；非生产测试 |
| `verification/adaptive/zero_mean_conditioning_evidence.v1.json` | 已审计 | 前轮25/25结果、六个SVD样例及声明定义域 |
| `docs/design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md` | 已审计 | 前轮限定理论：s=0、正尺度/抵消/窄锥、纯旋转raw平移零 |
| `verification/harmonics/verify_discrete_refit_roundtrip.wl` | 已审计 | 本轮35项实际Wolfram检查；源离散往返、满秩正near反例、互异方向与零通道分母；非软件验证 |
| `verification/harmonics/discrete_refit_roundtrip_evidence.v1.json` | 已审计 | 35/35结果及输入构造、精确−9/140、互异方向有理符号证书、源blob与执行脚本SHA-256 |
| `docs/design/SH_DISCRETE_REFIT_CONSISTENCY.md` | 已审计 | 限定理论专题：离散G对角充要条件、P幂等、正值反例与末端分母；不改变原算法/批准软件资格 |

## 附录 B：旧统计与证据入口（历史，不作本专项进度）

原全工程审计有100个路径，68已审计、32正在审计、0空白；原全工程整改有97个目标，74已整改、23未整改。以上数字**仅是旧口径冻结快照**，不再作为低阶球谐专项的当前绩效统计。此前Wolfram、数学模型与用户运行证据可在原固定提交及[PROGRESS.md](PROGRESS.md)查阅；本轮没有执行任何数学或软件审计。
