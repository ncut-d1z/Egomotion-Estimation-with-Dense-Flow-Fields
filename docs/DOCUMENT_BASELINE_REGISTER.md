# 文档类型、版本、Git SHA 与基线状态控制矩阵

> **文档管理登记，非正式已签署基线。** 以下 **29 份当前登记文件**的 Git blob SHA 对应固定快照 commit `44068279403782844648a99f0f8385dd28d27796`；本矩阵自身的更新提交晚于该快照。SRS v0.10、HLD v0.5、对象模型 v0.3、WBS/TEST_PLAN v0.4、LLD 索引 v0.3、容差参考 v0.2，各文件版本独立。**核心规范仍未冻结；PROGRESS/AUDIT/固定审查报告是过程登记，不适用软件基线冻结或 TC PASS**。后续文件更改必须重新登记新的快照与 blob SHA。

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
| **AUDIT** | 归纳用户历轮详细 Markdown 审查与修改意见，按相对路径和三态维护逐文件审计进度 | 审计状态不是软件测试 PASS、规范冻结或历史审批 JSON 的改写；动态证据仍归 PROGRESS |
| **政策与决定附件** | 所有者已确定的技术方向、DEC-004、L1 不必需和 L2/L3/L4 参考策略 | 只为规范提供来源，不替代核心规范批准；原审批历史保留原时间点 |

## 2. 当前核心文档与 blob SHA

**快照：** `44068279403782844648a99f0f8385dd28d27796`。当前文件 Git blob 身份已按这一固定快照登记；本矩阵的提交晚于该快照。`PROGRESS.md` 的“正式基线”列为**不适用（滚动证据登记）**，其它核心规范仍未冻结；审计记录中的“已审计”不等于已解决所有语义问题。

| 层次 | 文档 | 文档版本 | Git blob SHA | 正式基线 | 范围 |
|---|---|---|---|---|---|
| SRS | [docs/REQUIREMENTS.md](REQUIREMENTS.md) | v0.10 | `8abf270ad02210b48c3b48561b91b0d5cdbf5728` | 未冻结 | REQ/AC + 附录 A 设计约束 |
| HLD | [docs/ARCHITECTURE.md](ARCHITECTURE.md) | v0.5 | `1076a55eed9ee4d547e2cc4132d954cb34fd2c52` | 未冻结 | 组件/部署/27 条 target 边及 ADR |
| HLD 附件 | [docs/OBJECT_MODEL.md](OBJECT_MODEL.md) | v0.3 | `ebc8aa3bc5cd7bc504676059c34bb1d2d766d5c2` | 未冻结 | 对象生命周期/45 文件映射 |
| 跨模块 LLD | [docs/design/contracts.md](design/contracts.md) | v0.1 | `6ecd88effd5b3f05d6bd7c6f013cc2805af0bb9c` | 未冻结 | Result/Status/ErrorDetail 与领域数据 |
| CLI LLD | [docs/design/cli.md](design/cli.md) | v0.2 | `94fffed91f5f3ff8691f50eb69d9a81ece549c51` | 未冻结 | 命令、JSONL、固定输出根 |
| MAT I/O LLD | [docs/design/mat_io.md](design/mat_io.md) | v0.6 | `3a34af8995eaba5f10e89008f4ecfee5b30be9c6` | 未冻结 | 自拥有数据/Reader/Writer/安全 |
| KvD LLD | [docs/design/kvd.md](design/kvd.md) | v0.3 | `abf9883654f2405ee244fe52a8fc5d9575aeab6c` | 未冻结 | Franz/KvD 与逐轮顺序 |
| Adaptive LLD | [docs/design/adaptive_mfa.md](design/adaptive_mfa.md) | v0.4 | `c08f69f147845585370e789e3d34ab08d98dfeae` | 未冻结 | StepSolver/Session 失败不提交 |
| 球谐 LLD | [docs/design/harmonics.md](design/harmonics.md) | v0.6 | `4966216d3f9a38caa2429adbd6899eb28cbd4933` | 未冻结 | R3/R5、Eigen-only/禁 sdla |
| 采样/噪声 LLD | [docs/design/sampling_noise.md](design/sampling_noise.md) | v0.1 | `3d122777c04a3e57ba559d01eae89330d354d159` | 未冻结 | 有序采样、RNG/噪声 |
| 轨迹 LLD | [docs/design/trajectory.md](design/trajectory.md) | v0.1 | `ebd445dba6c56e062cf5d26d92f5ffe13344ab75` | 未冻结 | 轨迹/运动/近度 |
| 实验 LLD | [docs/design/experiments.md](design/experiments.md) | v0.2 | `3b439d4020f1c7aa8f21d233128d2182f320e4da` | 未冻结 | 五入口/当前 save、HistoricalOnly |
| 分析 LLD | [docs/design/analysis_plotting.md](design/analysis_plotting.md) | v0.2 | `78ff555c1d64f62ffcc953ba3f142ac5abc84d20` | 未冻结 | L2 读取、L3 曲线/缺字段拒绝 |
| LLD 索引 | [docs/design/README.md](design/README.md) | v0.3 | `fa9d6fb463654d4c6ba2e109cdb077be0940dc6a` | 未冻结 | 模块 LLD 范围/冻结条件 |
| 实施 WBS | [docs/PLAN.md](PLAN.md) | v0.4 | `52151182ccd23bca7c8f2f6da7eeb82fa58c663a` | 未冻结 | 98 个 P 工作项 |
| 测试计划 | [docs/TEST_PLAN.md](TEST_PLAN.md) | v0.4 | `382961192ea8c4c7f343f318d11ff0f3170d5734` | 未冻结 | 66 TC→候选测试→AC |
| 证据登记 | [docs/PROGRESS.md](PROGRESS.md) | v0.1（滚动） | `996c6a662f87276bee97e7906ae8b8f899443140` | 不适用（滚动证据登记） | 真实结果/历史证据/DEC 当前状态 |

