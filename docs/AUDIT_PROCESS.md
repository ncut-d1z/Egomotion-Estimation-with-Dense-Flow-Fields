# 审计整改进度台账（AUDIT_PROCESS）

> **用途**：专门跟踪 [AUDIT.md](AUDIT.md) 发现的**具体整改目标及其实际整改状态**。本台账不能替代 `AUDIT.md` 的文件审计三态、`PROGRESS.md` 的真实测试/验收登记，不能代表需求、接口、数学公式或软件基线已获批准。**本文件的整改状态只允许「未整改」「已整改」两种值**；不使用「正在审计」充当整改状态。
>
> **来源与固定审计快照**：先前文档审计 [FULL_DOCUMENT_AUDIT_2026-11-07.md](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md) 的审计输入 `4627cc1e48be9c43ad10b28fda646d65ee8ef01c`；上一轮逐条整改/静态复核 [AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md](reviews/AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md)，其限定复核输入 `893acb7b07862c0701e62b786462ef8395a116da`、最终版本登记提交 `3beb8c06a9d40380431f99e31ed0f70b346863a2`。**本次下一轮扫描基准也是 `3beb8c06a9d40380431f99e31ed0f70b346863a2`**。所有“已整改”仅表示具体文档冲突被修订并经过限定静态检查，不代表真实 C++/CTest/数值验收。

## 一、执行与更新规则

1. 发现新问题时**必须在本表新增行**，使用下一个 `AF-xxx` 编号或同一 AF 的原子子编号；同时记录**仓库根相对路径、具体整改目标、整改状态「未整改」**、发现所对应的固定 commit/章节/原句、目标依据。一个文件有多个独立问题时分多行，不能用一行“统一修订”掩盖遗漏。
2. **整改动作与审计动作分开**：仅扫描/分析/给建议**不改变「未整改」**；实际修改目标文件、核对新旧语义和相关 REQ/TC/P/章节链接、保留修订提交与复核证据后，才将该目标改为**「已整改」**。若复核失败或后续版本回归，则重新标「未整改」并记录新的快照。
3. [AUDIT.md](AUDIT.md) 的**逐文件审计状态**只可为「空白、正在审计、已审计」；本表的**整改状态**只可为「未整改、已整改」。**某文件“已审计”不代表其中每条整改目标“已整改”；某条目标“已整改”不代表整个文件数学逐式审计完成或可冻结。**
4. 保留旧 [FULL_DOCUMENT_AUDIT](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md) 的**发现当时未整改**和 [AF-001～012 修订记录](reviews/AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md) 的**后续已修订**历史事实，**不改写旧 SHA、审批 JSON 或历史报告**。新问题不得沿用或覆盖已存在的 AF 编号。
5. 执行仍是**文档审计阶段**：原 `origin/**` 只读；没有 H-10/H-11 就不编写生产代码/运行 Bash、CMake、CTest、MAT 写回读、数值实验或 MATLAB/Octave；只有独立实测与正式批准后才能声明数学或软件验收完成。

## 二、上一轮明确提出并已修订的全部整改目标

下表按**具体目标**逐行登记，而非只列文件名。其中 AF-001 的“取消用户根校验”与“确认固定根、RunId、Writer 二次验证”分开列出，以免把同一文件的两个整改义务合并遗漏。整改证据统一是 [12 项修订复核记录](reviews/AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md)（静态快照 `893acb7b07862c0701e62b786462ef8395a116da`）；不代表 C++ 运行成功。

