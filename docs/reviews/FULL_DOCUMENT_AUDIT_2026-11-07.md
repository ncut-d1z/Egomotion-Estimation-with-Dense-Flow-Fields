# 全量文档审计执行记录：AU-001～AU-028 与 40 文件台账（2026-11-07）

> **审计结果是文档问题清单，不是修复提交，不是程序/数值验收。** 核对 [AUDIT.md](../AUDIT.md) 中的 28 组历轮用户意见和 40 条文件跟踪记录，在当前 GitHub 分支的固定快照下执行只读结构、追踪、术语、来源、接口、参考等级与历史证据审查。**本报告不修改 SRS/HLD/LLD 的技术合同，不运行 MATLAB/Octave、Bash、CMake、CTest、任何生产代码或 MAT 写入，不获得/授予 H-10/H-11 权限。**

- 仓库：`ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields`
- 分支：`dev-plan-P0-P1-mat-dictionary-probe`
- **被审计的固定 Git commit：`4627cc1e48be9c43ad10b28fda646d65ee8ef01c`。** 本报告、AUDIT/PROGRESS 更新与登记表的未来提交晚于该快照，不能自动算作对那些新内容的源程序审计。
- 来源：GitHub 连接器按 commit 拉取当时规范文档、历史 reviews/approval/decisions、`scripts/check_docs_traceability.sh` 的源码和递归 Git tree；以 JavaScript 进行只读检查，人工解释文档语义。**不是现场执行 `bash scripts/check_docs_traceability.sh`**。
- 审计状态采用 `AUDIT.md` 的三态：`空白`/ `正在审计`/ `已审计`，**“已审计”不表示问题已修订，也不表示被审计对象已编译、测试或获正式签署**。

## 一、已核实的结构事实（仅固定快照）

| 审计对象 | 固定快照只读结果 | 不应据此推断 |
|---|---|---|
| SRS↔TC↔WBS | **66 REQ、66 TC、98 P；REQ↔AC↔TC、P→REQ/TC/AC、任务依赖及引用集合的本轮 JavaScript 检查不一致数 0** | 66 TC 已创建/注册/运行，98 P 已验收 |
| SRS 内部设计约束 | **8 个 REQ-DC** 保留在附录 A，原 ID/AC/TC 仍在集合 | 已成为外部用户功能或正式批准编码合同 |
| MATLAB 静态映射 | **45 个 .m**，SRS §9.2 和 OBJECT_MODEL §7 各 **45 条**，相对路径与 Git tree 的集合吻合，未发现缺漏/重复 | 已核对每个函数所有数值分支、全部调用方或原 MATLAB L1 |
| HLD 拟议依赖 | §4.1 **27 条边**，静态图无环，含 `egomotion_harmonics→egomotion_contracts` | 真实 CMake 目标已经创建/编译、链接闭包通过 |
| 归档 MAT | Git tree **582 .mat**；`MAT_DATA_SCHEMA.md` 数据表 **582 行且相对路径唯一**，五形状计数 **10×1:80、10×3:480、35×1:4、600×1:14、600×4:4**；表中 582 行均为 `MAT v5/double/double/zlib/READ_OK`，记录的 data_bytes 与 dims×8 一致 | 本轮重新解压/逐元素读取了 582 MAT，取得独立 SHA-256、实现正式 Reader 或通过 TC |
| 文件/相对链接 | 已按 AUDIT 的 40 条路径分五批进行文件存在性、Markdown 相对链接路径及围栏数量检查：**共 452 条本地 Markdown 相对引用，缺失目标 0，未配平围栏 0** | 每一处 GitHub 标题锚点的 slug 都已逐个渲染核对，或 C++ 声明可编译 |
| 版本矩阵 | `DOCUMENT_BASELINE_REGISTER.md` 中当时登记的 **28 个文件 blob SHA 与当前 Git tree 一致**；PROGRESS 为“不适用（滚动证据登记）” | 文件已获基线批准，未来 HEAD 可继续复用这一快照 SHA |
| MIT/输出保护 | 根 `LICENSE` 为 MIT，`.gitignore` 有 `/output/`；所有者明确固定 `output/<RunId>/`、禁 sdla、无需 MATLAB/L1 | 真正执行过 Writer/TOCTOU/软链接负例，或已经构建球谐代码 |

**数学核验范围限制**：只审核设计文件是否明确来源、公式顺序、错误/资源边界和 L2/L3/L4 证据门槛；**没有**本轮对 KvD/Adaptive/采样/轨迹逐式独立求值或读取上传的 Ivanic–Ruedenberg 原始 PDF，故相应数值完整性不得标为验收。既有 AN-G01～12 仍为待独立实测的 L3 候选。

