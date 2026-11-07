# 文档类型、版本、Git SHA 与基线状态控制矩阵

> **文档管理登记，非正式已签署基线。** 下列 **36 份登记文件**的 Git blob SHA 对应固定快照 commit `2a7482a11a232df291409c34497dfe7c4bed8143`；本矩阵自身的更新提交晚于该快照。各文档独立版本包括 SRS v0.12、HLD v0.8、对象模型 v0.5、contracts v0.2、MAT I/O v0.8、experiments v0.3、harmonics v0.7、KvD v0.4、sampling_noise v0.3、**analysis_plotting v0.4、参考容差提案 v0.3**、trajectory v0.2、PLAN/TEST_PLAN/LLD 索引 v0.6。**核心规范均未冻结；PROGRESS/AUDIT/AUDIT_PROCESS/审计 reviews 属于过程资料，不能代替软件基线或 TC PASS**。后续变更需另按固定 Git SHA 登记。

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
| **AUDIT_PROCESS** | **问题级整改目标与「未整改/已整改」状态**：发现问题即按相对路径登记，修改并固定 SHA 复核后才改变状态 | 不得与 AUDIT 的三态审计进度混用，不能凭登记就关闭未修订问题；真实结果仍归 PROGRESS |
| **政策与决定附件** | 所有者已确定的技术方向、DEC-004、L1 不必需和 L2/L3/L4 参考策略 | 只为规范提供来源，不替代核心规范批准；原审批历史保留原时间点 |

## 2. 当前核心文档与 blob SHA

**快照：** `2a7482a11a232df291409c34497dfe7c4bed8143`。全部 SHA 是这一固定提交的 blob 身份，不含本矩阵自身更新后的 blob。七份原“正在审计”文档已按可用原始 `.m` 的**静态数学合同检查**完成并登记，**尚无用户上传的 Ivanic–Ruedenberg PDF 及独立 L3/CTest 实测**；审计完成不等于数学结果通过或正式冻结。

