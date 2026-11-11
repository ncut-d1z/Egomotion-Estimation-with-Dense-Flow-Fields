# 全工程文档规范审计进度与整改台账（AUDIT_PROGRESS）

> **状态：审计进度 v0.4，第三批已实际整改SRS的66条AC主责任章节/文件身份、两项误归属、测试计划旧章节及基线旧快照标题；仍是未冻结文档审计，不是软件验收。** 审计准则及用户逐文档意见见 [AUDIT.md](AUDIT.md)。固定仓库基准：`cb3129b1510f4ffc5fadeb94b8fd67bdff0d5e82`；本文件首先登记用户提供的36项判断和六类问题。**用户意见不是本轮已经执行的源码审计；状态与整改结果仅随实际读取和提交证据变更。**

## 1. 口径、状态和可复核方式

- **36个目标文档**：对应用户两张表的17个核心规范/专题和19个支撑/过程/证据文档。当前审计文件 `AUDIT.md` 已替换为新的全工程范围，但低阶球谐原版仍在[固定历史提交](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/cb3129b1510f4ffc5fadeb94b8fd67bdff0d5e82/docs/AUDIT.md)。新增本文件是过程台账，不在36个待审目标中重复计数。
- **文件审计状态**：`未审计` → `正在审计` → `已完成限定审计`。结束审计只代表用户给定维度与记录的核查范围已检查、缺口已明确登记，**不表示文档冻结/代码通过/缺口全部修复**。
- **整改状态**：`未整改`／`已整改`，另设阻塞性质 `纯文档/需软件证据/需批准`。只有完整验收条件满足、相应固定提交或外部批准可追溯时才可标已整改；不得为了提高比例拆问题。
- **证据字段**：DOC编号、路径、初始用户判断、实际检查点、文件blob、问题ID、修订提交、剩余风险；对 66TC/98P/27DAG/L2/L3/L4 只记录有来源的数值，不伪造代码执行。
- **历史隔离**：`AUDIT_PROCESS.md`仍是SH-01～SH-06的低阶球谐专项理论整改记录，原97项全工程历史另有固定提交；本次新DAR不复用旧AF状态或承诺它们已消失。

## 2. 全36份审计对象（用户意见与当前实际审计状态分列）

