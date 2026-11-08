# 文档整改目标与进度（AUDIT_PROCESS）

> 只用 **未整改 / 已整改** 两态；与 [AUDIT.md](AUDIT.md) 的文件三态分开。历史结案仅证明**对应文字/合同**已修订，不代表数学、实现、测试或基线验收。

## 1. 更新规则

1. 扫描发现一个可独立核验的目标，立即登记 **AF ID、仓库根相对路径、目标、未整改**；一文件多目标逐行列出。
2. 仅在**修改实际目标文件、复读新固定提交并保留证据**后改为「已整改」；证据/人类批准类目标须实际取得证据或批准，不可仅改措辞关闭。
3. 历史审批/旧审计报告保留原日期、SHA 和当时状态；不改 `origin/**`、权限或历史 JSON。当前不运行被 H-10/H-11 暂缓的代码/命令/测试。

## 2. 已结案的历史目标（AF-001～031，32 项）

| ID | 文件 | 整改目标（简述） | 状态 |
|---|---|---|---|
| AF-001a | `docs/design/experiments.md` | 取消自定义输出根参数的旧语义 | 已整改 |
| AF-001b | `docs/design/experiments.md` | 固定 output/<RunId>、独占写入及 Writer 二次校验 | 已整改 |
| AF-002 | `docs/design/harmonics.md` | 球谐依 Ivanic 原文/勘误，纯 Eigen、禁 sdla | 已整改 |
| AF-003 | `docs/design/harmonics.md` | harmonics→contracts 已属 HLD 27 边 | 已整改 |
| AF-004 | `docs/design/contracts.md` | 失败继续/终止、不提交和 Manifest 策略定稿 | 已整改 |
| AF-005 | `docs/TEST_PLAN.md` | 区分历史 H-01 draft 与已选探索容差 | 已整改 |
| AF-006 | `docs/golden/NO_MATLAB_EXECUTION_POLICY.md` | 0.01/0.1 首轮筛查已选，最终阈值未批 | 已整改 |
| AF-007 | `docs/golden/NO_MATLAB_EXECUTION_POLICY.md` | 普通版权审查不阻碍原创开发 | 已整改 |
| AF-008 | `docs/design/mat_io.md` | Result/Failure 单一候选接口 | 已整改 |
| AF-009 | `docs/OBJECT_MODEL.md` | 统一 RealSphericalHarmonics::reconstruct | 已整改 |
| AF-010 | `docs/REQUIREMENTS.md` | MAT 结果只写固定 output/<RunId> | 已整改 |
| AF-011 | `docs/ARCHITECTURE.md` | 需旋转向量→矩阵的模块采用已选 Sophus | 已整改 |
| AF-012 | `docs/design/harmonics.md` | 球谐原文与 Eigen 重写来源表述 | 已整改 |
| AF-013 | `docs/design/kvd.md` | KvD 旧 1e−10 与现行首轮筛查区分 | 已整改 |
| AF-014 | `docs/design/kvd.md` | H-13 历史草案与现行失败纪律区分 | 已整改 |
| AF-015 | `docs/design/sampling_noise.md` | 采样失败传播与独立实验续跑策略 | 已整改 |
| AF-016 | `docs/OBJECT_MODEL.md` | Adaptive 失败不提交无效主系数 | 已整改 |
| AF-017 | `docs/ARCHITECTURE.md` | QA-01 改验固定根/RunId/软链/竞态 | 已整改 |
| AF-018 | `docs/design/mat_io.md` | MAT 严格 dtype/shape 不用宽松容差放行 | 已整改 |
| AF-019 | `docs/ARCHITECTURE.md` | ADR-002 标 Sophus 已选、未部署 | 已整改 |
| AF-020 | `docs/design/trajectory.md` | Rodrigues 原 epsilon 分支与额外阈值分离 | 已整改 |
| AF-021 | `docs/ARCHITECTURE.md` | HLD 删除普通版权审批开发门禁 | 已整改 |
| AF-022 | `docs/REQUIREMENTS.md` | SRS 明确 MAT 首版类型范围已选 | 已整改 |
| AF-023 | `docs/ARCHITECTURE.md` | HLD ADR-001 对齐 MAT 首版范围 | 已整改 |
| AF-024 | `docs/design/analysis_plotting.md` | Bias 历史 xwerte 与 N 分离；额外图非必需 | 已整改 |
| AF-025 | `docs/design/sampling_noise.md` | 原 fixed_* 返回 near_last 标量，非逐点向量 | 已整改 |
| AF-026 | `docs/design/analysis_plotting.md` | 分析 APL-G01～06 与全局 AN-G 分离 | 已整改 |
| AF-027 | `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | x_ref 来自可追溯 L2 或独立 L3 | 已整改 |
| AF-028 | `docs/PROGRESS.md` | PROGRESS 首轮容差已选、最终阈值未批 | 已整改 |
| AF-029 | `docs/PROGRESS.md` | PROGRESS MAT 首版类型范围已选 | 已整改 |
| AF-030 | `AGENTS.md` | AGENTS 不重复审批 DEC-004 来源归因 | 已整改 |
| AF-031 | `AGENTS.md` | AGENTS 不重复审批 MAT 首版范围 | 已整改 |

**历史结论**：32 项文字修订已按先登记、后改源、再复读流程结案；证明范围不含 C++/CTest/L2-L4 真实数值。详细证据仍由 `docs/reviews/AUDIT_*.md` 与 [PROGRESS.md](PROGRESS.md) 按当时固定版本保存，不再复制提交哈希。

## 3. 本轮新登记的整改目标（AF-032～062）

**口径**：前 5 项为低优先级文字/治理事项；余下 25 项为阶段性设计、真实证据与批准门槛，并非仅靠文字修改就能结案。项目处于文档阶段，**首次发现时全部先记「未整改」**，下表为整改后当前状态。实际实现/运行/签署均须尊重授权与 [TODO.md](TODO.md)；不要求 MATLAB/L1。

| ID | 类型 | 仓库根相对路径 | 具体整改/验收目标 | 状态 |
|---|---|---|---|---|
| AF-032 | 文字·低 | `docs/REQUIREMENTS.md` | §5 将“来源未明”统一为 DEC-004：当前有效 save／历史 HistoricalOnly／分析缺字段报错；历史成因不作前置门禁。 | 已整改 |
| AF-033 | 文字·低 | `docs/design/mat_io.md` | §4.1 标清 alleadap*/rottransratio 的 HistoricalOnly 身份；不要求先归因，不扩大当前 Writer save 字段。 | 已整改 |
| AF-034 | 文字·低 | `docs/design/experiments.md` | §5 五入口对照表简化“来源未知/未证实”残留，统一 HistoricalOnly、当前 save 及缺字段失败语义。 | 已整改 |
| AF-035 | 文字·低 | `docs/TEST_PLAN.md` | §5.1 with_errors 七份历史 MAT 中两份 alleadap* 显式标 HistoricalOnly；保留真实 shape 与 7/7 读取事实。 | 已整改 |
| AF-036 | 治理·低 | `docs/DOCUMENT_BASELINE_REGISTER.md` | 解释该矩阵 36 项固定版本/身份登记与 AUDIT 本轮 50 个审计对象口径不同；新增审计对象不自动成为已冻结基线。 | 已整改 |
| AF-037 | 基线·阶段 | `docs/REQUIREMENTS.md` | 正式冻结前复核 66 REQ/AC、附录 8 REQ-DC 内部约束身份，并为 AC→LLD 章节绑定精确版本/SHA/批准；DEC-005/006/008 及例外明确。 | 未整改 |
| AF-038 | 设计·阶段 | `docs/ARCHITECTURE.md` | 复核 27 边 DAG、部署/质量场景、Composition Root→固定 output/<RunId>→Writer 与 CLI/mat_io/Runner 一致；动态探针/DEC 证据仅引用，待真实构建再关闭。 | 未整改 |
| AF-039 | 设计·阶段 | `docs/OBJECT_MODEL.md` | 对齐 45 .m 职责映射、类名/所有权/Adaptive 状态机及 contracts/LLD 候选签名；正式冻结需一致且有批准记录。 | 未整改 |
| AF-040 | 设计·阶段 | `docs/design/contracts.md` | 冻结 Result/Status、Failure、ErrorDetail、异常/线程与 valueless_by_exception 行为；MatDenseArray 仅引用 mat_io，不重复定义；真实编译/ABI 与错误传播待验证。 | 未整改 |
| AF-041 | 设计·阶段 | `docs/design/cli.md` | 冻结负噪声参数边界、自动/手工 RunId、JSONL 转义、错误码/退出码及优先级；以 contracts 为唯一来源，真实 CLI 负例待测试。 | 未整改 |
| AF-042 | 验证·阶段 | `docs/design/mat_io.md` | 在授权后实现/验收 Reader/Writer、MatDenseArray、严格/兼容读、列主序、RAII、TOCTOU/路径隔离/非覆盖、负例与写回读；以独立 L2/来源 SHA-256 核验。 | 未整改 |
| AF-043 | 验证·阶段 | `docs/design/kvd.md` | 补 Franz/Original/Modified KvD 的逐轮 L3 参考（near/t/ω、停止阈值、次数、失败分类）；最终 DEC-005 阈值与 CTest 待批准/执行。 | 未整改 |
| AF-044 | 验证·阶段 | `docs/design/adaptive_mfa.md` | 补 M1～M4/6×6/rhs/MSE/near_new/C_new 的独立 L3；验证 Session 主/对照状态、异常原子不提交及周期刷新。 | 未整改 |
| AF-045 | 验证·阶段 | `docs/design/harmonics.md` | 取得 1996 原论文/1998 勘误后逐式核 R0/R3/R5、九项实基的 m/相位；Eigen-only 禁 sdla，验证 R3 全9/R5 全25元素并保留必要来源 notice。 | 未整改 |
| AF-046 | 设计·阶段 | `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 专题声明与 harmonics/object model 的文件、命名空间、类/方法一致；论文确认后才推进 Eigen 递推实现及独立 L3/CTest。 | 未整改 |
| AF-047 | 验证·阶段 | `docs/design/sampling_noise.md` | 补 i=1/2 有序方向、缺损球面、N uniform/3N normal、固定 RNG 耗尽/非有限样本的独立 L3 与 CTest；保留 near_last 标量区别。 | 未整改 |
| AF-048 | 验证·阶段 | `docs/design/trajectory.md` | 验证 Sophus/原 epsilon/反向向量/近 π、Rᵀ、球内近度正根及端点零值；明确 sampling/flow 的接口边界与独立 L3。 | 未整改 |
| AF-049 | 验证·阶段 | `docs/design/experiments.md` | 实现/核五个 Runner 的当前 save、Writer 注入、RunContext/Manifest/失败继续策略、8 次聚合分母；旧 MAT 只读、HistoricalOnly 不补造。 | 未整改 |
| AF-050 | 验证·阶段 | `docs/design/analysis_plotting.md` | 对四类分析的 L2→独立 L3 曲线、begin/end/zone、平滑边缘、单位/索引和 MissingVariable 负例逐项复核；绘图后端可选，不阻塞数值。 | 未整改 |
| AF-051 | 验证·阶段 | `docs/TEST_PLAN.md` | 将 66 TC→候选实体/CTest→AC 转成获授权的真实测试与日志，覆盖负例、Session、Writer、L2/L3/必要 L4；不得以 matio 探针宣称整体验收。 | 未整改 |
| AF-052 | 治理·阶段 | `docs/PLAN.md` | 复核 98 P 与 SRS/HLD/LLD/TC 依赖和里程碑退出证据；只保留稳定 WBS、通过 PROGRESS 引用实测，不捏造进度或工期。 | 未整改 |
| AF-053 | 证据·阶段 | `docs/PROGRESS.md` | 在获授权实际实施/测试后回填固定源码/输入身份、命令、stdout/stderr/exit、失败数、wall/CPU/RSS 与里程碑证据；历史 E-DOC 不能当最新 HEAD PASS。 | 未整改 |
| AF-054 | 证据·阶段 | `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | 以独立 L2/L3/适用 L4 及真实 C++ 误差确定 DEC-005 最终分模块 PASS 阈值；0.01/0.1 仅首轮筛查，几何/计数/shape 单列严格判据。 | 未整改 |
| AF-055 | 证据·阶段 | `docs/golden/analytic_golden_candidates.v1.json` | 核验 AN-G01～12 的固定输入、公式与参考值来源，补独立 L3 核对；不得将候选 JSON 或 AN/APL 编号一致当成已执行测试。 | 未整改 |
| AF-056 | 证据·阶段 | `docs/MAT_DATA_SCHEMA.md` | 独立复核 582 MAT 的文件身份/SHA-256、变量与 shape、L2 原始 double 逐值；FNV64 与 SHA-256 分开，不改写 origin/**。 | 未整改 |
| AF-057 | 治理·阶段 | `docs/TODO.md` | 在进入相应阶段时收齐论文 PDF、H-10/H-11 操作授权、DEC-005/006、正式基线/发布批准；已选 MIT/DEC-004/CLI 等不得重复申请。 | 未整改 |
| AF-058 | 治理·阶段 | `docs/DOCUMENT_BASELINE_REGISTER.md` | 正式签署前更新文档版本/blob/commit/范围/批准人/日期时区/例外及测试证据；滚动 PROGRESS/AUDIT 不伪装为已冻结规范。 | 未整改 |
| AF-059 | 设计·阶段 | `docs/design/README.md` | 逐模块核对 LLD 签名、唯一合同归属、独立 L3/TC 映射与真实代码边界，建立可编码/可验证的冻结清单；不以目录齐全等同实现。 | 未整改 |
| AF-060 | 证据·阶段 | `docs/golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | 逐 AC 复核 L2 逐值、L3 一般/退化/逐节点、适用 L4 的缺口与实际证据；L1/MATLAB 永久不列为必需门槛。 | 未整改 |
| AF-061 | 证据·阶段 | `docs/PLAN.md` | DEC-006 性能/资源目标基于未来真实全工程构建及运行 wall/CPU/RSS、平台事实与批准标准确定；VM 容量快照不能当性能验收。 | 未整改 |
| AF-062 | 治理·低 | `docs/design/README.md` | §1 不再将 36 个选定文档的固定版本身份矩阵称作“全部当前规范和过程文档”；另引用 AUDIT 50 条审计路径，明确两者用途及非冻结边界。 | 已整改 |

