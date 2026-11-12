# 全工程文档规范审计进度与整改台账（AUDIT_PROGRESS）

> **状态：审计进度 v1.4，第十三批发现并实修Result/Status默认构造可能产生伪成功的公共合同缺陷；H-10/H-11/DEC-005/DEC-006/BASELINE-SIGN/RELEASE-PR按所有者新决定无限期延期，DEC-008已详细说明但仍DecisionPending；同步SRS/CLI/TC/审计规则；仍按36文档/既有五项综合阻塞继续限定审计；仍是未冻结文档审计，不是软件验收。** 审计准则及用户逐文档意见见 [AUDIT.md](AUDIT.md)。固定仓库基准：`cb3129b1510f4ffc5fadeb94b8fd67bdff0d5e82`；本文件首先登记用户提供的36项判断和六类问题。**用户意见不是本轮已经执行的源码审计；状态与整改结果仅随实际读取和提交证据变更。**

## 1. 口径、状态和可复核方式

- **36个目标文档**：对应用户两张表的17个核心规范/专题和19个支撑/过程/证据文档。当前审计文件 `AUDIT.md` 已替换为新的全工程范围，但低阶球谐原版仍在[固定历史提交](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/cb3129b1510f4ffc5fadeb94b8fd67bdff0d5e82/docs/AUDIT.md)。新增本文件是过程台账，不在36个待审目标中重复计数。
- **文件审计状态**：`未审计` → `正在审计` → `已完成限定审计`。结束审计只代表用户给定维度与记录的核查范围已检查、缺口已明确登记，**不表示文档冻结/代码通过/缺口全部修复**。
- **整改状态**：`未整改`／`已整改`，另设阻塞性质 `纯文档/需软件证据/需批准`。只有完整验收条件满足、相应固定提交或外部批准可追溯时才可标已整改；不得为了提高比例拆问题。
- **证据字段**：DOC编号、路径、初始用户判断、实际检查点、文件blob、问题ID、修订提交、剩余风险；对 66TC/98P/27DAG/L2/L3/L4 只记录有来源的数值，不伪造代码执行。
- **历史隔离**：`AUDIT_PROCESS.md`仍是SH-01～SH-06的低阶球谐专项理论整改记录，原97项全工程历史另有固定提交；本次新DAR不复用旧AF状态或承诺它们已消失。

## 2. 全36份审计对象（用户意见与当前实际审计状态分列）

| ID | docs/下相对路径 | 用户初始规范判断 | 首轮实际需核对的问题 | 审计状态 | 本轮证据／整改关联 |
|---|---|---|---|---|---|
| DOC-001 | `REQUIREMENTS.md` | 草案级部分符合 | AC→LLD章节身份/正式批准 | 已完成限定审计 | SRS v0.20：AC-DC-DOMAIN-001补明默认Result/Status可产生假成功，禁止默认构造、工厂创建正确分支并沿用原66REQ/TC编号；提交08089a3，仍非正式冻结 |
| DOC-002 | `ARCHITECTURE.md` | 草案级部分符合 | 27拟议DAG与真实构建/ADR | 已完成限定审计 | HLD v0.10：设计图27箭头=25内部+2外部；根CMake仅低阶SH及3CTest，部署/DEC-006仍待 |
| DOC-003 | `OBJECT_MODEL.md` | 草案级部分符合 | 候选类/生命周期/签署 | 已完成限定审计 | OBJECT_MODEL v0.8：ICoefficientUpdatePolicy和Session正式区分初始化/普通对比同帧迭代写回与UpdateRate比较系数返回丢弃；修正事务性暂存和对象生命周期，提交959ac8e，尚无公共类实现 |
| DOC-004 | `design/README.md` | 索引符合，非LLD | 实际Markdown个数与模块覆盖条目 | 已完成限定审计 | 14份真实目录Markdown=1索引+1跨模块LLD+9模块LLD+1专题LLD+2理论，10个覆盖行；提交3614b28 |
| DOC-005 | `design/contracts.md` | 草案级部分符合 | Result/ABI/ODR/异常线程/自包含 | 正在审计 | contracts v0.8：从实际C++17 variant首alternative默认构造语义发现Result<T,E>/Status<E>伪成功漏洞；新增默认构造=delete、private in_place成功/失败构造、删除公共assign，实际修改6e29211/7fbf03d/4531476；DEC-008仍需审异常/二进制范围，真实公共头/CTest未建立 |
| DOC-006 | `design/cli.md` | 草案级部分符合 | CLI负例、RunId、JSONL、DEC-008 | 已完成限定审计 | cli v0.9：DEC-008候选把Reader确实完整读取与CLI是否默认打印全量double分离，stdout/JSONL仍待批准；提交42ce608，未建真实CLI |
| DOC-007 | `design/mat_io.md` | 草案级部分符合 | Reader/Writer、TOCTOU、写回读、L2 | 正在审计 | mat_io v0.12：纠正readFirstCompatible先inspect后另开readNamed可能混淆A/B输入身份；改同一mat_t*的枚举+解码候选并限定原地改写风险；修复§7表对Manifest权责旧句；提交796d1ce，真实TOCTOU/Reader测试未执行 |
| DOC-008 | `design/sampling_noise.md` | 草案级部分符合 | 生产几何/RNG负例/CTest | 正在审计 | sampling_noise v0.10：原fixed_*、nonpropfehler、erzeugeflussausnear、getvektor已逐源核对；修复durchlaenge/near_last合同并增抵消反例，提交09ae652；原三类面表与生产RNG/CTest尚待深审 |
| DOC-009 | `design/trajectory.md` | 草案级部分符合 | Sophus真实依赖及几何L3 | 已完成限定审计 | trajectory v0.6：在既有旋转尺度退化审阅外，补球外内向正根4/9、球面内外向及严格球内前置检查；固定分数推导复核，提交6dbecb5；Sophus/C++/L3仍未验收 |
| DOC-010 | `design/kvd.md` | 草案级部分符合 | 36轮数学非真实CTest/200轮 | 正在审计 | KvD v0.10：对照原两个误差.m恢复非平方欧氏/角度精确公式、零真值假零残差和5组输入；§4撤销不存在的SingularGeometry/NonFinite错误码而统一公共enum；§5将36轮已生成Wolfram固定参考与未执行CTest分列；提交9c80e41/6fd3220，仍需原生实现及全面数学审查 |
| DOC-011 | `design/harmonics.md` | 草案级部分符合 | 低阶叶子有实测，公共ABI未冻结 | 正在审计 | harmonics v0.15；固定SH 3/3与公共ABI未建分层核对，逐式LLD待核 |
| DOC-012 | `design/adaptive_mfa.md` | 草案级部分符合 | Session原子提交、病态输入和真实软件 | 正在审计 | Adaptive v0.8：与三原始入口源blob逐行核对，Update rates在i=134–182先旋转、按周期刷新本帧主旧系数，比较第一返回写入dumi弃置，仅主返回提交；原比较error不用于保存角误差；固定替身5帧静态重放，提交ba06bea/8dd5dd2/ba3d8c4，Session/CTest仍待 |
| DOC-013 | `design/experiments.md` | 草案级部分符合 | Runner/Writer/Manifest/Ctest | 正在审计 | Experiments v0.9：落实周期比较新系数不写回、主状态更新先后、源初始常数项全1及K=1/5对比输入序列；区分被弃的比较MSE和另算的normrelfehler角误差，提交e43eb09/a5865e4/024260d，未构建Runner |
| DOC-014 | `design/analysis_plotting.md` | 草案级部分符合 | 分析L2/L3曲线、CTest/绘图隔离 | 正在审计 | analysis v0.12：按原graphs_bias_test.m比对单loglog轴/固定xlim/ylim/一条参考+12条误差/仅五legend文本；补末次xwerte与真实N的15.47%差异反例、显示裁剪不能删除数值；提交8ae3344，完整Analyzer与L2/L3/CTest仍未执行 |
| DOC-015 | `design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 专题草案级部分符合 | 旧版本CMake描述与最新低阶叶子验收区分 | 已完成限定审计 | 对照旧1/1说明与用户固定5e522198 3/3；仅低阶叶子，公共ABI仍未实现；提交4a62a9d |
| DOC-016 | `design/SH_DISCRETE_REFIT_CONSISTENCY.md` | 理论专题合格，非LLD | 不能替代模块LLD/测试/批准 | 正在审计 | 已核专题定位与35项数学辅助检查，待独立逐式复算 |
| DOC-017 | `design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md` | 理论专题合格，非LLD | 不能替代Adaptive真实接口/实现 | 正在审计 | 理论专题v0.2：从M2/M3的叉积矩阵推导M对称充要条件mean((mu-1)d)=0，证明均值1不保证对称；提交28c20f6/555eeea，尚未逐式独立重算本专题其它全部25项 |
| DOC-018 | `TEST_PLAN.md` | 计划草案部分符合 | 66TC登记与真实运行证据区分 | 已完成限定审计 | TEST_PLAN §2.12.1A增加Result<double/FlowField>/Status默认构造不可编译、显式成功/失败alternative及move-only/valueless负例；提交a24bf08，没有新正式TC或CTest |
| DOC-019 | `PLAN.md` | WBS草案符合 | 98P、M0–M5、真实工期/性能 | 已完成限定审计 | PLAN R-02实改：论文缺失风险改为已取得IR96/IR98/Choi、三路线低阶SH固定证据与未完成公共接口/集成风险分列；提交6f2a426；不冒称27DAG/预算/新CTest已验收 |
| DOC-020 | `PROGRESS.md` | 符合证据登记 | 固定SHA与历史执行/新状态分离 | 已完成限定审计 | 本轮已核历史E-DOC/SH真实证据与旧97AF；正文修订b69e17c |
| DOC-021 | `AUDIT.md` | 原专项→本轮综合审计 | 已在初始提交转换为全工程文档规范 | 已完成限定审计 | AUDIT新增§5.1执行所有者新延期优先级：六项无限期推迟而非已批准或自动结案；DEC-008只详细解释待审，提交7cf5b34；保持36DOC审计原范围 |
| DOC-022 | `AUDIT_PROCESS.md` | 球谐专项整改，不适用全工程 | 保留SH-01～06，引用原专项快照避免误链 | 已完成限定审计 | 原SH-01～06理论台账保持不变，增加新审计与旧SH历史互不重计的说明；提交569a6e7 |
| DOC-023 | `DOCUMENT_BASELINE_REGISTER.md` | 部分符合 | 36项固定历史SHA与当期HEAD/批准分开 | 已完成限定审计 | DOCUMENT_BASELINE_REGISTER §6.4新增2026-11-12同一固定提交5de2e9d的36文件全Git blob和审计三态；原历史快照保留，新注册blob7cb2c627；签署/依赖/真实数值仍缺，DAR-005仅实质推进未结案 |
| DOC-024 | `SOURCE_INVENTORY.md` | 静态来源追踪符合 | 45 .m/50函数、MAT未独立SHA | 已完成限定审计 | 核对45 .m/50函数声明的固定源清单及blob身份；未重算MAT SHA256 |
| DOC-025 | `MIGRATION_MATRIX.md` | 静态迁移追踪符合 | 45/50候选映射，不是代码实现 | 已完成限定审计 | 核对45/50静态候选迁移映射；不等于真实移植 |
| DOC-026 | `TODO.md` | 管理清单符合 | 历史审批与分阶段授权时间线 | 已完成限定审计 | TODO §0将六项权限/数值/性能/冻结/发布统一标记DeferredIndefinitelyByOwner，§2.2详细提供DEC-008的六大决策问题、已定选项/建议/测试分层，提交a3550cf/150f098；保留DecisionPending且不重新申请MATLAB/L1 |
| DOC-027 | `decisions/DEC-004_HISTORICAL_MAT_FIELDS.md` | 决策记录符合 | HistoricalOnly与当前save/代码未测试 | 已完成限定审计 | DEC-004：选定当前save/HistoricalOnly/缺失字段失败规则，尚无生产Runner/Analyzer测试 |
| DOC-028 | `decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md` | 所有者决定符合 | 技术方向≠软件运行/基线签署 | 已完成限定审计 | 历史所有者决策文件增加2026-11-12后续完成附记，保留2026-11-07审批原意并注明论文已交付、5e522198局部SH3/3、其余授权/ABI/DEC仍开；提交5de2e9d |
| DOC-029 | `decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md` | 专项授权记录符合 | 历史SH局部权限不得扩展 | 已完成限定审计 | 2026-11-09球谐专项授权仅l≤2/Eigen并禁sdla，H-11历史状态不影响后来用户自己实测 |
| DOC-030 | `approval/DECISION_IMPLEMENTATION_NOTES.md` | 审批落实记录符合 | 旧draft/权限不当最新批准 | 已完成限定审计 | 核对H-01～16历史权限与批准状态；不代表当前新授权 |
| DOC-031 | `approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md` | 审批包符合，非规范 | applicationCommit/scopeIds空 | 已完成限定审计 | 核对历史HAR与L1政策冲突，真实修订35c9536 |
| DOC-032 | `golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | 证据等级规划符合 | L2/L3/L4缺；球谐3/3状态需避免旧措辞 | 已完成限定审计 | L3总体AN-G仍候选，已有5e522198 SH归档/投影子集真实3/3；提交5e5a92e |
| DOC-033 | `golden/GOLDEN_TOLERANCE_PROPOSAL.md` | 容差候选符合 | 0.01/0.1筛查非DEC-005 | 已完成限定审计 | 核对0.01/0.1仅供探索且DEC-005未批准；未逐值验证 |
| DOC-034 | `golden/NO_MATLAB_EXECUTION_POLICY.md` | 长期政策符合 | 取消L1，不取消软件验证 | 已完成限定审计 | 核对L1永不必需及L2/L3/L4和测试门禁 |
| DOC-035 | `golden/SH_PAPER_VERIFICATION_CASES.md` | 验证候选符合 | SH-P候选未执行，区别原生已测案例 | 已完成限定审计 | 核对SH-P候选未批准未执行与SH真实叶子测试区分 |
| DOC-036 | `golden/analytic_golden_candidates.v1.json` | JSON状态正确，非Markdown | DRAFT_UNAPPROVED_UNEXECUTED不是Golden批准 | 已完成限定审计 | 核对JSON头部DRAFT状态、AN-G01～12候选ID；未重算数值 |

