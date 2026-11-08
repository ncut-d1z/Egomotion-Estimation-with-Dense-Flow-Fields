# 实施进度与证据登记（PROGRESS）

> **滚动证据登记 v0.1（非需求、架构或编码规范）。** 此处是里程碑状态、DEC 处理、实际运行和批准记录的唯一当前入口；历史证据记录不得反向冒充当前分支的冻结 SHA。

> **2026-11-06 审批修订补充**：已新增 [HAR-2026-11-06-001 审查材料包](approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md)，把 H-01～H-16 分为已选技术路线、只供起草及暂缓执行三类，并提供具体静态源码证据、TC/Golden 对照和 M0～M5 候选退出证据。这是审查文档成果，**未改变 P/TC/AC 的验收状态**，也未解除全部 false 的执行权限。

> **项目长期决定**：[不运行 MATLAB 的数值证据政策](golden/NO_MATLAB_EXECUTION_POLICY.md) 已根据项目所有者的最新明确要求写入 SRS/TEST_PLAN/LLD/AGENTS：原 .m **只读静态审查**，L1 原 MATLAB 运行逐节点 Golden **不再作为任何冻结/验收阻塞**；下一步用 L2/L3/L4，保留真实 C++/CTest、DEC-005/008、版权、数据保护与人工签署。**本决定不是生产实施或运行测试授权。**

> **当前状态：审计及设计草案阶段；全部软件 REQ/TC/P/M0–M5 的验收状态仍依据实际执行证据，未宣告正式通过。** 版本与规范冻结状态见 [DOCUMENT_BASELINE_REGISTER.md](DOCUMENT_BASELINE_REGISTER.md)，`reviews/*` 的历史 SHA 是过往检查点，不能作为当前 HEAD 的动态通过证明。 本文件只记录可定位的实际子成果和未决项。验收规则以 [REQUIREMENTS.md](REQUIREMENTS.md)、[TEST_PLAN.md](TEST_PLAN.md) 和 [PLAN.md](PLAN.md) 为准；不得把已有文档、matio 独立探针或静态集合检查折算为业务 C++ 功能完成率。

**2026-11-07 项目所有者最新决定**：仓库根 [MIT LICENSE](../LICENSE) 已获明确授权创建，普通版权审核不再阻塞开发；球谐必须以 Ivanic–Ruedenberg **1996 原始论文＋1998 勘误**为依据并**纯 Eigen 重写、禁用 sdla**，等用户上传两份论文后实现；CLI 已删除自定义输出根参数，输出固定 `output/<RunId>/`，RunId 来自独立 OS 随机源；Result/Status 采用轻量封装、ErrorStage 稳定枚举、JSONL；实验使用独立配置、失败状态不提交、八次聚合不改分母，Manifest 区分 Complete/Incomplete/Failed；`atol=0.01,rtol=0.1` **仅作为已确认的首轮探索性筛查**，最终 DEC-005 验收门槛未冻结。详见 [最新技术决定](decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md)。这不是生产代码或测试执行授权。

**文档审计入口**：[AUDIT.md](AUDIT.md) 已汇总历轮用户提出的修改和 Markdown 评审意见，并以“空白、正在审计、已审计”跟踪逐文件审计。此表只记录文档审计过程，**不构成** C++/CTest/数值通过或任何人类批准；具体可推迟的人类事项仍集中在 [TODO.md](TODO.md)。

**2026-11-07 全量文档审计续记**：以 [固定 SHA 审计报告](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md) 为语义问题唯一清单，按照 [AUDIT.md](AUDIT.md) 的 40 行三态跟踪后续复核；本轮只审计和登记问题，**AF-001～012 未修订，且不得将“已审计”作为整改完成率**。

**2026-11-07 AF-001～012 修订续记**：[新修订复核记录](reviews/AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md) 将原来 12 项待修订的**文档语义冲突**更新为已修改/定向静态复核，后续状态以 [AUDIT.md §八](AUDIT.md) 为准；先前 E-DOC-017 和原始 FULL_DOCUMENT_AUDIT 的“当时未修订”文字**仅描述其固定输入快照**，不得当作当前结论。数学/实测证据仍不完整，不能提升软件状态。

## 1. 当前工作范围与证据

- 仓库：`ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields`
- 开发分支：`dev-plan-P0-P1-mat-dictionary-probe`（未合并至 `main`）。
- 原始数据范围：`origin/egomotion_estimation_dense_flow_fields_matlab_scripts/`；**45 个 .m、582 个 .mat，原始数据只读**。
- **已验证子能力**：既有 `src/mat_data_dict_probe.cpp` 的独立实测报告记录 Ubuntu 24.04 / GCC 13 / matio 1.5.26，582 个 MAT、582 个变量均 `READ_OK`、失败 0；详情及每个文件的属性见 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md)。这是归档数据格式证据，**不是**正式 Reader/Writer 或 TC/CTest 通过。
- **较早的静态检查**：使用 GitHub 连接器按固定 commit SHA 读取文档/递归源树，运行 JavaScript 静态断言并将结果记录在 [2026-11-06 静态追踪检查](reviews/STATIC_TRACEABILITY_2026-11-06.md)；实际观察 66 REQ、66 TC、98 P、45 .m 双向映射、26 条 HLD target 边，错误及环均为 0。该检查未在容器运行 Bash 脚本（外部 GitHub DNS 无法解析），也未运行任何业务 CTest。
- **2026-11-07 文档合同补充（非执行验收）**：见 [MAT I/O 与 SRS v0.4 一致性记录](reviews/LLD_MAT_IO_AND_CONSISTENCY_2026-11-07.md)；在固定提交 `d18c630008e5da0e4899906080e7dd560fdde8ef` 上，GitHub 连接器只读 JavaScript 检查 66 REQ、66 TC、98 P、26 条 HLD 依赖边，检查范围错误 0。原始 `origin/` 子树哈希保持 `70ee155131c24a50169346d0ffbaff0c01b1c547`（45 .m、582 .mat）。**不等于 Bash/CMake/CTest/Reader 真实运行**。
- **审查意见后的文档更正及复核**：参见 [跨文档一致性复核记录](reviews/DOC_CONSISTENCY_RECONCILIATION_2026-11-06.md)。固定提交 `5472c756b983fe002ef6e32143c96b6c6798cde9` 下，GitHub 连接器只读检查 66 REQ、66 TC、98 P、45 原始 `.m`、45/45 双向映射、26 条 HLD target 依赖边和 582 MAT Git 树路径，检查范围内不一致 0；与更正前比较，规范性 REQ/TC/P 表格行完全一致，`origin` Git 子树 SHA 不变。**这不是 Bash 现场执行或任何真实测试**。

- **2026-11-07 跨模块 LLD 候选合同修订**：见 [审查与未决项记录](reviews/LLD_CONTRACTS_TARGETS_TESTS_ALIGNMENT_2026-11-07.md)。当前仅修订文档，没有新增生产 C++、真实 CMake target、CTest/数值执行或 L2/L3/L4 实测证据；**不改变 98 项 P、66 项 TC、所有 AC 及 M0～M5 的验收状态**。

## 2. 可核验子交付物

