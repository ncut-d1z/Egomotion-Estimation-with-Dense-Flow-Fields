# 全工程文档规范审计进度与整改台账（AUDIT_PROGRESS）

> **状态：审计进度 v3.0，第二十四批完成docs/全目录64文件回退复核，36份正式文档均完成本轮限定审计；按所有者新口径，综合DAR-001～005转为无限期延期并不计未整改（总40/已整改35/未整改0/延期5），没有宣称软件测试或正式基线通过。第二十三批全目录复核首先发现并实修DAR-040：上轮DAR-038把无ExperimentKind的Writer错当能校验每入口save白名单，已将每入口集合门禁归Runner并保留Writer自身目标/内部同名与安全检查；第二十二批实修CurveSeries无法追踪跨K个运行的多MAT输入及.m Git blob与MAT SHA-256两类身份混淆；同步分析LLD/SRS/TEST_PLAN三份正文；第二十一批实修MAT Writer双变量名身份校验：明确MatDenseArray内部name与目标MAT文件基名一致性，新增两个有效白名单名字错配仍须预写拒绝的可判别负例并同步SRS/Runner/TEST_PLAN；第二十批实修Bias场景完整返回值构造与理想流Result未检查缺口，已以两视线固定uniform/normal和成功/失败路径补入采样LLD、Runner、TEST_PLAN；第十九批真实整改Bias噪声合成的未绑定coefficient实参与上/下层错误码反向依赖，纠正OBJECT_MODEL公共apply签名及CompositionRoot装配责任，补签名/噪声有符号分量/错误链/固定RNG负例；第十八批噪声模型互斥和协方差证据继续保留；六项延期及DEC-008已决定范围不变；仍按36文档/既有五项综合阻塞继续限定审计；仍是未冻结文档审计，不是软件验收。** 审计准则及用户逐文档意见见 [AUDIT.md](AUDIT.md)。固定仓库基准：`cb3129b1510f4ffc5fadeb94b8fd67bdff0d5e82`；本文件首先登记用户提供的36项判断和六类问题。**用户意见不是本轮已经执行的源码审计；状态与整改结果仅随实际读取和提交证据变更。**

## 1. 口径、状态和可复核方式

- **36个目标文档**：对应用户两张表的17个核心规范/专题和19个支撑/过程/证据文档。**2026-11-15全目录复核另读64实际文件（含这36正式文件和28过程/历史附件），不把后者混入36正式审计分母。**当前审计文件 `AUDIT.md` 已替换为新的全工程范围，但低阶球谐原版仍在[固定历史提交](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/cb3129b1510f4ffc5fadeb94b8fd67bdff0d5e82/docs/AUDIT.md)。新增本文件是过程台账，不在36个待审目标中重复计数。
- **文件审计状态**：`未审计` → `正在审计` → `已完成限定审计`。结束审计只代表用户给定维度与记录的核查范围已检查、缺口已明确登记，**不表示文档冻结/代码通过/缺口全部修复**。**无限期延期是DAR目标状态，不是文件审计状态**；本次限定文档复核已完成的文件不得仅凭未来软件/批准延期再计未审计。
- **整改状态（三态）**：`已整改`／`未整改`／`无限期延期`。第三态只属于需要已明确无限期推迟的生产实施、真实测试/性能/容差或所有者签署的综合目标，**单列延期，不计入未整改，也不能归入已整改**；软件与批准证据依旧缺失。正式审计仍依[AUDIT §5.2](AUDIT.md)；不得为了提高比例拆问题。
- **证据字段**：DOC编号、路径、初始用户判断、实际检查点、文件blob、问题ID、修订提交、剩余风险；对 66TC/98P/27DAG/L2/L3/L4 只记录有来源的数值，不伪造代码执行。
- **历史隔离**：`AUDIT_PROCESS.md`仍是SH-01～SH-06的低阶球谐专项理论整改记录，原97项全工程历史另有固定提交；本次新DAR不复用旧AF状态或承诺它们已消失。

## 2. 全36份审计对象（用户意见与当前实际审计状态分列）