**历史初始与当前口径**：第0阶段原36份均尚未审计、6个初始DAR均未整改；第1阶段是6份已完成限定审计、14份正在、16份未审及8项DAR中的3项已整改。**第2/3阶段历史状态为36份中15份已完成限定审计、11份正在审计、10份未审；9个DAR中4已整改、5未整改。第四阶段按下方真实审计复核为24份完成限定、9份正在、3份未审；10个DAR中5已整改、5未整改。第六阶段完成限定25份、正在11份、未审0份；14个DAR中9项文档整改完成、5项综合外部/软件条件未完成。第七阶段完成限定26份、正在10份、未审0份；17个DAR中12项文档问题已整改、5项软件/审批复合目标仍未整改。第八阶段仍有26份完成限定/10份正在/0份未审；19个DAR中14项纯文档缺陷已修复、5项综合软件及外部批准条件仍开放。第九阶段本轮实改SRS/TEST_PLAN/PLAN/历史决策/基线矩阵五份正文；36DOC三态不变，21个DAR中16项已修复、5项外部或软件条件仍开放。第十阶段已实改4份技术正文，23个DAR中18项纯文档已整改、5项仍需软件或正式批准；36DOC三态保持26/10/0。第十二阶段实改Adaptive、Runner、对象模型、SRS、TEST_PLAN、TODO六份非台账正文；26项DAR中21项纯文档已整改、5项综合软件/批准目标仍开放，36DOC三态保持26/10/0。** 限定审计仅核对该行明确说明的定位/证据/追踪范围，不等于每个AC、API、算法、签署与软件测试完成。

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
| DAR-010 | approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md；PROGRESS.md | 历史HAR将MATLAB原生Golden采集写成现行必要条件，E-DOC-069又被旧首页称为当期“本轮”，与L1永久不必需及新审计口径矛盾 | 在原文纠正现行L2/L3/L4验收来源并明确历史批次；保留原始审批事实、固定提交和阻塞范围 | 纯文档 | 已整改 |

| DAR-011 | design/adaptive_mfa.md；design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md；TEST_PLAN.md | Adaptive LLD 将s<0时的原始分子/翻号后分子都简称near，笼统声称“与先取绝对值归一化不等价”，与数学专题的严格式near_raw/abs(s)相冲突 | 统一near_raw、near_after_sign、s及near_normalized，在s≠0域写出逐式精确等价与不等价对象，用固定负值手算回归并映射原TC；不更改原MATLAB和已选失败状态 | 纯文档数学冲突 | 已整改 |

| DAR-012 | design/trajectory.md；TEST_PLAN.md | 原 `drehvektor_aus_zwei_vektoren.m` 对**未归一化**叉积范数施以绝对阈值1e−11；旧LLD仅把近零叉积与共线/反向联系，遗漏同夹角不同向量长度可能产生巨大旋转输出断层 | 与原只读源码blob `e41084d65cfc8605a8d377203e933a834c3098b3` 对照，明确阈值量纲、近垂直小长度反例、源保真/安全策略边界、固定TC输入；保留浮点恰等阈值限定，不更改算法源码 | 纯文档数学/源合同 | 已整改 |
| DAR-013 | design/sampling_noise.md；design/experiments.md；TEST_PLAN.md | 原两个fixed_*返回的`near`仅末次标量，但候选内部结果为全近度向量；原`durchlaenge=mean(norm(p_trans+p_rot))`易与两个平均模长之和混淆，导致错误噪声尺度与返回字段 | 对照原fixed_*与`nonpropfehler.m`，在候选结果中明确`near_last`和合成总平均模长；补单方向平移/旋转完全抵消反例及固定uniform返回值；调整Runner与TC责任，不增加原MAT字段 | 纯文档数学/返回合同 | 已整改 |
| DAR-014 | design/kvd.md；TEST_PLAN.md | 原KvD LLD伪代码使用未定义时点的`t_old`且`translation_rule`归一语义不清，随后再次`normalize(t_next)`，可能违背原MATLAB“符号翻转后旧t更新旋转、只归一化一次”的执行次序 | 对照原`norm_koenderink_iteration.m`与`mue_norm_koenderink_iteration.m`，分离`t_for_rotation/raw_t_next/t_next`，限制只归一一次、旋转使用翻号后旧t，补原TC的逐轮跟踪断言 | 纯文档伪代码/次序 | 已整改 |

