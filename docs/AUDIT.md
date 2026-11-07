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

## 3. 当前审计对象（三态按静态检查范围）

| 仓库根相对路径 | 状态 | 核对重点 |
|---|---|---|
| `docs/AUDIT.md` | 已审计 | 审计规则、AU 主题、逐文件三态 |
| `docs/AUDIT_PROCESS.md` | 已审计 | AF 目标和两态；历史已结案 |
| `docs/REQUIREMENTS.md` | 已审计 | SRS/66 REQ/AC、REQ-DC、DEC |
| `docs/PLAN.md` | 已审计 | WBS 98 P、依赖和里程碑 |
| `docs/TEST_PLAN.md` | 已审计 | 66 TC→候选测试→AC/L2～L4 |
| `docs/design/README.md` | 已审计 | LLD 索引、单一来源 |
| `docs/DOCUMENT_BASELINE_REGISTER.md` | 已审计 | 36 登记项与固定身份、冻结状态 |
| `docs/PROGRESS.md` | 已审计 | 真实证据与动态 DEC/里程碑 |
| `docs/ARCHITECTURE.md` | 已审计 | HLD/27 条设计边、部署/Writer |
| `docs/OBJECT_MODEL.md` | 已审计 | 生命周期、45 .m 映射 |
| `docs/design/contracts.md` | 已审计 | Result/Status/ErrorDetail、所有权 |
| `docs/design/cli.md` | 已审计 | 参数类型、RunId、JSONL/错误 |
| `docs/design/mat_io.md` | 已审计 | MAT 类型、RAII、Reader/Writer/安全 |
| `docs/design/kvd.md` | 已审计 | Franz/KvD 公式、逐轮顺序 |
| `docs/design/adaptive_mfa.md` | 已审计 | M1～M4、MSE、Session |
| `docs/design/harmonics.md` | 已审计 | 九项实球谐、R3/R5、Eigen |
| `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 已审计 | 论文/勘误和声明级边界 |
| `docs/design/sampling_noise.md` | 已审计 | 有序采样与 RNG 消耗 |
| `docs/design/trajectory.md` | 已审计 | Sophus、Rᵀ、近零、球内近度 |
| `docs/design/experiments.md` | 已审计 | 五入口、Runner/Manifest/save |
| `docs/design/analysis_plotting.md` | 已审计 | 四种分析、曲线、HistoricalOnly |
| `docs/decisions/DEC-004_HISTORICAL_MAT_FIELDS.md` | 已审计 | DEC-004 现行处理已选 |
| `docs/decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md` | 已审计 | 所有者已选技术方向 |
| `docs/golden/NO_MATLAB_EXECUTION_POLICY.md` | 已审计 | L1 永久不必需 |
| `docs/golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | 已审计 | L2/L3/L4 与 AC 映射 |
| `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | 已审计 | 探索筛查与最终 DEC-005 |
| `docs/TODO.md` | 已审计 | 分阶段人类输入与授权 |
| `docs/MAT_DATA_SCHEMA.md` | 已审计 | 582 MAT 字典与历史证据 |
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

## 4. 已完成审计与证据限制

- **此前静态审阅**：49 个路径「已审计」；旧 AF-001～031 共 32 个原子问题的**文本一致性整改**均有历史复核记录，概要保留在 [AUDIT_PROCESS.md](AUDIT_PROCESS.md)。
- **设计追踪**：66 REQ / 66 TC / 98 P / 8 REQ-DC / 45 个原 .m 映射、27 条 HLD 静态 DAG 边；不是 CMake/CTest 实绩。
- **MAT 数据**：独立 matio 探针曾报 582/582 READ_OK；尚缺独立 SHA-256、逐值 L2、正式业务 Reader/Writer/回读和安全负例。
- **数学/实现**：尚缺 Ivanic 1996/1998 原文复核、R3/R5 与一般/退化 L3、必要 L4、真实 C++/CTest；首轮 `atol=0.01,rtol=0.1` 已选，最终 DEC-005/006/008、文档冻结/发布仍未完成。
- 历史详细过程及固定 SHA 留在 [FULL_DOCUMENT_AUDIT](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md)、[七份数学文档复核](reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md)、[增量扫描](reviews/AUDIT_FOUR_POLICY_DRIFTS_SCAN_2026-11-08.md) 和 PROGRESS；此处不重复长篇日志或 hash 表。