| ID | docs/下相对路径 | 用户初始规范判断 | 首轮实际需核对的问题 | 审计状态 | 本轮证据／整改关联 |
|---|---|---|---|---|---|
| DOC-001 | `REQUIREMENTS.md` | 草案级部分符合 | AC→LLD章节身份/正式批准 | 已完成限定审计 | SRS v0.27：AC-F-AN-001补分析曲线所有输入来源与两种哈希身份不混淆；此前AC-F-MAT-004增加Writer两个名字一致、暂存前拒绝及Reader回读同名的验收判据；原AC-DC-NOISE-001对照原唯一IFlowNoiseModel::apply签名补coefficient显式传参、CompositionRoot正确绑定两Bias入口、Experiment与Numerical错误不反向依赖及替身验证；提交9bb48595，66REQ/TC保持未编译 |
| DOC-002 | `ARCHITECTURE.md` | 草案级部分符合 | 27拟议DAG与真实构建/ADR | 已完成限定审计 | HLD v0.10：设计图27箭头=25内部+2外部；根CMake仅低阶SH及3CTest，部署/DEC-006仍待 |
| DOC-003 | `OBJECT_MODEL.md` | 草案级部分符合 | 候选类/生命周期/签署 | 已完成限定审计 | 对象模型v1.2：修复类图IFlowNoiseModel::perturb(flow,rng)漏系数与唯一公共apply冲突；CompositionRoot注入正确采样器/噪声类，统一ComponentProportionalGaussianNoise名称且不虚构kind()判断，提交13c1d03c/26bc8580，真实装配集成仍待 |
| DOC-004 | `design/README.md` | 索引符合，非LLD | 实际Markdown个数与模块覆盖条目 | 已完成限定审计 | 14份真实目录Markdown=1索引+1跨模块LLD+9模块LLD+1专题LLD+2理论，10个覆盖行；提交3614b28 |
| DOC-005 | `design/contracts.md` | 草案级部分符合 | Result/ABI/ODR/异常线程/自包含 | 已完成限定审计 | contracts v1.3明确leaf Error{局部ErrorCode,const char*静态消息}/Result与公共NumericalErrorCode/Result不等价，禁止enum整数static_cast与局部bad_variant_access假冒科学Failure；未来跨层用完整ErrorFrame复制源code/message保真；提交a8d7b90，公共适配器未实施；2026-11-15按本次限定范围重新核对：Result/Status 禁默认构造、完整ErrorFrame与SH叶子接口边界，来源/证据身份与已结DAR无回退；数学全域、生产原生/CTest或正式批准仍不得据此宣称已完成 |
| DOC-006 | `design/cli.md` | 草案级部分符合 | CLI负例、RunId、JSONL、DEC-008 | 已完成限定审计 | cli v0.12：JSONL顶层error_domain在causes=[]也必须存在，Mat/Numerical InvalidShape直接报错不会碰撞，不从message或exit推断家族；提交ad52013，真实CLI未编写 |
| DOC-007 | `design/mat_io.md` | 草案级部分符合 | Reader/Writer、TOCTOU、写回读、L2 | 已完成限定审计 | mat_io v0.15：新增DAR-038严格Writer目标name与内部MatVariableInfo.name同一性、预暂存拒绝与固定别名冲突负例；此前§7.4.2限定inspect-mat仅读meta、保留业务Reader完整解码；§7.4纠正旧兼容单变量表的二次open指令；§7.5要求完整Mat→Analyzer/Runner包装链；进一步澄清压缩MAT元数据解析可能需解压必要头部字节，但**禁止数值载荷解码/显示**，提交b3c6793/a9a740e/36c1fa3/5fbf6fb，尚无真实matio/CTest；2026-11-15按本次限定范围重新核对：单句柄Reader、MAT双名称匹配和DAR-040每入口save分层，来源/证据身份与已结DAR无回退；数学全域、生产原生/CTest或正式批准仍不得据此宣称已完成 |
| DOC-008 | `design/sampling_noise.md` | 草案级部分符合 | 生产几何/RNG负例/CTest | 已完成限定审计 | sampling_noise v0.13：新增DAR-037的完整FlowSynthesisResult构造、逐阶段Result检查与可复核双视线固定值，二次实修DirectionSet/FlowField字段、0-based列访问与非有限门禁；原有实修generate_bias_flow遗漏coefficient、把ExperimentErrorCode下沉Simulation层及通过隐式函数猜几何模式；明确注入selected_sampler.sample(level)与selected_model.apply(flow,coefficient,rng)，错误只保留NumericalErrorCode并在上层包装；提交6dfcabed/7794781c/6b92e11c，无生产C++/CTest；2026-11-15按本次限定范围重新核对：固定3N高斯、显式coefficient、完整FlowSynthesisResult与N×6索引，来源/证据身份与已结DAR无回退；数学全域、生产原生/CTest或正式批准仍不得据此宣称已完成 |
| DOC-009 | `design/trajectory.md` | 草案级部分符合 | Sophus真实依赖及几何L3 | 已完成限定审计 | trajectory v0.6：在既有旋转尺度退化审阅外，补球外内向正根4/9、球面内外向及严格球内前置检查；固定分数推导复核，提交6dbecb5；Sophus/C++/L3仍未验收 |
| DOC-010 | `design/kvd.md` | 草案级部分符合 | 36轮数学非真实CTest/200轮 | 已完成限定审计 | KvD v0.10：对照原两个误差.m恢复非平方欧氏/角度精确公式、零真值假零残差和5组输入；§4撤销不存在的SingularGeometry/NonFinite错误码而统一公共enum；§5将36轮已生成Wolfram固定参考与未执行CTest分列；提交9c80e41/6fd3220，仍需原生实现及全面数学审查；2026-11-15按本次限定范围重新核对：两条KvD原source伪代码、旧t旋转更新、原36轮数学参考身份，来源/证据身份与已结DAR无回退；数学全域、生产原生/CTest或正式批准仍不得据此宣称已完成 |
| DOC-011 | `design/harmonics.md` | 草案级部分符合 | 低阶叶子有实测，公共ABI未冻结 | 已完成限定审计 | harmonics v0.16：实际读当前low_order.hpp/low_order_source_projection.cpp/test头与实现blob，对照8个局部ErrorCode、Result错态bad_variant_access，补适配责任及3项无一一公共同义项；纠正本节source_projection‘未编译’与固定5e522198下3/3 CTest证据矛盾；提交bc30242，公开接口与Bridge未实现；2026-11-15按本次限定范围重新核对：旧CS基/三旋转来源与固定5e522198局部SH3/3证据，来源/证据身份与已结DAR无回退；数学全域、生产原生/CTest或正式批准仍不得据此宣称已完成 |
| DOC-012 | `design/adaptive_mfa.md` | 草案级部分符合 | Session原子提交、病态输入和真实软件 | 已完成限定审计 | Adaptive v0.8：与三原始入口源blob逐行核对，Update rates在i=134–182先旋转、按周期刷新本帧主旧系数，比较第一返回写入dumi弃置，仅主返回提交；原比较error不用于保存角误差；固定替身5帧静态重放，提交ba06bea/8dd5dd2/ba3d8c4，Session/CTest仍待；2026-11-15按本次限定范围重新核对：近度均值负支、M非对称、UpdateRates系数返回弃置与Session失败，来源/证据身份与已结DAR无回退；数学全域、生产原生/CTest或正式批准仍不得据此宣称已完成 |
| DOC-013 | `design/experiments.md` | 草案级部分符合 | Runner/Writer/Manifest/Ctest | 已完成限定审计 | experiments v0.16：新增DAR-038写出内部/目标同名约束及回读身份门禁；此前DAR-037的理想流/噪声/完整场景返回类型区分和失败不提交要求；原Runner只验证有效参数/传播底层NumericalErrorCode，具体模型由CompositionRoot按ExperimentKind唯一装配；RNG中断可记录独立失败或必要时IncompleteRun，禁止仿真错误随意标SolverFailed；提交63ebbeb4/7b7f9df4/b85798d1，尚无Runner测试；2026-11-15按本次限定范围重新核对：CompositionRoot与Writer生存期、MAT每入口save、RunContext错误链，来源/证据身份与已结DAR无回退；数学全域、生产原生/CTest或正式批准仍不得据此宣称已完成 |
| DOC-014 | `design/analysis_plotting.md` | 草案级部分符合 | 分析L2/L3曲线、CTest/绘图隔离 | 已完成限定审计 | analysis v0.14：新增DAR-039每条曲线实际MAT输入列表、原.m源码Git blob与原MAT内容SHA-256独立身份及缺哈希不得冒充已验；此前Mat MissingVariable包装成Analysis错误时保留Mat原Frame并允许Runner继续push_front，无source_error_code单根因捷径；提交d2418c8，测试尚未运行；2026-11-15按本次限定范围重新核对：Bias K-run末次x与平均y、CurveSeries多MAT来源/可选绘图，来源/证据身份与已结DAR无回退；数学全域、生产原生/CTest或正式批准仍不得据此宣称已完成 |
| DOC-015 | `design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 专题草案级部分符合 | 旧版本CMake描述与最新低阶叶子验收区分 | 已完成限定审计 | 对照旧1/1说明与用户固定5e522198 3/3；仅低阶叶子，公共ABI仍未实现；提交4a62a9d |
| DOC-016 | `design/SH_DISCRETE_REFIT_CONSISTENCY.md` | 理论专题合格，非LLD | 不能替代模块LLD/测试/批准 | 已完成限定审计 | 已核专题定位与35项数学辅助检查，待独立逐式复算；2026-11-15按本次限定范围重新核对：离散G/D逐项算子、非幂等/负近度反例的数学专题适用域，来源/证据身份与已结DAR无回退；数学全域、生产原生/CTest或正式批准仍不得据此宣称已完成 |
| DOC-017 | `design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md` | 理论专题合格，非LLD | 不能替代Adaptive真实接口/实现 | 已完成限定审计 | 理论专题v0.2：从M2/M3的叉积矩阵推导M对称充要条件mean((mu-1)d)=0，证明均值1不保证对称；提交28c20f6/555eeea，尚未逐式独立重算本专题其它全部25项；2026-11-15按本次限定范围重新核对：负近度除旧s、M条件数与非对称/纯旋转退化的数学专题适用域，来源/证据身份与已结DAR无回退；数学全域、生产原生/CTest或正式批准仍不得据此宣称已完成 |
| DOC-018 | `TEST_PLAN.md` | 计划草案部分符合 | 66TC登记与真实运行证据区分 | 已完成限定审计 | TEST_PLAN §2.6补DAR-039两RunId三MAT来源+分别独立哈希及K=2固定数值负例；此前§2.9补DAR-038目标名与内部name两合法白名单值错配、空名的预写失败与正确名回读；此前§2.2.3补两视线精确近度、理想/含噪FlowField、全部场景输出、2 uniform/6 normal及失败中断；原§2.2.2补显式coefficient、唯一apply接口与无反向依赖，CompositionRoot对应采样器+噪声实现的level0 6/8条视线和18/24次normal对照及p_y=-1有符号分量反例；提交c2d470ae/488dbc40/cc4c6f4b，66TC不变且未原生验证 |
| DOC-019 | `PLAN.md` | WBS草案符合 | 98P、M0–M5、真实工期/性能 | 已完成限定审计 | PLAN R-02实改：论文缺失风险改为已取得IR96/IR98/Choi、三路线低阶SH固定证据与未完成公共接口/集成风险分列；提交6f2a426；不冒称27DAG/预算/新CTest已验收 |
| DOC-020 | `PROGRESS.md` | 符合证据登记 | 固定SHA与历史执行/新状态分离 | 已完成限定审计 | 本轮已核历史E-DOC/SH真实证据与旧97AF；正文修订b69e17c |
| DOC-021 | `AUDIT.md` | 原专项→本轮综合审计 | 已在初始提交转换为全工程文档规范 | 已完成限定审计 | AUDIT §5.1记录DEC-008六项设计均已决定、05数值打印属之前未经授权扩展必须撤回；仍不解除H-10/H-11等六项无限期延期；提交43eaf2e |
| DOC-022 | `AUDIT_PROCESS.md` | 球谐专项整改，不适用全工程 | 保留SH-01～06，引用原专项快照避免误链 | 已完成限定审计 | 原SH-01～06理论台账保持不变，增加新审计与旧SH历史互不重计的说明；提交569a6e7 |
| DOC-023 | `DOCUMENT_BASELINE_REGISTER.md` | 部分符合 | 36项固定历史SHA与当期HEAD/批准分开 | 已完成限定审计 | DOCUMENT_BASELINE_REGISTER §6.4新增2026-11-12同一固定提交5de2e9d的36文件全Git blob和审计三态；原历史快照保留，新注册blob7cb2c627；签署/依赖/真实数值仍缺，DAR-005仅实质推进未结案 |
| DOC-024 | `SOURCE_INVENTORY.md` | 静态来源追踪符合 | 45 .m/50函数、MAT未独立SHA | 已完成限定审计 | 核对45 .m/50函数声明的固定源清单及blob身份；未重算MAT SHA256 |
| DOC-025 | `MIGRATION_MATRIX.md` | 静态迁移追踪符合 | 45/50候选映射，不是代码实现 | 已完成限定审计 | 核对45/50静态候选迁移映射；不等于真实移植 |
| DOC-026 | `TODO.md` | 管理清单符合 | 历史审批与分阶段授权时间线 | 已完成限定审计 | TODO §2.2 F更新先成功创建新frame再splice_after、相容allocator/不抛/线性，并明确JSONL顶层error_domain不可缺；提交82ca1e3/05dc087，六无限期延期及DEC-008既定选择保持 |
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

**历史初始与旧阶段原二态口径（以下数字仅属历史，现行三态以本文件§7.3为准）**：第0阶段原36份均尚未审计、6个初始DAR均未整改；第1阶段是6份已完成限定审计、14份正在、16份未审及8项DAR中的3项已整改。**第2/3阶段历史状态为36份中15份已完成限定审计、11份正在审计、10份未审；9个DAR中4已整改、5未整改。第四阶段按下方真实审计复核为24份完成限定、9份正在、3份未审；10个DAR中5已整改、5未整改。第六阶段完成限定25份、正在11份、未审0份；14个DAR中9项文档整改完成、5项综合外部/软件条件未完成。第七阶段完成限定26份、正在10份、未审0份；17个DAR中12项文档问题已整改、5项软件/审批复合目标仍未整改。第八阶段仍有26份完成限定/10份正在/0份未审；19个DAR中14项纯文档缺陷已修复、5项综合软件及外部批准条件仍开放。第九阶段本轮实改SRS/TEST_PLAN/PLAN/历史决策/基线矩阵五份正文；36DOC三态不变，21个DAR中16项已修复、5项外部或软件条件仍开放。第十阶段已实改4份技术正文，23个DAR中18项纯文档已整改、5项仍需软件或正式批准；36DOC三态保持26/10/0。第十二阶段实改Adaptive、Runner、对象模型、SRS、TEST_PLAN、TODO六份非台账正文；26项DAR中21项纯文档已整改、5项综合软件/批准目标仍开放，36DOC三态保持26/10/0。** 限定审计仅核对该行明确说明的定位/证据/追踪范围，不等于每个AC、API、算法、签署与软件测试完成。

## 3. 整改目标（六项用户输入基线及新增独立事实冲突）

| 目标 | 涉及文件 | 发现/整改方向（待实际验证） | 完整验收条件 | 阻塞性质 | 状态 |
|---|---|---|---|---|---|
| DAR-001 | REQUIREMENTS.md；DOCUMENT_BASELINE_REGISTER.md | SRS AC→LLD路径/章节/版本/blob/批准身份目前不是正式闭合链 | 将现有导航与冻结义务明确分离，取得正式各方批准及固定身份再可结案 | 需批准／冻结 | 无限期延期 |
| DAR-002 | ARCHITECTURE.md；PLAN.md | 27条设计DAG、部署/性能预算和ADR容易与真实构建混淆 | 逐项标出设计/实测/批准三态；真实target/性能及DEC-006后才能宣布完成 | 需软件证据／批准 | 无限期延期 |
| DAR-003 | design/contracts.md；design/cli.md；design/mat_io.md；design/adaptive_mfa.md；design/harmonics.md | 大量公共接口/ABI、Session与错误合同仍是候选签名 | 先核正文是否注明未实现，正式代码/CTest/ABI与批准俱全才结案 | 需授权测试／批准 | 无限期延期 |
| DAR-004 | TEST_PLAN.md；PROGRESS.md；golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md | 66 TC的候选覆盖与球谐3/3固定SHA的真实执行必须分层 | 表头/现行段显式区分历史/执行/批准；完整66TC要真实软件证据 | 需软件证据／批准 | 无限期延期 |
| DAR-005 | DOCUMENT_BASELINE_REGISTER.md；REQUIREMENTS.md | 36文件版本矩阵是较早固定快照而非当前HEAD基线 | 补真实当前HEAD规范身份/范围/批准人/日期/例外后才允许签正式冻结 | 需批准／当前SHA | 无限期延期 |
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

| DAR-028 | design/contracts.md；TEST_PLAN.md；TODO.md；REQUIREMENTS.md | MatErrorCode与NumericalErrorCode均有InvalidShape，旧ErrorDetail的source_error_code只规定保存稳定名称，若使用不带家族的InvalidShape就丢失根因模块；多层包装可能再覆盖原stage/根因 | 实改候选内部根因标识为Family::Code，保留原stage和真实path/variable/RunId，并将两个同名错误的区分性固定TC及SRS联系明确；完整cause[]/JSONL拼写仍待DEC-008-04批准，无原生测试 | 纯文档跨层错误来源身份冲突 | 已整改 |
| DAR-029 | OBJECT_MODEL.md；design/experiments.md；TEST_PLAN.md | 对象模型旧表将Writer/结果目录整体称为RunContext独占，但Runner接口实际是非自有IResultWriter&且CompositionRoot负责寿命；与DEC-008-06的RunContext计数/Manifest责任不一致 | 实改CompositionRoot真正拥有Writer、RunContext借用IResultWriter且独占RunId/计数/Manifest；Runner完整成功门禁与Writer单文件Status分离，补八次聚合第5次失败及Manifest回读失败负例 | 纯文档资源与运行状态所有权冲突 | 已整改 |

| DAR-030 | design/cli.md；design/mat_io.md；REQUIREMENTS.md；TEST_PLAN.md；TODO.md；AUDIT.md | 用户原始需求仅为显示MAT文件数据结构，但先前助手擅自新增inspect-mat通过strict/compat读取数值、NumPy/pandas预览、全量转储及--mode/--format/--all等CLI参数，形成不真实的REQ/LLD/TC | 按DEC-008-05确认删除所有当前数值打印/预览设计，inspect-mat只调用Reader::inspect并可--variable筛选元数据，负例断言不能调用Mat_VarRead/业务Reader或打印元素；保留原业务readNamed/readFirstCompatible能力和历史审计撤回证据 | 纯文档用户需求越权扩展及CLI数据合同 | 已整改 |
| DAR-031 | design/contracts.md；design/experiments.md；design/mat_io.md；design/analysis_plotting.md；OBJECT_MODEL.md；REQUIREMENTS.md；TEST_PLAN.md；TODO.md | 旧source_error_code只留单个根因字符串，无法按所有者DEC-008-04要求保留Reader→Analyzer→Runner每层真正的包装历史，且DEC-008-06 RunContext/Manifest原只记录根因 | 已在统一ErrorDetail实改std::forward_list<ErrorFrame> causes，顶层Failure携外层Code、push_front存直接内层→最深根因，各帧保存domain/code/stage/message/路径及合法RunId；Reader/Analyzer/Runner/RunContext/JSONL/Manifest与TC完整同步，保证故障事件独立不伪造分支链 | 纯文档完整错误链与已决定运行状态跨模块合同 | 已整改 |

| DAR-032 | design/contracts.md；design/experiments.md；TODO.md；TEST_PLAN.md | DEC-008-04旧包装伪代码先move(inner.causes)再push_front新节点，push_front分配或ErrorFrame拷贝抛bad_alloc时内层历史已被搬走，不能保证已知链仍可追溯 | 实改先构造新ErrorFrame/外层字段并在独立新链emplace_front，成功后才用兼容allocator的std::forward_list::splice_after在新节点后接入原链；按C++17标准核不抛且整链线性复杂度；补既有TC的分配失败时inner链仍完整负例，明确最终Failure/日志持久化仍可失败 | 纯文档错误链异常安全及来源保真缺陷 | 已整改 |
| DAR-033 | design/contracts.md；design/cli.md；design/experiments.md；REQUIREMENTS.md；TEST_PLAN.md；TODO.md | 完整causes只保存内层包装，直接Mat/Numerical失败时causes=[]；原JSONL顶层只有error_code，MatErrorCode与NumericalErrorCode同名InvalidShape仍会混淆当前错误家族 | 实改顶层必需error_domain与error_code，由当前Failure<Code>的枚举家族生成，即使causes=[]仍标Mat/Numerical；causes有包装时保留全部原帧，不能凭消息/exit推断家族；SRS/TC补两同名且零包装的区别负例 | 纯文档JSONL顶层错误枚举家族遗漏 | 已整改 |

| DAR-034 | design/harmonics.md；design/contracts.md；OBJECT_MODEL.md；REQUIREMENTS.md；TEST_PLAN.md；真实include/egomotion/harmonics/low_order.hpp与source_projection.cpp | 低阶SH叶子已独立实现自有low_order::ErrorCode/Result，其错态访问std::bad_variant_access且包含NotProperRotation/ComplexResidue/InvalidMethod等公共NumericalErrorCode无无损对应值；旧文档未明确未来桥接责任，容易整数强转丢原始错误来源或把叶子局部行为冒充DEC-008已定公共Result；另SH LLD/TEST_PLAN局部source_projection段误写未编译而固定5e522198用户3/3实测已通过 | 实际核真实C++头与投影实现8个本地ErrorCode，增原局部family/code/message复制及std::forward_list<ErrorFrame>无损包装、分层bad_variant_access与公共logic_error边界；修LLD/TEST_PLAN真实固定SHA CTest范围和OBJECT_MODEL过时源/测试状态，SRS/既有TC映射并明确未实现桥接；不擅改C++或新建enum | 纯文档局部叶子到公共错误接口的无损适配缺口 | 已整改 |

| DAR-035 | design/sampling_noise.md；design/experiments.md；REQUIREMENTS.md；TEST_PLAN.md；原fixed_kugel_symmetric_perfectpoints_randomnear.m、fixed_propfehler_kugel_symmetric_perfectpoints_randomnear.m、nonpropfehler.m | 采样LLD旧伪代码对mean_length_noise与component_noise使用两个独立if，若双开会每分量计算两种噪声并消耗6N个normal，然而原任一MATLAB噪声路径仅一种模型、每点xyz共3次；同时未明确原三维独立高斯经球面切向投影后的分量相关性 | 真正改为由已有ExperimentKind派生的单选内部模型，不额外发明公共enum，非法选择在uniform/normal任何抽样之前拒绝；保留任意零噪/零分量仍3N normal；推导mean模型c² mean_norm²P及component模型c²Pdiag(p²)P，对d=(1,1,0)/√2,p=(1,-1,0),g=(1,0,0),c=1静态手算两种不同增量与xy协方差-1/-1/2；SRS与原TC添明确验证输入，不宣称实际C++通过 | 纯文档双噪声执行错误与切向协方差定义缺口 | 已整改 |

| DAR-036 | design/sampling_noise.md；design/experiments.md；OBJECT_MODEL.md；REQUIREMENTS.md；TEST_PLAN.md；ARCHITECTURE.md | 采样LLD候选generate_bias_flow使用未声明的coefficient且把ExperimentErrorCode::InvalidParameters直接作为仿真层返回，违反其Result<FlowField,NumericalErrorCode>及HLD egomotion_simulation不依赖egomotion_experiments的27条DAG；对象模型IFlowNoiseModel::perturb(flow,rng)还遗漏系数并与唯一已声明apply签名冲突；Runner最初还被要求对无kind()窄接口推断具体模型类型 | 实改采样伪代码显式系数及唯一已存在apply接口、不新建scale/NoiseKind公共API；ApplicationCompositionRoot按解析好的实验入口注入对应采样器/模型，Runner在任何RNG抽样前校验外部参数并按ExperimentErrorCode拒绝、Simulation只按NumericalErrorCode失败，Runner需保留完整ErrorFrame链；对象模型类图签名纠正、SRS和TC添加二入口配对、错误类型和有符号p_y数值负例；未实际建C++ | 纯文档噪声接口签名与架构反向依赖/装配合同冲突 | 已整改 |

| DAR-037 | design/sampling_noise.md；design/experiments.md；TEST_PLAN.md；原两个fixed_*_randomnear.m、erzeugeflussausnear.m | §2场景生成伪代码用`return noise_result.value() and original statistical fields`，却只有`FlowField`，没有构造§6承诺的`FlowSynthesisResult`全部七个字段；未显式注入FlowSynthesizer，且忽略其`Result<IdealFlowResult,NumericalErrorCode>`的失败分支。实现者照抄可能丢失`near_last`、`nearness`、理想流及三个统计量，把理想流计算失败误走成功噪声分支 | 实改伪代码使`synthesizer`显式注入、N次uniform先完成、检查采样/uniform/理想流/噪声四阶段Result，成功时建立完整FlowSynthesisResult；以两视线u=(1/2,1/12)、g=(0,1,0)/(0,0,1)确证near=(1,2)、总模长1/2、理想与含噪流不同及2+6 RNG调用，另列合成失败后不执行噪声的负例，联动Runner和原TC；不新建公共enum/新类，不执行C++ | 纯文档返回类型／错误路径合同遗漏 | 已整改 |

| DAR-038 | design/mat_io.md；design/experiments.md；REQUIREMENTS.md；TEST_PLAN.md；原MATLAB当前save与582文件单变量字典 | Writer公共候选同时接收`mat_variable_name`及`MatDenseArray::info().name`，旧文仅限制每个变量名在当前save白名单、非法路径和非覆盖，未验证两者相同；即使`xwerte`和`ywerte1`各合法，分别用于目标名/内部名也可构造文件名`xwerte.mat`、内部变量`ywerte1`的不真实成功文件 | Writer在暂存前检查已批准当前save字段与内部数据变量名非空逐字节相同、不可隐式改名；`(xwerte,xwerte)`正例与`(xwerte,ywerte1)`／空名负例纳入原TC，失败用已有OutputPathForbidden记录双名和RunId；同步SRS AC-F-MAT-004、Runner适配/回读和TEST_PLAN；非生产MAT/Writer实测 | 纯文档输出变量身份与源save语义缺口 | 已整改 |

| DAR-039 | design/analysis_plotting.md；REQUIREMENTS.md；TEST_PLAN.md；原graphs_bias_test.m与Adaptive分析入口 | 旧CurveSeries的SeriesProvenance只有一个original_mat_relative_path、一个original_mat_variable和模糊source_blob_sha，无法表示Bias的K份ywerte与最后一轮xwerte等多个实际数值来源；且source_blob_sha与L2原MAT SHA-256不同，原单字段既不能表述原.m Git blob又能表述MAT字节摘要 | 实际修订为每CurveSeries inputs[]逐个保存MAT路径/变量/k/角色/可核验SHA-256，原.m Git blob及平滑helper源码另列；无来源哈希标nullopt不宣称L2完成。K=2固定不同x例得x_last=[7,28,112]、ybar=[3,6,8]及三个参与MAT来源，原TC与SRS同步；未构建Analyzer/Reader或修改MAT | 纯文档多源聚合及不同哈希身份丢失 | 已整改 |

| DAR-040 | design/mat_io.md；design/experiments.md；OBJECT_MODEL.md；REQUIREMENTS.md；TEST_PLAN.md；原Comparison/UpdateRates的save清单 | 上轮DAR-038要求仅接收`(RunId,name,MatDenseArray)`的Writer验证“该ExperimentKind的有效save白名单”；但接口没有ExperimentKind/每运行策略，无法区别Comparison有效`rottransratio`与UpdateRates HistoricalOnly，同一设计把不可能兑现的义务下沉了 | 实改两层责任：掌握ExperimentKind的Runner在调用IResultWriter前验证完整当前save集，非法输出以既有DataMismatch报告且Writer调用数0；Writer仍提前核name==info.name、安全路径和原子非覆盖等自身可判定内容；Comparison/UpdateRates差异性负例、SRS与对象模型同步，禁止mat_io→experiments反向依赖。不新增公共接口/enum | 纯文档无入参却承诺按实验类型校验的错误分层 | 已整改 |

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

### 第14阶段：DEC-008两项决议落实及两处真实跨文档缺陷整改（2026-11-12）

**起始HEAD** `a3f46841f3a1405439b84337a1550488f2563902`：起始27目标已整改22/未整改5，36文件完成限定26/正在10/未审0。继续遵守六项无限期延期，不自行实施C++、编译/CTest、MAT写入或冻结发布。

- **DEC-008-01已决定**不承诺跨版本稳定二进制ABI，版本升级需要同构建链消费者共同重编；文档已改[contracts](design/contracts.md)，无ABI原生测试，也不推定全DEC-008批准。
- **DEC-008-02允许标准库异常，但仍待选择**std::bad_variant_access、std::logic_error及可选tryValue/tryError访问器；[TODO §2.2 D](TODO.md)给出实现例。03已详解禁默认构造、私有工厂、尽量不可变及ThrowOnMove/Copy/Ctor故障注入；04用同名InvalidShape具体解释enum家族/错误stage/退出码/根因；05解释inspect-mat及NumPy array2string/pandas DataFrame.to_string显示候选，不引入Python依赖且没有批准新CLI选项。
- **DEC-008-06已决定**原RunContext/Manifest建议，并真实写入[Runner LLD v0.10](design/experiments.md)与[SRS](REQUIREMENTS.md)及[TEST_PLAN](TEST_PLAN.md)：单调用失败Result只有Failure、RunContext拥有经验证部分结果/计数/Manifest、Writer仅单文件Status；未完成八次聚合不能改分母，发布后核验和Manifest未成功不能声称Complete。真实提交[f52c904](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/f52c904b63210f7fd05e8dee109f658d92a1fe4d)。
- **DAR-028独立缺陷已实修**：原`MatErrorCode::InvalidShape`和`NumericalErrorCode::InvalidShape`同名，裸source_error_code会丢失起源。现行[contracts v0.9](design/contracts.md)候选内部至少保留`Family::Code`与原stage，包装不得随意擦除根因，测试计划与SRS已增责任链；提交[df549e3](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/df549e35b41c86554be1da2bdde165032e347e73)。完整cause[]和JSONL协议仍是未定决策，不以文档整改代替最终审批。
- **DAR-029独立缺陷已实修**：原[OBJECT_MODEL](OBJECT_MODEL.md)把Writer称RunContext独占，与Runner LLD明确`IResultWriter&`非自有冲突；已区分CompositionRoot真实Writer所有者与RunContext结果/Manifest所有者；提交[3f62934](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/3f62934a393aa215acfc9d351e65e8af68bdbbb0)。

**七项统计**：整改目标27→29（+2真实纯文档缺陷），已整改22→24（+2），未整改5→5（0）；审计文件36→36（0）、完成限定26→26（0）、正在10→10（0）、未审0→0（0）。DAR-001～005仍受外部批准/软件执行条件限制且六延期事项没有恢复；DEC-008整体PartiallyDecided/NotFullyApproved。本文只登记真实修改与待测义务，不伪造生产CTest或MAT输出。
### 第15阶段：按所有者DEC-008-02～06真实整改接口链与撤销错误需求（2026-11-13）

**起始HEAD**：`0b1e6b9922aa00036a593323422f652df2b3c6ec`。起始29目标（24已整改、5未整改），36份审计文件（26限定完成、10正在、0未审）。用户确认DEC-008-02选择**方案B明确消息std::logic_error**、必要时才增C；03选择**推荐工厂/不公开赋值方案A**；04完整调用包装链使用C++17**std::forward_list<ErrorFrame>**（用户拼写`forword`应为`forward`）；05用户只要求**打印MAT结构**，不要求打印数据内容；06要求按04进一步修改文档。原六项无限期延期继续适用。

**DAR-030用户需求纠偏是真实删改而非新增功能**：以前的`inspect-mat`被助手擅加矩阵数值dump/NumPy/pandas预览/`--all`等选项，用户从未要求。**现行[CLI v0.11](design/cli.md)、[MAT I/O `7.4.2](design/mat_io.md)、[SRS v0.22](REQUIREMENTS.md)、[TEST_PLAN](TEST_PLAN.md)均已实改**：`inspect-mat --file PATH [--variable NAME]`只调用`MatResultReader::inspect`返回/筛选变量名、MAT class/dtype、rank/dims、元素数和可从头部确认的标志，**不调用Mat_VarRead/readNamed/readFirstCompatible、不打印任何数值元素**。`--mode/--format/--all/--precision/--rows/--cols`均不属于inspect-mat的命令设计，非法参数parse(2)；保留业务`MatResultReader::readNamed/readFirstCompatible`完整值读取能力而不混成元数据检查。原始需求不是NumPy/pandas数据打印，本轮已正式撤销该虚构需求，历史审计日志中的建议仅作**被撤回的历史事实**，不能据此实施。真实修订提交：[01fc772](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/01fc77233cb406db9e1ba728fe75b2ee2403e738)、[b3c6793](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/b3c6793c2e97f70b101d06fb8b5775a31f25f941)、[d056b65](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/d056b65f5f309e7865dd67fae68e3f795fbcc333)。额外修复MAT LLD旧`7.4单变量兼容表要求第二次公共readNamed打开路径的问题，必须继续遵守`7.4.1同一文件句柄来源资格。**没有运行真实matio、MATLAB或业务CLI/CTest**。

