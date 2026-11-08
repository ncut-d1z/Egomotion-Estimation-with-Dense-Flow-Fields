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
| `docs/AUDIT.md` | 已审计 | 审计规则、AU 原则及逐文件三态；**对象数以本节现行路径行数为准**，AF-082 已静态复核 |
| `docs/AUDIT_PROCESS.md` | 已审计 | 逐 AF 唯一目标与整改两态；**当前目标数以整改台账现行 AF 行数为准**，保留历史 reviews 证据 |
| `docs/REQUIREMENTS.md` | 正在审计 | SRS 的 58 条正文 REQ + 8 条附录 REQ-DC、AC/DEC；待 AF-037 |
| `docs/PLAN.md` | 正在审计 | AF-052 WBS 静态追踪已复核；DEC-006 实测/批准仍待 AF-061 |
| `docs/TEST_PLAN.md` | 正在审计 | 66 TC（含 8 TC-DC）→候选测试→AC；AF-063 已整改，AF-051 仍待真实 CTest |
| `docs/design/README.md` | 已审计 | AF-078：实时审计数改由 AUDIT 维护，AnalysisErrorCode 定义与退出映射归属已对齐；非软件验收 |
| `docs/DOCUMENT_BASELINE_REGISTER.md` | 正在审计 | AF-079：固定 36 文件历史快照与实时审计集合分离；AF-058 正式身份/批准仍未完 |
| `docs/PROGRESS.md` | 正在审计 | 真实证据与动态 DEC/里程碑；待 AF-053；AF-070 已定向复核；AF-053 真实证据待补 |
| `docs/ARCHITECTURE.md` | 正在审计 | HLD/27 条设计边、部署/Writer；待 AF-038 |
| `docs/OBJECT_MODEL.md` | 正在审计 | 生命周期、45 .m 映射；待 AF-039 |
| `docs/design/contracts.md` | 正在审计 | Result/Status/ErrorDetail、所有权；待 AF-040；AF-077 已定向复核（候选映射）；AF-040 仍待实际 ABI/错误运行验证 ；AF-084 绘图四错误映射已复读；AF-040 真实 Result/ABI 仍未完成 |
| `docs/design/cli.md` | 正在审计 | 参数类型、RunId、JSONL/错误；待 AF-041 |
| `docs/design/mat_io.md` | 正在审计 | MAT 类型、RAII、Reader/Writer/安全；AF-033 已整改，待 AF-042 |
| `docs/design/kvd.md` | 正在审计 | Franz/KvD 公式、逐轮顺序；待 AF-043 |
| `docs/design/adaptive_mfa.md` | 正在审计 | M1～M4、MSE、Session；待 AF-044 |
| `docs/design/harmonics.md` | 正在审计 | 九项实球谐、R3/R5、Eigen；待 AF-045 |
| `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 正在审计 | 论文/勘误和声明级边界；待 AF-046 |
| `docs/design/sampling_noise.md` | 正在审计 | 有序采样与 RNG 消耗；待 AF-047 |
| `docs/design/trajectory.md` | 正在审计 | AF-080 严格反向源返回已明确，TR-C01～04 静态候选已补；AF-048 仍缺独立 L3/CTest |
| `docs/design/experiments.md` | 正在审计 | 五入口、Runner/Manifest/save；AF-034 已整改，待 AF-049 |
| `docs/design/analysis_plotting.md` | 正在审计 | 四种分析、曲线、HistoricalOnly；待 AF-050；AF-075/076 已完成静态设计复核；AF-050 仍需独立 L3/真实 CTest ；AF-083 原文已复核，AF-050 实际数值/测试仍未通过 |
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
| `docs/reviews/AN_G_LEVEL0_FACE_ORDER_STATIC_2026-11-08.md` | 已审计 | 初始 8/6/20 面有序单位中心的符号级静态推导；非 level1/2 独立 L3/CTest |
| `docs/reviews/AN_G_RECURSION_INDEX_STATIC_2026-11-08.md` | 已审计 | AF-081：A/B/C/D 子面顶点与深度优先索引公式；仅静态/符号合同，不是完整 L3 |
| `docs/reviews/AUDIT_BIAS_XWERTE_LAST_RUN_STATIC_2026-11-08.md` | 已审计 | Bias 多次聚合仅 y 求平均、x 取最后 k 的静态源码审计；非 L2/L3/CTest |
| `AGENTS.md` | 已审计 | 仓库授权与数据安全规则 |
| `README.md` | 已审计 | 项目范围、证据/许可状态 |
| `LICENSE` | 已审计 | MIT 与上游作品边界 |
| `.gitignore` | 已审计 | 固定 output/ 忽略规则 |
| `scripts/check_docs_traceability.sh` | 已审计 | 静态追踪脚本，仅源码审查 |



## 4. 现行审计摘要与尚缺证据

- **现行统计（随台账更新）**：本文件 §3 的逐文件表共有 **60 个唯一仓库根相对路径**：**37 已审计、23 正在审计、0 空白**；[AUDIT_PROCESS.md](AUDIT_PROCESS.md) 的唯一 AF 行在 AF-085 固定提交复读并结案后共有 **90 项：67 已整改、23 未整改**（AF-085～089 的固定提交复核见整改台账 §21～25）。后续数字若变化，以两份台账**当前实际唯一行与状态**为准，不以本段文字作第二份状态源；每次审计结案应同步回填现行摘要。
- **历史固定证据（不可冒充最新 HEAD）**：`150b975db4045fbc99a343fdf422c7836e1a797b` 当时为 **58 个路径、35 已审计、23 正在审计、0 空白**，详见 [函数级声明检查](reviews/AUDIT_FUNCTION_DECLARATIONS_AND_LEDGER_COMPACT_2026-11-08.md)。后续已新增审阅对象，故该数字仅是固定历史快照；所有文件三态仅表示限定静态审阅，不代替软件/数学/批准状态。
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


## 10. AF-077 跨模块分析错误候选映射结案（2026-11-08）

- 先以 `4ffbbf93` 静态审计 `AnalysisErrorCode` 的 7 种状态与 `contracts.md §3.1` 统一退出码的缺口，在 `20b5fcaa` 登记未整改，再以固定提交 `e3c4c679` 复读 `contracts.md` v0.4、`analysis_plotting.md` v0.7、`cli.md` v0.5、`TEST_PLAN.md` v0.13：七种分析错误均对应单一候选 stage/exit，并区分非法命令参数 parse(2) 与 Analyzer 运行时不支持 analyze(5)，无重复 enum。
- **AF-077 在文档合同一致性范围已整改**；尚须 DEC-008/P0.8 的人类批准、真实 JSONL/CTest、底层错误传播与输入负例，AF-040/041/050/051 保持未整改。当前 **78 个目标中 55 已整改、23 未整改**；审计文件 57 路径（34 已审计、23 正在审计）。


## 11. 最新跨文档索引漂移（2026-11-08，先登记）

- 固定扫描输入 `5bc71e6ceca2de9f746a2c911a135b56db2cccbd`；[LLD 索引](design/README.md) 的职责描述仍说 AUDIT“目前 50 个路径”，并将公共错误来源概括为 Mat/Numerical/Experiment，未同步已出现的 `AnalysisErrorCode`→[contracts §3.1](design/contracts.md) 候选映射（AF-078）。
- [版本控制矩阵](DOCUMENT_BASELINE_REGISTER.md) §1 将 AUDIT 的 50 条当成现行范围，实际现行审计已跟踪 57 条；需只将**动态范围引用**改为 AUDIT 单一来源，不得改写 §2～3 36 项固定文件快照或历史某次的 50 条（AF-079）。
- 新发现已在 [AUDIT_PROCESS.md](AUDIT_PROCESS.md) 登记「未整改」；本轮不改变任何 REQ/AC/TC/算法或正式冻结结论。


## 12. 轨迹反向退化的确定性行为（先登记）

固定输入 `02a2a17fb49c911cdadd978033d14127cf48122d`：源 `drehvektor_aus_zwei_vektoren.m:8–16` 对严格反向非零向量求得叉积零、`acos(-1)=π`，从而实际返回**零旋转向量**。现 `trajectory.md §5` 称“可能输出零”，容易弱化这一确定性的原版异常行为；应显式记录精确向量例、继发轨迹/旋转后果，并将原 MATLAB 数学返回与候选 C++ 安全失败区分，关联原 `TC-F-TRAJ-001`（AF-080）。


## 13. AF-078～080 定向整改与复读（2026-11-08）

- 首先登记三个问题：动态审计文件数被 LLD 索引及版本矩阵重复写死（AF-078/079）；源严格反向旋转的**确定零向量**被描述为“可能”（AF-080）。随后 `02a2a17` 修改索引与版本矩阵，`07d3abb` 修改轨迹 LLD v0.4、TEST_PLAN v0.14。
- 固定复读确认两份索引以 AUDIT 为当前统计单一来源、36 项历史文件身份不改；轨迹修订给出严格反向 `0·π=0` 以及 Rᵀ 三点、单位球正根四组手算输入，全部只作静态候选。AF-078～080 **文本一致性整改完成**，未执行独立 L3/CTest 或批准任何新安全阈值。
- **当前三态：57 文件，34 已审计、23 正在审计；两态：81 个目标，58 已整改、23 未整改。** AF-048/037/058 等正式测试、数值与签署门槛继续开放。


## 14. 初始面行序的静态对照补记（2026-11-08）

依据原 `erzeugerichtungsvektoren*.m`、`richtungsvektoren_icosahedron.m`、`findeDreiecke.m` 与现有 AN-G JSON，已在 [符号级审查报告](reviews/AN_G_LEVEL0_FACE_ORDER_STATIC_2026-11-08.md) 按面表核对 AN-G01/02/03 的 `level=0` **8/6/20** 行方向与次序。固定修改提交 `844b647` 的静态校验核对符号类别及行序，未执行采样器或 MATLAB，level1/2 与独立 L3 尚缺。

该批报告作为第 58 个审计对象：**35 已审计、23 正在审计**。AF-047/055 仅新增静态候选证据，正式数值/CTest/批准缺口不变；当前 81 项中 58 已整改、23 未整改。


## 15. 深层球面递归行号的静态审计发现（2026-11-08）

- **固定输入**：`08de8d7af114afd70b078e88917c3434882ac032`；只读对照 `help_functions/findeDreiecke.m` 与完整八面体、缺损八面体、二十面体三个采样入口，以及 [零级审计报告](reviews/AN_G_LEVEL0_FACE_ORDER_STATIC_2026-11-08.md)。
- **AF-081 新发现**：当前采样 LLD 已规定 `A→B→C→D`，但没有明确每一级的三个子三角顶点次序、深度优先的四进制路径到 MATLAB 1-based 行号的转换，以及缺损面 `2/3/5/6/7/8` 的“原面编号”和“输出块序号”之区别。新增 [整改台账](AUDIT_PROCESS.md) 先登记「未整改」，再补充精确索引规则和测试资格。
- **边界**：不运行原 MATLAB，不生成 level=1/2 全量数值 L3、不改 AN-G JSON，也不将采样器/CTest/DEC-005 记为通过。原有 AF-047、AF-055 继续未整改。


## 16. AF-081 递归行号合同复读结论（2026-11-08）

- 在发现登记提交 `a0f52a6` 后，实际修订并于固定 `4f9e48252f8be2994c67e09aefd4c036b38e9f68` 逐项复核：原 `findeDreiecke` 四个子面顶点/顺序、停止条件、`L=1/2` 的行号公式与五组首末索引、原文件身份、三个相关文档引用与仅静态边界，**10/10 检查通过**。
- 文件级进度现为 **59 项：已审计 36、正在审计 23、空白 0**。新报告列入三态清单；原采样 LLD、TEST_PLAN、Golden 即使本次文字修订完仍需 AF-047/051/054/055 的真实数值/软件证据，维持「正在审计」。
- **整改项目两态**：AF-081 只关闭独立的递归**索引合同**缺口，当前共 **82 项：已整改 59、未整改 23**；不能把符号/行号证明说成 level=1/2 三维独立 L3、任何 TC PASS 或基线批准。


## 17. AUDIT 自身计数残留的再审计（2026-11-08）

- 本轮输入 `669ef118f04e76d5ae8c31b3177b2aff05b9c7fc`。§3 的 `docs/AUDIT.md` 与 `docs/AUDIT_PROCESS.md` 两行仍将**历史计数 54 文件/75 条 AF**写成当前文件描述，已与本次 59 文件/82 项整改不一致；该问题不同于已关闭的 AF-078（LLD 索引）。
- 新编号 **AF-082**，先在 [AUDIT_PROCESS](AUDIT_PROCESS.md) 登记「未整改」，目标是移除自我介绍中的动态硬编码计数，改为由 §3 实际行数和 AF 台账实时确定状态；历史有日期/固定输入的旧轮次数字继续保留，不追溯改写。


**AF-082 现行计数口径**：§3 的状态数字由文件表的唯一相对路径统计；[整改台账](AUDIT_PROCESS.md) 的问题数字由唯一 AF-ID 的当前状态统计。这里的文件用途描述**不再写死动态数量**。历史各轮含固定提交日期的 54/57/58 等旧数字仍只是当时事实，不应悄然改写为最新口径。此处尚须在新固定提交核对后才可将 AF-082 标为「已整改」。


## 18. AF-082 动态计数回归整改完成（2026-11-08）

原扫描 `669ef11` 发现表内仍有“54 文件/75 条 AF”作为**现行**说明；先在 `f29e406` 登记「未整改」，再以 `8cd782b2f2558edf7cb51c101a40117b80e61dcc` 的静态复核确认两行已改为从 AUDIT 当前相对路径与 AUDIT_PROCESS 当前唯一 AF-ID 计算，**7/7 定向检查通过**（含保留历史轮次原数字、路径/AF 唯一性）。这不是改写历史数值。

**目前整改项目 83 项：60 已整改、23 未整改；审计对象 59 个：36 已审计、23 正在审计、0 空白。** 真正的 L2/L3/L4、软件/CTest、P0 基线/DEC 仍缺，不因此宣布软件完成或启动正式第二轮全面审计。


## 19. 新增 Bias 历史聚合合同缺口（AF-083，先登记）

在固定输入 `19157a21221a8fde4447e3c96ff97a8da1a5746f` 静态对照 `origin/.../bias_test/data_analysis/graphs_bias_test.m` 与 [分析 LLD](design/analysis_plotting.md) §5、[测试计划](TEST_PLAN.md) 时发现：`k=1:number_of_cores` 中 `ywerte3…6` 的四组矩阵累加并在循环后除以 `number_of_cores`，但 `xwerte` 在**每次循环直接覆写**，最终绘图只使用**最后一个 k 的 xwerte**，不求 x 均值。现 LLD 的 Bias 聚合行及 APL-G01 还未明确此单侧更新顺序。整改在 [AUDIT_PROCESS](AUDIT_PROCESS.md) 先登记为「未整改」；既有阶段目标 AF-050 的独立 L2/L3/CTest 仍开放。


## 20. AF-083 定向结案与 AF-084 新发现（2026-11-08）

- **AF-083 已整改（仅合同文本）**：先于 `8f193ae` 登记原分析脚本的 `xwerte` 最后 k 覆写语义，再在 `ab9470936eeea1a536709e05a909bee6256b1168` 修改分析 LLD v0.8、TEST_PLAN v0.17 和 [静态审阅报告](reviews/AUDIT_BIAS_XWERTE_LAST_RUN_STATIC_2026-11-08.md)。固定提交上的 **9/9 静态合同断言**及本地链接检查通过。仅四组 `ywerte3…6` 跨 k 求均值、x 与理论参考始终用末轮 `xwerte`；AF-050/051 的 L2/L3/Analyzer/CTest 仍未完成。
- **AF-084 新发现，先登记未整改**：`analysis_plotting.md` §2 候选接口定义 `PlotErrorCode::{BackendUnavailable,InvalidSeries,OutputForbidden,WriteFailed}`；[contracts.md §3.1](design/contracts.md) 只用笼统“可选 plot 后端不存在→exit7”覆盖一种来源，未给另外三种稳定 `ErrorStage/exit`。这与“公共错误映射唯一来源”的设计承诺不完整；须同步 `contracts.md`、分析 LLD、CLI 与原 66 TC 对应 TEST_PLAN，不得凭文档宣布 DEC-008 或 CTest 已批准。
- 本轮状态（AF-083 结案、AF-084 登记）：**85 个目标：61 已整改、24 未整改**；新增一份 Bias 原文复核文件后 **60 个审计对象：37 已审计、23 正在审计、0 空白**。新的 AF-084 结案前始终保持「未整改」。


## 21. AF-084 绘图错误的单一映射定向复核（2026-11-08）

- 在首次登记 AF-084 为「未整改」后，实际修订 `docs/design/contracts.md` v0.5、`docs/design/analysis_plotting.md` v0.9、`docs/design/cli.md` v0.6、`docs/TEST_PLAN.md` v0.18、`docs/design/README.md` v0.9。初次固定复读发现 `WriteFailed` 只作了简写，随即在下一单独提交补成 `PlotErrorCode::WriteFailed`，**失败检查后修复才结案**。
- 固定复核提交 `597efb7f44e135fc2bc4b03ba3d31dffe734d3b1`：**10/10 静态合同断言及相对链接检查通过**，`BackendUnavailable→plot/7`、`InvalidSeries→plot/5`、`OutputForbidden/WriteFailed→output/6`；全部四个枚举身份与分析 LLD 的唯一声明相符，CLI 只引用 `contracts.md §3.1`，既有 TC 增补四类未来负例。
- **当前审计三态**：60 文件，37 已审计、23 正在审计、0 空白；**问题两态**：85 项，62 已整改、23 未整改。AF-084 **仅文本/候选映射已整改**；原 AF-040、041、050、051 等真实 ABI、CLI、绘图/分析 CTest、DEC-008 等仍开放，不能误作正式验收。正式第二轮全量审计仍须等待完整第一轮目标实际闭环。