## 二、发现的问题与修订建议

所有“原文位置”均指向**以上固定 commit** 的行号。处理状态列全部为 **待修订/待核验**；本轮是执行审计、形成证据，不跨越授权擅自改算法/SRS/LLD。优先级是**文档一致性严重度**，不是批准或人工介入等级。

| ID | 优先级 | 原文位置／关键原句 | 问题、冲突来源 | 修订建议 | 是否已修订 |
|---|---|---|---|---|---|
| **AF-001** | **高** | `docs/design/experiments.md:137,172`：“校验用户输出根”“输出根由 CLI/Composition Root 验证” | 用户已**永久删除** CLI `--output-root`；`cli.md`、`mat_io.md` 和最新所有者决定只允许固定仓库 `output/`。该两处可能被读成仍可传入用户自定义根。 | 明确仅校验**内部固定的仓库输出根**及用户可选 `--run-id`；Composition Root 检查代码/运行路径来源，但绝不解析自定义输出根选项；RunContext/Writer 独占且二次验证。 | **待修订** |
| **AF-002** | **中** | `docs/design/harmonics.md:108`：“把**上游计算输出**转换到源 MATLAB 的次序” | 会误导为直接复用 Don Williamson C++ 库，而当前决定是**1996 论文＋1998 勘误为数学依据，Eigen-only 独立实现，Don Williamson 仅参考，绝不依赖 sdla**。 | 叙述改为“将**本项目纯 Eigen 递推生成的旋转块**映射到原实球谐约定的索引/相位”，不更改数学公式。 | **待修订** |
| **AF-003** | **中** | `docs/design/harmonics.md:136`：“**若需新 egomotion_contracts 依赖**，应同步修订 HLD” | HLD §4 已包含该明确依赖，§4.1 27 边 DAG 也已有 `H→K`；现在把“已决定”重新写成条件性将来选择。 | 改为“contracts 依赖**已写入 HLD**；未来变更目标边时才同步 ADR/DAG”；公共头是否能自包含仍待编译，不冒充已构建。 | **待修订** |
| **AF-004** | **中** | `docs/design/contracts.md:155`：“继续/回滚/保留部分结果是实验专属的**开放决策**” | 所有者已确定独立样本可记录继续、Adaptive 失败步**不提交**、不能安全恢复终止序列、保留可追踪部分结果、Manifest 三态。未确定的只是实现/异常测试。 | 改为引用 `experiments.md`、`adaptive_mfa.md` 与所有者决策的**已选策略**，分开“策略已选”与“实现未验证”。 | **待修订** |
| **AF-005** | **中** | `docs/TEST_PLAN.md:88`：“H-01 **只批准**先整理误差/容差提案” | 历史审批登记 `approval/DECISION_IMPLEMENTATION_NOTES.md:11` 明确 H-01 是 **draft、referenceConfirmed=false**；最新所有者**另行**确定 0.01/0.1 是首轮筛查，不等于原 H-01 签字批准。 | 将“历史 H-01 批准”改为“原 H-01 为 draft；后来所有者已单独选定首轮筛查”，不修改旧 JSON/approval 原记录。 | **待修订** |
| **AF-006** | **中** | `docs/golden/NO_MATLAB_EXECUTION_POLICY.md:12`：0.01/0.1 “**未批准的宽松候选**” | 现行所有者决定已选**第一轮探索性筛查**，只有**最终 DEC-005 模块 PASS** 未批准；易混淆设计选择和正式验收阈值。 | 改为“首轮筛查已确定，最终模块数值阈值未冻结”，与 `GOLDEN_TOLERANCE_PROPOSAL.md` 一致。 | **待修订** |
| **AF-007** | **中** | `docs/golden/NO_MATLAB_EXECUTION_POLICY.md:14`：“**第三方版权**……门禁并未取消” | 普通版权复核不再构成原创研发阻塞；真实引入上游文件的已有 notice/来源义务仍保留。当前表述把两者混为一个阻塞项。 | 改为“实际引用第三方文件时的必要来源/许可告知保留；**一般版权审批不阻塞开发**”；不擅自重新许可他人的原资料。 | **待修订** |
| **AF-008** | **中** | `docs/design/mat_io.md:104`：“采用 Failure{code,detail} **还是其他实现方式**由公共合同统一冻结” | 所有者已选择**轻量封装 Result/Status＋Failure<Code>＋ErrorDetail（内部 variant）**；具体编译/ABI 尚未证实，不应重新开放已选类型方向。 | 仅保留“具体 C++ 头/ABI 尚待实际验证”，明确复用 `contracts.md §3` 的现行 Failure/Result/Status 设计。 | **待修订** |
| **AF-009** | **中** | `docs/OBJECT_MODEL.md:226`：`RealSphericalHarmonics::reconstructNearness` | 球谐专题 LLD §3 的唯一预定公开方法名是 `RealSphericalHarmonics::reconstruct`，已出现**同一 MATLAB 函数的两种 C++ 方法名**。 | 将对象模型中的 `near_aus_koeff.m` 映射为唯一声明 `RealSphericalHarmonics::reconstruct`，保留原函数来源/3×5 语义。 | **待修订** |
| **AF-010** | **低** | `docs/REQUIREMENTS.md:160`：“MAT 结果写入：**与 origin 不重叠的输出路径**” | “不重叠”只是必要安全条件，不够表达**只能是仓库根 `output/<RunId>/`** 的现行固定输出合同。后文 §6.1 已正确明确该路径。 | 表中也写明“固定 `output/<RunId>/`，非任意与 origin 不重叠的用户目录”；避免上下文依赖。 | **待修订** |
| **AF-011** | **低** | `docs/ARCHITECTURE.md:103`：“**Sophus 是可选外部实现依赖**” | H-02/现行技术决定已经选择以 Sophus 的 SO(3) 前向旋转 API 替代 Rodrigues 实际调用分支；“可选”易误认为可以另选数学实现。 | 写成“**对有旋转转换调用的实现是已选外部依赖**，仅相对其它不调用此能力的 targets 为条件性链接”，不改实际 HLD DAG。 | **待修订** |
| **AF-012** | **低** | `docs/design/harmonics.md:100`：标题“上游 C++ **复用路线与法律/构建边界**” | 标题停留在旧直接移植第三方代码路线，而正文已选择独立 Eigen 设计。 | 仅改标题为“原论文、参考实现与 Eigen-only 实现边界”，不删来源表及必要第三方 notice。 | **待修订** |