| 整改 ID | 文档文件相对路径（仓库根） | 具体整改目标 | 整改状态 | 修订证据 |
|---|---|---|---|---|
| AF-001a | `docs/design/experiments.md` | §6.1 删除“Composition Root 校验**用户输出根**”的旧语义，明确 **CLI 不提供/解析自定义输出根参数** | 已整改 | AF-001 文档复核 |
| AF-001b | `docs/design/experiments.md` | 固定仓库根 `output/`，验证可选 `--run-id`/OS 独立默认 ID，独占创建 `output/<RunId>/`；Writer 每次二次检查、不覆盖、不让 Runner 拼任意用户路径 | 已整改 | AF-001 文档复核 |
| AF-002 | `docs/design/harmonics.md` | §3 旋转块来自 **1996 原文及 1998 勘误指导的纯 Eigen 独立递推**，不是将“上游 C++ 计算输出”直接转换；维护原实基 m 顺序/相位、Don Williamson 仅参考、禁 sdla | 已整改 | AF-002 文档复核 |
| AF-003 | `docs/design/harmonics.md` | §4.1 `egomotion_harmonics→egomotion_contracts` 已写入 HLD §4 和 27 边 DAG，不得再描述成“若需新增的依赖”；只在未来改变依赖时同步 HLD/ADR | 已整改 | AF-003 文档复核 |
| AF-004 | `docs/design/contracts.md` | 将实验失败的继续/不提交/停止连续序列/保留部分结果及 Manifest Complete/Incomplete/Failed 从“开放决策”改为**已选策略**，并引用 Runner/Adaptive 的单一合同；保留尚未实施/测试的边界 | 已整改 | AF-004 文档复核 |
| AF-005 | `docs/TEST_PLAN.md` | 正确登记历史 H-01=`draft,referenceConfirmed=false`，不声称审批 JSON 已批准容差；区别后来所有者已选 0.01/0.1 首轮筛查与未批准的最终 DEC-005 | 已整改 | AF-005 文档复核 |
| AF-006 | `docs/golden/NO_MATLAB_EXECUTION_POLICY.md` | 将 0.01/0.1 标为**已选首轮宽松筛查**，明确最终模块正式 PASS 容差**尚未批准**，不再称“未选的候选” | 已整改 | AF-006 文档复核 |
| AF-007 | `docs/golden/NO_MATLAB_EXECUTION_POLICY.md` | 删除“普通第三方版权复核仍是开发阻塞”的误导；实际引入第三方材料只保留必要来源/既有 notice，项目 MIT 不自动重新许可他人作品 | 已整改 | AF-007 文档复核 |
| AF-008 | `docs/design/mat_io.md` | §2.2 复用已选 `Failure<MatErrorCode>{code,detail}` 与封装 `Result/Status`（内部 variant），不再重开“还是采用其它失败类型”的方向；实现和 ABI 仍待验证 | 已整改 | AF-008 文档复核 |
| AF-009 | `docs/OBJECT_MODEL.md` | `near_aus_koeff.m` 的 C++ 方法名 `RealSphericalHarmonics::reconstructNearness` 统一为专题 LLD 唯一声明 `RealSphericalHarmonics::reconstruct` | 已整改 | AF-009 文档复核 |
| AF-010 | `docs/REQUIREMENTS.md` | §6 的 MAT 输出合同必须明确**只写 `output/<RunId>/`**，不能将任何不与 `origin/**` 重叠的任意路径当作可配置根 | 已整改 | AF-010 文档复核 |
| AF-011 | `docs/ARCHITECTURE.md` | Sophus `SO3d::exp` 对需旋转向量→矩阵调用的模块是已选依赖；只对无此能力的 target 不要求链接，不再笼统写“可选” | 已整改 | AF-011 文档复核 |
| AF-012 | `docs/design/harmonics.md` | §3.1.1 标题改为**原论文、Don Williamson 参考与 Eigen-only 构建边界**，不再称“上游 C++ 复用路线”，保留正常数学来源身份 | 已整改 | AF-012 文档复核 |

**上一轮范围小结**：12 个原始 AF 编号，拆分为 **13 个可分别复核的整改目标**，状态均为 **已整改**。原 [FULL_DOCUMENT_AUDIT](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md) 仍会显示历史快照中的“待修订”，请勿用它覆盖本表的当前状态。

## 三、按照 AUDIT.md 开展的下一轮问题扫描：新增待整改目标

**下一轮扫描固定输入 commit**：`3beb8c06a9d40380431f99e31ed0f70b346863a2`。范围：当前 SRS/HLD/对象模型、公共与模块 LLD、TEST_PLAN、PLAN、决策/Golden 附件及原 `origin/**` 的**只读静态源码对照**（抽取了 KvD 两版本、Franz、八面体采样、轨迹、Adaptive 单步及分析相关源函数）。**没有**产生 MATLAB L1、独立 L3 数值、调用 CMake/CTest 或检查真实 C++。以下每一项都是**新发现或新确认的现行文档待整改目标**，不是上一轮已修订 AF 的重新打开。