**全部适用的规范基线签署字段目前未提供**：正式批准人、批准日期与时区、批准 commit SHA、冻结生效范围、例外 DEC 与批准证据。已有所有者**技术方案选择**和根 MIT 许可授权，不等于批准整份 SRS/HLD/LLD 或测试 PASS。对版本表的任何变更必须引用新 Git commit/对应 blob SHA。

## 3. 专题、证据与所有者决策附件

**历轮用户修改意见及逐文件审计进度**：[AUDIT.md](AUDIT.md)，与延期人工干预的 [TODO.md](TODO.md) 分开管理；AUDIT 不替代 SRS/HLD/LLD 或测试执行证据。

**人类干预事项的当前唯一清单**：[TODO.md](TODO.md)（延期原因/建议处理时间）。球谐 C++17 仅声明文件清单在 [专题 LLD §3](design/SH_ROTATION_IVANIC_RUEDENBERG.md)，对应的 .hpp/.cpp 文件**尚不存在**，不应登记为源码工件。

| 类型 | 文件 | Git blob SHA（同一快照） | 已知效力/限制 |
|---|---|---|---|
| 球谐专题附件 | [docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md](design/SH_ROTATION_IVANIC_RUEDENBERG.md) | `5b5678ae4d0db0e59714531df52f55a0a925ce8e` | 专题草案，论文 PDF 待上传 |
| MAT 字典 | [docs/MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) | `a472f76798d265dbeb8422de32fe78214d3f7ed8` | 先前 582/582 MAT 只读探针事实 |
| 所有者决定 | [docs/decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md](decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md) | `72a7ad3cf72d32603651d5114a66ea0fba3f8552` | 技术路线已选，非完整设计基线 |
| DEC-004 | [docs/decisions/DEC-004_HISTORICAL_MAT_FIELDS.md](decisions/DEC-004_HISTORICAL_MAT_FIELDS.md) | `063acaa2dc8e7524546fb85a985a7f8b8e860a88` | 已决策，代码未实施 |
| 无 MATLAB 政策 | [docs/golden/NO_MATLAB_EXECUTION_POLICY.md](golden/NO_MATLAB_EXECUTION_POLICY.md) | `c0553344482c7ce5068c73ec45d651eac70273f6` | L1 不再必需 |
| 证据等级附件 | [docs/golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md](golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md) | `7e19801646f2eed343c452bfd9a1736e1e6acca2` | L2/L3/L4 归类 |
| 容差策略附件 | [docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md](golden/GOLDEN_TOLERANCE_PROPOSAL.md) | `736c082e8beab2f76dff2585fca27b10f321d775` | v0.2；0.01/0.1 首轮筛查，最终阈值未冻结；R5 独立 Eigen 参考 |
| 开发规则 | [AGENTS.md](../AGENTS.md) | `7d609ccff2ecb41814b3bdbc7986dcccaaa2d6bf` | 只读原资料/实际运行授权 |
| 延期人类事项 | [docs/TODO.md](TODO.md) | `c3f6d4b79dd7c0bbe4074006c75f214b182f2af4` | 材料、H-10/H-11、DEC-005/006、最终基线/发布：原因及建议阶段，**没有当前授权** |
| 本轮只读审查 | [docs/reviews/DOCUMENT_REVIEW_AND_SH_DECLARATIONS_2026-11-07.md](reviews/DOCUMENT_REVIEW_AND_SH_DECLARATIONS_2026-11-07.md) | `580e98eae40a5378128b139ce531b6ded11c6c2e` | 历史固定 SHA 下的 GitHub 只读 ID/DAG/引用核对，非真实 Bash/CTest |
| 用户历轮审计台账 | [docs/AUDIT.md](AUDIT.md) | `0dc7125c35415514dce1a934b9ad92e16bcc03b6` | **AU-001～028** 历轮意见，按文件相对路径的“空白/正在审计/已审计”进度，不表示 SRS/HLD/LLD 已冻结 |
| 本轮完整审计报告 | [docs/reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md) | `43446ec3e7ebe3d9b7f2447561f72d64ccbb5e95` | 固定输入 commit `4627cc1e48be9c43ad10b28fda646d65ee8ef01c` 的 **40 文件文档审计**，记录 AF-001～012 待修订项与 AH-001～002 历史边界，非 CMake/CTest PASS |

