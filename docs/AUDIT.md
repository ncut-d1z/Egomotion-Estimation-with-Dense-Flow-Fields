# 用户历轮修改意见汇总与文档审计台账（AUDIT）

> **文档性质**：本文件是对本项目历轮对话中用户提出的审查意见、详细 Markdown 评价、修改原则和已确定技术决定的**主题性归纳与后续审计台账**，不是逐字转录聊天记录，不替代 [SRS](REQUIREMENTS.md)、[HLD](ARCHITECTURE.md)、[LLD 索引](design/README.md)、[测试计划](TEST_PLAN.md) 或人类批准。明确区分：**用户已决定的规则**、**评审建议/尚需逐文件核查**、**待真实实现/测试的证据缺口**。本台账只针对 `ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields`，不引入用户其他仓库的要求。
>
> **本次审计范围（2026-11-07）**：根据最近一轮“仍有低优先级残留”的审查，开始**纯文档一致性修订**，仅处理低优先级事项；不触碰生产 C++、`origin/**`、MAT 数据、CMake/CTest、Bash、MATLAB/Octave，不修改历史审批 JSON，不自行签署正式基线。**“已审计”只表示当前文档内容/引用已检查，不表示规范冻结、数值等价或测试通过。**
>
> **执行续记（同日，后续一轮）**：上述“仅低优先级文档修订”是上一轮已完成的范围；本次用户要求**按 AUDIT 执行覆盖全部 40 文件的文档审计**，已单独形成 [完整审计记录](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md)（固定审计输入 SHA `4627cc1e48be9c43ad10b28fda646d65ee8ef01c`）。本轮以连接器只读结构/语义检查为主，**不自动修订高/中发现项**，不做算法逐式或真实执行；以下状态按实际检查深度更新。


## 零、发现问题与整改台账规则（强制）

**双台账、双状态，不可混用**：

- **文件审计进度**由本文件 §五的逐文件表维护，审计状态**仅允许**「空白、正在审计、已审计」；其含义是该文件相应检查范围内的审计深度，**不是问题整改状态，也不是测试/审批结果**。
- **具体问题整改进度**必须由 [AUDIT_PROCESS.md](AUDIT_PROCESS.md) 维护，整改状态**仅允许**「未整改、已整改」。**每当审查任一文档发现问题时，必须新增一条整改目标记录**；至少填写**该文档文件相对于仓库根的路径、具体整改目标、整改状态**，并建议记录唯一问题 ID、严重度、原句/章节和固定 Git SHA。
- **一个文件有多个整改目标时逐项登记**，不能用“整篇文档已处理”笼统代替。第一次发现问题时状态为「未整改」；**仅当实际修改对应文档，且在新的固定提交上复读确认目标确实修好时**，才允许改为「已整改」，应保留变更提交/复核证据。仅阅读、提出方案或在审查报告中声称计划修订，不算“已整改”。
- 在 [AUDIT_PROCESS.md](AUDIT_PROCESS.md) 完成登记的同时，可将该文件在本表置为「正在审计」以表示尚有问题或深层数学核验未办结；若一项已经整改，但其它问题或独立 L3 审计仍开放，文件**仍可为「正在审计」**。文件审计状态变化**不能自动更新**任何整改行。
- 原审批文档及旧 `reviews/` 是历史快照；发现其当时措辞与后续决定不同，须**先判断是否仅属历史语境**。不要无理由改写历史证据、SHA 或审批 JSON。真实需修的**现行**规范性冲突才作为整改事项登记。
- 每轮扫描应写明**扫描依据、固定输入 commit、覆盖文件、发现问题 ID、修订目标、是否已修订及复核范围**；在 [PROGRESS.md](PROGRESS.md) 登记审计证据，更新 [DOCUMENT_BASELINE_REGISTER.md](DOCUMENT_BASELINE_REGISTER.md) 的固定文件身份。只读 GitHub/静态源码审计≠ MATLAB/CMake/CTest/真实 L2/L3 数值测试；不得用审计进度制造验收或人类授权。

## 一、审计口径、优先级与单一来源

1. **区分文档类型**：`REQUIREMENTS.md` 是 SRS；`ARCHITECTURE.md` 是 HLD；`OBJECT_MODEL.md` 是 HLD 附件；`design/contracts.md` 是跨模块 LLD；`design/*.md` 的模块文件是 LLD；`PLAN.md` 为 WBS；`TEST_PLAN.md` 为 TC 与测试策略；`PROGRESS.md` 为动态证据登记；`DOCUMENT_BASELINE_REGISTER.md` 为精确版本/blob SHA 矩阵；`TODO.md` 仅管理可延期人类输入；`decisions/`、`golden/`、`reviews/` 为决策、参考与历史证据附件。**不能用 SRS/HLD 标准硬套模块 LLD，也不能拿 reviews/审批附件替代规范文档。**
2. **草案与基线严格分开**：有需求、公式、伪代码、类声明、文件路径、候选 CTest 不等于有生产实现；独立 matio 探针曾对 582/582 MAT `READ_OK` 不等于正式 Reader/Writer、写回读/CTest/算法已通过；GitHub 连接器静态断言不等于在真实主机运行 Bash/CMake/CTest；没有批准人、日期/时区、固定 blob SHA、证据/例外的文档**不得写“已冻结”**。
3. **不破坏原追踪义务**：已建立的 **66 个 REQ、66 个 TC、98 个 P 工作项、附录 8 个 REQ-DC、45 个原 .m 的分配映射**保持 ID 与关联；允许对草案措辞做一致性编辑，不能默默删掉功能、改变原算法或重编号；HLD 最新候选 target DAG 为 **27 条静态边**，不是已创建 CMake target 的证据。
4. **规范单一来源**：SRS 管可验证义务/AC；HLD/ADR 管组件/选型与 DAG；`contracts.md` 管跨模块错误和值合同；`cli.md` 管逐命令类型及参数；模块 LLD 管算法、实现接口候选、状态/所有权；TEST_PLAN §3 管 TC→实体/CTest→AC；PROGRESS 管真实执行证据；版本矩阵只记录固定 Git 身份。避免在各文档重复维护可变的完整合同、版本或 PASS 数。