| 整改 ID | 文档文件相对路径（仓库根） | 具体整改目标（待修正的冲突） | 整改状态 | 固定快照线索/依据 |
|---|---|---|---|---|
| AF-013 | `docs/design/kvd.md` | §5 不得把历史 `1e−10` 再称为现行“候选”；统一为已由所有者选择的 **`atol=0.01,rtol=0.1` 仅用于首轮宽松筛查**，最终 DEC-005 按模块和严格几何/离散条件另审 | 已整改 | [本轮修订复核](reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md)，定向静态提交 `a782b5c8d86e50fe4d2ae4e2b198b3cd6345de52`；初始扫描位置见原问题描述 |
| AF-014 | `docs/design/kvd.md` | §10 对 H-13 原审批 `draft` 的描述应标为**历史状态**，并引用后续已选**独立样本可继续、Adaptive 失败步不提交、无法安全恢复停止连续序列、部分结果保留**；不能把新决定重新称作“用户不批准继续/回滚/终止”，也不将其等同于原 MATLAB 异常实测 | 已整改 | [本轮修订复核](reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md)，定向静态提交 `a782b5c8d86e50fe4d2ae4e2b198b3cd6345de52`；初始扫描位置见原问题描述 |
| AF-015 | `docs/design/sampling_noise.md` | §3.1 保留随机源耗尽等单次数值失败的显式传播，同时将**实验整体继续/状态不提交/部分结果保存策略**从“待 P0.8 和 Golden 审议”改为**已由所有者选择、真实 C++/CTest 尚未验收** | 已整改 | [本轮修订复核](reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md)，定向静态提交 `a782b5c8d86e50fe4d2ae4e2b198b3cd6345de52`；初始扫描位置见原问题描述 |
| AF-016 | `docs/OBJECT_MODEL.md` | §4 区分已选**Adaptive 失败不提交无效主系数**与仍须独立 L3/CTest 判断的安全恢复场景；不能笼统将“是否提交/是否终止”全部标成待决定；状态机及注释应呼应 Runner/Session | 已整改 | [本轮修订复核](reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md)，定向静态提交 `a782b5c8d86e50fe4d2ae4e2b198b3cd6345de52`；初始扫描位置见原问题描述 |
| AF-017 | `docs/ARCHITECTURE.md` | §7 **QA-01** 不能假设“用户把输出根目录设为 origin 子目录/软链接”，因为 CLI 已删除自定义输出根。改为用户提供非法/重复 `--run-id`、固定根被软链接/路径竞态替换、或输出目标已存在时的拒绝和原 `origin/**` 不变 | 已整改 | [本轮修订复核](reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md)，定向静态提交 `a782b5c8d86e50fe4d2ae4e2b198b3cd6345de52`；初始扫描位置见原问题描述 |
| AF-018 | `docs/design/mat_io.md` | §4.2 将外层分析比较的 0.01/0.1 从“**未批准候选**”改成**已选首轮筛查但不是正式 PASS 阈值**；MAT 解码/复制的 exact dtype/shape/原始 double 校验仍严格，不因宽松筛查放行错误 | 已整改 | [本轮修订复核](reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md)，定向静态提交 `a782b5c8d86e50fe4d2ae4e2b198b3cd6345de52`；初始扫描位置见原问题描述 |
| AF-019 | `docs/ARCHITECTURE.md` | §8 **ADR-002** 中“**拟采用 Sophus SO(3)**”与 §3 已选 `SO3d::exp` 口径不统一；改成“已选定设计路线，尚待真实依赖锁定/编译/数值验证”，不推断已部署 | 已整改 | [本轮修订复核](reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md)，定向静态提交 `a782b5c8d86e50fe4d2ae4e2b198b3cd6345de52`；初始扫描位置见原问题描述 |
| AF-020 | `docs/design/trajectory.md` | §5.1 已有“`omega.norm() < std::numeric_limits<double>::epsilon()` 返回 I 后调用 Sophus”来源规则，§6.1 又将“**是否把近零角度强制置零**”笼统说成待审批；明确**原始 epsilon 条件已确定**，额外扩大零角阈值或对其它异常输入的处理仍须独立测试/审查，不能混淆为同一未决事项 | 已整改 | [本轮修订复核](reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md)，定向静态提交 `a782b5c8d86e50fe4d2ae4e2b198b3cd6345de52`；初始扫描位置见原问题描述 |