| DAR-015 | design/cli.md；TEST_PLAN.md；design/mat_io.md | `inspect-mat` 同一命令表同时声称“未指定变量只列元数据”及“compat未指定变量读取首变量”，造成严格/兼容模式调用不确定 | 给出strict/compat×指定/未指定NAME的精确分支矩阵：inspect/readNamed/readFirstCompatible/Parse(2)，多变量无证据时AmbiguousVariable(3)，不把head-only当完整解码；同步现有TC并提交固定SHA | 纯文档CLI行为冲突 | 已整改 |
| DAR-016 | design/contracts.md；TEST_PLAN.md | `Result/Status` 虽使用private std::variant作为公开类的成员，却被写成“不把内部容器作为对外ABI”；该表述错误忽略完整类型布局、调用约定及动态库边界 | 明确访问器不公开内部操作≠二进制布局不外露；保留项目已选轻量variant方向，列出跨TU、编译器/stdlib/共享库版本与异常/ODR的审核条件；不冒称ABI已稳定 | 纯文档ABI事实错误 | 已整改 |
| DAR-017 | design/trajectory.md；TEST_PLAN.md；原getnear_in_kugel_Drehung.m | 球内近度LLD只笼统要求正根与球外失败，未明确证明正判别式、正交点距离仍可能在球外内向输入下成立 | 以o=(5/4,0,0)、n=-ex推导Delta=1、rho=9/4、near=4/9，明确严格球内位置必须先独立检查；补球面内外向和球外外向的静态对照、TC范围与源保真区别 | 纯文档几何定义域缺口 | 已整改 |

| DAR-018 | design/adaptive_mfa.md；design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md；TEST_PLAN.md | Adaptive 6×6 M的四块中M2与M3一般不是互为转置，原LLD却把Eigen LDLT无条件与QR/SVD并列为可替代原inverse的通用求解器 | 从原oneit.m构造完整正近度、s=1、detM=64/729而M非对称的六轴有理反例，推导M对称充要条件mean((mu−1)d)=0；删除通用LDLT，要求非对称求解/全M、b、z/残差和既有TC映射；保留原算法和软件待验证 | 纯文档数学与错误求解器前提 | 已整改 |
| DAR-019 | design/mat_io.md；design/cli.md；TEST_PLAN.md | readFirstCompatible旧§7.1先inspect(path)再调用重新Mat_Open的readNamed(path,name)，同一路径在两次open间可被替换，导致字段选择与值解码混用不同输入文件身份 | 修正为单次有效MatFileHandle枚举/选择/Mat_VarRead/深拷贝，说明仍不能抵御原地并发修改及需真实matio 1.5.26校验；补A/B双文件竞态负例、CLI同一调用约束和Writer单文件Status/RunContext Manifest责任 | 纯文档数据来源一致性/安全合同 | 已整改 |

| DAR-020 | REQUIREMENTS.md；PLAN.md；decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md；TODO.md | SRS§9把MAT只读inspect-mat REQ-F-CLI-001错误映射为五个MATLAB生成入口；SRS§7、PLAN R-02和历史技术决定的“1996/1998论文仍待上传”描述与已取得PDF和SH局部CTest证据冲突 | 改正来源矩阵但不改变66个REQ/AC/TC；现行SRS/PLAN按真实材料和低阶局部证据更新，保留2026-11-07历史决定原时点并增加后续状态与待办边界；固定提交及66项一致性核验 | 纯文档来源/时间线一致性 | 已整改 |
| DAR-021 | TEST_PLAN.md；design/kvd.md；verification/kvd/near_sign_flip_exact_trace.v1.json | TEST_PLAN§2.19.2八方向负near翻号的关键数学符号因反斜杠转义错误变为6个ASCII退格控制字符，使beta、boldsymbol、bar mu等公式无法准确阅读/复核 | 对照既有kvd.md§9.5公式和固定理论输入，重新按LaTeX逐式写出Franz初态、alpha/beta、near均值、Original/Modified raw t及输出；确认退格字符0，保持现有数学证据与TC状态不变 | 纯文档数学公式文本损坏 | 已整改 |

| DAR-022 | REQUIREMENTS.md；design/analysis_plotting.md；design/experiments.md；TEST_PLAN.md；原graphs_bias_test.m、两个Bias生成入口 | AC-F-BIAS-003把原理论参考简写100/sqrt(N)，容易把完整球面实际方向数量8·4^i当作原作者保存xwerte=6·4^i，生成不同的参考值 | 实际将SRS验收表改为使用最后一运行xwerte、四组y平均并保留真实N为单独元信息；LLD与TC给出level2 x=96,N=128,10.206207对8.838835及sqrt(4/3)比值，禁止悄改存档；对应远端正文提交 | 纯文档核心数值引用错误 | 已整改 |
| DAR-023 | design/analysis_plotting.md；TEST_PLAN.md；原graphs_bias_test.m | Bias图被误称“log-log双轴”，但原源只有loglog单坐标轴；源1条参考+三噪声列各四条误差总13条，仅给5个图例文本，且显示窗口与数值索引应分离 | 修正图形类型与准确线数、图例源限制、固定xlim/ylim及源绘图j=3行x=96在xlim外仍是合法分析数值；写入原TC的显示/数值分离判据，不冒充图例自动映射或Plot/CTest已执行 | 纯文档图形/数值合同错误 | 已整改 |

| DAR-024 | design/kvd.md；REQUIREMENTS.md；design/experiments.md；TEST_PLAN.md；原transfehler_div.m与normrelfehler.m | 原transfehler_div注释声称squared error，但源码返回尺度对齐后的sqrt(dot(diff,diff))欧氏距离；更重要是truth=0、estimate非零时原距离表达式返回0而方向误差无定义，可能被移植误判为有效零误差/统计样本 | 逐源码写出两个指标精确公式和正/零定义域；用(e_x,2e_y)→(√2,90°)、(e_x,-2e_x)→(2,180°)、(0,2e_x)→源距离0但方向无定义等固定反例；修正SRS AC-F-ERROR-001、Bias ywerte含义/无定义分母、原TC资格；保留原传播与候选C++显式失败差别 | 纯文档数学与实验统计合同 | 已整改 |
| DAR-025 | design/kvd.md；TEST_PLAN.md；design/contracts.md | KvD LLD§4建议的SingularGeometry及泛用NonFinite不属于唯一NumericalErrorCode，未来照写会出现不存在的枚举或第二套错误映射；同一LLD§5还将已生成的36轮Wolfram参考误写“未生成” | 只引用contracts.md既有ZeroNorm/UndefinedAngle/SingularSystem/IllConditionedSystem/NonFiniteInput/NonFiniteResult等，新增固定同方向Franz分母为0等对应TC；§5真实区分24+12次数学trace已生成与200轮/生产CTest未执行，不增加公共枚举/伪造软件证据 | 纯文档错误码/已验数学证据错配 | 已整改 |

| DAR-026 | design/adaptive_mfa.md；design/experiments.md；OBJECT_MODEL.md；REQUIREMENTS.md；TEST_PLAN.md；原Adaptive三入口 | 原多更新率脚本 `[dumi mittrans mitrot error]=oneit(mitkoeff(:,:,k),...)` 的第一输出未写回比较系数，但原LLD和对象模型只笼统说“调用单步/周期刷新”，易误写成四条每帧持续自适应基线；对照返回error(MSE)也未用于保存的两列方向误差 | 按原源134–182逐步明确四组比较只旋转+按1/5/11/21从本帧主更新前刷新，`dumi`丢弃、`error`未进入存档角误差，唯主akkoeff每帧提交；同时区分Initialization/Comparison的帧内返回覆盖；SRS/Runner/对象/TC一并实改；固定主初始=比较初始1及替身C→C+100，逐帧验证period1与period5错误变体和失败暂存纪律 | 纯文档状态时序/返回所有权 | 已整改 |

| DAR-027 | design/contracts.md；TEST_PLAN.md；REQUIREMENTS.md | 原候选Result<T,E>未声明禁用默认构造，T可默认构造时variant首alternative即成功状态；Status<E>默认monostate必然成功，导致即使没有求解/操作也可能hasValue()==true并传播空Eigen/非法数值 | 实改公共LLD：Result/Status默认构造=delete，仅private带tag in-place构造，success/failure工厂明确选分支；删除公共copy/move assignment抑制变体alternative变更；SRS AC-DC-DOMAIN-001与TEST_PLAN §2.12.1A加入三种默认构造编译负例、move-only/异常分支；不伪称现有公共头/CTest已执行或DEC-008批准 | 纯文档公共接口伪成功缺陷 | 已整改 |

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

