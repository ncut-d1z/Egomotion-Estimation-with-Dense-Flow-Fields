# 审计计划与文件进度（AUDIT）

> 现行规则与唯一文件三态表。数学证明、来源复核、源代码审阅、实际编译/CTest及正式批准必须分开；已审计只代表该行限定范围完成。问题两态及完整AF义务见 [AUDIT_PROCESS](AUDIT_PROCESS.md)。

## 1. 规则与本轮边界

- 文件状态只用**空白 / 正在审计 / 已审计**；问题只用未整改/已整改。原复合AF的数学子义务有新证据，不等于整个AF可以关闭。
- **当前用户明确要求：只做理论推导，允许符号计算，禁止C++实现。** 本轮不修改C++、头文件、CMake、CTest、Bash、MAT或`origin/**`；既往球谐代码授权及用户已有3/3测试不构成本轮新增代码许可。
- 先锁定分支/提交，再核SRS↔HLD↔LLD、REQ↔AC↔TC↔P和数据/状态合同。来源与实测归PROGRESS；冻结身份归DOCUMENT_BASELINE_REGISTER；历史审批JSON和review不回写。
- 发现问题先登记路径/义务；在已有AF范围内的子问题不另造易结案小目标。取得实际修改和固定提交复核后方可按完整义务结案。
- SRS管理REQ/AC；HLD管理27条设计边；对象模型管理生命周期；LLD管理接口/数学；TEST_PLAN管理66TC；PLAN管理98P；新数学参考只映射既有TC，不新增正式TC。
- MATLAB/Octave/L1永久不列为必需门槛；源文件和MAT只读。符号计算与独立参考不是原MATLAB运行或生产C++/CTest，不能用其替代软件证据。
- 第一轮所有开放目标清零后，才进入用户条件性的第二轮全面审计。不得以局部SH或数学参考通过冒充全工程通过。

## 2. 历轮评审要点（28项保留）

| 编号 | 审计原则 |
|---|---|
| AU-001 | 文档分类、审查结构与正式基线分离 |
| AU-002 | SRS外部义务与附录REQ-DC分层 |
| AU-003 | AC冻结引用绑定章节/版本/SHA/批准 |
| AU-004 | HLD组件、部署、27边DAG、ADR、质量场景 |
| AU-005 | 对象所有权、生命周期、45个.m映射 |
| AU-006 | 582 MAT格式/类型/来源与逐值区分 |
| AU-007 | MatDenseArray自拥有、RAII、边界 |
| AU-008 | 严格/兼容读取、路径安全、无覆盖 |
| AU-009 | CLI精确类型及错误码单一来源 |
| AU-010 | 无自定义output-root，固定output/RunId |
| AU-011 | Result/Status/ErrorDetail、JSONL显式失败 |
| AU-012 | 五实验原更新顺序与save忠实 |
| AU-013 | Bias xwerte与N、RNG消耗公平性 |
| AU-014 | Franz/KvD逐轮轨迹、阈值及停止 |
| AU-015 | Adaptive M1～M4、MSE、Session状态提交 |
| AU-016 | Ivanic 1996/1998、Eigen-only、禁sdla |
| AU-017 | Sophus物理旋转、Rᵀ、epsilon/反向 |
| AU-018 | DEC-004当前生成/历史专属/缺字段 |
| AU-019 | 分析索引、分段/平滑、数值曲线 |
| AU-020 | 不运行MATLAB/L1；L2/L3/L4分级 |
| AU-021 | 0.01/0.1仅筛查；最终DEC-005待定 |
| AU-022 | 66TC实体/CTest证据真实性 |
| AU-023 | 98P、里程碑、角色与证据归口 |
| AU-024 | 统一版本/SHA/批准身份登记 |
| AU-025 | MIT与第三方notice边界 |
| AU-026 | 历史审批表认知负荷与权限边界 |
| AU-027 | 人类待办集中TODO，不重复决策 |
| AU-028 | 历轮回归扫描、风险分级 |

## 3. 审计对象与逐文件三态

保留本轮起点 `982537556be1ac5a414af368a0082806a9179d32` 的94个路径和各自状态；新增3份限定理论工件，均仅在数学复核范围内标为已审计。现有32份在审文件不因新增数学证明而自动关闭。旧的“新SH源/测试未运行”叙述属于历史时点，用户已在固定`5e522198`完成SH 3/3原生测试，不能继续当作当前缺口。