**该次扫描发现时的历史状态：8 个新增整改 ID 当时均「未整改」，分布于 `docs/design/kvd.md`、`sampling_noise.md`、`OBJECT_MODEL.md`、`ARCHITECTURE.md`、`mat_io.md`、`trajectory.md` 六份现行设计文档。** 其中纯文档权限可后续逐项整改，而公式、退化近零条件仍须尊重原只读 .m 的确切分支，不得在本轮引入生产 C++ 或猜测原 MATLAB 运行结果。

## 四、下一轮扫描的正向核对与覆盖限制

- **已静态交叉检查的来源**：原 `norm_koenderink_iteration.m` 与 `mue_norm_koenderink_iteration.m` 的区别确为原始平移更新的 `near²` 与修正版本的 `near`、旧 `t` 算 `rot`；`franzeigen.m` 三轴匹配；`erzeugerichtungsvektoren.m` 的八面体 8 面顺序及缺损版本保留面 2/3/5/6/7/8；`findeDreiecke.m` 的 A/B/C/D 细分；圆/正弦的 `stellen` 分母；轨迹 `Rᵀ`；`oneit_adaptiv_matched_filter.m` 的 6×6 和返回 `error=MSE`；`kurveglaetten.m` 的半窗/边界零值。
- **仍未完整核对**：各模式的全部 MATLAB 源分支、所有调用者、i=1/2 的完整有序 L3 数组、近退化输入、球谐 1996 原文/1998 勘误全文（用户尚未上传）、分段分析全部实际 L2 数值。**源代码静态查看不等于已生成或运行独立 L3，也不等于数值完全等价。**
- **原扫描轮次的边界（历史事实）**：当时只创建/更新审计登记、没有修订 AF-013～020 的目标设计文件；之后项目按台账逐项改动六份现行设计文档，并在 [后续固定 SHA 修订报告](reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md) 中核对，**本表上方现已据此标记「已整改」**。
- [AUDIT.md](AUDIT.md) 继续使用文件审计三态；[PROGRESS.md](PROGRESS.md) 只记录真实执行/文档检查证据与基线状态；历史报告的 `AF-001～012` 原始“待修订”是**当时状态**，本表保留完整时间顺序。

**历史扫描时 AF-013～020 曾为「未整改」；随后依据修订证据均转为「已整改」**。目前 21 个具体整改目标已全部完成**文档语义修订及定向静态复核**，并非所有模块已实现或数值验收。延期论文、H-10/H-11、DEC-005/006、基线和发布仍见 [TODO.md](TODO.md)。


## 五、AF-013～AF-020 的后续整改结案（2026-11-07）

- **原发现快照**：`3beb8c06a9d40380431f99e31ed0f70b346863a2`，当时本台账 AF-013～020 的八行状态确为「未整改」。
- **修订后固定复核快照**：`a782b5c8d86e50fe4d2ae4e2b198b3cd6345de52`；[逐条整改、文件版本、静态证据及限制](reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md)。八项均改动其对应的 **六份当前设计文档**，并同步修正 `docs/design/README.md` 的 MAT I/O 版本引用。**8/8 条定向文本断言通过**，原 REQ/TC/P 与 HLD DAG 的设计集合保持一致。
- **本台账现行状态**：AF-001a/001b 与 AF-002～012 共 **13 个「已整改」**；AF-013～020 共 **8 个「已整改」**；合计 **21 条整改目标全部在文档一致性意义上已整改**。本状态只代表问题条目对应的**文字和接口口径**已修订，不表示不存在未发现的新问题。
- **仍未完成**：KvD、采样噪声、轨迹以及 Adaptive/球谐/分析中必要的原公式逐式核验、独立 L3/适用 L4 与真实 C++/CTest/TOCTOU 数值/安全测试、最终 DEC-005/006、正式 SRS/HLD/LLD 冻结。对应**文件审计三态**继续见 [AUDIT.md](AUDIT.md)。
- **实施权限边界**：没有实施/编译 C++、没有运行 Bash/CMake/CTest/MATLAB/Octave、没有写入 MAT，也没有改变 H-10/H-11/合并/发布授权。本次仅文档修订与 GitHub 连接器静态核对。