原审批与 `docs/reviews/*.md` 保存历史决定、历史 SHA 和当时的验证信息，不随当前版本重写。特别是先前的 26 条 HLD 边证据只能用于当时快照；目前 HLD 的 27 条边只做过 GitHub 连接器读取下的静态无环检查，**不是 CMake 构建证据**。

**审计进度与当前问题**：见 [AUDIT.md](AUDIT.md) 的三态文件表及 [FULL_DOCUMENT_AUDIT_2026-11-07.md](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md) 的 AF-001～AF-012；固定快照中当前文档有 12 个待修订语义问题（高 1、中 8、低 3）。**尚未修复这些问题**，不能把 25 项“已审计”解释为 25 个无缺陷/已冻结文件。历史 reviews 的报告保留其当时 SHA 和旧版本事实。

## 4. 已选设计与真正未完成条件

**无需再反复确认（可推迟的人类事项见 [TODO.md](TODO.md)）**：项目根 MIT、Ivanic–Ruedenberg 1996 原文及 1998 勘误、纯 Eigen 且禁 sdla、固定 `output/<RunId>/` 且无自定义输出根 CLI 参数、封装 Result/Status 和 JSONL、独立 Runner 配置与失败状态策略、首轮 `atol=0.01, rtol=0.1` 筛查、DEC-004 仅保存当前有效 save/历史专属字段只读保留、MATLAB/Octave/L1 不必需。

**仍须实际取得的证据与人类介入时间**（见 [TODO.md](TODO.md)，当前仅文档阶段）：用户上传球谐原文/勘误 PDF 后逐式核验；实现的自包含 C++17 头文件/类型/错误合同、真实 CMake target 和 CTest；L2 原始 MAT 逐元素/独立 SHA-256、L3 一般及退化输入、必要 L4 统计、DEC-004 缺字段/非覆盖测试、最终 DEC-005 分模块数值阈值和性能数据；正式文档/PR 的批准或签署。**缺乏测试并非文档类型不正确，且不会自动因为有版本号/静态核对 0 错误而成为已验收基线。**

## 5. 正式冻结的必填身份字段与推荐顺序

每次冻结应按具体文档记录：`path`、`document_version`、`git_blob_sha`、`source_commit_sha`、`dependent_docs_and_sha`、`scope`、`REQ/AC/TC/P`、`open_DEC_and_exceptions`、`reviewer`、`approval_date_timezone`、`test_evidence_refs`、`status=ApprovedBaseline`。未获同意时保持 `ReviewDraft` 或 `ProposedBaseline`，不擅自签署。

1. 审阅 SRS 外部可验证要求与附录 A 的八条内部设计约束，不改动 REQ/AC/TC ID。
2. 对齐 HLD、对象模型、跨模块 contracts 的组件/头文件/错误/所有权与 27 条设计依赖边。
3. 分模块审阅 LLD 的签名、前后置/失败、真实测试入口、L2/L3/L4 数值和最终适用容差。
4. 实际测试和数值验收按授权进行，把每项证据/样本数与输出日志回填 TEST_PLAN 关联的 PROGRESS，而不是重复标记在多个 LLD。
5. 由所有者对固定 Git SHA、范围、证据及开放风险明确签署；**本轮未进行该签署**。

**本表仅登记版本/责任与固定文件身份：SRS/HLD/LLD/TEST_PLAN 仍为未冻结评审草案；PROGRESS/AUDIT 是过程登记，不适用规范基线冻结，均不代表算法或测试已经通过。**