| 证据 ID | 工件/记录 | 可以支持的事实 | 不能据此推断 |
|---|---|---|---|
| E-DATA-001 | [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) + `src/mat_data_dict_probe.cpp` | 指定源参数 SHA `48c590ba2564e4bdd293d2cbecd2eef83b291452` 的 MAT v5/zlib/double 582/582 解码证据；已记录 FNV64、变量、dims、读取状态 | 新数据类型兼容、正式 Reader 运行、业务输出写回读、MATLAB 算法数值等价、P10.4 独立复核 |
| E-DOC-001 | [2026-11-06 追踪核查](reviews/STATIC_TRACEABILITY_2026-11-06.md) | 指定 Git 提交的文档内容及源树集合对应无缺项、无 P/DAG 环 | `bash scripts/check_docs_traceability.sh` 已执行，真实 CMake target 无环、CTest 通过 |
| E-DOC-002 | [design/cli.md](design/cli.md) | 命令/参数类型/错误码/诊断输出的候选 LLD 已存在 | CLI 已实现、已获批准、错误/参数 schema 已冻结 |
| E-DOC-003 | [OBJECT_MODEL.md](OBJECT_MODEL.md) + [design/sampling_noise.md](design/sampling_noise.md) | Bias 场景编排、`FlowSynthesizer`、`sampling::randomUnitDirection` 的职责和命名已协调 | 对应头文件/测试已创建、随机数调用序列已数值验证 |
| E-DOC-004 | [审查后跨文档一致性复核](reviews/DOC_CONSISTENCY_RECONCILIATION_2026-11-06.md) | 8 份方案文档的 CLI/output、RunOutcome、SH R5、MAT 形状来源、HLD 环境/预算与章节引用得到文本统一；固定提交下的 66 REQ/66 TC/98 P、源目录和 DAG 静态断言未发现不一致 | H-03/DEC-005/008 已批准、H-10/H-11 已解除暂缓、Bash/CTest/Golden 已执行、SRS/HLD/LLD 可冻结 |
| E-DOC-005 | [Golden 及容差候选](golden/GOLDEN_TOLERANCE_PROPOSAL.md) + [解析数值 JSON](golden/analytic_golden_candidates.v1.json) | 已静态提出 12 组带固定输入与具体参考数字的解析候选；建议第一轮宽松连续值比较 `atol=0.01, rtol=0.1`，并强制结构/物理不变量；记录为独立 L3 候选（未经真实测试） | MATLAB 运行/原 L1 已验证、DEC-005 阈值已批准、C++ 通过测试、M0～M5/P/TC/AC 可标记通过 |
| E-DOC-006 | [Golden L1–L4 证据等级与 AC 缺口](golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md) | 已逐类记录目前缺失的原 MATLAB 每轮/每节点/RNG/异常/SH 旋转/逐帧 Session L1，区别于现存最终 L2、解析候选 L3 和待执行 L4；该文件目前已按项目所有者的新决定标出 L1 非必需，重点 AC 按更新的 SRS 采用 L2/L3/L4 | L2 全量值和 SHA-256 已复核、L3/L4 已测试、DEC/AC 已通过、可以冻结或合并 |
| E-DOC-007 | [长期无需 MATLAB 运行政策](golden/NO_MATLAB_EXECUTION_POLICY.md) + [SRS](REQUIREMENTS.md) + [TEST_PLAN](TEST_PLAN.md) + [L2/L3/L4 矩阵](golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md) | 已从规范性 AC 中撤销 MATLAB 运行及 L1 逐节点强制前提，关键验收路线改为 L2/L3/L4；未修改原始 MATLAB/MAT 或旧审批 JSON | L2/L3/L4 已独立复核、代码/CTest/DEC-005 已完成、SRS/HLD/LLD 已冻结或原 MATLAB 运行全行为已等价 |
| E-DOC-008 | [2026-11-07 LLD/MAT I/O 一致性审查](reviews/LLD_MAT_IO_AND_CONSISTENCY_2026-11-07.md) + [mat_io.md v0.5](design/mat_io.md) | 已统一 SRS v0.4 的 PLAN/TEST_PLAN 引用、当前未批准候选 atol=0.01/rtol=0.1 与历史 rtol=1e-10；修正各 LLD 的 L3 参考表述；MAT I/O 补 MatDenseArray 自拥有候选接口、单/多变量边界、错误与 RunContext、L2/L3、拟议 target 与负例→TC/CTest；指定 SHA 下连接器静态检查 66 REQ/66 TC/98 P/26 HLD 边均一致，origin Git 子树未改 | 新 C++ 类已创建/编译、真实 MatResultReader/Writer 与 MAT 读回/负例已执行、L2 SHA-256/逐值参考已完成、CMake/CTest 通过、数值阈值/接口已冻结 |
| E-DOC-009 | [2026-11-07 跨模块 LLD 合同/target/测试映射](reviews/LLD_CONTRACTS_TARGETS_TESTS_ALIGNMENT_2026-11-07.md) + [contracts.md](design/contracts.md)、[experiments.md](design/experiments.md)、[harmonics.md](design/harmonics.md)、[sampling_noise.md](design/sampling_noise.md) | 在固定提交 `2689ac522aaa480d9e66ed993ae1ffc94254065f` 上，通过 GitHub 连接器只读核对 66 REQ、66 TC、98 P、26 条 HLD 边及 9 项限定合同断言，错误 0；已起草统一 Failure/Result/Status、ErrorDetail 阶段、RunContext 所有权、MatDenseArray io 边界、各模块 target/候选测试映射与 L2/L3/L4 术语，并将 CLI 多变量合同与 TEST_PLAN 候选关联 | 新公共头/Result/RunContext/SHRotation/RNG/Reader/Writer/CMake/CTest 已实现或编译、L2 SHA-256/L3 已执行、DEC-003/005/007/008 已批准、H-10/H-11 解除暂缓、LLD 已冻结或 AC 已验收 |
| E-DOC-010 | [项目所有者 2026-11-07 决定](decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md) + [MIT LICENSE](../LICENSE) + [Ivanic–Ruedenberg 原文与 Eigen LLD](design/SH_ROTATION_IVANIC_RUEDENBERG.md) + [新审查记录](reviews/OWNER_TECH_DECISIONS_ALIGNMENT_2026-11-07.md) | 已建立仓库根 MIT、明确 1996 原文/1998 勘误优先和 Eigen-only 禁用 sdla；SRS v0.5、HLD v0.4、CLI v0.2（无自定义输出根）、MAT I/O v0.6；轻量 Result、ErrorStage、JSONL、独立 Runner 配置/失败策略、首轮 `atol=0.01`/`rtol=0.1` 筛查已选定；固定 SHA 下 66 REQ/66 TC/98 P/27 HLD 边静态一致 | 原始论文已上传/完成公式逐式核验、生产 R5/Result/CLI/Reader/Writer 已实现、CMake/CTest/数值测试/最终 DEC-005 已通过、用户允许合并发布或完成基线冻结 |
| E-DOC-011 | [DEC-004 历史字段决定](decisions/DEC-004_HISTORICAL_MAT_FIELDS.md) + [SRS v0.6](REQUIREMENTS.md) + [文档控制矩阵](DOCUMENT_BASELINE_REGISTER.md) | 已将 DEC-004 当前保存字段/历史字段/缺失分析显式失败的原则写入 SRS、Runner/Analyzer LLD、TEST_PLAN、PLAN；REQ-DC 八项迁入 SRS 附录（ID/AC/TC不变），SRS 与设计职责分层、Golden L2/L3/L4 术语归口、当前文档版本/SHA 单一清单计划建立 | DEC-004 的生产 Runner/Analyzer 已实施、当前 branch CMake/CTest 或任何数值测试已执行、SRS/HLD/LLD/TC/AC 已冻结或全部完成 |
| E-DOC-012 | [SRS v0.6](REQUIREMENTS.md)、[DEC-004](decisions/DEC-004_HISTORICAL_MAT_FIELDS.md)、[文档控制矩阵](DOCUMENT_BASELINE_REGISTER.md)、[TEST_PLAN](TEST_PLAN.md) | GitHub 连接器对固定提交 `fdbb0800ec77539a4470decde5009ba294b33135` 的文件内容进行**只读 JavaScript 核对**：66 REQ/66 TC/98 P、附录 A 8 条 REQ-DC、27 条 HLD 依赖边无环、原始 45 .m/582 .mat 路径数量；DEC-004 已反映于 SRS/LLD/TEST_PLAN/PLAN，限定范围内不一致 0。该固定提交的版本矩阵链接尚未创建，后续有新增 docs/DOCUMENT_BASELINE_REGISTER.md | 并未执行现场 Bash、生产 C++/CMake/CTest/数值测试，未取得 L2 全量 SHA-256 或 L3 全部数值，未签署任何 SRS/HLD/LLD/测试/里程碑正式基线 |
| E-DOC-013 | [本轮需求阶段静态审查](reviews/DOCUMENT_REVIEW_AND_SH_DECLARATIONS_2026-11-07.md) + [TODO.md](TODO.md) + [球谐声明专题](design/SH_ROTATION_IVANIC_RUEDENBERG.md) | 根据固定 Git SHA `0d6060241a390a5c8c146154256f0a0f9173e277` 的 GitHub 连接器只读检查：66 REQ、66 TC、98 P、附录 A 8 REQ-DC、27 HLD 边静态无环，原始 45 .m/582 .mat 路径集合、交叉链接与重要文档语义核对范围内 0 项不一致；已修 HLD output-root/球谐、SRS 参数类型、DEC-004 历史措辞；专题 LLD 增加**声明级**预定 C++ 文件/结构体/类/静态方法，不包含任何实现；所有可推迟人类决策集中 TODO | 当前 Git HEAD 与上述静态证据 SHA 相同、真实 C++ 头/源文件已创建、球谐递推已实现、Bash/CMake/CTest/数值测试执行、最终容差/性能门槛获签署、SRS/HLD/LLD 正式冻结 |
| E-DOC-014 | [SRS](REQUIREMENTS.md) / [PLAN](PLAN.md) / [harmonics LLD](design/harmonics.md) / [TEST_PLAN](TEST_PLAN.md) | **纯文档一致性修订**：SRS REQ-F-CLI-002 删除“结果目录参数”；PLAN §1/P1.1/P7.6 删除 Don Williamson C++ 直接适配表述；harmonics §3.1.1/§7.2 明确 Ivanic–Ruedenberg 1996 原文＋1998 勘误为正式数学依据、Eigen-only、Don Williamson 仅作代码参考；TEST_PLAN §2.8 更正 H-14 名称及 R3/R5 验证范围。GitHub 只读固定检查点 `5678a1cbf94ff4ee8c87845c5176cf46efa3725e`：66 REQ/66 TC/98 P、HLD 27 边，范围内 8 项断言均通过；SRS v0.8、PLAN v0.3、harmonics v0.6、TEST_PLAN v0.3 均仍为草案 | 上述文档已构成正式批准基线、真实生产代码已修改、论文 PDF 已核验、Bash/CMake/CTest/数值测试已执行、DEC-005 最终容差已批准 |
| E-DOC-015 | [SRS v0.9](REQUIREMENTS.md)、[对象模型 v0.3](OBJECT_MODEL.md)、[L2/L3 容差文档](golden/GOLDEN_TOLERANCE_PROPOSAL.md)、[PLAN v0.4](PLAN.md)、[TEST_PLAN v0.4](TEST_PLAN.md) | **仅中优先级纯文档一致性修订**：UC-05 改为 DEC-004 `HistoricalOnly`/不等待归因/缺必需字段显式失败；`SHRotate.m` 的对象映射改为 `HarmonicRotator::blocks` 且引用球谐专题 LLD 的唯一声明；L3 旋转参考改为 Ivanic–Ruedenberg 1996/1998＋**Eigen 独立重写、禁用 sdla**，Don Williamson 仅作参考。PLAN P1.1 在检查前已修正，未重复覆盖。固定 GitHub 只读 JavaScript 检查点 `930316daad3b678e40c9592d5a4dc9a150c302f6`：66 REQ/66 TC/98 P、附录 8 REQ-DC、27 HLD 边静态无环；限定范围发现不一致 0。该 SHA 不包括后续进度及矩阵登记提交 | **不代表**已编写/编译球谐 C++、运行 Bash/CMake/CTest、取得 L2/L3/L4 实测参考、完成最终 DEC-005 或冻结基线；原始 `.m/.mat` 均未修改 |
| E-DOC-016 | [历轮用户意见汇总及三态审计台账](AUDIT.md) + [LLD 索引](design/README.md) + [SRS 草案](REQUIREMENTS.md) + [文档控制矩阵](DOCUMENT_BASELINE_REGISTER.md) | **仅低优先级纯文档一致性修订**：归纳历轮用户 Markdown 详细审查为 AU-001～AU-028，逐文件审计表使用“空白/正在审计/已审计”；LLD 索引纠正“Rodrigues 来源待审”与“绘图后端未定阻塞”；SRS 的两条来源/MIT 义务改为可观察需求并链接 HLD/ADR，不重复实现方案；WBS 与 TEST_PLAN 同步 SRS v0.10；滚动 PROGRESS 的正式基线列计划改为“不适用”。本条仅登记提交中的文档审计范围，静态复核与版本矩阵的固定 SHA 仍需后续同轮核对记录 | 该台账已涵盖未在对话中提供的原始聊天逐字稿、所有源文件均已完成审计、Bash/CMake/CTest/MATLAB 已执行、原 45 .m/582 MAT 已重跑、软件验收或正式基线已冻结 |
| E-DOC-017 | [AUDIT 全文档审计台账](AUDIT.md) + [固定快照审计报告](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md) | 在**固定审计输入提交** `4627cc1e48be9c43ad10b28fda646d65ee8ef01c` 上，按 AU-001～AU-028 对 40 路径做 GitHub 连接器只读文档审计：452 条本地 Markdown 链接无缺失目标、围栏配对，66 REQ/66 TC/98 P、8 REQ-DC、45 .m（SRS/OBJECT 各 45）、27 条 HLD 边静态无环，582 条 MAT 字典内部尺寸/类型/READ_OK 一致；报告登记 **AF-001～AF-012 12 个待修订问题＋AH-001～AH-002 两个历史/执行边界**。AUDIT 三态现为 **已审计 25/正在审计 12/空白 3**；“已审计”仅指明确的审计范围，**不等于无问题或已修订** | 未修复 12 项文档问题；未开展全部原始数学源码逐式/独立 L3 核验；没有 Bash/CMake/CTest/MATLAB/原 MAT 读取或写入/发布/正式基线签署；报告固定快照不覆盖后续 HEAD |
| E-DOC-018 | [AF-001～012 文档问题逐条修订与复读](reviews/AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md) + [AUDIT 40 文件审计台账](AUDIT.md) + [最新版本/SHA 控制矩阵](DOCUMENT_BASELINE_REGISTER.md) | 在固定 SHA `893acb7b07862c0701e62b786462ef8395a116da` 上，**12/12 AF 文本一致性定向断言通过**（AF-001 高 1、AF-002～009 中 8、AF-010～012 低 3），已统一 Runner 固定输出根、球谐 Eigen/既有 contracts 依赖、Result/Failure、Adaptive 失败策略、H-01 历史 draft、首轮 0.01/0.1 筛查与版权边界、对象模型方法名及 Sophus 已选范围；独立版本增至 SRS v0.11/HLD v0.6/OBJECT v0.4/contracts v0.2/mat_io v0.7/experiments v0.3/harmonics v0.7/TEST_PLAN v0.5/PLAN v0.5。静态检查保留 **66 REQ/66 TC/98 P/8 REQ-DC/45 .m 映射/27 DAG 边**，11 个受审现行文件的 250 条本地 Markdown 链接无缺失目标；AUDIT 三态现在为 **已审计 33/正在审计 4/空白 3**。**仅指文档修订，原始审计报告仍保留旧快照的发现事实** | 尚无 C++ 实现/构建/CTest/新 L2 SHA-256/一般输入 L3/性能测试；球谐论文原文与勘误尚待用户上传，KvD/采样/轨迹及部分 Adaptive/分析仍待深层数学审计；未解除 H-10/H-11，未冻结 SRS/HLD/LLD，未批准 DEC-005 最终阈值 |
| E-DOC-019 | [审计规则和三态文件台账](AUDIT.md) + [问题级整改进度](AUDIT_PROCESS.md) + [固定版文档矩阵](DOCUMENT_BASELINE_REGISTER.md) | **纯文档下一轮问题扫描**：在固定旧输入 commit `3beb8c06a9d40380431f99e31ed0f70b346863a2` 基础上，根据之前 [AF-001～012 修订证据](reviews/AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md) 将其 **12 个原始问题/13 个原子整改目标标为「已整改」**；下一轮在六份现行 LLD/HLD/对象文件发现并登记 **AF-013～020 8 个新整改目标，全部「未整改」**（KvD 首轮容差/H-13；采样失败策略；对象模型 Adaptive 状态；HLD QA-01 固定根/ADR-002 Sophus；MAT I/O 筛查；轨迹 near-zero/eps）。AUDIT 文件审计三态已更新为 **41 文件：已审计 31 / 正在审计 10 / 空白 0**；仅进行了 GitHub 连接器只读文本/部分原 .m 源码核对，未修改这 8 个被审计源文档 | AF-013～020 已修复或已获完整独立 L3、生产 C++ 已编译/CTest 已执行、MATLAB/Octave 已运行、DEC-005/006 最终门槛已通过、SRS/HLD/LLD/TEST_PLAN 基线已冻结、H-10/H-11 已放开或 PR 已合并 |
| E-DOC-020 | [AF-013～020 逐条文档修订/固定快照复核](reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md) + [整改台账](AUDIT_PROCESS.md) + [文件审计三态](AUDIT.md) | **AF-013～020 在修订后固定 SHA `a782b5c8d86e50fe4d2ae4e2b198b3cd6345de52` 的 8/8 定向静态检查通过**，消除了 KvD 历史 1e−10 候选/H-13、采样失败处理、Adaptive 失败状态提交、HLD QA-01 非法用户输出根/ADR-002 Sophus、MAT I/O 首轮筛查与 Rodrigues epsilon 的跨文档残留；六份设计文件草案更新为 KvD v0.4、sampling_noise v0.2、OBJECT_MODEL v0.5、ARCHITECTURE v0.7、mat_io v0.8、trajectory v0.2，LLD 索引 v0.5。**当前整改台账 21/21 个具体目标「已整改」**，AUDIT 文件审计三态 **已审计 34/正在审计 7/空白 0**；固定快照中 66 REQ/66 TC/98 P/8 REQ-DC/27 HLD 静态边一致，十份现行文档的 242 条相对 Markdown 文件链接均可定位；报告只表述**文档**修订 | 没有实际 C++/CMake/CTest/Bash/MATLAB、独立 L3 数值或 MAT 读写；未证明所有原函数数学等价、路径 TOCTOU 实际安全、SRS/HLD/LLD 正式冻结，未批准 DEC-005/006/发布或解除 H-10/H-11 |
| E-DOC-021 | [先审计固定快照报告](reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md) + [问题级整改台账](AUDIT_PROCESS.md) | 先在固定输入 Git SHA `af9198906cf89b6fb5162aea1d74683906740d78` 对现行规范、部分只读原 .m 及历史审查资料开展新一轮 AUDIT 扫描，并在目标修改前登记 AF-021～025 **五项未整改**：HLD 一般版权门禁、SRS/ADR MAT 首版支持范围、分析可选实际 N 对照图，以及固定噪声流函数的**原 MATLAB near_last 标量**与 C++ 新增逐点 nearness 向量的区别；检查 66 REQ/66 TC/98 P/27 静态边、MAT 原 Git 归档不变。此记录是**扫描当时**尚未整改的事实，之后改动需另行留证 | 不得由发现事项推断已经修改/通过；没有 C++/CTest/Bash/MATLAB、独立 L3 或真实 Reader 验收 |
| E-DOC-023 | [七份在审文档的原源码静态审计](reviews/AUDIT_SEVEN_MATH_DOCS_SCAN_2026-11-08.md) + [AF-026/027 文档整改复核](reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md) + [问题整改两态](AUDIT_PROCESS.md) + [审计文件三态](AUDIT.md) | **先审计后整改**：固定扫描输入 `e190ed74240bf7b3b32a65c8fb6099fbdedc46b9` 对 7 个仍“正在审计”的 KvD/Adaptive/球谐/采样/轨迹/分析/容差文档逐一核对现存只读 .m 的公式/索引/时序；先将 **AF-026（分析 APL 与全局 AN-G 参考编号撞号）、AF-027（x_ref 误称强制原 MATLAB L1）登记「未整改」**，修改两个规范后固定快照 `22bc38635b2350bc9078c2952ba9ad520a55f7b1` 定向复核无不一致，整改状态转为「已整改」。AUDIT 本次**文档静态审计范围**更新为 **48 文件：已审计 48、正在审计 0、空白 0**；AUDIT_PROCESS **28 个原子整改目标已整改、未整改 0**。本轮 7 份在审文件的 53 条相对 Markdown 链接无缺失、围栏配平；仅使用 GitHub 连接器读取原静态资料 | **未完成** Ivanic–Ruedenberg 原文/1998 勘误 PDF 的数学逐式核验、R5 全 25 元素独立 L3、KvD/采样/Adaptive/轨迹/分析全量数值 L3、生产 C++/CMake/CTest、MAT 重新读取/安全写回读、DEC-005/006 正式阈值或基线批准；本次“已审计”只指可用原源码与文档的静态审计范围，不能宣称数学数值或软件验收通过 |
| E-DOC-022 | [AF-021～025 修订复核证据](reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md) + [AUDIT 当前文件三态](AUDIT.md) + [AUDIT_PROCESS 当前整改两态](AUDIT_PROCESS.md) | **先审计后整改**：在定向修订后的固定 SHA `ac683f41af2f906137705f3ad31e8da53fe5c036` 上，AF-021～025 **5/5 静态合同复核通过**；已同步 SRS v0.12、HLD v0.8、analysis/sampling v0.3、PLAN/TEST_PLAN/LLD 索引 v0.6；当前整改台账 26/26 原子目标**已整改**，文件审计三态 **46 文件：已审计 39、正在审计 7、空白 0**。源 MAT/原 MATLAB 只读不变，REQ/TC/P 66/66/98、HLD 27 边静态不变，七份现行修改文档 221 条相对链接无缺失 | **仅针对文本合同已修订**，不是 46 个文件的完整数学逐式审核、MATLAB/原 MAT 重新验证、C++/CTest PASS 或项目 SRS/HLD/LLD 基线冻结；H-10/H-11、DEC-005/006/发布仍在 TODO |
| E-DEC-001 | [JSON 审批落实记录](approval/DECISION_IMPLEMENTATION_NOTES.md) | **H-02/H-14 已指定 Sophus 与 Don Williamson Ivanic 方案 A**；H-04 已提供 Ubuntu 24.04.4 VM 快照；H-12 已确定 `output/` 并更新 `.gitignore` | 任意第三方 C++ 文件已复制、Sophus 已安装/构建、MAT 写入/CTest 已执行、Golden/容差已批准、程序可发布 |