## 六、先审计：AF-021～AF-025 的新发现与初始整改状态

> **已先完成独立审计，再进入整改。** 新问题的**固定审计输入 commit**：`af9198906cf89b6fb5162aea1d74683906740d78`；证据：[AUDIT_FOLLOWUP_SCAN_2026-11-07.md](reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md)。下列整改行在未修改目标文件之前登记为**未整改**，与 AF-001～020 历史已整改事项分开。本次扫描只通过 GitHub 连接器读取当前资料及部分只读源 `.m`，不是实际 C++/CTest/实验验证。

| 整改 ID | 文档文件相对路径（仓库根） | 具体整改目标 | 整改状态 | 审计依据 |
|---|---|---|---|---|
| AF-021 | `docs/ARCHITECTURE.md` | HLD §1 不再把**一般版权审批**写成研发门禁；保持实际上游文件已有来源/许可 notice 及真实 CMake/CTest、接口/指标和基线证据要求，项目原创 MIT 已确定 | 已整改 | [修订后静态复核](reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md)，固定提交 `ac683f41af2f906137705f3ad31e8da53fe5c036`；初始审计见 [扫描报告](reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md) |
| AF-022 | `docs/REQUIREMENTS.md` | SRS DEC-001 把**首版仅支持二维、密集、实数、非 logical double 的 MAT v5/zlib**写为已由所有者选定的范围；将尚未完成的正式 Reader/负例和基线签署与已经确定的范围分开，不重复请求支持范围审批 | 已整改 | [修订后静态复核](reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md)，固定提交 `ac683f41af2f906137705f3ad31e8da53fe5c036`；初始审计见 [扫描报告](reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md) |
| AF-023 | `docs/ARCHITECTURE.md` | HLD ADR-001 与 SRS DEC-001 / mat_io LLD 一致，写明初版 MAT 类型界限**已选**、实际 Reader/Writer/负例与正式基线仍未验收；不再笼统说范围待批准 | 已整改 | [修订后静态复核](reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md)，固定提交 `ac683f41af2f906137705f3ad31e8da53fe5c036`；初始审计见 [扫描报告](reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md) |
| AF-024 | `docs/design/analysis_plotting.md` | §5.3 的 Bias 历史 xwerte 与实际视线 N 明确分开；原始图表为当前复现要求，**额外实际 N 对照图不属于当前必须交付项**，不再要求所有者现在决定是否增加可选新图；需要时另提正式需求、不阻塞核心分析 | 已整改 | [修订后静态复核](reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md)，固定提交 `ac683f41af2f906137705f3ad31e8da53fe5c036`；初始审计见 [扫描报告](reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md) |
| AF-025 | `docs/design/sampling_noise.md` | 标记 `fixed_kugel_...randomnear.m`、`fixed_propfehler_...randomnear.m` 的**原 MATLAB 返回 near 是逐点循环最后赋值的标量**，而候选 `FlowSynthesisResult.nearness` 是 C++ 新增 N 维逐点内部记录；不得将其当作源码实际返回的向量，仍保持 N 次 uniform/3N 次 normal 等既有采样时序 | 已整改 | [修订后静态复核](reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md)，固定提交 `ac683f41af2f906137705f3ad31e8da53fe5c036`；初始审计见 [扫描报告](reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md) |

**本节刚登记时状态**：AF-021～025 为 **5 个未整改目标**；AF-001～020 为 **21 个已整改目标**。只有修改实际目标文件、在新提交复核并提供依据后，才能把这些新问题逐项改为「已整改」。[AUDIT.md](AUDIT.md) 的文件审计三态与本表的两态无直接等价关系。

