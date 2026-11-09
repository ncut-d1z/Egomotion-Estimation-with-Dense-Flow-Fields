# 审计整改进度（AUDIT_PROCESS）

> **问题两态**仅未整改/已整改。每个AF必须按表内完整义务验收；数学子问题完成不能替代软件、授权或批准。文件三态以 [AUDIT](AUDIT.md) 的唯一路径表维护。
>
> **当前用户约束**：只做理论推导，允许符号计算，禁止C++代码实现。本轮不修改C++、头文件、CMake、CTest、Bash、MAT、原始`origin/**`，也不运行生产构建或测试。既有SH原生3/3来自用户固定5e522198的历史运行；不能据此扩展本轮权限。MATLAB/Octave/L1仍永久非必需。
>
> 原§2～15详细发现、纠偏、Git blob、SHA-256与结案证据完整保存在[9825375不可变台账](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/982537556be1ac5a414af368a0082806a9179d32/docs/AUDIT_PROCESS.md)及PROGRESS/reviews；E-DOC-068的上轮状态亦见[21ad0c3固定台账](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/21ad0c34f3c43fd1bbf33c847406dfd302e239ea/docs/AUDIT_PROCESS.md)。全部AF编号/路径/目标/状态保留，不改写Git历史或旧审批。本轮E-DOC-069新增记录见§5、计数见§6。

## 1. 现行唯一问题台账

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

## 2. 历史固定证据与压缩边界

本版未新增或删除AF，没有改变任何AF状态或表内完整验收目标。原详细§2～15，包括SH论文/三路线证明、用户SH原生实测、714行核验、Adaptive单步及KvD36轮数学trace，均在[9825375完整台账](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/982537556be1ac5a414af368a0082806a9179d32/docs/AUDIT_PROCESS.md)保留；[同一提交的PROGRESS](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/982537556be1ac5a414af368a0082806a9179d32/docs/PROGRESS.md)保留E-DOC-054～067。不可把当时NOT_RUN误当当前，也不可改写历史版本成PASS。

## 3. AF-044/051/054/055/060 前轮理论子义务（E-DOC-068，历史）

### 3.1 当时待补问题

前轮从固定9825375继续第一轮，数学问题是：s=0的原CS输入、s趋零的尺度与抵消区别、均值有效但几何退化、M良态但raw平移零时的定义域。未另造易结案AF。

### 3.2 已取得的限定证据

[均值与条件数专题](design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md)、[Wolfram脚本](../verification/adaptive/verify_zero_mean_conditioning_families.wl)、[数学结果](../verification/adaptive/zero_mean_conditioning_evidence.v1.json)记录25/25检查及6组70位SVD样例。源oneit blob为`48e51bcd52fb8f27cf35b3fce38b28507af36e55`，脚本完成版blob为`36eedaf6477d0255b7ab0f12572506970da6847c`。

- s非零时mu=raw_r/abs(s)，均值sign(s)；将已经翻号的r_flip改除abs(s)才会改变负支。
- s=0的非全零与全零原CS输入都使源初始均值除法无定义；后置1e-11不保护此前除法。
- 正near同比缩小时M可保持条件数1，但raw平移变小、方向对绝对扰动敏感。
- 有符号抵消例的det(M)与全部特征值恒定，条件数却发散；不扩张为正深度场景。
- 正深度四方向窄锥在s=1时仍有kappa2=h^-4。
- 纯旋转在M良态时给raw平移零，后置方向归一无定义；旋转可估与无平移方向不能混淆。

这些数学子证据不代替Session、真实软件负例、正式容差或完整AC。其完整历史叙述及源数据见页首固定21ad0c3链接，参数族与证明没有被本轮更改。

## 4. E-DOC-068前轮起止计数（历史）

| 指标 | 开始 | 结束 | 净变化 |
|---|---:|---:|---:|
| 整改目标总数 | 97 | 97 | 0 |
| 已整改 | 74 | 74 | 0 |
| 未整改 | 23 | 23 | 0 |
| 审计文件总数 | 94 | 97 | +3 |
| 已审计文件 | 62 | 65 | +3 |
| 正在审计文件 | 32 | 32 | 0 |
| 未审计文件 | 0 | 0 | 0 |

## 5. AF-044/045/047/051/055/060 离散SH重新拟合的理论整改（E-DOC-069，2026-11-10）

### 5.1 本轮待核子问题与来源

