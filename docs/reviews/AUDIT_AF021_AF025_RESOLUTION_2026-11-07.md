# AF-021～AF-025：先审计后整改的固定快照复核（2026-11-07）

> **仅文档合同复核，不是正式设计基线、数值等价或软件通过证明。** 审计固定输入见 [AUDIT_FOLLOWUP_SCAN_2026-11-07.md](AUDIT_FOLLOWUP_SCAN_2026-11-07.md)，由 `af9198906cf89b6fb5162aea1d74683906740d78` 的现行文档及只读 MATLAB 源码得出。扫描后先以「未整改」登记 [AUDIT_PROCESS.md](../AUDIT_PROCESS.md)，然后才修订实际目标文件。本报告记录**修改后的限定范围静态复核**，不追溯改写“最初未整改”的历史事实。

## 1. 固定提交与审查范围

- 仓库：`ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields`；分支：`dev-plan-P0-P1-mat-dictionary-probe`。
- **审计输入 SHA**：`af9198906cf89b6fb5162aea1d74683906740d78`，报告在先创建。
- **AF-021～025 全部文档修订及版本同步后的只读复核 SHA**：`ac683f41af2f906137705f3ad31e8da53fe5c036`。本报告及台账后续提交发生在此快照之后。
- 本轮现行目标：`docs/ARCHITECTURE.md`、`docs/REQUIREMENTS.md`、`docs/design/analysis_plotting.md`、`docs/design/sampling_noise.md`。同步版本引用：`docs/PLAN.md`、`docs/TEST_PLAN.md`、`docs/design/README.md`。
- 未修改 `origin/**` 或任何生产 C++、CMake、CTest、Bash 脚本、MAT、历史审批 JSON/旧审查报告，未运行 MATLAB/Octave/L1，也没有真实 L2/L3/L4 数值验收。

## 2. 五项问题的原子修订与静态复核

| 整改 ID | 修订文档 | 具体整改内容 | 在固定修订提交的复核结果 |
|---|---|---|---|
| **AF-021** | `docs/ARCHITECTURE.md` §1 | 一般版权复核不再写作原创 MIT C++ 开发门禁；但实际引入第三方文件时须保留必要原有来源/notice，真实 CMake/CTest、接口、性能/基线证据义务不变 | **通过**：旧“CMake/CTest、版权、接口与指标审批门禁保持”句已消除，现行区分已选 MIT 与实际工程证据 |
| **AF-022** | `docs/REQUIREMENTS.md` DEC-001 | 项目首版只支持 MAT v5/zlib 的二维 dense real non-logical double **已经选定**；现存 582/582 格式事实与正式业务 Reader/负例/数据身份/最终基线未验收分别记录 | **通过**：有已选具体首版类型范围，不再笼统“项目支持范围待所有者批准”，不擅自承诺 v7.3/HDF5/其它类型 |
| **AF-023** | `docs/ARCHITECTURE.md` ADR-001 | 与 SRS DEC-001 一致，首版 MAT 类型范围已选；正式 Reader/Writer、负例、CMake/数值证据及最终基线仍待完成 | **通过**：已选范围与尚缺真实实现明确分开，HLD 现有 27 条内部分层依赖不变 |
| **AF-024** | `docs/design/analysis_plotting.md` §5.3 | 历史 `xwerte=6·4^i` 与完整球面实际方向 `N=8·4^i` 严格分开；可选 `actual_flow_count` 只作新元信息，另一张实际 N 对照图不属于当前必要交付，不能因其未选阻塞核心 Analyzer | **通过**：不再要求当前所有者决定是否“保留另一种对照图”，保留原画图/数值合同 |
| **AF-025** | `docs/design/sampling_noise.md` §6 | 根据原两份 `fixed_*_randomnear.m` 的逐点 `near=1/entf` 标量覆盖，说明**原函数返回 near 是最后一次采样标量**；`FlowSynthesisResult.nearness` 为 C++ 新增自拥有逐点向量，不能冒充源函数输出/MAT 已存字段，RNG/采样次序不变 | **通过**：文档具备独立的源返回标量与 N 维内部记录说明；源码证据分别为 `fixed_kugel_...` 47–55 行、`fixed_propfehler_...` 45–53 行 |

以上五项是**文档措辞及数据合同边界的修订**。未添加生成历史额外 MAT、未经请求的实际 N 对照图，也未把项目新内部字段错误说成原作者的保存变量。为了保留科学追踪，修订后仍需通过真实 L3/数值测试核验采样/完整实验中该新增内部向量的逐点数值与生命周期。

## 3. 同步版本与独立检查

- **独立草案版本**：SRS `v0.12`、HLD `v0.8`、分析 LLD `v0.3`、采样/噪声 LLD `v0.3`；PLAN/TEST_PLAN/LLD 索引 `v0.6`。它们都是**未冻结草案**，没有确认任何 TC PASS 或获得 H-10/H-11 编码/运行授权。
- 在修改提交 `ac683f41af2f906137705f3ad31e8da53fe5c036` 的现行文档中，对 **AF-021～025 的 5 个定向新旧语义检查全部通过**，相关 7 个版本/引用检查也全部通过。
- 在该快照中，**66 REQ、66 TC、98 P、8 REQ-DC** 及 REQ↔AC↔TC↔P 的限定集合映射无不一致；HLD §4.1 **27 条设计 DAG 边、静态无环**；原 `origin/**` Git tree 为 `70ee155131c24a50169346d0ffbaff0c01b1c547`（45 .m/582 .mat）。
- 对本次七份改动的现行文档检查 **221 条相对 Markdown 文件链接**，缺失目标 0，未配平代码围栏 0；此检查不代表每个标题 slug 已渲染。
- **没有**运行 Bash、CMake、CTest、MATLAB、任何仿真或对原 MAT 的新读写；没有产生独立 L3 数值、最终 DEC-005 容差、DEC-006 性能阈值或正式批准基线。

## 4. 整改进度与后续需核验部分

在 [AUDIT_PROCESS.md](../AUDIT_PROCESS.md) 的整改状态中，**现在可以将 AF-021～025 五项从「未整改」改为「已整改」**，应引用本报告和上述固定 SHA；此前 AF-001～020 的已整改目标照旧保留，各自有独立历史复核报告。

在 [AUDIT.md](../AUDIT.md) 的**文件审计**进度中：SRS/HLD 的本轮合同可回到「已审计」，而采样/噪声、分析/绘图仍欠全部逐式独立 L3/真实测试，故即使其 AF-024/025 已修复，也**仍可保持「正在审计」**。项目原创 MIT、Eigen-only/no-sdla、固定输出根、DEC-004、L1 不必需和默认失败安全策略均维持不变。

**此结案仅对应上面 5 个具体整改目标，不表示所有文档无其它潜在问题，也不表示软件或数学已经验收。**
