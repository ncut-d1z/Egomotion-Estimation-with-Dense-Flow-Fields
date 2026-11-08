# 文档类型、版本、Git SHA 与基线状态控制矩阵

> **文件身份登记，不是已签署基线。** 下列 §2～3 的 **36 项选定文件**按固定提交 `3e3ad2aba75d6ec8d16d2875ad3c846cee7d671d` 登记 Git blob SHA（包含规范、附件与历史证据，不等于全部核心规范）；本矩阵自身的后续修改不在该快照中。该固定快照包含 SRS v0.13、MAT I/O v0.9、experiments v0.4、PLAN/TEST_PLAN/LLD 索引 v0.7；**后续 TEST_PLAN v0.8 的身份另记 §6**；**所有规范仍未冻结，新增审计对象不表示通过验收**。

## 1. 文档职责与唯一规范来源

**范围（AF-036）**：本矩阵是所选 **36 项固定版本身份**的管理清单；[AUDIT.md](AUDIT.md) **动态**维护完整审计对象集合与逐文件三态（包括仓库配置、历史 reviews 与候选 Golden JSON）；该集合自旧快照的 50 个路径继续增长，**现行数量以 AUDIT 表为准**，不是本矩阵的固定 36 个身份。审计仅用于静态审查，不代表正式基线。两组数量按用途不同，不可互作验收分母；正式签署需另填批准人与证据。


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

## 2. 当前规范及核心文件的 blob SHA

**记录快照：** `3e3ad2aba75d6ec8d16d2875ad3c846cee7d671d`。以下 SHA 是该提交的文件身份；真实业务实现、独立 L2/L3、CTest 和批准尚未发生。表中 PROGRESS 等过程件的正式基线列不适用。

