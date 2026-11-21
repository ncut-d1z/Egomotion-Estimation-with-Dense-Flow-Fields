# 全工程文档规范审计（AUDIT）

> **状态：文档规范审计规则 v1.2，已增加分类型草案/正式基线双轴审计与LLD实际可得性规则，仍非正式项目基线。** 基准分支 `dev-plan-P0-P1-mat-dictionary-probe`，启动提交 [`cb3129b1510f`](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/cb3129b1510f4ffc5fadeb94b8fd67bdff0d5e82)。配套动态审计与整改台账：[AUDIT_PROGRESS.md](AUDIT_PROGRESS.md)。本文件是**规范和输入意见基线**，不是声称下列全部文档已经逐项通过独立审计。

## 一、评价基准与总体意见

按**通用文档工程及本项目自身声明的职责**审查，**不宣称符合任何外部标准的逐条条款**。逐项核对七个维度：

1. **定位与范围**：文档性质（SRS/HLD/LLD/索引/管理/证据/专题）、适用目标、排除范围、使用者是否明确。
2. **术语与假设**：名称、版本、输入依赖、环境约束及原作者源码事实是否有清楚来源。
3. **需求可验证性与追踪**：REQ/AC→HLD组件→LLD章节/接口→TC/实体→证据的唯一引用是否可查；未冻结链接不能称已固定身份。
4. **架构责任与依赖**：组件职责、27条设计DAG、数据流/状态流和部署候选与真实构建事实分列。
5. **详细设计完整性**：函数签名、数据类型/shape/单位、所有权/生命周期、算法及边界、错误与状态、测试映射和禁止隐式行为。
6. **状态与配置管理**：文件版本、固定Git blob/commit、批准人/日期/范围/例外、历史快照与当前HEAD分开；批准与执行不能互相替代。
7. **证据真实性**：数学候选、静态复核、原生C++/CTest、L2/L3/L4、验收/发布批准分别记录；禁止把候选或NOT_RUN写成已测试/已批准。

**用户输入的总体初始评价（不是本轮全部已验证的结论）**：核心SRS/HLD/LLD在“评审草案”层面总体结构规范、状态声明诚实、单一来源与追踪意识较强；但没有一份核心规范达到“已冻结、已批准、已验收”的正式基线要求。大量接口仍为候选，多数TC未创建/运行；DEC-005/006的最终数值/性能批准无限期延期，DEC-008-01～06已有设计决定但其公共实现、原生测试和正式冻结仍未验证；CMake/CTest/数值证据不完整。支撑文档大多符合自身定位，但专项审计/理论专题/Golden候选不能代替全工程审计或正式LLD。

**状态辞典**：`符合其用途（草案）`只表示文档定位、结构及当前说明经审查；`部分符合（草案）`表示仍有重要合同、追踪或证据缺口；`不适用LLD`表示不按模块详细设计要求考核。两者都不代表正式批准。独立维度`未开始／正在审计／已完成限定审计`与整改`已整改／未整改／无限期延期`由进度台账逐条记录；**延期不计入未整改且不是已整改**。**没有依据时写“待验证”，不得将缺证据推断为代码缺陷。**

## 二、核心规范类初始评审基线（17份）

下表忠实收录本次用户给出的评价方向，作为逐文件审计输入。**不以这张意见表代替打开文件、引用位置、核对状态、生成整改证据。**