| ID | docs/下相对路径 | 用户初始规范判断 | 首轮实际需核对的问题 | 审计状态 | 本轮证据／整改关联 |
|---|---|---|---|---|---|
| DOC-001 | `REQUIREMENTS.md` | 草案级部分符合 | AC→LLD章节身份/正式批准 | 已完成限定审计 | 本轮v0.15实改：§9.7新增66项REQ/AC/TC→14种主要责任文件与章节及完整Git blob快照，纠正FLOW-004、ERROR-001误归属；正式完整依赖/批准仍缺 |
| DOC-002 | `ARCHITECTURE.md` | 草案级部分符合 | 27拟议DAG与真实构建/ADR | 已完成限定审计 | HLD v0.10：设计图27箭头=25内部+2外部；根CMake仅低阶SH及3CTest，部署/DEC-006仍待 |
| DOC-003 | `OBJECT_MODEL.md` | 草案级部分符合 | 候选类/生命周期/签署 | 已完成限定审计 | 对象模型v0.7：所有权、候选公共类与已实现SH叶子区分；正式生命周期/ABI测试未做 |
| DOC-004 | `design/README.md` | 索引符合，非LLD | 实际Markdown个数与模块覆盖条目 | 已完成限定审计 | 14份真实目录Markdown=1索引+1跨模块LLD+9模块LLD+1专题LLD+2理论，10个覆盖行；提交3614b28 |
| DOC-005 | `design/contracts.md` | 草案级部分符合 | Result/ABI/ODR/异常线程/自包含 | 正在审计 | contracts v0.5未冻结；ABI/Result/线程/ODR列入后续深审 |
| DOC-006 | `design/cli.md` | 草案级部分符合 | CLI负例、RunId、JSONL、DEC-008 | 正在审计 | cli v0.6未冻结；固定输出根/失败码候选已识别，负例和DEC需证据 |
| DOC-007 | `design/mat_io.md` | 草案级部分符合 | Reader/Writer、TOCTOU、写回读、L2 | 正在审计 | mat_io v0.10；582只读探针与生产Writer/TOCTOU分层，细项待审 |
| DOC-008 | `design/sampling_noise.md` | 草案级部分符合 | 生产几何/RNG负例/CTest | 未审计 | — |
| DOC-009 | `design/trajectory.md` | 草案级部分符合 | Sophus真实依赖及几何L3 | 未审计 | — |
| DOC-010 | `design/kvd.md` | 草案级部分符合 | 36轮数学非真实CTest/200轮 | 未审计 | — |
| DOC-011 | `design/harmonics.md` | 草案级部分符合 | 低阶叶子有实测，公共ABI未冻结 | 正在审计 | harmonics v0.15；固定SH 3/3与公共ABI未建分层核对，逐式LLD待核 |
| DOC-012 | `design/adaptive_mfa.md` | 草案级部分符合 | Session原子提交、病态输入和真实软件 | 正在审计 | adaptive v0.5；源数学专题≠Session业务CTest，生命周期待核 |
| DOC-013 | `design/experiments.md` | 草案级部分符合 | Runner/Writer/Manifest/Ctest | 未审计 | — |
| DOC-014 | `design/analysis_plotting.md` | 草案级部分符合 | 分析L2/L3曲线、CTest/绘图隔离 | 未审计 | — |
| DOC-015 | `design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 专题草案级部分符合 | 旧版本CMake描述与最新低阶叶子验收区分 | 已完成限定审计 | 对照旧1/1说明与用户固定5e522198 3/3；仅低阶叶子，公共ABI仍未实现；提交4a62a9d |
| DOC-016 | `design/SH_DISCRETE_REFIT_CONSISTENCY.md` | 理论专题合格，非LLD | 不能替代模块LLD/测试/批准 | 未审计 | — |
| DOC-017 | `design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md` | 理论专题合格，非LLD | 不能替代Adaptive真实接口/实现 | 未审计 | — |
| DOC-018 | `TEST_PLAN.md` | 计划草案部分符合 | 66TC登记与真实运行证据区分 | 已完成限定审计 | 本轮v0.31实改：§3改引用SRS v0.15→§9.7且修正不存在的harmonics.md §4.1为§6；66TC静态一一对应、其它测试仍未运行 |
| DOC-019 | `PLAN.md` | WBS草案符合 | 98P、M0–M5、真实工期/性能 | 已完成限定审计 | PLAN v0.8：98个唯一P/WBS静态确认，SH-only实测与整工程M0–M5未验收分开 |
| DOC-020 | `PROGRESS.md` | 符合证据登记 | 固定SHA与历史执行/新状态分离 | 正在审计 | PROGRESS滚动历史；真实用户执行与数学证明按SHA分层，动态信息待审 |
| DOC-021 | `AUDIT.md` | 原专项→本轮综合审计 | 已在初始提交转换为全工程文档规范 | 已完成限定审计 | 新全工程文档审计规则、36对象、用户评价基线已提交1081f0f；原SH范围保留固定历史 |
| DOC-022 | `AUDIT_PROCESS.md` | 球谐专项整改，不适用全工程 | 保留SH-01～06，引用原专项快照避免误链 | 已完成限定审计 | 原SH-01～06理论台账保持不变，增加新审计与旧SH历史互不重计的说明；提交569a6e7 |
| DOC-023 | `DOCUMENT_BASELINE_REGISTER.md` | 部分符合 | 36项固定历史SHA与当期HEAD/批准分开 | 已完成限定审计 | 本轮实改：§2–3标题改为3e3ad2a历史固定快照；§6.18登记本轮SRS v0.15加14个主要候选设计文件的审查提交Git blob身份，非批准 |
| DOC-024 | `SOURCE_INVENTORY.md` | 静态来源追踪符合 | 45 .m/50函数、MAT未独立SHA | 正在审计 | SOURCE_INVENTORY原45.m/50函数为静态快照，未查全部字节与MAT SHA |
| DOC-025 | `MIGRATION_MATRIX.md` | 静态迁移追踪符合 | 45/50候选映射，不是代码实现 | 正在审计 | MIGRATION_MATRIX为45/50候选职责，未映射实际源码全量 |
| DOC-026 | `TODO.md` | 管理清单符合 | 历史审批与分阶段授权时间线 | 已完成限定审计 | TODO：人类待办/时间线与原SH局部授权、用户3/3已核；全工程文档审计导航已更正 |
| DOC-027 | `decisions/DEC-004_HISTORICAL_MAT_FIELDS.md` | 决策记录符合 | HistoricalOnly与当前save/代码未测试 | 已完成限定审计 | DEC-004：选定当前save/HistoricalOnly/缺失字段失败规则，尚无生产Runner/Analyzer测试 |
| DOC-028 | `decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md` | 所有者决定符合 | 技术方向≠软件运行/基线签署 | 已完成限定审计 | 2026-11-07所有者决定是历史时间点，MIT/Eigen/CLI等方向已选但不是当前完整软件授权 |
| DOC-029 | `decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md` | 专项授权记录符合 | 历史SH局部权限不得扩展 | 已完成限定审计 | 2026-11-09球谐专项授权仅l≤2/Eigen并禁sdla，H-11历史状态不影响后来用户自己实测 |
| DOC-030 | `approval/DECISION_IMPLEMENTATION_NOTES.md` | 审批落实记录符合 | 旧draft/权限不当最新批准 | 正在审计 | 历史审批落实H-01～16的draft/deferred/approved与空申请身份已抽查，逐字段仍待 |
| DOC-031 | `approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md` | 审批包符合，非规范 | applicationCommit/scopeIds空 | 正在审计 | 历史HAR申请包12章节/AI-DRAFT及applicationCommit/scopeIds空已核，尚需逐H事项审阅 |
| DOC-032 | `golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | 证据等级规划符合 | L2/L3/L4缺；球谐3/3状态需避免旧措辞 | 已完成限定审计 | L3总体AN-G仍候选，已有5e522198 SH归档/投影子集真实3/3；提交5e5a92e |
| DOC-033 | `golden/GOLDEN_TOLERANCE_PROPOSAL.md` | 容差候选符合 | 0.01/0.1筛查非DEC-005 | 未审计 | — |
| DOC-034 | `golden/NO_MATLAB_EXECUTION_POLICY.md` | 长期政策符合 | 取消L1，不取消软件验证 | 正在审计 | NO_MATLAB政策已核L1永久非必需；未审全部业务合同映射 |
| DOC-035 | `golden/SH_PAPER_VERIFICATION_CASES.md` | 验证候选符合 | SH-P候选未执行，区别原生已测案例 | 未审计 | — |
| DOC-036 | `golden/analytic_golden_candidates.v1.json` | JSON状态正确，非Markdown | DRAFT_UNAPPROVED_UNEXECUTED不是Golden批准 | 未审计 | — |