| 层次 | 文档 | 文档版本 | Git blob SHA | 正式基线 | 范围 |
|---|---|---|---|---|---|
| SRS | [docs/REQUIREMENTS.md](REQUIREMENTS.md) | v0.13 | `f7f024e2a2a6a397d19b2dddeb41c1790a6aa6d4` | 未冻结 | REQ/AC + 附录 A 设计约束 |
| HLD | [docs/ARCHITECTURE.md](ARCHITECTURE.md) | v0.8 | `3a79a1a566a1f51cdec3905d1b220093d5121b33` | 未冻结 | 组件/部署/27 条 target 边及 ADR |
| HLD 附件 | [docs/OBJECT_MODEL.md](OBJECT_MODEL.md) | v0.5 | `26db2608f60826b89bd8341d237ad4207a87dd29` | 未冻结 | 对象生命周期/45 文件映射 |
| 跨模块 LLD | [docs/design/contracts.md](design/contracts.md) | v0.2 | `ccc90e7ba40b65d0dd64bafdbda55ac02543a33f` | 未冻结 | Result/Status/ErrorDetail 与领域数据 |
| CLI LLD | [docs/design/cli.md](design/cli.md) | v0.2 | `94fffed91f5f3ff8691f50eb69d9a81ece549c51` | 未冻结 | 命令、JSONL、固定输出根 |
| MAT I/O LLD | [docs/design/mat_io.md](design/mat_io.md) | v0.9 | `08d3cfbe464978792cc7a6eb6ae17d8da3bf08f1` | 未冻结 | 自拥有数据/Reader/Writer/安全 |
| KvD LLD | [docs/design/kvd.md](design/kvd.md) | v0.4 | `b59a151a0bdb7942c999538dffa94dbee02b655f` | 未冻结 | Franz/KvD 与逐轮顺序 |
| Adaptive LLD | [docs/design/adaptive_mfa.md](design/adaptive_mfa.md) | v0.4 | `c08f69f147845585370e789e3d34ab08d98dfeae` | 未冻结 | StepSolver/Session 失败不提交 |
| 球谐 LLD | [docs/design/harmonics.md](design/harmonics.md) | v0.7 | `e8b28b096f8c4b2fdcd6aa6cfadca26d29fe8f2e` | 未冻结 | R3/R5、Eigen-only/禁 sdla |
| 采样/噪声 LLD | [docs/design/sampling_noise.md](design/sampling_noise.md) | v0.3 | `d4a73fe9d5dd6a984228ec2ef0e4a3ac928e5878` | 未冻结 | 有序采样、RNG/噪声 |
| 轨迹 LLD | [docs/design/trajectory.md](design/trajectory.md) | v0.2 | `9f1d0bf0604f515b17bca58b94ab7d8311b850cb` | 未冻结 | 轨迹/运动/近度 |
| 实验 LLD | [docs/design/experiments.md](design/experiments.md) | v0.4 | `d3f967c6809e91fe3264239f4ed885de034128ea` | 未冻结 | 五入口/当前 save、HistoricalOnly |
| 分析 LLD | [docs/design/analysis_plotting.md](design/analysis_plotting.md) | v0.4 | `7544280ee47402ecd4630bf97544ff02254dd225` | 未冻结 | L2 读取、L3 曲线/缺字段拒绝 |
| LLD 索引 | [docs/design/README.md](design/README.md) | v0.7 | `0ce69a28afdf59af0f36c640696b7816c9d970b0` | 未冻结 | 模块 LLD 范围/冻结条件 |
| 实施 WBS | [docs/PLAN.md](PLAN.md) | v0.7 | `646b517bf206b3df02c7da4251c8bc9a1fbbcdc2` | 未冻结 | 98 个 P 工作项 |
| 测试计划 | [docs/TEST_PLAN.md](TEST_PLAN.md) | v0.7 | `3abee9cfa5d30ae7b98e8f438653a87981ae1a94` | 未冻结 | 66 TC→候选测试→AC |
| 证据登记 | [docs/PROGRESS.md](PROGRESS.md) | v0.1（滚动） | `8502338222f43b912120173340954b3404d82a17` | 不适用（滚动证据登记） | 真实结果/历史证据/DEC 当前状态 |

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
| 开发规则 | [AGENTS.md](../AGENTS.md) | `296a49fd7a09e71796ea7ebff984a2904505bc2d` | 只读原资料/实际运行授权 |
| 延期人类事项 | [docs/TODO.md](TODO.md) | `c3f6d4b79dd7c0bbe4074006c75f214b182f2af4` | 材料、H-10/H-11、DEC-005/006、最终基线/发布：原因及建议阶段，**没有当前授权** |
| 本轮只读审查 | [docs/reviews/DOCUMENT_REVIEW_AND_SH_DECLARATIONS_2026-11-07.md](reviews/DOCUMENT_REVIEW_AND_SH_DECLARATIONS_2026-11-07.md) | `580e98eae40a5378128b139ce531b6ded11c6c2e` | 历史固定 SHA 下的 GitHub 只读 ID/DAG/引用核对，非真实 Bash/CTest |
| 用户历轮审计台账 | [docs/AUDIT.md](AUDIT.md) | `3e849f76f105aba385cb57407cd2000e9dd16813` | AU-001～028；50 个路径：26 已审计、24 在审（静态范围，非基线） |
| 问题级整改进度 | [docs/AUDIT_PROCESS.md](AUDIT_PROCESS.md) | `74035309ccf7b2f23784ab46938fc1f8302eab58` | AF-001～062：32 已整改、31 未整改（固定快照，非测试验收） |
| 本轮完整审计报告 | [docs/reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md) | `43446ec3e7ebe3d9b7f2447561f72d64ccbb5e95` | 固定输入 commit `4627cc1e48be9c43ad10b28fda646d65ee8ef01c` 的 **40 文件文档审计**，记录 AF-001～012 待修订项与 AH-001～002 历史边界，非 CMake/CTest PASS |
| AF-001～AF-012 修订复核 | [docs/reviews/AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md](reviews/AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md) | `ae44ae6ba782a9930b400ea860a50cc326a0cd3f` | 定向文本核验快照 `893acb7b07862c0701e62b786462ef8395a116da`：12 项 AF 已做纯文档修订并通过限定静态复核；仍缺逐式数值、代码编译和正式测试 |
| AF-013～AF-020 文档修订复核 | [docs/reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md](reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md) | `1466ca9534a54cac6bf9b21a5b2bf42491d6e6a9` | 固定修订/静态复核 commit `a782b5c8d86e50fe4d2ae4e2b198b3cd6345de52`；八条 AF 均完成合同文本修订，非真实软件/数值通过 |
| AF-021～AF-025 扫描 | [docs/reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md](reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md) | `9d01086ad3f56105e6e32b4911fcecfef8028b1f` | 固定输入 `af9198906cf89b6fb5162aea1d74683906740d78` 的**先审计、未整改**报告，五项发现先于规范修改 |
| AF-021～AF-025 修订复核 | [docs/reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md](reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md) | `36870185a6e51cf8d0b1ece0dc70d6277c3629a1` | 修订固定提交 `ac683f41af2f906137705f3ad31e8da53fe5c036` 的 **5/5 纯文本合同复核**，不是 CMake/CTest 或 L2/L3 验收 |
| 七份在审文档静态扫描 | [docs/reviews/AUDIT_SEVEN_MATH_DOCS_SCAN_2026-11-08.md](reviews/AUDIT_SEVEN_MATH_DOCS_SCAN_2026-11-08.md) | `fbf2e0e8f7963b51e7037a42714b89320773ff1a` | 固定审计输入 `e190ed74240bf7b3b32a65c8fb6099fbdedc46b9`；对现存原 .m 的 KvD、Adaptive、球谐、采样、轨迹、分析、Golden 文档逐项核查并先登记 AF-026/027 未整改 |
| 七份数学文档/AF-026～027 修订复核 | [docs/reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md](reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md) | `4a9d9698652caf3f3505bb877a0e1ba4db4cc154` | 文档修订固定提交 `22bc38635b2350bc9078c2952ba9ad520a55f7b1`，七份可用原源静态数学合同核对及 APL/AN-G 编号、L2/L3 来源两项修订；未验原论文 PDF 和独立 L3/CTest |

