# AUDIT 后续扫描：AF-021～AF-025（2026-11-07）

> **审计先于整改。** 本报告是在任何 AF-021～025 源文件修改之前，以固定 Git commit `af9198906cf89b6fb5162aea1d74683906740d78` 的现行设计和只读 MATLAB 源码为依据完成。这里只记录**发现时的事实、优先级、可复核证据与拟议整改目标**。此报告本身不是对问题的修复，不是 C++ 源码/CTest/实验运行或正式基线批准；后续整改状态以 [AUDIT_PROCESS.md](../AUDIT_PROCESS.md) 为准。

## 一、审计方法和范围

- 仓库：`ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields`；分支：`dev-plan-P0-P1-mat-dictionary-probe`；输入 commit：`af9198906cf89b6fb5162aea1d74683906740d78`。
- 按 [AUDIT.md](../AUDIT.md) 的 AU-001～028 检查**文档类型与单一来源、已确定的所有者决定、REQ/TC/P/target 追踪、MAT/L2/L3/L4 资料、CLI 输出、错误/失败、球谐纯 Eigen、原 MATLAB 只读源码公式**；审核前依 [AUDIT_PROCESS.md](../AUDIT_PROCESS.md) 确认 AF-001～020 既有目标均已整改，并核查原历史两份整改报告的固定快照限制。
- GitHub 连接器读取当前 Markdown/README/AGENTS/决策/历史 reviews/源函数。**按三组 12 份与一组 7 份的文件只读核查共 43 个文档/项目文本**（包括 AUDIT 追踪的现行文件、额外历史记录及 .gitignore/LICENSE/脚本），合计 **537 条相对 Markdown 文件引用**，当前 Git tree 无缺失目标、无未配平围栏。此处未以 HTML 渲染器重新验证所有页内标题锚点。
- 编号/依赖只读检查：**66 REQ、66 TC、98 P、8 REQ-DC、HLD 27 条静态 DAG 边无环**；版本登记固定 blob 计数 **32 项**，未发现与该快照的 Git tree 不符。
- 另外抽查只读原 `.m`（固定/缺损八面体、Franz、两条 KvD、Adaptive 单步与三个生成入口、轨迹、噪声、MAT load、平滑及球谐函数），**只对明确列出的公式和赋值分支核查**。未取得 Ivanic–Ruedenberg 上传 PDF，未进行 L3 数值生成、MAT 逐值重读、SHA-256 采集或任何 Bash/CMake/CTest/MATLAB。
- 本轮源归档 Git subtree 为 `70ee155131c24a50169346d0ffbaff0c01b1c547`，含 **45 .m/582 .mat**；本审计没有修改原档。报告生成以后如有分支更新，不得把此输入 SHA 的“检查通过”推广到更新后的 HEAD。

## 二、新发现：每条问题都必须进入 AUDIT_PROCESS，初始为「未整改」

| ID | 优先级 | 需要整改的现行文档与位置（审计输入固定 SHA） | 源证据与问题 | 整改目标 |
|---|---|---|---|---|
| **AF-021** | 中 | `docs/ARCHITECTURE.md:7`，长期证据边界 | “真实 CMake/CTest、**版权**、接口与指标**审批门禁保持**”把普通版权复核继续列作开发门禁，违背所有者已明确的原创代码采用 MIT、普通版权审查不阻塞开发的选择 | 改为保留**实际上游文件必要来源/现有 notice**，并明确这**不是另一个研发审批门禁**；真实 CMake/CTest、接口/指标、正式基线要求仍不豁免 |
| **AF-022** | 中 | `docs/REQUIREMENTS.md:205` 的 DEC-001 | 582/582 历史 MAT v5+zlib 的**事实**已证实；所有者又已确定**首版读取只支持二维、密集、实数、非 logical 的 double**；现行句尾仍称“项目支持范围待所有者批准”，可能让开发者再次请求已选的范围决定 | 明确将**首版范围已选定**与**正式 Reader 实现/负例/最终基线批准仍未完成**区分；不承诺未选的 v7.3/HDF5、稀疏/复数/其它类型 |
| **AF-023** | 中 | `docs/ARCHITECTURE.md:248` 的 ADR-001 | HLD 说“**项目所有者待批准支持范围**”，与 SRS/最新技术决定及 MAT I/O LLD 中首版二维 dense real double 的已选界限不一致 | 在 ADR-001 将第一版数据合同标为**方向已确定、实现/兼容与读回负例未验收**，保留 582 归档探针与正式 Reader 未建立的事实 |
| **AF-024** | 低 | `docs/design/analysis_plotting.md:101`，Bias xwerte | 已明确原版 `xwerte=6·4^i` 必须保留、实际完整球面样本 `8·4^i` 仅能作为**新增独立元信息**；但“待项目所有者决定是否保留另一种对照图”容易将一个尚未提出的可选新图表误写成核心功能待审批项 | 明确原版图和数据合同无需再决定；物理实际样本数如作为元信息须正确标注；**额外对照图不在当前必要交付/验收范围**，仅将来出现明确新需求时另行评审，不阻塞 Analyzer/LLD |
| **AF-025** | 中 | `docs/design/sampling_noise.md:169–219`，`FlowSynthesisResult.nearness` 与两个 `fixed_*_randomnear.m` | **原源码输出语义易混淆**：`fixed_kugel_...m` **47–55**、`fixed_propfehler_...m` **45–53** 内，`near=1/entf` 在逐点 for 循环反复赋给**标量**，没有 `near(i)`；其 MATLAB 返回参数 `near` 因而是**最后一次抽样的标量**，而 LLD 中的 `Eigen::VectorXd nearness` 是更完整的逐点内部记录。不能默认为逐点向量就是原作者同名返回值 | 在 `sampling_noise.md` 明确**源返回 near_last 标量 vs C++ 新增 nearness(N) 内部值对象**，记录两个固定流生成函数的原 `near` 返回语义；如需对照源 API/原值，使用最后一点的标量且区分未定义 N=0，不把完整向量伪称“MATLAB 原返回 near”。原 N 次采样/RNG 消费顺序不变 |

