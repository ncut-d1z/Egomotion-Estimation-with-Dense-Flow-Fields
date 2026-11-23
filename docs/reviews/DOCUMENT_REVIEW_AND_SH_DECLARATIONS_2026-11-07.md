# 需求阶段文档边界、球谐接口声明与延期人类事项记录（2026-11-07）

> **此文件只是文档级 GitHub 连接器只读静态核对的证据，不是实际 C++ 头文件/算法实现、Bash/CMake/CTest、MAT 读取测试、MATLAB/Octave 运行或正式基线批准。** 本轮只修改 Markdown；任何已选技术方案均未因文档完成而自动成为测试 PASS。

## 当前审查范围与固定版本

- 目标分支：`dev-plan-P0-P1-mat-dictionary-probe`
- 本轮静态核对的**精确 Git commit**：`0d6060241a390a5c8c146154256f0a0f9173e277`。本文件与后续 PROGRESS/文档版本矩阵提交发生在此 SHA **之后**，不能将旧检查伪称覆盖后续 HEAD。
- 核对方式：GitHub 连接器读取固定提交下的文档和 Git tree，再由 JavaScript 执行 ID 集合、引用、依赖环及精确声明文本存在性核对；**没有**在 Debian/Ubuntu 运行 `scripts/check_docs_traceability.sh` 或任何其它脚本。

## 已完成的文档更正

| 审查问题 | 已落实文件/范围 | 不应扩大解读 |
|---|---|---|
| HLD §5.1 仍提 output-root | `ARCHITECTURE.md` 时序图现在由 CLI 参数与可选 RunId 进入 Composition Root，在固定仓库 `output/<RunId>/` 独占创建；已删掉用户定义输出根的旧文案 | 没有可执行 CLI/实际文件写入 |
| HLD harmonics 第三方适配层残留 | HLD §3 与 Object Model §7 统一“原论文及 1998 勘误＋**Eigen-only 自行重写**，Don Williamson 仅参考、sdla 完全禁止” | 不意味着已经复制或编译递推 C++ |
| SRS §6.1 CLI 宽泛类型 | SRS v0.7 改为 `UINT32` / `NONNEG_INT` / `FINITE_DOUBLE`；保持 TEST_PLAN/cli.md 同名与现有 66 REQ/TC | 参数具体解析和 ABI 仍是草案 |
| DEC-004 旧措辞 | `experiments.md v0.2`、`analysis_plotting.md v0.2` 统一 `HistoricalOnly`、来源未知无需先归因；不补造当前 save 缺失字段 | 没有实际 Runner/Analyzer/历史来源验证 |
| 球谐 C++ 设计不足 | `SH_ROTATION_IVANIC_RUEDENBERG.md v0.2 §3` 定义拟议公共/私有头及实现文件路径、`egomotion::harmonics` 和 `detail` 命名空间、类/值对象/静态函数**声明**；`harmonics.md v0.5` 引用该唯一声明而不复制第二套接口草案 | **未创建任何 .hpp/.cpp、未给函数体、未实现递推**；论文全文/勘误仍待用户上传 |
| 可推迟人类干预需集中 | 新 `docs/TODO.md` 逐项记录 PAPER-SH、H-10/H-11、DEC-005/006、正式基线/发布、可选日程，说明需要人类干预的原因与建议处理时间；PLAN/TEST_PLAN/LLD/PROGRESS 链接 TODO | 当前仅文档阶段，未取得任何 C++/Bash/CMake/CTest/MAT 写入/合并授权 |
| 状态与版本分散 | 计划在后续一次独立提交刷新 `DOCUMENT_BASELINE_REGISTER.md` 的每文件文档版本/文件 blob SHA；`PROGRESS` 仍是实际状态的唯一滚动来源 | 文件版本/静态核对均不等于项目所有者已签署正式基线 |

## 只读静态核对

在 `0d6060241a390a5c8c146154256f0a0f9173e277` 上：

| 检查项 | 观察结果 |
|---|---|
| `REQUIREMENTS.md` 规范性 REQ | **66**，唯一 AC 同尾号 |
| `TEST_PLAN.md` TC | **66**，逐项与 REQ/AC 对齐 |
| `PLAN.md` P | **98**，引用 REQ/TC/AC 配套、无未知 P 依赖或静态环 |
| 迁入 SRS 附录 A 的 REQ-DC | **8**，ID/TC/AC 保持 |
| HLD §4.1 的目标直接边 | **27**，静态无环（**非 CMake 编译证据**） |
| 原始目录 | Git tree 有 **45 个 .m 与 582 个 .mat**；本轮仅修改 docs 下的 Markdown，不修改原始归档 |
| 相应文档相对 Markdown 链接 | 本次检查范围内未发现不存在的目标 |
| HLD/对象模型/LLD/DEC-004/TODO 关键语义检查 | 与本轮文档修改目标一致，检查范围内 **0 项不一致** |

## 必须维持的状态边界

- **不运行 MATLAB/Octave、不要求 L1**；L2 作者 MAT、L3 独立参考、L4 统计各自保留真实证据要求。
- **不把本轮 Markdown 中的 C++ 声明当成已经创建的源码**；真实 public header、private `.cpp`、R5 实现及完整测试仍待后续。
- 已选定的 MIT、Eigen-only/禁 sdla、CLI 固定 `output/<RunId>/`、Result/Status、Adaptive 失败不提交、DEC-004 均无需反复请求技术决策。
- 未来必要的**人类材料、执行授权、正式 DEC-005/006 验收与合并发布**只在 `docs/TODO.md` 标定的时间办理。当前全部 SRS/HLD/LLD、TEST PLAN 仍为评审草案，**未签署正式冻结或通过 TC/AC**。