## 3. 实施任务与里程碑状态

| 阶段 | 有证据的局部成果 | 阶段状态 | 下一验收证据 |
|---|---|---|---|
| P0：源资料/SRS/HLD/LLD 审计 | 45 .m 清单、582 MAT 类型字典；根 MIT 已建立、球谐数学/Eigen 路线已明确（论文待上传） | **进行中，未验收** | P0.1–P0.8 追踪、第三方既有 notice（非研发阻塞）、接口/CLI/错误合同、独立参考与最终容差证据 |
| P1：运行环境、CLI、MAT | 独立 matio 探针子能力；H-04 指定 VM 环境；H-12 配置排除未来 `output/` 数据上传 | **H-10 暂缓生产代码/运行，P1 未验收** | 生产 Reader/Writer、RAII、路径保护负例、写后回读和真实 CMake/CTest；解除暂缓和授予运行权限后再执行 |
| P2–P9：几何/模拟/求解器/实验/分析 | LLD 草案与 source-to-responsibility 映射 | **代码未实现/未验证** | 对应 C++ 实现、固定输入 Golden、分模块数值误差与通过日志 |
| P10：集成验收与复现 | 规划与证据模板 | **未开始验收** | 逐模块集成、论文复现、P10.4 独立 MAT 审核、性能/完整报告 |

