# 全工程文档规范审计进度与整改台账（AUDIT_PROGRESS）

> **状态：审计进度 v0.2，第一批有范围的实际文档审查及纯文档整改已完成，其余文件继续审计。** 审计准则及用户逐文档意见见 [AUDIT.md](AUDIT.md)。固定仓库基准：`cb3129b1510f4ffc5fadeb94b8fd67bdff0d5e82`；本文件首先登记用户提供的36项判断和六类问题。**用户意见不是本轮已经执行的源码审计；状态与整改结果仅随实际读取和提交证据变更。**

## 1. 口径、状态和可复核方式

- **36个目标文档**：对应用户两张表的17个核心规范/专题和19个支撑/过程/证据文档。当前审计文件 `AUDIT.md` 已替换为新的全工程范围，但低阶球谐原版仍在[固定历史提交](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/cb3129b1510f4ffc5fadeb94b8fd67bdff0d5e82/docs/AUDIT.md)。新增本文件是过程台账，不在36个待审目标中重复计数。
- **文件审计状态**：`未审计` → `正在审计` → `已完成限定审计`。结束审计只代表用户给定维度与记录的核查范围已检查、缺口已明确登记，**不表示文档冻结/代码通过/缺口全部修复**。
- **整改状态**：`未整改`／`已整改`，另设阻塞性质 `纯文档/需软件证据/需批准`。只有完整验收条件满足、相应固定提交或外部批准可追溯时才可标已整改；不得为了提高比例拆问题。
- **证据字段**：DOC编号、路径、初始用户判断、实际检查点、文件blob、问题ID、修订提交、剩余风险；对 66TC/98P/27DAG/L2/L3/L4 只记录有来源的数值，不伪造代码执行。
- **历史隔离**：`AUDIT_PROCESS.md`仍是SH-01～SH-06的低阶球谐专项理论整改记录，原97项全工程历史另有固定提交；本次新DAR不复用旧AF状态或承诺它们已消失。

## 2. 全36份审计对象（用户意见与当前实际审计状态分列）

