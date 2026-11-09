# 审计整改进度（AUDIT_PROCESS）

> **问题两态**：仅「未整改 / 已整改」。实际修改目标并在新固定提交复读后才可结案；软件、数学验证、权限及批准目标不能以文字更新代替。文件三态见[AUDIT](AUDIT.md)。`origin/**`只读，MATLAB/L1永久非必需。**当前权限增量：用户已明确开放球谐相关C++和Wolfram符号验证，其他模块代码未开放；用户已在`8eab5dd`、`445ed151`和`601c74de`真实执行球谐原生CTest各1/1 Passed；代理自行运行全工程仍未授权。**
>
> 历史完整发现与结案保留于Git及PROGRESS/reviews。前轮固定版本为`4fe7be6cc50a2c2b7dec1555edf9859c0497870b`，更早分散§2～28记录位于`98e95e2f867dc7142fa0d5ac0f1c79f5d6429ed1`。旧“缺论文”“全部代码暂缓”不能继续作为当前球谐专项的阻塞。

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

## 2. 前轮登记与结案证据（历史固定快照，2026-11-09）

前轮输入`98e95e2f867dc7142fa0d5ac0f1c79f5d6429ed1`：92项＝69已整改＋23未整改。在`d0618b8f02ba35ff47ae8355eca8f319cde8b7ed`登记AF-092～095为未整改，实际修订并在`f2c3aca4205035d670df625d22d4b48935cb4a2f`复读后关闭四个文档级目标。

| AF | 实际修订提交 | 固定复读确认的限定结果 |
|---|---|---|
| 092 | 论文记录`c30078b`、专题`138f45b`、TODO`04a57d4`、索引`f2c3aca` | 材料及哈希已核；当时PAPER-SH取得不等于H-10/H-11开放 |
| 093 | `c30078b`、`138f45b` | 勘误/P边界、负m因子冲突、x45纸面反例及零系数短路记录完成 |
| 094 | `ad34ed0`、`138f45b` | 转置、相位及源行为与指定物理场作用分层完成，非当时的全一般数值证明 |
| 095 | `e37f392` | 全25显式公式、10候选、W共轭及文献性能边界的设计完成 |

前轮结案后96项＝73已整改＋23未整改。其完整内容可由固定提交`4fe7be6`读取，不把当时的未执行状态反写成通过。

## 3. 本轮原有AF-045/046/051/055/057的实质进展

本轮输入`4fe7be6cc50a2c2b7dec1555edf9859c0497870b`；未新造“文档问题”以增加关闭数，而是推进已有复合目标。专项授权先提交`ae8c094`；代码/脚本检查点`7eb04ede3d6f381caad5be2f80bba88b16d21b3c`，结果JSON`0985914`，数学/实现报告`9a508c6`，当前TODO`e34eee9`、harmonics LLD`2be34a1`。

| 原有目标 | 本轮已完成的子工作 | 整项仍开放的原因 |
|---|---|---|
| AF-045 | 2015式59–67九基定义差9/9为0，递推值差9/9为0，Gram81项=I9；一般四元数覆盖全SO(3)，IR与Choi各自一阶9/9、二阶25/25对直接定义差为0；场作用、相位/布局转换、勘误加号均已精确验证 | double原生实现/边界结果尚未执行，整体工程验收未完成；不能把精确代数当编译器或浮点证明 |
| AF-046 | 仅球谐叶子C++已实现：低阶基值、三旋转路线、重建、布局转换；纯Eigen，无sdla，局部API与未来公共类身份明确区分 | 公共contracts/domain/旧规划三类适配及原生ABI/CTest、集成未完成；不擅自实现其他模块 |
| AF-051 | 已创建独立球谐测试源码，含30旋转/10方向/9基/3路线、独立25值参考、错误分支；三份Wolfram脚本已实际执行，均allExact=true | 原生C++测试未执行，更广66TC义务不因专项通过而完成 |
| AF-055 | 新的独立精确有理旋转25项参考已由Wolfram直接张量路径生成、保存；明确不是递推自算Expected | 既有AN-G01～12 JSON未改变，其他范围仍待完整验证 |
| AF-057 | 球谐代码与Wolfram权限已提供；材料和一般数学核查不再是这部分卡点 | 原生执行、其他模块授权、最终DEC/基线/发布仍未提供 |