**DAR-031完整调用包装链是有API、Runner及测试合同实改的闭环**：旧[contracts v0.9](design/contracts.md)只有一个`std::optional<std::string> source_error_code`，即使带Family::Code也只能表达单个根因，无法满足新DEC-008-04。现已在[contracts v1.0](design/contracts.md)的`ErrorDetail`定义`std::forward_list<ErrorFrame> causes`，记录**当前Failure顶层+每次真实包装层**，`push_front`只插入直接内层、链首最近内层、链尾根因；每帧保留枚举家族/code/stage/message及已知上下文。比如`MatErrorCode::InvalidShape @ Decode`→`AnalysisErrorCode::ShapeMismatch @ Analyze`→`ExperimentErrorCode::DataMismatch @ Simulation`，最终顶层为Experiment，链顺序`[Analysis, Mat]`；CLI只序列化不额外虚加一层。对应同步[Runner v0.11 `6.1.2](design/experiments.md)、[MAT LLD `7.5](design/mat_io.md)、[Analysis LLD](design/analysis_plotting.md)、[对象模型](OBJECT_MODEL.md)、[SRS](REQUIREMENTS.md)、[TEST_PLAN](TEST_PLAN.md)、[TODO `2.2](TODO.md)，确保**DEC-008-06下RunContext每个失败事件/Manifest/JSONL保存完整链**而不是只保存根因；Manifest写入失败不能宣称链已持久化。提交：[4581f7c](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/4581f7c6982616adc4bbe8c143a0692f8b4ea2cf)、[c7e26bb](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/c7e26bb806edc34af485516b855e6d9c05bcd0c3)、[a9a740e](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/a9a740e982802a15a4ecd9a8efd99eba74760443)。**尚无真实C++链构造/故障注入/JSONL与Manifest持久化的原生证据**。

**DEC-008已选但未原生验收**：02在[contracts `3.A.2](design/contracts.md)明确`Result::value()`失败→`std::logic_error("Result::value(): called on a failure result")`，`error()`成功→明确对应消息，内部valueless另提示；03采用禁默认构造、private tagged success/failure工厂、无公开赋值/emplace的推荐方案，移动源不可当有效业务数值；方案C仅在确有非抛出访问器需要时增补，不提前扩API。01不承诺跨版本稳定ABI、06RunContext独立部分结果/Manifest仍有效；05仅MAT结构。用户已经给全DEC-008设计答案，**不再询问重复决策**，但代码/构建/测试/冻结状态仍是`ImplementationNotVerified`，不是宣称已经完成生产软件。