| 仓库根相对路径 | 状态 | 核对重点 |
|---|---|---|
| `docs/AUDIT.md` | 已审计 | AU规则、唯一文件三态、现行与历史统计分开 |
| `docs/AUDIT_PROCESS.md` | 已审计 | 唯一AF、完整目标、状态及数学子义务证据 |
| `docs/REQUIREMENTS.md` | 正在审计 | 58正文+8附录REQ、AC/DEC与正式冻结；AF-037 |
| `docs/PLAN.md` | 正在审计 | 98P静态追踪已核；真实DEC-006/性能仍缺；AF-061 |
| `docs/TEST_PLAN.md` | 正在审计 | SH 5e522198原生3/3已记录；新均值/几何/平移归一数学专题映射既有Adaptive TC，软件及正式阈值仍缺；AF-051 |
| `docs/design/README.md` | 已审计 | LLD索引限定静态范围，非全软件通过 |
| `docs/DOCUMENT_BASELINE_REGISTER.md` | 正在审计 | 固定36项历史身份与实时集合分离；正式签署AF-058 |
| `docs/PROGRESS.md` | 正在审计 | 用户SH原生及独立数学结果分SHA记录；全工程/资源/批准仍缺；AF-053 |
| `docs/ARCHITECTURE.md` | 正在审计 | 27条设计边、真实部署与质量场景；AF-038 |
| `docs/OBJECT_MODEL.md` | 正在审计 | 生命周期、45.m职责与批准；叶子不冒充公共类；AF-039 |
| `docs/design/contracts.md` | 正在审计 | 公共ABI/异常AF-040，本轮不实施代码 |
| `docs/design/cli.md` | 正在审计 | 参数、RunId、JSONL与真实负例；AF-041 |
| `docs/design/mat_io.md` | 正在审计 | Reader/Writer、RAII、TOCTOU、安全与L2；AF-042 |
| `docs/design/kvd.md` | 正在审计 | 36轮独立数学trace和首翻号已有；一般/病态/200轮、正式DEC-005和生产CTest仍缺；AF-043 |
| `docs/design/adaptive_mfa.md` | 正在审计 | 对称及非对称正/负均值参考已有；本轮均值零/抵消/正深度窄锥/零raw平移专题另列；Session、软件负例与批准仍缺；AF-044 |
| `docs/design/harmonics.md` | 正在审计 | 九基/旋转/二阶投影已有独立数学及5e522198原生证据；公共ABI/动态阶数/正式DEC仍缺；AF-045 |
| `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 正在审计 | 公共接口/声明适配及冻结仍缺；AF-046 |
| `docs/design/sampling_noise.md` | 正在审计 | 714行有序候选及独立逐坐标核查完成；生产采样/噪声CTest、异常/RNG仍缺；AF-047 |
| `docs/design/trajectory.md` | 正在审计 | 源反向/TR-C候选，完整独立参考/CTest仍缺；AF-048 |
| `docs/design/experiments.md` | 正在审计 | 五入口/save/Manifest/聚合，真实Runner仍缺；AF-049 |
| `docs/design/analysis_plotting.md` | 正在审计 | 曲线、分段/平滑、历史字段，完整L3/CTest仍缺；AF-050 |
| `docs/decisions/DEC-004_HISTORICAL_MAT_FIELDS.md` | 已审计 | 已选来源规则，不重复审批 |
| `docs/decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md` | 已审计 | 保留当时所有者决定 |
| `docs/golden/NO_MATLAB_EXECUTION_POLICY.md` | 已审计 | L1永久非必需 |
| `docs/golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | 正在审计 | 逐AC数学/软件/批准分层；本轮无量纲与谱例不能替代正式阈值；AF-060 |
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
| `docs/reviews/DOCUMENT_REVIEW_AND_SH_DECLARATIONS_2026-11-07.md` | 已审计 | 历史声明级记录 |
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
| `verification/adaptive/verify_zero_mean_conditioning_families.wl` | 已审计 | 本轮25项精确恒等式/谱及70位SVD检查通过；禁止以此冒充生产测试 |
| `verification/adaptive/zero_mean_conditioning_evidence.v1.json` | 已审计 | 实际25/25结果、六个SVD样例及定义域/物理与有符号输入范围 |
| `docs/design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md` | 已审计 | 限定理论专题：raw/翻号身份、s=0、正尺度/抵消/正深度窄锥、纯旋转raw平移零及诊断边界 |

## 4. 本轮结果与仍开放义务

- **问题两态：97个唯一AF＝74已整改＋23未整改。** 没有新增易结案AF，也没有减少原复合义务。
- **文件三态：97个唯一对象＝65已审计＋32正在审计＋0空白。** 本轮起点94/62/32/0；新增三份数学工件限定审阅，不代表生产接口冻结。
- 本轮证明的四类独立失败/风险：s=0使源均值归一无定义；正近度同比缩小时M可保持条件数1但raw平移变小；有符号抵消时M行列式及全部特征值恒定而条件数发散；正近度四视线窄锥在s=1下仍有kappa2=h^-4；纯旋转可在M良态时给出零raw平移。参阅[均值与条件数专题](design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md)。
- AF-044/051仍需真实软件负例/逐节点、Session和动态情形；AF-054正式DEC-005阈值、AF-055/060完整参考/AC的L2/L3/L4与批准仍缺。证明参数族不是全输入的浮点正确性证明。
- 第一轮23项仍开放，因此未启动条件性第二轮全面审计。本轮不要求用户重跑原SH构建，因为相关源/测试/脚本没有修改。

## 5. 固定历史证据

为避免旧计数和旧NOT_RUN叙述重复占据现行摘要，较早详细过程保留在不可变Git快照中：[本轮起点完整AUDIT](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/982537556be1ac5a414af368a0082806a9179d32/docs/AUDIT.md)。该文件的全部94条路径/状态及28个AU原则已在本版保留；本轮仅增加三条数学对象并更新当前叙述。更早固定来源、Wolfram与用户运行细节由 [PROGRESS](PROGRESS.md)、[AUDIT_PROCESS](AUDIT_PROCESS.md) 的历史链接追踪，不改写旧审批或Git历史。