### 3.1 纳入AF-045的新增来源核对问题：2015附录第30页

发现：2015论文第30页最后d1²、d2²展开中的h4符号与式66/72的定义相反。整改目标是在球谐定义/证明/实现中不复制该错误，保留原文及明确更正依据，**不打开Adaptive等其他模块代码**。实际Wolfram证明印刷xx等于y²、印刷yy等于x²；正确xx用正h4、yy用负h4。结果和报告已记录。九基式59–67本身正确。

本问题作为既有AF-045的数学来源子项记录，不改写原PDF、不声称作者正式公布勘误，也不把上层耦合矩阵软件验收关闭。

### 3.2 实际证据及范围

[三路报告](reviews/LOW_ORDER_SH_THREE_ROUTE_VERIFICATION_2026-11-09.md)、[结果JSON](../verification/harmonics/verification_results_2026-11-09.json)及三份`.wl`保存一般符号域、内核版本和结果。最终Module局部脚本复算无消息且allExact=true；早期探索性调用的全局符号警告及连接器/页面回退限制亦有说明。代码逐式审阅不是C++实际执行。

**本轮问题两态仍为96项＝73已整改＋23未整改，净变化0/0/0。** 新增6份代码/测试与6份授权/证明/结果文档纳入文件级审计；其数量见AUDIT。原23项复合目标未降低要求，不以数学子项完成伪造整项结案。

## 4. 后续规则和卡点

先登记目标/子目标 → 修改 → 固定提交复读或实际验证 → 留证 → 满足完整义务才结案。当前球谐下一门槛是**仅该模块的原生编译/浮点/异常测试与公共接口集成边界**；不能再写“缺论文”“未获球谐实现许可”“一般旋转完全未经验证”。其他模块没有获得代码实现权限，原23项广义验收继续开放。第一轮未清零，不启动条件性第二轮全面审计；不合并发布，不修改origin。


## 5. AF-096 固定远端复读结案与首次原生测试范围（2026-11-09）

- **固定起点**：`8eab5dd7971f9ba9263528d62e9dccdeb926d638`（此前 96 个唯一AF：73 已整改、23 未整改；74 文件：45 已审计、29 在审）。用户提供 `./build.sh` Ubuntu 终端原始显示：GCC 16.2.0、CMake 4.3.4、Eigen 3.4.0、Release/C++17；球谐静态库、测试可执行文件全部生成；单个CTest `low_order_harmonics` **1/1 Passed、0 failed、总墙钟 0.11s**。这是**用户的执行证据**，代理未独立获取用户机器的日志字节/SHA或另外的 shell exit 数字；不推断未报告的运行。
- **先登记后修订**：AF-096 首次登记提交 `ee01e4846a7b6e0954c8aa603cb243d3104d068b`，状态未整改。限定球谐测试源码增补提交 `ba78d407e9868aabb3cfbf16f0e379bbf8bac77d`，新增独立直接基锚点、2015附录dyadic的h4正确符号回归、MATLAB CS相位源重建；**新测试源码提交在旧受测HEAD之后，仍未实际运行**。测试计划现行正文修订 `3461568c735093d7c1695b3cfbfef6423640d013`，前言复核修订 `0cee8b13ca83e4ce7a6bb6c13afe3f2dea952deb`；实际证据归 `PROGRESS E-DOC-056`：`6662de252049f3b28f71e30874d8aee171af1891`。
- **跨文档一致性**：`AUDIT` 已加入根CMake/build两条限定静态审阅路径，并修正各SH代码行的旧“未编译”说法（`f3bf071`、`186ecd8`）；`design/README` `fb0c99b`、`design/harmonics` `8a16242`、球谐专题 `bfcd701`、`TODO` `e26181c` 都明确低阶叶子实测与尚未实现的公共接口/其他模块的界限。
- **新固定提交的只读定向复核**：对 `e26181c1828fb3545fc9d9e7df8800aa0e195a57` 读取 TEST_PLAN/PROGRESS/AUDIT/AUDIT_PROCESS/TODO/LLD/测试源码，**13/13 静态事实、计数、关联与未执行标注检查通过**。这不是新版C++编译；不能把新增解析锚点说成已经通过。整改台账现为 **97 个唯一AF：74 已整改、23 未整改**；审计路径 **76：47 已审计、29 正在审计、0 空白**。
- **真正仍开放**：AF-045/046/051 的全业务接口/必要边界/新增CTest重跑、正式 DEC-005/008 及一般模块软件资格未完成；原23项不改变状态。原 `origin/**`、MAT及非球谐业务 C++ 未修改，代理未在用户机器或现场执行CMake/CTest。第一轮未清零，因此不将当前工作误标为第二轮全面审计。

