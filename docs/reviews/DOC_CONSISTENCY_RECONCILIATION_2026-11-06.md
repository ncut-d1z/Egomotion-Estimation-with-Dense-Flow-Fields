# 审查意见后的跨文档一致性修订与只读静态复核（2026-11-06）

> **性质：文档更正与 GitHub 连接器上的 JavaScript 只读核对，不是 Bash 现场执行，不是完整 CMake/CTest，也不是 Golden、数值容差、SRS/HLD/LLD 冻结或软件验收。** 对原始 `origin/**` 保持只读，未复制第三方 C++ 源码或运行 MATLAB。

## 1. 本轮范围与基线

- 仓库/分支：`ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields` / `dev-plan-P0-P1-mat-dictionary-probe`。
- 文档更正前分支 HEAD：`522ec8a1eed5c1b48c9edae78664017d6722012a`。
- 本轮更正后的**只读复核提交**：`5472c756b983fe002ef6e32143c96b6c6798cde9`。后续添加本审查记录与进度链接的提交不属于前述固定读入 SHA。
- 检查方式：GitHub 连接器按上述固定提交读取 Git 树和文档，使用 JavaScript 检验计数、映射、依赖环和跨文档修订结果；这不是在完整 Debian/Ubuntu 工作树中实际运行 `scripts/check_docs_traceability.sh`。
- 审批边界：H-02/H-04/H-12/H-14 的限定设计决定保留；H-10/H-11 暂缓且 `technicalDetails.permissions` 均为 `false`，故没有进行生产实现、现场 Bash、CMake/CTest、MATLAB、MAT 写入、性能测量、合并或发布。

## 2. 已落实的审查问题（仅文档层）

| 文件 | 本次一致性更正 | 未改变的门禁 |
|---|---|---|
| `REQUIREMENTS.md` | §3 补 LLD 引用在冻结时需绑定章节/版本/blob SHA/commit SHA/审批记录；§6.1 明确摘要型 `--output-root DIR` 不授权任意路径，且 `INTEGER/NUMBER` 不是冻结类型 | 不改动 §4 中任何 REQ 需求行与 AC 原文 |
| `ARCHITECTURE.md` | §2.1 将 H-04 的系统、内存、磁盘、历史工具链和待批准性能阈值分栏；部署示意明确 matio 版本是历史探针事实 | 不声称完整 CMake target 已存在/编译 |
| `design/cli.md` | §1/§2.1 将 `--output-root DIR` 候选限制为仓库根 `output/`，由 RunContext 新建 `output/<RunId>/`；没有新增默认值 | H-03/DEC-008 的最终参数、错误 schema、并发策略仍待审批 |
| `design/mat_io.md` | §3 区分输出根 `output/`、运行目录 `output/<RunId>/` 和 Writer 的不可越权边界 | 无真实 Reader/Writer、MAT 写回读/负例 |
| `design/experiments.md` | §2 统一 `Result<RunOutcome, ExperimentErrorCode>`；§5 拆开当前脚本的预分配/save 和历史 MAT 的 matio 实测形状/字段 | 历史 `alleadap*`/`rottransratio` 不推断来源、不伪造 |
| `design/harmonics.md` | §7.2 以 Don Williamson Ivanic–Ruedenberg C++ 为待实施 R5 实现来源；原 `SHRotate.m` 保持独立行为 Golden 来源 | 需许可、sdla 依赖闭包、R5 全 25 元素 Golden/DEC-005 |
| `TEST_PLAN.md` | §2.1 参数类型改为与 CLI 候选同名的 `UINT32`/`NONNEG_INT`/`FINITE_DOUBLE`；写出根语义对齐 | 未执行 CLI、CTest、MAT 写回读或 Bash |
| `PLAN.md` | §3.3 精确区分设计原则 §3 与 ADR §8；§4 与 CLI 输出根候选规则对齐 | 原 98 项 P 工作内容、优先级与验收状态未改变 |

`OBJECT_MODEL.md` 已将对象模型评审、HLD 评审与 LLD 代码签名冻结分为独立门禁，无需强制把 HLD 附件变成 LLD。`contracts.md` 原有错误码唯一候选表仍是 `cli.md` 的参照，本轮没有擅自冻结其值。

## 3. 已执行的 GitHub 连接器只读检查（固定提交 5472c756…）

| 静态项目 | 观察值与结果 |
|---|---|
| REQUIREMENTS 规范性 REQ 行 | **66 个，唯一、每行唯一匹配 AC** |
| TEST_PLAN TC 行及 REQ/AC 对应 | **66 个，严格配套** |
| PLAN P 工作项 | **98 个，引用的 REQ/TC/AC 配套，无未知依赖、无环** |
| 原始 MATLAB 源树 | **45 个 .m** |
| SRS 与 OBJECT_MODEL 中逐文件 .m 映射 | **各 45 项**，均与原始 .m 路径集合相同 |
| HLD §4.1 组件 target 依赖边 | **26 条**，静态无环 |
| 原始 MAT 文件数量 | **582 个**（未重新解码原始文件） |
| 原 `origin` Git 子树 SHA | 更正前后均为 `70ee155131c24a50169346d0ffbaff0c01b1c547`，未改变 |
| REQ/TC/P 规范性表格行 | 与更正前固定提交逐行文本**完全一致** |
| `.gitignore` | 仍存在根 `/output/` 忽略规则 |
| 补充语义检查 | `experiments.md` 不再出现 `RunReport(`；TEST_PLAN §2.1 不再出现旧 `--seed INT`/`--level INT`/`--noise-scale NUMBER`；R5 实现/行为依据已区分 |
| **上述检查中的不一致数量** | **0** |

**证据限制**：上述检查是 GitHub 读取 + JavaScript 断言的**特定范围结果**，不得被描述为脚本的现场 exit=0、Bash 的 stdout/stderr 日志、MAT 文件 582/582 重新读取、SHA-256 全量独立复核、算法等价、真实 target DAG 构建或任何 TC/AC PASS。完整检出环境真实执行 `bash scripts/check_docs_traceability.sh` 仍受 H-11 的单独授权门禁。

## 4. 尚未具备的冻结/编码/验收证据

- **SRS**：DEC-005 的逐模块 atol/rtol、合法的独立 MATLAB Golden，以及 CLI/错误合同、精确 LLD 引用 SHA 与批准记录未齐。
- **HLD / 对象模型**：依赖图仅静态核对，真实 CMake targets、接口替身、状态和资源生命周期未进行代码及测试验证。
- **LLD**：Result/Status、RunContext、RNG、Session 异常提交以及球谐 R5 的完整合法实现仍需进一步人类签署和源 Golden。
- **测试与运行**：当前没有新增业务 CLI、MAT Reader/Writer、CTest、Bash 现场日志、真实 MATLAB Golden、SHA-256 582 文件独立复核或性能测量。
- **发布**：PR 合并、正式发布、项目根 LICENSE、需求/设计冻结均未获许可；`origin/**` 不得修改。

**结论：** 当前已纠正所列的文档间候选合同表述问题，且本次针对既有 REQ/TC/P、源文件映射、目录保护与 HLD 静态 DAG 的只读检查未发现新增不一致。此结论**不提升** M0–M5、P0–P10、DEC、SRS/HLD/LLD、TC/AC 的验收或批准状态。
