# 文档审计计划与文件进度（AUDIT）

> **范围**：本仓库规范、设计、测试、治理文件的静态一致性审计；**不是**编译、数值验证、正式签署或软件验收。现有结构较完整，但 SRS/HLD/LLD 均为**未冻结草案**。

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
| `docs/AUDIT.md` | 已审计 | 审计规则、AU 主题、逐文件三态 |
| `docs/AUDIT_PROCESS.md` | 已审计 | AF 目标和两态；历史已结案 |
| `docs/REQUIREMENTS.md` | 正在审计 | SRS/66 REQ/AC、REQ-DC、DEC；待 AF-032、AF-037 |
| `docs/PLAN.md` | 正在审计 | WBS 98 P、依赖和里程碑；待 AF-052、AF-061 |
| `docs/TEST_PLAN.md` | 正在审计 | 66 TC→候选测试→AC/L2～L4；待 AF-035、AF-051 |
| `docs/design/README.md` | 正在审计 | LLD 索引、单一来源；待 AF-059 |
| `docs/DOCUMENT_BASELINE_REGISTER.md` | 正在审计 | 36 登记项与固定身份、冻结状态；待 AF-036、AF-058 |
| `docs/PROGRESS.md` | 正在审计 | 真实证据与动态 DEC/里程碑；待 AF-053 |
| `docs/ARCHITECTURE.md` | 正在审计 | HLD/27 条设计边、部署/Writer；待 AF-038 |
| `docs/OBJECT_MODEL.md` | 正在审计 | 生命周期、45 .m 映射；待 AF-039 |
| `docs/design/contracts.md` | 正在审计 | Result/Status/ErrorDetail、所有权；待 AF-040 |
| `docs/design/cli.md` | 正在审计 | 参数类型、RunId、JSONL/错误；待 AF-041 |
| `docs/design/mat_io.md` | 正在审计 | MAT 类型、RAII、Reader/Writer/安全；待 AF-033、AF-042 |
| `docs/design/kvd.md` | 正在审计 | Franz/KvD 公式、逐轮顺序；待 AF-043 |
| `docs/design/adaptive_mfa.md` | 正在审计 | M1～M4、MSE、Session；待 AF-044 |
| `docs/design/harmonics.md` | 正在审计 | 九项实球谐、R3/R5、Eigen；待 AF-045 |
| `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 正在审计 | 论文/勘误和声明级边界；待 AF-046 |
| `docs/design/sampling_noise.md` | 正在审计 | 有序采样与 RNG 消耗；待 AF-047 |
| `docs/design/trajectory.md` | 正在审计 | Sophus、Rᵀ、近零、球内近度；待 AF-048 |
| `docs/design/experiments.md` | 正在审计 | 五入口、Runner/Manifest/save；待 AF-034、AF-049 |
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
- **新增事项**：`AF-032～035`（4 条历史字段措辞统一）、`AF-036`（36 份版本登记与 50 个审计对象的**不同统计口径**）、`AF-037～061`（25 条设计冻结、真实验证、数据及批准证据）。每条的**文件、具体目标、两态**统一见 [AUDIT_PROCESS.md §3](AUDIT_PROCESS.md)。所有新条目首先标「未整改」，没有因编制台账而假称已实施。
- **建议顺序**：先修四处措辞及登记口径 → SRS/AC/REQ-DC → HLD/对象模型/contracts → 各模块 LLD → 在获授权后建测试及 L2/L3/L4 证据 → DEC-005/006/008、冻结签署 → 更新版本矩阵。绘图后端可选；无 MATLAB/L1 强制门禁。
- **阶段边界**：本轮仅压缩两台账并登记计划，**尚未整改上述源文件或执行 C++、Bash、CMake、CTest、MAT 写入、MATLAB/Octave**；历史 AF-001～031 已结案事实不因此重开。

## 5. 历史审计结论与证据限制

- **此前静态审阅**：49 个路径曾在限定静态范围记为「已审计」；旧 AF-001～031 共 32 个原子问题的**文本一致性整改**均有历史复核记录，概要保留在 [AUDIT_PROCESS.md](AUDIT_PROCESS.md)。
- **设计追踪**：66 REQ / 66 TC / 98 P / 8 REQ-DC / 45 个原 .m 映射、27 条 HLD 静态 DAG 边；不是 CMake/CTest 实绩。
- **MAT 数据**：独立 matio 探针曾报 582/582 READ_OK；尚缺独立 SHA-256、逐值 L2、正式业务 Reader/Writer/回读和安全负例。
- **数学/实现**：尚缺 Ivanic 1996/1998 原文复核、R3/R5 与一般/退化 L3、必要 L4、真实 C++/CTest；首轮 `atol=0.01,rtol=0.1` 已选，最终 DEC-005/006/008、文档冻结/发布仍未完成。
- 历史详细过程及固定 SHA 留在 [FULL_DOCUMENT_AUDIT](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md)、[七份数学文档复核](reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md)、[增量扫描](reviews/AUDIT_FOUR_POLICY_DRIFTS_SCAN_2026-11-08.md) 和 PROGRESS；此处不重复长篇日志或 hash 表。
