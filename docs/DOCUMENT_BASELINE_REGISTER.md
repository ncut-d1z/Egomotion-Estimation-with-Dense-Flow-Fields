# 文档类型、版本、Git SHA 与基线状态控制矩阵

> **文档管理登记，非正式已签署基线。** 下表全部文件的 Git blob SHA 对应固定快照 commit `663f25274c744ac5eeec258520d8920677ca9f31`。本矩阵创建提交晚于该快照，不声称任何 Git SHA 已获整套 SRS/HLD/LLD/TEST_PLAN 的基线批准。各文档采用**独立版本号**，SRS v0.6、HLD v0.4 和各 LLD v0.x 同时存在是正常的，不要求改为同一版本。后续提交改变文件内容时须以新的快照重新登记，不能复用旧 SHA。

## 1. 文档职责与唯一规范来源

| 文档类型 | 唯一维护内容 | 不应重复维护 |
|---|---|---|
| **SRS** | 外部功能/数据/接口/非功能需求、优先级、REQ/AC、DEC、变更；原 8 条 REQ-DC 作为附录 A **设计约束索引**，保留原 ID、AC、TC | Eigen/Sophus/Result 内部实现、target DAG/ABI/具体测试文件不在需求正文重复定义，按 HLD/ADR/LLD 引用 |
| **HLD** | 架构目标、组件能力、依赖 DAG（目前静态 27 条边）、部署、时序、质量属性、ADR | 不复制 CLI 错误映射、类型签名或模块计算实现 |
| **对象模型** | 类职责、所有权/状态机、45 .m 源文件到候选职责的映射 | 不取代模块精确 C++17 签名 |
| **contracts.md** | 领域值类型、封装 Result/Status、错误枚举/ErrorDetail、跨模块 ABI 候选 | 具体 MatDenseArray 类型归 mat_io LLD；各 Runner 数据归对应 LLD |
| **CLI LLD** | 命令/参数/错误/JSONL 与固定输出根契约 | 不另定义第二套错误 enum；错误枚举/退出码参照 contracts.md |
| **模块 LLD** | 数学公式/伪代码、输入/返回、异常/生命周期、模块 target 所需边界及对应 TC | target DAG 以 HLD §4/§4.1、TC→候选实体→AC 以 TEST_PLAN §3 为单一来源 |
| **TEST_PLAN** | 66 TC 的候选文件/CTest→AC、参考来源 L2/L3/L4、测法和证据模板 | 实际是否执行、失败数、PASS/FAIL 只从 PROGRESS 与定 SHA reviews 获取 |
| **PLAN** | 98 P 工作项、前置关系、里程碑、证据类别 | 不复制所有具体测试函数和实际结果；链接 TEST_PLAN/PROGRESS |
| **PROGRESS/reviews** | 当前工作/DEC/里程碑与真实执行记录；reviews 是历史固定 SHA 快照 | 不作为 SRS/HLD/LLD 正文，不把旧快照 26 条边视为现有 HLD 27 条边的执行证明 |
| **政策与决定附件** | 所有者已确定的技术方向、DEC-004、L1 不必需和 L2/L3/L4 参考策略 | 只为规范提供来源，不替代核心规范批准；原审批历史保留原时间点 |

## 2. 当前核心文档与 blob SHA

**快照：** `663f25274c744ac5eeec258520d8920677ca9f31`。状态均为 **评审草案、未冻结/未验收**，其中 PROGRESS 为滚动记录而非设计基线。