**历史初始与当前口径**：第0阶段原36份均尚未审计、6个初始DAR均未整改；第1阶段是6份已完成限定审计、14份正在、16份未审及8项DAR中的3项已整改。**第2阶段最新状态为36份中15份已完成限定审计、11份正在审计、10份未审；9个DAR中4已整改、5未整改。** 限定审计仅核对该行明确说明的定位/证据/追踪范围，不等于每个AC、API、算法、签署与软件测试完成。

## 3. 整改目标（六项用户输入基线及新增独立事实冲突）

| 目标 | 涉及文件 | 发现/整改方向（待实际验证） | 完整验收条件 | 阻塞性质 | 状态 |
|---|---|---|---|---|---|
| DAR-001 | REQUIREMENTS.md；DOCUMENT_BASELINE_REGISTER.md | SRS AC→LLD路径/章节/版本/blob/批准身份目前不是正式闭合链 | 将现有导航与冻结义务明确分离，取得正式各方批准及固定身份再可结案 | 需批准／冻结 | 未整改 |
| DAR-002 | ARCHITECTURE.md；PLAN.md | 27条设计DAG、部署/性能预算和ADR容易与真实构建混淆 | 逐项标出设计/实测/批准三态；真实target/性能及DEC-006后才能宣布完成 | 需软件证据／批准 | 未整改 |
| DAR-003 | design/contracts.md；design/cli.md；design/mat_io.md；design/adaptive_mfa.md；design/harmonics.md | 大量公共接口/ABI、Session与错误合同仍是候选签名 | 先核正文是否注明未实现，正式代码/CTest/ABI与批准俱全才结案 | 需授权测试／批准 | 未整改 |
| DAR-004 | TEST_PLAN.md；PROGRESS.md；golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md | 66 TC的候选覆盖与球谐3/3固定SHA的真实执行必须分层 | 表头/现行段显式区分历史/执行/批准；完整66TC要真实软件证据 | 需软件证据／批准 | 未整改 |
| DAR-005 | DOCUMENT_BASELINE_REGISTER.md；REQUIREMENTS.md | 36文件版本矩阵是较早固定快照而非当前HEAD基线 | 补真实当前HEAD规范身份/范围/批准人/日期/例外后才允许签正式冻结 | 需批准／当前SHA | 未整改 |
| DAR-006 | AUDIT.md；AUDIT_PROCESS.md；design/README.md；DOCUMENT_BASELINE_REGISTER.md | 此前AUDIT.md属于球谐专项，与本轮全工程文档审计及目录新增专题文件的分类/计数不一致 | 建立全工程审计规则和独立进度，固定球谐历史来源，核对design目录实际分类/数量并更新相关文字 | 纯文档 | 已整改 |