**AF-062 结案时的历史统计**：32 项历史 + 6 项纯文档结案，25 项阶段目标仍开放；当时共 **63 项（已整改 38、未整改 25）**。后续增量结论见 §6。治理文档的“已审计”不等于这些目标“已整改”；冻结或真实测试证据缺失时，禁止将阶段门槛提前关闭。

## 4. 本轮六项文档修订结案（2026-11-08）

- **先审计并登记**：输入 `6c22fe7`，审计登记提交 `ae87532`，六项在修改前均标「未整改」。
- **实际修改与复读**：SRS、mat_io、experiments、TEST_PLAN、LLD 索引、PLAN 的文本修订提交 `3e3ad2a`；AF-032～035、AF-062 五项定向静态检查通过。控制矩阵 AF-036 在提交 `2d608b9` 完成，**36/36 个登记文件的 blob SHA 均匹配其明确标注的固定快照**；非最新动态 HEAD 的验收状态。
- **边界**：AF-037～061 的 25 个分阶段目标仍需实际 C++/CTest、独立 L2/L3/L4、性能数据、DEC-005/006/008 和相应批准，不得以静态审计关闭；原 `origin/**` 与历史审批/报告未变。

## 5. 新发现的四项文档冲突（2026-11-08：先登记）

**扫描输入**：`349206743af23abf039cbe5e97ea8b8c13341608`。以下问题已在修改目标文件**之前**登记为「未整改」，不得以审计报告完成代替整改。