原审批与 `docs/reviews/*.md` 保存历史决定、历史 SHA 和当时的验证信息，不随当前版本重写。特别是先前的 26 条 HLD 边证据只能用于当时快照；目前 HLD 的 27 条边只做过 GitHub 连接器读取下的静态无环检查，**不是 CMake 构建证据**。

**审计/整改快照说明**：在所登记提交上，AUDIT 的 50 路径中 **26 已审计、24 正在审计**（静态文档范围）；AUDIT_PROCESS 共 **63 个原子事项：32 已整改、31 未整改**（新增 AF-062 已登记，尚未改目标文件）。本矩阵随后的修订会改变最新文件 SHA，但**不改变该固定快照的历史事实**。所有真实 C++、MAT 逐值、L3、CTest、DEC-005/006/008 与规范冻结仍待独立证据或批准。

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


## 6. 后续变更

最新审计和问题状态始终以 [AUDIT.md](AUDIT.md)、[AUDIT_PROCESS.md](AUDIT_PROCESS.md) 为准，实际执行证据以 [PROGRESS.md](PROGRESS.md) 为准；此处保存上方**单个固定提交的文件身份**。后续修订应另登记新固定提交，禁止把旧 SHA 当作 HEAD，亦不得借登记宣称签署或 CTest 通过。


### 6.1 本轮新增文件身份（仅记录，不冻结）

| 变更文档 | 修订版本 | 固定复核提交 | Git blob SHA | 效力 |
|---|---|---|---|---|
| [docs/TEST_PLAN.md](TEST_PLAN.md) | v0.8 | `edd56c99dc04e3774788699ed61c26d3bad3c465` | `c867d1fb8aa13203bb96ab4e41412b67b77ce940` | 修正五处尚未创建 SOURCE_INVENTORY 的引用口径；无新增 TC/CTest 或软件验收 |
| [docs/PLAN.md](PLAN.md) | v0.6（仅补静态核对） | `6a1867a4b36ad71b1a523c4b3ed7372e276ece5d` | `cda72332eb32a77a5c353929011dcc47663d31d4` | AF-052 WBS 98 P 静态追踪/退出证据审阅；M0～M5 均未通过 |
| [docs/design/README.md](design/README.md) | v0.7（仅补检查清单） | `6a1867a4b36ad71b1a523c4b3ed7372e276ece5d` | `021174c363d18d81773acbc8f6b61364a9059faa` | AF-059 10 模块 LLD 冻结前核对；不等于正式冻结、已编译或 CTest PASS |

原 §2～3 的 36 项身份仍是较早固定快照，**不可**以新版本 blob 直接替换旧快照行并继续声称同一提交身份。审计新问题的状态见 [AUDIT](AUDIT.md)、[AUDIT_PROCESS](AUDIT_PROCESS.md)；本矩阵和滚动审计文件本身修改后的最新 blob 身份需在后续正式固定版本时统一登记。所有 SRS/HLD/LLD/TEST_PLAN 仍未冻结。