## 二、历轮用户修改意见（重点归纳 Markdown 详细评审）

下表的“审计要求”是用户历轮意见的**归纳**；不代表所有事项在本轮执行，更不代表每一项已实现。对应的历史讨论、审批及修订证据可从 [reviews/](reviews/) 与 [PROGRESS.md](PROGRESS.md) 查阅。历史方案已被后来的所有者决定覆盖时，以**现行决定**优先；保留历史材料用于溯源而非恢复旧门禁。

| 编号 | 用户历轮审查/修改意见（按主题） | 审计要求及应核查的主要文档 | 当前约束/判定边界 |
|---|---|---|---|
| **AU-001** | **文档类型规范与可评审性**：多轮 Markdown 评价强调 SRS/HLD/LLD/测试计划/WBS/进度登记职责不同；结构通常已经合格，**基线成熟度不足**是主要矛盾 | SRS 的范围、利益相关方、REQ/AC/DEC/变更；HLD 的组件/部署/DAG/数据流/ADR；各 LLD 的签名、算法、错误、资源、测试映射；TEST_PLAN/PLAN/PROGRESS 各守其职 | 任何“结构符合”均不提升为“已冻结/可编码/已验收” |
| **AU-002** | **SRS 层级与实现选型下沉**：原 `REQ-DC-*` 不应混成外部功能需求；Eigen/Sophus/sdla/Result/JSONL 等选型不应在 SRS 正文复制实现细节 | `REQUIREMENTS.md` §4/附录 A、HLD ADR、`OBJECT_MODEL.md`、`contracts.md`；保留 8 REQ-DC 与原 AC/TC/P | SRS 可保留必要可验收约束、来源和 MIT 合规义务；具体代码方案由 HLD/LLD/所有者决定维护 |
| **AU-003** | **AC 引用身份/变更门禁**：AC 引用 LLD 章节只能作草案导航；冻结时应登记文件、章节、版本、blob SHA、commit SHA、批准人/日期/DEC/有效范围 | `REQUIREMENTS.md`、`DOCUMENT_BASELINE_REGISTER.md`、`TEST_PLAN.md` | 不以动态分支链接或文件存在代替审批签署 |
| **AU-004** | **HLD 逻辑、部署与质量场景**：组件/窄接口、27 边静态 target DAG、部署视图/Adaptive 时序/错误流/ADR/冻结清单须一致；环境快照与性能预算分离 | `ARCHITECTURE.md`、`OBJECT_MODEL.md`、`contracts.md`、`PLAN.md` | 27 条边只说明文档静态无环；未实际构建；Ubuntu VM 容量快照不等于 DEC-006 门槛 |
| **AU-005** | **对象模型 45 源文件映射**：模块职责、资源所有权、生命周期、状态机与每个原 .m 的预定 C++ 入口明确，不要设计全能求解器 | `OBJECT_MODEL.md`、各 LLD；注意 `HarmonicRotator::blocks`/`rotate` 与唯一球谐声明的名称一致 | 已建立声明≠已实现类；对象模型不是独立完整 LLD |
| **AU-006** | **MAT 数据事实和数据字典**：当前原作者 582 MAT 已由独立 matio 探针报告 v5/zlib/double、五类 shape 的 582/582 读取；必须分清格式元数据、源脚本有效 save、历史真实字段、来源 SHA、逐值和写回读 | `MAT_DATA_SCHEMA.md`、`mat_io.md`、`experiments.md`、`TEST_PLAN.md` | 探针 FNV64/source_sha ≠ SHA-256/函数 blob SHA；L2 全量原值、独立 SHA-256、正式 Reader/Writer/CTest 仍要实际验证 |
| **AU-007** | **MAT Reader/Writer 的详细接口**：用户详细 LLD 审查要求 `MatDenseArray` 自拥有 double/列主序，dims/class/dtype/complex/logical、复制移动、RAII、溢出/零长度、前后置及错误上下文 | `design/mat_io.md`、`design/contracts.md`、`OBJECT_MODEL.md`、`TEST_PLAN.md` | 第一版仅二维实数 dense double；不暗中支持 single/整数/complex/sparse；接口仍是候选 |
| **AU-008** | **MAT 严格/兼容读取与路径安全**：`readNamed` 严格匹配；`readFirstCompatible` 单变量可读，多变量没有已核实顺序时拒绝 `AmbiguousVariable`；不猜字母序；`openat/O_NOFOLLOW/O_EXCL`、TOCTOU、原子发布、独占 RunId、失败保留证据 | `mat_io.md`、`cli.md`、`contracts.md`、`TEST_PLAN.md` | `origin/**` 永远只读；损坏负例和 MAT 写回读只在将来获授权的隔离副本执行 |
| **AU-009** | **CLI 精确类型/错误协议**：SRS 概览的 `INTEGER/NUMBER` 与详细类型不一致；统一为 `UINT32/NONNEG_INT/FINITE_DOUBLE`；错误枚举/退出码以 contracts 为唯一来源 | `REQUIREMENTS.md` §6.1、`cli.md`、`TEST_PLAN.md` | CLI 仍未实现；负噪声等边界只能在具体接口/测试证据形成后验收 |
| **AU-010** | **永久删除 CLI `--output-root`**：取消任意输出根；固定仓库根 `output/<RunId>/`，可选 `--run-id`，无 ID 时用不消耗实验 RNG 的 OS 随机源；目录/文件不覆盖，不允许路径逃逸 | SRS/HLD 时序图、`cli.md`、`mat_io.md`、PLAN/TEST_PLAN/README | 不能重新引入输出根参数或受控任意根；读取/分析不产生新结果时不强行创建 RunId |
| **AU-011** | **跨模块 Result/Status 与 ErrorDetail**：选择轻量封装类（内部 C++17 variant）、单一 Failure/错误码、稳定 ErrorStage、JSONL；不能把失败伪装为默认值 | `contracts.md`、`cli.md`、`experiments.md`、`mat_io.md` | 错误传播保留 code、stage、run_id、变量、索引和底层 cause；JSONL 需合法转义，不强制大型第三方依赖 |
| **AU-012** | **原始数学与五类实验不可被重构抹平**：两类 Bias 与 Adaptive initialization/comparison/update-rates 应保留 seed、35/600 轨迹点、8 次 repetition、1/5/11/21 周期、RNG 消耗及原 save 语义 | `experiments.md`、`sampling_noise.md`、`adaptive_mfa.md`、`PLAN.md` | `RunOutcome` 单一返回概念，不恢复 `RunReport` 混用；独立 Config 优先，CLI 可以只负责分发 |
| **AU-013** | **Bias 细节和公平统计**：原 xwerte `6·4^i` 与完整球面 `8·4^i` 不一致不得偷偷改历史文件；随机 N uniform、3N normal 顺序不等同逐点交错，0 噪声仍消耗抽样 | `sampling_noise.md`、`experiments.md`、`TEST_PLAN.md` | 不要求 MATLAB 与 C++ 同 seed 产生相同随机流；固定注入 L3 验调用序列，L4 验统计 |
| **AU-014** | **KvD/Franz 迭代不可只验最终值**：分别记录 Franz 各轴模板响应/分母、Original/Modified a/b/c、每轮 near/t/ω、旧 t 算 ω 的时序、RMS/delta/grenze、至少 6/最多 200 次、停止原因与尺度 | `kvd.md`、SRS/TEST_PLAN 对应 AC、L3 方案 | 缺 L1 不阻塞；必须构建独立 L3 固定输入和实际 C++ trace 才能宣称覆盖特定顺序 |
| **AU-015** | **Adaptive M1–M4/Session 语义**：单步 6×6、rhs、解 z、`error=MSE` 而不是 RMS、near_new/C_new，主/对照状态与每帧/周期刷新规则应分开；失败步不提交主系数 | `adaptive_mfa.md`、`experiments.md`、`contracts.md` | 已选失败不提交；不能安全继续就终止当前连续序列；8 次聚合不足不得改除数或伪装完整 |
| **AU-016** | **Ivanic–Ruedenberg 球谐旋转**：必须以 1996 年原始论文和 **1998 年勘误**为数学依据，参考 Don Williamson 但**完全以 Eigen 独立重写且禁用 sdla**；R0/R3/R5、九个实基、m 顺序、相位、主动被动方向逐项验证 | `harmonics.md`、`SH_ROTATION_IVANIC_RUEDENBERG.md`、HLD/对象模型 | 用户将在开始实现时上传两份 PDF；现在仅允许**C++ 文件/头文件/命名空间/类/成员/方法声明**，不写递推函数体/生产代码 |
| **AU-017** | **Rodrigues/轨迹替代**：物理旋转已选 Sophus SO3d::exp，只替代实际使用的向量→旋转矩阵分支；(R^\top) 的局部运动、小角 eps、反向向量及球内近度须保留合同 | `trajectory.md`、`OBJECT_MODEL.md`、`design/README.md` | Sophus 路线已决定，不再写“第三方 Rodrigues 来源待审”；真实实现/边界测试尚缺 |
| **AU-018** | **DEC-004 历史 MAT 额外字段**：当代脚本有效 save 与历史 `alleadap*`/`rottransratio` 分开；新 Runner **仅保存当前字段**，旧文件只读、额外字段标 `HistoricalOnly`；分析新结果所需字段缺失则明确失败 | `DEC-004_HISTORICAL_MAT_FIELDS.md`、`experiments.md`、`analysis_plotting.md`、SRS/TEST_PLAN/PLAN | 无需先找到历史字段的真实归因，且**不得**补零、伪造、新增历史 MAT 字段 |
| **AU-019** | **分析/绘图的数学边界**：四个原始分析入口、从 L2 读取曲线/分段 `begin/end/zone`、平滑边界零值、绘图坐标/单位/有效样本必须能独立 L3 核对 | `analysis_plotting.md`、`TEST_PLAN.md`、`design/README.md` | 可选绘图后端未选**不阻塞核心数值分析**；不能只比图像像素，不能以 L2 最终值倒推不存在的逐帧状态 |
| **AU-020** | **Golden 等级及“不运行 MATLAB”永久决定**：L1 原 MATLAB 逐节点运行输出永久退出必要证据；L2 原 MAT 最终值；L3 独立解析/重实现；L4 统计；无 L1 不得被用作 AC/LLD 永久阻塞 | `golden/*`、SRS/LLD/TEST_PLAN/PROGRESS | 只读源码是公式/循环依据；不以 Octave 冒充原 MATLAB；无法证明原版 NaN/warning/RNG 逐位行为必须如实限定 |
| **AU-021** | **数值容差两层判据**：历史 `rtol=1e-10` 不再是现行候选；所有者选 `atol=0.01, rtol=0.1` **仅首轮宽松筛查**，零值/正交性/旋转、维度、MAT 数值/计数/状态另有严格要求 | `GOLDEN_TOLERANCE_PROPOSAL.md`、SRS/TEST_PLAN/PLAN | **最终 DEC-005 模块级 PASS 门槛**须待真实误差与独立参考，不得由 0.01/0.1 直接宣布 |
| **AU-022** | **测试计划追踪与真实性**：66 TC→拟议测试实体/CTest→AC，负例/RAII/Writer/CLI/数值 L3 的文件与 CTest 只是规划；Bash 现场静态脚本尚未实际执行 | `TEST_PLAN.md`、`scripts/check_docs_traceability.sh`、`PROGRESS.md` | GitHub 只读 JS 检查“0 错误”不能写成 Bash/CTest PASS；需要固定输入、环境、stdout/stderr/exit、源与参考 SHA |
| **AU-023** | **WBS 与证据状态**：98 个 P 包括前置依赖、里程碑 M0–M5、角色/退出条件和相应 REQ/TC/AC；缺负责人/日期/资源预算不得虚填 | `PLAN.md`、`PROGRESS.md`、`DOCUMENT_BASELINE_REGISTER.md` | 证据类别不是测试文件已创建；动态状态只回填 PROGRESS，历史审核以各自固定 Git SHA 解释 |
| **AU-024** | **版本/审批身份与维护成本**：SRS/HLD/LLD 可以有不同版本；需集中“文件路径、版本、状态、blob SHA、commit、批准人/时间/范围/例外”矩阵，精确引用而不复制多张动态表 | `DOCUMENT_BASELINE_REGISTER.md`、SRS/PLAN/TEST_PLAN/PROGRESS、`reviews/*` | 登记表自身是治理工件，不是已冻结基线；**PROGRESS 滚动记录的正式基线状态为“不适用”** |
| **AU-025** | **项目许可与第三方出处**：项目原创代码/文档 MIT 已由所有者选定，普通版权审查不再阻塞原创开发；实际引用上游文件保留其现有必要来源/notice | SRS 中来源/许可的可观察义务、HLD/ADR 中技术选型、README/LICENSE、PLAN | 不能把仓库 MIT 自动推广为重新许可他人原始 .m/MAT/论文；不再将版权选型上升为重复人工审批 |
| **AU-026** | **审批 HTML/JSON 的认知负担与界限**：历史审批页面被批评为卡片过多、术语不解释、信息架构不清，应提供合理预填参考；Rodrigues 与 SHRotate 替代详情和批准范围须可辨认 | `approval/HAR-*REVIEW_PACKAGE.md`、`approval/DECISION_IMPLEMENTATION_NOTES.md`、历史审批 HTML/JSON | 历史申请不是动态项目规则：H-10/H-11/运行权限和批准状态不能由 AI 自动提升，新的所有者技术决定覆盖冲突旧方向但**不回写原始审批 JSON** |
| **AU-027** | **可延期的人类干预集中管理**：只有论文 PDF、实际生产代码/测试授权、DEC-005/006 最终阈值、基线签署、PR 合并/发布、可选正式日程等需要所有者阶段性参与 | `TODO.md`、`PROGRESS.md`、`PLAN.md` | 当前纯文档阶段不应反复询问普通 C++ 类名/依赖和已决定的 DEC-004、MIT、Eigen、CLI；不进行后续代码/测试命令 |
| **AU-028** | **历轮“检查上一轮后是否修改干净”及优先级修订**：反复审查发现 CLI 旧输出参数、HLD/PLAN 旧 SH 路线、RunReport/RunOutcome、SRS 类型、MAT shape 列、PROGRESS 版本、文档章节引用等残留；每次修订必须继续只读检查 | 全部核心规范与 LLD、`reviews/*`、此 AUDIT 台账 | 保留历史报告的原结论/日期，不因现版本修订而倒改旧快照；高/中问题应分别立项，不在本次“仅低优先级”行动里越界 |