| 文档（docs/下相对路径） | 类型 | 初始判断 | 用户指出的主要缺口／审计重点 |
|---|---|---|---|
| `REQUIREMENTS.md` | SRS | 草案级部分符合 | AC→LLD精确版本/blob/批准、DEC-005/006/008、签署 |
| `ARCHITECTURE.md` | HLD | 草案级部分符合 | 27条拟议DAG与真实CMake区分、部署/ADR/质量目标未批准 |
| `OBJECT_MODEL.md` | 对象模型/HLD附件 | 草案级部分符合 | 候选类/生命周期需冻结及批准，不能称真实C++实现 |
| `design/README.md` | LLD索引 | 索引职责符合，非LLD | 实际文件/覆盖条目数量与专题分类保持同步 |
| `design/contracts.md` | 跨模块LLD | 草案级部分符合 | 公共Result/ABI/ODR/异常/线程与头文件自包含尚未实测 |
| `design/cli.md` | CLI LLD | 草案级部分符合 | 参数负例、JSONL转义、RunId目录竞争与DEC-008未验收 |
| `design/mat_io.md` | MAT I/O LLD | 草案级部分符合 | Reader/Writer、RAII/TOCTOU、写回读与L2逐值/SHA-256未验收 |
| `design/sampling_noise.md` | 采样/噪声 LLD | 草案级部分符合 | 生产采样、RNG耗尽/非有限、CTest与DEC-005尚缺 |
| `design/trajectory.md` | 轨迹 LLD | 草案级部分符合 | Sophus真实版本/链接、几何边界L3与CTest尚缺 |
| `design/kvd.md` | Franz/KvD LLD | 草案级部分符合 | 数学trace不代替真实算法CTest、一般/病态/200轮与DEC-005 |
| `design/harmonics.md` | 球谐 LLD | 草案级部分符合 | 低阶叶子局部已实测，公共ABI/完整集成及DEC-005尚缺 |
| `design/adaptive_mfa.md` | Adaptive LLD | 草案级部分符合 | Session原子提交、非对称/病态真实测试与正式容差未验收 |
| `design/experiments.md` | 实验Runner LLD | 草案级部分符合 | Runner/Writer、Manifest、失败继续与真实CTest尚缺 |
| `design/analysis_plotting.md` | 分析/绘图 LLD | 草案级部分符合 | L2逐值/L3曲线、Analyzer/Exporter/CTest尚缺；可选绘图不阻塞 |
| `design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 球谐专题LLD | 专题LLD草案级部分符合 | 低阶叶子已构建≠拟议公共类已创建；ABI及集成未冻结 |
| `design/SH_DISCRETE_REFIT_CONSISTENCY.md` | 数学理论专题 | 专题补充合格，非LLD | 不能替代harmonics/Adaptive接口、生命周期或软件验收 |
| `design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md` | 数学理论专题 | 专题补充合格，非LLD | 仅数学证明，不能替代Adaptive正式LLD/生产测试 |

## 三、支撑、过程、证据与附件类初始评审基线（19份）

| 文档（docs/下相对路径） | 类型 | 初始判断 | 用户指出的主要缺口／审计重点 |
|---|---|---|---|
| `TEST_PLAN.md` | 测试计划 | 测试计划草案级部分符合 | 66 TC候选追踪≠真实完成；归档与已实测固定SHA分开 |
| `PLAN.md` | WBS | WBS草案符合 | 98 P与依赖合理；M0–M5未验收，预算/工期未批准 |
| `PROGRESS.md` | 证据登记 | 符合其自身用途 | 固定SHA/执行与理论区别需持续维护，不替代验收 |
| `AUDIT.md` | 本全工程文档规范审计 | 需重新建立全工程口径 | 原文件是低阶球谐专项，须保留历史链接并转换为文档审计 |
| `AUDIT_PROCESS.md` | 低阶球谐专项整改历史 | 专项记录符合，非全工程整改 | 现有SH-01～06限定理论结论，不作为本轮全工程审计进度 |
| `DOCUMENT_BASELINE_REGISTER.md` | 版本/配置管理 | 部分符合 | §2–3为固定旧快照，当前HEAD及正式批准身份未登记 |
| `SOURCE_INVENTORY.md` | 静态来源清单 | 符合静态追踪 | 45个.m/50函数、582 MAT及Git blob非软件验收/独立MAT SHA-256 |
| `MIGRATION_MATRIX.md` | 迁移映射 | 符合静态追踪 | 45/45与50/50为候选责任映射，非实现/CTest |
| `TODO.md` | 人类待办 | 符合管理清单 | 按时间线区别历史审批、球谐局部授权与其它模块权限 |
| `decisions/DEC-004_HISTORICAL_MAT_FIELDS.md` | 决策记录 | 符合决策记录 | 已选规则未实施测试，HistoricalOnly与当前save仍需验证 |
| `decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md` | 所有者决策 | 符合决策记录 | 已选技术方向≠代码或批准基线 |
| `decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md` | 授权记录 | 符合历史授权记录 | 仅球谐局部许可，后续实测与当时授权按时间线解释 |
| `approval/DECISION_IMPLEMENTATION_NOTES.md` | 审批落实记录 | 符合历史落实记录 | H-01～16的历史draft/权限不可当最新完整授权 |
| `approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md` | 审批材料包 | 符合材料包，非规范 | applicationCommit/scopeIds未签署；不等于执行许可 |
| `golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | Golden证据等级 | 符合规划，非批准 | L2逐值/SHA、L3广度、L4统计与DEC仍缺 |
| `golden/GOLDEN_TOLERANCE_PROPOSAL.md` | 容差候选 | 符合候选，非批准 | 0.01/0.1仅探索筛查，DEC-005未签 |
| `golden/NO_MATLAB_EXECUTION_POLICY.md` | 长期政策 | 符合政策记录 | L1永久非必需，仍须L2/L3/L4与真实授权 |
| `golden/SH_PAPER_VERIFICATION_CASES.md` | 球谐验证候选 | 符合候选，非已运行L3 | SH-P01～10属候选，勿混淆低阶叶子已实测3/3 |
| `golden/analytic_golden_candidates.v1.json` | 机器参考候选 | 状态标注合格，非Markdown | DRAFT_UNAPPROVED_UNEXECUTED，不得冒充已批准Golden |