### 第4阶段：文档实改与扩大限定审计（2026-11-11）

- **DAR-010已整改**：历史审批包 `approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md` 的H-09原将MATLAB版本、采集脚本与逐节点原生Golden写成未来必需证据，与已经生效的L1永久非必需政策不符；真实修改了§8、§9及相关审批用语，改用独立L2/L3/L4来源并保留历史人类JSON结论。提交 [35c9536](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/35c95361aeafb41e573f53fad18003d4e60e73e7)，Git blob `f2ece1cdd82641e9978513d06ff47f43c5a6ee9c`。并把 `PROGRESS.md` 的2026-11-10 E-DOC-069“本轮”明确改为历史数学批次，防止旧97AF冒充新36文件/10DAR台账；提交 [b69e17c](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/b69e17c939d790e6e63c29cdcd46584ab4f16db3)，blob `00aa3aa77f47081f1fd304adf3c7d1089d275fd7`。
- 限定审计：检查了跨模块合同、CLI、MAT I/O、采样、轨迹、KvD、球谐、Adaptive八份设计的候选/测试声明；新增深入审阅中的Runner、分析模块及两份数学专题；限定完成九份的**文件定位、状态/来源身份、审批/证据分层**复核（详见DOC各行）。不表示所有公式、898行AN-G JSON或全部接口已逐项通过，未独立重算数值。
- **仍受阻的DAR-001～005保持未整改**：需正式所有者批准及完整冻结身份、实际生产目标/性能/DEC-006证据、公共ABI与Session代码/测试、其余真实TC及DEC-005/008。历史SH3/3和582只读探针不可扩张为完整软件验收。当前轮次不写生产C++、不执行CMake/CTest/MATLAB/MAT写入、不批准或发布。

本阶段统计：文件总数36→36（0）；已审计15→24（+9），正在11→9（−2），未审10→3（−7）；整改目标9→10（+1），已整改4→5（+1），未整改5→5（0）。

### 第5阶段：优先真正修订技术正文，不靠改变审计文件计数（2026-11-11）

本次直接读取目标分支固定起点 `0e04e451f85c3394931a48fb313f14f029612e03` 下的五份主技术文本，并用原只读 MATLAB 源 `kurveglaetten.m`、两份 `graph_agent_in_sphere_adaptive_matched_filter*.m` 校对分段语义；数学与接口合同只在原设计权限内修订。**不是编译/CTest或已批准接口，也没有更改origin/MAT或任何生产代码。**

| 分类/关联DAR | 已修订的非台账正文（不是仅更改统计） | 可核查提交与整改证据 | 保留缺口 |
|---|---|---|---|
| **DAR-011独立数学错误，已整改** | `design/adaptive_mfa.md` v0.5→v0.6：在有限非零s下明确 `near_normalized=near_raw/abs(s)`，但不等于 `near_after_sign/abs(s)`；旧“先取绝对值不等价”没区分分子，现逐式纠正； `TEST_PLAN §2.10` 补 `[-1,-3]→[-1/2,-3/2]` 的分支负例 | [159ef22](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/159ef22539a54adaa88ea67745cd8538b735adc6)；[2c37ad6](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/2c37ad60254f00823c49352f493e69293d636f7b) | 尚无完整3×5/N×6的独立L3、Adaptive实际C++/Session与CTest，DAR-003/004不关 |
| **DAR-003的真实候选API改进，仍未整改总目标** | `design/contracts.md` v0.5→v0.6：将旧 `T&& value() &&` 改为 `T value() &&`；新增右值自拥有 `error()` 与const右值禁止借用，明确 `valueless_by_exception` 不是可取Failure；`TEST_PLAN §2.12.1` 加move-only/生命周期/异常注入验收条件 | [95cdc62](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/95cdc624d1dcf6f52bdcfcfea983f57e0a0645ba)；[a5f8d1b](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/a5f8d1bd3ae1f6098fd25e95bdf93b1082759a0f) | 真实模板头/ODR/ABI、异常边界和编译均未完成 |
| **DAR-004的静态分析合同纠偏，仍未整改总目标** | `design/analysis_plotting.md` v0.10→v0.11：按源的两个 `if`、固定4段、末尾覆盖第4段建立可复现正常/阈值相等/缺段/多穿越夹具；`TEST_PLAN §2.11` 增精确 begin/end，并纠正原“平移前三列平滑”应为“**第一列原样、后三列平滑**” | [5b52808](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/5b528083d03934f92fda1d30a3afd5068d6ed23d)；[2c37ad6](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/2c37ad60254f00823c49352f493e69293d636f7b)；独立JS控制流静态重放确认 `begin=[50,180,320,460]`、`ende=[120,260,400,550]`，高值全部输入出现零边界 | 没有实测MATLAB、完整L3曲线、Analyzer或CTest；其它分析合同仍待测 |
| **DAR-003的MAT Writer安全合约修复，仍未整改总目标** | `design/mat_io.md` v0.10→v0.11：统一**暂存关闭→严格回读/哈希→原子非覆盖发布→最终身份核验**，定出发布前失败不得产生最终名、发布后失败Manifest不可Complete、RunId/variable名二次验证、Writer只负责单文件Status、全RunOutcome由RunContext/Runner负责；`TEST_PLAN §2.9` 增对应阶段注入矩阵 | [ccc89cc](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/ccc89cc9db20bc661fb72ed36c05df073346a9a6)；[94b6f31](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/94b6f316d3bb341dae09f00b4cd9e8bcdffe115a)；[f801669](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/f801669f8ef37d17e037869dfcc495a27b208436) | 尚无具防竞态保证的生产Writer、独立回读、安全负例与CMake/CTest |

**七项起止口径（本阶段）**：整改目标总数10→11（+1，只有独立数学冲突新立目标）；已整改5→6（+1），未整改5→5（0）。审计文件总数36→36、已审计24→24、正在审计9→9、未审3→3（均0）：**这不是没干活，而是5份技术正文已真实整改，但限制性审阅/软件证据并未达到扩大“已审计”范围或关闭五项综合DAR的条件**。所有新增/旧TC的执行状态仍为未运行；没有虚造软件通过/批准证据。

### 第6阶段：逐源整改三类数学/算法/返回合同（2026-11-11）

**本轮起始HEAD** `79be039f6ab832691245eb7f5bec9f95d14b8ba3`。先读取本分支既有AUDIT/AUDIT_PROGRESS及其36份、11目标口径，再实际读取原轨迹/旋转/近度六份`.m`、采样/加噪五份`.m`、两套Bias入口、Franz+Original+Modified KvD三份源文件；不凭旧审计计数直接关单。所有源文件只读。

| 编号/实质缺口 | 实际改动、固定证据与提交 | 不能据此认定的结果 |
|---|---|---|
| **DAR-012（文档数学错误已修正）** | [trajectory v0.5](design/trajectory.md)：原`drehvektor_aus_zwei_vektoren.m` 的 `normfaktor3=||v1×v2||` 是**带尺度的原始叉积**，不是方向角阈值；在`v1=e_x,v2=5e−12e_y`与`v2=2e−11e_y`时夹角都90°，源结果分别为`(π/2)·5e−12e_z`和`(π/2)e_z`。已补TR-C05/源保真与安全模式区别及浮点恰等限制；提交 [befced3](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/befced3045a5f230f5e1fadd8b83c72c859f59f1)、[458b544](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/458b5443abeac1a6a5cd84429c8e9ac4264905cc) | 不是实际Sophus C++/轨迹测试通过；未批准自动“纠正”源算法 |
| **DAR-013（返回与噪声数学义务已修正）** | [sampling_noise v0.10](design/sampling_noise.md)：原fixed_* 的`near`覆盖式只留下**最后一次标量**，候选`nearness`为新内部N维诊断，增加`near_last`；原`durchlaenge`取合成流的平均模长，不得用两个分量平均模长之和。对于`d=e_z,mu=1,t=e_x,omega=−e_y`两分量相消，源总模长为0而分量模长和为2；非零噪声比例下仍调用3个normal。另以缺损level0的6个固定uniform`[0,0,0,0,0,1/2]`确认标量末值1（这是注入边界值，**不代表观察到MATLAB随机流输出0**）。改动同步 [experiments v0.6](design/experiments.md) 与 [TEST_PLAN](TEST_PLAN.md)；提交 [09ae652](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/09ae65235918cacaebae96c59b5052991c95680d)、[1d89bf3](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/1d89bf3a3bad106aee3672b8d0c56cc8a87dc7b5) | 这是源数学/候选ABI及测试计划修订，非真实随机样本/生产数值、MAT生成/CTest |
| **DAR-014（KvD伪代码顺序已修正）** | [kvd v0.9](design/kvd.md) 的步骤改为负near符号翻转后缓存`t_for_rotation`，算`raw_t_next`，**仅一次**归一化成`t_next`，旋转用**旧的翻号后t**再提交新t，终局近度不重算。与两份原KvD源的`transneu=...; transneu=transneu/norm; rot=...trans...; trans=transneu`次序一致；提交 [d7e4378](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/d7e43789cc752f03028e9bef1f26df9c0145231e) | 不声称已测试200轮、一般数值、生产C++/CTest |
| **关联TEST_PLAN实际修订** | 固定角度/长度边界、`near_last`/总流模长、KvD旧t与只归一一次的复核输入，全部映射现有TC而不增66TC；提交 [604c62e](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/604c62e32a0457d3f745ae5ea79e9e05945864a7)、[af2b1ef](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/af2b1ef11837dafdf93eee5ae89353ec30a8897c) | 没有创建新正式TC/运行代码、没有签DEC-005/008 |