| DAR-007 | design/README.md；design/SH_ROTATION_IVANIC_RUEDENBERG.md | 同一个现行低阶球谐模块在某处写“3/3原生已测”，另一处仍写“仅旧1/1且新增2项未验收”；专题LLD未列投影源和两个新CTest | 按固定5e522198区分历史1/1与3/3、受限叶子与未建公共ABI，不改变真实授权/批准 | 纯文档 | 已整改 |
| DAR-008 | golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md | L3概述笼统称AN-G01～12尚无真实C++测试，但AN-G06/07/08子集已有固定SHA用户原生CTest | 明确整份AN-G JSON未批准与固定SH子集已实测同时成立，不误关其它AN-G及AC | 纯文档 | 已整改 |
| DAR-009 | ARCHITECTURE.md；OBJECT_MODEL.md；PLAN.md；DOCUMENT_BASELINE_REGISTER.md；TEST_PLAN.md；TODO.md | 当前资料与历史表的球谐论文材料、全部代码未创建、1/1专项CTest及全工程审计导航混写 | 区分历史批准与已取得论文/受限SH叶子3/3真实构建，纠正当前HLD/对象/计划/基线/测试/待办文字，不假称其它软件已通过 | 纯文档 | 已整改 |
## 4. 已实施审计、具体整改和验证日志

本轮先固定两份**意见基准文件**：AUDIT.md 文档规范提交 `1081f0f9c2de`，AUDIT_PROGRESS.md 初始36文件/6DAR提交 `140ea0e36000`；随后实际读取上表范围内的源Markdown、目录14项清单、固定5e522198球谐用户测试证据及历史/现行台账，做了如下**纯文档整改**：

| 目标 | 审计发现与定位（起点cb3129b） | 已完成修订及提交 | 仍未宣称的内容 |
|---|---|---|---|
| DAR-006 | `AUDIT.md` 原是SH限定审计；`AUDIT_PROCESS.md`链接旧AUDIT；`design/README.md`第20/37行把所有目录文件当LLD并沿用“11份”说法；`DOCUMENT_BASELINE_REGISTER.md`第7/53/104/115/150行将旧AUDIT/AUDIT_PROCESS当动态全工程台账 | 新规范及进度 `1081f0f`/`140ea0e`；旧SH台账交叉引用 `569a6e7`；目录14份分类 `3614b28`；历史/动态基线导航修订 `5358c3c` | 原SH理论结案仍仅属SH-01～06；旧全工程97AF与本次36份文档评审是不同对象；未批准任何SRS/HLD/LLD |
| DAR-007 | `design/README.md` 第32行同时有“3/3已实测”和“新增2项未验收”；`SH_ROTATION_IVANIC_RUEDENBERG.md`第5、158–159行只写旧低阶库3源/1CTest及“Eigen未实施” | 目录索引 `3614b28`，专题LLD `4a62a9d`；均区分受限SH叶子原生3/3、SH-PAPER候选未执行和公共ABI未实现 | 无新增编译、无正式DEC-005/公共接口批准，原先仅SH的3/3不覆盖全66TC |
| DAR-008 | `golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` 第11行笼统“尚无真实C++测试”，第43行又记录SH 3/3 PASS | 依 `5e522198` 的用户日志将第11行明确拆为整套AN-G候选未批准、AN-G06/07/08归档子集176次已执行，以及二阶投影76次已执行；提交 `5e5a92e` | 其它AN-G、完整独立L3、L2 MAT、L4及正式AC仍未通过 |