| 层次 | 文档 | 文档版本 | Git blob SHA | 正式基线 | 范围 |
|---|---|---|---|---|---|
| SRS | [docs/REQUIREMENTS.md](REQUIREMENTS.md) | v0.6 | `14b9be53a8c42b6e5c8b4f19fc84bb28383100c6` | 未冻结 | REQ/AC + 附录 A 设计约束 |
| HLD | [docs/ARCHITECTURE.md](ARCHITECTURE.md) | v0.4 | `3ee0862acd1f81394186117671eeda06c9bca70d` | 未冻结 | 组件/部署/27 条 target 边及 ADR |
| HLD 附件 | [docs/OBJECT_MODEL.md](OBJECT_MODEL.md) | v0.1 | `ccd112408b58cd00031deedacc1840b418474512` | 未冻结 | 对象生命周期/45 文件映射 |
| 跨模块 LLD | [docs/design/contracts.md](design/contracts.md) | v0.1 | `6ecd88effd5b3f05d6bd7c6f013cc2805af0bb9c` | 未冻结 | Result/Status/ErrorDetail 与领域数据 |
| CLI LLD | [docs/design/cli.md](design/cli.md) | v0.2 | `94fffed91f5f3ff8691f50eb69d9a81ece549c51` | 未冻结 | 命令、JSONL、固定输出根 |
| MAT I/O LLD | [docs/design/mat_io.md](design/mat_io.md) | v0.6 | `3a34af8995eaba5f10e89008f4ecfee5b30be9c6` | 未冻结 | 自拥有数据/Reader/Writer/安全 |
| KvD LLD | [docs/design/kvd.md](design/kvd.md) | v0.3 | `abf9883654f2405ee244fe52a8fc5d9575aeab6c` | 未冻结 | Franz/KvD 与逐轮顺序 |
| Adaptive LLD | [docs/design/adaptive_mfa.md](design/adaptive_mfa.md) | v0.4 | `c08f69f147845585370e789e3d34ab08d98dfeae` | 未冻结 | StepSolver/Session 失败不提交 |
| 球谐 LLD | [docs/design/harmonics.md](design/harmonics.md) | v0.4 | `95cbccf547fa66b99b13c6d77288e001efe2390b` | 未冻结 | R3/R5、Eigen-only/禁 sdla |
| 采样/噪声 LLD | [docs/design/sampling_noise.md](design/sampling_noise.md) | v0.1 | `3d122777c04a3e57ba559d01eae89330d354d159` | 未冻结 | 有序采样、RNG/噪声 |
| 轨迹 LLD | [docs/design/trajectory.md](design/trajectory.md) | v0.1 | `ebd445dba6c56e062cf5d26d92f5ffe13344ab75` | 未冻结 | 轨迹/运动/近度 |
| 实验 LLD | [docs/design/experiments.md](design/experiments.md) | v0.1 | `dbe566e77d5cc590d572aacb845a5ec101381674` | 未冻结 | 五入口/当前 save、HistoricalOnly |
| 分析 LLD | [docs/design/analysis_plotting.md](design/analysis_plotting.md) | v0.1 | `573aee00f91ff9ce2059cd9a06a0911338cd6b90` | 未冻结 | L2 读取、L3 曲线/缺字段拒绝 |
| LLD 索引 | [docs/design/README.md](design/README.md) | v0.1 | `8b7098fd55eb3d4d96fa3f3bc690cc0d1d1cf710` | 未冻结 | 模块 LLD 范围/冻结条件 |
| 实施 WBS | [docs/PLAN.md](PLAN.md) | v0.1 | `2ce9b3adab0da96191ecde3ed85929787a9d6eb6` | 未冻结 | 98 个 P 工作项 |
| 测试计划 | [docs/TEST_PLAN.md](TEST_PLAN.md) | v0.1 | `92e460072ff1e266ef487d88093a490059569f91` | 未冻结 | 66 TC→候选测试→AC |
| 证据登记 | [docs/PROGRESS.md](PROGRESS.md) | v0.1（滚动） | `04aa5d69e9ccf0e97d584e67088994822b097bf4` | 未冻结 | 真实结果/历史证据/DEC 当前状态 |

**全部核心基线签署字段现为未提供**：正式批准人、批准日期与时区、批准 commit SHA、冻结生效范围、例外 DEC 与批准证据。已有所有者**技术方案选择**和根 MIT 许可授权，不等于批准整份 SRS/HLD/LLD 或测试 PASS。对版本表的任何变更必须引用新 Git commit/对应 blob SHA。

## 3. 专题、证据与所有者决策附件