**七项指标**：整改目标29→31（+2项真实跨文件合同纠正），已整改24→26（+2），未整改5→5（0）；审计文件36→36（0）、已完成限定26→26（0）、正在审计10→10（0）、未审0→0（0）。所有新增目标均以技术正文与现有TC真实更新为依据，不通过改变文件统计制造工作量。 第15阶段末尾又对MAT压缩元数据与数值载荷作了必要区分：允许解析压缩头部所必需的内部读取，但`inspect-mat`仍不解码或显示数值元素；修复相关章节链接，提交[36c1fa3](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/36c1fa30f6b81b87535ac8dc3a7fb8474d94ccee)、[5fbf6fb](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/5fbf6fb623bee22822af49e35e605e82e26b5b5a)。DAR-001～005仍在六延期/需真实测试批准范围内，继续文档静态审计不因此中断。

### 第16阶段：DEC-008完整错误链异常安全与零包装错误家族的真正整改（2026-11-13）

**起始HEAD**：`646fbe48be798cad8b5282f714470ee3898d86fb`。起始36DOC（26限定已审、10在审、0未审），31个DAR（26已整改、5未整改）。本轮不修改生产C++、MATLAB/MAT、CMake/CTest；H-10/H-11/DEC-005/DEC-006/BASELINE-SIGN/RELEASE-PR依所有者要求仍无限期延期，DEC-008六项设计已定不再重复请示。

**DAR-032（真实异常安全设计缺陷）**：检查[contracts `3.0.2](design/contracts.md)发现旧顺序`outer.causes=move(inner.causes)`后才`push_front(make_frame(inner))`；一旦新节点分配或字符串构造抛`std::bad_alloc`，内层历史可能已经从inner搬走，调用者既无完整新Failure又无法可靠读取旧链。本轮在正文**实改**为：①先快照真实被包装的ErrorFrame、构造外层无链字段并成功`emplace_front`建立新节点，期间`inner.causes`保持不动；②成功后调用`outer.causes.splice_after(outer.causes.begin(),inner.causes)`，旧链节点不重新分配且置于新frame之后；③构造并传播外层Failure。两个forward_list须分配器相等，否则行为未定义；**C++17标准`splice_after`抛异常为Nothing，整链操作复杂度随旧链长度线性**，不能写成常数复杂度。已同步[Runner `6.1.2](design/experiments.md)、[TODO `2.2 F](TODO.md)与[TEST_PLAN `2.12.3](TEST_PLAN.md)；定点故障注入要求`inner.causes=[Mat]`时新frame分配失败，inner仍为`[Mat]`，正常再次包裹后链为`[Analysis,Mat]`。对应提交[ fcb05e9 ](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/fcb05e96fe81167acb33f34f046454c33b90ea6c)、[a990f0e](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/a990f0eeb345666d911ac4963d8373ccf8c58668)、[82ca1e3](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/82ca1e313b38bd19d959db45e4786af006cb65b8)和[63381d0](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/63381d0a12187d124ff01cd433c35221031f4efb)。**修正仅保证新节点准备阶段不会提前搬空原链，不声称后续Failure移动、日志/Manifest落盘永不失败或已执行原生CTest**。

**DAR-033（现行JSONL顶层家族歧义）**：DEC-008-04链的`causes`按定义**不包含当前最外层Failure**。若Reader或数值求解器直接报错且未再包装，`causes=[]`，原JSONL只有顶层裸`error_code=InvalidShape`，会使`MatErrorCode::InvalidShape`与`NumericalErrorCode::InvalidShape`仍不可区分，之前DAR-028的来源家族修订在零包装场景失效。真实整改：[contracts v1.2](design/contracts.md)及[CLI v0.12](design/cli.md)增加**顶层必需`error_domain`**，由当前`Failure<Code>`的enum家族确定，不从message/exit/stage猜；已有cause帧的domain/code照旧逐层保留；[SRS AC-NF-LOG-001](REQUIREMENTS.md)、[Runner](design/experiments.md)、[TEST_PLAN](TEST_PLAN.md)、[TODO](TODO.md)同步。固定负例：直接MAT`InvalidShape@Decode` → `error_domain=MatErrorCode,error_code=InvalidShape,causes=[]`，直接Numerical`InvalidShape@Solve` → `error_domain=NumericalErrorCode,error_code=InvalidShape,causes=[]`，两者JSONL必须不同。主要提交[183828f](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/183828f8c43e5fc80f069355148b0d84ef2f788e)、[ad52013](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/ad5201392c0e1380944ec032e0a56aee413fcafc)、[316e03d](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/316e03d4e5723e5cc3e6eb091106e2835a5b57ba)。**没有新公共枚举或已运行的JSONL软件测试**。

**独立受限核算**：用不涉及仓库C++或MAT写入的JavaScript状态机按“先准备新帧、后串接旧链”重放，合成新帧分配失败时原`[Mat]`不变，正常Analysis包裹得到`[Analysis,Mat]`，再包裹Runner得到`[Runner,Analysis,Mat]`；全部三个静态断言成立。**这不是C++ forward_list的真实异常/allocator注入结果**，该语义按C++17`splice_after`文档核对，仍待正式编译验证。

**本轮统计**：整改总数31→33（+2独立有真实正文改动的问题），已整改26→28（+2仅文档行为合同），未整改5→5（0）。审计文件36→36、完成限定26→26、在审10→10、未审0→0均不变；没有用增加文件审计数冒充工程实施。尚未解决的DAR-001～005由无限期延期的实施/原生验证或正式批准条件阻塞，但这不停止当前其它文档审计。**本轮无需人类立刻作出新决定**；已有六项延期和DEC-008设计决定均已记录，仍应在用户以后主动重新开放权限之前保持不授权实际生产软件执行。

