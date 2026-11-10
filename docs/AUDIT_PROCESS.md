# 低阶球谐三路线一致性专项整改台账（AUDIT_PROCESS）

> **本文件的现行整改目标仅限用户明确指定的六项球谐一致性任务**，详见 [AUDIT.md](AUDIT.md)。这是范围修订，不是数学审计或代码整改成果。本轮**只修改本文件与 AUDIT.md**，不运行符号验证、不实施 C++、不修改 MATLAB、也不开展新的逐公式审计。

## 1. 专项唯一整改目标（当前有效）

**唯一目的**：保证论文 *Insect-Inspired Self-Motion Estimation with Dense Flow Fields—An Adaptive Matched Filter Approach* 依赖的低阶实球谐计算公式和相关 MATLAB 代码，按三条独立路线 **A 直接定义、B Ivanic–Ruedenberg 实球谐递推、C Choi 等复球谐递推转实基** 严格核对一致。

| 专项ID | 路线/对象 | 唯一整改与验收目标 | 状态 |
|---|---|---|---|
| SH-01 | 直接定义 | 依据数学定义及 Condon–Shortley/实基转换，明确并核对 l=0,1,2 九个 Y_lm 的解析式、归一化、符号及顺序；独立于递推提供基准 | 未整改 |
| SH-02 | 实球谐递推 | 核对 Ivanic–Ruedenberg 1996 及 1998 勘误、l=0/1/2 实球谐旋转递推的全部 R0/R1/R2 元素与一般 SO(3) 作用，逐式与 SH-01 一致 | 未整改 |
| SH-03 | 复球谐递推 | 核对 Choi 等 1999 复球谐递推及复基→实基转换（含共轭、m 排列与相位），使 l=0/1/2 的全部旋转块与 SH-01、SH-02 一致 | 未整改 |
| SH-04 | MATLAB 九基实现 | 对照 origin/.../help_functions/kugelflfunc2.m 的 legendre、相位、归一化、m<0/m=0/m>0 与九个解析定义；说明是否确实等价或定位真实差异 | 未整改 |
| SH-05 | MATLAB 旋转实现 | 对照 origin/.../help_functions/SHRotate.m 的 degree=0/1/2、M/U/P/V/W、索引、矩阵方向、系数布局及相位映射；分别判定与 SH-02、SH-03 和直接基准的对应关系 | 未整改 |
| SH-06 | 三路线最终交叉结案 | 在已核对的九基与 R0(1×1)/R1(3×3)/R2(5×5) 上，用独立一般 SO(3) 数学/符号证据复核 A=B=C；汇总 MATLAB 逐函数结论、论文印刷或1998勘误差异、适用约定及不能证明的范围，形成唯一最终结案判定 | 未整改 |

**本轮只是重定义待办目标，未执行任何数学审计；六项暂均标记“未整改”，并不否认过去已存在的论文核对、Wolfram 精确证明或用户SH专项CTest证据。** 以后的核验必须复用既有证据，确认各项目标完整闭合才能改变状态，不得按新增文件数量计进度。六项之间有依赖：SH-06 只能在前五项明确裁定后完成。

### 1.1 严格排除范围（禁止再次加入本专项）

- **不审计** Franz、KvD、Adaptive MFA 运动矩阵/迭代、near 均值与正值性、离散重新拟合算子、球面几何采样/RNG/噪声、Session、MAT/CLI/Runner/实验绘图、全工程接口或性能。
- **不研究**任意高阶球谐或一般数值稳定性；仅关注论文实际使用的 `l=0,1,2` 九个实球谐函数及旋转 `R0/R1/R2`。
- **不新增**无关 AF、TC、Golden、数学反例、证明脚本或审计文件；任何下一项工作必须明确对应 SH-01～SH-06 的具体式子、MATLAB 代码行或直接三路线交叉证明。
- **禁止**通过开展外围数学研究来推进本专项计数；禁止为了数字变化人为拆分/关闭整改目标。
- MATLAB 源 `kugelflfunc2.m` 与 `SHRotate.m`（包括 M/U/P/V/W）只读；用户没有在本轮授权代码实现。禁止修改 C++、CMake、CTest、Bash、MAT 或 `origin/**`。

### 1.2 逐项闭合标准