| 里程碑 | 目前状态 | 已有输入 | 缺失的退出条件 |
|---|---|---|---|
| M0 源资料与设计评审 | 未通过 | 源树清单、数据字典、MIT、新 SRS/HLD/LLD 与静态核查 | P0.8 正式接口/签名审查、独立数值参考、CLI/错误范围与未决技术事项 |
| M1 MAT 读写/构建闭环 | 未通过 | 582/582 独立探针记录 | 正式 Reader/Writer、CMake 业务 targets、CTest、路径/变量负例、写回读 |
| M2 Bias 数据生成与统计 | 未通过 | 设计合同 | Franz/KvD/噪声 C++ 实现及 Golden、完整 Bias 证据 |
| M3 Adaptive 估计实验 | 未通过 | 设计合同 | 球谐/Adaptive 求解、状态/周期逻辑、固定输入及逐轮 Golden |
| M4 分析及绘图 | 未通过 | 设计合同 | 分析曲线与输出核验，可选绘图后端的批准 |
| M5 最终交付 | 未通过 | 项目 MIT、工程方案及候选证据 | 完整平台/CTest/数值证据、维护文档、批准合并/发布；一般版权审批不作为研发阻塞 |

**日期/资源状态**：H-04 已提供一台 Ubuntu 24.04.4/x86_64 VirtualBox 的 `uname/df/free` 快照，物理内存总 8127380 KiB、当时 available 4908696 KiB、Swap 4194300 KiB；这些**不是**批准的资源配额或性能通过阈值。所有里程碑的具体日期、工时、CPU 核数、预算及验收指标仍待决定；不根据文档任务顺序擅自承诺日程。正式通过条件还需要相应 REQ/TC/AC 和证据编号。