| ID | 仓库根相对路径 | 整改目标 | 状态 |
|---|---|---|---|
| AF-063 | `docs/TEST_PLAN.md` | §3 五个 TC 行引用的 `docs/SOURCE_INVENTORY.md` 尚不存在；明确它是 P0.6 待创建交付物，并引用现存 SRS §9.2 / OBJECT_MODEL / MAT_DATA_SCHEMA 的相应可用证据，不伪称来源审计完毕。 | 已整改 |
| AF-064 | `docs/AUDIT.md` | 追踪计数明确为 58 正文 REQ + 8 REQ-DC = 66 总 REQ，TC 同理；不得把“66 + 8”重复计数。 | 已整改 |
| AF-065 | `docs/AUDIT.md` | 逐文件进度表移除已整改 AF-032～036、AF-062 的“待”标签，只保留实际尚未完成的 AF-037～061。 | 已整改 |
| AF-066 | `docs/AUDIT.md` | §4 登记阶段的“当时尚未整改”与 §7 当前已完成的六项结案分开标注，避免旧进度冒充最新状态。 | 已整改 |

**登记后两态**：总 67 项，其中历史已整改 38、新发现未整改 4、阶段未整改 25；故已整改 **38**、未整改 **29**。后续必须先修改对应文件并在新固定提交复读，再逐项结案；历史报告/审批 JSON、`origin/**` 不改。

## 6. AF-063～066 定向文档整改完成（2026-11-08）

先按固定提交 `3492067` 审计，在 `795afb7` 登记四条「未整改」，再实际修改 `docs/TEST_PLAN.md` 和 `docs/AUDIT.md`，并在固定提交 `edd56c99dc04e3774788699ed61c26d3bad3c465` 逐条复读：**4/4 通过**。其中 TEST_PLAN v0.8 的五条历史引用已改为真实可用文档与“计划、未创建”的 SOURCE_INVENTORY 两类；58 正文 + 8 REQ-DC = 66 REQ，66 TC 包含 8 TC-DC。此次没有创建测试、SOURCE_INVENTORY 或任何新数据。

**最新两态：67 条原子目标，42 已整改，25 未整改（AF-037～061）。** 所有仍未关闭的目标需要正式设计/代码、独立 L2/L3/L4、CTest、性能或人类批准的实际证据，不以本轮纯文本核查代替。