## 三、最近历轮详细 Markdown 评审形成的整改重点

### 3.1 原文性质与结构符合度

- 多轮用户评价均认为：**SRS/HLD/LLD 的文档类型和章节结构已基本合格，可以用于评审，但均不足以作为正式冻结/编码/验收基线**。不得把“目录齐全、公式完整、对照表完整、审查通过”写成“软件已实现、可编译、CTest 已通过”。
- SRS 审计检查编号/AC/TC、`REQ-I` 与 `REQ-DC` 分层、变更与 DEC/LLD 身份门禁；HLD 检查部署事实与预算、组件能力表和 27 边 DAG、时序/ADR/冻结清单；对象模型只作为 HLD 附件；模块 LLD 检查精确头文件、数据不变量、错误、资源生命周期、算法先后、L2/L3、测试、构建边界，不重复要求 SRS 干系人章节。
- TEST_PLAN 的 66 TC、SRS 的 66 REQ、PLAN 的 98 P、45 .m 映射需**联合**核对；PROGRESS 只登记证据，不应按 SRS/HLD 标准评定。实际 Bash 脚本不可因为连接器 JavaScript 等价检查而假称执行。

### 3.2 最容易回潮的跨文档冲突

- `--output-root` **已删除**，不应在 HLD 时序、SRS 正文、CLI/TEST_PLAN/WBS/Writer 签名恢复；固定 `output/<RunId>/`，不覆盖原始 MAT。
- **R5 实现来源不再是“移植 Don Williamson/sdla”**：现行选择是 1996 原文＋1998 勘误、Eigen-only，Don Williamson 仅参考；对象模型须引用 `HarmonicRotator::blocks`、`rotate` 的统一声明。
- 需求概览 `UINT32/NONNEG_INT/FINITE_DOUBLE` 与 `cli.md` 对齐；原本 `RunReport/RunOutcome` 只用 `RunOutcome`；源脚本预分配/save 与历史 MAT 实测类型/shape 分列；HLD §8 ADR 与 §9 冻结清单不混用。
- 新旧 MAT 字段分别报告：`with_errors` 历史 7 字段、现行 5；`many_parameter` 历史 6、现行 3，`rottransratio` 对新结果缺失时明确报告。
- 顶层项目 MIT 已选，Sophus 替代 Rodrigues 已选，**不要写“第三方 Rodrigues 来源待审”**；绘图后端是可选、其未选不阻塞核心数值部分。
- **版本表对滚动 PROGRESS 的“正式基线”列应为“不适用（滚动登记）”**；其它真正规范草案仍是“未冻结”。版本/SHA 在每轮有内容修改后需重新登记相应固定快照。