**限定独立算术复查**：使用纯JavaScript按原算式重算3个几何/噪声条件，得两个同90°转动输出模长约`7.853981633974482e-12`和`1.5707963267948963`；流的总模长0、分量模长和2；六次uniform夹具对应近度`[2.5,2.5,2.5,2.5,2.5,1]`，所有静态断言通过。**该JavaScript核算不是MATLAB、Wolfram证书、生产C++或CTest**。本次未修改原`origin/**`、MAT、CMake/CTest/build.sh，不运行MATLAB/Octave，不进行正式批准。

**审计范围统计变动**：DOC-008采样、DOC-010 KvD由“未审计”进入“正在审计”，因完整面表/全部迭代及真实证据尚需进一步核对；DOC-009轨迹按源级公式、职责与明确留存缺口完成**限定审计**，不是正式软件通过。DOC-013实验、DOC-018测试已有整改证据更新，其审计状态按原规则保持。原历史SH 6/6独立结案及全工程旧97 AF不重新计入本台账。

**本阶段起止统计**：目标总数11→14（+3个独立且已有实际正文更正的真实缺口），已整改6→9（+3），未整改5→5（0）；审计文件总数36→36（0）、已审计24→25（+1）、正在9→11（+2）、未审3→0（−3）。综合DAR-001～005的真实软件/批准条件仍全部开放，不把这3个文档级闭环冒充软件级闭环。

### 第7阶段：三个独立技术合同缺陷的实际修订（2026-11-11）

本轮起始提交 `a6bc8c970c6b409d9fb90687f05a798df1bbe7ab`；首先重读全工程`AUDIT_PROGRESS.md`（14 DAR：9已整改/5未整改；36 DOC：25完成限定、11正在、0未审），核对历史SH专项与当前全工程文档口径分离。所有整改只写现行`docs/`，原`origin/**`只读。

| 具体缺陷/真实修订 | 证据、提交与验证的边界 | 不会据此关闭的事项 |
|---|---|---|
| **DAR-015 inspect-mat执行分支矛盾** | [`design/cli.md`](design/cli.md) v0.6→v0.7 将“无NAME只inspect”与“compat无NAME读取首变量”拆为**strict无NAME=inspect metadata；strict有NAME=readNamed实际读值；compat无NAME=readFirstCompatible；compat有NAME=parse(2)**。多变量无顺序证据必须`AmbiguousVariable/read(3)`，错误变量不能回退，重复未知mode走Parse(2)。实改提交 [4629edf](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/4629edf5adccb7d4bcab2ee96e7976dac11f55df)，与[MAT Reader LLD](design/mat_io.md) §7.4核对；[TEST_PLAN](TEST_PLAN.md) 对应既有`TC-F-CLI-001`补四分支/解析优先级，提交 [2291b16](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/2291b16e02d48797f5e372f28ed60ea49bd94e31)。 | 并无真实CLI、合成多变量文件/Reader业务测试、DEC-008或ABI批准；DAR-003/004保留 |
| **DAR-016 private variant ABI误述** | [`design/contracts.md`](design/contracts.md) v0.6→v0.7 明确private成员仍参与公开完整值类型的size/align/move/destructor与二进制调用约定；不得把源码隐藏误当ABI稳定。修订提交 [a3db9b5](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/a3db9b5d5be3e8b536f4510638e2877fa16140df)，[TEST_PLAN](TEST_PLAN.md)加入跨TU/如有动态库边界的工具链/版本/ODR验证（[c97c9ef](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/c97c9ef19d6e6db23ba2a87e1a3122fe1c0030dd)）。 | 没有重选std::variant设计或创建头文件；实际ABI/link/CTest未执行，DAR-003不关 |
| **DAR-017 球外内向射线的正根陷阱** | [`design/trajectory.md`](design/trajectory.md) v0.5→v0.6 用原`getnear_in_kugel_Drehung.m` blob `ea807d35ae70f4a2cd2533160dc501ad3a154b8b` 的`b*b-c+1,-b+sqrt`公式核出：**球外o=5/4ex、内向n=-ex**仍有`Delta=1,rho=9/4,near=4/9`；因此正根不等于位置满足`||o||<1`。补TR-C06、球面边界内向/外向与外部拒绝合同，提交 [6dbecb5](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/6dbecb5d35c86b6e95027a70c78e38c5612f415e)，测试文档同步提交 [c97c9ef](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/c97c9ef19d6e6db23ba2a87e1a3122fe1c0030dd)。使用纯JavaScript按原算式验证5种球内/边界/球外分数组合；**不是MATLAB/原生CTest**。 | 原MATLAB在球外仍算出公式值；候选C++拒绝属于显式安全策略。真实Sophus、数值范围、近边界容差及生产CTest未通过，不关DAR-003/004 |

**审核范围与状态**：DOC-006完整阅读CLI的模式/参数/错误/权限分支并与Reader、TEST_PLAN交叉核对，完成限定的**文档行为审计**；其后续软件证据/批准仍显式登记。DOC-005合同对ABI与Result的静态复核已实改，但公共头、异常、ODR缺原生实证，保留正在审计。DOC-009和DOC-018维持已有完成限定审计身份、仅更新已审核的具体数学/测试要求；不扩展到其它未核算法。**没有新C++/CMake/CTest/原MATLAB/MAT写入，没有证据批准或正式基线冻结。**

**第7阶段七项起止值**：整改目标总数14→17（+3）；已整改9→12（+3）；未整改5→5（0）；审计文件总数36→36（0）；已完成限定审计25→26（+1）；正在审计11→10（−1）；未审计0→0（0）。新增3项均有真正的技术正文更正与固定提交证据；不是调整文件计数代替实改。

### 第8阶段：Adaptive 非对称求解与 MAT Reader 同一来源身份的真实整改（2026-11-11）

**起点**：固定 `d1383997d199e715dbfad6d96c3245c18909cd17`；第7阶段36文件、26已审计/10正在/0未审，17目标、12已整改/5未整改。当前全工程AUDIT规范与历史SH专项不混计。本阶段只修原有技术正文和测试合同，未经新增生产源码、MAT/原MATLAB或真实CTest授权，不扩张到正式基线冻结。

| 具体错误与实改对象 | 原始来源/真正证据 | 已提交与未声称的范围 |
|---|---|---|
| **DAR-018：错误的通用LDLT求解器资格** | 只读`oneit_adaptiv_matched_filter.m` blob `48e51bcd52fb8f27cf35b3fce38b28507af36e55`。原四块的`M2=[-mean(d)]×`、`M3=[mean(mu d)]×`，一般不互为转置；完整矩阵对称当且仅当`mean((mu-1)d)=0`。固定六轴`±ex,±ey,±ez`及正近度`(3/2,1/2,1,1,1,1)`可由原CS系数构造，s=1，得到`M1=M4=2/3 I3,M2=0,M3=[ex]×/6`，`det M=64/729`但`M≠M^T`。切向流`p=-mu(I-dd^T)ey`的`rhs=(2/3 ey,1/6 ez)`，精确解`z=(ey,0)`。本轮曾用独立Python/SymPy有理运算实际复核M、rhs、rank、det、z及零残差；此运算未提交为批准Golden、不是Eigen/CTest。 | 实改[Adaptive LLD v0.7](design/adaptive_mfa.md)删除LDLT通用候选并要求一般非对称QR/SVD或等效求解；[理论专题v0.2](design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md)补严格对称判据和特例范围；[TEST_PLAN](TEST_PLAN.md)补已有ADAPT/NUM TC完整6×6验证。提交：[17c2b52](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/17c2b520b213efef13cfbe6e416a82f115bf3c2d)、[28c20f6](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/28c20f6ebe8aa79437b81f604ca40eb126f63ef1)、[d5d7385](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/d5d7385aa5e21a5284065aab487d580f359c9cc9)；保留H-10/DEC-005、生产Adaptive/CTest缺口，DAR-003/004不关。 |
| **DAR-019：同一MAT路径两次打开的元数据/载荷竞态** | 原`getflowfield.m` blob `fafcfeaa357a55572058fa5c6e92d58f934c65a5`一次load到x后fieldnames/getfield；原LLD的`readFirstCompatible`先`inspect(path)`再公共`readNamed(path,name)`分别Mat_Open，可能A/B身份混用。单个mat_t*也不能保证防止同一inode原地并发写入。 | 实改[MAT LLD v0.12](design/mat_io.md)枚举/解码同一句柄、保留matio 1.5.26实测门槛，修正Writer只返回单文件Status而RunContext持有Manifest的旧表格冲突；[CLI LLD v0.8](design/cli.md)禁止兼容模式二次打开；[TEST_PLAN](TEST_PLAN.md)补A含alpha/B含beta路径置换与同inode修改的隔离负例。提交：[796d1ce](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/796d1ce2266361ec58e1916acd3aac7529e4b4ce)、[6d0e212](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/6d0e212c35ea4ed37fb32a66208d78724574e0ff)、[3e9d9aa](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/3e9d9aae684f639dd460e125caf9960e905156da)；尚无真实Reader/TOCTOU/CTest，DAR-003/004不关。 |