## 6. AF-045/046/051 原有复合目标的本轮增量（2026-11-09，**不结案**）

- **从首次成功到增强成功**：项目所有者先在 `8eab5dd` 执行低阶球谐 1/1 CTest Passed，再在 `445ed151e5fc1faaad34b920f86363e29fb0a484` 通过 `git pl && ./build.sh` 完成增强解析/dyadic/MATLAB相位回归的实际 GCC16 C++ 编译和 CTest **1/1 Passed，0 failed**（CTest 总墙钟 0.07s）。该实际新证据归 `PROGRESS E-DOC-057`，未获取用户 VM 日志文件字节及独立 exit 数值；所以不虚称代理自己执行。
- **实际修正既有 TC 身份**：在 `docs/TEST_PLAN.md` `ebb352a2e707909fdb6b3ee88fbec30ea0dd97f2`，原 `TC-F-SH-001/002` 过去指向未创建的 `tests/unit/test_harmonics.cpp`；现映射到已存在、已在 `445ed151` 实测的 `tests/unit/test_low_order_harmonics.cpp` 和唯一已注册 `low_order_harmonics`。这只修正实际测试路径和局部观察，不增加 TC、重命名未知业务依赖或虚报完整 AC/DEC-005 PASS。TEST_PLAN 同时单列首次 `8eab5dd`、增强 `445ed151` 的用户运行历史。
- **之后又新增但未测试的球谐压力/负例**：`tests/unit/test_low_order_harmonics.cpp` 持续修改 `974889b0bd4f6c2d45e3369228817c7fcbdef380`、`9d4c71abd74d233e4eb9626292e67a210ee8c030`、`8d40f1a3ce680475507ab86f14201f14155cc647`：固定非零整数四元数 `5^4-1=624` 组、三种旋转方法/正交/场作用、边界 ErrorCode、有限极大系数重建溢出，以及矩阵与标量残差诊断。测试设计与 **NOT_RUN** 状态归 `TEST_PLAN`，不得把 `445ed151` 成功继承给新 HEAD。
- **开放原因不变**：AF-045/046 仍缺最终数值误差/非普通输入覆盖、公共API/ABI及集成/批准；AF-051 的其它业务 TC/Golden/故障负例仍未开发与验收；AF-055 其余数值参考身份不受专项成功影响。所有其它业务模块代码实现权限仍关闭，原 `origin/**` 只读。本次不结案上述广义目标，不实施以全部未整改项清零为条件的第二轮全面审计。
- **本轮动态台账**：唯一 97 AF：**74 已整改、23 未整改**；76 审计文件：**47 已审计、29 正在审计、0 空白**。本段是已登记 AF 的**进展证据**，不是额外新增/关闭的小任务。

## 7. AF-045/046/051/053/057 第三次原生压力验证与 build.sh 日志整改（2026-11-09，仍不关闭复合目标）

