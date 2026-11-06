# 人类审批与执行授权申请表

> **状态：待人类填写／待逐项审批。** 本文件仅是申请材料；16 项均默认为**未批准、未授权、未冻结、未验收**。AI 不得代替人类审批、签名、分派人员、承诺预算或关闭 DEC。
>
> 权限依据：[AGENTS.md `10](../../AGENTS.md)；决策编号：[REQUIREMENTS.md `8](../REQUIREMENTS.md)；架构/ADR：[ARCHITECTURE.md](../ARCHITECTURE.md)；详细设计：[design/README.md](../design/README.md)；测试：[TEST_PLAN.md](../TEST_PLAN.md)；任务：[PLAN.md](../PLAN.md)；证据/状态：[PROGRESS.md](../PROGRESS.md)。

## 0. 申请信息（人类填写）

| 字段 | 填写内容 |
|---|---|
| 申请编号 | HAR-2026-11-06-001（查找用，不是审批编号） |
| 所在仓库 | ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields |
| 申请分支 | dev-plan-P0-P1-mat-dictionary-probe |
| 本表对应 commit SHA | 待填写：________________ |
| 项目所有者/批准人 | 待填写：________________ |
| 其他审查人及角色 | 待填写：________________ |
| 审批日期、时区 | 待填写：________________ |
| 本次审批范围（编号） | 待填写：________________ |
| 关联证据 | [MAT_DATA_SCHEMA.md](../MAT_DATA_SCHEMA.md)、[静态核查](../reviews/STATIC_TRACEABILITY_2026-11-06.md) |

**如何填写**：对 `1 中 H-01～H-16 **分别**在 `4 的决议表中选择“批准／退回修改／暂缓／拒绝”，填写实际批准人、日期及时区、**所批准的文件/commit SHA**、适用范围、条件和证据。留空或未完整签署一律仍为“待批准”。**批准候选设计不授权运行；授权执行不代表测试通过；测试通过不等于冻结或发布。**

## 1. 人类需要逐项处理的事项

下列每项最高权限为 **[HUMAN-EXECUTE/APPROVE]**；其候选方案可由 AI 按 **[AI-DRAFT + HUMAN-APPROVE]** 先行起草，但必须保持“候选/未冻结”。

| 编号 | 事项与资料 | AI 可以事先准备 | 人类必须决定/授权 |
|---|---|---|---|
| H-01 | **DEC-005 数值容差**；[REQ-NF-NUM-001](../REQUIREMENTS.md)、[TEST_PLAN](../TEST_PLAN.md)、各算法 LLD | 误差、条件数、退化样本、逐模块 atol/rtol 和统计噪声验收**候选** | 数值验证负责人审查/提案，项目所有者批准阈值、统计方法及异常口径 |
| H-02 | **DEC-003/007 第三方版权与再分发**；原 `rodrigues.m`、`SHRotate.m`；[harmonics.md](../design/harmonics.md)、[trajectory.md](../design/trajectory.md) | 作者/版权头/可能的独立重写来源调查 | 权利审查人员和项目所有者确定可复制、移植、再分发的范围；AI 不得代做法律授权结论或创建 LICENSE |
| H-03 | **DEC-008 CLI/CTest/Golden 身份映射**；[cli.md](../design/cli.md)、[contracts.md](../design/contracts.md)、[TEST_PLAN](../TEST_PLAN.md) | 命令、RunId、错误映射/诊断格式、测试实体/Golden identity 候选 | 批准合同、真实测试文件和 CTest 名称、Golden 身份及冻结准则 |
| H-04 | **DEC-006 Debian/Ubuntu 与性能预算**；[SRS `8](../REQUIREMENTS.md) | 工具链版本、CPU/RAM、wall/CPU/RSS、构建时间测量方案 | 确定支持发行版、编译器、预算、资源上限和性能阈值 |
| H-05 | **SRS 义务/AC/优先级/范围的实质变更**；[REQUIREMENTS.md](../REQUIREMENTS.md) | 影响分析、追踪矩阵及候选修改 | 项目所有者批准需求义务和验收门槛变更；纯文字/引用修正不属于实质批准 |
| H-06 | **HLD/LLD/ADR、CLI/错误合同冻结**；[ARCHITECTURE.md](../ARCHITECTURE.md)、[OBJECT_MODEL.md](../OBJECT_MODEL.md)、[design/](../design/README.md) | 类/错误/状态/参数候选及开放风险 | 对指定 SHA 的设计、ADR、错误接口、冻结清单和例外作人类批准 |
| H-07 | **DEC-004 历史 MAT 额外字段归因**；[experiments.md](../design/experiments.md) | 对照当前 save、历史归档及分析输入里的 `alleadap*`、`rottransratio` | 数据审计人/所有者确认来源归因、统计口径、是否支持兼容导出 |
| H-08 | **DEC-001/002 MAT 支持及 FNV64/SHA-256 身份标准**；[MAT_DATA_SCHEMA.md](../MAT_DATA_SCHEMA.md)、[mat_io.md](../design/mat_io.md) | 保留 582/582 READ_OK、FNV64；起草 SHA-256 补充方案 | 项目所有者批准支持类型、错误拒绝边界、身份哈希强度与独立审核 |
| H-09 | **真实 MATLAB Golden 生成/存储**；[TEST_PLAN `5.2](../TEST_PLAN.md) | 固定输入、采集脚本、结果字段、来源 SHA 与参考 SHA-256 规范 | 提供合法执行环境、授权运行/保管/再分发，并批准 Golden 来源身份 |
| H-10 | **生产 C++、CMake、CTest、负例、MAT 写回读及性能测试**；[PLAN P1–P10](../PLAN.md) | 代码/测试计划、命令、数据隔离及日志模板 | 人类明确可修改文件/分支、可运行命令、设备/数据/输出边界；AI 可在授权环境实施，不能自行宣布验收 |
| H-11 | **Bash 静态核查脚本现场执行**；[check_docs_traceability.sh](../../scripts/check_docs_traceability.sh)、[TEST_PLAN `8.1](../TEST_PLAN.md) | 连接器只读等价检查、命令/日志模板 | 提供或授权完整 Debian/Ubuntu 检出环境执行 Bash；保存 stdout/stderr/exit/HEAD SHA；不能以连接器等价检查代替 |
| H-12 | **MAT 原子发布/符号链接竞态与路径保护**；[mat_io.md](../design/mat_io.md) | 不覆盖、`openat/O_NOFOLLOW/O_EXCL`、写回读/负例的设计候选 | 人类授权测试并按 P1.9 审核真实结果，才能批准安全合同 |
| H-13 | **KvD/Adaptive 失败与后验提交策略**；[kvd.md](../design/kvd.md)、[adaptive_mfa.md](../design/adaptive_mfa.md) | 原 MATLAB NaN/Inf/迭代行为与 commit/rollback 的差异候选 | 数值验证者/所有者批准失败时状态提交、停止/继续及其与原版的差异 |
| H-14 | **球谐 R5 合法实现/兼容路线**；[harmonics.md](../design/harmonics.md) | R3/R5 索引、递推公式与独立实现候选 | 版权审查后批准实现范围，Golden/DEC-005 核验后才可认定等价 |
| H-15 | **角色、里程碑日期、预算、资源**；[PLAN `6](../PLAN.md)、[PROGRESS.md](../PROGRESS.md) | WBS、角色描述、空白时间/预算字段 | 有权人员指派具体负责人、签署 M0–M5 日期、CPU/RAM/工期/预算承诺 |
| H-16 | **PR 合并、正式发布、许可证、最终基线签字**；[AGENTS.md](../../AGENTS.md) | Draft PR/证据/风险清单、发布候选 | 对具体 PR/commit、许可证适用范围、SRS/HLD/LLD 冻结和发布作明确人类批准 |