### 历史文档的解释风险（不得改写原记录）

| ID | 类型 | 历史位置 | 处理要求 |
|---|---|---|---|
| **AH-001** | **历史语境标记，不是当前设计缺陷** | `docs/approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md:32` 曾要求“原 MATLAB Goldens”；早期 reviews 保留 26 边 DAG、SRS v0.3/v0.4/旧 SHRotation/旧输出根 | 当作**当时的审查记录**，不能从该原句推导现行仍须采 L1 或引入 sdla。项目当前政策以 `decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md`、`golden/NO_MATLAB_EXECUTION_POLICY.md`（待修 AF-006/007）为准；**不要批量覆盖或删掉历史 SHA/旧意见**。可在统一审计索引注明现行优先级。 |
| **AH-002** | **真实运行证据尚缺** | `docs/TEST_PLAN.md §8.1`、`scripts/check_docs_traceability.sh`、所有模块 LLD | 脚本源码静态阅读与等价 JS 集合检查完成；**Bash/CTest 未执行**。未来执行范围统一由 `TODO.md` 的 H-10/H-11 决定，不能把本报告静态“0 不一致”写成 TC PASS。 |

## 三、各模块审计边界与下一轮检查建议

| 模块或资料 | 本次审阅的主要内容 | 下一轮应补的**文档/数学独立核查** |
|---|---|---|
| Franz/KvD `kvd.md` | 双入口、trace 参数及停止规则的 LLD 结构/公式声明、L3 逐轮证据字段 | 依静态源文件逐式核对 Original/Modified a/b/c、旧/新 t 的 ω 顺序、近度尺度/阈值、6～200 次；本轮**未重新独立计算任何数值** |
| Adaptive `adaptive_mfa.md` | 单步/Session 分层、M1–M4、返回 error=MSE、无效状态不提交原则 | 逐式核对 6×6/MSE/新系数及三入口时序，独立 L3 异常/固定状态输入；不能因为合同文本完整就宣称数值等价 |
| 采样/噪声 `sampling_noise.md` | N=base·4^level、严格递归顺序、2 uniform + seed reset + N uniform + 3N normal 等文档约束 | 实际原源码的 i=1/2 全量有序方向与固定注入 RNG 逐值证据，暂不执行 |
| 轨迹 `trajectory.md` | Sophus 所需的 Rodrigues 前向分支、`R^T`、近零/反向与球内近度边界 | 源文件函数调用分支、反向向量零角、小角 eps、端点 zeros 与独立解析参考 |
| 球谐 `harmonics.md`、专题 LLD | 唯一声明级文件/命名空间/类接口存在、Eigen-only/禁 sdla/原文勘误优先、R3/R5 样本规划 | 等用户上传原文＋勘误后**逐式数学审计**；本轮只检查声明文本，不写生产递推代码 |
| MAT I/O `mat_io.md` | 二维 dense double、owning MatDenseArray/RAII、单变量兼容、多变量 AmbiguousVariable、固定结果根、独立负例计划 | 修 AF-008 并结合正式头/Reader/Writer/TOCTOU 的未来测试验证；未写 MAT |
| 分析/Runner `analysis_plotting.md`、`experiments.md` | 四类分析、历史/当前 MAT 差异 DEC-004、8 次聚合、失败 Manifest、只读历史数据 | 修 AF-001；对历史 begin/end/zone、平滑边界、全量 L3 曲线及新的缺失字段负例独立核验 |
| MAT 数据字典 | 582 记录、单变量 double/dims/READ_OK 的**已保存报告内部结构** | 独立 SHA-256/逐元素 L2 读取与真实生产 Reader 证据仍缺（FNV64 不可冒充 SHA-256） |
| 历史审批/reviews、AGENTS/README/LICENSE/.gitignore | 历史时间范围、来源/许可、固定输出、L1 非强制、可延期许可、Git 安全与 `/output/` 忽略项 | 原始审批 JSON/历史 reviews 不改；需求中若发现新强制技术选型，另评审最新规范而非追溯覆盖历史 |