- **真实证据与归口**：用户给出 `601c74de315fdeb7b913cecd2eceeadee56fb6b2` 的 Ubuntu24.04、GCC16.2.0、CMake4.3.4、Eigen3.4.0、Release/C++17 构建日志；`./build.sh` 的球谐CTest 1/1 Passed，后续显式 `ctest -V` 同为1/1 Passed。程序真实打印 **53,532 checks、624 非零四元数、最大矩阵/向量残差8.8817841970012523e-16、最大标量残差1.0658141036401503e-14，代码回归阈值9.0949470177292824e-13**。原始 VM 日志位置记录于 `PROGRESS E-DOC-059`，代理没有取得日志字节/用户独立 shell exit 数字。
- **旧测试并未被覆盖**：新增 `testAnalyticBasisAndDyadic`、`testMatlabSourcePhase`、`testDeterministicQuaternionGrid`、`testFailureTaxonomyAndFiniteOverflow` 均作为新的函数/主程序调用追加至 `tests/unit/test_low_order_harmonics.cpp`，原有 basis/action、独立参考、composition/layout 和 failure 测试仍存在。CMake始终只有一个 `low_order_harmonics` CTest 条目；**1/1是一个测试可执行程序通过，53,532是其require检查次数，不能说有53,532个独立CTest。**
- **补齐被遗漏的构建日志能力**：原 `build.sh` 仅用 `--output-on-failure`，通过时隐藏PASS诊断，迫使用户额外重复运行 `ctest -V`。后续提交 `2e059fb3717619a70e1433fe65e21f153a1567fc` 已把脚本改为**只执行一次 `ctest --verbose --no-tests=error`**，保留原 `pipefail`/日志/严格失败；**此脚本版本尚未在用户环境实际运行，不可把 `601c74de` 的通过作为新脚本通过**。
- **数学及统计边界**：`q` 与 `-q` 表示同一旋转，且整数分量可有其它等比例重复；624代表非零四元数样本数，不能称624个不同SO(3)旋转。两类最大绝对残差低于现有4096eps专项回归阈值，**不能代表DEC-005已批准或全部IEEE double异常/可移植性已经形式化验证**。
- **当前台账不虚关**：原AF-045/046/051等仍需公共API/ABI、完整异常覆盖、独立参考范围及最终数值批准；AF-053/057仍缺其它模块工程/授权/性能/正式门槛。97个唯一AF＝**74已整改、23未整改**；76个审计文件＝**47已审计、29正在审计**。用户执行的是球谐叶子，非球谐生产代码与`origin/**`仍没有开放写权限，第一轮全清零前不启动条件性的第二轮全面审计。

## 8. AF-045/051/055 继续整改：AN-G06/07/08 独立候选接入（2026-11-09，完整目标仍开放）

- **实际变更范围**：仅新增 `tests/unit/test_low_order_archived_golden.cpp`（球谐专项，独立于旧回归），注册 `low_order_harmonics_archived_golden` 第二个CTest，更新 `CMakeLists.txt` 与 `build.sh` 的Scope显示；存储可重放 `verification/harmonics/verify_archived_sh_candidates.wl`，并更新 TEST_PLAN、harmonics LLD、PROGRESS、AUDIT。所有 `origin/**`、MAT 数据、已通过的 `tests/unit/test_low_order_harmonics.cpp`、球谐生产三路实现及非球谐生产代码均**未修改**。
- **固定来源/静态字面复核**：候选 JSON blob `1e6499eff3ce5b55503f8cd913b9f492629da499`；AN-G03的20行（60数）、AN-G06九基、AN-G08系数15值及R3九项/R5二十五项在新增C++文件与原JSON中逐字面一致，全部 `max_abs_difference=0`；20方向 `max_abs_squared_norm_error≈3.3306690738754696e-16`。AN-G07采用候选的`3.5449077018110318`原字面系数，不以被测结果反填。该核查**不代表原采样器/正式MAT数据已复现**。
- **实际独立数学核对**：Wolfram 15.0.1 已执行原MATLAB含CS相位低阶实基的代数检查，`AN_G06_9Residuals={0,…,0}`、`AN_G07_constantOne=1`、`AN_G08_R3_3Residuals={0,0,0}`、`AN_G08_R5_5Residuals={0,…,0}`、`AN_G08_9CoefficientsFieldResidual=0`，`allExact=True`；输入源 `verification/harmonics/verify_archived_sh_candidates.wl` 已保存。Wolfram符号正确性 ≠ 新C++编译/CTest/浮点验证。
- **是否可结案**：**不能关闭**AF-045/051/055，因为这是限定低阶叶子的数学子证据与候选测试，实现公共API、全部既有Golden、各类数值/异常完整覆盖、正式DEC-005/008、全部66TC及原业务阶段测试仍缺。本批新增的第二个CTest处于 `NOT_RUN`，绝不可把用户在 `601c74de` 上的旧单项 `1/1 Passed` 当新版本 `2/2 Passed`。未获更广H-10/H-11授权的MAT/CLI/KvD/Adaptive等代码保持未实施。
- **问题两态与文件三态**：97个唯一 AF＝**74已整改、23未整改**（未新增人为小目标）；文件三态本轮从 **76＝47已审计＋29正在审计＋0空白** 增加为 **78＝48已审计＋30正在审计＋0空白**：一个新C++测试在审，一个已完成限定符号审查的Wolfram输入文件已审。第一轮复合项目尚未清零，因此第二轮正式全面审计/整改**未启动**。

