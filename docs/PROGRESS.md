# 实施进度与证据登记（PROGRESS）

> **2026-11-06 审批修订补充**：已新增 [HAR-2026-11-06-001 审查材料包](approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md)，把 H-01～H-16 分为已选技术路线、只供起草及暂缓执行三类，并提供具体静态源码证据、TC/Golden 对照和 M0～M5 候选退出证据。这是审查文档成果，**未改变 P/TC/AC 的验收状态**，也未解除全部 false 的执行权限。

> **当前状态：审计及设计草案阶段；所有正式软件需求、P 任务及里程碑均未宣布验收完成。** 本文件只记录可定位的实际子成果和未决项。验收规则以 [REQUIREMENTS.md](REQUIREMENTS.md)、[TEST_PLAN.md](TEST_PLAN.md) 和 [PLAN.md](PLAN.md) 为准；不得把已有文档、matio 独立探针或静态集合检查折算为业务 C++ 功能完成率。

## 1. 当前工作范围与证据

- 仓库：`ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields`
- 开发分支：`dev-plan-P0-P1-mat-dictionary-probe`（未合并至 `main`）。
- 原始数据范围：`origin/egomotion_estimation_dense_flow_fields_matlab_scripts/`；**45 个 .m、582 个 .mat，原始数据只读**。
- **已验证子能力**：既有 `src/mat_data_dict_probe.cpp` 的独立实测报告记录 Ubuntu 24.04 / GCC 13 / matio 1.5.26，582 个 MAT、582 个变量均 `READ_OK`、失败 0；详情及每个文件的属性见 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md)。这是归档数据格式证据，**不是**正式 Reader/Writer 或 TC/CTest 通过。
- **较早的静态检查**：使用 GitHub 连接器按固定 commit SHA 读取文档/递归源树，运行 JavaScript 静态断言并将结果记录在 [2026-11-06 静态追踪检查](reviews/STATIC_TRACEABILITY_2026-11-06.md)；实际观察 66 REQ、66 TC、98 P、45 .m 双向映射、26 条 HLD target 边，错误及环均为 0。该检查未在容器运行 Bash 脚本（外部 GitHub DNS 无法解析），也未运行任何业务 CTest。
- **审查意见后的文档更正及复核**：参见 [跨文档一致性复核记录](reviews/DOC_CONSISTENCY_RECONCILIATION_2026-11-06.md)。固定提交 `5472c756b983fe002ef6e32143c96b6c6798cde9` 下，GitHub 连接器只读检查 66 REQ、66 TC、98 P、45 原始 `.m`、45/45 双向映射、26 条 HLD target 依赖边和 582 MAT Git 树路径，检查范围内不一致 0；与更正前比较，规范性 REQ/TC/P 表格行完全一致，`origin` Git 子树 SHA 不变。**这不是 Bash 现场执行或任何真实测试**。

## 2. 可核验子交付物

| 证据 ID | 工件/记录 | 可以支持的事实 | 不能据此推断 |
|---|---|---|---|
| E-DATA-001 | [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) + `src/mat_data_dict_probe.cpp` | 指定源参数 SHA `48c590ba2564e4bdd293d2cbecd2eef83b291452` 的 MAT v5/zlib/double 582/582 解码证据；已记录 FNV64、变量、dims、读取状态 | 新数据类型兼容、正式 Reader 运行、业务输出写回读、MATLAB 算法数值等价、P10.4 独立复核 |
| E-DOC-001 | [2026-11-06 追踪核查](reviews/STATIC_TRACEABILITY_2026-11-06.md) | 指定 Git 提交的文档内容及源树集合对应无缺项、无 P/DAG 环 | `bash scripts/check_docs_traceability.sh` 已执行，真实 CMake target 无环、CTest 通过 |
| E-DOC-002 | [design/cli.md](design/cli.md) | 命令/参数类型/错误码/诊断输出的候选 LLD 已存在 | CLI 已实现、已获批准、错误/参数 schema 已冻结 |
| E-DOC-003 | [OBJECT_MODEL.md](OBJECT_MODEL.md) + [design/sampling_noise.md](design/sampling_noise.md) | Bias 场景编排、`FlowSynthesizer`、`sampling::randomUnitDirection` 的职责和命名已协调 | 对应头文件/测试已创建、随机数调用序列已数值验证 |
| E-DOC-004 | [审查后跨文档一致性复核](reviews/DOC_CONSISTENCY_RECONCILIATION_2026-11-06.md) | 8 份方案文档的 CLI/output、RunOutcome、SH R5、MAT 形状来源、HLD 环境/预算与章节引用得到文本统一；固定提交下的 66 REQ/66 TC/98 P、源目录和 DAG 静态断言未发现不一致 | H-03/DEC-005/008 已批准、H-10/H-11 已解除暂缓、Bash/CTest/Golden 已执行、SRS/HLD/LLD 可冻结 |
| E-DOC-005 | [Golden 及容差候选](golden/GOLDEN_TOLERANCE_PROPOSAL.md) + [解析数值 JSON](golden/analytic_golden_candidates.v1.json) | 已静态提出 12 组带固定输入与具体参考数字的解析候选；建议第一轮宽松连续值比较 `atol=0.01, rtol=0.1`，并强制结构/物理不变量；记录了真实 MATLAB Golden 缺口 | MATLAB 已执行或输出已验证、DEC-005 阈值已批准、C++ 通过测试、M0～M5/P/TC/AC 可标记通过 |
| E-DEC-001 | [JSON 审批落实记录](approval/DECISION_IMPLEMENTATION_NOTES.md) | **H-02/H-14 已指定 Sophus 与 Don Williamson Ivanic 方案 A**；H-04 已提供 Ubuntu 24.04.4 VM 快照；H-12 已确定 `output/` 并更新 `.gitignore` | 任意第三方 C++ 文件已复制、Sophus 已安装/构建、MAT 写入/CTest 已执行、Golden/容差已批准、程序可发布 |