## 2. 请人类补充关键决策数据

### 2.1 H-01 / DEC-005：数值容差与 Golden

**空白不代表默认 0 或默认 1e-10。** 有可靠 MATLAB/独立 Golden 及人类审批前，所有行都继续保持待决定。

| 模块 | atol | rtol | 统计/退化判据 | Golden 输入/参考 SHA | 审查状态 |
|---|---|---|---|---|---|
| Franz、Original/Modified KvD（逐轮） | 待填 | 待填 | 待填 | 待填 | 待审批 |
| 实球谐 9 项、R3/R5、近度拟合 | 待填 | 待填 | 待填 | 待填 | 待审批 |
| Adaptive M1–M4、6×6、MSE、Session | 待填 | 待填 | 待填 | 待填 | 待审批 |
| 采样、光流、噪声、RNG | 待填 | 待填 | 待填 | 待填 | 待审批 |
| 轨迹、旋转、球内近度 | 待填 | 待填 | 待填 | 待填 | 待审批 |
| MAT 读写、统计分析 | 待填 | 待填 | 待填 | 待填 | 待审批 |

统计噪声样本量、失败/未定义样本及判定规则：________________。

### 2.2 H-02 / H-14 / H-16：权利与发布

| 请核查/批准 | 人类决定或链接 |
|---|---|
| `rodrigues.m` 的来源/许可证/可移植、修改及再分发范围 | __________________ |
| `SHRotate.m` / R5 的来源/许可证/可独立实现范围 | __________________ |
| MATLAB 归档 .m/.mat 和新增 Golden 的存储/公开范围 | __________________ |
| 项目级 LICENSE：是否需要、具体文本和适用代码范围 | __________________ |
| 法律/来源审查证据（请勿填写凭据、密钥） | __________________ |

### 2.3 H-03 / H-08 / H-09：CLI、错误合同、Golden 身份

| 需要人类确认的合同 | 批准值或退回意见 |
|---|---|
| `cli.md` 的受审 commit SHA、命令/参数/合法范围 | __________________ |
| RunId 自动命名及并发创建 | __________________ |
| JSONL / 键值行 / 其他诊断序列化与转义 | __________________ |
| 负噪声系数/其他未定参数语义 | __________________ |
| `contracts.md` 枚举/错误上下文到 exit 码映射 | __________________ |
| 真实 TC→测试文件/CTest 注册名→AC 与 Golden identity | __________________ |
| FNV64 是否可作为基线，是否必须补 SHA-256 | __________________ |
| MATLAB 环境、Golden 采集脚本 blob SHA、参考数据 SHA-256 与存储路径 | __________________ |