### 6.2 AF-037～041 第一批文档修订的固定文件身份（仅设计草案）

以下文件身份**属于固定提交** `306b9ac4e8c832344d703b8d29d371710055ae86`，不得与 §2–3 较早固定快照混成同一 SHA，也不因在本表登记就提升为已冻结。

| 变更路径 | 文件版本 | 该固定提交的 Git blob SHA | 审计用途及限制 |
|---|---|---|---|
| `docs/REQUIREMENTS.md` | v0.14 | `51c0cfed22a80340eaadce059e5f579e9c56f3ef` | AF-037：AC/LLD 身份门禁，未冻结 |
| `docs/ARCHITECTURE.md` | v0.9 | `549a74093bc128fb948e3e98a98f008e42ec585d` | AF-038：Writer/Session/HLD 责任对照，未编译 |
| `docs/OBJECT_MODEL.md` | v0.6 | `9bc35d7d8b66c6337b85a9e37e3de530e0278176` | AF-039：唯一签名/所有权来源，待批准 |
| `docs/design/contracts.md` | v0.3 | `cef56ac06e4f552d65790d8d42f3caff365a6c53` | AF-040：variant/错误边界待编译 |
| `docs/design/cli.md` | v0.3 | `4e8f2cce1f6dad75ba576f8768646aa0baa900c0` | AF-041：端值/RunId/JSONL 候选待测试 |
| `docs/PLAN.md` | v0.7（引用更新） | `58fef1368f9a866a5f39de0a8a16f80dab50172e` | SRS v0.14 引用，同步非动态进度 |
| `docs/TEST_PLAN.md` | v0.8（引用更新） | `4f02e3c5b66b8cab6e0b633d8ce5b4b0fe306a28` | SRS v0.14 引用，未执行 CTest |

上述目标仍为 AF-037～041 **未整改**，正式批准、接口实现/ABI、CMake/CTest/独立 L2/L3/L4 和 DEC-005/006/008 未完成；当前状态统一见 [AUDIT_PROCESS](AUDIT_PROCESS.md)。此登记不是软件或数学验收证据。


### 6.3 本次第一轮设计修订的固定文件身份（非正式基线）

以下 **8 个文件的 Git blob** 对应固定源提交 `f846e7b96a1b75b5d1d29621b4a295fd6504bade`；不要将它们覆盖写入 §2 原快照并声称同一身份。后续审计滚动文档的 SHA 另在后续固定提交登记。

| 路径 | 版本 | 固定提交 blob SHA | 说明 |
|---|---|---|---|
| `docs/design/mat_io.md` | v0.10 | `b8742f42a4e28578ad9cd01bfbbae1637ad44035` | AF-042：独占写出/竞态与读取夹具；未实现 |
| `docs/design/sampling_noise.md` | v0.4 | `d126d0037231ce1e8a6f81adc2a8e973501b3b5b` | AF-047：有序采样/RNG 合同；未生成 L3 |
| `docs/design/trajectory.md` | v0.3 | `901104da31bb574bf6acd47ea0f9422590e7609f` | AF-048：Sophus/球内正根边界；未编译 |
| `docs/design/experiments.md` | v0.5 | `9c94b27d9bb4b38eb15d58fb13bb5d75fb88c123` | AF-049：Runner/Manifest 夹具；未实现 |
| `docs/design/analysis_plotting.md` | v0.5 | `1352c66499b3e20b5ad17879bcddb0c5d59f0a41` | AF-050：四入口分析场景；缺真实曲线 |
| `docs/TEST_PLAN.md` | v0.9 | `4dc15c476f5af1ba3fe7cca071aadd90d1167d14` | AF-051：已有 66 TC 的夹具补充；CTest 未运行 |
| `docs/golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | 未冻结附件 | `9432d537a1aaa169450e873eb4097126b9d35077` | AF-060：L2/L3/L4 证据口径；数值未生成 |
| `docs/design/README.md` | v0.7（MAT 版本引用更新） | `c437eaba39e7f39e1a378c9df8756a7db6d8c6e4` | 仅引用 mat_io v0.10；不冻结 LLD |

**本表仅作文件身份追踪**：AF-042/047/048/049/050/051/060 仍未整改，未取得实际 C++/CTest、独立 L2/L3/L4、最终 DEC-005/006/008 或正式批准；两台账当前进度见 [AUDIT](AUDIT.md) / [AUDIT_PROCESS](AUDIT_PROCESS.md)。


### 6.4 数学文档和 JSON 的本轮固定身份（未冻结）

本节不改写 §2～3 的历史快照；`d5265cc576e1c66ba1c63f8621ff1ad91d981195` 是**静态数学设计修订**，`18ddb7198490e9d2ff49ee2f66e8b3a4945ee58a` 是**JSON 来源 SHA 修订**。

| 文件路径 | 文件 Git blob SHA | 修订输入/作用 |
|---|---|---|
| `docs/design/kvd.md` | `e5e4281ef3c3b704b616ba070f75e6dab7f6834d` | `d5265cc`；AF-043 静态来源与分支判别 |
| `docs/design/adaptive_mfa.md` | `eb1d9f43083a203b240dc66f071cca7f9f6fb336` | `d5265cc`；AF-044 负均值/矩阵合同 |
| `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | `bb5498ad6541bfa90cd7c3562eeddd212b331954` | `d5265cc`；AF-054 未来区分性 L3 条件 |
| `docs/TEST_PLAN.md` | `c19a01f44c59e3fcaed7bca58a8303866272ad18` | `d5265cc`；AF-051 对应既有 TC |
| `docs/golden/analytic_golden_candidates.v1.json` | `1e6499eff3ce5b55503f8cd913b9f492629da499` | `18ddb719`；AF-067 补齐一处原源 blob SHA |