## 四、正式整改与冻结的六项跨文档原则

- **DOC-P1 核心文件不得虚称冻结。** SRS/HLD/LLD都是评审草案；正式身份须填文件路径、版本、blob SHA、commit SHA、批准人、日期/时区、范围、限制与真实证据。现有历史矩阵仅作快照。
- **DOC-P2 AC→LLD引用须能追溯。** 对 `AC-F-FILTER-001` → `kvd.md`、`AC-F-SH-001` → `harmonics.md` 等，在冻结前登记准确章节与固定版本/blob/commit、批准身份；当前导航链接不构成正式批准。
- **DOC-P3 候选签名不等于可编码/已实现ABI。** 公共Result/Status、CLI退出码、MAT Reader/Writer、Adaptive Session、球谐公共接口需区分设计合同、头文件自包含/链接/ABI/异常/线程及真实CTest。
- **DOC-P4 66 TC追踪不等于66项执行。** 明确哪一固定源码SHA的哪些CTest真的执行；局部球谐历史3/3不能代表其余TC。
- **DOC-P5 历史快照不冒充当前HEAD。** `DOCUMENT_BASELINE_REGISTER` §2–3 的36项旧SHA、旧审批表、历史reviews与滚动PROGRESS分别标识时间及来源。
- **DOC-P6 专题与候选不能替代正式规范。** 原低阶球谐专项[历史AUDIT.md](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/cb3129b1510f4ffc5fadeb94b8fd67bdff0d5e82/docs/AUDIT.md)及现存`AUDIT_PROCESS.md`的SH-01～06只对三路线理论结案负责；本轮新AUDIT是**全工程文档规范审计**。数学专题不是模块LLD；SH-P/AN-G候选不是批准Golden。

长期政策：**永久取消强制运行MATLAB/Octave和L1 Golden**；不能因此删减原`.m`只读审核、L2/L3/L4、真实软件测试、安全/权限与项目所有者批准要求。

## 五、审计与整改操作规则

1. 先将第二区和第三区的**36份文档**逐文件登记到 `AUDIT_PROGRESS.md`，明确初始用户意见与实际检查结果，记录文件当前blob、被查位置、来源、问题ID。新增/删改文档必须更新分母，禁止只增长“审计文件个数”而没有整改目标变化。
2. 文件审查先检查标题/首段定位、章节目录、真实源事实、REQ/AC/TC/设计边/证据引用；再依据文档职能采样核对接口、矩阵、角色、失败、数据、计划和版本。**审计覆盖范围要具体**，不能以读取标题就声称全文审计或某AC通过。
3. 问题登记表每个目标有唯一ID、相对路径和源码定位、整改前描述、明确验收条件、责任类型（纯文档／需软件测试／需所有者批准）、状态、提交SHA。一个复合义务不拆成虚假的完成计数；确有独立实质缺陷才新增目标。
4. **先修能安全证实的纯文档问题**（错误动态计数、过时证据文字、错误链接/身份、类型混淆）。涉及正式批准、真实构建/CTest、源MAT/MAT数据或生产代码的目标**不得以文档措辞修复宣称闭环**；必须保留阻塞并说明所需证据。
5. 整改先固定旧文件blob→比对来源→修改→只核本次影响范围→GitHub推送→复读最终HEAD→更新 `AUDIT_PROGRESS.md` 前后状态。若文档或基线有并发更新，重新读取并合并，不能盲目覆盖。
6. 每轮回复输出：审计文件总数、已审计、正在审计、未审计；整改目标总数、已整改、未整改、批准／执行阻塞；**开始／结束／净变化**，实际提交与未做事项。正式审批字段只由用户或批准人提供，禁止捏造。
7. 本次审计只授权**文档审阅与真实文本整改**；无新增C++/CMake/CTest/MATLAB运行/MAT写入、无合并或发布。各模块其它既有授权不得推定为本轮写码许可。