## 七、AF-021～025 修订闭环（2026-11-07）

**履行顺序证据**：先以 `af9198906cf89b6fb5162aea1d74683906740d78` 完成 [新扫描报告](reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md)，再在原表 §六用 **「未整改」** 建立五行，然后依 [修订复读证据](reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md) 修改四份现行规范并在固定提交 `ac683f41af2f906137705f3ad31e8da53fe5c036` 进行定向复核。

- AF-021、AF-023：HLD 的普通版权复核不阻塞原创开发；首版 MAT 支持范围已选、正式 Reader/Writer/基线未通过。
- AF-022：SRS DEC-001 明确首版 v5/zlib、二维 dense real non-logical double 的既定范围；独立验证和额外类型支持没有被擅自批准。
- AF-024：原 Bias xwerte 与真实 N 分离；新增对照图不属于当前必要工作，不增加即时人工审批。
- AF-025：原 `fixed_*` 返回 `near_last` 标量，而新 C++ `FlowSynthesisResult.nearness` 是内部逐点记录，不能伪称原 MATLAB 直接返回该向量。
- 当前台账所有 AF-001～025 的**原子整改行共 26 条，26/26 为「已整改」**。这里的闭环仅指**文本/设计合同一致性**，独立 L3、数据写回读、真实 C++/CTest、Ivanic 论文细节及正式基线依然开放，文件审计三态由 [AUDIT.md](AUDIT.md) 单独维护。

历史「未整改」扫描事实只存放在前期固定报告及 §六的引言，**不是当前状态**；任何后续扫描发现回归，必须重新将对应**整改行**置为「未整改」并留下新固定 SHA。

## 八、七份在审文件的静态数学/来源复核：AF-026～027 的首次发现（2026-11-08）

> **先审计、先登记，后修订**。本轮扫描输入固定提交 `e190ed74240bf7b3b32a65c8fb6099fbdedc46b9`，扫描和原 `.m` 源码对照记录见 [七份数学文档审计报告](reviews/AUDIT_SEVEN_MATH_DOCS_SCAN_2026-11-08.md)。下列两条在目标文件修改前首次登记为**未整改**，不得因报告文字已经完成而直接改为“已整改”。