**数量：5 项新增整改目标，来自 4 份现行规范文件：HLD 两项、SRS 一项、分析 LLD 一项、采样/噪声 LLD 一项。** 它们属于**文档合同一致性**问题，不批准额外算法功能或更改原数学。单项整改状态首次登记统一为**「未整改」**，在真正修订与固定 SHA 复读以后才允许改「已整改」。

## 三、此次源码交叉审计确认无须改动的关键行为

- `norm_koenderink_iteration.m` 与 `mue_norm_koenderink_iteration.m` 的平移项分别含 `near²` 与 `near`，旋转使用**提交新 t 前的旧 t**；迭代至少 6 次、最多 200 次；Franz 初值来自 `franzeigen.m`。对照 `docs/design/kvd.md` 的现有路径/时序**在抽查范围内**一致。
- `erzeugerichtungsvektoren*.m` 和 `findeDreiecke.m` 的八面体 8 面与遮挡版本选面 2/3/5/6/7/8、A/B/C/D 子面顺序、归一化面心与 `sampling_noise.md` 的规定一致。Bias 中的两次 `getvektor`、重新播种、N 次 uniform 再 3N 次 normal 语义未发现回归。
- `kreis_trajektorie.m` / `sinuskurve_trajektorie.m` 使用 `stellen` 作为角度/位置离散分母；`trajektorie_aus_punkten.m` 保留 `R^T` 局部坐标以及端点未填充；`getnear_in_kugel_Drehung.m` 的 `1/(-b+sqrt(b²-c+1))` 与轨迹 LLD 抽查一致。
- `oneit_adaptiv_matched_filter.m` 的 M1～M4 及 `error` 返回 **MSE**，另算但不返回 `sqrt(error)`；`koeff_aus_near.m` 逐项投影、`near_aus_koeff.m` 3×5 有效项求和；对照 Adaptive/球谐现有文档限定范围一致。
- 历史字段按 DEC-004：`agent_in_sphere_adaptive_matched_filter_many_parameter.m` 仅实际 `save` 两个 `allemit*` 及 `akrot`，其旧 MAT 中的 `alleadap*` 与 `rottransratio` 为 `HistoricalOnly`，源定义未因本轮文档扫描变化。
- `bias_test_spherical_prop_error.m` 源中 `xwerte(i+1)=(8·4^i)·(6/8)`，就是 **6·4^i**；但生成 `fixed_propfehler...` 的方向数为 8·4^i，故原图 x 不能被默改为样本总数。此检查支持 AF-024 的范围界定，不授权新图表。
- 原 1996/1998 PDF **尚未由用户上传**，因此未声称 R5 每个递推符号/下标通过独立原始论文核验。

## 四、后续按 AUDIT_PROCESS 整改的复核要求

1. 先将 AF-021～025 以**文件相对路径、具体目标、状态「未整改」**写入 [AUDIT_PROCESS.md](../AUDIT_PROCESS.md)，标注本报告和固定输入 SHA；旧 AF-001～020 的“已整改”以此前两份修订报告限定证据维持。
2. 然后只修改四份现行文档的**以上明确语义**，不更动原论文/公式/决定，也不添加算法具体实现。对变更的草案文件独立升级各自版本号（若有）并同步 SRS→PLAN/TEST_PLAN 版本引用。
3. 以**修订后的固定 Git SHA**再次检查 AF-021～025 原句消除、新口径存在、66 REQ/66 TC/98 P、HLD 27 边无环、45 .m/582 .mat 原资料未改、Markdown 路径无断链；另建整改复核证据，最后在 [AUDIT_PROCESS.md](../AUDIT_PROCESS.md) 标为「已整改」并同步 [AUDIT.md](../AUDIT.md)、[PROGRESS.md](../PROGRESS.md)、[DOCUMENT_BASELINE_REGISTER.md](../DOCUMENT_BASELINE_REGISTER.md)。
4. 已整改**不代表** KvD/Adaptive/采样/轨迹/分析/球谐完整逐式数学或独立 L3 已完成；这些文件若仍欠独立 L3/原论文/真实 CTest，可在 AUDIT 的**文件审计三态**保留「正在审计」。不要求现在取得 H-10/H-11 或最终 DEC-005/006 的人工签署。

**此报告仅是先审计的证据。以下整改必须发生在本文件与台账初始“未整改”状态建立以后。**