### 5.1 2026-11-12 用户新决定：先完成其他整改、无限期推迟六项人类门禁

**优先级高于本文件此前“批准/测试尚缺”的建议处理顺序**：H-10、H-11、DEC-005、DEC-006、BASELINE-SIGN、RELEASE-PR已由用户明确设为**无限期延期**，在其余可独立完成的审计整改结束前不主动催办、不设置截止日期、不将其作为停止现行文档审计的理由。它们仍属需要真实代码/原生运行或所有者批准的历史综合义务，**不得因为延期直接标为已整改/已批准**；无新授权仍严禁实施未获准的C++、CMake/CTest、MAT写入及合并/发布。具体唯一当前状态与以后可能的恢复方式见[TODO §0](TODO.md)。

**DEC-008不在延期清单**：截至2026-11-13用户已对01～06全部给出具体设计决定：01不承诺跨版本稳定ABI；02选择显式检查且抛固定明确消息的`std::logic_error`方案B，非抛出`try*`仅必要时扩展；03采用工厂/禁默认构造/尽量不可变方案；04选择完整`std::forward_list<ErrorFrame>`单向包装链（最近内层→最深根因）；05**只要求检查MAT数据结构，助手之前附加的数值打印、NumPy/pandas预览、全量输出相关设计及决策已撤销**；06要求RunContext/Manifest同样保留完整包装链。DEC-008状态现为`DesignDecided / ImplementationNotVerified`，细节以[TODO §2.2](TODO.md)为准。所有六项无限期延期事项继续保持不授权状态，设计已定不等于真实公共C++/CTest/冻结已完成。当前文档可继续真正修订，历史已撤回的数值打印需求只在审计日志中作为纠错记录保存，不得重新列入REQ/TC。

### 5.2 2026-11-15 从头复核与无限期延期单列的新口径

**此节覆盖此前把延期目标算作“未整改”的历史统计口径；不追溯改写旧审计快照、审批文书或 SH-01～06 专项事实**。本轮全仓`docs/`目录有64个实际文件：其中**36份是本AUDIT第二区/第三区明列的正式审计对象**，余28份是`AUDIT_PROGRESS.md`自身、`MAT_DATA_SCHEMA.md`、历史reviews、审批HTML等过程/证据附件。64个文件均须核对结构/定位/来源状态/相对链接及已关闭DAR的回退锚点；但是正式“审计文件总数”仍指36份，**不将历史证据附件机械并入、虚增审计分母**。

- 文件状态`已完成限定审计`：本次明确的**文档内容与已完成整改回退核查范围**已经逐文件复核，已确认仍缺的数学全域、源码实现、实际L2/L3/CTest或签署在证据边界另行声明；**不承诺全域数学定理重新证明、更不代表软件验收**。在文档此范围已经核对完成时，不能仅因为 H-10/H-11/DEC-005/006 等被无限期延期，就仍把该文档算成“正在审计/未审计”。
- 整改目标使用**三态且互斥**：`已整改`（实质修订和对应可核查源码/文档证据）、`未整改`（当前获授权、尚有实际可处理的文档缺陷）、`无限期延期`（历史目标保留，最终条件依赖已明确无限期推迟的人类许可、运行/性能/容差或正式签署）。**无限期延期不计入“未整改”，也不等于“已整改”**；单列 `deferred`。活跃整改目标数 = 总目标数 − 无限期延期数；不虚构代码/审批证据。
- 本轮明确延期的六人类门禁（H-10/H-11/DEC-005/DEC-006/BASELINE-SIGN/RELEASE-PR）不再占用活跃未整改计数，对应DAR-001～005按**综合义务整体**标`无限期延期`，未获批准状态原样保留，未来**只有项目所有者明确恢复**才能转回待办，不能自动恢复/设截止日。
- 历史进度表在形成时采用过旧二态，原数值仍保留为“当时记录”，现行统计只读取[审计进度第24阶段](AUDIT_PROGRESS.md)的新三态和本节规则；不可把旧表5项未整改叠加到当前口径。