| 整改 ID | 文档文件相对路径（仓库根） | 具体整改目标 | 整改状态 | 固定扫描证据 |
|---|---|---|---|---|
| AF-026 | `docs/design/analysis_plotting.md` | §7 把分析模块的 `AN-G01～AN-G06` 编号替换为独立 **`APL-G01～APL-G06`**，因为全局 `GOLDEN_TOLERANCE_PROPOSAL.md`/JSON 已将 `AN-G01～AN-G12` 赋予**不同数学解析候选**；不得重命名原 JSON 的 12 个 ID，按现有 TC 映射保留六个分析场景 | 已整改 | [修订后静态复核](reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md)：固定提交 `22bc38635b2350bc9078c2952ba9ad520a55f7b1`，源 `.m`/候选 JSON 身份不变，限定文本校验通过 |
| AF-027 | `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | §1 将 `x_ref` 的“原 MATLAB 数值”改为**有来源身份的 L2 历史 MAT 数值或独立生成的 L3 数值参考**，L4 统计另审；保留仅作初轮筛查的 `atol=0.01,rtol=0.1` 与逐元素公式，不重新引入运行 MATLAB/L1 取证门禁 | 已整改 | [修订后静态复核](reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md)：固定提交 `22bc38635b2350bc9078c2952ba9ad520a55f7b1`，源 `.m`/候选 JSON 身份不变，限定文本校验通过 |

**扫描时状态**：旧 AF-001～025 的 26 个原子目标已有固定 SHA 的文档修订证据；新 AF-026～027 共 2 项，**在本节首次登记时均为「未整改」**。只有目标文件真正修改并经固定 SHA 独立复读验证，才可在本表将两行转为「已整改」，并在 `AUDIT.md` 同步本轮审计状态。

## 九、AF-026/027 后续纯文档整改结案（2026-11-08）

**已履行先发现/登记为「未整改」→修改两份规范→固定 SHA 复查→更新为「已整改」的顺序**。固定审计输入 `e190ed74240bf7b3b32a65c8fb6099fbdedc46b9`；修订后的固定复核输入 `22bc38635b2350bc9078c2952ba9ad520a55f7b1`；详细证据见 [七份文档的审计结案报告](reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md)。

- **AF-026 已整改**：分析六项改用 `APL-G01～APL-G06`，保留全局 JSON 的 `AN-G01～AN-G12`，避免参考身份冲突，不修改原公式/TC。
- **AF-027 已整改**：`x_ref` 明确为可追溯的 **L2 历史 MAT 或独立 L3 数值**，L4 统计单列，`0.01/0.1` 仍只是首轮宽松筛查。
- **当前问题级整改**：历史 AF-001～025 原子目标 26 条，加本轮两条，合计 **28/28 已整改，0 未整改**（限定为已发现的文档语义问题）；这不表示所有独立 L3 数值、球谐论文、C++/CTest、正式基线完成。


## 十、增量审计新发现：AF-028～031（2026-11-08，整改前登记）

> **固定审计输入**：`0fa760423532fda691c046f2e4eac5fc37784503`；独立发现报告：[AUDIT_FOUR_POLICY_DRIFTS_SCAN_2026-11-08.md](reviews/AUDIT_FOUR_POLICY_DRIFTS_SCAN_2026-11-08.md)。本节是**先发现、先登记「未整改」**的状态，不提前将设计方向选择误写为软件测试/最终验收批准。

| 整改 ID | 文档文件相对路径（仓库根） | 具体整改目标（发现时的冲突） | 整改状态 | 发现来源与检查点 |
|---|---|---|---|---|
| AF-028 | `docs/PROGRESS.md` | 当前“Golden 证据等级最新状态”仍写“候选 0.01/0.1 尚未批准”；改为**首轮探索筛查已由所有者选定，最终 DEC-005 模块 PASS 阈值尚未批准**，不更改 L2/L3/L4 和真实验收缺口。 | 未整改 | 输入 `0fa760423532fda691c046f2e4eac5fc37784503`，PROGRESS 原第 91 行；对照 SRS §8 DEC-005、NO_MATLAB_EXECUTION_POLICY §2。 |
| AF-029 | `docs/PROGRESS.md` | §5 DEC-001/002 行仍写“MAT 支持范围尚待审阅”；改为**MAT v5/zlib、二维密集实数非 logical double 首版支持范围已选**，正式 Reader/Writer、负例、L2 SHA-256 和完整基线仍未通过。 | 未整改 | 输入 `0fa760423532fda691c046f2e4eac5fc37784503`，PROGRESS 原第 106 行；对照 SRS §8 DEC-001/002、HLD ADR-001。 |
| AF-030 | `AGENTS.md` | 当前审批职责表“DEC-004 历史 MAT 额外字段归因”仍要求审批“来源归因与历史兼容策略”；应写清**DEC-004 处理政策已选，来源真实成因无需先查清**；只保留新 Runner/分析错误语义的实施与独立验证门槛。 | 未整改 | 输入 `0fa760423532fda691c046f2e4eac5fc37784503`，AGENTS 原第 123 行；对照 DEC-004 专题决定、SRS §8。 |
| AF-031 | `AGENTS.md` | 当前 DEC-001/002 职责行仍要求项目所有者再次批准“MAT 支持范围”；应保留**已决定的首版类型/形状范围**，将 L2 来源 SHA-256、正式 Reader/Writer、负例、规格冻结等未验收工作明确分开；FNV64 不等于 SHA-256。 | 未整改 | 输入 `0fa760423532fda691c046f2e4eac5fc37784503`，AGENTS 原第 124 行；对照 SRS DEC-001/002、HLD ADR-001。 |

**本轮登记后的当前状态**：此前 AF-001～027 共 28 个原子整改目标已修订并有固定复核证据；新 AF-028～031 **4 项未整改**，总计 **28 已整改、4 未整改**。必须先修改目标，再按新提交复读后改变四项状态。