## 4. 角色与批准权限（不杜撰人员分配）

| 责任角色 | 当前可确认的职责 | 人员/批准情况 |
|---|---|---|
| 项目所有者/需求批准人 | 已直接选择仓库 MIT、Eigen-only 球谐/原论文与 1998 勘误、删除 CLI 输出根、ErrorStage/Result/JSONL/RunContext/探索性筛查 | **设计方向已选**；最终数值阈值/真实实施与发布授权仍独立办理 |
| 来源登记维护者（非审批门禁） | 只读原论文/脚本和实际使用第三方文件的来源及原 notice 归档，不再安排普通版权审批来阻塞开发 | 不要求专门人员签字以启动原创 C++ 研发 |
| 架构与 C++ 实现者 | 按 P 任务维护对象合同、目标依赖、代码、提交/PR | 本轮只记录文档维护与连接器检查；正式实施责任人待分配 |
| 数值验证者 | 独立 Golden、统计样本、批准容差下的逐轮对照 | 未指派、Golden 和 DEC-005 未冻结 |
| 测试/集成维护者 | Debian/Ubuntu 构建、CTest、数据/资源记录、静态审计核查 | Ubuntu 探针已有记录；完整工程/CTest 未获验收 |

> **Golden 证据等级最新状态**：依 [E-DOC-007](golden/NO_MATLAB_EXECUTION_POLICY.md)，L1 原 MATLAB 运行**永久非强制、非阻塞**；L2 作者 MAT 存在但独立 SHA-256/逐值复核仍缺，L3 的 AN-G01～12 仅为候选（完整逐轮/递归/一般输入仍待独立参考），L4 尚无新执行统计。故 KvD/Adaptive/RNG/SH/Session 的 AC 仍因**独立参考/真实 C++/CTest/容差批准不足**而未验收，**不是因为缺 L1**。`atol=0.01`、`rtol=0.1` 已由所有者选定为首轮宽松探索性筛查；**最终 DEC-005 分模块 AC/TC PASS 数值阈值仍未批准**。

**本阶段的人类输入已集中在 [docs/TODO.md](TODO.md)**：论文 PDF、生产代码及运行许可、DEC-005 最终阈值、DEC-006 真实性能预算、基线签署与发布均可在相应阶段办理；**当前需求分析只修改文档，未获得实际 C++/CMake/CTest/Bash/MAT 写入授权**。DEC-004、MIT、Eigen-only、无自定义输出根、Result/ErrorStage/JSONL、L1 退出均**已选定，不应重复申请**。此前 E-DOC-xxx 的版本/SHA 均是历史证据，不代表当前文件版本。

**问题级整改入口**：[AUDIT_PROCESS.md](AUDIT_PROCESS.md) 维护相对路径/具体目标以及「未整改/已整改」两态；[AUDIT.md](AUDIT.md) 维护「空白/正在审计/已审计」三态。**E-DOC-019 记载的 AF-013～020 当时仍未整改是历史扫描事实**，随后按 [E-DOC-020 修订证据](reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md) 8/8 目标已在**文档语义**范围修订复核，整改台账现 21/21 目标均为「已整改」。两种状态不能互换，数学与软件独立验收仍缺。

**2026-11-07 后续审计结论**：先前 E-DOC-021 的“AF-021～025 未整改”是固定输入 SHA 下的历史发现；最新 [AF-021～025 修订复读](reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md) 已将 5 项完成文本整改，当前 [AUDIT_PROCESS.md](AUDIT_PROCESS.md) 为 **26/26 已整改**，但 [AUDIT.md](AUDIT.md) 仍有**七份文件处于“正在审计”**（缺全量逐式/独立 L3）。不能把整改结案当成人类基线批准或功能验证。

**2026-11-08 当前审计快照说明**：七份在审文件已完成现存静态源码/公式/索引审阅；发现并修复两项文档级冲突 AF-026/027。详见 [七文件审核与整改证据](reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md)、[AUDIT](AUDIT.md)、[AUDIT_PROCESS](AUDIT_PROCESS.md)。**源论文 PDF 未上传、独立 L3 数值/真实 CTest 未做，不可用 48/48 文档审计完成率代替真实软件验收或冻结。**

## 5. 未解决决策与验证阻塞

| 决策/阻塞 | 当前实际状态 | 下一步必须提交的材料 |
|---|---|---|
| DEC-004 历史字段处理 | **项目所有者已经选定规则，非继续待人类选型**：新 Runner 仅保存当前有效 save 字段，旧 582 MAT 完整只读；额外字段 HistoricalOnly，分析必需字段缺失明确失败 | [DEC-004](decisions/DEC-004_HISTORICAL_MAT_FIELDS.md) 已记录；实际 Runner/Analyzer、L2/L3 负例及 TC/CTest 尚未实现，历史成因未知不阻塞开发 |
| DEC-001/002 当前 MAT 格式和类型 | 独立探针的 582/582 读取事实已记录；**首版 MAT v5/zlib、二维密集实数非 logical double 数组的读取范围已由所有者选定**，不是仍待决定的选型 | 正式 Reader/Writer 实现与兼容性/拒绝未知类型负例、安全写回读、L2 逐值与独立 SHA-256，以及最终 SRS/HLD 基线签署仍待取得证据；不要将原探针结果视为这些项目通过 |
| DEC-003/007 项目 MIT 与来源跟踪 | **项目 MIT 根 LICENSE 已由所有者授权建立，球谐 Eigen-only 禁 sdla；普通版权审核不再阻塞开发** | 等待用户上传 1996/1998 论文以开展数学实现；实际上游文件保留现有来源 notice 即可；不把独立作品自动重新许可为 MIT |
| DEC-005 数值验收 | **所有者确认 `atol=0.01, rtol=0.1` 为首轮宽松筛查，最终模块 PASS 阈值未冻结** | 根据 L2/L3/L4 固定输入、真实 C++ 误差、严格状态/物理不变量决定正式分模块阈值；无需 MATLAB/L1 |
| DEC-006 平台与资源 | **用户已提供 H-04 主机环境事实；预算/性能仍未冻结** | Ubuntu 24.04.4 VirtualBox/x86_64 的资源快照已记；待将来实际 Debian/Ubuntu 业务构建和性能测量，不以容量快照作为性能验收标准 |
| DEC-008 CLI/CTest/Golden 映射 | **未关闭** | 批准 [cli.md](design/cli.md) 参数及错误序列化，真实测试文件/注册名、Golden identity、执行证据 |
| Bash 现场检查 | **H-11=deferred，禁止本轮现场执行** | 仅保留过去的连接器静态审查；未来人类另行授权后才能在完整仓库运行 Bash 并留 stdout/stderr/exit/SHA |
| MAT 业务读写 | **尚未实现；H-10=deferred 且 permissions.writeMat=false** | 已确定新数据根 `output/`、已编辑 `.gitignore`；Reader/Writer、路径安全、写后回读/负例需取得单独运行授权和真实证据 |