## 六、审计结案与正式基线的区别

**单份文档审计结案**要求定位明确、核对条目/问题记录完成、剩余缺口公开、修订经固定SHA复核。**正式规范冻结**另外要求最新版本、完整追踪链、真实测试与批准，不能因为审计表变为“已审计”而自动发生。全项目正式基线建议顺序：SRS外部义务及AC → HLD组件/依赖/质量属性 → LLD精确签名/状态 → TEST_PLAN/PLAN → PROGRESS真实执行 → 所有者签署。

本次审计进度与问题证据以 [AUDIT_PROGRESS.md](AUDIT_PROGRESS.md) 为准；它不能替代 `TEST_PLAN.md`、`PROGRESS.md`、`DOCUMENT_BASELINE_REGISTER.md`。原低阶球谐三路线历史不被删除，固定在上述不可变Git版本中。

## 七、2026-11-15 新一轮“文档类型 × 规范符合性”审计输入

> **审计输入，不是审计结果或签署结论。** 本节保留本次用户提供的两份材料范围不同的判定：**意见A**只取得部分核心文件，缺少LLD正文，因此“LLD无法判定”仅对当时材料成立；**意见B**后来提供了各跨模块/模块LLD正文，因此更新为“多数LLD结构上基本符合草案要求，但均未正式冻结”。当前Git分支已有这些LLD，实际审计必须采用意见B的资料可用性，不得倒退为“仓库不存在LLD”，也不得将用户预判冒充对当前HEAD的逐行验收。用户提供的`SRS v0.25`、`TEST_PLAN v0.31`等是**意见形成时的版本记述**，现行文件以本轮读取到的blob/标题为准。
>
> **术语分层**：`草案级基本符合`只评价内容结构与自述职责；`正式基线未符合/未冻结`表示本项目所需签名、固定身份、真实测试、签署尚缺；`不适用`表示该文档不应按SRS/HLD/LLD的标准考核；`材料不足无法判定`只表示指定材料包未提供正文，**不等于文档本身质量不合格**。无外部标准条文的逐条合规宣称。

### 7.1 两轮意见的时间与证据范围

| 维度 | 意见A：限定提供材料 | 意见B：追加LLD正文（当前采用的审阅范围） | 不能作出的推论 |
|---|---|---|---|
| SRS | `REQUIREMENTS.md`：草案结构基本符合；正式需求基线未满足 | 同意；需核版本、66 REQ/AC/TC、§9追踪、冻结字段、DEC-005/006/008状态 | 候选AC→LLD链接≠已批准身份、66TC编号≠66TC已执行 |
| HLD | `ARCHITECTURE.md`草案结构基本符合；`OBJECT_MODEL.md`为附件 | 同意；27条DAG是设计候选，真实CMake/部署/ADR/性能未验收 | 对象模型≠模块LLD，设计箭头≠真实target链 |
| LLD | 因当时未提供`design/contracts.md`等正文，仅能写**材料不足无法判定** | 已提供跨模块与模块正文；**多数LLD草案结构基本符合，但正式/可编码冻结基线均未完成**；低阶SH叶子局部已有固定SHA原生证据 | 不得将旧材料缺失写成现行仓库无LLD，也不能把受限SH局部CTest推广至完整业务 |
| 过程/证据/管理 | 按自身用途评估，多数符合，不适用设计冻结 | 保持；历史决策/授权、WBS、测试计划、JSON、数学专题不是LLD | 过程文档数量或历史审批不等于正式批准或软件通过 |

**综合判定（用户意见，待本轮逐文件证据复核）**：没有一份核心SRS/HLD/完整公共LLD达到“冻结＋批准＋验收”的正式项目基线；SRS/HLD在草案结构上基本符合，模块LLD正文可审且多数草案结构基本符合，过程/证据文件多数符合其自述用途。尤其**DEC-008-01～06的设计决定已作出，状态是DesignDecided / ImplementationNotVerified**：意见中“DEC-008未批准”应精确解释为**公共接口实施/验收/正式冻结未完成**，不是重新要求人类决定01～06。H-10/H-11/DEC-005/006/BASELINE-SIGN/RELEASE-PR无限期延期，须单列且不当作当前未整改。

### 7.2 本次适用的分类型审计准则