**范围**：仅文档和参考元数据，AF-043/044/051/054/055 及其它需要真实数值/源码/验收的阶段目标仍未完成。无正式文档签署、软件构建、CTest 或 MAT 读写证据。


### 6.5 本轮治理文件的复读身份（非冻结）

固定复读提交 `fa85cc638e58638768506b25c4b375566f6e8379`：`docs/AUDIT.md` blob `55ab0daad40d23ab0a12edf9ab20820d2a6fa05b`、`docs/AUDIT_PROCESS.md` blob `0d5b59847f2a7a09a8f7d0aaf8a6daae02b19a2c`、`docs/PROGRESS.md` blob `006d55de69f6e2fa98c01caa7163499b35c6bab4`。审计规则和标题核对仅属静态治理修订，后续动态台账有变化时依新的固定提交复读；正式 SRS/HLD/LLD/DEC/测试仍未冻结或验收。


### 6.6 候选公式与测试规划文档的本轮身份（非冻结）

本次静态复核输入 `a27b092421a0465b10fb46054d158ab06a2c729f`（历史 §2～3 的固定文件集合与结论不被改写）：

| 当前文件 | 该提交的 Git blob SHA | 新草案 / 限制 |
|---|---|---|
| `docs/design/cli.md` | `a5a8230e4ec98e11f57ac095f529122603baf1a6` | v0.4；负噪声只是原源码静态候选 |
| `docs/design/sampling_noise.md` | `1435f344755a2b119cbc897a63a5a17a9cd48910` | v0.5；零/负尺度 3N 抽样静态合同 |
| `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | `bf2866c853e192fdc25f1dab9cfd93ca94877f26` | v0.4；8 组局部符号复核非 L3 PASS |
| `docs/TEST_PLAN.md` | `9a464e37dc25d249749c27bafad644d968c71dda` | v0.10；未创建/执行 66 TC |
| `docs/reviews/AN_G_STATIC_FORMULA_REVIEW_2026-11-08.md` | `e82f04464e09ebcc29024fdf203368ed4091ebb4` | 只读原式的局部代数证据 |

AF-041/047/055/058 与其它开放目标仍须独立数据/真实软件及审批；本矩阵新增章节不构成签署冻结。


### 6.7 本轮源文件清单及迁移矩阵的固定身份（非冻结）

固定静态复核提交 650546a06ff7bb89b8c6bf29cac48f712ed2f2fa：

| 文件 | Git blob SHA | 范围 |
|---|---|---|
| `docs/SOURCE_INVENTORY.md` | `3159b8c93219a7bc828fc8daa23aba536c36a948` | 45 个原 .m 的 Git 身份、字节与 REQ/TC |
| `docs/MIGRATION_MATRIX.md` | `8926c4fb9e37118f469660c6ce3c6f83f9b906c5` | 45 个原 .m 的候选 C++ 责任，未实现 |
| `docs/PLAN.md` | `74bfba0b94c4a0cb615a6cea1da1f03b567322eb` | v0.8，P0.6 阶段事实更新 |
| `docs/TEST_PLAN.md` | `bc326b773e6d627433c3b4ea06f5378f9ee04bf1` | v0.11，五处来源清单引用更新 |

不扩大旧固定 36 文件快照的签署含义；当前审计集合增至 53 文件，所有软件/验收门槛保持原状态。


### 6.8 函数级迁移清单及审计台账的定向身份（未冻结）

- 固定函数清点提交 `150b975db4045fbc99a343fdf422c7836e1a797b`：`docs/MIGRATION_MATRIX.md` blob `3d61d69f7962a29c8720528f7fa71628c7008331`，`docs/SOURCE_INVENTORY.md` blob `9d72f7517e93f774c4b67338cc6b93f6723e7712`（45 文件、50 个函数声明，不含数学逐式验收）。
- 固定审计压缩提交 `ae831be3a0901d69c0897044fd339fda38468963`：`docs/AUDIT.md` blob `05f8576cfdd868d4a846fa2773140eeb1891ac5e`，`docs/AUDIT_PROCESS.md` blob `926276afe93fa88d30d5113e94b68d14773685d2`，只作**当时**静态表行与文档压缩复核，不代表随后滚动状态或正式 ApprovedBaseline。


### 6.9 AN-G08 特例静态审阅文件身份（未冻结）

固定审阅提交 `ee3d9c9bb42e6d71f9808510402d1394e385647d`，仅登记该时点的文件 Git 身份：

| 文件 | Git blob SHA | 复核边界 |
|---|---|---|
| `docs/reviews/AN_G08_Z90_STATIC_CONVENTION_2026-11-08.md` | `903544444af25a0a1832822225b3f635ab4cf573` | z90 系数的三角恒等式与 R3 原赋值；非通用 R5 L3 |
| `docs/design/harmonics.md` | `05eac137a3334ad2d14ce1c0f5e5d2d0d4e471e0` | §7.1.1 明确 m 次序、系数方向，不是 Eigen 生产实现 |
| `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | `82687c7638e989eb06792139bf1adbc929409de2` | AN-G08 局部符号级来源，DEC-005 仍未批准 |
| `docs/AUDIT.md` | `b65b70a0c43d95b1a9d8ecc52dea7bfbd06e9ca3` | 审计对象增加该历史报告后的固定身份 |