**用户 JSON 的授权矩阵**：`runCMake`、`runCTest`、`runMatlab`、`writeMat`、`runBash`、`measurePerformance`、`mergePR`、`publish`、`createLicense`、`freezeBaseline` 均为 `false`。H-01/H-03/H-05/H-06/H-07/H-08/H-09/H-13/H-15/H-16 仅允许**先起草**；目前不能据这些决定扩大权限或认定需求/软件验收。上传原始审批 JSON 未自动提交到仓库。

## 6. 证据回填模板（后续每次实际执行时新增记录）

```text
evidence_id:
date_time_with_timezone:
owner_or_executor:
repository / branch / commit_sha:
REQ / AC / TC / P:
source_matlab_blob_sha / input_mat_sha_or_hash:
command_and_arguments:
os / compiler / dependencies / cmake_config:
expected_vs_observed:
pass / fail / skip / not_run counts:
stdout_stderr_log_path / output_path:
actual_test_exit_code:
open_issue / DEC / caveats:
reviewed_by / review_date:
```

本模板是 [TEST_PLAN.md §4.1](TEST_PLAN.md) 单次测试证据字段的**扩展记录格式**；其中 `expected_vs_observed` 对应预期/实测，`source_matlab_blob_sha / input_mat_sha_or_hash` 对应输入与参考身份，`pass / fail / skip / not_run counts` 对应状态计数。真正的 Golden 数值文件应另记 SHA-256 与经过批准的 DEC-005 容差，且不能以本次审查文档代填测试结果。

每次新增证据只更新对应状态，不以补齐文档或估算百分比改变未验收的 98 个任务。严格按 [AGENTS.md](../AGENTS.md) 禁止重写历史、未经授权改动原始 MAT 或自动合并分支。


## 7. 2026-11-08 增量文档审计与修订证据（仅静态复核）

| 证据 ID | 固定输入/提交与文档 | 实际完成 | 不能推断 |
|---|---|---|---|
| E-DOC-025 | [审计报告](reviews/AUDIT_FOUR_POLICY_DRIFTS_SCAN_2026-11-08.md)，原始输入 `0fa760423532fda691c046f2e4eac5fc37784503`，登记提交 `815c748476b4b8ca47ec3e894a57931e4aeabee8` | 对现行 SRS/HLD/LLD、Golden/DEC、AGENTS/PROGRESS 的跨文档静态比对发现 AF-028～031 四条决策口径冲突，按 AUDIT_PROCESS 先以未整改登记 | 不代表原 MATLAB 运行、独立 L3、生产编译、CTest 或性能测试 |
| E-DOC-026 | 修订及固定复核提交 `c4944b085ff92c0b8259d766668e2fa97939ee9d`；`docs/PROGRESS.md` blob `5943a869807c7777b337e738ba15b64ad58ecabd`、`AGENTS.md` blob `296a49fd7a09e71796ea7ebff984a2904505bc2d` | AF-028～031 四处实际修改并对该固定提交定向复读：**4/4 通过**；本轮修订后整改台账为 32 已整改、0 未整改。历史证据/审批原文未变 | 不批准 H-10/H-11、DEC-005/006 最终阈值、冻结、软件验收；不以四条定向断言冒充全量数学验证 |

继续按 [AUDIT.md](AUDIT.md) 与 [AUDIT_PROCESS.md](AUDIT_PROCESS.md) 各自维护文件审计三态和问题整改两态，原 `origin/**` 未修改。


**E-DOC-027（2026-11-08，先审计）**：在固定输入 `6c22fe7448cc9f3aed2c1f9f42766208e3a3e7fe` 按 AUDIT 核查历史字段、版本矩阵、LLD 索引与相关设计/测试材料，确认 AF-032～036 尚需文字/治理修订，新增 AF-062（索引误称全量登记）；已在 AUDIT_PROCESS 先标「未整改」。AF-037～061 的真实软件/数值/授权证据仍缺，保持开放。本轮审计未执行 Bash/CMake/CTest/MATLAB、未读取或写入 MAT、未实施 C++。整改及静态复核应另提交，不在发现时声明结案。


**E-DOC-028（2026-11-08，先整改后定向复核）**：对应审计记录 E-DOC-027，源文件修订固定提交 `3e3ad2a`，版本登记修订复核提交 `2d608b9`。AF-032～036、AF-062 的 **6 项纯文档目标已修改并静态复读**：当前有效 save 与历史 `HistoricalOnly` 区分、MissingVariable、无需归因、36 个固定身份登记与 50 审计路径的统计用途。静态核对 **66 REQ / 66 TC / 98 P** 唯一、27 设计 DAG 边无环；版本矩阵 **36/36 SHA 对齐指定快照**。当前整改台账 **38 已整改、25 未整改**；剩余 AF-037～061 仍需真实实现/测试/证据/批准。无生产 C++、Bash/CMake/CTest/MATLAB、MAT 写入或独立 L3/性能执行；不声称软件已通过。


## 8. 本轮新一轮审计与文档整改证据（2026-11-08）