## 四、给 AUDIT 进度表的更新依据

- 可在**本次审计范围内**标为“已审计”：原已审计的 AUDIT/PLAN/LLD 索引/登记表/PROGRESS/TODO；CLI 的无输出根约束、DEC-004 决策/所有者决定、L2/L3/L4 分类、MAT 数据字典报告**内部结构**、MIT/LICENSE/README/AGENTS/.gitignore、旧 approval/reviews 的**历史范围**、静态 Bash 源码**文本**（未执行）。
- 应保留“**正在审计**”：具有 AF-001～AF-012 未修订事项的 SRS/HLD/OBJECT_MODEL/contracts/experiments/harmonics/mat_io/TEST_PLAN/NO_MATLAB_POLICY，以及已做部分算法合同核对但仍欠原源码逐式证据的 Adaptive、分析/绘图、独立数值参考候选文档。
- 应保留“**空白**”：未在本次独立逐式审阅源函数/生成完整固定数值证据的 KvD、采样噪声和轨迹模块的**深层数学审计**。虽然本轮已有跨文档扫描，不能因此视为原算法数学合同已完整审计。
- **审计状态与修订状态是两回事**：未修订的问题在此报告按 `AF-...` 保留，即使相应历史记录已审计也不得删除。用户若指示下一轮修订，可逐条实施并固定 SHA 复读验收；**本轮不擅自修改任何原功能/数学合同**。

## 五、复核及关闭条件

1. 改动规范前，检查是否与已确认的 Eigen-only、固定输出、DEC-004、失败状态/Result/JSONL、首轮 0.01/0.1 筛查/最终 DEC-005 以及不要求 L1 冲突。技术路线已选，**普通细节不重复索求人类批准**。
2. 对每个 AF 项按**路径+行号+原句+修改方案+修订 commit SHA+复查结论**关闭；每轮只处理已明确的修订范围。若没有执行数值/CMake/CTest，不填 `PASS`、stdout、exit code 或未知参考数据的 SHA。
3. 各文档更新后同步 `DOCUMENT_BASELINE_REGISTER.md` 的对应精确 Git blob SHA 与独立文档版本；注册表的固定快照只能说明身份，不能代表批准签名。将新的审计进度与失败证据身份登记到 `PROGRESS.md`。
4. **此时没有需要用户马上重新决定的普通技术方案。** 可推迟的论文上传、H-10/H-11 执行、DEC-005/006 以及最终冻结/发布仍只从 `TODO.md` 选择适当阶段办理。

**结论**：本次完成了 40 文件台账的第一轮**跨文档与结构审计**，证实源文件/数据字典/编号和 HLD DAG 的已保存结构一致，同时检出 **12 个待修订的当前文档问题**及 **2 项历史/执行边界说明**。这不是 40 个文件的逐式数学审计、不是正式基线冻结；未进行任何生产代码/测试/原 MAT 写入。