从固定 `21ad0c34f3c43fd1bbf33c847406dfd302e239ea` 的97AF/97文件继续第一轮。源 `koeff_aus_near.m` 是每系数独立归一，`near_aus_koeff.m` 是基函数线性重建。本轮核对二者能否在任意有限采样上互为逆、是否保持正近度，以及最后系数拟合的零分母能否被前置运动矩阵检查覆盖。以上归既有AF，不改变源算法，不创建生产代码或新TC。

源SH三文件blob：koeff `5ff91a2210a20e5acf79dec6e59ddd33b55529d3`、near `7373c4376dfd5bbf281136ced39657d411a10ffa`、kugelflfunc2 `edc0caa1f88bf91d2a189e4d3a283334f809b905`。新增[理论专题](design/SH_DISCRETE_REFIT_CONSISTENCY.md)、[Wolfram复算脚本](../verification/harmonics/verify_discrete_refit_roundtrip.wl)、[实际结果JSON](../verification/harmonics/discrete_refit_roundtrip_evidence.v1.json)。脚本完成版blob `41a8d9c034d739702a7e867de1f52b1c1865c38f`，UTF-8 SHA-256 `6a212ec37201c741e33550b47a11d28b8381269df6a97d16745b7d69c6641c9e`；已从固定提交重新读取实际执行，35/35检查通过。

### 5.2 已完成的理论子结果

- **精确逆的适用条件**：B为N×9原实基、G=BᵀB、D=diag(G)，在每个Djj正的域内，源c_hat=D⁻¹Bᵀr。系数往返T=D⁻¹G，对所有c精确还原当且仅当G对角；样本算子P=BD⁻¹Bᵀ幂等也当且仅当G对角。一般证明写在专题，有限实例辅助符号核验，不把连续球面积分正交性当成离散正交。
- **正向对照**：24个不同带符号排列(1,2,2)/3使Gram对角、rank9、全部分母正，源拟合可以精确往返，避免把局部反例泛化为“永远错误”。
- **正近度精确反例**：在该24行后加12个v=(1,2,2)/3，全部36个near均1、rank9、全部分母正，重新拟合后在原样本d*=(2,-2,-1)/3重建为**−9/140**，全部样本均值变为383/210。原始近度是全正的，非前轮有符号负输入反例。
- **互异方向交叉核查**：将额外12行改为有理立体投影psi(1/5+j/10^6,2/5)，得到36条互异精确单位方向且保留满秩基集。在d*处所得精确有理数仍负，位于(−65/1000,−64/1000)，近似−0.06427848307778179；精确分子2853位负、分母2855位正。不是舍入、重复行或零分母造成。
- **末端定义域独立于运动条件**：八立方体角方向的M=(2/3)I6良态，但Y20/Y22全样本为零、最后两个系数比值为数学0/0。四方向窄锥的Y2,-2恒零，h=1/4时运动M仍可逆。前置均值/运动检查不能替代最后各球谐通道分母检查。
- **不夸大谱结论**：G非对角时孤立T存在放大模态，但完整Adaptive还包含其他更新与归一，因此未证明完整迭代发散。

### 5.3 完成范围与卡点

本轮整改的是数学使用前提、独立参考与错误断言边界。**正确九基/IR/Choi旋转及历史3/3来源忠实性CTest不被推翻**；亦未据此认定当前C++有实现错误。未实现最小二乘替换、裁剪、正则、额外权重或新容差。理论零分母在浮点中可变为极小非零，未取得原MATLAB/C++运行时行为。

AF-044仍含完整C_new/Session/状态提交，AF-045含完整声明/来源/数学及软件资格，AF-047含采样/RNG实际验证，AF-051含真实TC日志，AF-055/060含全部独立参考/AC和正式批准。没有这些生产/批准证据，因此原23个未整改目标全部保留。无需重跑旧SH构建，源/构建/测试文件没有修改。

## 6. 本轮E-DOC-069起止计数与阶段

| 指标 | 本轮开始 | 本轮结束 | 净变化 |
|---|---:|---:|---:|
| 整改目标总数 | 97 | 97 | 0 |
| 已整改 | 74 | 74 | 0 |
| 未整改 | 23 | 23 | 0 |
| 审计文件总数 | 97 | 100 | +3 |
| 已审计文件 | 65 | 68 | +3 |
| 正在审计文件 | 32 | 32 | 0 |
| 未审计文件 | 0 | 0 | 0 |

三份新增文件只按其限定数学范围审核通过；旧97条AF完整行以及原97个文件路径/状态保留。第一轮未清零，未启动条件性第二轮全面审计。最终计数由本文件唯一AF和AUDIT唯一路径表复读，不以叙述条数替代。
