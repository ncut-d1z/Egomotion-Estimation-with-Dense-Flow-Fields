# 文档审计计划与文件进度（AUDIT）

> **范围**：本仓库规范、设计、测试与治理文件的静态审阅；**不是**软件实现、编译、数值验收或正式批准。SRS/HLD/LLD 均属未冻结草案。最新状态以 §3 的文件三态和 [AUDIT_PROCESS](AUDIT_PROCESS.md) 的问题两态为准。

## 1. 规则

- 文件状态只用 **空白 / 正在审计 / 已审计**；「已审计」仅指约定范围的**文档静态审阅**，不表示实现/数学/CTest 通过。
- 发现问题即在 [AUDIT_PROCESS.md](AUDIT_PROCESS.md) **逐目标登记相对路径、具体目标、未整改**；实际改源文件、在新固定提交复读确认后才能改为「已整改」。一文件多目标分行。
- 先核对当前分支与固定提交，再查 SRS↔HLD↔LLD、REQ↔AC↔TC↔P、链接/版本、数据与错误合同；在 [PROGRESS.md](PROGRESS.md) 记证据，在 [DOCUMENT_BASELINE_REGISTER.md](DOCUMENT_BASELINE_REGISTER.md) 登记需冻结的文件身份。历史审批 JSON 和 `reviews/` 不回写。
- 遵守 [AGENTS.md](../AGENTS.md) 与 [TODO.md](TODO.md) 的授权：`origin/**` 只读，当前不运行 Bash/CMake/CTest/MATLAB/Octave、不写 MAT、不擅自创建生产 C++ 或签署基线；L1 永久非必需。
- **规范归口**：SRS 管 REQ/AC；HLD/ADR 管架构、27 边设计 DAG；对象模型管生命周期；LLD 管接口/算法；TEST_PLAN 管 66 TC；PLAN 管 98 P；PROGRESS 管实测；版本矩阵管 SHA/签署；TODO 管人类延期事项。
- **持续整改**：开放目标不意味着停止推进；继续修订可由文档/只读源码核对完成的部分，及时提交。需要 H-10/H-11、论文、真实 L2/L3/CTest 或批准的目标继续标「未整改」，绝不假冒验收。只有第一轮目标实际清零后才开展用户要求的第二轮正式审计。

## 2. 历轮评审要点（AU-001～028，压缩索引）

| 编号 | 审计原则 |
|---|---|
| AU-001 | 文档分类、审查结构与正式基线分离 |
| AU-002 | SRS 外部义务与附录 REQ-DC 分层 |
| AU-003 | AC 冻结引用须绑定章节/版本/SHA/批准 |
| AU-004 | HLD 组件、部署、27 边 DAG、ADR、质量场景 |
| AU-005 | 对象所有权、生命周期、45 个 .m 映射 |
| AU-006 | 582 MAT 格式/类型/来源与逐值区分 |
| AU-007 | MatDenseArray 自拥有、RAII、边界 |
| AU-008 | 严格/兼容读取与路径安全、无覆盖 |
| AU-009 | CLI 精确类型及错误码单一来源 |
| AU-010 | 取消 --output-root，固定 output/<RunId>/ |
| AU-011 | Result/Status/ErrorDetail、JSONL 显式失败 |
| AU-012 | 五种实验、原更新顺序与 save 忠实 |
| AU-013 | Bias xwerte 与 N、RNG 消耗公平性 |
| AU-014 | Franz/KvD 分轮轨迹、阈值与停止 |
| AU-015 | Adaptive M1～M4、MSE、Session 状态提交 |
| AU-016 | Ivanic 1996/1998、Eigen-only、禁 sdla |
| AU-017 | Sophus 物理旋转、Rᵀ、epsilon/反向 |
| AU-018 | DEC-004 CurrentGenerated/HistoricalOnly/缺字段 |
| AU-019 | 分析索引、分段、平滑、数值曲线 |
| AU-020 | 不运行 MATLAB/L1；L2/L3/L4 分级 |
| AU-021 | 0.01/0.1 仅首轮筛查；最终 DEC-005 待定 |
| AU-022 | 66 TC 测试实体/CTest 证据真实性 |
| AU-023 | 98 P、里程碑、角色与证据归口 |
| AU-024 | 统一版本/SHA/批准身份登记 |
| AU-025 | 项目 MIT 与第三方来源 notice 界限 |
| AU-026 | 历史审批表认知负荷与权限边界 |
| AU-027 | 人类延期输入集中 TODO；不重复决策 |
| AU-028 | 历轮残留回归扫描和低/中/高分级 |