| 文档类别 | 草案必须检查的核心要素 | 从草案升级为本项目正式基线额外需要 | 不适用的替代材料 |
|---|---|---|---|
| SRS/需求分析 | 目的范围、干系人/用例、功能/非功能/数据/接口需求、优先级、AC、REQ/AC/TC追踪、术语、假设依赖、变更 | 精确AC→LLD文件/章节/blob/commit、适用DEC例外、真实证据、批准人/日期/范围 | 对象模型、测试计划不能替代需求 |
| HLD/概要设计 | 架构目标/质量属性/约束、组件边界、接口责任、依赖DAG、部署、数据/状态流、错误、ADR、质量场景与冻结清单 | 实际target/链接依赖验证、部署与性能/质量证据、ADR批准、基线身份和所有者签署 | 仅类图或目录矩阵不能证明架构可构建 |
| LLD/详细设计 | 准确函数/模板签名、类型/shape/单位、所有权/生命周期、算法/伪代码、失败/异常状态、并发/ABI/边界、TC/AC与依赖映射 | 候选公共头/实现、真实编译/CTest/输入域与负例、ODR/ABI/线程/TOCTOU（适用时）、DEC和冻结身份/签署 | HLD/OBJECT_MODEL/TEST_PLAN/MIGRATION_MATRIX/SOURCE_INVENTORY/索引/理论专题不得替代 |
| WBS/计划 | 任务/依赖/里程碑/风险/角色/验收前提 | 如果需承诺交付，再取得批准的预算/工期/性能 | 不按LLD签名检验 |
| 测试计划/证据登记 | TC→候选实体/CTest→AC映射；L2/L3/L4来源、状态和固定SHA | **真实执行证据另记**，不同SHA不得混同 | TEST_PLAN不是测试结果，PROGRESS不是正式批准 |
| 审计/配置/来源/迁移/授权 | 文档定位、版本/对象清单、历史与现行隔离、权限/身份和未完成声明 | 对应类型需有完整记录/批准（若适用） | 不计入SRS/HLD/LLD正式符合分母 |
| 专题理论/候选JSON/政策 | 数学假设/证明域/反例或机器状态、证据来源、变更/适用边界 | 只按自述用途评判；不得升级为正式模块接口或测试通过 | 数学25/35检查、Golden候选、L1政策都不能替代软件验收 |

**冻结前共同清单**：路径、精确章节、文档版本、Git blob SHA与commit SHA、依赖集合、批准人、日期及时区、批准范围、例外及DEC、对应真实测试实体与固定来源证据必须能逐项核实；缺项应记具体缺口。没有冻结授权时只登记`未冻结/无限期延期`，不要通过修改Markdown虚构签字。

### 7.3 设计类逐文档用户意见（17份）