**SHA 概念必须区分**：Git 源树/commit SHA、原 MATLAB 单文件 blob SHA、探针运行参数 `source_sha`、Golden 采集脚本 blob SHA、Golden 文件的 SHA-256。已经记录的探针参数 SHA 不是 Golden SHA。

### 2.4 H-04 / H-10 / H-11 / H-12 / H-15：实际执行与资源

| 执行授权字段 | 人类批准的精确范围 |
|---|---|
| 可修改的分支与文件/目录 | __________________ |
| 是否允许 CMake 配置/编译；OS、GCC/Clang 版本 | __________________ |
| 是否允许 CTest；哪些用例、失败处理方式 | __________________ |
| MAT 写回读/损坏副本/负例授权范围及输出目录 | __________________ |
| MATLAB 实际执行/Golden 生成的输入、工具及存储位置 | __________________ |
| Bash `check_docs_traceability.sh` 现场执行权限 | __________________ |
| 性能/CPU/RAM/墙钟测量权限、预算与资源上限 | __________________ |
| 负责人/审查人、M0–M5 日期/预算 | __________________ |

**安全边界**：无论批准哪些试验，原始 `origin/**` 仍只读；负例只能修改隔离测试副本；本表不授权强推、改写历史、绕过 Git 安全或覆盖历史输出。

## 3. AI 不需要本表批准的纯文档任务 [AI-EDIT]

不改变需求义务/算法语义/验收门槛/冻结状态时，AI 可以按 [AGENTS.md `10](../../AGENTS.md) 直接处理：修正 PLAN 章节引用/PROGRESS 状态、统一 SRS→CLI 交叉链接和 SHA 术语、对齐 HLD 组件表/target DAG、注明 TEST_PLAN 的 Bash“待执行”状态、补全标为候选的错误/RNG/采样器设计、统一 LLD 数量和执行只读 REQ/TC/AC/P/45 .m 静态核查。**任何新增或改变合同内容均属于 [AI-DRAFT + HUMAN-APPROVE]，不可因文档修改已提交就自动批准。**

## 4. 逐项审批表（由人类填写）

**请逐项独立填写**。“批准/退回修改/暂缓/拒绝”的选择、实际审批人/角色、日期/时区、批准对象 commit/blob SHA、范围/条件及依据缺一不可；未完整的行维持待审批。

| 编号 | 人类决定 | 审批人/角色 | 审批日期及时区 | 批准对象 SHA、范围和附带条件 | 审批依据/意见 |
|---|---|---|---|---|---|
| H-01 | 待决定 | — | — | — | — |
| H-02 | 待决定 | — | — | — | — |
| H-03 | 待决定 | — | — | — | — |
| H-04 | 待决定 | — | — | — | — |
| H-05 | 待决定 | — | — | — | — |
| H-06 | 待决定 | — | — | — | — |
| H-07 | 待决定 | — | — | — | — |
| H-08 | 待决定 | — | — | — | — |
| H-09 | 待决定 | — | — | — | — |
| H-10 | 待决定 | — | — | — | — |
| H-11 | 待决定 | — | — | — | — |
| H-12 | 待决定 | — | — | — | — |
| H-13 | 待决定 | — | — | — | — |
| H-14 | 待决定 | — | — | — | — |
| H-15 | 待决定 | — | — | — | — |
| H-16 | 待决定 | — | — | — | — |

**本次明确批准的编号（人类填写）**：________________。

**本次退回/拒绝/暂缓的编号**：________________。

**审批补充说明及下次复核日期**：________________。

## 5. 审批后处理

1. 由人类分别作决定，更新本表并保存**新提交**；AI 核对被批准的文档与 commit/blob SHA、授权环境和范围。不能以“本表已创建”或空白表格当作批准。
2. 已批准的**设计合同**再按批准范围记录在 SRS/HLD/LLD；其他条目继续“候选/未冻结”。批准设计不等于批准运行 CMake/CTest/MATLAB 或写回读。
3. 已授权的**真实执行**另记 Git SHA、实际命令/环境、RunId/数据哈希、stdout/stderr、exit code、Golden/容差、有效/失败样本，再由人类按 AC/TC 验收签字；绝不把连接器静态检查写成 Bash 现场执行。
4. DEC 只在所需证据与人类审批要素完整时改变状态；同步 [REQUIREMENTS.md `8](../REQUIREMENTS.md) 与 [PROGRESS.md](../PROGRESS.md)；未验收的 P0–P10 任务不自动计入完成。
5. 合并 PR、正式发布、新增 LICENSE 与 SRS/HLD/LLD 冻结需分别核对 H-05/H-06/H-16 的**明确许可范围**，不可通过其他批准事项推导出通用发布权限。

---
**创建时 H-01～H-16 全部待人类处理，当前没有任何审批生效。**
