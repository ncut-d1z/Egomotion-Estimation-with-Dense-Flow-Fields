# 文档审计计划与文件进度（AUDIT）

> **范围**：本仓库规范、设计、测试、治理文件的静态一致性审计；**不是**编译、数值验证、正式签署或软件验收。现有结构较完整，但 SRS/HLD/LLD 均为**未冻结草案**。**当前：50 文件（已审计 25、正在审计 25），67 目标（已整改 38、未整改 29；本轮新问题尚待修订）**。

## 1. 规则

- 文件状态只用 **空白 / 正在审计 / 已审计**；「已审计」仅指约定范围的**文档静态审阅**，不表示实现/数学/CTest 通过。
- 发现问题即在 [AUDIT_PROCESS.md](AUDIT_PROCESS.md) **逐目标登记相对路径、具体目标、未整改**；实际改源文件、在新固定提交复读确认后才能改为「已整改」。一文件多目标分行。
- 先核对当前分支与固定提交，再查 SRS↔HLD↔LLD、REQ↔AC↔TC↔P、链接/版本、数据与错误合同；在 [PROGRESS.md](PROGRESS.md) 记证据，在 [DOCUMENT_BASELINE_REGISTER.md](DOCUMENT_BASELINE_REGISTER.md) 登记需冻结的文件身份。历史审批 JSON 和 `reviews/` 不回写。
- 遵守 [AGENTS.md](../AGENTS.md) 与 [TODO.md](TODO.md) 的授权：`origin/**` 只读，当前不运行 Bash/CMake/CTest/MATLAB/Octave、不写 MAT、不擅自创建生产 C++ 或签署基线；L1 永久非必需。
- **规范归口**：SRS 管 REQ/AC；HLD/ADR 管架构、27 边设计 DAG；对象模型管生命周期；LLD 管接口/算法；TEST_PLAN 管 66 TC；PLAN 管 98 P；PROGRESS 管实测；版本矩阵管 SHA/签署；TODO 管人类延期事项。

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
| `docs/AUDIT.md` | 正在审计 | 审计规则、AU 主题、逐文件三态；新增 AF-064～066 待修 |
| `docs/AUDIT_PROCESS.md` | 已审计 | AF 目标和两态；历史已结案 |
| `docs/REQUIREMENTS.md` | 正在审计 | SRS 的 58 条正文 REQ + 8 条附录 REQ-DC、AC/DEC；待 AF-037 |
| `docs/PLAN.md` | 正在审计 | WBS 98 P、依赖和里程碑；待 AF-052、AF-061 |
| `docs/TEST_PLAN.md` | 正在审计 | 66 TC（含 8 条 TC-DC）→候选测试→AC；AF-063 已改源待静态复核，AF-051 仍待真实测试 |
| `docs/design/README.md` | 正在审计 | LLD 索引、单一来源；AF-062 已整改，待 AF-059 |
| `docs/DOCUMENT_BASELINE_REGISTER.md` | 正在审计 | 36 项固定快照身份与正式冻结状态；AF-036 已整改，待 AF-058 |
| `docs/PROGRESS.md` | 正在审计 | 真实证据与动态 DEC/里程碑；待 AF-053 |
| `docs/ARCHITECTURE.md` | 正在审计 | HLD/27 条设计边、部署/Writer；待 AF-038 |
| `docs/OBJECT_MODEL.md` | 正在审计 | 生命周期、45 .m 映射；待 AF-039 |
| `docs/design/contracts.md` | 正在审计 | Result/Status/ErrorDetail、所有权；待 AF-040 |
| `docs/design/cli.md` | 正在审计 | 参数类型、RunId、JSONL/错误；待 AF-041 |
| `docs/design/mat_io.md` | 正在审计 | MAT 类型、RAII、Reader/Writer/安全；AF-033 已整改，待 AF-042 |
| `docs/design/kvd.md` | 正在审计 | Franz/KvD 公式、逐轮顺序；待 AF-043 |
| `docs/design/adaptive_mfa.md` | 正在审计 | M1～M4、MSE、Session；待 AF-044 |
| `docs/design/harmonics.md` | 正在审计 | 九项实球谐、R3/R5、Eigen；待 AF-045 |
| `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 正在审计 | 论文/勘误和声明级边界；待 AF-046 |
| `docs/design/sampling_noise.md` | 正在审计 | 有序采样与 RNG 消耗；待 AF-047 |
| `docs/design/trajectory.md` | 正在审计 | Sophus、Rᵀ、近零、球内近度；待 AF-048 |
| `docs/design/experiments.md` | 正在审计 | 五入口、Runner/Manifest/save；AF-034 已整改，待 AF-049 |
| `docs/design/analysis_plotting.md` | 正在审计 | 四种分析、曲线、HistoricalOnly；待 AF-050 |
| `docs/decisions/DEC-004_HISTORICAL_MAT_FIELDS.md` | 已审计 | DEC-004 现行处理已选 |
| `docs/decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md` | 已审计 | 所有者已选技术方向 |
| `docs/golden/NO_MATLAB_EXECUTION_POLICY.md` | 已审计 | L1 永久不必需 |
| `docs/golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | 正在审计 | L2/L3/L4 与 AC 映射；待 AF-060 |
| `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | 正在审计 | 探索筛查与最终 DEC-005；待 AF-054 |
| `docs/golden/analytic_golden_candidates.v1.json` | 正在审计 | AN-G01～12 独立 L3 参考核验；待 AF-055 |
| `docs/TODO.md` | 正在审计 | 分阶段人类输入与授权；待 AF-057 |
| `docs/MAT_DATA_SCHEMA.md` | 正在审计 | 582 MAT 字典与历史证据；待 AF-056 |
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
| `AGENTS.md` | 已审计 | 仓库授权与数据安全规则 |
| `README.md` | 已审计 | 项目范围、证据/许可状态 |
| `LICENSE` | 已审计 | MIT 与上游作品边界 |
| `.gitignore` | 已审计 | 固定 output/ 忽略规则 |
| `scripts/check_docs_traceability.sh` | 已审计 | 静态追踪脚本，仅源码审查 |


## 4. 本轮新增审计与整改任务（2026-11-08）

- **新增审计对象**：`docs/golden/analytic_golden_candidates.v1.json`；连同原 49 个路径，现共 **50 个对象**。本轮处于「正在审计」的 24 个目标文件，是因为新增文字整改或证据/批准检查尚未闭环；其余 26 个「已审计」仅指既有静态范围，空白 0。
- **首次登记事项**：`AF-032～035`（4 条历史字段措辞统一）、`AF-036`（36 份版本登记与 50 个审计对象的**不同统计口径**）、`AF-037～061`（25 条设计冻结、真实验证、数据及批准证据）。具体目标和**最新两态**见 [AUDIT_PROCESS.md §3](AUDIT_PROCESS.md)；首次发现时均为「未整改」，后来六项文档整改（含 AF-062）见 §7。
- **建议顺序**：先修四处措辞及登记口径 → SRS/AC/REQ-DC → HLD/对象模型/contracts → 各模块 LLD → 在获授权后建测试及 L2/L3/L4 证据 → DEC-005/006/008、冻结签署 → 更新版本矩阵。绘图后端可选；无 MATLAB/L1 强制门禁。
- **初次登记时的边界（历史）**：当时仅压缩两台账并登记 AF-032～061，未改目标源文件；其后 AF-032～036、AF-062 的文档修订与结案见 §7，**当前仍未完成的是 AF-037～061**。此前与本次均未执行 C++、Bash、CMake、CTest、MAT 写入或 MATLAB/Octave；历史 AF-001～031 的结案不被重开。

## 5. 历史审计结论与证据限制

- **此前静态审阅**：49 个路径曾在限定静态范围记为「已审计」；旧 AF-001～031 共 32 个原子问题的**文本一致性整改**均有历史复核记录，概要保留在 [AUDIT_PROCESS.md](AUDIT_PROCESS.md)。
- **设计追踪**：**58 条正文 REQ + 附录 8 条 REQ-DC = 66 条 REQ**；相应 66 条 TC（其中 8 条 TC-DC）；98 P、45 个原 .m 映射及 27 条 HLD 静态 DAG 边。**8 REQ-DC 已包含在 66 中，不是额外新增**；不是 CMake/CTest 实绩。
- **MAT 数据**：独立 matio 探针曾报 582/582 READ_OK；尚缺独立 SHA-256、逐值 L2、正式业务 Reader/Writer/回读和安全负例。
- **数学/实现**：尚缺 Ivanic 1996/1998 原文复核、R3/R5 与一般/退化 L3、必要 L4、真实 C++/CTest；首轮 `atol=0.01,rtol=0.1` 已选，最终 DEC-005/006/008、文档冻结/发布仍未完成。
- 历史详细过程及固定 SHA 留在 [FULL_DOCUMENT_AUDIT](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md)、[七份数学文档复核](reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md)、[增量扫描](reviews/AUDIT_FOUR_POLICY_DRIFTS_SCAN_2026-11-08.md) 和 PROGRESS；此处不重复长篇日志或 hash 表。

## 6. 本轮审计发现（2026-11-08）

- 固定审计输入：`6c22fe7448cc9f3aed2c1f9f42766208e3a3e7fe`。比对两台账、SRS/HLD、MAT I/O/Runner/LLD 索引、TEST_PLAN、DEC-004、版本登记与部分算法 LLD；**先审计、后修改**。
- **AF-032～035 证实**：SRS §5、MAT I/O §4.1、Runner §5 与 TEST_PLAN §5.1 对 `HistoricalOnly` 标记或来源不明的写法仍可统一；现行 DEC-004 决策**已选，无需追溯归因**。
- **AF-036 证实**：控制矩阵的 **36 个选定文件身份登记**与 AUDIT 的 **50 个审计路径**是不同集合；不能把 36 写成全部仓库规范或认为多纳入审计即已批准。
- **新增 AF-062**：`docs/design/README.md` §1 将控制矩阵称为“**全部当前规范和过程文档**”的版本索引，范围过宽；应明确它是所选文档的固定身份矩阵，完整审计对象见本文件。已在整改台账先登记「未整改」。
- **AF-037～061**：所需设计冻结、真实 L2/L3/L4、Reader/Writer、测试、性能和人类批准仍缺独立证据；不运行生产命令、不填写假 PASS。本轮不以改文案结案这些阶段目标。历史 AF-001～031 的定向文本结论不被重新解释为运行验收。

## 7. 本轮文档整改与固定快照复核（2026-11-08）

- **AF-032～035、AF-062**：在源文档提交 `3e3ad2a` 中修正 SRS §5、MAT I/O §4.1、Runner §5、TEST_PLAN §5.1 的 `HistoricalOnly`/当前 save/缺字段语义及 LLD 索引的全量登记误称，**5/5 静态复读通过**；同步 SRS v0.13、MAT I/O v0.9、Runner v0.4、PLAN/TEST_PLAN/LLD 索引 v0.7。
- **AF-036**：控制矩阵提交 `2d608b9`，36 项固定文件身份 **36/36 与指定快照一致**，解释与 AUDIT 50 路径的不同用途；尚不构成正式版本冻结。上述六项**已整改**。
- **联合静态审阅**：当前文档 66 REQ、66 TC、98 P 身份各自唯一，HLD 27 条依赖边无环（文档级检查）。**AF-037～061 仍有 25 项「未整改」**，涵盖真实数值/Reader/Writer/测试、最终容差与人类签署。文件三态保持 **26 已审计、24 正在审计、0 空白**：新修订虽已复读，但这些文件仍有独立的阶段性目标。
- 未执行 Bash、CMake、CTest、MATLAB/Octave、独立 L3 数值或 MAT 写回读；未修改 `origin/**`、旧审批/历史审计报告或签署软件基线。实施及运行授权仍见 [TODO.md](TODO.md)。

## 8. 增量静态扫描：新发现先登记（2026-11-08）

- **固定输入**：`349206743af23abf039cbe5e97ea8b8c13341608`；核对 SRS、TEST_PLAN、PLAN、HLD、OBJECT_MODEL、AU/AF 台账和真实 Git 树。发现四项新的**现行文档**问题，按 [整改台账 §5](AUDIT_PROCESS.md) 先登记「未整改」；未修改被审计的目标文件。
- **AF-063**：TEST_PLAN 的 TC-DC-DOMAIN/IO/RNG/NOISE 和 TC-NF-COPYRIGHT 共五行，将**尚未创建**的 `docs/SOURCE_INVENTORY.md` 列为来源评审依据；PLAN P0.6 明确该文件是待交付，须标记“计划中、未创建”并指向已存在的 SRS §9.2、OBJECT_MODEL / MAT_DATA_SCHEMA（各自适用时）。
- **AF-064**：AUDIT 的“66 REQ / 8 REQ-DC”易被误读为 74；实际为 **58 条正文 REQ + 附录 8 条 REQ-DC = 66**，对应 **58 条普通 TC + 8 条 TC-DC = 66**。
- **AF-065/066**：AUDIT 进度表仍把 AF-032～036、AF-062 六条**已整改**目标写成“待”，§4 的“当轮仅登记未整改”历史句也需标明时间语境，避免覆盖较新的 §7 结案。
- **静态校验范围**：TEST_PLAN 66 个 TC ID、PLAN 98 项 P、HLD 27 条设计依赖边、对象模型 45 个 .m 映射；Git 树保留 45 .m/582 .mat，仅发现旧独立 MAT 探针 `src/mat_data_dict_probe.cpp`，无业务 C++ 测试文件。**没有**编译、运行 Bash/CTest/Matlab、读取 MAT 逐值、生成 L3 或进行人类批准；AF-037～061 仍保留 25 个阶段性目标。