| 文档 | 真实类型 | 用户意见中的草案级判定 | 当前优先复核的实质点及正式缺口 |
|---|---|---|---|
| `REQUIREMENTS.md` | SRS | 基本符合结构 | 目的/范围/干系人/用例、66REQ/AC/TC、功能/数据/接口/非功能、优先级、§9和§9.7映射；引用仍是候选身份、DEC-005/006无限延期、008设计已定但未实测、所有者签署缺 |
| `ARCHITECTURE.md` | HLD | 基本符合结构 | 质量属性/部署/组件/时序/ADR/质量场景；27候选DAG≠真实CMake/链接/CTest，冻结清单/性能批准缺 |
| `OBJECT_MODEL.md` | HLD附件 | 对象模型部分符合，不能替代LLD | 类接口/值对象、所有权/生命周期/状态机/45文件映射；候选公共签名未实现/批准 |
| `design/README.md` | LLD索引 | 符合索引用途，非LLD | 14文件分类/10覆盖条目、模板、冻结字段与证据辞典，不把索引当详细设计 |
| `design/contracts.md` | 跨模块LLD | 草案结构基本符合 | Result/Status、ErrorDetail及forward_list、JSONL、类型/线程/ABI与TC；公共头自包含/ODR/跨TU/异常/ABI未实测 |
| `design/cli.md` | 模块LLD | 草案结构基本符合 | 参数schema、exit/stage/JSONL/RunId、安全优先级；inspect-mat仅结构元数据，CLI/负例/TOCTOU未运行 |
| `design/mat_io.md` | 模块LLD | 草案结构基本符合 | RAII、class/type/shape/列主序、Reader/Writer、暂存/回读/不覆盖与责任边界；582/582探针不等于业务Reader/Writer/CTest |
| `design/sampling_noise.md` | 模块LLD | 草案结构基本符合 | 光流/采样/噪声公式、N uniform/3N normal、两噪声互斥、返回与错误；714行独立数学参考不等于生产采样CTest |
| `design/trajectory.md` | 模块LLD | 草案结构基本符合 | 圆/正弦/局部运动/球内近度、Sophus替代/奇异边界；链接与L3/CTest未完成 |
| `design/kvd.md` | 模块LLD | 草案结构基本符合 | Franz/KvD两公式、近度/旧t/一次归一、迭代停止、trace与TC；36轮Wolfram是数学参考，非一般/200轮/生产CTest |
| `design/harmonics.md` | 模块LLD | 草案结构基本符合；受限叶子有实测 | 论文与原CS相位、l≤2基/拟合/三旋转路线、局部Result错误桥接；固定`5e522198` 3/3 SH CTest不能代替公共Harmonic* ABI/集成/DEC-005 |
| `design/adaptive_mfa.md` | 模块LLD | 草案结构基本符合 | M1–M4、RHS、MSE、Session周期/失败不提交、正/负近度与非对称矩阵；生产C++/CTest缺 |
| `design/experiments.md` | 模块LLD | 草案结构基本符合 | 五入口Config/RunContext/Writer注入/Manifest/八轮分母、CurrentSave/HistoricalOnly；真实Runner/Writer/CTest缺 |
| `design/analysis_plotting.md` | 模块LLD | 草案结构基本符合 | 四分析入口、索引/分段/窗口、CurveSeries多MAT来源、可选绘图隔离；L2/L3曲线与Analyzer/CTest缺 |
| `design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 专题LLD | 专题草案符合，公共接口未实现 | IR96/IR98/Choi证据、纯Eigen/禁sdla、递推边界与公共接口候选，不能代替完整harmonics业务 |
| `design/SH_DISCRETE_REFIT_CONSISTENCY.md` | 纯理论专题 | 符合数学专题，不适用LLD | G/D离散逐项拟合、非幂等、正性反例的证明域；数学参考不等于实际C++ |
| `design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md` | 纯理论专题 | 符合数学专题，不适用LLD | s=0/负均值、条件数、非对称/纯旋转退化的数学限定，不新增TC或Session实现 |

**类型分母**：正式36审计对象中，设计结构评价应分别报告SRS=1、HLD=1、HLD附件=1、跨模块/模块LLD=10、专题LLD=1；`design/README.md`索引1、理论专题2和其他19份**不计入核心SRS/HLD/模块LLD的正式符合分母**。11份含专题LLD的候选正文均须区分数学/原生/冻结状态，不据其章节目录宣称全部公式正确。

### 7.4 支撑/过程/授权类逐文件用户意见（19份）

| 文档 | 类型 | 本次意见结论／必须保留的效力边界 |
|---|---|---|
| `TEST_PLAN.md` | 测试计划 | 草案级部分符合；66TC→候选测试实体/CTest→AC，不代表66通过，正式容差未批准 |
| `PLAN.md` | WBS实施计划 | 符合自身用途；98任务P、M0–M5尚未验收，工期/性能不应臆断 |
| `PROGRESS.md` | 证据记录 | 符合用途；固定SHA与原生/数学/历史批次分层，不能替代测试计划或基线 |
| `AUDIT.md` | 审计规则 | 符合自身用途；七维、状态、DOC-P1～P6；本轮新增的分类判定不得冒充已做审计 |
| `AUDIT_PROCESS.md` | 历史SH台账 | SH-01～06限定理论，不能代替现行全工程审计 |
| `DOCUMENT_BASELINE_REGISTER.md` | 配置管理 | 部分符合；固定旧快照可查，不是现行签署基线，当前HEAD/签字/例外缺 |
| `SOURCE_INVENTORY.md` | 源清单 | 静态追踪符合；45 .m/50函数的Git blob不是MAT字节SHA-256、实现或CTest |
| `MIGRATION_MATRIX.md` | 迁移责任表 | 静态追踪符合；45文件/50函数归属是候选，不是C++实现 |
| `TODO.md` | 人类输入/延期 | 符合管理职责；六无限期延期独立不计当前未整改，DEC-008六设计决定已记录、实现未验 |
| `approval/DECISION_IMPLEMENTATION_NOTES.md` | 历史审批落实 | 当时H项/权限为历史事实，applicationCommit/scopeIds不完整，不等于现行许可 |
| `approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md` | 审批材料包 | 符合材料包，不等于执行许可/测试验收；权限位不得按后续局部授权倒填 |
| `decisions/DEC-004_HISTORICAL_MAT_FIELDS.md` | 决策 | CurrentGenerated/HistoricalOnly/缺字段明确失败设计已选，生产测试尚缺 |
| `decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md` | 历史决定 | MIT、Eigen/禁sdla、固定输出根、Result/JSONL、失败等当时决定不能当作当前实现 |
| `decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md` | 专项授权 | 仅l≤2球谐局部，不能自动授权其它模块或代理全工程CTest |
| `golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | 证据规划 | L1永久非必需，L2/L3/L4与AC证据未全部闭合 |
| `golden/GOLDEN_TOLERANCE_PROPOSAL.md` | 容差候选 | atol=0.01/rtol=0.1仅探索，DEC-005未批准，不作正式PASS |
| `golden/NO_MATLAB_EXECUTION_POLICY.md` | 长期政策 | 取消MATLAB/Octave/L1强制门禁，但不取消真实软件/安全/批准 |
| `golden/SH_PAPER_VERIFICATION_CASES.md` | 论文验证候选 | SH-P候选/未执行，不混淆受限SH叶子的真实旧SHA回归 |
| `golden/analytic_golden_candidates.v1.json` | 候选机器数据 | DRAFT_UNAPPROVED_UNEXECUTED、非原MATLAB/生产输出，不能计入设计/软件已验收分母 |