### 第17阶段：低阶SH已实现叶子与拟议公共Result的真实类型边界整改（2026-11-14）

**起始HEAD**：`9b4a373c420dfcda3abbf9c85bcfbb2b8c7ddf36`。起始33整改目标（28已整改、5未整改），36文件（26已完成限定/10正在/0未审）。本轮遵照既有[AUDIT.md](AUDIT.md)仍只读已有生产C++/原MATLAB、修改现有文档，不运行C++/CTest、不写原MAT、不重新要求已无限期延期的H-10/H-11/DEC-005/006/基线/发布授权。

**DAR-034的真实技术缺陷及源码证据**：直接读取当前[`low_order.hpp`](../include/egomotion/harmonics/low_order.hpp) blob`8b2a8723d22fec0c7e544cab57952173c8daacfb`、[`low_order_source_projection.cpp`](../src/harmonics/low_order_source_projection.cpp) blob`7d3c7aadc90841c5c66d41ccebcfdfdca11306e5`与[专项CTest源](../tests/unit/test_low_order_source_projection.cpp)。该**已实现、仅获低阶专项授权**的`egomotion::harmonics::low_order`有**独立局部**`ErrorCode/Result<T>`（`Error{code,const char* message}`，`std::get`错误访问抛`std::bad_variant_access`），不能当作未实现的公共`Result<T,NumericalErrorCode>`。八个实际局部Code为`NonFiniteInput/NotUnitDirection/NotProperRotation/NonFiniteResult/ComplexResidue/InvalidMethod/InvalidSampleShape/ZeroProjectionNorm`；其中`NotProperRotation/ComplexResidue/InvalidMethod`在拟议公共`NumericalErrorCode`中**没有可无损直接替代的同义成员**。如果未来直接`static_cast`局部enum为公共enum，会产生错误类型甚至把有限ComplexResidue误称非有限；如果错误调用叶子`value()`导致bad_variant_access，不能将它当成普通数学失败抛给Runner。已真正修改[球谐详细设计v0.16 `2.1](design/harmonics.md)、[公共合同v1.3 `3.0.3](design/contracts.md)、[对象模型v1.1](OBJECT_MODEL.md)、[SRS v0.23 AC-NF-OOP-001](REQUIREMENTS.md)与[TEST_PLAN](TEST_PLAN.md)，规定未来显式检查叶子`hasValue()`、按值取得局部Error并拷贝`const char*`消息到`ErrorFrame.message`，保留`domain="egomotion::harmonics::low_order::ErrorCode"`与原局部code，Runner如需外层业务类型只用现有`ExperimentErrorCode::SolverFailed`真实包装并保留根因帧；未来直接返回NumericalErrorCode的公共Fitter必须先明确无损/无法无损映射策略，**不得自动添加公共enum或未经批准编程**。主要提交：[bc30242](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/bc302424986053714c6fc3582dc046bb0e33ae80)、[a8d7b90](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/a8d7b90d9e65ea7cf28290ed30d99703e909cd38)、[0bdaf0b](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/0bdaf0b75b659be49dc5f4c559e95f61bcfda152)、[ded6fcf](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/ded6fcf2b09f9308e378983d9679faab96c7b30c)。

**受限软件证据的另一个真矛盾已修**：[harmonics.md](design/harmonics.md) `2和[TEST_PLAN](TEST_PLAN.md) `2的旧段落说新`source_projection`源/CTest**尚未原生编译**，但同文/后文已经据用户固定`5e5221988cf093dbfddb88f2f65f06632460f3a1`记录低阶SH专项**3/3 Passed**、源投影专项76项检查。现正文纠正为“**固定5e522198低阶叶子已由用户原生验证，最新分支变化及未来公共Result适配器未运行/未验收**”，不把固定3/3当全66 TC PASS。旧对象模型关于1996/1998论文“未核对”也改为局部数学/材料已核、规划中的公共HarmonicRotator未建。测试计划继续使用原66 TC，并新增八种局部Code/完整ErrorFrame链/异常来源的**待授权测试设计**；本轮未执行新的C++编译或CTest、MAT/MATLAB/写回。

**不属于新的人类即时决策**：DEC-008已明确公共Result逻辑异常、工厂与forward_list，**不需要为了这次文档纠正重新选择它们**。若以后用户恢复H-10进行实际通用HarmonicFitter接口实施，针对局部`NotProperRotation/ComplexResidue/InvalidMethod`的**无损外层失败策略**可优先采用“Runner旧`SolverFailed`+根因Frame”方案；如果未来直接API被要求使用`NumericalErrorCode`且没有合义项，必须先在实现阶段给出受控接口方案/范围，不能未经批准假映射。六项无限期延期照旧。此轮其它可独立静态审计继续推进。

**七项计数**：整改目标33→34（+1独立技术接口错误），已整改28→29（+1纯文档合同），未整改5→5（0）；审计文件36→36（0）、已完成限定26→26（0）、正在10→10（0）、未审0→0（0）。本轮具体实改五份非台账技术/需求文档，**审计文件数量不变**；仍没有实际公共接口/桥接代码及其CTest，通过实证仅限历史固定低阶叶子SHA。

### 第18阶段：Bias两噪声模型互斥与三维高斯切向投影的实际修复（2026-11-14）

**起始HEAD**：`dc2f260be3aa2c245b033f60fd32efbca0328063`；起始34个整改目标（29已整改、5未整改），36份文档（26已完成限定审计、10在审、0未审）。本轮只读原MATLAB源并对照现行LLD，直接修改技术正文/SRS/测试计划而非调整审计文件数；H-10/H-11/DEC-005/DEC-006/BASELINE-SIGN/RELEASE-PR继续无限期延期，DEC-008六项设计决定不变。

**DAR-035源代码可证的真正缺陷**：旧[采样LLD `2](design/sampling_noise.md)`generate_bias_flow`伪代码以两个独立`if mean_length_noise`、`if component_noise`计算两种随机扰动，两个标志同时生效时将每个方向的高斯抽样从3次变成6次，且叠加两个原MATLAB**从未在同一场景同时调用**的误差模型。只读原`fixed_kugel_symmetric_perfectpoints_randomnear.m` blob `8829ceca395bba3d371db690471ec2c1f086d11b`、`fixed_propfehler_kugel_symmetric_perfectpoints_randomnear.m` blob `e3e01b4fdc0a68b6a14262c417b5dea69eb1e8c1`、`nonpropfehler.m` blob `c25d4743f5dc99b4825b738d822457d541813350`逐函数确认**每轮恰一种模型，按每点x/y/z顺序消耗3N次randn**，包括零噪声倍数、零尺度、单分量0时也不省略。

**实际整改**：已在[sampling_noise v0.11](design/sampling_noise.md)将独立双if改成由已有`ExperimentKind`派生的**单选内部`noise_kind`**，非法配置在距离uniform及Gaussian normal**之前**以既有`ExperimentErrorCode::InvalidParameters`拒绝，而非先取随机数再查参数；按点逐分量固定抽样顺序，先加完整三维Gaussian再`P_d=I-dd^T`投影，不能偷改为两维独立高斯。对应[Runner v0.13](design/experiments.md)明确BiasOccluded只用MeanMagnitude、BiasProportional只用ComponentProportional，一次场景不得两者叠加；[SRS v0.24](REQUIREMENTS.md)将互斥、精确3N消耗及投影公式写入原`AC-F-FLOW-003`；[TEST_PLAN `2.2.1](TEST_PLAN.md)增加真实区分性固定输入，**不新建任何TC/公共enum**。本轮提交：[142d322e](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/142d322ecaa4324972d72aa92d73713d3b8e5bad)、[8e94d4e9](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/8e94d4e9dceb1141b630040e39f40b40f6c3fd2c)、[b3d4b748](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/b3d4b7481ac269d70a9653fb63ddefaa9158a31a)、[7b476c2b](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/7b476c2b784e445c9eaa8899ef1fe9b8ffe6e99c)、[fa6e43a2](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/fa6e43a29991e5cfc0b079db06c0abf9637ba21c)。

**独立数学验证**：固定`N=1,d=(1,1,0)/√2,p=(1,-1,0),c=1,g=(1,0,0)`，`mean_norm=√2`。按原源逐分量扰动并投影，MeanMagnitude结果`(1/√2,-1/√2,0)`、ComponentProportional`(1/2,-1/2,0)`；两者互斥且任一模型**只抽三次normal**。令`g~N(0,I3)`，理论`Cov(mean)=c² mean_norm²P`、`Cov(component)=c²P diag(p_x²,p_y²,p_z²)P`，此方向`xy`非对角项分别严格为`-1`和`-1/2`，因此“投影后三维坐标依然独立”不成立。已用不接触仓库C++、MATLAB或MAT的JavaScript独立向量/矩阵算术复核`P`、两个投影增量及协方差；**这是数学/合同证据，不是原生Cpp/CTest、RNG统计实测或DEC-005正式容差批准**。不能因用单个固定方向反例就说真实6/8面Bias全部服从一模一样的协方差。

**人类干预**：本轮**没有新增需立即批准的选项**，两原脚本早已把实验入口/噪声模型唯一确定，不需用户选择新噪声类型。无权限扩大；将来真实C++/CTest属于明确无限期延期的H-10/H-11范围，未恢复前只继续文档/源码/数学审查。其它六项延期及已选DEC-008均保持。

**七项净变化**：整改目标34→35（+1可证实的实际技术合同修复），已整改29→30（+1），未整改5→5（0）；审计文件36→36（0），已完成限定26→26（0），正在10→10（0），未审0→0（0）。本轮四份非台账技术/需求文档均有真实diff、数学固定输入与源依据；**没有通过更改DOC状态或新增审计文件假造工作量**，综合DAR-001～005仍受未来真实软件/批准证据限制。

### 第19阶段：Bias噪声接口参数和组件依赖的实质性整改（2026-11-14）

**起始HEAD**：`db6e47c46f0c674ae4e6ba045baaa3f160ad007c`。起始35项整改目标，30已整改/5未整改；36审计文件26已完成限定/10正在/0未审。H-10、H-11、DEC-005/006、BASELINE-SIGN、RELEASE-PR仍无限期延期，DEC-008的6项设计决定不改变。本轮仍只读源和现有C++/HLD/LLD，实际修改受审文档，不执行生产C++/MATLAB/CTest/MAT写入。