## 四、本轮低优先级纯文档一致性修订范围

| 低优先级问题 | 处理原则 | 状态记录 |
|---|---|---|
| LLD 索引把 Rodrigues/Sophus 来源写成“待审” | 改为 **Sophus 已选定**、实际库版本/小角/反向输入与真实 CTest 仍待验证 | **本轮已修订并待最终 GitHub 只读核对** |
| 可选绘图后端与必需 L2/L3 数值参考混成同一阻塞 | 明确**独立 L3 曲线/CTest 未完成**是核心验收缺口；绘图后端可选且不阻塞核心数值 | **本轮已修订并待最终 GitHub 只读核对** |
| 文档版本矩阵把 `PROGRESS.md` “正式基线”列写为“未冻结” | 改为 **不适用（滚动证据登记）**，保留实际文件 Git blob SHA 和历史快照语义 | **本轮按矩阵重新登记完成后核对** |
| SRS 来源/许可条目复制过多实现选型 | 保留 `REQ-D-SOURCE-002` / `REQ-NF-COPYRIGHT-001` 的可观察来源/许可义务及原 AC/TC；具体 Eigen/Sophus/Don Williamson 选型引用 HLD/ADR/所有者决定 | **本轮已做低优先级文字调整，仍保留 SRS 草案状态** |
| 受 SRS 文字修订影响的版本引用 | 更新 `PLAN.md`、`TEST_PLAN.md` 对应 SRS 文档版本，最终登记各相关 blob SHA | **本轮已修改，待固定快照登记** |