| 证据 | 输入与结果 | 严格限制 |
|---|---|---|
| E-DOC-029（先审计） | 固定输入 `3492067`；GitHub 只读结构检查：58 正文 REQ + 8 REQ-DC = 66，TC=66（含 8 TC-DC），P=98，HLD DAG 27 边，原 45 .m/582 .mat；独立探针之外未发现生产业务 C++ 或 tests/*.cpp。发现 AF-063～066 并先登记 `795afb7` | 未运行 Bash/CMake/CTest/MATLAB，未实际解码 MAT 或生成独立 L3；静态一致性不等于运行通过 |
| E-DOC-030（修订复核） | `docs/TEST_PLAN.md` v0.8、`docs/AUDIT.md` 在固定提交 `edd56c99dc04e3774788699ed61c26d3bad3c465` 进行 **AF-063～066 的 4/4 定向静态复读**，已区别不存在的 SOURCE_INVENTORY 与已建追踪/归档文档，原 66 TC/98 P/27 边保留 | 只有文本与编号复核，未新建测试/读取器，不批准 DEC-005/006/008、H-10/H-11，也不冻结 SRS/HLD/LLD |

状态以 [AUDIT.md](AUDIT.md) 和 [AUDIT_PROCESS.md](AUDIT_PROCESS.md) 为准：新发现四项文档目标已按“先登记、后修改、再复核”结案；**AF-037～061 的 25 项独立阶段目标仍开放**。


### E-DOC-031（2026-11-08：第一轮阶段性整改，非软件测试）

- 固定检查输入 `b75998bc8fc6644d76050bf88d45c75887ce2006`，源文档修订/定向复核提交 `6a1867a4b36ad71b1a523c4b3ed7372e276ece5d`；`docs/PLAN.md` blob `cda72332eb32a77a5c353929011dcc47663d31d4`，`docs/design/README.md` blob `021174c363d18d81773acbc8f6b61364a9059faa`。
- AF-052：98 P 唯一、前置依赖存在且无环、REQ/TC/AC 与 SRS 98 条反向映射一致；66 REQ/66 TC、50 条 PLAN 相对文件引用均可定位；HLD 27 条设计边无环；M0～M5 在本文件 §3 **全部未通过**。此次仅 GitHub 连接器只读静态核对，未运行 Bash。
- AF-059：LLD 索引新增 10 模块签名候选、唯一合同归属、TC/L3 入口及尚缺代码/验证/批准的清单。定向文本复读 **2/2** 通过，不批准任何编码合同或 DEC-005/006/008。
- 最新 `AUDIT_PROCESS` 的 **67 项中 44 已整改、23 未整改**。遵照“剩余不为零即退出”，本轮在此停止，没有开展新的审计或第二轮整改；仍缺独立 L2/L3/L4、生产 C++/CTest、Reader/Writer、性能及正式人类签署。未改变 H-10/H-11、`origin/**`、历史审批/报告，也未执行 MAT 写入或 MATLAB/Octave。


### E-DOC-032（2026-11-08：AF-037～041 设计合同部分整改）

- **固定修订/复核输入**：`306b9ac4e8c832344d703b8d29d371710055ae86`；源文件 `docs/REQUIREMENTS.md`（SRS v0.14）、`docs/ARCHITECTURE.md`（HLD v0.9）、`docs/OBJECT_MODEL.md`（v0.6）、`docs/design/contracts.md`（v0.3）、`docs/design/cli.md`（v0.3），以及 PLAN/TEST_PLAN 中的 SRS v0.14 引用。
- **实际完成**：新增 AC→候选 LLD 固定身份门禁、Writer 二次路径检查/Adaptive 已选失败纪律、对象与唯一签名来源表、Result/variant 未决错误语义表、CLI 输入/RunId/JSONL 负例矩阵。五处目标文档锚点定向复读均可定位；**此为文档推进，不是冻结或验收**。
- **待完成**：AF-037～041 的正式批准与实际编码/ABI/负例/CTest、独立 L2/L3/L4 及其它 AF 阶段目标。AUDIT_PROCESS 当前仍 **44 已整改、23 未整改（67 总项）**；由于尚有未整改，按条件在第一轮停止，不开展新审计与第二轮整改。
- **未执行** Bash/CMake/CTest、MATLAB/Octave、MAT 写入或独立数值/性能实验；`origin/**`、历史审批 JSON/reviews、H-10/H-11 均未改。


### E-DOC-033（2026-11-08：第一轮整改的设计证据，未执行软件测试）

- **目标与固定来源**：从 `AUDIT_PROCESS.md` 的 23 个开放目标选取 AF-042/047/048/049/050/051/060，使用 `0dccbff62862983efd9810bf086dec759b866602` 和 `f846e7b96a1b75b5d1d29621b4a295fd6504bade` 两个源修订提交。七个模块/测试参考合同新增明确固定夹具、错误期望、来源信息与测试关联；定向静态复读 **7/7**，不代表七项验收通过。
- **实际覆盖**：MAT 安全/列主序、八面体层数和 N/3N 抽样、轨迹和正根、八次聚合/Manifest、501 点分析和缺字段、66 TC 的夹具规划、L2/L3/L4 有效样本与身份字段；0.01/0.1 仍仅**已选首轮宽松筛查**而非正式 DEC-005。
- **当前证据边界**：没有运行 Bash/CMake/CTest/MATLAB/Octave、MAT 逐值/独立 SHA-256、L3/L4 生成、生产 Reader/Writer、异常负例、性能实验或 MAT 写入。原 `origin/**` 和历史审批文件不变。真实软件验收及人类签署仍缺；H-10/H-11 未改变。
- **第一轮结论**：整改台账维持 **67 项＝44 已整改＋23 未整改**；文件三态 **50＝27 已审计＋23 正在审计**。依停止条件不进行新 AUDIT 扫描和第二轮整改。


### E-DOC-034（2026-11-08：定向源码数学检查、AF-067 来源 SHA 纠错）

- **原只读源**：`franzeigen.m` blob `6fe556fd038b983f8d80aac5c76ee2ea11c08f38`；Original KvD `d48048f37b859bfb19402ca85b0f4c20e76afbfd`；Modified KvD `9ffdac1baaecac06db20bcc6849bf4cdd165a484`；Adaptive oneit `48e51bcd52fb8f27cf35b3fce38b28507af36e55`。限定为源码静态/代数核对。
- **设计修订**：固定提交 `d5265cc576e1c66ba1c63f8621ff1ad91d981195` 将 KVD/Adaptive 可判别输入、负均值近度符号、旧 t 计算 ω、MSE 与非均匀近度覆盖写入 LLD、Golden 提案、TEST_PLAN。没有独立生成逐轮 L3 或执行测试。
- **新问题 AF-067**：在输入 `d5265cc` 发现 1/19 原始源 SHA 仅 39 位，在 `86e3755` 先登记；`18ddb7198490e9d2ff49ee2f66e8b3a4945ee58a` 修正 JSON 元数据。该提交上按 JSON 声明的来源提交 `ddb4b15e78dbd617d5d083d80c2d75aa47f14464` 核验 **19/19 文件 SHA 一致、12/12 AN-G 编号完整唯一**；原候选数值未修改。AF-055 独立 L3 仍缺。
- **退出判定**：67 项既有目标 + AF-067 = 68 项；**45 已整改、23 未整改**。尚无真实 C++/CTest、L2/L3/L4、正式容差或人类批准；不借来源核对声明数值 PASS。已按要求停止，不开始第二轮审计/整改。


### E-DOC-035（2026-11-08：恢复未办结第一轮的静态治理整改）

- 实际复查目标分支原有 68 个 AF 原子目标：45 已整改、23 未整改。23 条阶段目标尚缺真实实现、独立参考或批准，不因描述修改而关闭。
- 本轮依据用户最新要求继续可完成的整改，不将旧审计中“未整改即停止”的语句作为现行执行指令。新发现 AUDIT 末尾 §11/12 重复、AUDIT_PROCESS §10 缺号及滚动证据未续记，先登记 AF-068～070，再修订并复读源文件。
- 尚无新的 H-10/H-11 具体生产或运行授权证据；未运行 Bash/CMake/CTest/MATLAB/Octave，未写原 MAT、未产生独立 L2/L3/L4，也未批准正式基线。原 `origin/**` 保持只读。


### E-DOC-036（2026-11-08：AF-068～070 定向复读结案）

- 新问题首次登记提交 `6a54726`，目标修订固定输入 `fa85cc638e58638768506b25c4b375566f6e8379`：审计计划章节 1～16、整改台账章节 1～13 顺序完整，滚动进度 E-DOC-035 明确继续处理的当前原则；只读静态检查 **3/3 通过**。
- AF-068～070 仅在提交修订并复读后结案；现台账 **48 已整改、23 未整改（共 71 项）**。不代表剩余阶段目标的数字参考、代码/测试/批准可提前关闭。


### E-DOC-038（2026-11-08：AN-G 候选局部复核与 AF-071 结案）

- 改动提交 `a27b092421a0465b10fb46054d158ab06a2c729f`：补 [AN-G 静态公式记录](reviews/AN_G_STATIC_FORMULA_REVIEW_2026-11-08.md) 及 CLI v0.4、sampling v0.5、Golden v0.4、TEST_PLAN v0.10 的对应设计边界；逐个检查八个手算候选 ID、未核验的四组、零/负噪声下 3N 抽样纪律及非实测声明。**并非** 8/12 独立 L3 已验收。
- 控制矩阵 §6.1～6.5 的静态连续性通过（AF-071），问题台账转为 49 已整改/23 未整改，文件审计含新增公式记录共 51 路径。
- 尚未获得 H-10/H-11 运行权限或 Ivanic 原文/勘误与最终批准；无 MAT 逐值、业务 C++/CTest、独立 L3/L4、MAT 写回读或性能数值。原始 `origin/**` 保持只读。


### E-DOC-039（2026-11-08：45 文件 SOURCE_INVENTORY 与 MIGRATION_MATRIX）

- 固定提交 650546a06ff7bb89b8c6bf29cac48f712ed2f2fa 建立两份新来源/迁移文档，45 个源 .m 文件与 Git 树、SRS、OBJECT_MODEL 的实际路径集合一一对应，另核 MAT 文件数量 582；PLAN v0.8 / TEST_PLAN v0.11 改为引用已建静态来源。
- 只读静态检查路径、Git SHA 格式、行数、文内相对链接通过；没有真实源码移植、原 MAT 独立逐值 SHA-256、CMake/CTest/数值 L3/L4/性能或冻结批准。P0.6 仅文件级来源清单建成，方法级与真实验收仍开放。


### E-DOC-040（2026-11-08：AF-072～074 函数级来源及治理压缩闭环）

- 初始审计输入 `fd05af4`、发现登记 `d40b149`；45 个原 .m 的函数声明逐文件只读扫描结果：44 文件各 1 函数，`SHRotate.m` 6 函数，**50 个定义**。目标文档修改提交 `150b975`，45 个文件级表行与 6 个 SHRotate 具体声明行定向检查通过；**无独立数学数值或 C++/CTest**。
- 精简 `AUDIT.md`、`AUDIT_PROCESS.md` 采用固定修改提交 `ae831be3a0901d69c0897044fd339fda38468963`，两文件合计 33347→17073 字符，保留 28 AU、54 路径三态、75 AF 两态（当时 49 已整改、26 未整改）；静态复核后 AF-072/073/074 标为已整改，现为 **52/75 已整改、23 未整改**。
- 只读源不变；在 H-10/H-11 暂缓状态下未编译、执行 Bash/CTest/MATLAB/Octave、写 MAT、生成独立 L3/L4 或签署正式基线。23 个阶段性目标仍按原 ID 推进，不伪造 PASS。


### E-DOC-041（2026-11-08：原 MAT 数据字典静态集合及 AN-G08 z90 源码公式）

- **原 MAT Git 元数据对照**：在固定树 `bffd10c` 上只读比较 `origin/**` 下 582 个 `.mat` 文件的相对路径/字节大小，与 [MAT_DATA_SCHEMA](MAT_DATA_SCHEMA.md) 的 582 行逐项匹配，**路径/字节 582/582 一致**；表中变量名与 MAT basename、MAT v5/zlib/double、`dims×8=data_bytes`、FNV64 字符串格式和 `READ_OK` 状态检查范围错误 **0**。五种 shape 计数为 `10×1:80, 10×3:480, 35×1:4, 600×1:14, 600×4:4`；这是旧读取报告的**文本/目录元数据交叉审计**，**未重新解压 MAT/独立 SHA-256/逐值验证**，AF-056 仍未整改。
- **AN-G08 z90 公式静态复核**：修订固定提交 `ee3d9c9bb42e6d71f9808510402d1394e385647d`，详见 [专门审计报告](reviews/AN_G08_Z90_STATIC_CONVENTION_2026-11-08.md)。由 `SHRotate.m` R3 九项排列及 `kugelflfunc2.m` 的 sin/cos 相位在 `φ→φ+π/2` 特例下得到与候选 JSON 一致的 R3/R5 和三组系数；`transfor_koeff.m` 的左乘方向已显式限定。**没有**用独立数值程序验证一般 R5/全部递推、Ivanic 论文/勘误或生产 C++/CTest，故 AF-045/055 均继续「未整改」。
- H-10/H-11 原授权未改变，`origin/**` 没有变更；此证据只表示静态元数据/代数核验范围，不能推导 MAT 解码或软件验收通过。


### E-DOC-042（2026-11-08：分析分段/平滑忠实性 AF-075/076）

- 根据只读 `graph_agent_in_sphere_adaptive_matched_filter_many_parameter.m` 和 `graph_agent_in_sphere_adaptive_matched_filter_test.m`，先登记 AF-075/076，随后分别修订 `analysis_plotting.md` v0.6 与 `TEST_PLAN.md` v0.12，保留四段旋转局部平滑/边缘零值/裁剪、平移全 600 行（或未平滑）后连续绘制 50:550 的**不同执行域**；见 [审计纠偏](reviews/AUDIT_ANALYSIS_SEGMENT_SMOOTHING_CORRECTION_2026-11-08.md)。
- 定向复读输入 `d8e7e64e5affd97130a2b7436a039e4681de502d`，§5.1/5.2/5.2.1/5.3 顺序连续，保留 66 项 TC 编号及 L3 未执行说明。修正最初扫描报告中关于“相同内部裁剪后的均值必不同”的错误断言，不把纠偏当成 L3 PASS。
- AF-075/076 文档层关闭；AF-050/051 真实 Analyzer、独立曲线、CTest 及 DEC-005/008 未通过。现台账 77 项（54 已整改、23 未整改），审计路径 56（33 已审计、23 正在审计）。未变更任何 `origin/**`、生产代码或授权。


### E-DOC-043（2026-11-08：分析错误协议的单一候选映射）

- 发现 `analysis_plotting.md §6` 的 `AnalysisErrorCode` 七个枚举没有加入原 `contracts.md §3.1` 统一退出码单一来源，先登记 AF-077 未整改，再于 `e3c4c6794b3d2b562ebc39e92251eaaa52f44c34` 同步 contracts v0.4、analysis v0.7、CLI v0.5、TEST_PLAN v0.13。审阅确认 **7/7 候选错误码有唯一来源和候选 stage/exit**：缺文件/变量 read(3)，shape decode(4)，分析内部状态 analyze(5)，非法 CLI 参数 parse(2)；底层 `ErrorDetail.source_error_code` 不得丢弃。
- AF-077 的静态合同一致性复核通过并结案；DEC-008、实际解析/分析器、错误传播、JSONL 和 CTest 仍须独立授权与测试。现为 78 个 AF（55 已整改、23 未整改）；旧 66 TC 不增加/伪造 PASS，原始数据未变。


### E-DOC-044（2026-11-08：索引漂移及严格反向轨迹契约）

- 新发现 AF-078/079/080 先以未整改登记；定向修订提交 `02a2a17fb49c911cdadd978033d14127cf48122d` 将 LLD 索引的旧“50 路径”与错误枚举归属修正、将基线矩阵的 36 文件固定快照和实时审计范围严格分开；`07d3abbd41cc6caa3d3fb57b3b5064ed0448fd6a` 将轨迹严格反向的原 rotvec **确定返回 0** 及四个手算输入候选写入 `trajectory.md` v0.4 / TEST_PLAN v0.14。
- 只读原 `drehvektor_aus_zwei_vektoren.m`、`trajektorie_aus_punkten.m`、`getnear_in_kugel_Drehung.m` 的式子用于静态核对；TR-C01 的 Rᵀ 符号、TR-C02 的反向丢失 π、TR-C03/04 的 near 4/3 与 4/5 均明确是数学**待独立 L3 复核的候选**，没有声称原 MATLAB/生产 C++ 已运行。
- AF-078～080 只关闭已发现的文档矛盾；目前 81 个 AF：58 已整改、23 未整改；57 文件：34 已审计、23 正在审计。H-10/H-11 仍暂缓，DEC-005/006/008 尚未签署；`origin/**` 保持不变。
