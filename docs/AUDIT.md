# 全工程文档规范审计（AUDIT）

> **状态：文档规范审计规则 v1.0，评审范围已定义，非正式项目基线。** 基准分支 `dev-plan-P0-P1-mat-dictionary-probe`，启动提交 [`cb3129b1510f`](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/cb3129b1510f4ffc5fadeb94b8fd67bdff0d5e82)。配套动态审计与整改台账：[AUDIT_PROGRESS.md](AUDIT_PROGRESS.md)。本文件是**规范和输入意见基线**，不是声称下列全部文档已经逐项通过独立审计。

## 一、评价基准与总体意见

按**通用文档工程及本项目自身声明的职责**审查，**不宣称符合任何外部标准的逐条条款**。逐项核对七个维度：

1. **定位与范围**：文档性质（SRS/HLD/LLD/索引/管理/证据/专题）、适用目标、排除范围、使用者是否明确。
2. **术语与假设**：名称、版本、输入依赖、环境约束及原作者源码事实是否有清楚来源。
3. **需求可验证性与追踪**：REQ/AC→HLD组件→LLD章节/接口→TC/实体→证据的唯一引用是否可查；未冻结链接不能称已固定身份。
4. **架构责任与依赖**：组件职责、27条设计DAG、数据流/状态流和部署候选与真实构建事实分列。
5. **详细设计完整性**：函数签名、数据类型/shape/单位、所有权/生命周期、算法及边界、错误与状态、测试映射和禁止隐式行为。
6. **状态与配置管理**：文件版本、固定Git blob/commit、批准人/日期/范围/例外、历史快照与当前HEAD分开；批准与执行不能互相替代。
7. **证据真实性**：数学候选、静态复核、原生C++/CTest、L2/L3/L4、验收/发布批准分别记录；禁止把候选或NOT_RUN写成已测试/已批准。

**用户输入的总体初始评价（不是本轮全部已验证的结论）**：核心SRS/HLD/LLD在“评审草案”层面总体结构规范、状态声明诚实、单一来源与追踪意识较强；但没有一份核心规范达到“已冻结、已批准、已验收”的正式基线要求。大量接口仍为候选，多数TC未创建/运行，DEC-005/006/008尚未获正式批准，CMake/CTest/数值证据不完整。支撑文档大多符合自身定位，但专项审计/理论专题/Golden候选不能代替全工程审计或正式LLD。

**状态辞典**：`符合其用途（草案）`只表示文档定位、结构及当前说明经审查；`部分符合（草案）`表示仍有重要合同、追踪或证据缺口；`不适用LLD`表示不按模块详细设计要求考核。两者都不代表正式批准。独立维度`未开始／正在审计／已完成限定审计`与整改`未整改／已整改／外部批准阻塞`由进度台账逐条记录。**没有依据时写“待验证”，不得将缺证据推断为代码缺陷。**

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

**DEC-008不在延期清单**：项目所有者已在2026-11-12对 **DEC-008-01（不承诺跨版本稳定ABI）及DEC-008-06（RunContext独占部分结果/Manifest、失败Result只携Failure）作出决定**，并允许DEC-008-02使用C++标准库派生自`std::exception`的异常。02具体异常/访问器、03工厂/异常安全、04完整根因协议、05 NumPy/pandas式`inspect-mat`显示仍须详细评审，整体状态为`PartiallyDecided / NotFullyApproved`。现行分项说明在[TODO §2.2](TODO.md)，不能将推荐默认当所有者已签字，亦不能把这两项设计决定转变成六项延期工作的实施授权。继续按原源码和数学证据进行实际文档整改；5项综合阻塞保持未整改。

## 六、审计结案与正式基线的区别

**单份文档审计结案**要求定位明确、核对条目/问题记录完成、剩余缺口公开、修订经固定SHA复核。**正式规范冻结**另外要求最新版本、完整追踪链、真实测试与批准，不能因为审计表变为“已审计”而自动发生。全项目正式基线建议顺序：SRS外部义务及AC → HLD组件/依赖/质量属性 → LLD精确签名/状态 → TEST_PLAN/PLAN → PROGRESS真实执行 → 所有者签署。

本次审计进度与问题证据以 [AUDIT_PROGRESS.md](AUDIT_PROGRESS.md) 为准；它不能替代 `TEST_PLAN.md`、`PROGRESS.md`、`DOCUMENT_BASELINE_REGISTER.md`。原低阶球谐三路线历史不被删除，固定在上述不可变Git版本中。