**本轮不处理**上一轮列出的高/中优先级残留，不冻结正式接口或验收门槛，不改历史审批/证据。跨文档检查发现新问题时按本台账状态另行安排；不能因为表中出现“已审计”而将其自动标为已修复或已验收。

## 五、逐文件审计进度表

**表格口径**：路径均相对于**仓库根目录**；仅允许三种状态文本：**空白、正在审计、已审计**。

- **空白**：纳入审计清单，但本轮未逐文件完成审查；不表示文件没有问题。
- **正在审计**：发现问题、已进行部分核对或属于下轮继续收敛的文件；不表示正在后台执行任务。
- **已审计**：**本轮已按指定的文档一致性范围核对其当前内容/修订**；不是功能通过、没有人类批准或正式基线冻结。
- 状态仅是**文档审计进度**，不是 TC/P/M 审计通过状态；如文件发生后续实质变化，应再次进入“正在审计”并留下对应固定 SHA/审查记录。

| 文件相对路径（仓库根） | 审计状态 | 本轮核查重点或下一轮审计事项 |
|---|---|---|
| `docs/AUDIT.md` | 已审计 | 28 组历史意见归纳已复核；本轮更新三态审计表、完整执行记录入口 |
| `docs/AUDIT_PROCESS.md` | 已审计 | 双状态整改台账已登记：原 AF-001～020 的 21 个原子目标维持已整改，下一轮 AF-021～025 先登记未整改、经文件修订和固定 SHA 复核后已整改；**当前共 26/26 项文档整改目标已整改**，不等于数值验收 |
| `docs/REQUIREMENTS.md` | 已审计 | SRS v0.12：AF-022 MAT v5/zlib 首版二维 dense real double 选型已明确，Reader/负例/正式基线仍未验收；原 66 REQ 保留 |
| `docs/PLAN.md` | 已审计 | 98 P 的引用/依赖静态核对完成；WBS 为计划非真实完成状态 |
| `docs/TEST_PLAN.md` | 已审计 | AF-005 已区分历史 H-01 draft 与后来选定的 0.01/0.1 首轮筛查；TC/真实执行仍未验收 |
| `docs/design/README.md` | 已审计 | LLD 单一来源、模块清单与低优先级修订复读完成，不意味着所有 LLD 已完成算法审计 |
| `docs/DOCUMENT_BASELINE_REGISTER.md` | 已审计 | 固定快照 28 项 blob SHA 已核；后续审计完成后需再登记新快照 |
| `docs/PROGRESS.md` | 已审计 | 证据记录与历史 SHA 语义已核，当前审计不生成 TC PASS |
| `docs/ARCHITECTURE.md` | 已审计 | HLD v0.8：AF-021 一般版权复核不作开发门禁、AF-023 MAT 首版范围已选，与早前 AF-017/019 合同一致；27 边仍为设计 DAG，未编译 |
| `docs/OBJECT_MODEL.md` | 已审计 | AF-016：状态机明确 Adaptive 失败不提交主系数，不能安全恢复则终止轨迹；对象模型 v0.5 文档合同已审，实际 Session/CTest 未验收 |
| `docs/design/contracts.md` | 已审计 | AF-004 已把实验失败策略标为已选择、实现及异常测试未验证；跨模块合同 v0.2 未冻结 |
| `docs/design/cli.md` | 已审计 | 逐命令类型、固定 output/<RunId>/、OS RunId 与错误单一来源已做静态核对，未实现/测试 |
| `docs/design/mat_io.md` | 已审计 | AF-018：0.01/0.1 已选仅首轮筛查，MAT 原值/类型/shape 仍严格；MAT I/O v0.8 文档合同已审，生产 Reader/Writer 未实现/测试 |
| `docs/design/kvd.md` | 正在审计 | AF-013/014：已更新首轮筛查和 H-13 历史/后续失败策略，定向文档复核通过；原 KvD 两版逐轮独立 L3、病态/停止条件仍待深审 |
| `docs/design/adaptive_mfa.md` | 正在审计 | 本次未改变 M1–M4、MSE、无效状态不提交；仍缺原源码逐式及独立 L3 核验 |
| `docs/design/harmonics.md` | 正在审计 | AF-002/003/012 已完成文字修订与静态复核；仍待 1996 原文/1998 勘误 PDF、R3/R5 全元素独立 L3 数学审计 |
| `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 已审计 | 仅声明级文件/命名空间/类方法及论文 DOI/勘误优先已审；PDF 未上传、无 Eigen 递推实现 |
| `docs/design/sampling_noise.md` | 正在审计 | AF-025 已区分 fixed_* 源 near_last 标量与 C++ 内部 nearness(N) 逐点记录；完整 i=1/2 方向、固定 RNG 的独立 L3/CTest 仍待审 |
| `docs/design/trajectory.md` | 正在审计 | AF-020：已明确 Rodrigues eps 返回单位阵与旋转向量叉积 1e−11 的不同门槛，文本复核通过；小角/反向/球外独立 L3 仍待审 |
| `docs/design/experiments.md` | 已审计 | AF-001 已统一固定仓库 output/、可选 RunId 与 Writer 二次验证；Runner v0.3 仍无真实实现 |
| `docs/design/analysis_plotting.md` | 正在审计 | AF-024 可选实际 N 对照图已从当前必要分析功能分离；仍在审计原始 begin/end/zone/平滑的独立 L3 数值与 CTest |
| `docs/decisions/DEC-004_HISTORICAL_MAT_FIELDS.md` | 已审计 | 现行 save/历史 HistoricalOnly/缺字段错误已与 Runner/分析设计对应，仍未实施 |
| `docs/decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md` | 已审计 | MIT、Eigen-only、无 sdla、固定输出、Result、数值筛查等已选设计依据已核 |
| `docs/golden/NO_MATLAB_EXECUTION_POLICY.md` | 已审计 | AF-006/007 已区分首轮筛查和最终 DEC-005，普通版权审查不阻塞开发；L1 永久非必需 |
| `docs/golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | 已审计 | L1 退出必要门禁、L2/L3/L4 适用范围与候选≠验收边界已核 |
| `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | 正在审计 | 首轮筛查与严格不变量口径已核；AN-G01～12、最终模块阈值及 L3 数值独立实测尚缺 |
| `docs/TODO.md` | 已审计 | PAPER-SH、H-10/11、DEC-005/006、基线签署与发布的人类延期安排已核 |
| `docs/MAT_DATA_SCHEMA.md` | 已审计 | 报告表 582 行唯一路径、5 种 shape 分布、dtype/dims×8/zlib/READ_OK 静态一致；**未重新读取 MAT** |
| `docs/approval/DECISION_IMPLEMENTATION_NOTES.md` | 已审计 | 历史 H-01 draft、H-10/11 deferred 与当前决定的时间边界已核；不改原审批 |
| `docs/approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md` | 已审计 | 仅对历史时间点及 AH-001 旧 L1 用语做审计；不可把旧文本当现行验收合同 |
| `docs/reviews/STATIC_TRACEABILITY_2026-11-06.md` | 已审计 | 历史 26 条 HLD 边及固定 SHA 已核为旧快照，不替代当前 27 条 |
| `docs/reviews/DOC_CONSISTENCY_RECONCILIATION_2026-11-06.md` | 已审计 | 历史 v0.3/v0.4、旧 CLI 选项是当时记录，不能当现行约束 |
| `docs/reviews/LLD_MAT_IO_AND_CONSISTENCY_2026-11-07.md` | 已审计 | 历史 mat_io v0.5 与 26 边的固定 SHA 有明确边界 |
| `docs/reviews/LLD_CONTRACTS_TARGETS_TESTS_ALIGNMENT_2026-11-07.md` | 已审计 | 旧 Result/运行器候选与 HLD 26 边属于旧检查点 |
| `docs/reviews/OWNER_TECH_DECISIONS_ALIGNMENT_2026-11-07.md` | 已审计 | MIT/Eigen/固定输出的历史验证范围已核，当前未编译 |
| `docs/reviews/DOCUMENT_REVIEW_AND_SH_DECLARATIONS_2026-11-07.md` | 已审计 | 球谐仅声明级设计、历史静态验证 SHA 已核，不当成已实现 |
| `docs/reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md` | 已审计 | 原初 AF-001～012 的旧审计输入/当时未整改状态已核，不反向覆盖后续复核 |
| `docs/reviews/AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md` | 已审计 | AF-001～012 的 12/12 定向文档修订有固定历史复核快照，非数值通过 |
| `docs/reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md` | 已审计 | AF-013～020 的 8/8 文档问题已在后续固定 SHA 限定复核，数学 L3 未覆盖 |
| `docs/reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md` | 已审计 | 本轮 AF-021～025 先审计、固定 SHA、提出五项未整改目标的只读扫描报告 |
| `docs/reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md` | 已审计 | 本轮 AF-021～025 的 5/5 文档修订及定向静态复核报告，原始数据和生产代码未变 |
| `AGENTS.md` | 已审计 | 项目只读原始数据、永久无需 MATLAB/L1、阶段运行权限及禁止改写历史的规则已核 |
| `README.md` | 已审计 | MIT、Ivanic 原文＋勘误、Eigen-only/禁 sdla、固定输出根与探针证据边界已核 |
| `LICENSE` | 已审计 | 根 MIT 文本及自有作品适用范围已核，不推导原作者数据再授权 |
| `.gitignore` | 已审计 | 确认含 /output/ 排除规则；该规则不能替代 Writer 文件系统安全 |
| `scripts/check_docs_traceability.sh` | 已审计 | 只读检查器源码解析：REQ/TC/P、45 .m、HLD DAG 的 AWK 检查路径已核；Bash **未执行** |

## 六、审计闭环与完成标准

1. 每次审计前确定**目标分支与固定 Git commit SHA**，从 GitHub 读取实际文件，不凭过时的文档副本判断；检查 REQ↔AC↔TC↔P、版本/章节/相对链接、HLD DAG、源数据身份和用户决定。
2. 每条问题应登记**路径、章节/原句摘录、发现原因、修改方案、优先级、是否已修订、固定 SHA**；在 PROGRESS 或后续固定范围 reviews 留证。不在 AUDIT 中把描述性计划标记为 CTest PASS。
3. **纯文档阶段**可以补充球谐 C++17 头文件名、路径、命名空间、结构体/类/成员/方法/静态函数声明，但**不得提供递推具体实现或创建生产代码文件**；其它项目所有者授权的运行边界仍遵循 [TODO.md](TODO.md)。
4. 只有在该文件完成所列范围核对且本轮修订已复读确认后，才可将该行改为“已审计”；如只完成关键字扫描则保持“正在审计”或“空白”。**文件审计完成不意味着文档冻结，更不意味着 REQ/AC/TC/P 已验收。**
5. 本轮**只清理低优先级**；如发现高/中优先级语义问题，明确列入下一轮，**不借本轮授权擅自扩大到实际代码/编译/测试**。完成本轮后按固定快照更新 [DOCUMENT_BASELINE_REGISTER.md](DOCUMENT_BASELINE_REGISTER.md) 的各文件 Git blob SHA，并在 [PROGRESS.md](PROGRESS.md) 登记审计证据、范围和“不能推断什么”。

**当前结论**：用户历轮意见已归入本台账作为后续逐文件审计依据；具体修改应由本轮提交差异和固定 SHA 验证。所有核心 SRS/HLD/LLD 与测试计划仍是**评审草案、未冻结/未验收**，没有运行 MATLAB/Octave、Bash、CMake、CTest 或任何 MAT 写入/数值实验。

## 七、完整文档审计执行结果（2026-11-07，后续审计轮）

**审计输入的固定 Git commit**：`4627cc1e48be9c43ad10b28fda646d65ee8ef01c`。审计报告：[FULL_DOCUMENT_AUDIT_2026-11-07.md](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md)。审计报告和本文件的后续提交不是该固定输入 SHA 的一部分。

- **覆盖**：40 个路径均检查文件身份、Markdown 链接/围栏、类型/版本/决策边界。共有 **452 条本地 Markdown 相对引用，未发现缺失目标**；这不代表每个链接的标题锚点均单独渲染验证。
- **静态合同/追踪**：66 REQ、66 TC、98 P、附录 8 个 REQ-DC、45 .m（SRS/OBJECT_MODEL 各 45 映射）、HLD 27 条边无环；检查范围内 ID/映射不一致为 0，**不是 Bash/CTest 的 PASS**。
- **MAT 历史字典**：582 行唯一路径、五种 shape 分布、MAT v5/double/zlib/READ_OK 的**报告内部结构**一致；未重新读取 MAT、未取得新的逐元素 L2/SHA-256。
- **待修订问题**：报告登记 **AF-001～AF-012 共 12 项**（高 1、中 8、低 3）及 **AH-001～AH-002 两项历史/执行边界**。AF 是文档语义冲突或口径不完整，**均未在本轮直接修复**，之后要逐项修订/复核并登记提交 SHA。
- **后续数学审计**：KvD、采样/噪声、轨迹保持“空白”（原源码逐式未进行）；Adaptive、分析、球谐/数值参考等部分检查过但未完成原文/独立 L3，标“正在审计”。**不得将三态审计完成率等同算法完成率或文档基线成熟度**。
- **归档边界**：旧审批包、2026-11-06 的 26 条 DAG、较早版本的 MATLAB L1/SHRotation/CLI 文案保留为历史记录，不直接反向当作当前义务；现行项目所有者决定和 L1 退出政策优先。

**本次不要求新增人工决定**。后续论文 PDF、H-10/H-11 执行权限、DEC-005/006 最终数值/性能阈值、基线/发布仍按 [TODO.md](TODO.md) 在对应阶段安排。

## 八、AF-001～AF-012 文档一致性问题修订及复读结论（2026-11-07）

**整改后的定向静态复读快照**：`893acb7b07862c0701e62b786462ef8395a116da`。详见 [逐项修订/复读报告](reviews/AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md)。该报告单独记录前述 [原始审计报告](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md) 中 **AF-001～AF-012 全部 12 项（高 1、中 8、低 3）** 在**文档语义一致性**范围内的改动、固定提交身份、复核断言和证据限制。**原审计报告仍保留原输入 SHA 和“当时尚未修订”的历史事实，不得从原文字反推当前仍未修订。**

本轮确认：

- **AF-001～012：已完成文字修订；12/12 定向静态断言通过**，修订在 SRS v0.11、HLD v0.6、对象模型 v0.4、contracts v0.2、mat_io v0.7、experiments v0.3、harmonics v0.7、TEST_PLAN v0.5、PLAN v0.5 等草案文件中体现；所有者的选择和历史审批 JSON 均未改变。
- **当前三态审计表**：已审计 **33**、正在审计 **4**、空白 **3**（共 40）；其中仍在审计的 harmonics、adaptive_mfa、analysis_plotting、GOLDEN_TOLERANCE_PROPOSAL 是未完成的数学来源/独立参考审核，不表示这 12 项文档措辞仍未修订。KvD、sampling_noise、trajectory 的**深层原源码逐式审核**仍保持空白。
- **不能扩大结论**：此轮没有实现/编译任何 C++、没有运行 CTest、Bash、MATLAB/Octave，也没有重新读取/写入原 MAT；没有为 12 项整改生成真实数值证明、批准模块最终 DEC-005 阈值、签署设计冻结或变更 H-10/H-11。
- **后续范围**：进一步逐式数学审计应按照此台账继续记录；需要项目所有者提供论文或批准真实操作时按 [TODO.md](TODO.md) 对应阶段办理。审计报告的“已修订”不能替代最终 P/REQ/AC/TC 验收。

## 九、下一轮问题扫描与 AUDIT_PROCESS 登记结果（2026-11-07）

**扫描固定输入 SHA**：`3beb8c06a9d40380431f99e31ed0f70b346863a2`（扫描中创建/修改的审计台账和版本矩阵发生在其后，不能用旧 SHA 代表新提交）。详细**逐项目标、相对路径及整改状态**统一见 [AUDIT_PROCESS.md §二～三](AUDIT_PROCESS.md)。上轮 12 个 AF 问题在已复核的范围内维持“已整改”，不重开；本轮新增问题必须以新编号登记为“未整改”。

- **AF-001～012**：由上一轮 [逐条修订证据](reviews/AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md) 支持，分拆后 13 条具体目标，整改状态**已整改**。这是文档一致性结论，未冻结正式软件基线。
- **AF-013～020**：本轮新增 **8 条整改目标，全部未整改**，分别涉及 `docs/design/kvd.md`（2）、`docs/design/sampling_noise.md`（1）、`docs/OBJECT_MODEL.md`（1）、`docs/ARCHITECTURE.md`（2）、`docs/design/mat_io.md`（1）、`docs/design/trajectory.md`（1）。新问题主要是历史 1e−10/首轮 0.01/0.1 容差口径、失败策略/提交边界、固定输出场景、Sophus 的已选状态与 epsilon 规则。
- **扫描方式及来源限制**：GitHub 连接器只读查看最新现行 SRS/HLD/LLD、历史决定/报告和部分原 `.m` 源文件，按已选设计逐条比对。已静态核对 KvD 两路近度幂次、Franz 模板、八面体取面、轨迹点列、Adaptive 一次求解及分析平滑的部分源码逻辑；**不是对全部原始 45 个 .m 的逐式穷尽审核，也没有新生成 i=1/2 完整方向数值、L2 SHA-256 或 R5 原论文公式验证**。
- **审计三态总数**：本次扫描后 `AUDIT.md` 跟踪 **41 个文件：已审计 31、正在审计 10、空白 0**。原三份数学 LLD 从“空白”变为“正在审计”，仅表示已有第一批只读源码局部审查、尚未完成全部数学核验；存在 AF-013～020 未整改的文件也保持“正在审计”。
- **后续执行原则**：如下一轮要实际修复新问题，按 [AUDIT_PROCESS.md](AUDIT_PROCESS.md) 每条目标修改对应现行文档，再以新 Git SHA 核对并转为「已整改」；**不能因为本轮新增了审计记录就将 8 项自动标为完成**。没有进行 C++、Bash、CMake、CTest、MATLAB、MAT 写入或任何基线冻结。

**本轮不新增需立即由人类决定的技术选择**。论文、真实测试授权、DEC-005/006 最终验收和发布仍按 [TODO.md](TODO.md) 适时处理。


## 十、AF-013～AF-020 后续纯文档整改与复读（2026-11-07）

**这是一轮在 §九所记原始“8 项未整改”扫描之后执行的整改**。原扫描快照 `3beb8c06a9d40380431f99e31ed0f70b346863a2` 的历史发现事实保持有效；[后续整改报告](reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md) 独立记录了六份现行设计文档及 LLD 索引的文字修订、固定核对提交 `a782b5c8d86e50fe4d2ae4e2b198b3cd6345de52` 与证据限制。

- **现行整改状态只从 [AUDIT_PROCESS.md](AUDIT_PROCESS.md) 判定**：AF-013～020 八条问题的各自**文档语义**已修改、8/8 定向静态检查通过，现均为**「已整改」**；历史 AF-001～012 的 13 个原子目标继续为「已整改」。合计 **21/21 项具体目标在文档一致性范围内已整改**。这不是没有其它潜在问题，也不意味着算法等价、真实 C++/CTest/数值验收通过。
- **本台账文件审计三态**：共 41 个文件，当前 **已审计 34、正在审计 7、空白 0**。HLD、OBJECT_MODEL、MAT I/O 的此次**文档合同检查**已完成，状态改为“已审计”；KvD、采样噪声和轨迹虽然消除了本次文本冲突，仍因原公式/固定输入/边界值独立 L3 未完成而保留“正在审计”；Adaptive、球谐、分析及容差附件等继续按原剩余数学审核范围推进。
- **当前剩余性质**：不再是 AF-013～020 八项已登记的旧措辞冲突，而是逐式数学核对、L2/L3/L4 参考、真实代码/安全/数值测试及人类签署门槛。继续依照 §零规则，一旦发现**新的**文档问题须在 AUDIT_PROCESS 另编号登记初始「未整改」，不能篡改旧修订证据。
- **没有实施**生产 C++、Bash、CMake、CTest、MATLAB/Octave、MAT 写入、真实数值仿真，也没有批准最终 DEC-005/006 或冻结基线；实际权限按 [TODO.md](TODO.md)。

## 十一、先审计、后整改：AF-021～AF-025 文档修订闭环（2026-11-07）

**审计优先顺序**已执行：先以固定 Git 提交 `af9198906cf89b6fb5162aea1d74683906740d78` 按 [AUDIT 规则](#零发现问题与整改台账规则强制) 对现行 SRS/HLD/各 LLD、旧审批/决策、MAT 源公式和部分原 MATLAB `.m` 只读审查，写入 [本轮扫描报告](reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md)；随后在 [AUDIT_PROCESS.md](AUDIT_PROCESS.md) 中以**未整改**状态先行登记 AF-021～025 五项；之后才修订实际目标文档并按 [定向修订复核报告](reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md) 在固定修改快照 `ac683f41af2f906137705f3ad31e8da53fe5c036` 做**5/5** 项静态新旧合同检查，最后将状态转为**已整改**。

- **五项整改已结案（仅文档语义）**：AF-021 HLD 普通版权门禁、AF-022 SRS MAT 首版范围、AF-023 HLD ADR-001 MAT 范围、AF-024 可选实际 N 对照图范围、AF-025 fixed_* 的 MATLAB 标量近度返回与 C++ 新增逐点向量记录之区分。**当前 AUDIT_PROCESS 的 26 个原子整改目标全部为「已整改」**；旧 AF-001～020 不因扫描新问题被重开。
- **逐文件审计状态**：本台账现共 **46 个路径：已审计 39、正在审计 7、空白 0**。新增的五份 reviews 仅按其**固定 SHA 的文档证据范围**标「已审计」，并不意味着历史结论适用于后续 HEAD。采样/噪声和分析 LLD 即使对应 AF 已整改，仍因逐式独立 L3 缺失而保持**正在审计**；KvD、Adaptive、轨迹、球谐及容差参考同理。
- **结构核对**：在修订快照中 66 REQ、66 TC、98 P、8 REQ-DC、HLD **27 条**设计依赖边（静态无环）保持一致；对七份受改现行文档的 **221 条**相对 Markdown 路径无缺失目标。原 Git subtree 仍为 `70ee155131c24a50169346d0ffbaff0c01b1c547`（45 .m / 582 .mat），**未重新读取 MAT 或运行 C++/CTest**。
- **范围限制**：本次只改文档及审计证据，不生成或实现 `.hpp`/`.cpp`、不运行 Bash、CMake、CTest、MATLAB/Octave、不写 MAT、不给 DEC-005/006 设最终阈值、不擅自批准人类基线或合并发布。全文逐式数学与独立参考核验仍继续按文件审计三态跟踪；延期人类材料和执行权限见 [TODO.md](TODO.md)。

**请勿把“26/26 文档整改已关闭”解释为“46/46 文件的全部数学内容已验证”或软件正式验收。**
