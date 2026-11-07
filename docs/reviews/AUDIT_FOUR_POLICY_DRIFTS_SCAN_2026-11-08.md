# 增量文档一致性审计：四处已选决策口径漂移（2026-11-08）

## 固定输入与方法

- 仓库：`ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields`；目标分支：`dev-plan-P0-P1-mat-dictionary-probe`；**审计输入 commit**：`0fa760423532fda691c046f2e4eac5fc37784503`。
- 依据：`docs/AUDIT.md` 的 AU-001～028、三态文件表、问题先登记后整改规则；`docs/AUDIT_PROCESS.md` 的双台账、两态整改规则。
- 方法：GitHub 连接器只读读取现行 SRS、HLD、对象模型、PLAN、TEST_PLAN、PROGRESS、AGENTS、相关 LLD、DEC-004、项目所有者技术决定、Golden 政策与参考、AUDIT 两台账，交叉核对当前设计选择和历史快照语义；另检查 JSON 中 AN-G01～12 身份及部分历史 reviews/审批包。**不是全部原 .m 的逐式重审，不运行 Bash/MATLAB/CMake/CTest、独立 L3 或 MAT 读写。**

## 新发现（先登记为未整改，不凭报告直接结案）

| ID | 严重度 | 文档相对路径 / 原位置 | 核实到的冲突与影响 | 对照的当前权威依据 | 整改目标 |
|---|---|---|---|---|---|
| AF-028 | 中 | `docs/PROGRESS.md:91`，“候选 0.01/0.1 尚未批准” | 目前是当前状态段落，与已有所有者首轮筛查决定矛盾，容易误把已选筛查当仍待审批 | `docs/REQUIREMENTS.md` §8 DEC-005；`docs/golden/NO_MATLAB_EXECUTION_POLICY.md` | 明确首轮已选、最终 PASS 阈值未定。 |
| AF-029 | 中 | `docs/PROGRESS.md:106`，“支持范围尚待审阅” | 当前 DEC-001/002 行否定已选首版 MAT 类型/形状；与 SRS/HLD 不一致 | `docs/REQUIREMENTS.md` §8 DEC-001；`docs/ARCHITECTURE.md` ADR-001 | 纠正已选范围，并保留正式 Reader/Writer/负例等实际缺口。 |
| AF-030 | 中 | `AGENTS.md:123`，“批准统计口径、来源归因与历史兼容策略” | DEC-004 已决定当前 save / HistoricalOnly / 缺字段显式失败，真实历史来源归因无需作为门禁 | `docs/decisions/DEC-004_HISTORICAL_MAT_FIELDS.md`；SRS §8 DEC-004 | 去除重复技术选型/历史归因审批阻塞。 |
| AF-031 | 中 | `AGENTS.md:124`，“批准 MAT 支持范围” | DEC-001 首版 MAT 范围已选，仍需批准的是正式接口、数据身份证据与基线，不是重复选范围 | SRS DEC-001/002；HLD ADR-001 | 以“已选设计/待实现验证”两层表述职责。 |

## 审计结论及不可推断项

既有 AF-001～027 的历史修订证据未因本轮发现自动否定；旧 `docs/approval/` 和 `docs/reviews/` 包含当时保留的已过时方案/状态，不应追溯改写。四项新问题将由 `AUDIT_PROCESS.md` 标记为「未整改」，`AUDIT.md` 中 PROGRESS 与 AGENTS 标为「正在审计」。本报告是**整改前**固定输入发现；修改、复读与结案需用独立的后续提交。没有改变生产源码、原 `origin/**`、MAT 文件、正式授权或验收状态。