**抽样/审计限制**：其余30份文档的用户评价已录入，但没有对每个AC/字段/算法逐字完成深审；其中14份已进行定位/状态/证据抽查，仍列“正在审计”，16份未完成独立核查。不得将文档初审与软件合格混淆。此次修改仅为MD文档，不改C++/CMake/CTest/原MAT/MAT。

### 第2阶段：核心追踪与过时状态核对

| 审计对象/整改 | 实际读到的可复核事实 | 本轮处理与边界 |
|---|---|---|
| DAR-001 / SRS | `REQUIREMENTS.md` 66个唯一REQ表行与 `TEST_PLAN.md §3` 66个唯一TC表行一一对应，AC编号无重复；示例AC-F-FILTER-001/SH-001仍是可变LLD导航 | 静态编号映射已核，但**尚无逐AC固定LLD blob/批准**，保持未整改 |
| DAR-002 / HLD | §4.1 Mermaid图27个箭头，**25个内部模块边＋2个外部库边**；当前根CMake只有`egomotion_harmonics`叶子及3项CTest | 纠正旧论文上传门禁与历史/实际构建（`15d2830`），更正27/25+2口径（`5298e7d`）；完整业务target/DEC-006仍缺，保持未整改 |
| DAR-003 / 对象模型 | 45源职责/Session候选公开声明与已测`low_order`叶子不是同一公共类ABI | `OBJECT_MODEL.md` 修正声明（`c02f02b`）；跨模块ABI及真实CTest未完成 |
| DAR-004 / 测试 | §3恰有66个唯一TC及对应REQ/AC；用户固定`5e522198`SH3项CTest 3/3（53,532/176/76内部断言） | `TEST_PLAN.md` 增加当前事实和旧NOT_RUN时点边界（`9fff57a`）；其它大多数业务TC未实际执行 |
| DAR-005 / 配置 | `DOCUMENT_BASELINE_REGISTER.md §2–3`是早期固定36项；本表§6实际从基准`9af189f`单一Git tree获取本轮36个blob且无缺失 | 修正历史状态/审计导航（`9ea9b56`），本轮起点blob快照提交`d5ab2de`；**并非正式冻结/当前最新批准SHA** |
| DAR-009 / 当前实施事实 | HLD等旧文仍称论文待上传、全部球谐源码/CTest未实现，PLAN/基线与TODO会误导到旧审计入口 | 修订`ARCHITECTURE`、`OBJECT_MODEL`、`PLAN`、`DOCUMENT_BASELINE_REGISTER`、`TEST_PLAN`、`TODO`（提交`15d2830`、`c02f02b`、`516ea95`、`9ea9b56`、`9fff57a`、`683d082`），**仅文字状态冲突结案**，不宣布其它模块实现/批准 |

**限定文件审阅说明**：DOC-001/002/003/018/019的“已完成”仅限本表写明的编号、依赖、身份、对象边界、WBS静态检查；DOC-026～029仅限短篇管理/决策文档的职责和时间线。DOC-030/031虽已浏览历史审批范围与状态，仍未完成各H项逐字段校核，继续“正在审计”。未实际审查的文件维持未审计。不存在本轮原生构建/CTest或新批准证据。

### 第3阶段：直接整改设计正文与跨文档引用（DAR-001/004/005的限定子义务）

本批**没有新建待审文件或新DAR**。整改前后对比以固定Git身份和提交记录为依据，不以审计完成数的改变代替实际修订。