**DAR-036真正可复核的工程合同错误**：上一轮只修“双噪声if”时发现`sampling_noise.md`候选`generate_bias_flow(level,truth_t,truth_omega,noise_kind,rng)`本来就漏掉源`fehlerprozent`必需`coefficient`，却在内部公式使用未定义量；之后一度把`ExperimentErrorCode::InvalidParameters`直接放进采样/仿真层伪代码，而`IFlowNoiseModel::apply`真实唯一候选签名是`Result<FlowField,NumericalErrorCode> apply(const FlowField&,double coefficient,IRandomSource&)`，按[ARCHITECTURE `4](ARCHITECTURE.md)27条拟议DAG`egomotion_simulation`不得依赖上层`egomotion_experiments`。此外[OBJECT_MODEL](OBJECT_MODEL.md)类图旧`perturb(flow,rng)`漏系数且重造第二API；[Runner](design/experiments.md)一度写“按ExperimentKind运行时识别IFlowNoiseModel具体类型”，但现有窄接口没有`kind()`可检测这一信息。上述相互冲突的候选若照抄到生产头，会出现未声明实参、错误依赖方向、重复接口或无法实现的类型判断，是真正的文档设计错误而非缺少测试这一形式状态。

**正文已经真实修改**：[采样LLD v0.12](design/sampling_noise.md)改为`generate_bias_flow(...,coefficient,selected_model,rng)`并直接使用**原唯一`selected_model.apply(perfect_flow,coefficient,rng)`**，不造`scale()`方法/新公共`NoiseKind`；[Runner v0.15](design/experiments.md)明确CompositionRoot按`ExperimentKind`装配两种采样器和唯一噪声类、Runner只验证外部参数与记录真实数值失败，Simulation只能产生`NumericalErrorCode`，需完整`ErrorFrame`链向上包装；[OBJECT_MODEL v1.2](OBJECT_MODEL.md)类图改`apply(flow,coefficient,rng)`并承认抽象引用无`kind()`不能运行时自识别，须以CompositionRoot替身和装配集成证明；[SRS v0.25 AC-DC-NOISE-001](REQUIREMENTS.md)把参数、合法二入口配对、类型边界变成可判定需求；[TEST_PLAN `2.2.2](TEST_PLAN.md)增加选型/数值错误/零RNG消费负例。主要提交：[6dfcabed](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/6dfcabedba7d8a62a4a8eefaa5b020906712a412)、[7794781c](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/7794781c72f6f11738d4f3e3d39d39e28e8e9047)、[13c1d03c](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/13c1d03c918a19f114be64e39b78894d1eed0863)、[9bb48595](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/9bb485953fdd452c6c0c21853f6e87c60ec66c0c)、[c2d470ae](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/c2d470ae26c6e8ab842fdc2182bbc3169b952691)。

**最终联动补查**：纠正对象模型`IFlowNoiseModel`具体类名与Runner不一致的问题，统一为`ComponentProportionalGaussianNoise`；采样伪代码同时显式接收已经绑定的`IDirectionSampler`，而不是依赖下层无法获知的ExperimentKind暗选完整/缺损八面体。TEST_PLAN固定`level=0`时Occluded对应6方向/18次normal、Proportional对应8方向/24次normal，且仍需用实际数值识别错误交换的噪声模型。提交[26bc8580](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/26bc858002e1d80f5b11c5be8d3dfdb5b31b2fcb)、[6b92e11c](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/6b92e11c5c1ec32246ebb4746c773fc9a337947f)、[cc4c6f4b](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/commit/cc4c6f4b2bd36afd378e80c7d22430dada65425a)。这一细化仍属于**DAR-036同一接口/依赖问题的实际整改**，不因多次修正文档重复新增DAR编号或冒充原生测试。

**固定符号与消耗反例追加**：沿用上一轮`d=(1,1,0)/√2,p=(1,-1,0),c=1`，本轮注入`g=(0,1,0)`专门检查负`p_y=-1`：平均模长模型投影为`(-1/√2,+1/√2,0)`，逐分量模型投影为`(+1/2,-1/2,0)`；误用`abs(p_y)`会反成`(-1/2,+1/2,0)`。每个有效模型仍消费三个normal，参数无效或相互矛盾在抽样**之前**拒绝，RNG消费为零。该例通过纯JavaScript确定性向量代数复核，不等于源MATLAB/真实C++/CTest已运行。没有因为修伪代码新增公共enum、运行时RTTI或另一个可变状态类。

**不需要人类立刻干预**：模型选择已由原Matlab入口及现行`ExperimentKind`决定，符合之前选定CompositionRoot注入原则；上下层错误枚举责任属于现有架构，不需要新标准。**建议继续文档/公式/独立输入审计**，禁止以“需编码测试”作为终止当前工作的理由；六项明确无限期推迟继续维持，若以后用户主动恢复权限，再据现行TEST_PLAN实测。

**七项统计**：整改目标35→36（+1个具体跨层接口/实参缺陷），已整改30→31（+1，限合同文本/测试候选），未整改5→5（0）；审计文件36→36、完成限定26→26、正在10→10、未审0→0（全部不变）。本轮有五份非台账技术/需求正文的实差异和真实源/架构依据，未通过DOC状态增长来表现工作量。DAR-001～005仍处于延期的执行/审批/真实验收义务，不因此关闭。

### 第20阶段：Bias完整场景返回对象与理想流失败闭环（2026-11-14）

**本轮开始状态**：固定HEAD `3d4605b4ffebf81baf5683f497e53283e341d94f`；36个整改目标/31已整改/5未整改，36份审计文件26完成限定/10正在/0未审。按现行AUDIT_PROCESS的历史SH专项已无待办，故本次实际从本文件全工程DAR台账继续第一轮整改，不将SH历史6/6误算为全项目全部通过。

**发现与真正修复的DAR-037**：对照只读原`fixed_kugel_symmetric_perfectpoints_randomnear.m`、`fixed_propfehler_kugel_symmetric_perfectpoints_randomnear.m`和`erzeugeflussausnear.m`，确认现行`generate_bias_flow`在`IFlowNoiseModel::apply`之后仅取`FlowField`，用无类型的“and original statistical fields”冒充包含`noisy/noiseless/nearness/near_last/mean_total_flow_magnitude/mean_translational_magnitude/mean_rotational_magnitude`的场景结果；还遗漏检查理想流synthesize的Result。此前DAR-036只涉及coefficient、枚举依赖及采样器/模型装配，未解决完整返回对象的构造。已**实际修改**[sampling_noise.md](design/sampling_noise.md)完整伪代码及错误门禁、[experiments.md](design/experiments.md)的Runner返回与不提交纪律、[TEST_PLAN §2.2.3](TEST_PLAN.md)的固定数学输入/失败替身。采样与理想流/噪声三种返回值严格区分；全部成功后才返回自拥有FlowSynthesisResult，不能错误覆盖原理想场。

**独立固定向量复算**：注入两方向d1=ez,d2=ex，t=ex,ω=0；两uniform依次1/2、1/12，来自源公式的nearness=(1,2)、near_last=2；原理想光流(-ex,0)，合成均值模长1/2、独立平移均值1/2及旋转0。仅使用平均模长高斯模型，按点xyz分别取g1=(0,1,0)、g2=(0,0,1)，投影后两行含噪光流分别(-1,1/2,0)、(0,0,1/2)，但无噪流不变；需2次uniform+6次normal。使用直接向量代数独立验证，**不是**源MATLAB运行或生产C++/CTest；两点仅为隔离的IDirectionSampler替身，不假称原固定6/8方向。另要求synthesize数值失败后不能调用noise或提交Complete，保留NumericalErrorCode及完整错误历史。

**授权与继续条件**：H-10、H-11、DEC-005、DEC-006、BASELINE-SIGN、RELEASE-PR六项无限期延期决定未变，DEC-008-01～06已定设计不重选。已完成一个真实文档合同缺陷的静态整改，但五项复合DAR-001～005依赖未授权业务代码/真实测试/正式批准，**不得虚构清零**，因此全工程第二轮审计按AUDIT.md的“全部目标已修复后”条件暂未触发。可继续在授权范围内审核其它公式/文档，不能因此承诺生产验收。

**推送后的真实回读与同DAR追加实修**：在确认首个提交四个文件均被远端回读后，又发现伪代码把`DirectionSet`包装值误写成有`rows()`成员，并以`perfect_flow[i,4:6]`混用MATLAB 1-based与C++ 0-based列下标；已改为`directions.directions.rows()`与`perfect_flow.observations.row(i).segment<3>(3)`，并增加总模长非有限值先拒绝、Noise成功返回N×6/方向保持不变的门禁；联动Runner/TEST_PLAN。仍属DAR-037同一场景结果值对象与返回资格修订，**不重复计算新整改目标**，不宣称实际运行C++。

**七项统计**：整改目标36→37（+1）、已整改31→32（+1）、未整改5→5（0）；审计文件36→36（0）、完成限定26→26（0）、正在10→10（0）、未审0→0（0）。

### 第21阶段：MAT Writer双变量名一致性与预写安全门禁（2026-11-14）

**本轮起点**：远端HEAD `d72a2120404f18c10241f106c713b5fbe4667503`；37个整改目标（32已整改、5未整改）；36份审计文件（26已完成限定审计、10正在审计、0未审计）。先核最新AUDIT_PROCESS历史SH-01～06全部已完成，现行全工程整改按AUDIT.md规定使用本文件，不把旧SH6/6重复计入全工程DAR。

**DAR-038的实质缺陷**：对照`MatDenseArray::info().name`与`RunMatWriter::saveNewVariable(run_id,mat_variable_name,value)`两个独立可输入字段，旧§7.2.1仅列Writer目标名白名单、路径和非覆盖要求，未保证`value.info().name == mat_variable_name`。故两个分别合法但不相同的`xwerte/ywerte1`可使文件基名和内部MAT变量不一致；该行为与源Bias`save([speicherstr 'xwerte'],'xwerte')`、单变量归档同名性质及AC-F-MAT-004不符。**实际修改**[MAT LLD §2.1/§7.1/§7.2.2/§8](design/mat_io.md)、[SRS AC-F-MAT-004](REQUIREMENTS.md)、[Runner输出字段责任](design/experiments.md)及[TEST_PLAN §2.9](TEST_PLAN.md)。固定输入同一`double[2×3]`，`target=xwerte,info.name=xwerte`应进入真实回读门禁；`target=xwerte,info.name=ywerte1`须提前拒绝，即使两个名字均在白名单；空内部名同样失败。错误暂采用既有`MatErrorCode::OutputPathForbidden`与实际RunId/双名上下文，不新增公共enum，不给Writer静默重命名权。未创建任何真实MAT、C++、CTest，不宣称已通过业务软件测试。

**剩余五项的实质阻塞**：DAR-001需求追踪/签署、002完整target/性能实测及DEC-006、003接口实现/ABI与真实测试、004全部66TC实际证据、005本次最新HEAD基线及人类冻结批准，仍属获用户无限期延期的H-10/H-11/DEC-005/DEC-006/BASELINE-SIGN/RELEASE-PR或其后续从属条件，不可能用纯文档修订合法关闭。DEC-008六项设计已经有决定，无需重问；继续在允许的文档审阅/真实静态整改范围内做迭代，但全工程正式第二轮审计尚不满足清零触发条件。

**七项计数**：整改目标37→38（+1）、已整改32→33（+1）、未整改5→5（0）；审计文件36→36、已完成限定26→26、正在10→10、未审0→0（全部0）。其中实际正文有四份非台账技术/需求/测试文件，而非仅修改台账统计。

### 第22阶段：分析曲线K-run多来源及MAT/SHA类别身份实改（2026-11-14）

**本阶段起点**：已推送`bf14eb475ee6f97b44c13995de76b51d8771bf9d`，整改38/33/5，审计文件36/26完成限定/10正在/0未审。仍只在文档审阅与静态数值夹具范围内工作，未执行业务MAT/源C++/CTest，六无限期延期审批不变。

**DAR-039实质缺陷**：对照原只读`graphs_bias_test.m`源中`xwerte`逐k覆盖、`ywerte3..6`跨k累加求平均，与旧[analysis_plotting.md §6](design/analysis_plotting.md)只有一个`original_mat_relative_path/variable`的`CurveSeries`矛盾；该结构会丢失K份y来源且无法另记最后一份x。原`source_blob_sha`又没定义为.m Git blob还是原MAT SHA-256，违背[contracts.md §4](design/contracts.md)对源码/数据哈希严格区分的口径。现**实际修订**分析LLD的`SeriesProvenance.inputs[]`、`MatInputProvenance`（逐个MAT path/variable/k/角色/可选SHA-256），并把原.m Git blob及平滑helper SHA单独保存；当前未取得的原MAT SHA-256必须明确为未核验。同步[SRS AC-F-AN-001](REQUIREMENTS.md)与[TEST_PLAN §2.6](TEST_PLAN.md)，没有制造不存在的历史MAT数字摘要或增加正式TC。

