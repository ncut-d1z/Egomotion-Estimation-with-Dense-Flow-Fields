# 审计整改进度（AUDIT_PROCESS）

> **问题两态**：仅「未整改 / 已整改」。一条目标一行，**实际修改目标并以新固定提交复核**才能记已整改；软件/数学/权限/正式批准目标必须有真实证据，不可用文档修改代替。文件审计的「空白 / 正在审计 / 已审计」独立见 [AUDIT](AUDIT.md)。原 `origin/**` 只读，H-10/H-11 现未授权运行/实施，MATLAB/L1 永久不必需。

## 1. 最新整改台账（按 ID 唯一登记）

| ID | 相对路径 | 独立整改/验收目标 | 状态 |
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
| AF-032 | `docs/REQUIREMENTS.md` | §5 将“来源未明”统一为 DEC-004：当前有效 save／历史 HistoricalOnly／分析缺字段报错；历史成因不作前置门禁。 | 已整改 |
| AF-033 | `docs/design/mat_io.md` | §4.1 标清 alleadap*/rottransratio 的 HistoricalOnly 身份；不要求先归因，不扩大当前 Writer save 字段。 | 已整改 |
| AF-034 | `docs/design/experiments.md` | §5 五入口对照表简化“来源未知/未证实”残留，统一 HistoricalOnly、当前 save 及缺字段失败语义。 | 已整改 |
| AF-035 | `docs/TEST_PLAN.md` | §5.1 with_errors 七份历史 MAT 中两份 alleadap* 显式标 HistoricalOnly；保留真实 shape 与 7/7 读取事实。 | 已整改 |
| AF-036 | `docs/DOCUMENT_BASELINE_REGISTER.md` | 解释该矩阵 36 项固定版本/身份登记与 AUDIT 本轮 50 个审计对象口径不同；新增审计对象不自动成为已冻结基线。 | 已整改 |
| AF-037 | `docs/REQUIREMENTS.md` | 正式冻结前复核 66 REQ/AC、附录 8 REQ-DC 内部约束身份，并为 AC→LLD 章节绑定精确版本/SHA/批准；DEC-005/006/008 及例外明确。 | 未整改 |
| AF-038 | `docs/ARCHITECTURE.md` | 复核 27 边 DAG、部署/质量场景、Composition Root→固定 output/<RunId>→Writer 与 CLI/mat_io/Runner 一致；动态探针/DEC 证据仅引用，待真实构建再关闭。 | 未整改 |
| AF-039 | `docs/OBJECT_MODEL.md` | 对齐 45 .m 职责映射、类名/所有权/Adaptive 状态机及 contracts/LLD 候选签名；正式冻结需一致且有批准记录。 | 未整改 |
| AF-040 | `docs/design/contracts.md` | 冻结 Result/Status、Failure、ErrorDetail、异常/线程与 valueless_by_exception 行为；MatDenseArray 仅引用 mat_io，不重复定义；真实编译/ABI 与错误传播待验证。 | 未整改 |
| AF-041 | `docs/design/cli.md` | 冻结负噪声参数边界、自动/手工 RunId、JSONL 转义、错误码/退出码及优先级；以 contracts 为唯一来源，真实 CLI 负例待测试。 | 未整改 |
| AF-042 | `docs/design/mat_io.md` | 在授权后实现/验收 Reader/Writer、MatDenseArray、严格/兼容读、列主序、RAII、TOCTOU/路径隔离/非覆盖、负例与写回读；以独立 L2/来源 SHA-256 核验。 | 未整改 |
| AF-043 | `docs/design/kvd.md` | 补 Franz/Original/Modified KvD 的逐轮 L3 参考（near/t/ω、停止阈值、次数、失败分类）；最终 DEC-005 阈值与 CTest 待批准/执行。 | 未整改 |
| AF-044 | `docs/design/adaptive_mfa.md` | 补 M1～M4/6×6/rhs/MSE/near_new/C_new 的独立 L3；验证 Session 主/对照状态、异常原子不提交及周期刷新。 | 未整改 |
| AF-045 | `docs/design/harmonics.md` | 取得 1996 原论文/1998 勘误后逐式核 R0/R3/R5、九项实基的 m/相位；Eigen-only 禁 sdla，验证 R3 全9/R5 全25元素并保留必要来源 notice。 | 未整改 |
| AF-046 | `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 专题声明与 harmonics/object model 的文件、命名空间、类/方法一致；论文确认后才推进 Eigen 递推实现及独立 L3/CTest。 | 未整改 |
| AF-047 | `docs/design/sampling_noise.md` | 补 i=1/2 有序方向、缺损球面、N uniform/3N normal、固定 RNG 耗尽/非有限样本的独立 L3 与 CTest；保留 near_last 标量区别。 | 未整改 |
| AF-048 | `docs/design/trajectory.md` | 验证 Sophus/原 epsilon/反向向量/近 π、Rᵀ、球内近度正根及端点零值；明确 sampling/flow 的接口边界与独立 L3。 | 未整改 |
| AF-049 | `docs/design/experiments.md` | 实现/核五个 Runner 的当前 save、Writer 注入、RunContext/Manifest/失败继续策略、8 次聚合分母；旧 MAT 只读、HistoricalOnly 不补造。 | 未整改 |
| AF-050 | `docs/design/analysis_plotting.md` | 对四类分析的 L2→独立 L3 曲线、begin/end/zone、平滑边缘、单位/索引和 MissingVariable 负例逐项复核；绘图后端可选，不阻塞数值。 | 未整改 |
| AF-051 | `docs/TEST_PLAN.md` | 将 66 TC→候选实体/CTest→AC 转成获授权的真实测试与日志，覆盖负例、Session、Writer、L2/L3/必要 L4；不得以 matio 探针宣称整体验收。 | 未整改 |
| AF-052 | `docs/PLAN.md` | 复核 98 P 与 SRS/HLD/LLD/TC 依赖和里程碑退出证据；只保留稳定 WBS、通过 PROGRESS 引用实测，不捏造进度或工期。 | 已整改 |
| AF-053 | `docs/PROGRESS.md` | 在获授权实际实施/测试后回填固定源码/输入身份、命令、stdout/stderr/exit、失败数、wall/CPU/RSS 与里程碑证据；历史 E-DOC 不能当最新 HEAD PASS。 | 未整改 |
| AF-054 | `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | 以独立 L2/L3/适用 L4 及真实 C++ 误差确定 DEC-005 最终分模块 PASS 阈值；0.01/0.1 仅首轮筛查，几何/计数/shape 单列严格判据。 | 未整改 |
| AF-055 | `docs/golden/analytic_golden_candidates.v1.json` | 核验 AN-G01～12 的固定输入、公式与参考值来源，补独立 L3 核对；不得将候选 JSON 或 AN/APL 编号一致当成已执行测试。 | 未整改 |
| AF-056 | `docs/MAT_DATA_SCHEMA.md` | 独立复核 582 MAT 的文件身份/SHA-256、变量与 shape、L2 原始 double 逐值；FNV64 与 SHA-256 分开，不改写 origin/。 | 未整改 |
| AF-057 | `docs/TODO.md` | 在进入相应阶段时收齐论文 PDF、H-10/H-11 操作授权、DEC-005/006、正式基线/发布批准；已选 MIT/DEC-004/CLI 等不得重复申请。 | 未整改 |
| AF-058 | `docs/DOCUMENT_BASELINE_REGISTER.md` | 正式签署前更新文档版本/blob/commit/范围/批准人/日期时区/例外及测试证据；滚动 PROGRESS/AUDIT 不伪装为已冻结规范。 | 未整改 |
| AF-059 | `docs/design/README.md` | 逐模块核对 LLD 签名、唯一合同归属、独立 L3/TC 映射与真实代码边界，建立可编码/可验证的冻结清单；不以目录齐全等同实现。 | 已整改 |
| AF-060 | `docs/golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | 逐 AC 复核 L2 逐值、L3 一般/退化/逐节点、适用 L4 的缺口与实际证据；L1/MATLAB 永久不列为必需门槛。 | 未整改 |
| AF-061 | `docs/PLAN.md` | DEC-006 性能/资源目标基于未来真实全工程构建及运行 wall/CPU/RSS、平台事实与批准标准确定；VM 容量快照不能当性能验收。 | 未整改 |
| AF-062 | `docs/design/README.md` | §1 不再将 36 个选定文档的固定版本身份矩阵称作“全部当前规范和过程文档”；另引用 AUDIT 50 条审计路径，明确两者用途及非冻结边界。 | 已整改 |
| AF-063 | `docs/TEST_PLAN.md` | §3 五个 TC 行引用的 `docs/SOURCE_INVENTORY.md` 尚不存在；明确它是 P0.6 待创建交付物，并引用现存 SRS §9.2 / OBJECT_MODEL / MAT_DATA_SCHEMA 的相应可用证据，不伪称来源审计完毕。 | 已整改 |
| AF-064 | `docs/AUDIT.md` | 追踪计数明确为 58 正文 REQ + 8 REQ-DC = 66 总 REQ，TC 同理；不得把“66 + 8”重复计数。 | 已整改 |
| AF-065 | `docs/AUDIT.md` | 逐文件进度表移除已整改 AF-032～036、AF-062 的“待”标签，只保留实际尚未完成的 AF-037～061。 | 已整改 |
| AF-066 | `docs/AUDIT.md` | §4 登记阶段的“当时尚未整改”与 §7 当前已完成的六项结案分开标注，避免旧进度冒充最新状态。 | 已整改 |
| AF-067 | `docs/golden/analytic_golden_candidates.v1.json` | `source_blob_shas["bias_test/data_generation/bias_test_spherical_prop_error.m"]` 登记 39 位 SHA `fc3c4f0a7a417171415fb5e3d869a744ba24d04`，与固定来源提交真实 40 位 blob `fc3c4f0a7a417171415fb5e3d869a744ba24d04f` 不符；修正元数据并复核 19/19 来源，不修改 AN-G 数值或源 `.m`。 | 已整改 |
| AF-068 | `docs/AUDIT.md` | 末尾 §11/12 重复编号；统一章节顺序，并区分历史“退出”事实与本轮继续整改的最新执行原则。 | 已整改 |
| AF-069 | `docs/AUDIT_PROCESS.md` | §9→§11 跳号；规范剩余未整改项的继续处理、阶段证据与限制说明，不把未执行的授权/测试误关单。 | 已整改 |
| AF-070 | `docs/PROGRESS.md` | 增加本轮滚动证据记录：明确当前仍有 23 项未整改，按最新请求继续推进可执行的整改；将 H-10/H-11、论文/数字参考/审批的外部门槛保持可追踪。 | 已整改 |
| AF-071 | `docs/DOCUMENT_BASELINE_REGISTER.md` | §6.3 两次出现、后续 §6.4 顺序错位；调整为连续唯一的 6.1～6.5，保留原有固定文件身份/历史快照、审批和内容事实。 | 已整改 |
| AF-072 | `docs/MIGRATION_MATRIX.md` | 明确源 45 .m 对应 50 个函数定义；将 `SHRotate.m` 的内部 M/U/P/V/W 五函数与主入口逐个登记，并声明它们的递推/独立 L3/生产实现均未完成。 | 已整改 |
| AF-073 | `docs/AUDIT.md` | 去掉重复历史审计过程、旧退出语句和冗长 SHA 段落，保留 AU 主题、文件三态、有效规则、最新进度及历史 review 引用。 | 已整改 |
| AF-074 | `docs/AUDIT_PROCESS.md` | 压缩重复历轮结案，保留全部 AF 编号/相对路径/独立目标/两态及真正未完成证据，详细 SHA 仅由历史 reviews/PROGRESS 留存。 | 已整改 |

## 2. 本轮范围与未关闭条件

- **发现前**原有 72 项：49 已整改、23 未整改；新 AF-072～074 为函数级审计追踪及两份台账压缩，首次登记时状态为未整改。**本次登记时**合计 **75 项：49 已整改、26 未整改**（随后复核结案，见 §4）；待后续真实提交复核再转状态。
- 剩余 23 项为 AF-037～061 的阶段目标（不含已完成的 AF-052、AF-059），涉及正式 SRS/HLD/LLD 身份与接口、MAT 逐值 SHA-256、安全 Reader/Writer、KvD/Adaptive/采样/轨迹/球谐/分析的 L2/L3/L4、真实 C++/CTest、DEC-005/006/008、论文及人类签署。
- **局部进展不等于结案**：`MIGRATION_MATRIX.md` 已核 45 文件/50 函数声明；CLI/噪声候选与部分 AN-G 手算、SOURCE_INVENTORY/TEST_PLAN 静态对应已经补充。它们不解除缺失的独立数值与实施门槛。

## 3. 证据与更新流程

每轮先锁定分支提交并扫描 → 新 AF 先填「未整改」 → 修改原目标文档/实现/实测数据 → 在**新提交**复读并把证据记入 [PROGRESS.md](PROGRESS.md)/[版本登记](DOCUMENT_BASELINE_REGISTER.md) → 方可改「已整改」；旧的未解决项目保留 ID、缺口与状态。历史完整发现与核验可按固定提交从 [审计报告](reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md)、[AF-067 来源 SHA 纠正](reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md)及 [本轮函数声明报告](reviews/AUDIT_FUNCTION_DECLARATIONS_AND_LEDGER_COMPACT_2026-11-08.md) 追溯。此处不重复不断追加旧轮次日志和 hash 表。第二轮全面审计仅在第一轮全部适用目标实际完成后启动。


## 4. 定向复核后现行状态（2026-11-08）

- AF-072：`MIGRATION_MATRIX.md` 与 `SOURCE_INVENTORY.md` 已记录 45 .m 中的全部 **50 个 function 定义**，含 SHRotate 的 5 个内部函数；固定检查提交 `150b975`，45 条文件级记录、44×1＋1×6 和对应 6 个源行位置均核对。
- AF-073/074：`AUDIT.md` 从 15831→8021 字符，`AUDIT_PROCESS.md` 从 17516→9052 字符，固定提交 `ae831be3a0901d69c0897044fd339fda38468963` 复读：**28 AU、54 文件三态、75 个唯一 AF** 均保留，旧过程由 reviews/PROGRESS/Git 历史查证，不重复复制 hash。
- **现行问题级两态：75 项中 52 已整改、23 未整改**（AF-037～061 中除 AF-052、AF-059 外的阶段目标）。历史事实仍保留，未完成权限/论文/代码/数值/测试/正式批准不因台账压缩而减少。


**后续局部审计补记（仍未整改）**：AF-045/055 对 `AN-G08` 的 z90 特例作了 [R3/R5 系数方向静态推导](reviews/AN_G08_Z90_STATIC_CONVENTION_2026-11-08.md)，AF-056 对 582 MAT 的 Git 路径/字节和既有数据字典做了逐项静态匹配；三者均未取得一般 L3、实际 MAT 逐值/独立 SHA-256、代码测试或批准，故此表 **75 目标：52 已整改、23 未整改**不变。


## 5. 源码—分析 LLD 增量问题（2026-11-08，整改前）

| ID | 仓库根相对路径 | 原子整改目标 | 状态 |
|---|---|---|---|
| AF-075 | `docs/design/analysis_plotting.md` | 原 `many_parameter` **旋转误差**应每个 begin/end 分段单独调用 `kurveglaetten(segment,f)`（f=2/5/10），先处理 segment 后裁剪 begin+f…end−f；与对 600 行全局平滑后裁段不同。须补明确算法和既有 TC 的对照输入。 | 已整改 |
| AF-076 | `docs/design/analysis_plotting.md` | 原 comparison 平移误差图直接以完整曲线 x=50:550 绘制（不按 ratio 分段）；原 many_parameter 平移误差四曲线先对完整 600 行（后三条 f=2/5/10）全局平滑，再取 x=50:550。须与旋转路径分离且加独立 TC/Golden 断言。 | 已整改 |

**发现后当前两态**：77 项中 52 已整改、25 未整改；本节是先登记，不能仅凭源代码扫描就关闭 AF-075/076 或尚缺实际独立数值的 AF-050/051。


## 6. AF-075/076 修订与定向复核结案（2026-11-08）

- 发现登记提交：`8730c631`；分析 LLD/TEST_PLAN 修订 `98ff092`，小节顺序与静态复核 `d8e7e64`。**AF-075** 已区分局部旋转分段平滑、局部零边缘和显示裁剪；**AF-076** 已明确两种分析脚本的平移**不分段**，多更新率平移先全局平滑再绘制 50:550。测试映射仍是现有 `TC-F-AN-001`、`TC-I-ANALYSIS-001`、APL-G04/05。
- 原扫描报告中“相同裁剪后内部旋转均值必不同”的论断过强，**保留旧扫描作为历史事实**并在 [纠偏报告](reviews/AUDIT_ANALYSIS_SEGMENT_SMOOTHING_CORRECTION_2026-11-08.md) 明确反例：裁剪内部窗口时均值本应相等。后续必须检查完整段级临时数组、边界/短段及平移绘制域，而非编造差异。
- 两项**只在文档算法合同范围结案**；AF-050/051、实际独立 L3/CTest/正式签署均未关闭。当前共 **77 项：54 已整改、23 未整改**。


## 7. AF-077 分析错误跨模块映射缺口（首次登记）

| ID | 文件相对路径 | 整改目标 | 状态 |
|---|---|---|---|
| AF-077 | `docs/design/contracts.md` | `analysis_plotting.md` 已定义 AnalysisErrorCode 七个状态，但 contracts §3.1 统一 CLI 退出码尚未映射；补单一候选的错误来源/stage/exit，明确缺 MAT、坏 shape、无效分段、非有限与非法实验命令的差异，并在分析/CLI/TEST_PLAN 引用，保持 DEC-008 未批准与真实测试未完成。 | 已整改 |

**发现时**：78 个 AF 原子目标，54 已整改、24 未整改；此阶段只有静态发现，没有修改接口源文件或批准退出码。


## 8. AF-077 定向复核结案（2026-11-08）

- 发现登记 `20b5fcaa` 时「未整改」；统一候选错误来源 `contracts.md §3.1` 的固定修订提交 `e3c4c6794b3d2b562ebc39e92251eaaa52f44c34` 上完成 7 类 `AnalysisErrorCode` 的 `read(3)/decode(4)/analyze(5)`、非法 CLI 选项 `parse(2)` 与跨层原始错误保留的**静态定向检查**。分析/CLI LLD 只引用同一表，TEST_PLAN 仍是拟议负例，故仅 AF-077 的文档冲突关闭。
- **当前两态：78 个原子目标，55 已整改、23 未整改**。其中 AF-040/041/050/051 和 DEC-008 正式审批、真实 C++/JSONL/CTest 仍开放；不宣称任何新软件 PASS。


## 9. 新发现的索引漂移（初始状态）

| ID | 仓库根相对路径 | 整改目标 | 状态 |
|---|---|---|---|
| AF-078 | `docs/design/README.md` | 移除把 AUDIT 的“50 个路径”误作当前计数的旧文字，改链接动态审计表；跨模块模块矩阵说明 `AnalysisErrorCode` 在分析 LLD 定义、对外 exit/stage 由 contracts 单一候选映射，保持其他所有者/冻结状态不变。 | 已整改 |
| AF-079 | `docs/DOCUMENT_BASELINE_REGISTER.md` | §1 将 AUDIT 的 50 条当作现行审计范围，须明确**历史 36 项身份快照不变、实时审计对象数与三态只在 AUDIT 维护**；禁止覆盖历史固定 SHA/批准栏。 | 已整改 |

**登记时**：80 项整改目标（55 已整改、25 未整改）；两项目标尚未实际修改，不能提前关闭。


## 10. AF-080 轨迹反向边界静态审计（先登记）

| ID | 文件相对路径 | 整改目标 | 状态 |
|---|---|---|---|
| AF-080 | `docs/design/trajectory.md` | 将源 `drehvektor_aus_zwei_vektoren.m` 对非零严格反向 v1/v2 的“可能零向量”纠正为**由叉积0与 acos(-1)=π 确定返回零向量**，并给出可手算的三点局部 Rᵀ、正根近度案例及现有 TC/Golden 候选，不把源异常等同 C++ 有效旋转或测试 PASS。 | 已整改 |

登记时累计 81 项：55 已整改、26 未整改；本节只是发现记录，未修改目标 LLD 或产生独立 L3。


## 11. AF-078～080 修订结案（2026-11-08）

- AF-078/079：`02a2a17` 已把 LLD 目录与基线矩阵的实时审计数交给 AUDIT，保留 36 项固定 SHA 的历史语境，并将 `AnalysisErrorCode` 的定义与唯一候选退出映射分层；静态复读通过。
- AF-080：`07d3abb` 已将严格反向非零向量的原返回明确为 `cross=0, acos(-1)=π → rotationVector=0`；轨迹 LLD 与 TEST_PLAN 新增 TR-C01～04 的符号级输入（Rᵀ 局部变换、球内正根 near），**不标为已生成 L3/CTest**。
- **最新状态：81 项中 58 已整改、23 未整改**；仍需真实实现、独立数值/生产测试、论文材料及人类批准的原 AF-037～061 不变。第二轮正式全面审计尚未具备清零条件。


## 12. AF-047/055 局部静态数学进展（2026-11-08，仍未整改）

- `844b647` 补充 [AN-G01～03 源码初始面表符号推导](reviews/AN_G_LEVEL0_FACE_ORDER_STATIC_2026-11-08.md)：八面体 8、缺损 6、二十面体 20 个 `level=0` 行，与现有 JSON 的符号和顺序对齐（仅静态文档校验）。
- 不把这个结果当成完整 i=0/1/2 有序独立 L3、RNG 实际消耗、生产 C++/CTest 或 TC/AC PASS。**AF-047/055 仍未整改**；当前整改两态仍为 **81 项：58 已整改、23 未整改**。动态审计文件为 58 项（35 已审计、23 正在审计）。