**本阶段进度与分母**：整改总数17→19（+2，均为有具体缺陷及实际源码位置的独立整改）；已整改12→14（+2，**仅文档问题**）；未整改5→5（0，均仍需软件和/或正式人类批准）。36审计文件不增不减；已审计26→26（0），正在10→10（0），未审0→0（0）。本阶段实际修订五份非台账技术文件，无原始MATLAB/MAT/生产C++/CMake/CTest运行或批准。未把文档修订当成正式TC或ABI通过。

### 第9阶段：需求源追踪、数学公式修复与近期配置身份（2026-11-12）

**起始HEAD**：`298b8398471106948b567af425e3ff76d847120b`；起始台账36份文档（已审计26/正在10/未审0）、19个目标（已整改14/未整改5）。遵守[AUDIT.md](AUDIT.md)规定的**仅文档审核与真实文本修复**，不擅自执行生产C++、CMake/CTest、MAT读写测试、MATLAB或正式基线批准。所有原MATLAB/MAT文件只读且未改写。

**一、本轮实改不是调整台账**：

- **DAR-020 来源追踪与现行阻塞错误**：实际修改[REQUIREMENTS.md](REQUIREMENTS.md) v0.15→v0.16，将 `REQ-F-CLI-001/TC-F-CLI-001/AC-F-CLI-001` 的源从错误的“五生成入口”修为只读 `getflowfield.m`/MAT检查，并改正§7旧论文待上传描述；提交[99535c3](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/99535c3cdf0c4c2f62e15b9b62fc84ceef26780a)。实际修改[PLAN.md](PLAN.md) R-02，把过时“等论文才能做数学/递推”改成真实公共ABI/旧新场作用/全业务集成/容差风险；提交[6f2a426](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/6f2a426d0e010f82d6896868d19b67a6bcfd1ba3)。历史[项目所有者2026-11-07决定](decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md)保留当时条件并补实际后续已取得PDF、低阶局部SHA下3/3及未解除授权边界；提交[5de2e9d](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/5de2e9d521efd36ca9f2add3d0c76f9090f9949a)。`TODO.md`已明确PAPER-SH材料完结，未修改用户历史批准JSON。
- **DAR-021 数学公式本体损坏**：[TEST_PLAN.md §2.19.2](TEST_PLAN.md)原八方向近度翻号trace中有**6个ASCII退格字符**，来自 `\\boldsymbol/\\beta/\\bar` 等文本转义造成显示及机器核查失效。已按[原 KvD LLD §9.5](design/kvd.md)恢复Franz向量、\(\epsilon,\alpha,\beta,\bar\mu\)、两类未归一平移和终端缩放的完整LaTeX；字符扫描 **6→0**，不改固定Wolfram证据文件、66TC或代码；提交[71b4358](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/71b43588369b98ab7bbc89e7991337144ca294d4)。这不是新增数学运算或原生CTest。
- **DAR-005真实推进但未关**：[DOCUMENT_BASELINE_REGISTER.md §6.4](DOCUMENT_BASELINE_REGISTER.md)已实写**全部36份**审计目标在单一固定提交[5de2e9d](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/5de2e9d521efd36ca9f2add3d0c76f9090f9949a)上的**36个完整Git blob SHA**及其对应26/10/0审计身份，提交[715af6e](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/715af6e8488f3f2cb3679a183c7ef14b5fd305f4)，新增登记文件blob `7cb2c627e92674afb0307ac1396fe6d51365bc89`。严格保留旧3e3ad2a历史快照；特别注明自身修改后的新blob不能反写到旧提交、也不自动跟随HEAD。**已取得当前阶段可复核配置身份，但正式签署人/日期时区、LLD全依赖集合、DEC处置与实际TC仍缺，所以DAR-005保持未整改**。

**二、静态一致性核对**：以GitHub远端最新版读取`REQUIREMENTS.md`和`TEST_PLAN.md`，实际按表行解析 SRS正文58条+附录DC8条=**66个不重复REQ**；TEST_PLAN §3共**66条 TC→REQ→AC**，逐编号匹配不一致**0**；恢复后的测试文档退格字符数**0**。新版本矩阵§6.4精确36个不重复文件路径和SHA，不把候选、数学研究和真实软件通过混用。这些是**连接器内容静态检查**，未实际执行`scripts/check_docs_traceability.sh`或CTest，不能冒称软件测试通过。

**本阶段计数**：整改目标19→21（+2个独立真实文档缺陷），已整改14→16（+2），未整改5→5（0）；审计文件36→36（0）、已完成限定26→26（0）、正在10→10（0）、未审0→0（0）。五项DAR-001～005为含正式软件/审批的综合义务，本轮已推进DAR-001来源身份与DAR-005当前固定快照，但未满足完整关闭条件；无新增批准/原生运行，不能宣布完成最终第二轮审计。

### 第10阶段：Bias 源代码数值和显示合同的实际修复（2026-11-12）

**起始提交**：`10537d4d8ef49782331d37ed2621c429724bbaf2`；本批按照现行[审计规范](AUDIT.md)只做原MATLAB源静态复核和技术正文实改。已读取原`bias_test/data_analysis/graphs_bias_test.m`，及两份Bias数据生成`.m`；完整球面真实N、保存xwerte、参考绘图数值的互异在原源中可直接计算，审计不凭旧统计直接宣称已修。

- **DAR-022**：原`graphs_bias_test.m:20–51`按k覆盖xwerte，仅对四类y求均值，参考`100/sqrt(xwerte(j))`；两原生成器存的`xwerte=6·4^i`，完整球面真实视线却为`8·4^i`。因此SRS`AC-F-BIAS-003`的旧`100/sqrt(N)`在完整球面可误导实现，已真正修成**末次保存xwerte**而非实际N；`REQUIREMENTS.md` v0.17提交[281a7c3](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/281a7c3e14588f874c9daf5eb84bcb1be3e1f8f6)。`design/analysis_plotting.md` v0.12提交[8ae3344](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/8ae3344671f90030eee7dccb7d9981dce789c159)，`design/experiments.md` v0.7提交[9d5b26e](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/9d5b26e4afb570b462cc34eaea9bef7fd04a29b9)，测试计划提交[a51aa9d](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/a51aa9d9b37708c28b44425b830e0ce7a2ea99ea)。固定人工数值检验：level=2时xwerte=96、N_actual=128，原参考100/√96≈10.206207，错用N为100/√128≈8.838835，二者比√(4/3)≈1.15470；不能混写为同一理论曲线。
- **DAR-023**：原`graphs_bias_test.m:44–79`调用`loglog`但没有`plotyy`；Bias属于**单个x-log+y-log轴**，此前分析LLD“log-log双轴”表述错误。源一条黑参考线与三噪声列×四y组一共13条曲线，但尾部仅调用5个legend字符串，不能从5字母推断所有13条源值归属；源xlim=[100,3000000]、ylim=[0.06,200]，使level=2的x=96**属于已生成的数值却在可视范围外**。已更正分析LLD并添加显示过滤≠丢弃原数值/非有限判断的测试条件，提交[8ae3344](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/8ae3344671f90030eee7dccb7d9981dce789c159)、[a51aa9d](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/a51aa9d9b37708c28b44425b830e0ce7a2ea99ea)。未猜测MATLAB运行时5个legend与具体曲线对象的绑定顺序；不新增已批准的绘图后端或ABI字段。
- **仍缺的真证据**：以上为源代码数学/显示事实和手算数值；**未运行MATLAB、C++、Analyzer、PlotExporter、CTest、MAT写回读或数值L2/L3/L4完整对照**。综合DAR-001～005仍因批准/软件证据未关，本批新增整改仅限两项已落地的独立文档合同错误，不能把它们冒充生产代码修复。

**七项统计与净变化**：整改目标21→23（+2），已整改16→18（+2），未整改5→5（0）；审计文件总数36→36（0），已完成限定审计26→26（0），正在审计10→10（0），未审计0→0（0）。**4份非台账技术正文已实改**，审计文件数不增加，不靠文件状态膨胀表现进度。

### 第11阶段：原误差指标数学与公共枚举合同的真实整改（2026-11-12）

**起始固定HEAD**：`b462c286f661ff1d6f9b11b240112e5e81a4f82a`；起始36文档（26已完成限定/10正在/0未审），23个整改目标（18已整改/5未整改）。本轮遵守`AUDIT.md`只授权文档审查与真实文本修改的范围，读取原只读`transfehler_div.m`、`normrelfehler.m`、`franzeigen.m`、Bias两数据生成脚本和现行`contracts.md`错误枚举，不修改原算法/原MAT或生产代码。