### 7.5 本轮检查计划与优先整改建议（不越权实施）

1. **先审材料身份**：以当前HEAD分别读上述36份正式对象与真实设计正文，依“实际提供/实际存在”裁决，不把意见A的“LLD缺失”记成当前事实。
2. **类型分母独立报告**：SRS/HLD/HLD附件/10模块LLD/专题LLD/理论专题/索引/19过程证据分开，避免以`36/36已完成限定审计`冒充“正式设计符合”。
3. **SRS/HLD冻结门槛**：对AC→LLD精确身份/批准、27拟议DAG与真实CMake/CTest、部署/性能/ADR/质量场景的证据缺口逐项登记；受无限期延期限制的动作只标`DeferredIndefinitelyByOwner`，不催办，不造假签署。
4. **LLD独立逐模块审**：逐项核实字段/接口/shape/单位/所有权/算法/错误/线程/ABI/TC和状态；对实际前后不一致、错误源事实或不可实现的签名，能文档整改就实改并留新DAR证据；真实C++/运行/签署保持延期。
5. **计划与实证分开**：TEST_PLAN中的66TC、PROGRESS固定SHA的3/3球谐局部和582/582只读探针不能互相替代、不能扩大覆盖；L1永久非必需。
6. **清理正文中的历史讨论**：优先按内容性质将DAR/AF长篇历史过程放到审计台账，LLD保留稳定合同、输入输出、现行错误与TC；但**不得机械删除使证据或数学前置条件丢失**，改变正文时需回退审计。
7. **输出逐类型结论和真实审计记录**：每条给出“草案结构/正式冻结”两个独立结果、实际证据身份及尚缺范围；零新增整改必须如实写0，禁止靠增减状态数代替真实修正。


### 7.6 依据本轮实际文档检查确定的评价界限（固定快照）

已在`a59c62ce9c15c80f91b2c341c864fdf6088ca386`固定快照上读取36份正式审计对象：SRS1、HLD1、HLD附件1、跨模块/模块LLD10、专题LLD1、索引1、数学理论2、过程/证据/管理19。**第一批材料的LLD“无法判定”已因正文可取得而失去适用前提**；对现行分支的11份LLD正文只能评价其**草案结构**，绝不据此断言整体代码可编译或正式基线获批。核验后逐文件证据、实际版本、链接和状态另见[AUDIT_PROGRESS §10](AUDIT_PROGRESS.md)，本规范正文不伪造所有者签字。

**需要持续执行的文档卫生规则**：历史DAR/AF整改故事尽量收敛到台账/历史评审，模块LLD正文以当前规范性公式、输入输出、错误/状态、固定测试输入和映射为主；若搬迁某段，必须保证其仍承担的数学证据/判别负例不丢失，并重新核对前后链接。此项属分阶段可维护性建议，不能为了清理字数直接删真实科学合同，也不等于本批次已经完成全部模块重排。
