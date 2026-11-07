# 实施进度与证据登记（PROGRESS）

> **滚动证据登记 v0.1（非需求、架构或编码规范）。** 此处是里程碑状态、DEC 处理、实际运行和批准记录的唯一当前入口；历史证据记录不得反向冒充当前分支的冻结 SHA。

> **2026-11-06 审批修订补充**：已新增 [HAR-2026-11-06-001 审查材料包](approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md)，把 H-01～H-16 分为已选技术路线、只供起草及暂缓执行三类，并提供具体静态源码证据、TC/Golden 对照和 M0～M5 候选退出证据。这是审查文档成果，**未改变 P/TC/AC 的验收状态**，也未解除全部 false 的执行权限。

> **项目长期决定**：[不运行 MATLAB 的数值证据政策](golden/NO_MATLAB_EXECUTION_POLICY.md) 已根据项目所有者的最新明确要求写入 SRS/TEST_PLAN/LLD/AGENTS：原 .m **只读静态审查**，L1 原 MATLAB 运行逐节点 Golden **不再作为任何冻结/验收阻塞**；下一步用 L2/L3/L4，保留真实 C++/CTest、DEC-005/008、版权、数据保护与人工签署。**本决定不是生产实施或运行测试授权。**

> **当前状态：审计及设计草案阶段；全部软件 REQ/TC/P/M0–M5 的验收状态仍依据实际执行证据，未宣告正式通过。** 版本与规范冻结状态见 [DOCUMENT_BASELINE_REGISTER.md](DOCUMENT_BASELINE_REGISTER.md)，`reviews/*` 的历史 SHA 是过往检查点，不能作为当前 HEAD 的动态通过证明。 本文件只记录可定位的实际子成果和未决项。验收规则以 [REQUIREMENTS.md](REQUIREMENTS.md)、[TEST_PLAN.md](TEST_PLAN.md) 和 [PLAN.md](PLAN.md) 为准；不得把已有文档、matio 独立探针或静态集合检查折算为业务 C++ 功能完成率。

**2026-11-07 项目所有者最新决定**：仓库根 [MIT LICENSE](../LICENSE) 已获明确授权创建，普通版权审核不再阻塞开发；球谐必须以 Ivanic–Ruedenberg **1996 原始论文＋1998 勘误**为依据并**纯 Eigen 重写、禁用 sdla**，等用户上传两份论文后实现；CLI 已删除自定义输出根参数，输出固定 `output/<RunId>/`，RunId 来自独立 OS 随机源；Result/Status 采用轻量封装、ErrorStage 稳定枚举、JSONL；实验使用独立配置、失败状态不提交、八次聚合不改分母，Manifest 区分 Complete/Incomplete/Failed；`atol=0.01,rtol=0.1` **仅作为已确认的首轮探索性筛查**，最终 DEC-005 验收门槛未冻结。详见 [最新技术决定](decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md)。这不是生产代码或测试执行授权。

**文档审计入口**：[AUDIT.md](AUDIT.md) 已汇总历轮用户提出的修改和 Markdown 评审意见，并以“空白、正在审计、已审计”跟踪逐文件审计。此表只记录文档审计过程，**不构成** C++/CTest/数值通过或任何人类批准；具体可推迟的人类事项仍集中在 [TODO.md](TODO.md)。

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

> **Golden 证据等级最新状态**：依 [E-DOC-007](golden/NO_MATLAB_EXECUTION_POLICY.md)，L1 原 MATLAB 运行**永久非强制、非阻塞**；L2 作者 MAT 存在但独立 SHA-256/逐值复核仍缺，L3 的 AN-G01～12 仅为候选（完整逐轮/递归/一般输入仍待独立参考），L4 尚无新执行统计。故 KvD/Adaptive/RNG/SH/Session 的 AC 仍因**独立参考/真实 C++/CTest/容差批准不足**而未验收，**不是因为缺 L1**。候选 0.01/0.1 尚未批准。

**本阶段的人类输入已集中在 [docs/TODO.md](TODO.md)**：论文 PDF、生产代码及运行许可、DEC-005 最终阈值、DEC-006 真实性能预算、基线签署与发布均可在相应阶段办理；**当前需求分析只修改文档，未获得实际 C++/CMake/CTest/Bash/MAT 写入授权**。DEC-004、MIT、Eigen-only、无自定义输出根、Result/ErrorStage/JSONL、L1 退出均**已选定，不应重复申请**。此前 E-DOC-xxx 的版本/SHA 均是历史证据，不代表当前文件版本。

## 5. 未解决决策与验证阻塞

| 决策/阻塞 | 当前实际状态 | 下一步必须提交的材料 |
|---|---|---|
| DEC-004 历史字段处理 | **项目所有者已经选定规则，非继续待人类选型**：新 Runner 仅保存当前有效 save 字段，旧 582 MAT 完整只读；额外字段 HistoricalOnly，分析必需字段缺失明确失败 | [DEC-004](decisions/DEC-004_HISTORICAL_MAT_FIELDS.md) 已记录；实际 Runner/Analyzer、L2/L3 负例及 TC/CTest 尚未实现，历史成因未知不阻塞开发 |
| DEC-001/002 当前 MAT 格式和类型 | 归档事实已有 582/582 证据；支持范围尚待审阅 | 当前数据限定 v5/double 与拒绝未知类型的具体 Reader/负例 |
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
