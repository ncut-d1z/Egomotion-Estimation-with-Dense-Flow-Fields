# 全工程文档规范审计进度与整改台账（AUDIT_PROGRESS）

> **状态：基准登记 v0.1，尚未启动实际逐文件审计。** 审计准则及用户逐文档意见见 [AUDIT.md](AUDIT.md)。固定仓库基准：`cb3129b1510f4ffc5fadeb94b8fd67bdff0d5e82`；本文件首先登记用户提供的36项判断和六类问题。**用户意见不是本轮已经执行的源码审计；状态与整改结果仅随实际读取和提交证据变更。**

## 1. 口径、状态和可复核方式

- **36个目标文档**：对应用户两张表的17个核心规范/专题和19个支撑/过程/证据文档。当前审计文件 `AUDIT.md` 已替换为新的全工程范围，但低阶球谐原版仍在[固定历史提交](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/cb3129b1510f4ffc5fadeb94b8fd67bdff0d5e82/docs/AUDIT.md)。新增本文件是过程台账，不在36个待审目标中重复计数。
- **文件审计状态**：`未审计` → `正在审计` → `已完成限定审计`。结束审计只代表用户给定维度与记录的核查范围已检查、缺口已明确登记，**不表示文档冻结/代码通过/缺口全部修复**。
- **整改状态**：`未整改`／`已整改`，另设阻塞性质 `纯文档/需软件证据/需批准`。只有完整验收条件满足、相应固定提交或外部批准可追溯时才可标已整改；不得为了提高比例拆问题。
- **证据字段**：DOC编号、路径、初始用户判断、实际检查点、文件blob、问题ID、修订提交、剩余风险；对 66TC/98P/27DAG/L2/L3/L4 只记录有来源的数值，不伪造代码执行。
- **历史隔离**：`AUDIT_PROCESS.md`仍是SH-01～SH-06的低阶球谐专项理论整改记录，原97项全工程历史另有固定提交；本次新DAR不复用旧AF状态或承诺它们已消失。

## 2. 全36份审计对象（先登记用户意见，尚未独立核对）

| ID | docs/下相对路径 | 用户初始规范判断 | 首轮实际需核对的问题 | 审计状态 | 本轮证据／整改关联 |
|---|---|---|---|---|---|
| DOC-001 | `REQUIREMENTS.md` | 草案级部分符合 | AC→LLD章节身份/正式批准 | 未审计 | — |
| DOC-002 | `ARCHITECTURE.md` | 草案级部分符合 | 27拟议DAG与真实构建/ADR | 未审计 | — |
| DOC-003 | `OBJECT_MODEL.md` | 草案级部分符合 | 候选类/生命周期/签署 | 未审计 | — |
| DOC-004 | `design/README.md` | 索引符合，非LLD | 实际Markdown个数与模块覆盖条目 | 未审计 | — |
| DOC-005 | `design/contracts.md` | 草案级部分符合 | Result/ABI/ODR/异常线程/自包含 | 未审计 | — |
| DOC-006 | `design/cli.md` | 草案级部分符合 | CLI负例、RunId、JSONL、DEC-008 | 未审计 | — |
| DOC-007 | `design/mat_io.md` | 草案级部分符合 | Reader/Writer、TOCTOU、写回读、L2 | 未审计 | — |
| DOC-008 | `design/sampling_noise.md` | 草案级部分符合 | 生产几何/RNG负例/CTest | 未审计 | — |
| DOC-009 | `design/trajectory.md` | 草案级部分符合 | Sophus真实依赖及几何L3 | 未审计 | — |
| DOC-010 | `design/kvd.md` | 草案级部分符合 | 36轮数学非真实CTest/200轮 | 未审计 | — |
| DOC-011 | `design/harmonics.md` | 草案级部分符合 | 低阶叶子有实测，公共ABI未冻结 | 未审计 | — |
| DOC-012 | `design/adaptive_mfa.md` | 草案级部分符合 | Session原子提交、病态输入和真实软件 | 未审计 | — |
| DOC-013 | `design/experiments.md` | 草案级部分符合 | Runner/Writer/Manifest/Ctest | 未审计 | — |
| DOC-014 | `design/analysis_plotting.md` | 草案级部分符合 | 分析L2/L3曲线、CTest/绘图隔离 | 未审计 | — |
| DOC-015 | `design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 专题草案级部分符合 | 旧版本CMake描述与最新低阶叶子验收区分 | 未审计 | — |
| DOC-016 | `design/SH_DISCRETE_REFIT_CONSISTENCY.md` | 理论专题合格，非LLD | 不能替代模块LLD/测试/批准 | 未审计 | — |
| DOC-017 | `design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md` | 理论专题合格，非LLD | 不能替代Adaptive真实接口/实现 | 未审计 | — |
| DOC-018 | `TEST_PLAN.md` | 计划草案部分符合 | 66TC登记与真实运行证据区分 | 未审计 | — |
| DOC-019 | `PLAN.md` | WBS草案符合 | 98P、M0–M5、真实工期/性能 | 未审计 | — |
| DOC-020 | `PROGRESS.md` | 符合证据登记 | 固定SHA与历史执行/新状态分离 | 未审计 | — |
| DOC-021 | `AUDIT.md` | 原专项→本轮综合审计 | 已在初始提交转换为全工程文档规范 | 未审计 | — |
| DOC-022 | `AUDIT_PROCESS.md` | 球谐专项整改，不适用全工程 | 保留SH-01～06，引用原专项快照避免误链 | 未审计 | — |
| DOC-023 | `DOCUMENT_BASELINE_REGISTER.md` | 部分符合 | 36项固定历史SHA与当期HEAD/批准分开 | 未审计 | — |
| DOC-024 | `SOURCE_INVENTORY.md` | 静态来源追踪符合 | 45 .m/50函数、MAT未独立SHA | 未审计 | — |
| DOC-025 | `MIGRATION_MATRIX.md` | 静态迁移追踪符合 | 45/50候选映射，不是代码实现 | 未审计 | — |
| DOC-026 | `TODO.md` | 管理清单符合 | 历史审批与分阶段授权时间线 | 未审计 | — |
| DOC-027 | `decisions/DEC-004_HISTORICAL_MAT_FIELDS.md` | 决策记录符合 | HistoricalOnly与当前save/代码未测试 | 未审计 | — |
| DOC-028 | `decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md` | 所有者决定符合 | 技术方向≠软件运行/基线签署 | 未审计 | — |
| DOC-029 | `decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md` | 专项授权记录符合 | 历史SH局部权限不得扩展 | 未审计 | — |
| DOC-030 | `approval/DECISION_IMPLEMENTATION_NOTES.md` | 审批落实记录符合 | 旧draft/权限不当最新批准 | 未审计 | — |
| DOC-031 | `approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md` | 审批包符合，非规范 | applicationCommit/scopeIds空 | 未审计 | — |
| DOC-032 | `golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | 证据等级规划符合 | L2/L3/L4缺；球谐3/3状态需避免旧措辞 | 未审计 | — |
| DOC-033 | `golden/GOLDEN_TOLERANCE_PROPOSAL.md` | 容差候选符合 | 0.01/0.1筛查非DEC-005 | 未审计 | — |
| DOC-034 | `golden/NO_MATLAB_EXECUTION_POLICY.md` | 长期政策符合 | 取消L1，不取消软件验证 | 未审计 | — |
| DOC-035 | `golden/SH_PAPER_VERIFICATION_CASES.md` | 验证候选符合 | SH-P候选未执行，区别原生已测案例 | 未审计 | — |
| DOC-036 | `golden/analytic_golden_candidates.v1.json` | JSON状态正确，非Markdown | DRAFT_UNAPPROVED_UNEXECUTED不是Golden批准 | 未审计 | — |