| 本轮确实修复的缺陷 | 数学/代码前置证据与实际正文提交 | 不据此宣称 |
|---|---|---|
| **DAR-024：误差指标公式/定义域与Bias统计错误** | `transfehler_div.m` 注释的squared与实际`abs(sqrt(dot(diff,diff)))`矛盾。非零输入应为`E_scale=norm(u-norm(u)/norm(v)*v)`，角度是`normrelfehler=abs(acos(dot(u,v)/(norm(u)*norm(v))))*180/pi`，二者单位和含义不同。**u=0、v=2e_x时，源尺度对齐误差=0但方向角无定义**，这不是算法正确，更不能作为C++有效统计样本；u=e_x,v=2e_y得到sqrt2/90°，u=e_x,v=-2e_x得到2/180°。采用纯JavaScript沿源运算符独立重算五组，正常值及两类零范数边界均如表；**不是MATLAB或生产C++实测**。已真实改[`design/kvd.md`](design/kvd.md) `1.1、[`TEST_PLAN.md`](TEST_PLAN.md) `2.1.1、[`REQUIREMENTS.md`](REQUIREMENTS.md) AC-F-ERROR-001、[`design/experiments.md`](design/experiments.md) 的Bias六y值及非法样本/Manifest纪律。提交 [9c80e41](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/9c80e41667a072304c76fa7a4c3b5a1577cd3a3b)、[9f0b744](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/9f0b744fc6d77adad284f5c6aac2e28eedf5f233)、[58ab90f](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/58ab90f2bb7d8ba3f5dffa05f8413a5b4abb94ea)、[8f876c8](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/8f876c8d9384107c06d7fbec3a97d4517dc0216b)。 | 未执行源MATLAB、生产Cpp、TC-F-ERROR-001原生编译/CTest或签DEC-005/008；不能把新C++拒绝说成源MATLAB返回错误码。 |
| **DAR-025：KvD非法枚举及旧数学参考状态错报** | `design/kvd.md` §4旧建议`SingularGeometry`、`NonFinite`在唯一[`contracts.md §3`](design/contracts.md)的`NumericalErrorCode`中不存在；现用已有`SingularSystem/IllConditionedSystem/NonFiniteInput/NonFiniteResult/ZeroNorm/UndefinedAngle`并区分Franz零模板分母与初始平移零模。`TEST_PLAN.md` §2.1.2补`d_i=e_z`时Franz z模板分母零的静态负例及现有错误阶段映射。另外`kvd.md` §5之前连续五行误写参考数值“未生成”，实际`verification/kvd/six_direction_exact_trace.v1.json`的24轮与`near_sign_flip_exact_trace.v1.json`的12轮数学trace均已登记；修正为**36次真实Wolfram数学参考已存在、其它迭代/真实C++/CTest尚缺**，避免伪称从未计算或相反把36数学轮当36CTest。提交 [6fd3220](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/6fd32200409068a28a7e6a62e9325996888c34b9)、[f349498](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/f349498e25e8c40bafc05623a21e8bc603e20be6)。 | 不扩展公共enum/未在生产源码实现任何安全失败分支；H-10/H-11及DEC批准仍受限。 |

**统计**：目标23→25（+2个有具体源码及正文位置的独立修复），已整改18→20（+2），未整改5→5（0）；审计文件36→36（0）、已完成限定26→26（0）、正在10→10（0）、未审0→0（0）。实际修改四份非台账技术文档、按原源码作数学/类型审查；不将文档修复当作生产软件测试通过，也不为增长已审计数而提前结案仍缺全面证据的DOC-010/013。

### 第12阶段：UpdateRate 比较状态写回语义的实质性整改及人类决策边界（2026-11-12）

**固定起点HEAD**：`b84dba184bd5b5ab3b1b5a5a453c1f0cda33a833`。起始36DOC：26已完成限定、10正在、0未审；25DAR：20已整改、5未整改。本轮按[AUDIT.md](AUDIT.md)只做GitHub源码只读/数学与文档实改，不新增生产C++、CTest、MAT写回读或批准。

**DAR-026 已修复的实际缺陷**：读取三份原`adaptive_matched_filter_test/data_generation`入口，确认其第一返回系数的拥有/写回不同：Initialization `agent_in_sphere_initialization_test.m:97–118`、Comparison `agent_in_sphere_adaptive_matched_filter_test.m:139–162` 的比较单步把新系数赋回`ohnekoeff/mitkoeff`供同帧内迭代使用；Update rates `agent_in_sphere_adaptive_matched_filter_many_parameter.m:134–182` 则在每帧旋转四组比较状态，按period 1/5/11/21以**旋转后、主单步前**的`akkoeff`刷新，然后 `[dumi mittrans mitrot error]=oneit(...)` 的`dumi`被弃置，**没有写回比较基线**，第182行唯一主求解才更新`akkoeff`。源比较返回`error`被其它调用覆盖，存档`mittransfehler/mitrotfehler`是另外计算的`normrelfehler(besttrans,mittrans)`及对应旋转角度，不能混为单步MSE。

**实际正文已改**：[Adaptive LLD §5.0.1](design/adaptive_mfa.md) v0.8加入三入口第一返回赋值矩阵、准确顺序和暂存状态；[Runner §7/§8.0.1](design/experiments.md) v0.9增加源比较不写回与判别性替身；[对象模型](OBJECT_MODEL.md) v0.8把策略/Session的拥有者写清；[SRS](REQUIREMENTS.md) v0.19更新原AC-F-ADAPT-003/EXP-003，而不更改任何REQ/AC/TC ID；[TEST_PLAN §2.7.1](TEST_PLAN.md)固定真实 MATLAB i=2..6 的时序和失败负例。对应提交：[ba06bea](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/ba06beab8ca05c4378e72338a2497d0ec8685c94)、[e43eb09](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/e43eb0952e3ef4ee6753b34c885970f030d5c003)、[959ac8e](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/959ac8e838e2c0458d6496cc4aefea658c24f390)、[cccd2f4](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/cccd2f47d1be3d561e7dd543bab49cc21369a287)、[e7d1ae4](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/e7d1ae48926741a406acee9adab336cb0abaea76)。

**固定替身的独立静态控制流复核**：源初始四组与主系数常数项都取1、旋转为I，令一步求解新系数`C+100`。逐帧模拟i=2..6，按源逻辑丢弃全部比较返回；period=1在i=2/3/5/6的输入为`[1,101,301,401]`，period=5为`[1,1,301,301]`，各帧主求解后A为`[101,201,401,501]`。**逻辑静态重放全部断言通过**；这不是运行原MATLAB、真实三乘五球谐/Session、数值L3或CTest。错误写回dumi会让i=3的period5变为101，先提交主再刷新会让i=5的period5变为401，故两种错误实现有可判别的负例。

**真正需要人类批准的事项**已逐项写入[TODO §2.1](TODO.md)，提交[e1b4826](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/e1b4826de5175af3003208a0e5de62bf4a27441c)：H-10其他模块生产编码、H-11代理实际构建/CTest/MAT写回读范围、DEC-008公共ABI与最终错误策略、DEC-005数值阈值、DEC-006性能预算、正式固定范围的BASELINE-SIGN与后续RELEASE-PR。**本轮纯文档审计无需用户立刻决定任何一项**；既有球谐材料/Eigen-only/不运行MATLAB-L1等决定不再重复申请。

**七项计数**：整改目标25→26（+1独立源级时序缺陷），已整改20→21（+1），未整改5→5（0）；36审计文档与26已完成限定/10正在/0未审均不变。六份非台账正文有实际技术/管理修订；综合DAR-001～005仍需真实生产代码、原生验证或批准，不能靠本次文档完成而关闭。代码/原MAT/项目构建从未在本轮执行。

### 第13阶段：Result/Status 构造伪成功修复、六项无限期延期、DEC-008决策说明（2026-11-12）

**起始HEAD**：`9aa02fd5c5a3ae55a4aa9d5d0a143d9010ff8742`；起始36文档26已完成限定、10正在、0未审；26DAR中21已整改、5未整改。本轮仍遵守[AUDIT.md](AUDIT.md)仅源代码只读审查与**文档真实修改**的权限，不修改生产C++/MATLAB/MAT、CMake/CTest，亦不使用未批准软件/性能测试。