**可判定固定输入**：K=2，x^(1)=[6,24,96]、x^(2)=[7,28,112]，y3^(1)(:,1)=[2,4,6]、y3^(2)(:,1)=[4,8,10]；原源得到x_plot=x^(2)、ymean=[3,6,8]、j=3的(x,y)=(112,8)，参考100/sqrt(112)，每条相应曲线至少追溯run1/y3、run2/y3和run2/x三份MAT。x不相同只是**隔离的差异性负例**，不是原作者实际正常Bias输出已改变。未实际创建MAT文件、运行C++、MATLAB、Analyzer或正式测试。

**结案界限**：只关闭DAR-039的文档可审查合同缺口；36份原文件中的软件/全部Golden/正式批准未完成。DAR-001～005仍因既有长期延期、未实现公共接口/Reader/Writer及66TC证据不能合法关闭；不通过增审计文件数来冒充执行。

**七项计数**：整改目标38→39（+1）、已整改33→34（+1）、未整改5→5（0）；36份文档已审限定26、正在10、未审0保持不变。

### 第23阶段：全目录回退检查发现并修复Writer错误职责（2026-11-15）

**复核起点**：当前分支`209dd3c846f40ffcdff2fef62b2fa396d3459d90`。起点正式审计对象36项：26完成限定审计、10正在审计、0未审；全工程DAR39项，34已整改、5综合外部延期项按历史表暂记未整改。本阶段先复查全部docs/目录64文件（36正式审计对象 + 28辅助归档/审批/管理文件），按当前HEAD逐文件读取/静态链接与状态检查，另复读关键原MATLAB函数的固定blob，不以旧历史报告文字代替新代码证据。

**DAR-040实际回退发现**：上轮DAR-038在正确要求MAT目标变量名与内部变量名相同的同时，错误把`ExperimentKind`当前有效save集合的校验也交给没有该形参的Writer。上一轮文字修复因此在跨层责任上形成不能实际实现的承诺。真实修改[MAT LLD §7.2.1–7.2.3](design/mat_io.md)、[Runner LLD](design/experiments.md)、[OBJECT_MODEL](OBJECT_MODEL.md)、[SRS AC-F-MAT-004](REQUIREMENTS.md)、[TEST_PLAN](TEST_PLAN.md)；由已知ExperimentKind的Runner先按该入口save计划拒绝UpdateRates下历史`rottransratio`，Writer独立检查名称同一、路径和非覆盖。没有扩大接口、发明枚举或实现C++。区分性替身规定UpdateRates错误字段在Writer调用前拒绝，Comparison同名合法，不把Writer全局合法名与per-RunId合法字段混淆。

**本阶段仅数学/接口静态验收，不代表项目最终软件验收**。已形成DAR-040可复查正文和待测输入，未运行MATLAB/CTest/Writer。后续同轮还需要统一五项无限期延期的计数口径、记录全64份文件审阅证据并回读最终远端HEAD，当前不能把本阶段视为最终正式基线冻结。

## 5. 各轮计数与变化

**第24阶段：全64份文件重读与三态单列**：39→40个目标（+1真实DAR-040），34→35已整改（+1），未整改5→0（−5仅由延期单列），延期0→5（+5仅重新分类）；正式文件36不变，已完成限定26→36、正在10→0、未审0不变。新口径以本轮§8为准，历史各轮不回填。

**第23阶段本次实修**：新增DAR-040并落实五份技术/需求/测试正文，本阶段末尾正式回读后计算目标与状态，保留第22阶段历史原始计数，不预先声称完成待办软件验收。

**第22阶段相对上一轮**：整改目标38→39（+1）、已整改33→34（+1）、未整改5→5（0）；审计文件36、已完成限定26、正在10、未审0不变。已修分析LLD多来源与SHA身份、SRS/TEST_PLAN，未做软件测试。

**第21阶段相对上一轮**：整改目标37→38（+1）、已整改32→33（+1）、未整改5→5（0）；文件36、已完成限定26、正在10、未审0不变。已修改MAT Writer、SRS、Runner、TEST_PLAN实际合同；无软件运行。

**第20阶段相对上一轮**：整改目标36→37（+1）、已整改31→32（+1）、未整改5→5（0）；文件36、完成限定26、正在10、未审0不变。实际修复场景返回类型、理想流失败Result检查及完整七字段，跨采样/实验/测试三份正文同步，不是单改台账。

**第19阶段相对上一轮**：整改目标35→36（+1）、已整改30→31（+1）、未整改5→5（0）；审计文件36、完成限定26、正在10、未审0均不变。实际修复coefficient遗漏、向上错误码侵入Simulation、对象模型接口签名、CompositionRoot模型绑定/无类型识别及定点负例；只改设计正文。

**第18阶段相对上一轮**：整改目标34→35（+1）、已整改29→30（+1）、未整改5→5（0）；文档36、完成限定26、在审10、未审0全部不变。实际修订采样噪声算法/Runner/SRS/TEST_PLAN并用源数学值交叉检验，未执行生产软件。

**第17阶段相对上一轮**：整改目标33→34（+1）、已整改28→29（+1）、未整改5→5（0）；文件36、已完成限定26、正在10、未审0均未变。实改球谐LLD/公共合同/对象模型/SRS/测试计划，基于真实已实现低阶叶子源码而非仅增加审计数。

**第16阶段相对上一轮**：整改目标31→33（+2）、已整改26→28（+2）、未整改5→5（0）；审计文件36→36、已完成限定26→26、正在10→10、未审0→0，全部没有变化。实修完整错误链节点分配次序及JSONL顶层错误家族，更新跨模块设计和实际可判别测试计划。

**第15阶段相对上一轮**：目标29→31（+2），已整改24→26（+2），未整改5→5（0）；DOC36、完成限定26、正在10、未审0全部不变。本轮实改CLI/MAT/Result/Runner/对象/分析/SRS/TC/TODO/AUDIT正文、撤销虚构数值打印设计并同步完整错误单向链。

**第14阶段相对上一轮**：整改目标27→29（+2），已整改22→24（+2），未整改5→5（0）；36文件/已审26/在审10/未审0均不变。真正修改对象包括公共合同、对象模型、Runner、SRS、测试及DEC-008详细说明。

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
| 第14阶段：DEC-008决议/根因家族/Writer拥有者整改 | 36 | 26 | 10 | 0 | 29 | 24 | 5 |
| 第15阶段：结构检查纠偏/完整forward_list调用链 | 36 | 26 | 10 | 0 | 31 | 26 | 5 |
| 第16阶段：完整错误链异常安全/顶层error_domain | 36 | 26 | 10 | 0 | 33 | 28 | 5 |
| 第17阶段：SH叶子局部Result与公共错误链适配/证据状态纠偏 | 36 | 26 | 10 | 0 | 34 | 29 | 5 |
| 第18阶段：Bias噪声模型互斥/RNG及切向协方差整改 | 36 | 26 | 10 | 0 | 35 | 30 | 5 |
| 第19阶段：Bias噪声显式实参与跨模块依赖整改 | 36 | 26 | 10 | 0 | 36 | 31 | 5 |
| 第20阶段：Bias完整场景返回与理想流失败闭环整改 | 36 | 26 | 10 | 0 | 37 | 32 | 5 |
| 第21阶段：MAT双变量名一致与预写拒绝 | 36 | 26 | 10 | 0 | 38 | 33 | 5 |
| 第22阶段：分析曲线多MAT贡献/双哈希身份合同 | 36 | 26 | 10 | 0 | 39 | 34 | 5 |
| 第23阶段：DAR-040.Writer职责实改（旧二态） | 36 | 26 | 10 | 0 | 40 | 35 | 5 |
| 第24阶段：全目录64复核与延期单列（新三态） | 36 | 36 | 0 | 0 | 40 | 35 | 0 |

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

## 7. 2026-11-15 从头复核全部docs目录、正式36份状态结案与延期单列

**固定审查起点**：`209dd3c846f40ffcdff2fef62b2fa396d3459d90`，发现的DAR-040已通过后续提交`992cebaf340db7500055201c3a76e1eddb8f0c25`实质修复；本节下方64个文件的SHA采自后一个提交的完整Git tree（`63c2e25f57eaa9b37419513f27e0dfbc8a302692`），`truncated=false`。本节自身随提交会产生**新的Git blob**，下表`docs/AUDIT_PROGRESS.md`一行只能说明**本次取证时的旧blob**，不冒充本节提交后自身身份；每次最终身份以远端HEAD回读为准。

### 7.1 实际覆盖范围及哪些能力没有验证

- **目录全体64个实际文件已逐一从GitHub读取**，分为正式审计对象36份（SRS/HLD/LLD/过程/候选，其中17核心+19支撑）和28个辅助过程/审查快照/原始MAT字典/审批HTML。检查各自定位、首段/关键章节/历史时间与版本声明、相对链接有效性、可核验Git blob与当前源身份、现有关键REQ/AC/TC、已修DAR是否在活跃正文中回退。历史reviews的老“未整改/待实施”结论属于当时SHA的**历史观测**，不得拿来覆盖当前DAR；历史HAR/HTML不可冒充现行签署。
- **所有35个已结DAR（DAR-006～040）按其限定文档义务复查关键不回退锚点**，包括源球谐CS相位与固定叶子3/3、KvD旧t/仅一次归一、Adaptive负均值与不对称M/丢弃dumi、源MAT固定6/8面/3N调用、MAT Reader单句柄、inspect仅读元数据、Writer同名+实验级字段集合责任、Result默认构造与完整forward_list错误链、JSONL顶层error_domain、Bias末次x与平均y、CurveSeries多MAT来源及独立SHA类别。**审阅完成的是上述文档限定事项；所有Wolfram旧25/35项和全部数学全域证明没有本轮重新执行，亦未运行任何C++/CTest/MATLAB/Writer或修改原MAT。**
- 结构核验从同一Git tree实际得到原`.m`**45份**、原`.mat`**582份**、`docs/design/`**14份Markdown**（1索引+1跨模块LLD+9模块LLD+1专项LLD+2理论专题）；现行SRS的**66个唯一REQ**与TEST_PLAN的**66个唯一TC**按编号一一对应、无缺失。HLD中目标依赖Mermaid图**27条箭头=25内部+2外部**（总文档的其它流程图箭头不得混计）。这只是Git静态数量/追踪身份与图形核对，**不等于66TC已执行或582MAT做了本轮重新读取**。
- 关键原只读MATLAB源码在本轮固定blob下复读，包括`oneit_adaptiv_matched_filter.m`(`48e51bcd`)、`koeff_aus_near.m`(`5ff91a22`)、`near_aus_koeff.m`(`7373c437`)、`nonpropfehler.m`(`c25d4743`)、`franzeigen.m`(`6fe556fd`)、`norm_koenderink_iteration.m`(`d48048f3`)、`mue_norm_koenderink_iteration.m`(`9ffdac1b`)、两个fixed Bias入口(`8829ceca`/`e3e01b4f`)、`graphs_bias_test.m`(`112aa9ec`)及UpdateRates分析入口(`67cb6133`)。历史数学报告仍只作为报告，源源码没有被写入。
- **单份文档状态结案**：之前10份“正在审计”的数学/跨模块LLD已完成本次**限定范围（身份、接口职责、先前问题回退）**复核，均改为`已完成限定审计`。例如DOC-016/017的35/25项原历史Wolfram报告未重新执行，不声称全公式全输入100%正确；其余10份的业务/运行/批准证据仍由独立延期DAR和TEST_PLAN保留。不能因延期就把这些文件继续计为“正在/未审计”。