| 层次 | 文档 | 文档版本 | Git blob SHA | 正式基线 | 范围 |
|---|---|---|---|---|---|
| SRS | [docs/REQUIREMENTS.md](REQUIREMENTS.md) | v0.12 | `acd6dd8c21e071fb29a1e152dc8c79281d9bda18` | 未冻结 | REQ/AC + 附录 A 设计约束 |
| HLD | [docs/ARCHITECTURE.md](ARCHITECTURE.md) | v0.8 | `3a79a1a566a1f51cdec3905d1b220093d5121b33` | 未冻结 | 组件/部署/27 条 target 边及 ADR |
| HLD 附件 | [docs/OBJECT_MODEL.md](OBJECT_MODEL.md) | v0.5 | `26db2608f60826b89bd8341d237ad4207a87dd29` | 未冻结 | 对象生命周期/45 文件映射 |
| 跨模块 LLD | [docs/design/contracts.md](design/contracts.md) | v0.2 | `ccc90e7ba40b65d0dd64bafdbda55ac02543a33f` | 未冻结 | Result/Status/ErrorDetail 与领域数据 |
| CLI LLD | [docs/design/cli.md](design/cli.md) | v0.2 | `94fffed91f5f3ff8691f50eb69d9a81ece549c51` | 未冻结 | 命令、JSONL、固定输出根 |
| MAT I/O LLD | [docs/design/mat_io.md](design/mat_io.md) | v0.8 | `1b4044765209cc384c87ebf8d1ce0a7dc95594c0` | 未冻结 | 自拥有数据/Reader/Writer/安全 |
| KvD LLD | [docs/design/kvd.md](design/kvd.md) | v0.4 | `b59a151a0bdb7942c999538dffa94dbee02b655f` | 未冻结 | Franz/KvD 与逐轮顺序 |
| Adaptive LLD | [docs/design/adaptive_mfa.md](design/adaptive_mfa.md) | v0.4 | `c08f69f147845585370e789e3d34ab08d98dfeae` | 未冻结 | StepSolver/Session 失败不提交 |
| 球谐 LLD | [docs/design/harmonics.md](design/harmonics.md) | v0.7 | `e8b28b096f8c4b2fdcd6aa6cfadca26d29fe8f2e` | 未冻结 | R3/R5、Eigen-only/禁 sdla |
| 采样/噪声 LLD | [docs/design/sampling_noise.md](design/sampling_noise.md) | v0.3 | `d4a73fe9d5dd6a984228ec2ef0e4a3ac928e5878` | 未冻结 | 有序采样、RNG/噪声 |
| 轨迹 LLD | [docs/design/trajectory.md](design/trajectory.md) | v0.2 | `9f1d0bf0604f515b17bca58b94ab7d8311b850cb` | 未冻结 | 轨迹/运动/近度 |
| 实验 LLD | [docs/design/experiments.md](design/experiments.md) | v0.3 | `ad1626baefbc907f9a430311bf7cb1b95b49e2b3` | 未冻结 | 五入口/当前 save、HistoricalOnly |
| 分析 LLD | [docs/design/analysis_plotting.md](design/analysis_plotting.md) | v0.4 | `7544280ee47402ecd4630bf97544ff02254dd225` | 未冻结 | L2 读取、L3 曲线/缺字段拒绝 |
| LLD 索引 | [docs/design/README.md](design/README.md) | v0.6 | `c6bfb22e9351dec6620a0bf22ab1ceb09f4919ed` | 未冻结 | 模块 LLD 范围/冻结条件 |
| 实施 WBS | [docs/PLAN.md](PLAN.md) | v0.6 | `1185dbb297421ba30aaf1424c4bba5726691a2b6` | 未冻结 | 98 个 P 工作项 |
| 测试计划 | [docs/TEST_PLAN.md](TEST_PLAN.md) | v0.6 | `e860c32b6622162f891c7632dfc2e0ee537834ab` | 未冻结 | 66 TC→候选测试→AC |
| 证据登记 | [docs/PROGRESS.md](PROGRESS.md) | v0.1（滚动） | `8439b9eea32183b6fe4463a88a699409ad1895de` | 不适用（滚动证据登记） | 真实结果/历史证据/DEC 当前状态 |

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
| 无 MATLAB 政策 | [docs/golden/NO_MATLAB_EXECUTION_POLICY.md](golden/NO_MATLAB_EXECUTION_POLICY.md) | `5f2bc13f3fbf0636ac15f5dfe0c38611b80074be` | L1 不再必需 |
| 证据等级附件 | [docs/golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md](golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md) | `7e19801646f2eed343c452bfd9a1736e1e6acca2` | L2/L3/L4 归类 |
| 容差策略附件 | [docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md](golden/GOLDEN_TOLERANCE_PROPOSAL.md) | `2d70a8a66bf6b5249e3181ec7452097276926534` | v0.3；0.01/0.1 首轮筛查，最终阈值未冻结；R5 独立 Eigen 参考 |
| 开发规则 | [AGENTS.md](../AGENTS.md) | `7d609ccff2ecb41814b3bdbc7986dcccaaa2d6bf` | 只读原资料/实际运行授权 |
| 延期人类事项 | [docs/TODO.md](TODO.md) | `c3f6d4b79dd7c0bbe4074006c75f214b182f2af4` | 材料、H-10/H-11、DEC-005/006、最终基线/发布：原因及建议阶段，**没有当前授权** |
| 本轮只读审查 | [docs/reviews/DOCUMENT_REVIEW_AND_SH_DECLARATIONS_2026-11-07.md](reviews/DOCUMENT_REVIEW_AND_SH_DECLARATIONS_2026-11-07.md) | `580e98eae40a5378128b139ce531b6ded11c6c2e` | 历史固定 SHA 下的 GitHub 只读 ID/DAG/引用核对，非真实 Bash/CTest |
| 用户历轮审计台账 | [docs/AUDIT.md](AUDIT.md) | `a5b631b8b2512fe5e961de003944dbf81547a872` | **AU-001～028** 历轮意见，按文件相对路径的“三态审计进度”；七份可用 .m 资料的静态文档审阅已完成，论文/L3 仍待验证 |
| 问题级整改进度 | [docs/AUDIT_PROCESS.md](AUDIT_PROCESS.md) | `cf0a00070358c262d874c6451cb408b24cf5df87` | **AF-001～027 共 28 个原子整改目标均已完成纯文档修订与固定 SHA 复核**；AF-026/027 的发现先以“未整改”登记，后由独立快照核对再转为“已整改”；非测试或基线批准 |
| 本轮完整审计报告 | [docs/reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md) | `43446ec3e7ebe3d9b7f2447561f72d64ccbb5e95` | 固定输入 commit `4627cc1e48be9c43ad10b28fda646d65ee8ef01c` 的 **40 文件文档审计**，记录 AF-001～012 待修订项与 AH-001～002 历史边界，非 CMake/CTest PASS |
| AF-001～AF-012 修订复核 | [docs/reviews/AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md](reviews/AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md) | `ae44ae6ba782a9930b400ea860a50cc326a0cd3f` | 定向文本核验快照 `893acb7b07862c0701e62b786462ef8395a116da`：12 项 AF 已做纯文档修订并通过限定静态复核；仍缺逐式数值、代码编译和正式测试 |
| AF-013～AF-020 文档修订复核 | [docs/reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md](reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md) | `1466ca9534a54cac6bf9b21a5b2bf42491d6e6a9` | 固定修订/静态复核 commit `a782b5c8d86e50fe4d2ae4e2b198b3cd6345de52`；八条 AF 均完成合同文本修订，非真实软件/数值通过 |
| AF-021～AF-025 扫描 | [docs/reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md](reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md) | `9d01086ad3f56105e6e32b4911fcecfef8028b1f` | 固定输入 `af9198906cf89b6fb5162aea1d74683906740d78` 的**先审计、未整改**报告，五项发现先于规范修改 |
| AF-021～AF-025 修订复核 | [docs/reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md](reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md) | `36870185a6e51cf8d0b1ece0dc70d6277c3629a1` | 修订固定提交 `ac683f41af2f906137705f3ad31e8da53fe5c036` 的 **5/5 纯文本合同复核**，不是 CMake/CTest 或 L2/L3 验收 |
| 七份在审文档静态扫描 | [docs/reviews/AUDIT_SEVEN_MATH_DOCS_SCAN_2026-11-08.md](reviews/AUDIT_SEVEN_MATH_DOCS_SCAN_2026-11-08.md) | `fbf2e0e8f7963b51e7037a42714b89320773ff1a` | 固定审计输入 `e190ed74240bf7b3b32a65c8fb6099fbdedc46b9`；对现存原 .m 的 KvD、Adaptive、球谐、采样、轨迹、分析、Golden 文档逐项核查并先登记 AF-026/027 未整改 |
| 七份数学文档/AF-026～027 修订复核 | [docs/reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md](reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md) | `4a9d9698652caf3f3505bb877a0e1ba4db4cc154` | 文档修订固定提交 `22bc38635b2350bc9078c2952ba9ad520a55f7b1`，七份可用原源静态数学合同核对及 APL/AN-G 编号、L2/L3 来源两项修订；未验原论文 PDF 和独立 L3/CTest |

原审批与 `docs/reviews/*.md` 保存历史决定、历史 SHA 和当时的验证信息，不随当前版本重写。特别是先前的 26 条 HLD 边证据只能用于当时快照；目前 HLD 的 27 条边只做过 GitHub 连接器读取下的静态无环检查，**不是 CMake 构建证据**。

**审计进度与整改状态（仅文档审计）**：[AUDIT.md](AUDIT.md) 的 **48 份文件三态均为「已审计」**，指本轮在可用原 `.m` 和规范条件内完成的静态文档审阅；[AUDIT_PROCESS.md](AUDIT_PROCESS.md) 的 **28/28 原子整改目标均为「已整改」**，AF-026/027 已遵守先扫描/登记、后改源、固定 SHA 复核的顺序。旧 [FULL_DOCUMENT_AUDIT](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md) 和各次 AF 原报告的旧状态是它们当时固定输入快照的历史事实，不能改写。**Ivanic 原论文与勘误 PDF、R5 独立参考/其它 L3、生产代码/CTest、DEC-005/006 和正式冻结仍缺，不能从“已审计”推出软件/数学验证通过。**

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