**DAR-027 已解决的独立实质API缺陷**：旧[contracts.md `3](design/contracts.md)只给`std::variant<T,Failure<Code>>`和`Status`的`std::variant<monostate,Failure>`，**未禁止默认构造**；当T有默认构造时，`Result<T,E> r;`可令首`T`分支成功而未经历任何算法检查；`Status<E> st;`默认`monostate`成功更加确定。这与“错误不得伪成功”和可能未初始化Eigen数值的安全义务直接矛盾。现已实改[contracts.md](design/contracts.md) v0.8，`Result()/Status()=delete`、private`in_place_index`构造器、成功/失败显式工厂、删除公开assignment；同时明确工厂仍须验证值的数学不变量、异常构造/valueless不能凭文本保证完全消除。提交 [6e29211](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/6e2921167514dae2365db190fc82d77862e9d40f)、[7fbf03d](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/7fbf03d8a57a89845b8f68d672561467ff4a54ac)、[4531476](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/453147634085889330e8d13f00722845d7c59799)。[TEST_PLAN `2.12.1A](TEST_PLAN.md)补`Result<double,E>/Result<FlowField,E>/Status<E>`默认构造不可编译及正/负alternative、move-only、跨TU/异常等未来真实用例，提交[a24bf08](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/a24bf0864c92dfb85212f06d9d2c07de11e9b11a)；[SRS `附录AC-DC-DOMAIN-001](REQUIREMENTS.md)明确同一资格，提交[08089a3](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/08089a3ccb462686a350d1400e5db154553e3e1f)。**这只是设计缺陷已修，不是Result实现/编译/ABI、异常故障注入或CTest通过**。综合DAR-003/004仍未完成。

**用户新增六项无限期延期——只改变优先级，不假关目标**：[TODO `0](TODO.md)实际把**H-10、H-11、DEC-005、DEC-006、BASELINE-SIGN、RELEASE-PR**改为`DeferredIndefinitelyByOwner`，禁止在其他整改完成前催办或拿它们阻止文档修复，提交[a3550cf](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/a3550cf267ea6e948f88a3def9cc654e6f2211aa)。[AUDIT `5.1](AUDIT.md)也同步这条优先级规则，提交[7cf5b34](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/7cf5b348afdb43eb51a3fd4203233b6f2e0d8561)。延期**没有**扩大以前局部SH的代码授权、没有允许本轮生产构建/写MAT/合并；DAR-001～005仍为未整改、后续条件明列，不可将无限期延期当批准或彻底豁免。

**DEC-008 只解释、不代替用户决定**：[TODO `2.2](TODO.md)实际增加六子项：①跨版本二进制ABI与源码级API的边界；②失败`value()/error()`的异常政策；③Factory/variant valueless/move-only；④错误码与stage/exit根因；⑤JSONL字段/`inspect-mat`输出范围；⑥Session/RunContext/Manifest的失败状态。每项区分已选方向、A/B建议、未批准和需真实测试的义务，提交[150f098](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/150f098380c6d8345b14c48b637a7cd17e0aec5f)；[CLI LLD v0.9](design/cli.md)把“Reader真的完整读值”和“CLI是否应把所有double打印”分开，提交[42ce608](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/42ce6083067f9fab8f06882dd69b5b77ba364b45)。**DEC-008状态DecisionPending/NotApproved**，不能将文本中的推荐A当所有者已批准；现行详细说明不要求当即执行H-10/H-11/DEC-005/006。

**本阶段起止统计**：整改目标26→27（+1），已整改21→22（+1），未整改5→5（0）；审计文件36→36、已审26→26、正在10→10、未审0→0（均0）。真正的非台账正文变更集中在公共ABI/状态设计、TC/SRS与延期管理/DEC-008候选。66 TC原编号、36审计范围保持不变。无真实C++/CTest/完整L2-L4/数值容差/签署证据。

## 5. 各轮计数与变化

**第13阶段相对上一轮**：目标26→27（+1）、已整改21→22（+1）、未整改5→5（0）；36DOC/完成限定26/在审10/未审0均不变。实改公共合同及相应SRS/TEST_PLAN，并更新用户延期与DEC-008候选；仍没有软件测试或新批准。

**第12阶段相对上一轮**：整改目标25→26（+1），已整改20→21（+1），未整改5→5（0）；审计文件36、已审26、在审10、未审0全部保持。实际技术正文含Adaptive/Runner/SRS/OBJECT/TEST_PLAN，附独立静态状态回放且没有新的生产软件运行。

**第11阶段相对上一轮**：整改目标23→25（+2），已整改18→20（+2），未整改5→5（0）；审计文件36→36、已审计26→26、正在10→10、未审0→0，审计分母和状态全不变。四份技术正文有真实数学/枚举修订，未夸大测试。

**第10阶段相对上一轮**：本批实改4份核心技术文档并定向复读，目标21→23（+2）、已整改16→18（+2）、未整改5→5（0）；审计文件36→36、已完成限定26→26、正在10→10、未审0→0，全部不变。仅文档合同闭环，不是软件测试通过。

**第9阶段相对上一轮**：21个整改目标，已整改16、未整改5；相对第8阶段目标19→21（+2）、已整改14→16（+2）、未整改5→5（0）。审计36文件、完成26、正在10、未审0，四项与第8阶段均无变化；五份非台账正文已实际修复。

**第8阶段相对上一轮**：目标17→19（+2）、已整改12→14（+2）、未整改5→5（0）；文档36→36（0）、已完成限定26→26（0）、正在10→10（0）、未审0→0（0）。实改五份技术正文，并非只修改计数。

**第7阶段相对上一轮**：审计文件36→36（0），已完成限定25→26（+1），正在11→10（−1），未审0→0（0）；目标14→17（+3），已整改9→12（+3），未整改5→5（0）。本阶段实改4份技术文件，未凭历史3/3或文档修订声称真实软件通过。

**第6阶段相对上一轮**：文件36→36（0）、已审计24→25（+1）、正在9→11（+2）、未审3→0（−3）；整改目标11→14（+3）、已整改6→9（+3）、未整改5→5（0）。以上变化有原始MATLAB对照与五份非台账技术文档实改，而不是通过文件计数代替整改。

**第5阶段相对上一轮**：文档36→36、已审计24→24、正在审计9→9、未审3→3，全部0；整改目标10→11（+1），已整改5→6（+1），未整改5→5（0）。本阶段真实改5份技术正文，并非审计进度统计操作。

**第4阶段相对上一轮**：36→36（0），完成15→24（+9），正在11→9（−2），未审10→3（−7）；DAR9→10（+1），已整改4→5（+1），未整改5→5（0）。

**第3阶段相对上一轮**：审计文件总数36→36、已完成15→15、正在11→11、未审10→10（均0变化）；整改目标9→9、已整改4→4、未整改5→5（均0变化）。**目标数未变并不等于没整改：本轮已实际修复REQUIREMENTS/TEST_PLAN/DOCUMENT_BASELINE_REGISTER三份正文并取得固定源码复核结果**。由于五个复合DAR的最终条件涉及真实软件/正式签署，本轮不能造假关闭。

**第2阶段相对上一轮**：36份不变；完成限定审计6→15（+9），正在审计14→11（−3），未审计16→10（−6）；整改目标8→9（+1），已整改3→4（+1），未整改5→5（0）。五项复合阻塞未解除，不能以文本修复宣布正式基线完成。


**第1阶段（历史快照）**：审计对象36→36（0），完成限定审计0→6（+6），正在审计0→14（+14），未审计36→16（−20）；整改目标6→8（+2个实际独立文字冲突），已整改0→3（+3），未整改6→5（−1）。新登记的DAR-007、DAR-008独立于原六类综合阻塞，且均以具体文本矛盾与提交为依据；其余五个复合义务不能因上述局部措辞更正关闭。

| 阶段 | 文件总数 | 已完成限定审计 | 正在审计 | 未审计 | 整改目标 | 已整改 | 未整改 |
|---|---:|---:|---:|---:|---:|---:|---:|
| 第0阶段：用户意见录入、实际审计前 | 36 | 0 | 0 | 36 | 6 | 0 | 6 |
| 第1阶段：源文档限定审查与三项纯文档整改 | 36 | 6 | 14 | 16 | 8 | 3 | 5 |
| 第2阶段：追踪与历史状态核对 | 36 | 15 | 11 | 10 | 9 | 4 | 5 |
| 第3阶段：66项责任链与实际正文修复 | 36 | 15 | 11 | 10 | 9 | 4 | 5 |
| 第4阶段：现行政策纠偏与限定审计 | 36 | 24 | 9 | 3 | 10 | 5 | 5 |
| 第5阶段：数学/接口/分析/MAT安全正文整改 | 36 | 24 | 9 | 3 | 11 | 6 | 5 |
| 第6阶段：轨迹/采样/Bias/KvD源码合同整改 | 36 | 25 | 11 | 0 | 14 | 9 | 5 |
| 第7阶段：CLI/ABI/球内定义域合同整改 | 36 | 26 | 10 | 0 | 17 | 12 | 5 |
| 第8阶段：非对称求解器/Reader源身份整改 | 36 | 26 | 10 | 0 | 19 | 14 | 5 |
| 第9阶段：SRS追踪/公式修复/同提交基线身份 | 36 | 26 | 10 | 0 | 21 | 16 | 5 |
| 第10阶段：Bias理论参考/单轴绘图合同整改 | 36 | 26 | 10 | 0 | 23 | 18 | 5 |
| 第11阶段：误差指标与KvD错误码/参考状态修复 | 36 | 26 | 10 | 0 | 25 | 20 | 5 |
| 第12阶段：UpdateRate丢弃返回值/时序与人类批准边界 | 36 | 26 | 10 | 0 | 26 | 21 | 5 |
| 第13阶段：Result默认伪成功/无限期延期/DEC-008候选 | 36 | 26 | 10 | 0 | 27 | 22 | 5 |

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