| 类型 | 文件 | Git blob SHA（同一快照） | 已知效力/限制 |
|---|---|---|---|
| 球谐专题附件 | [docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md](design/SH_ROTATION_IVANIC_RUEDENBERG.md) | `b0d09b65f4b600bdfc19d2578994ba1061b0908a` | 专题草案，论文 PDF 待上传 |
| MAT 字典 | [docs/MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) | `a472f76798d265dbeb8422de32fe78214d3f7ed8` | 先前 582/582 MAT 只读探针事实 |
| 所有者决定 | [docs/decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md](decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md) | `72a7ad3cf72d32603651d5114a66ea0fba3f8552` | 技术路线已选，非完整设计基线 |
| DEC-004 | [docs/decisions/DEC-004_HISTORICAL_MAT_FIELDS.md](decisions/DEC-004_HISTORICAL_MAT_FIELDS.md) | `063acaa2dc8e7524546fb85a985a7f8b8e860a88` | 已决策，代码未实施 |
| 无 MATLAB 政策 | [docs/golden/NO_MATLAB_EXECUTION_POLICY.md](golden/NO_MATLAB_EXECUTION_POLICY.md) | `c0553344482c7ce5068c73ec45d651eac70273f6` | L1 不再必需 |
| 证据等级附件 | [docs/golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md](golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md) | `7e19801646f2eed343c452bfd9a1736e1e6acca2` | L2/L3/L4 归类 |
| 容差策略附件 | [docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md](golden/GOLDEN_TOLERANCE_PROPOSAL.md) | `a1a2122887c4030dfadfd01199aed5e482b7b8b2` | 0.01/0.1 首轮筛查，最终阈值未冻结 |
| 开发规则 | [AGENTS.md](../AGENTS.md) | `7d609ccff2ecb41814b3bdbc7986dcccaaa2d6bf` | 只读原资料/实际运行授权 |

原审批与 `docs/reviews/*.md` 保存历史决定、历史 SHA 和当时的验证信息，不随当前版本重写。特别是先前的 26 条 HLD 边证据只能用于当时快照；目前 HLD 的 27 条边只做过 GitHub 连接器读取下的静态无环检查，**不是 CMake 构建证据**。

## 4. 已选设计与真正未完成条件

**无需再反复确认**：项目根 MIT、Ivanic–Ruedenberg 1996 原文及 1998 勘误、纯 Eigen 且禁 sdla、固定 `output/<RunId>/` 且无自定义输出根 CLI 参数、封装 Result/Status 和 JSONL、独立 Runner 配置与失败状态策略、首轮 `atol=0.01, rtol=0.1` 筛查、DEC-004 仅保存当前有效 save/历史专属字段只读保留、MATLAB/Octave/L1 不必需。

**仍须实际取得的证据**：用户上传球谐原文/勘误 PDF 后逐式核验；实现的自包含 C++17 头文件/类型/错误合同、真实 CMake target 和 CTest；L2 原始 MAT 逐元素/独立 SHA-256、L3 一般及退化输入、必要 L4 统计、DEC-004 缺字段/非覆盖测试、最终 DEC-005 分模块数值阈值和性能数据；正式文档/PR 的批准或签署。**缺乏测试并非文档类型不正确，且不会自动因为有版本号/静态核对 0 错误而成为已验收基线。**

## 5. 正式冻结的必填身份字段与推荐顺序

每次冻结应按具体文档记录：`path`、`document_version`、`git_blob_sha`、`source_commit_sha`、`dependent_docs_and_sha`、`scope`、`REQ/AC/TC/P`、`open_DEC_and_exceptions`、`reviewer`、`approval_date_timezone`、`test_evidence_refs`、`status=ApprovedBaseline`。未获同意时保持 `ReviewDraft` 或 `ProposedBaseline`，不擅自签署。

1. 审阅 SRS 外部可验证要求与附录 A 的八条内部设计约束，不改动 REQ/AC/TC ID。
2. 对齐 HLD、对象模型、跨模块 contracts 的组件/头文件/错误/所有权与 27 条设计依赖边。
3. 分模块审阅 LLD 的签名、前后置/失败、真实测试入口、L2/L3/L4 数值和最终适用容差。
4. 实际测试和数值验收按授权进行，把每项证据/样本数与输出日志回填 TEST_PLAN 关联的 PROGRESS，而不是重复标记在多个 LLD。
5. 由所有者对固定 Git SHA、范围、证据及开放风险明确签署；**本轮未进行该签署**。

**本表仅已建立版本/责任控制架构，当前全部核心规范仍是未冻结评审草案。**