| 真实问题 | 已修改的规范正文 | 本批提交与验证结果 | 没有虚报的剩余验收 |
|---|---|---|---|
| SRS缺66个AC的精确设计责任索引（DAR-001） | `REQUIREMENTS.md` **v0.15 §9.7**：逐项从58正文＋8附录REQ生成66条REQ/AC/TC映射，列主要负责HLD/LLD文件、实际章节与固定Git blob；其中14种主要责任文件确实存在，列出的章节在固定提交内可定位 | `03e5a2d`，随后责任更正`3d40bad`、版本记录`9820c5d`。远端独立读回66个唯一映射、对应TEST_PLAN66TC无漏号、全部66个候选文件blob匹配**同一固定审查提交**`acb6753` | 映射是主要责任章节而非每条AC全依赖包；正式SRS/HLD/LLD版本、批准人/日期、DEC与真实软件证据仍缺，DAR-001整体不关单 |
| SRS两条AC设计责任归属不准确（DAR-001） | `AC-F-FLOW-004` 的球内射线交点/近度主合同由采样改指`trajectory.md §3/§6`；`AC-F-ERROR-001` 的误差生成主合同从分析消费层改指`experiments.md §4/§5`并区分分析模块仅消费 | `3d40bad`，相应目标章节存在、Git blob与源提交对应 | 尚未据此证明所有66条AC语义/多模块关联均已完整冻结 |
| 测试计划引用错误及版本漂移（DAR-004） | `TEST_PLAN.md` **v0.31 §3** 将旧的“SRS v0.14”改为v0.15；指向不存在的`harmonics.md §4.1`改为实际存在的`§6`；接入SRS §9.7候选身份表 | `7bb6fc2`；66个TC与REQ、AC在编号层静态一一对应，不能把固定5e522198的3/3球谐CTest扩大为66个TC通过 | 大多数业务CTest未创建/未执行；DEC-005/006/008与正式测试证据仍缺，DAR-004整体不关单 |
| 旧配置快照标题误导及AC审查身份未归档（DAR-005） | `DOCUMENT_BASELINE_REGISTER.md §2–3`标明是固定**3e3ad2a旧快照**而非当前HEAD；§6.18按固定`9820c5d`的同一完整Git tree列出v0.15 SRS+14个候选主设计文档的精确Git blob | 新审阅身份`cb41072`，历史标题更正`6f75866`；15个文件来自同一未截断的固定Git tree，旧36项身份仍原样保留 | 这仍是**审查身份**而不是正式ApprovedBaseline；批准人/时区、最终commit、范围、签署证据未提供，DAR-005整体不关单 |

**本阶段已验证的十项结构断言**：SRS新表66个唯一REQ且对应66个AC/TC、66个源blob匹配固定Git tree、TEST_PLAN66个唯一TC和全量对应、基线§6.18确有15文件、FLOW-004及ERROR-001责任纠正、不存在的harmonics §4.1引用已删、SRS/TEST_PLAN版本更新、配置矩阵§2旧快照身份明确，全部通过。仅审查结构与来源，**不是新编译、数值计算或正式批准**。

## 5. 各轮计数与变化

**第3阶段相对上一轮**：审计文件总数36→36、已完成15→15、正在11→11、未审10→10（均0变化）；整改目标9→9、已整改4→4、未整改5→5（均0变化）。**目标数未变并不等于没整改：本轮已实际修复REQUIREMENTS/TEST_PLAN/DOCUMENT_BASELINE_REGISTER三份正文并取得固定源码复核结果**。由于五个复合DAR的最终条件涉及真实软件/正式签署，本轮不能造假关闭。

**第2阶段相对上一轮**：36份不变；完成限定审计6→15（+9），正在审计14→11（−3），未审计16→10（−6）；整改目标8→9（+1），已整改3→4（+1），未整改5→5（0）。五项复合阻塞未解除，不能以文本修复宣布正式基线完成。


**第1阶段（历史快照）**：审计对象36→36（0），完成限定审计0→6（+6），正在审计0→14（+14），未审计36→16（−20）；整改目标6→8（+2个实际独立文字冲突），已整改0→3（+3），未整改6→5（−1）。新登记的DAR-007、DAR-008独立于原六类综合阻塞，且均以具体文本矛盾与提交为依据；其余五个复合义务不能因上述局部措辞更正关闭。