| ID | docs/下相对路径 | 用户初始规范判断 | 首轮实际需核对的问题 | 审计状态 | 本轮证据／整改关联 |
|---|---|---|---|---|---|
| DOC-001 | `REQUIREMENTS.md` | 草案级部分符合 | AC→LLD章节身份/正式批准 | 正在审计 | SRS v0.14未冻结；已核§3/§4及AC-F-FILTER/SH导航链接，正式逐AC身份待查 |
| DOC-002 | `ARCHITECTURE.md` | 草案级部分符合 | 27拟议DAG与真实构建/ADR | 正在审计 | HLD v0.9；已核27边设计DAG/部署候选与真实执行边界，ADR逐项待核 |
| DOC-003 | `OBJECT_MODEL.md` | 草案级部分符合 | 候选类/生命周期/签署 | 正在审计 | 对象模型v0.6候选；已核职责/生命周期章节与45来源映射声明，签名细项待核 |
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
| DOC-018 | `TEST_PLAN.md` | 计划草案部分符合 | 66TC登记与真实运行证据区分 | 正在审计 | TEST_PLAN v0.29；3/3仅SH固定5e522198，66TC总体未运行，测试追踪待深审 |
| DOC-019 | `PLAN.md` | WBS草案符合 | 98P、M0–M5、真实工期/性能 | 正在审计 | PLAN v0.8；98P与里程碑非承诺，证据链待深审 |
| DOC-020 | `PROGRESS.md` | 符合证据登记 | 固定SHA与历史执行/新状态分离 | 正在审计 | PROGRESS滚动历史；真实用户执行与数学证明按SHA分层，动态信息待审 |
| DOC-021 | `AUDIT.md` | 原专项→本轮综合审计 | 已在初始提交转换为全工程文档规范 | 已完成限定审计 | 新全工程文档审计规则、36对象、用户评价基线已提交1081f0f；原SH范围保留固定历史 |
| DOC-022 | `AUDIT_PROCESS.md` | 球谐专项整改，不适用全工程 | 保留SH-01～06，引用原专项快照避免误链 | 已完成限定审计 | 原SH-01～06理论台账保持不变，增加新审计与旧SH历史互不重计的说明；提交569a6e7 |
| DOC-023 | `DOCUMENT_BASELINE_REGISTER.md` | 部分符合 | 36项固定历史SHA与当期HEAD/批准分开 | 已完成限定审计 | §2–3原36项为固定快照；当前全工程审计应链接AUDIT_PROGRESS；提交5358c3c，正式SHA/批准仍待 |
| DOC-024 | `SOURCE_INVENTORY.md` | 静态来源追踪符合 | 45 .m/50函数、MAT未独立SHA | 正在审计 | SOURCE_INVENTORY原45.m/50函数为静态快照，未查全部字节与MAT SHA |
| DOC-025 | `MIGRATION_MATRIX.md` | 静态迁移追踪符合 | 45/50候选映射，不是代码实现 | 正在审计 | MIGRATION_MATRIX为45/50候选职责，未映射实际源码全量 |
| DOC-026 | `TODO.md` | 管理清单符合 | 历史审批与分阶段授权时间线 | 未审计 | — |
| DOC-027 | `decisions/DEC-004_HISTORICAL_MAT_FIELDS.md` | 决策记录符合 | HistoricalOnly与当前save/代码未测试 | 未审计 | — |
| DOC-028 | `decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md` | 所有者决定符合 | 技术方向≠软件运行/基线签署 | 未审计 | — |
| DOC-029 | `decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md` | 专项授权记录符合 | 历史SH局部权限不得扩展 | 未审计 | — |
| DOC-030 | `approval/DECISION_IMPLEMENTATION_NOTES.md` | 审批落实记录符合 | 旧draft/权限不当最新批准 | 未审计 | — |
| DOC-031 | `approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md` | 审批包符合，非规范 | applicationCommit/scopeIds空 | 未审计 | — |
| DOC-032 | `golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | 证据等级规划符合 | L2/L3/L4缺；球谐3/3状态需避免旧措辞 | 已完成限定审计 | L3总体AN-G仍候选，已有5e522198 SH归档/投影子集真实3/3；提交5e5a92e |
| DOC-033 | `golden/GOLDEN_TOLERANCE_PROPOSAL.md` | 容差候选符合 | 0.01/0.1筛查非DEC-005 | 未审计 | — |
| DOC-034 | `golden/NO_MATLAB_EXECUTION_POLICY.md` | 长期政策符合 | 取消L1，不取消软件验证 | 正在审计 | NO_MATLAB政策已核L1永久非必需；未审全部业务合同映射 |
| DOC-035 | `golden/SH_PAPER_VERIFICATION_CASES.md` | 验证候选符合 | SH-P候选未执行，区别原生已测案例 | 未审计 | — |
| DOC-036 | `golden/analytic_golden_candidates.v1.json` | JSON状态正确，非Markdown | DRAFT_UNAPPROVED_UNEXECUTED不是Golden批准 | 未审计 | — |

**第0阶段真实起点**：36份待审＝0已完成、0正在、36未审计；6个初始DAR目标＝0已整改、6未整改。上述动态状态为本轮实际源文档核对结果，仅标“已完成限定审计”的文件，其检查边界是证据身份、声明定位、链接/版本与明确的文字一致性；**不是逐个AC、接口、算法或批准证据的全面签署**。已完成6项、正在审计14项、未审计16项；软件/批准阻塞并未因此消除。

## 3. 初始整改目标（来自用户的六条主要建议）

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
## 4. 已实施审计、具体整改和验证日志

本轮先固定两份**意见基准文件**：AUDIT.md 文档规范提交 `1081f0f9c2de`，AUDIT_PROGRESS.md 初始36文件/6DAR提交 `140ea0e36000`；随后实际读取上表范围内的源Markdown、目录14项清单、固定5e522198球谐用户测试证据及历史/现行台账，做了如下**纯文档整改**：

| 目标 | 审计发现与定位（起点cb3129b） | 已完成修订及提交 | 仍未宣称的内容 |
|---|---|---|---|
| DAR-006 | `AUDIT.md` 原是SH限定审计；`AUDIT_PROCESS.md`链接旧AUDIT；`design/README.md`第20/37行把所有目录文件当LLD并沿用“11份”说法；`DOCUMENT_BASELINE_REGISTER.md`第7/53/104/115/150行将旧AUDIT/AUDIT_PROCESS当动态全工程台账 | 新规范及进度 `1081f0f`/`140ea0e`；旧SH台账交叉引用 `569a6e7`；目录14份分类 `3614b28`；历史/动态基线导航修订 `5358c3c` | 原SH理论结案仍仅属SH-01～06；旧全工程97AF与本次36份文档评审是不同对象；未批准任何SRS/HLD/LLD |
| DAR-007 | `design/README.md` 第32行同时有“3/3已实测”和“新增2项未验收”；`SH_ROTATION_IVANIC_RUEDENBERG.md`第5、158–159行只写旧低阶库3源/1CTest及“Eigen未实施” | 目录索引 `3614b28`，专题LLD `4a62a9d`；均区分受限SH叶子原生3/3、SH-PAPER候选未执行和公共ABI未实现 | 无新增编译、无正式DEC-005/公共接口批准，原先仅SH的3/3不覆盖全66TC |
| DAR-008 | `golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` 第11行笼统“尚无真实C++测试”，第43行又记录SH 3/3 PASS | 依 `5e522198` 的用户日志将第11行明确拆为整套AN-G候选未批准、AN-G06/07/08归档子集176次已执行，以及二阶投影76次已执行；提交 `5e5a92e` | 其它AN-G、完整独立L3、L2 MAT、L4及正式AC仍未通过 |

**抽样/审计限制**：其余30份文档的用户评价已录入，但没有对每个AC/字段/算法逐字完成深审；其中14份已进行定位/状态/证据抽查，仍列“正在审计”，16份未完成独立核查。不得将文档初审与软件合格混淆。此次修改仅为MD文档，不改C++/CMake/CTest/原MAT/MAT。

## 5. 各轮计数与变化

**起点→终点净变化**：审计对象36→36（0），完成限定审计0→6（+6），正在审计0→14（+14），未审计36→16（−20）；整改目标6→8（+2个实际独立文字冲突），已整改0→3（+3），未整改6→5（−1）。新登记的DAR-007、DAR-008独立于原六类综合阻塞，且均以具体文本矛盾与提交为依据；其余五个复合义务不能因上述局部措辞更正关闭。

| 阶段 | 文件总数 | 已完成限定审计 | 正在审计 | 未审计 | 整改目标 | 已整改 | 未整改 |
|---|---:|---:|---:|---:|---:|---:|---:|
| 第0阶段：用户意见录入、实际审计前 | 36 | 0 | 0 | 36 | 6 | 0 | 6 |
| 第1阶段：源文档限定审查与三项纯文档整改 | 36 | 6 | 14 | 16 | 8 | 3 | 5 |

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