## 3. 实施任务与里程碑状态

| 阶段 | 有证据的局部成果 | 阶段状态 | 下一验收证据 |
|---|---|---|---|
| P0：源资料/SRS/HLD/LLD 审计 | 45 .m 清单、582 MAT 类型字典；**H-02/H-14 替代路线选择已形成设计记录** | **进行中，未验收** | P0.1–P0.8 各项审查、许可告知保存/依赖核实、错误与 CLI 合同、Golden/容差决定 |
| P1：运行环境、CLI、MAT | 独立 matio 探针子能力；H-04 指定 VM 环境；H-12 配置排除未来 `output/` 数据上传 | **H-10 暂缓生产代码/运行，P1 未验收** | 生产 Reader/Writer、RAII、路径保护负例、写后回读和真实 CMake/CTest；解除暂缓和授予运行权限后再执行 |
| P2–P9：几何/模拟/求解器/实验/分析 | LLD 草案与 source-to-responsibility 映射 | **代码未实现/未验证** | 对应 C++ 实现、固定输入 Golden、分模块数值误差与通过日志 |
| P10：集成验收与复现 | 规划与证据模板 | **未开始验收** | 逐模块集成、论文复现、P10.4 独立 MAT 审核、性能/完整报告 |

| 里程碑 | 目前状态 | 已有输入 | 缺失的退出条件 |
|---|---|---|---|
| M0 源资料与设计评审 | 未通过 | 源树清单、数据字典、SRS/HLD/LLD 草案、静态核查 | P0.8 审核签署、源身份与版权、CLI/错误范围、未决 DEC 处置 |
| M1 MAT 读写/构建闭环 | 未通过 | 582/582 独立探针记录 | 正式 Reader/Writer、CMake 业务 targets、CTest、路径/变量负例、写回读 |
| M2 Bias 数据生成与统计 | 未通过 | 设计合同 | Franz/KvD/噪声 C++ 实现及 Golden、完整 Bias 证据 |
| M3 Adaptive 估计实验 | 未通过 | 设计合同 | 球谐/Adaptive 求解、状态/周期逻辑、固定输入及逐轮 Golden |
| M4 分析及绘图 | 未通过 | 设计合同 | 分析曲线与输出核验，可选绘图后端的批准 |
| M5 最终交付 | 未通过 | 文件架构与验收规则 | 完整平台/CTest/数值证据、版权决策、维护文档、批准的合并/发布 |

**日期/资源状态**：H-04 已提供一台 Ubuntu 24.04.4/x86_64 VirtualBox 的 `uname/df/free` 快照，物理内存总 8127380 KiB、当时 available 4908696 KiB、Swap 4194300 KiB；这些**不是**批准的资源配额或性能通过阈值。所有里程碑的具体日期、工时、CPU 核数、预算及验收指标仍待决定；不根据文档任务顺序擅自承诺日程。正式通过条件还需要相应 REQ/TC/AC 和证据编号。

## 4. 角色与批准权限（不杜撰人员分配）

| 责任角色 | 当前可确认的职责 | 人员/批准情况 |
|---|---|---|
| 项目所有者/需求批准人 | 决定 SRS、接口、版权使用范围、数值容差、基线冻结和例外 | **尚无本轮签署记录**；不可代签 |
| 源 MATLAB / 版权审核者 | 审查原作者材料、第三方 Rodrigues/SHRotate 权利和移植方式 | **尚未确认具体审核人或批准意见** |
| 架构与 C++ 实现者 | 按 P 任务维护对象合同、目标依赖、代码、提交/PR | 本轮只记录文档维护与连接器检查；正式实施责任人待分配 |
| 数值验证者 | 独立 Golden、统计样本、批准容差下的逐轮对照 | 未指派、Golden 和 DEC-005 未冻结 |
| 测试/集成维护者 | Debian/Ubuntu 构建、CTest、数据/资源记录、静态审计核查 | Ubuntu 探针已有记录；完整工程/CTest 未获验收 |

## 5. 未解决决策与验证阻塞

| 决策/阻塞 | 当前实际状态 | 下一步必须提交的材料 |
|---|---|---|
| DEC-001/002 当前 MAT 格式和类型 | 归档事实已有 582/582 证据；支持范围尚待审阅 | 当前数据限定 v5/double 与拒绝未知类型的具体 Reader/负例 |
| DEC-003/007 第三方版权 | **H-02/H-14 已选定限定替代路径；发布/实现尚未验收** | Sophus `SO3d::exp`、Don Williamson `IvanicSHRotate.cpp/.h`（方案 A）及其 MIT 告知；将来保存复用目录 README/LICENSE、核实 sdla 依赖/源许可与 Golden；整个 MATLAB 归档权利不因 H-02 自动通过 |
| DEC-005 数值容差 | **未关闭** | 条件数/数值尺度分组、固定独立 MATLAB Golden、逐字段误差及批准门槛；人类可通过 [HTML 审批页](approval/HUMAN_APPROVAL_REQUEST.html) 记录决定并自行保存 JSON（不自动等同正式批准） |
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