**基准统计（先写入意见后的第0阶段）**：36份待审=0已完成、0正在、36未审计；6个DAR目标=0已整改、6未整改。之后更新必须填写开始/结束/净变化，且每个实际问题关联具体文件及确定的核查内容。

## 3. 初始整改目标（来自用户的六条主要建议）

| 目标 | 涉及文件 | 发现/整改方向（待实际验证） | 完整验收条件 | 阻塞性质 | 状态 |
|---|---|---|---|---|---|
| DAR-001 | REQUIREMENTS.md；DOCUMENT_BASELINE_REGISTER.md | SRS AC→LLD路径/章节/版本/blob/批准身份目前不是正式闭合链 | 将现有导航与冻结义务明确分离，取得正式各方批准及固定身份再可结案 | 需批准／冻结 | 未整改 |
| DAR-002 | ARCHITECTURE.md；PLAN.md | 27条设计DAG、部署/性能预算和ADR容易与真实构建混淆 | 逐项标出设计/实测/批准三态；真实target/性能及DEC-006后才能宣布完成 | 需软件证据／批准 | 未整改 |
| DAR-003 | design/contracts.md；design/cli.md；design/mat_io.md；design/adaptive_mfa.md；design/harmonics.md | 大量公共接口/ABI、Session与错误合同仍是候选签名 | 先核正文是否注明未实现，正式代码/CTest/ABI与批准俱全才结案 | 需授权测试／批准 | 未整改 |
| DAR-004 | TEST_PLAN.md；PROGRESS.md；golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md | 66 TC的候选覆盖与球谐3/3固定SHA的真实执行必须分层 | 表头/现行段显式区分历史/执行/批准；完整66TC要真实软件证据 | 需软件证据／批准 | 未整改 |
| DAR-005 | DOCUMENT_BASELINE_REGISTER.md；REQUIREMENTS.md | 36文件版本矩阵是较早固定快照而非当前HEAD基线 | 补真实当前HEAD规范身份/范围/批准人/日期/例外后才允许签正式冻结 | 需批准／当前SHA | 未整改 |
| DAR-006 | AUDIT.md；AUDIT_PROCESS.md；design/README.md | 此前AUDIT.md属于球谐专项，与本轮全工程文档审计及目录新增专题文件的分类/计数不一致 | 建立全工程审计规则和独立进度，固定球谐历史来源，核对design目录实际分类/数量并更新相关文字 | 纯文档 | 未整改 |

## 4. 已实施审计、具体整改和验证日志

尚无。本轮将在完成上述意见基准提交后，逐文件实际读取并登记具体检查范围；核对不符时在本节追加复核和失败记录。**不得把该空白初始状态写成全部文档不合格。**

## 5. 各轮计数

| 阶段 | 文件总数 | 已完成限定审计 | 正在审计 | 未审计 | 整改目标 | 已整改 | 未整改 |
|---|---:|---:|---:|---:|---:|---:|---:|
| 第0阶段：用户意见录入、实际审计前 | 36 | 0 | 0 | 36 | 6 | 0 | 6 |

## 6. 结案纪律

针对文档措辞/数量错误可以完成纯文档整改；但AC冻结、正式接口ABI、真实C++/CTest/写回读、独立MAT SHA-256、DEC-005/006/008及所有者签字，不得通过编辑Markdown宣布完成。严格保持MATLAB/Octave/L1**永久非必需**。代码/原MAT修改、编译、运行及发布不属于本轮用户指示。