原 36 选定基线矩阵的历史快照保持不变；本节不增加 ApprovedBaseline 或通过状态。


### 6.10 分段分析合同与测试计划版本身份（未冻结）

固定静态复读提交 `d8e7e64e5affd97130a2b7436a039e4681de502d`：

| 文件 | 复读时的 Git blob SHA | 意义 |
|---|---|---|
| `docs/design/analysis_plotting.md` | `0eac034b5d54e99b498c1044e8e4f03728599af1` | v0.6；AF-075/076 已按源码明确旋转段级/平移全域数据路径 |
| `docs/TEST_PLAN.md` | `cd977141f8e7bfa63e0d6748eb80e50ac58fe6d3` | v0.12；沿用 66 TC，未生成/运行分析测试 |
| `docs/reviews/AUDIT_ANALYSIS_SEGMENT_SMOOTHING_CORRECTION_2026-11-08.md` | `fd30206e5fc7a29125d4b64946354d09b6156558` | 扫描错误断言的后续纠偏，旧报告不回写 |

正式 L2/L3/Analyzer/CTest、原文件哈希与批准仍未通过，历史 §2～3 的固定 36 文件登记不作为 HEAD 当前基线。


### 6.11 分析错误协议的当前草案文件身份（非冻结）

固定修订及静态复读输入 `e3c4c6794b3d2b562ebc39e92251eaaa52f44c34`：

