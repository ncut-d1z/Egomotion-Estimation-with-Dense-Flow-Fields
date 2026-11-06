# 实施进度与证据登记（PROGRESS）

> **当前状态：审计及设计草案阶段；所有正式软件需求、P 任务及里程碑均未宣布验收完成。** 本文件只记录可定位的实际子成果和未决项。验收规则以 [REQUIREMENTS.md](REQUIREMENTS.md)、[TEST_PLAN.md](TEST_PLAN.md) 和 [PLAN.md](PLAN.md) 为准；不得把已有文档、matio 独立探针或静态集合检查折算为业务 C++ 功能完成率。

## 1. 当前工作范围与证据

- 仓库：`ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields`
- 开发分支：`dev-plan-P0-P1-mat-dictionary-probe`（未合并至 `main`）。
- 原始数据范围：`origin/egomotion_estimation_dense_flow_fields_matlab_scripts/`；**45 个 .m、582 个 .mat，原始数据只读**。
- **已验证子能力**：既有 `src/mat_data_dict_probe.cpp` 的独立实测报告记录 Ubuntu 24.04 / GCC 13 / matio 1.5.26，582 个 MAT、582 个变量均 `READ_OK`、失败 0；详情及每个文件的属性见 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md)。这是归档数据格式证据，**不是**正式 Reader/Writer 或 TC/CTest 通过。
- **本轮实际检查**：使用 GitHub 连接器按固定 commit SHA 读取文档/递归源树，运行 JavaScript 静态断言并将结果记录在 [2026-11-06 静态追踪检查](reviews/STATIC_TRACEABILITY_2026-11-06.md)；实际观察 66 REQ、66 TC、98 P、45 .m 双向映射、26 条 HLD target 边，错误及环均为 0。尚未在容器运行 Bash 脚本（外部 GitHub DNS 无法解析），也未运行任何业务 CTest。

## 2. 可核验子交付物

| 证据 ID | 工件/记录 | 可以支持的事实 | 不能据此推断 |
|---|---|---|---|
| E-DATA-001 | [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) + `src/mat_data_dict_probe.cpp` | 指定源参数 SHA `48c590ba2564e4bdd293d2cbecd2eef83b291452` 的 MAT v5/zlib/double 582/582 解码证据；已记录 FNV64、变量、dims、读取状态 | 新数据类型兼容、正式 Reader 运行、业务输出写回读、MATLAB 算法数值等价、P10.4 独立复核 |
| E-DOC-001 | [2026-11-06 追踪核查](reviews/STATIC_TRACEABILITY_2026-11-06.md) | 指定 Git 提交的文档内容及源树集合对应无缺项、无 P/DAG 环 | `bash scripts/check_docs_traceability.sh` 已执行，真实 CMake target 无环、CTest 通过 |
| E-DOC-002 | [design/cli.md](design/cli.md) | 命令/参数类型/错误码/诊断输出的候选 LLD 已存在 | CLI 已实现、已获批准、错误/参数 schema 已冻结 |
| E-DOC-003 | [OBJECT_MODEL.md](OBJECT_MODEL.md) + [design/sampling_noise.md](design/sampling_noise.md) | Bias 场景编排、`FlowSynthesizer`、`sampling::randomUnitDirection` 的职责和命名已协调 | 对应头文件/测试已创建、随机数调用序列已数值验证 |

## 3. 实施任务与里程碑状态

| 阶段 | 有证据的局部成果 | 阶段状态 | 下一验收证据 |
|---|---|---|---|
| P0：源资料/SRS/HLD/LLD 审计 | 45 .m 清单、582 MAT 类型字典；文档层 REQ/TC/P/映射静态一致 | **进行中，未验收** | P0.1–P0.8 各项审批/来源身份、错误与 CLI 合同、版权决定 |
| P1：运行环境、CLI、MAT | 仅独立 matio 数据字典探针的实测子能力 | **部分子能力可核对，P1 仍未验收** | 生产 Reader/Writer、RAII、输出保护、正反例、写后回读、真实 CMake/CTest |
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

**日期/资源状态**：所有里程碑的具体日期、工时、预算、CPU/RAM 指标均**待项目所有者确认**。不根据文档任务顺序擅自承诺日程。正式通过条件还需要相应 REQ/TC/AC 和证据编号。

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
| DEC-003/007 第三方版权 | **未关闭** | Rodrigues、SHRotate 来源/许可链、可重写范围和项目所有者决定 |
| DEC-005 数值容差 | **未关闭** | 条件数/数值尺度分组、固定独立 MATLAB Golden、逐字段误差及批准门槛；人类可通过 [HTML 审批页](approval/HUMAN_APPROVAL_REQUEST.html) 记录决定并自行保存 JSON（不自动等同正式批准） |
| DEC-006 平台与资源 | **未关闭** | 实际 Debian/Ubuntu 业务构建/测量矩阵；不使用探针结果替代 |
| DEC-008 CLI/CTest/Golden 映射 | **未关闭** | 批准 [cli.md](design/cli.md) 参数及错误序列化，真实测试文件/注册名、Golden identity、执行证据 |
| Bash 现场检查 | **尚未执行** | 完整检出并运行 `bash scripts/check_docs_traceability.sh`，保存 stdout/stderr/exit/SHA；本轮只有 E-DOC-001 的连接器等价检查 |
| MAT 业务读写 | **尚未实现** | P1.3/1.4/1.7/1.9/1.10 的生产接口、路径安全及端到端回读证据 |

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

每次新增证据只更新对应状态，不以补齐文档或估算百分比改变未验收的 98 个任务。严格按 [AGENTS.md](../AGENTS.md) 禁止重写历史、未经授权改动原始 MAT 或自动合并分支。