## 9. AF-045/051/055 原始SH二阶逐系数拟合缺口（2026-11-09，增量推进、整项目仍未结案）

- **发现与允许范围**：原 `koeff_aus_near.m` 中 \(a_{lm}=\frac{(4\pi/N)\sum_i\mu_iY_{lm}(d_i)}{(4\pi/N)\sum_iY_{lm}(d_i)^2}\) 是每项分别求标量归一分母，**不是联合9元最小二乘**。现有SH叶子此前只实现九基、三种旋转、单点近度重建与CS布局转换；尚无源二阶逐系数投影。依用户已开放的**仅SH模块**实现权限补 `src/harmonics/low_order_source_projection.cpp`、声明在 `include/egomotion/harmonics/low_order.hpp`，限固定 `ordnung=2` 和源3×5九有效项，不开发其它模块、不擅建跨模块 `HarmonicFitter`。
- **数学验证实际执行**：已运行 `verification/harmonics/verify_source_projection_order2.wl` 的 Wolfram 15.0.1 精确运算：7个非对称单位方向+允许负值的固定近度，九项原CS投影与独立解析参考差均0，九个基范数分母均正，Gram非对角项 `G[2,3]≈-0.2206948544207615` 非零（能区分联合LS）；`allExact=True`。新数学参考 JSON `verification/harmonics/source_projection_reference.v1.json` 含输入、准确根式、近似17位双精度、生成脚本及**UNAPPROVED、CPP_NOT_RUN**身份。独立JS double算术模型从固定JSON与代码的9通道映射复核，最大绝对差 `2.6645352591003757e-15`；**不是C++运行**。
- **代码/测试现状**：`tests/unit/test_low_order_source_projection.cpp` 使用上述独立九系数，并新增 shape、非有限、单位向量、零平方基范数、极大有限近度溢出和多余流列隔离测试；根 `CMakeLists.txt` 新增第三个SH-only `low_order_harmonics_source_projection` 测试。旧原回归、AN-G06/07/08第二个CTest不覆盖、不删除。实际三项 CTest 的新组合与新增生产CPP **尚未获用户Ubuntu原生编译和运行结果，全部作为新 HEAD NOT_RUN**；此前单项 `601c74de` 的1/1 PASS只能作为历史旧SHA证据。
- **错误处理差异明确保留**：空输入/非一致shape返回InvalidSampleShape，零基范数返回ZeroProjectionNorm，非有限和运算溢出分别NonFiniteInput/NonFiniteResult；原MATLAB非有限传播/警告并无原生证据，故仅承诺明确失败，不冒充逐位忠实MATLAB异常。仍未支持任意阶、多模块fitter、MAT/Session/Adaptive，DEC-005/008与所有其它TC未批准。
- **台账状态**：没有新造独立小目标/虚关原AF。仍为 **97个唯一AF＝74已整改＋23未整改**；审计路径新登记4项（1个SH生产CPP、1个SH测试CPP、1个已实跑Wolfram脚本、1个独立数学参考JSON）后为 **82＝50已审计＋32正在审计＋0空白**。AF-045/051/055仍开放；旧其它23项均未缩减承诺。第一轮未清零，用户条件性的第二轮全面审计不启动。