### 7.2 64份文件逐文件审查身份（含28份历史证据附件）

| 实际仓库路径 | 本次开始检查时的Git blob SHA（未必是本轮最终blob） | 审阅类别 | 本轮核查内容属性 |
|---|---|---|---|
| `docs/approval/DECISION_IMPLEMENTATION_NOTES.md` | `2dc89dfa563b35c81c747f89c2e0012ddc2ada20` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md` | `f2ece1cdd82641e9978513d06ff47f43c5a6ee9c` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/approval/HUMAN_APPROVAL_REQUEST.html` | `3524578fff58d4a3482d1e8f06e5a077820e6bab` | 辅助历史／配置／数据附件静态复核 | 审批HTML的历史身份 |
| `docs/ARCHITECTURE.md` | `3e2c749cf83122ca811a4dbb2255e2ec5cac61c2` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/AUDIT_PROCESS.md` | `5ec8a185d662275c0b8e21f5e5f4288b6236f24f` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/AUDIT_PROGRESS.md` | `a5d0582e887a342b045b4f46ed955b52c48f70f5` | 辅助历史／配置／数据附件静态复核 | 本台账旧blob，更新后再复核 |
| `docs/AUDIT.md` | `2e60fe610ce592aedd747d7697e97605a59642d5` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/decisions/DEC-004_HISTORICAL_MAT_FIELDS.md` | `063acaa2dc8e7524546fb85a985a7f8b8e860a88` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md` | `461db8f0ffdc4d11696607fbd2dfb34335a3c042` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md` | `0c708af126d031707e737136c075d29483667877` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md` | `8a21282d3b8730556992fac8cc1a15b31b7dacdc` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/design/adaptive_mfa.md` | `f2728c0236cb85651a0a621071710abbcda0fd6f` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/design/analysis_plotting.md` | `a2fd485311baeb0949f83ff5396c5ca89bb0eea1` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/design/cli.md` | `5b5c54cb42e5978e72274879d2eb4ccc58684077` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/design/contracts.md` | `23ea8d49b8467a8fba7ff69983a9fbee28fab43b` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/design/experiments.md` | `fbb633e809eabe37d8a5f2e54a5b2198d382ba97` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/design/harmonics.md` | `1fd08beb05d6a7c46fb36b57de46ace8bf1c8ca7` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/design/kvd.md` | `dc9e3bd5982676356d271a691b2e4d6a0724bcc2` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/design/mat_io.md` | `f05de12a3189cd140f979b858aa83eaec9ae4c75` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/design/README.md` | `18cbb3c153bf6c1eddae65e225deb2d8cf0b2186` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/design/sampling_noise.md` | `5ed2341dd717d1a46ff5a0368e36eb8a4566d51d` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/design/SH_DISCRETE_REFIT_CONSISTENCY.md` | `caba819f7fcabebdba84dfcefd0fc312e7215b00` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md` | `1f428091894e2899aa9225c7c9f99fe2cf5655da` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/design/trajectory.md` | `2f207fbdb0595b3419a4efcb02c382d70187cb81` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/DOCUMENT_BASELINE_REGISTER.md` | `7cb2c627e92674afb0307ac1396fe6d51365bc89` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/golden/analytic_golden_candidates.v1.json` | `1e6499eff3ce5b55503f8cd913b9f492629da499` | 正式36对象／限定内容复核 | JSON 候选状态 |
| `docs/golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | `df9612759549a9e9820b33a772540a80bbc90c3e` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | `fe1ad898090eb2d66ca99149dfeac189cf7c5696` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/golden/NO_MATLAB_EXECUTION_POLICY.md` | `5f2bc13f3fbf0636ac15f5dfe0c38611b80074be` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/golden/SH_PAPER_VERIFICATION_CASES.md` | `1ac44cba46e0d63b9f54f40fe18dbbc4279a627a` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/MAT_DATA_SCHEMA.md` | `a472f76798d265dbeb8422de32fe78214d3f7ed8` | 辅助历史／配置／数据附件静态复核 | 历史探针字典与来源边界 |
| `docs/MIGRATION_MATRIX.md` | `3d61d69f7962a29c8720528f7fa71628c7008331` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/OBJECT_MODEL.md` | `fe21548e4a3a625ad0982ffdc80d47393c0f7d02` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/PLAN.md` | `9e9e821241c3e1b6fe055726d1c303a56ccf01fb` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/PROGRESS.md` | `00aa3aa77f47081f1fd304adf3c7d1089d275fd7` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/REQUIREMENTS.md` | `f09c9a2c0784f7d503468d94bbf86d7177216124` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/reviews/AN_G_LEVEL0_FACE_ORDER_STATIC_2026-11-08.md` | `47e50aa5ff1db49e79206425bb218014578cd1f3` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/AN_G_RECURSION_INDEX_STATIC_2026-11-08.md` | `3d6c44bb6f5a7d1baf31c72dcfd476dd17f0e9e3` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/AN_G_STATIC_FORMULA_REVIEW_2026-11-08.md` | `e82f04464e09ebcc29024fdf203368ed4091ebb4` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/AN_G08_Z90_STATIC_CONVENTION_2026-11-08.md` | `903544444af25a0a1832822225b3f635ab4cf573` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md` | `ae44ae6ba782a9930b400ea860a50cc326a0cd3f` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md` | `1466ca9534a54cac6bf9b21a5b2bf42491d6e6a9` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md` | `36870185a6e51cf8d0b1ece0dc70d6277c3629a1` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/AUDIT_ANALYSIS_ERROR_MAPPING_SCAN_2026-11-08.md` | `803e095e70c56320267f22ee3cc9881450ff707a` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/AUDIT_ANALYSIS_SEGMENT_SMOOTHING_2026-11-08.md` | `e5a342d4a9db09a95c0ece6803fd82d94948962c` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/AUDIT_ANALYSIS_SEGMENT_SMOOTHING_CORRECTION_2026-11-08.md` | `fd30206e5fc7a29125d4b64946354d09b6156558` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/AUDIT_BIAS_XWERTE_LAST_RUN_STATIC_2026-11-08.md` | `b9653af0a0782b1a6fae1bbdd5e75c77257c338e` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md` | `9d01086ad3f56105e6e32b4911fcecfef8028b1f` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/AUDIT_FOUR_POLICY_DRIFTS_SCAN_2026-11-08.md` | `6f11a90c38ba7d1277002c495ee55189e1b60d00` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/AUDIT_FUNCTION_DECLARATIONS_AND_LEDGER_COMPACT_2026-11-08.md` | `a42a21e52313f3e7afc8b79421b65aa61d2fc703` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md` | `4a9d9698652caf3f3505bb877a0e1ba4db4cc154` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/AUDIT_SEVEN_MATH_DOCS_SCAN_2026-11-08.md` | `fbf2e0e8f7963b51e7037a42714b89320773ff1a` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/DOC_CONSISTENCY_RECONCILIATION_2026-11-06.md` | `2d440d1b3273196334150bd35bcb8aa7a9e7b2b2` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/DOCUMENT_REVIEW_AND_SH_DECLARATIONS_2026-11-07.md` | `580e98eae40a5378128b139ce531b6ded11c6c2e` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md` | `43446ec3e7ebe3d9b7f2447561f72d64ccbb5e95` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/LLD_CONTRACTS_TARGETS_TESTS_ALIGNMENT_2026-11-07.md` | `0852db6e7654b39698073626e9f92f3fadc8cf1d` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/LLD_MAT_IO_AND_CONSISTENCY_2026-11-07.md` | `d2b34d0d27bc2edec7969d28cf38429507095146` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/LOW_ORDER_SH_THREE_ROUTE_VERIFICATION_2026-11-09.md` | `70f7bcb66d515ab2fa52f1ae8652aac357bea2ef` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/OWNER_TECH_DECISIONS_ALIGNMENT_2026-11-07.md` | `2447abdf341d0ff4d05f6c332453f32b9d1ec047` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/SH_PAPERS_SOURCE_AUDIT_2026-11-09.md` | `141ce73789fce7e6257ad63bc844c7ce0a0f81d6` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/reviews/STATIC_TRACEABILITY_2026-11-06.md` | `2fa3002e1b45b1d3700b3df0dceb2d8ee7c9582c` | 辅助历史／配置／数据附件静态复核 | 固定历史评审，不作现行批准 |
| `docs/SOURCE_INVENTORY.md` | `9d72f7517e93f774c4b67338cc6b93f6723e7712` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/TEST_PLAN.md` | `a37343ccf9a8fedd3e899c6cf7d14e1d59e5ed7b` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |
| `docs/TODO.md` | `23a279aa17cf86e6a401344674719b4061f31b95` | 正式36对象／限定内容复核 | 标题/引用/相对链接/证据状态 |

以上SHA是**固定审查快照**，不覆盖本轮随后修订文件的新blob，也不构成最终批准或全文数学形式证明。若将来任一这些文件再变动，须以新HEAD重新运行相应DAR锚点审阅；不能继承本次“无回退”的结论。

### 7.3 当前真实统计（不把延期写成成功或未整改）

| 指标 | 2026-11-15本轮开始（原二态） | 本次审阅结束（新三态） | 数量差/解释 |
|---|---:|---:|---|
| 正式审计文件总数 | 36 | 36 | 0 |
| 已完成限定审计 | 26 | 36 | +10（10份实读限定核对） |
| 正在审计 | 10 | 0 | −10 |
| 未审计 | 0 | 0 | 0 |
| 整改目标总数 | 39 | 40 | +1：真实修复DAR-040 |
| 已整改 | 34 | 35 | +1：DAR-040 |
| **未整改（不含延期）** | 5（原二态把延期包含其中） | **0** | −5为**重新分类**，不是5项实际整改 |
| **无限期延期（单列）** | 0（当时未单列） | **5** | +5为**重新分类**，不是批准 |

恒等式：**40 = 35已整改 + 0当前未整改 + 5无限期延期**。可继续独立整改的活跃范围为**35/35个已整改，当前无未登记的可独立完成DAR**；这**不等于完整工程、全部数学、66TC、L2/L3/L4、公共接口、性能、基线或发布已经获验收**。未来新证据若发现真实回退，应重新打开对应原DAR或另登记独立实质问题，不凭“零未整改”永远禁止继续审计。

### 7.4 防回退规则（以后文档提交前适用）

对当前已结DAR的技术正文修改至少复核：当前MATLAB源/论文与Git blob身份、旧原型和候选公共签名、禁止MAT数值打印与不得把L1重新设门禁、数学CS相位/递推/不对称求解器的正确域、每种实验当前save与历史字段、N次uniform/3N次normal、Result/Status失败与forward_list完整包装、Writer/Runner权限层和MAT/数值SHA域、真实3/3仅固定旧HEAD且不覆盖66TC。需要原生代码或所有者签署者始终处于无限期延期，**状态不得直接变为已整改**。每次报告都需有具体文件diff、固定HEAD与未运行事项，不允许仅更新状态数字。

## 8. 结案纪律

针对文档措辞/数量错误可以完成纯文档整改；但AC冻结、正式接口ABI、真实C++/CTest/写回读、独立MAT SHA-256、DEC-005/006/008及所有者签字，不得通过编辑Markdown宣布完成。严格保持MATLAB/Octave/L1**永久非必需**。代码/原MAT修改、编译、运行及发布不属于本轮用户指示。