- **SH-01**：固定九个 `Y_lm` 解析式、来源约定及完整 `m` 对照表，并明确常见 CS 相位差异。
- **SH-02**：以直接定义为基准，逐式列出 Ivanic 1996/1998 实递推以及 `R0/R1/R2` 的一般旋转一致性；把实际论文勘误和源公式差异区分开。
- **SH-03**：从复基递推到九实基转换的矩阵 `W`、共轭、相位及旋转作用作完整推导，与 SH-01/02 的块逐项一致。
- **SH-04**：对 `kugelflfunc2.m` 的 l/m、`legendre`、`theta/phi`、`sin/cos`、归一因子作逐项源对照，判定九项函数是否等价；不得用旋转测试代替它。
- **SH-05**：对 `SHRotate.m` 的 M/U/P/V/W 及 `degree<=2` 递推、(R) 与 (R^\top)、作用于系数或球面函数的方向作逐源码结论。
- **SH-06**：明确给出“完全一致”“仅在明确约定转换下等价”“真实公式/代码错误”或“证据不足”的逐项判定与来源；每项必须有确切定位，不能以单个数值特例或旧 3/3 CTest 代替一般符号证明。允许保留有依据的未证范围，不得声称所有浮点环境/异常输入形式化证明完成。

**许可的验证方法**：读取指定论文/原MATLAB、直接数学推导、最少量必要的 Wolfram 符号运算、复用现有低阶球谐专项数值证据。此处是后续可能工作的计划，**本轮没有实际使用上述方法执行具体核验**。

### 1.3 第二轮审计触发条件（仍只限本专项）

仅 SH-01～SH-06 六项全部完整结案后，才可对 AUDIT.md 第2节列出的**13个范围内文件**作一次**三路线专项复核**；绝不得自动回到全项目97个历史 AF、100个历史文件或实施跨模块“第二轮全面审计”。

## 2. 历史全工程整改目标存档（不参与本专项）