| 阶段 | 文件总数 | 已完成限定审计 | 正在审计 | 未审计 | 整改目标 | 已整改 | 未整改 |
|---|---:|---:|---:|---:|---:|---:|---:|
| 第0阶段：用户意见录入、实际审计前 | 36 | 0 | 0 | 36 | 6 | 0 | 6 |
| 第1阶段：源文档限定审查与三项纯文档整改 | 36 | 6 | 14 | 16 | 8 | 3 | 5 |
| 第2阶段：追踪与历史状态核对 | 36 | 15 | 11 | 10 | 9 | 4 | 5 |
| 第3阶段：66项责任链与实际正文修复 | 36 | 15 | 11 | 10 | 9 | 4 | 5 |

## 6. 36份文件的同一基准提交身份（DAR-005：只读静态快照）

本表已从 GitHub 固定提交 [`9af189f97071`](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/9af189f97071692f4d96c9a431144485e3b76aa3) 的完整Git tree实际核对全部36份本轮评审文档，并记录对应的**Git blob对象SHA**；树内容未截断。它**不是**MAT数据的独立SHA-256，也不是每份规范的最新HEAD批准身份。已有文档在这一基准之后被修改，修订提交及其新blob与该基准旧blob应分开记录，不能把下面任一旧SHA用作正式冻结依据。