## 3. 审计对象分类与逐文件进度

| 类别 | 适用规范与结论 |
|---|---|
| SRS | `REQUIREMENTS.md`；结构完整，AC/批准及冻结仍缺 |
| HLD | `ARCHITECTURE.md` + `OBJECT_MODEL.md`；27 边是设计 DAG，非实际构建 |
| LLD | `design/contracts.md`、`design/cli.md`、各模块 LLD/专题；签名与数学候选，非可编译基线 |
| 测试与 Golden | `TEST_PLAN.md`、`golden/*`；66 TC 与 L2/L3/L4 尚待真实核验 |
| 治理/历史 | `PLAN.md`、`PROGRESS.md`、`DOCUMENT_BASELINE_REGISTER.md`、两台账、`TODO.md`、`reviews/*`；按各自用途评判，历史资料不可回写 |



| 仓库根相对路径 | 状态 | 核对重点 |
|---|---|---|
| `docs/AUDIT.md` | 已审计 | 精简后保留 28 AU、54 文件三态及当前结论，重复历史记录由 reviews/PROGRESS 追溯 |
| `docs/AUDIT_PROCESS.md` | 已审计 | 压缩为 75 条唯一 AF 两态与真实缺口，历史详细证据保留 Git/reviews |
| `docs/REQUIREMENTS.md` | 正在审计 | SRS 的 58 条正文 REQ + 8 条附录 REQ-DC、AC/DEC；待 AF-037 |
| `docs/PLAN.md` | 正在审计 | AF-052 WBS 静态追踪已复核；DEC-006 实测/批准仍待 AF-061 |
| `docs/TEST_PLAN.md` | 正在审计 | 66 TC（含 8 TC-DC）→候选测试→AC；AF-063 已整改，AF-051 仍待真实 CTest |
| `docs/design/README.md` | 已审计 | AF-059 已补各模块冻结前检查清单；仅静态合同检查，不代表实际冻结或测试 |
| `docs/DOCUMENT_BASELINE_REGISTER.md` | 正在审计 | 36 项固定快照身份与正式冻结状态；AF-036 已整改，待 AF-058 ；AF-071 章节编号已复核；AF-058 的正式批准/身份更新仍开放 |
| `docs/PROGRESS.md` | 正在审计 | 真实证据与动态 DEC/里程碑；待 AF-053；AF-070 已定向复核；AF-053 真实证据待补 |
| `docs/ARCHITECTURE.md` | 正在审计 | HLD/27 条设计边、部署/Writer；待 AF-038 |
| `docs/OBJECT_MODEL.md` | 正在审计 | 生命周期、45 .m 映射；待 AF-039 |
| `docs/design/contracts.md` | 正在审计 | Result/Status/ErrorDetail、所有权；待 AF-040；待 AF-077 AnalysisErrorCode 到 CLI 的单一候选映射 |
| `docs/design/cli.md` | 正在审计 | 参数类型、RunId、JSONL/错误；待 AF-041 |
| `docs/design/mat_io.md` | 正在审计 | MAT 类型、RAII、Reader/Writer/安全；AF-033 已整改，待 AF-042 |
| `docs/design/kvd.md` | 正在审计 | Franz/KvD 公式、逐轮顺序；待 AF-043 |
| `docs/design/adaptive_mfa.md` | 正在审计 | M1～M4、MSE、Session；待 AF-044 |
| `docs/design/harmonics.md` | 正在审计 | 九项实球谐、R3/R5、Eigen；待 AF-045 |
| `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 正在审计 | 论文/勘误和声明级边界；待 AF-046 |
| `docs/design/sampling_noise.md` | 正在审计 | 有序采样与 RNG 消耗；待 AF-047 |
| `docs/design/trajectory.md` | 正在审计 | Sophus、Rᵀ、近零、球内近度；待 AF-048 |
| `docs/design/experiments.md` | 正在审计 | 五入口、Runner/Manifest/save；AF-034 已整改，待 AF-049 |
| `docs/design/analysis_plotting.md` | 正在审计 | 四种分析、曲线、HistoricalOnly；待 AF-050；AF-075/076 已完成静态设计复核；AF-050 仍需独立 L3/真实 CTest |
| `docs/decisions/DEC-004_HISTORICAL_MAT_FIELDS.md` | 已审计 | DEC-004 现行处理已选 |
| `docs/decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md` | 已审计 | 所有者已选技术方向 |
| `docs/golden/NO_MATLAB_EXECUTION_POLICY.md` | 已审计 | L1 永久不必需 |
| `docs/golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | 正在审计 | L2/L3/L4 与 AC 映射；待 AF-060 |
| `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | 正在审计 | 探索筛查与最终 DEC-005；待 AF-054 |
| `docs/golden/analytic_golden_candidates.v1.json` | 正在审计 | AN-G01～12 独立 L3 参考核验；待 AF-055 |
| `docs/TODO.md` | 正在审计 | 分阶段人类输入与授权；待 AF-057 |
| `docs/MAT_DATA_SCHEMA.md` | 正在审计 | 582 MAT 字典与历史证据；待 AF-056 |
| `docs/SOURCE_INVENTORY.md` | 已审计 | 45 个源 .m 路径、Git blob、字节与 REQ/TC 的静态身份清单；非软件验收 |
| `docs/MIGRATION_MATRIX.md` | 已审计 | 45 个源文件/50 个函数定义的声明级追踪已固定复核；完整数学/实现测试待 AF-039/045 |
| `docs/approval/DECISION_IMPLEMENTATION_NOTES.md` | 已审计 | 历史权限/决定释义 |
| `docs/approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md` | 已审计 | 历史审批包，保留当时事实 |
| `docs/reviews/STATIC_TRACEABILITY_2026-11-06.md` | 已审计 | 历史审计/修订记录（只读） |
| `docs/reviews/DOC_CONSISTENCY_RECONCILIATION_2026-11-06.md` | 已审计 | 历史审计/修订记录（只读） |
| `docs/reviews/LLD_MAT_IO_AND_CONSISTENCY_2026-11-07.md` | 已审计 | 历史审计/修订记录（只读） |
| `docs/reviews/LLD_CONTRACTS_TARGETS_TESTS_ALIGNMENT_2026-11-07.md` | 已审计 | 历史审计/修订记录（只读） |
| `docs/reviews/OWNER_TECH_DECISIONS_ALIGNMENT_2026-11-07.md` | 已审计 | 历史审计/修订记录（只读） |
| `docs/reviews/DOCUMENT_REVIEW_AND_SH_DECLARATIONS_2026-11-07.md` | 已审计 | 历史审计/修订记录（只读） |
| `docs/reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md` | 已审计 | 历史审计/修订记录（只读） |
| `docs/reviews/AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md` | 已审计 | 历史审计/修订记录（只读） |
| `docs/reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md` | 已审计 | 历史审计/修订记录（只读） |
| `docs/reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md` | 已审计 | 历史审计/修订记录（只读） |
| `docs/reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md` | 已审计 | 历史审计/修订记录（只读） |
| `docs/reviews/AUDIT_SEVEN_MATH_DOCS_SCAN_2026-11-08.md` | 已审计 | 历史审计/修订记录（只读） |
| `docs/reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md` | 已审计 | 历史审计/修订记录（只读） |
| `docs/reviews/AUDIT_FOUR_POLICY_DRIFTS_SCAN_2026-11-08.md` | 已审计 | 历史审计/修订记录（只读） |
| `docs/reviews/AN_G_STATIC_FORMULA_REVIEW_2026-11-08.md` | 已审计 | 8 组候选局部代数手算复核，另外 4 组未独立核验；非 L3/CTest PASS |
| `docs/reviews/AUDIT_FUNCTION_DECLARATIONS_AND_LEDGER_COMPACT_2026-11-08.md` | 已审计 | 45 个 .m/50 个函数定义与台账膨胀的静态发现；非数值测试 |
| `docs/reviews/AN_G08_Z90_STATIC_CONVENTION_2026-11-08.md` | 已审计 | z90 特例的 R3/R5、m 次序及系数旋转方向代数核对；非通用递推或 L3 PASS |
| `docs/reviews/AUDIT_ANALYSIS_SEGMENT_SMOOTHING_CORRECTION_2026-11-08.md` | 已审计 | 修正旧扫描报告关于窗口必不同的断言，逐域确认旋转/平移图差异；不代表 L3/CTest |
| `docs/reviews/AUDIT_ANALYSIS_ERROR_MAPPING_SCAN_2026-11-08.md` | 已审计 | 分析错误枚举未纳入跨模块统一退出码的静态发现（AF-077） |
| `AGENTS.md` | 已审计 | 仓库授权与数据安全规则 |
| `README.md` | 已审计 | 项目范围、证据/许可状态 |
| `LICENSE` | 已审计 | MIT 与上游作品边界 |
| `.gitignore` | 已审计 | 固定 output/ 忽略规则 |
| `scripts/check_docs_traceability.sh` | 已审计 | 静态追踪脚本，仅源码审查 |



## 4. 最新审核结论与尚缺证据

- 当前固定扫描：`150b975db4045fbc99a343fdf422c7836e1a797b`，并参考 [函数级声明检查](reviews/AUDIT_FUNCTION_DECLARATIONS_AND_LEDGER_COMPACT_2026-11-08.md)。共 **57 个审计路径**，其中 **34 已审计、23 正在审计、0 空白**；这些三态仅表示限定静态范围，不能代替软件/数学/批准状态。
- 既有设计为**评审草案**；58 条正文 REQ＋8 条附录 REQ-DC＝66 REQ，对应 66 TC；98 P、原 45 个 .m、HLD 静态 27 边。已建立源 [45 文件/50 函数定义的迁移矩阵](MIGRATION_MATRIX.md)：SHRotate 主函数＋M/U/P/V/W 五个内部函数，**仅声明级审查，不是代码/数值验收**。
- 历史 MAT 由独立 matio 探针报告 582/582 READ_OK；另将 582 Git MAT 路径/字节与既有字典作静态逐项交叉核对（0 个目录/元数据差异），但**没有**完成独立 SHA-256、逐元素 L2、Reader/Writer/TOCTOU、逐轮及退化 L3、必要 L4 或正式 CTest。候选 `AN-G01～12` 仍为 `DRAFT_UNAPPROVED_UNEXECUTED`；小范围手算及 AN-G08 的 z90 符号特例不代表一般 R5/Golden 通过。
- 永久不要求 MATLAB/Octave/L1。物理旋转路线 Sophus 已选；R3/R5 的 Eigen-only 实现须核 Ivanic–Ruedenberg 1996/1998 原文、禁 sdla。首轮 `atol=0.01, rtol=0.1` 已选，**最终 DEC-005/006/008、H-10/H-11 与正式冻结/发布尚未通过**。
- 本轮待整改目标的**路径、具体义务和两态**只见 [AUDIT_PROCESS.md](AUDIT_PROCESS.md)。其中 23 个现存阶段门槛需要真实软件/独立参考/审批；不可仅修改文字就关闭。

## 5. 历史审计与版本来源

历史 AF-001～071 的原扫描、修订顺序和固定提交由 Git 历史及 [FULL_DOCUMENT_AUDIT](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md)、[七数学文档复核](reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md)、[AF-028～031 扫描](reviews/AUDIT_FOUR_POLICY_DRIFTS_SCAN_2026-11-08.md)、[函数声明审查](reviews/AUDIT_FUNCTION_DECLARATIONS_AND_LEDGER_COMPACT_2026-11-08.md) 保留；真实过程证据/适用权限见 [PROGRESS.md](PROGRESS.md) 与 [TODO.md](TODO.md)，冻结身份见 [DOCUMENT_BASELINE_REGISTER.md](DOCUMENT_BASELINE_REGISTER.md)。旧审计或审批的时间点事实不应改写为当前 HEAD 的通过记录。


## 6. AF-072～074 本轮复核结论（2026-11-08）

- 先在 `d40b149` 登记三项目标为未整改，后修改源文件：函数级映射提交 `150b975`，两台账精简提交 `ae831be3a0901d69c0897044fd339fda38468963`；两次独立静态复读确认 45 文件/50 函数（含 5 个局部辅助）、28 个 AU、54 个审计文件与 75 个唯一 AF 均完整。
- **AF-072～074 在文本/函数声明范围内已整改**。本轮追踪 **54 文件：31 已审计、23 正在审计、0 空白**；其余 23 个阶段目标仍需独立 L2/L3/L4、实际 C++/CTest、论文材料/DEC 批准或正式签署，不能因清单存在而关单。
- 本次没有修改 `origin/**`、运行 MATLAB/Octave、Bash、CMake、CTest、MAT 写回读，也没有批准正式软件基线。


## 7. 新增分析流程审计（2026-11-08，先登记）

在固定提交 `02d59d7a118870073bdad3b043b1fd01df7f9628` 上只读核对原始 `graph_agent_in_sphere_adaptive_matched_filter_test.m`、`graph_agent_in_sphere_adaptive_matched_filter_many_parameter.m` 和 `kurveglaetten.m`。发现图形路径**不可把旋转/平移误差通用化成同一分段和平滑策略**：

- **AF-075**：多更新率旋转误差对每个 begin/end 段独立平滑；第 1 条原曲线不平滑，后三条分别半窗 2/5/10，绘制 begin+f…end−f。现有分析 LLD 只笼统介绍平滑范围，需写清**每段重新开始的窗口**，不能拿全长数据平滑后再裁剪。
- **AF-076**：比较实验的平移误差图完全不使用 begin/end；多更新率平移误差则先在全 600 行上平滑，再提取 x=50:550，不随旋转误差四段切片。需补两脚本的差异和不同错误/测试断言。

先在 [整改台账](AUDIT_PROCESS.md) 登记未整改；原有 AF-050/051 因缺独立 L3/真实 Analyzer/CTest 仍开放。详细扫描见 [审查报告](reviews/AUDIT_ANALYSIS_SEGMENT_SMOOTHING_2026-11-08.md)。


## 8. AF-075/076 定向文档合同结案（2026-11-08）

先在 `8730c631` 登记 `analysis_plotting.md` 源码差异为两项「未整改」，再于 `98ff092` 修订分析 LLD v0.6、TEST_PLAN v0.12 及独立纠偏报告，`d8e7e64` 修复 §5.2.1 章节位置并在该提交定向复读：旋转**按区间局部平滑和裁剪**、平移**完整 600 行平滑和连续 501 点**、comparison 平移**不分段**与完整临时数组的独立断言均已明确。旋转正确裁剪内部窗口与全域平滑对应窗口的数值等价也已写清，**不能以假的差异判据充当测试通过**。本次只修订算法与测试设计，故 AF-075/076 在**文字合同范围已整改**；AF-050/051 仍需真实 L2/L3、Analyzer/CTest，保持未整改。


## 9. 跨模块分析错误码扫描（2026-11-08，先登记）

- 输入 `4ffbbf93d0acf8177bbc4d9dfd28ed6aaaef197b`。核对 `docs/design/analysis_plotting.md §6` 的 `AnalysisErrorCode` 七个候选枚举与 `docs/design/contracts.md §3.1` 统一退出码表：后者只覆盖 Mat/Numerical/Experiment，分析层的 MissingResult/MissingVariable/ShapeMismatch/InvalidSegment/NonFiniteInput/UndefinedStatistic/UnsupportedExperiment 尚缺明确且单一的映射。见 [静态扫描报告](reviews/AUDIT_ANALYSIS_ERROR_MAPPING_SCAN_2026-11-08.md)。
- 新增 AF-077 先记「未整改」；后续应由 contracts 维护候选 stage/exit，分析与 CLI LLD 只引用而不复制，并把缺字段、坏形状、非法区间的 TC 检查点写清。DEC-008 正式批准、真实 JSONL/CTest 仍独立开放。