| 文件 | Git blob SHA | 文档合同作用 |
|---|---|---|
| `docs/design/contracts.md` | `37326d9c1b1e0949fcbcc3b1b71bf7c6d5bc13a0` | v0.4；AnalysisErrorCode 的对外候选 stage/exit 单一映射 |
| `docs/design/analysis_plotting.md` | `3e301b6312f4ef154bdd7cbcfe953fa7548a971a` | v0.7；原七 enum 仍属本模块，公共合同仅提供映射 |
| `docs/design/cli.md` | `15524a70b97cbc58bd6e7b6254d6086ef6c882bf` | v0.5；区分 parse(2)、read(3)、decode(4)、analyze(5) |
| `docs/TEST_PLAN.md` | `d56199932442da7648a6076d6dd91b67361b6d6e` | v0.13；现有 TC 对应候选负例，未实际建立/执行 |

以上仅固定此提交的文档身份；DEC-008/实际 ABI、MAT 数据及 CTest/数值没有签署或通过。


### 6.12 动态索引与轨迹静态候选的固定版本身份（未冻结）

| 文件 | 固定复读提交及 blob SHA | 限制 |
|---|---|---|
| `docs/design/README.md` | `02a2a17` · `0727c8951a2db8405bc4ea840bd1c16705b85703` | v0.8；实时审计三态仅引用 AUDIT、公共分析错误语义不重复定义 |
| `docs/DOCUMENT_BASELINE_REGISTER.md` | `02a2a17` · `cfb3e08f04dcf458a6ca038c1c55e7c3579b2664` | 当前范围链接更新，36 项历史身份快照保持原样 |
| `docs/design/trajectory.md` | `07d3abb` · `b749ba78545d561c664acc079e54cbc1beb6083b` | v0.4；严格反向与 TR-C01～04 仅手算候选 |
| `docs/TEST_PLAN.md` | `07d3abb` · `149fbb7c34843fcb957919deec9b34b447fc0343` | v0.14；原 66 TC 不变，未运行测试 |

以上是修订时点的 Git blob 身份而非已批准版本；实际签署、性能及数值证据仍按 TODO/PROGRESS 处理。


### 6.13 零级采样符号参考的固定身份（未冻结）

固定修订/复读提交 `844b64747f50f0e2284fc0aa61d792d59b82ea2c`：

| 文档 | Git blob SHA | 边界 |
|---|---|---|
| `docs/reviews/AN_G_LEVEL0_FACE_ORDER_STATIC_2026-11-08.md` | `47e50aa5ff1db49e79206425bb218014578cd1f3` | 8/6/20 初始面行序的源码符号推导，不是已执行 L3 |
| `docs/design/sampling_noise.md` | `6a7a4fb36e6f0f31dc5266d262bf1fcb3bdff29b` | v0.6，level0 静态审阅与后续递归缺口 |
| `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | `ebcf703eda27aa0fe22614e274be108eeef28f08` | v0.5，AN-G 候选状态仍未批准 |
| `docs/TEST_PLAN.md` | `e974b80d0df87e61eefb5f84546e40e1f617e1e6` | v0.15，仍仅候选 TC 及数学前置条件 |

此处不改变旧 36 文件固定版本身份矩阵的历史值或任何批准/CTest 状态；最新审计集合与阶段目标分别见 AUDIT/AUDIT_PROCESS。


### 6.14 新的递归行号合同固定身份（非签署基线）

定向静态复核输入 `4f9e48252f8be2994c67e09aefd4c036b38e9f68`，仅保留下列当时 Git blob SHA，不将旧 36 文件矩阵追溯改成新版本：

| 文件 | 该提交的 blob SHA | 范围 |
|---|---|---|
| `docs/design/sampling_noise.md` | `d74ee2fc2fcb55caa235f0030bb9a2309c15f9e6` | v0.7；原递归索引和测试资格，不是 L3 PASS |
| `docs/TEST_PLAN.md` | `38765a42c335254e369aeef83ddbc5e27122dc5b` | v0.16；既有 TC 的额外索引条件 |
| `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | `fe1ad898090eb2d66ca99149dfeac189cf7c5696` | v0.6；level1/2 数值仍待独立 L3 |
| `docs/reviews/AN_G_RECURSION_INDEX_STATIC_2026-11-08.md` | `3d6c44bb6f5a7d1baf31c72dcfd476dd17f0e9e3` | 静态子面顺序、四叉行号的审阅报告 |

**权限和基线界限不变**：此轮没有真实 CMake/CTest、MAT 逐值、数学独立 L3 或批准签署。