| DOC ID | 基准时docs/下相对路径 | 固定基准Git blob SHA | 证据性质 |
|---|---|---|---|
| DOC-001 | `REQUIREMENTS.md` | `51c0cfed22a80340eaadce059e5f579e9c56f3ef` | 仅文件身份；不是已批准基线 |
| DOC-002 | `ARCHITECTURE.md` | `549a74093bc128fb948e3e98a98f008e42ec585d` | 仅文件身份；不是已批准基线 |
| DOC-003 | `OBJECT_MODEL.md` | `9bc35d7d8b66c6337b85a9e37e3de530e0278176` | 仅文件身份；不是已批准基线 |
| DOC-004 | `design/README.md` | `18cbb3c153bf6c1eddae65e225deb2d8cf0b2186` | 仅文件身份；不是已批准基线 |
| DOC-005 | `design/contracts.md` | `f7369e836820603fdb9237bf685f12c7f5daaed4` | 仅文件身份；不是已批准基线 |
| DOC-006 | `design/cli.md` | `82c38559d48fb1ad6cc6a76c8c1c6533b6a4977d` | 仅文件身份；不是已批准基线 |
| DOC-007 | `design/mat_io.md` | `b8742f42a4e28578ad9cd01bfbbae1637ad44035` | 仅文件身份；不是已批准基线 |
| DOC-008 | `design/sampling_noise.md` | `e47fd2295afe1f3889ce07fa68ab3adda8d05a27` | 仅文件身份；不是已批准基线 |
| DOC-009 | `design/trajectory.md` | `b749ba78545d561c664acc079e54cbc1beb6083b` | 仅文件身份；不是已批准基线 |
| DOC-010 | `design/kvd.md` | `ca48de30582168275f470db61e22834f321d9869` | 仅文件身份；不是已批准基线 |
| DOC-011 | `design/harmonics.md` | `348f664d91932f1493f5ef9cec8bc407ef4648ab` | 仅文件身份；不是已批准基线 |
| DOC-012 | `design/adaptive_mfa.md` | `97c087e9b42e4e23c7f046f4dbf0632fb16ae063` | 仅文件身份；不是已批准基线 |
| DOC-013 | `design/experiments.md` | `224b2707be6b0b0df885f727498c70dfe7eb6fb5` | 仅文件身份；不是已批准基线 |
| DOC-014 | `design/analysis_plotting.md` | `e9bc9250730609393734ad7235a57c08154aed65` | 仅文件身份；不是已批准基线 |
| DOC-015 | `design/SH_ROTATION_IVANIC_RUEDENBERG.md` | `1f428091894e2899aa9225c7c9f99fe2cf5655da` | 仅文件身份；不是已批准基线 |
| DOC-016 | `design/SH_DISCRETE_REFIT_CONSISTENCY.md` | `caba819f7fcabebdba84dfcefd0fc312e7215b00` | 仅文件身份；不是已批准基线 |
| DOC-017 | `design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md` | `a952451caa058e321fc34350de54b50822a83b65` | 仅文件身份；不是已批准基线 |
| DOC-018 | `TEST_PLAN.md` | `cab2bf5db049619ba14cd29f8b499b79cd178c60` | 仅文件身份；不是已批准基线 |
| DOC-019 | `PLAN.md` | `74bfba0b94c4a0cb615a6cea1da1f03b567322eb` | 仅文件身份；不是已批准基线 |
| DOC-020 | `PROGRESS.md` | `46bfabf0d4a8c19f9a65f7585967d93700e05dac` | 仅文件身份；不是已批准基线 |
| DOC-021 | `AUDIT.md` | `c36cb0a0b8d4f96222687e41c340796527070a13` | 仅文件身份；不是已批准基线 |
| DOC-022 | `AUDIT_PROCESS.md` | `5ec8a185d662275c0b8e21f5e5f4288b6236f24f` | 仅文件身份；不是已批准基线 |
| DOC-023 | `DOCUMENT_BASELINE_REGISTER.md` | `2fbf852ae7799f2ddebd352a10177f0a5c79b5f0` | 仅文件身份；不是已批准基线 |
| DOC-024 | `SOURCE_INVENTORY.md` | `9d72f7517e93f774c4b67338cc6b93f6723e7712` | 仅文件身份；不是已批准基线 |
| DOC-025 | `MIGRATION_MATRIX.md` | `3d61d69f7962a29c8720528f7fa71628c7008331` | 仅文件身份；不是已批准基线 |
| DOC-026 | `TODO.md` | `d3a41435ecb9d79997e97e0063decf8acc96bb74` | 仅文件身份；不是已批准基线 |
| DOC-027 | `decisions/DEC-004_HISTORICAL_MAT_FIELDS.md` | `063acaa2dc8e7524546fb85a985a7f8b8e860a88` | 仅文件身份；不是已批准基线 |
| DOC-028 | `decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md` | `72a7ad3cf72d32603651d5114a66ea0fba3f8552` | 仅文件身份；不是已批准基线 |
| DOC-029 | `decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md` | `0c708af126d031707e737136c075d29483667877` | 仅文件身份；不是已批准基线 |
| DOC-030 | `approval/DECISION_IMPLEMENTATION_NOTES.md` | `2dc89dfa563b35c81c747f89c2e0012ddc2ada20` | 仅文件身份；不是已批准基线 |
| DOC-031 | `approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md` | `8da7c5d30cea2bdeb411d6e817721d53a09302bb` | 仅文件身份；不是已批准基线 |
| DOC-032 | `golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | `df9612759549a9e9820b33a772540a80bbc90c3e` | 仅文件身份；不是已批准基线 |
| DOC-033 | `golden/GOLDEN_TOLERANCE_PROPOSAL.md` | `fe1ad898090eb2d66ca99149dfeac189cf7c5696` | 仅文件身份；不是已批准基线 |
| DOC-034 | `golden/NO_MATLAB_EXECUTION_POLICY.md` | `5f2bc13f3fbf0636ac15f5dfe0c38611b80074be` | 仅文件身份；不是已批准基线 |
| DOC-035 | `golden/SH_PAPER_VERIFICATION_CASES.md` | `1ac44cba46e0d63b9f54f40fe18dbbc4279a627a` | 仅文件身份；不是已批准基线 |
| DOC-036 | `golden/analytic_golden_candidates.v1.json` | `1e6499eff3ce5b55503f8cd913b9f492629da499` | 仅文件身份；不是已批准基线 |

**当前状态仍以上方DOC三态为准。** 确认全部36份文件在同一提交存在，并不意味着36份都已完成内容审计，更不意味着批准人/范围/时区/例外/真实测试俱全。正式基线应在签署时重新固定最新commit+blob并记录完整批准证据。

## 7. 结案纪律

针对文档措辞/数量错误可以完成纯文档整改；但AC冻结、正式接口ABI、真实C++/CTest/写回读、独立MAT SHA-256、DEC-005/006/008及所有者签字，不得通过编辑Markdown宣布完成。严格保持MATLAB/Octave/L1**永久非必需**。代码/原MAT修改、编译、运行及发布不属于本轮用户指示。