以下 **原97项 AF、74已整改、23未整改** 均原样保存自固定提交 [`c38b4b6`](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/c38b4b63307c5f3febf2bee0aac95b565f44a46c/docs/AUDIT_PROCESS.md) 的完整目标和状态；不删除历史、不宣称它们结案、不授予继续研究这些任务的权限。**下面仅供历史追溯，绝不进入 SH-01～SH-06 当前有效台账的统计或下一轮任务选择。**

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
| AF-017 | `docs/ARCHITECTURE.md` | HLD QA-01 改验固定根/RunId/软链/竞态 | 已整改 |
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
| AF-034 | `docs/design/experiments.md` | §5 五入口对照表统一 HistoricalOnly、当前 save 及缺字段失败语义。 | 已整改 |
| AF-035 | `docs/TEST_PLAN.md` | §5.1 with_errors 七份历史 MAT 中两份 alleadap* 显式标 HistoricalOnly；保留真实 shape 与 7/7 读取事实。 | 已整改 |
| AF-036 | `docs/DOCUMENT_BASELINE_REGISTER.md` | 解释 36 项固定身份登记与当时 AUDIT 50 对象的不同口径；新增对象不自动冻结。 | 已整改 |
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
| AF-056 | `docs/MAT_DATA_SCHEMA.md` | 独立复核 582 MAT 的文件身份/SHA-256、变量与 shape、L2 原始 double 逐值；FNV64 與 SHA-256 分开，不改写 origin/。 | 未整改 |
| AF-057 | `docs/TODO.md` | 在进入相应阶段时收齐论文 PDF、H-10/H-11 操作授权、DEC-005/006、正式基线/发布批准；已选 MIT/DEC-004/CLI 等不得重复申请。 | 未整改 |
| AF-058 | `docs/DOCUMENT_BASELINE_REGISTER.md` | 正式签署前更新文档版本/blob/commit/范围/批准人/日期时区/例外及测试证据；滚动 PROGRESS/AUDIT 不伪装为已冻结规范。 | 未整改 |
| AF-059 | `docs/design/README.md` | 逐模块核对 LLD 签名、唯一合同归属、独立 L3/TC 映射与真实代码边界，建立可编码/可验证的冻结清单；不以目录齐全等同实现。 | 已整改 |
| AF-060 | `docs/golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | 逐 AC 复核 L2 逐值、L3 一般/退化/逐节点、适用 L4 的缺口与实际证据；L1/MATLAB 永久不列为必需门槛。 | 未整改 |
| AF-061 | `docs/PLAN.md` | DEC-006 性能/资源目标基于未来真实全工程构建及运行 wall/CPU/RSS、平台事实与批准标准确定；VM 容量快照不能当性能验收。 | 未整改 |
| AF-062 | `docs/design/README.md` | 36 个固定身份不冒充全部现行审计范围；动态集合归 AUDIT，保持非冻结边界。 | 已整改 |
| AF-063 | `docs/TEST_PLAN.md` | SOURCE_INVENTORY 未建立时明确计划身份、引用实际证据，不伪称来源审计完成；后续实际清单身份可追踪。 | 已整改 |
| AF-064 | `docs/AUDIT.md` | 58 正文 REQ + 8 REQ-DC = 66，总 TC 同理，不重复计数。 | 已整改 |
| AF-065 | `docs/AUDIT.md` | 文件状态描述移除已经整改的旧“待”标签，保留真实开放目标。 | 已整改 |
| AF-066 | `docs/AUDIT.md` | 历史发现阶段与现行结案统计分开。 | 已整改 |
| AF-067 | `docs/golden/analytic_golden_candidates.v1.json` | 修复 bias_test_spherical_prop_error.m 的39位SHA为真实40位fc3c4f0a7a417171415fb5e3d869a744ba24d04f；19/19来源复核，不改候选数值。 | 已整改 |
| AF-068 | `docs/AUDIT.md` | 修复重复章节号，历史退出事实与现行继续整改规则分开。 | 已整改 |
| AF-069 | `docs/AUDIT_PROCESS.md` | 修复章节跳号，未执行授权/测试不关单。 | 已整改 |
| AF-070 | `docs/PROGRESS.md` | 记录继续整改和23个开放阶段门槛，保留授权/论文/数值/批准的追踪。 | 已整改 |
| AF-071 | `docs/DOCUMENT_BASELINE_REGISTER.md` | §6.1～6.5唯一顺序，保持历史固定文件身份及审批不变。 | 已整改 |
| AF-072 | `docs/MIGRATION_MATRIX.md` | 45 .m/50函数，SHRotate的主函数及M/U/P/V/W独立登记；非数学或软件验收。 | 已整改 |
| AF-073 | `docs/AUDIT.md` | 压缩重复历史，保留AU、完整文件三态、现行规则、历史证据。 | 已整改 |
| AF-074 | `docs/AUDIT_PROCESS.md` | 压缩重复结案但保留全部AF编号/路径/目标/两态，历史证据由固定Git/PROGRESS/reviews留存。 | 已整改 |
| AF-075 | `docs/design/analysis_plotting.md` | 旋转按段局部平滑并裁剪，保留完整段级边缘数组；纠正“裁剪后内部值必不同”的旧断言。 | 已整改 |
| AF-076 | `docs/design/analysis_plotting.md` | comparison平移不分段；many_parameter平移先全600行平滑，再取50:550；与旋转分开。 | 已整改 |
| AF-077 | `docs/design/contracts.md` | 七种AnalysisErrorCode的唯一候选exit/stage与CLI/TC对齐，DEC-008/真实测试不关闭。 | 已整改 |
| AF-078 | `docs/design/README.md` | 动态审计数归AUDIT；分析错误定义与公共退出映射分层。 | 已整改 |
| AF-079 | `docs/DOCUMENT_BASELINE_REGISTER.md` | 历史36项固定身份与现行审计集合分离，旧快照不覆盖。 | 已整改 |
| AF-080 | `docs/design/trajectory.md` | 严格反向非零向量的源结果确定为零rotvec，不能描述为“可能”；补TR-C01～04候选，不当有效π旋转。 | 已整改 |
| AF-081 | `docs/design/sampling_noise.md` | A/B/C/D子面顶点、深度优先四进制行号、缺损原面ID与连续输出块区分；level1/2索引资格，非全量L3。 | 已整改 |
| AF-082 | `docs/AUDIT.md` | 现行描述移除过期54文件/75AF计数；历史数字保持原时点。 | 已整改 |
| AF-083 | `docs/design/analysis_plotting.md` | Bias xwerte只取最后k，四组y累计除K；绘图3:end、100/sqrt(xwerte)，不把x平均或换实际N。 | 已整改 |
| AF-084 | `docs/design/contracts.md` | PlotErrorCode四成员唯一exit/stage、CLI/分析/TC和输出安全对齐；不代表绘图/CTest/批准。 | 已整改 |
| AF-085 | `docs/AUDIT.md` | 现行摘要与150b975历史58/35/23统计区分，实际唯一行是计数源。 | 已整改 |
| AF-086 | `docs/design/README.md` | 区分10条模块/合同行与11个实际Markdown文件（含球谐专题）。 | 已整改 |
| AF-087 | `docs/design/kvd.md` | 补Franz、near分母/翻号、归一前后t、旋转a/b/c、预测流、停止布尔和末轮缩放的候选trace，并映射原TC；不改变源数学。 | 已整改 |
| AF-088 | `docs/design/experiments.md` | akrot为真实相对rotvec模长换算deg、非误差或角速度；首末零、当前save及0/90/180度候选。 | 已整改 |
| AF-089 | `docs/design/experiments.md` | ratio虽在加噪后赋值，分子分母取加噪前理想分量平均模长；零分母/非有限候选与末次缓冲规则，禁止补造历史字段。 | 已整改 |
| AF-090 | `docs/design/analysis_plotting.md` | 球谐插图P_l^m*cos(mφ)、41×41网格及rho=5+3yy/max(abs(yy))不等同核心九实基；既有TC-F-AN-002候选。 | 已整改 |
| AF-091 | `docs/design/analysis_plotting.md` | 两轨迹插图600点、sphere(50)、y<=0的z-only NaN遮罩；正弦源的circular标题错误与可选纠正分开。 | 已整改 |
| AF-092 | `docs/TODO.md`；`docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 登记用户已提供1996原文、1998勘误、1999论文的页码/文件身份及审阅范围；撤销这些现行文件的“等待上传”卡点，不擅自开放H-10/H-11。 | 已整改 |
| AF-093 | `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 建立1996→1998公式6.7/7.9a/b/c及Table1/2→原M/U/P/V/W的对应；显式记录1998 Table2的sqrt(1−δ)与源码sqrt(1+δ)冲突、边界P索引及零权重不求值规则，给可复核反例，不能默改论文。 | 已整改 |
| AF-094 | `docs/design/harmonics.md` | 明确1999式8.15与原SHRotate一阶块的转置、系数左乘及现有九实基相位；加入绕x/y及非交换旋转的区分性检查，不把z90特例当一般物理方向证明，不自动修改原算法。 | 已整改 |
| AF-095 | `docs/golden/SH_PAPER_VERIFICATION_CASES.md` | 从1996 Table3重排全25元素的独立非递归参考候选，结合1999 F/G与W变换构造交叉验证路线；关联既有TC-F-SH-001/002，文献高阶速度/精度不冒充本项目实测。 | 已整改 |
| AF-096 | `docs/TEST_PLAN.md`; `docs/PROGRESS.md`; `docs/AUDIT.md`; `docs/design/README.md`; `docs/design/harmonics.md`; `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md`; `docs/TODO.md` | 根据用户提供受测 HEAD 8eab5dd 在 Ubuntu/GCC16/CMake4.3.4/Eigen3.4.0 下球谐原生编译、CTest 1/1 Passed 的真实输出，清除当前测试计划、LLD、TODO 中“球谐从未实现/编译/测试”的过时表述；记录日志/退出码证据边界，根CMake/build纳入审计，严格区分旧测试通过与后续新增回归未运行，不降低原AF-045/046/051的完整验收。 | 已整改 |

## 3. 本轮状态、口径转换与禁止误报

| 指标 | 修订前的全工程历史口径 | 修订后的球谐专项现行口径 |
|---|---:|---:|
| 整改目标总数 | 97 | 6 |
| 已整改 | 74 | 0（本轮未实际审计） |
| 未整改 | 23 | 6（待逐项目标结案） |
| 审计文件总数 | 100 | 13 |
| 已审计文件 | 68 | 9（继承历史限定状态） |
| 正在审计文件 | 32 | 2 |
| 未审计文件 | 0 | 2 |

**这是一项审计口径和范围替换，绝不是对原工程整改成果的减少、撤销或完成率变化。** 六项未整改表示新的专项结案任务尚未执行，不代表之前全部证明无效。旧 AF 的状态保持原样，但不再阻塞本专项；新增的两个 MATLAB 审计文件标记空白是下一轮才核查，而不是判定错误。

本轮完成标准仅为：两份治理文件的范围与任务定义一致、原历史完整可追溯、仅修改这两个文件。**本轮不出具 A/B/C 是否一致的最终结论，不开展符号计算，也不新增生产代码。**
