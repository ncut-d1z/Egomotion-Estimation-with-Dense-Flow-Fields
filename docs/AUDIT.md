# 审计计划与文件进度（AUDIT）

> **范围**：规范、设计、测试、治理及已明确授权的球谐实现/验证工件。文件三态见§3，问题两态以[AUDIT_PROCESS](AUDIT_PROCESS.md)唯一AF行维护。数学符号证明、C++源码审阅、实际编译/浮点测试和正式验收必须分别陈述。

## 1. 规则

- 文件状态只用**空白 / 正在审计 / 已审计**；已审计仅表示该行核对重点所限定的范围，不自动表示软件测试或批准通过。
- 发现问题登记路径、义务和未整改；既有目标的新子问题可在原AF下登记，不以新增文档条目虚增结案数。实际修改并在固定提交复读/实际验证后才能按完整义务结案。
- 先锁定分支/提交，再查SRS↔HLD↔LLD、REQ↔AC↔TC↔P、版本/链接、数据及错误合同；证据归[PROGRESS](PROGRESS.md)，冻结身份归[版本矩阵](DOCUMENT_BASELINE_REGISTER.md)。旧审批JSON和历史reviews不回写。
- 遵守[AGENTS](../AGENTS.md)、[TODO](TODO.md)和[球谐专项授权](decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md)：**仅球谐C++及其专项测试源码允许实现，Wolfram符号验证允许执行**；其他模块不开工。用户已亲自执行三次限于球谐的 CMake/CTest（`8eab5dd`、`445ed151`、`601c74de` 均 1/1 Passed），但代理仍没有其他模块/全工程/MAT写回读的执行授权。origin只读，MATLAB/Octave/L1永久非必需。
- SRS管REQ/AC；HLD/ADR管架构及27边设计DAG；对象模型管生命周期；LLD管接口/数学；TEST_PLAN管66TC；PLAN管98P；PROGRESS管实际证据；版本矩阵管SHA/签署；TODO管人类事项。新专项用例不新增TC。
- 持续推进可授权部分并及时提交。第一轮未清零的整体义务不能以局部成功代替，第二轮全面审计仍以第一轮完成为前提。

## 2. 历轮评审要点（28项保留）

| 编号 | 审计原则 |
|---|---|
| AU-001 | 文档分类、审查结构与正式基线分离 |
| AU-002 | SRS外部义务与附录REQ-DC分层 |
| AU-003 | AC冻结引用绑定章节/版本/SHA/批准 |
| AU-004 | HLD组件、部署、27边DAG、ADR、质量场景 |
| AU-005 | 对象所有权、生命周期、45个.m映射 |
| AU-006 | 582 MAT格式/类型/来源与逐值区分 |
| AU-007 | MatDenseArray自拥有、RAII、边界 |
| AU-008 | 严格/兼容读取、路径安全、无覆盖 |
| AU-009 | CLI精确类型及错误码单一来源 |
| AU-010 | 无自定义output-root，固定output/<RunId>/ |
| AU-011 | Result/Status/ErrorDetail、JSONL显式失败 |
| AU-012 | 五实验原更新顺序与save忠实 |
| AU-013 | Bias xwerte与N、RNG消耗公平性 |
| AU-014 | Franz/KvD逐轮轨迹、阈值及停止 |
| AU-015 | Adaptive M1～M4、MSE、Session状态提交 |
| AU-016 | Ivanic 1996/1998、Eigen-only、禁sdla |
| AU-017 | Sophus物理旋转、Rᵀ、epsilon/反向 |
| AU-018 | DEC-004当前生成/历史专属/缺字段 |
| AU-019 | 分析索引、分段、平滑、数值曲线 |
| AU-020 | 不运行MATLAB/L1；L2/L3/L4分级 |
| AU-021 | 0.01/0.1仅筛查；最终DEC-005待定 |
| AU-022 | 66TC实体/CTest证据真实性 |
| AU-023 | 98P、里程碑、角色与证据归口 |
| AU-024 | 统一版本/SHA/批准身份登记 |
| AU-025 | MIT与第三方notice边界 |
| AU-026 | 历史审批表认知负荷与权限边界 |
| AU-027 | 人类待办集中TODO，不重复决策 |
| AU-028 | 历轮回归扫描、风险分级 |

## 3. 审计对象与逐文件三态

保留历史62个路径，随后已新增12个球谐专项工件、2个根构建文件、2个AN-G06/07/08归档测试/证明和本轮4个原koeff_aus_near二阶投影工件。新增SH生产源及测试源码**正在审计，未原生编译/测试**；新增Wolfram脚本与独立参考JSON的有限数学身份已审阅。现有旧通过结果只属于601c74de及更早固定HEAD，不能把新增组合冒充为已通过3/3。

| 仓库根相对路径 | 状态 | 核对重点 |
|---|---|---|
| `docs/AUDIT.md` | 已审计 | AU规则、完整路径/三态、现行与历史统计分开 |
| `docs/AUDIT_PROCESS.md` | 已审计 | 唯一AF、完整目标、状态及本轮数学子项证据 |
| `docs/REQUIREMENTS.md` | 正在审计 | 58正文+8附录REQ、AC/DEC与正式冻结；AF-037 |
| `docs/PLAN.md` | 正在审计 | 98P静态追踪已核；真实DEC-006/性能仍缺；AF-061 |
| `docs/TEST_PLAN.md` | 正在审计 | SH 5e522198 已在 Ubuntu/GCC16 原生3/3 CTest PASS（53532/176/76 checks）；正式66TC/DEC-005、其它模块继续开放，AF-051 |
| `docs/design/README.md` | 已审计 | LLD索引的限定静态范围；球谐新进展由harmonics/PROGRESS覆盖，非全软件通过 |
| `docs/DOCUMENT_BASELINE_REGISTER.md` | 正在审计 | 固定36项历史身份与实时集合分离；正式签署AF-058 |
| `docs/PROGRESS.md` | 正在审计 | 用户原生 5e522198 的构建/3项CTest报告归 E-DOC-062；全工程/资源/正式证据仍缺，AF-053 |
| `docs/ARCHITECTURE.md` | 正在审计 | 27条设计边；本轮未创建业务target；AF-038 |
| `docs/OBJECT_MODEL.md` | 正在审计 | 生命周期、45.m职责与批准；叶子不冒充公共类；AF-039 |
| `docs/design/contracts.md` | 正在审计 | 实际公共ABI/异常AF-040；未获准开启该模块代码 |
| `docs/design/cli.md` | 正在审计 | 参数、RunId、JSONL与真实负例；AF-041 |
| `docs/design/mat_io.md` | 正在审计 | Reader/Writer、RAII、TOCTOU、安全与L2；AF-042 |
| `docs/design/kvd.md` | 正在审计 | 两种公式/完整trace、六方向候选，独立逐轮L3/CTest仍缺；AF-043 |
| `docs/design/adaptive_mfa.md` | 正在审计 | M1～M4、MSE、Session与原子提交；AF-044；本轮没有实现 |
| `docs/design/harmonics.md` | 正在审计 | 九基/旋转/二阶源逐系数投影均有独立数学与固定 5e522198 原生数值验收；公共ABI/完整动态阶数/最终DEC-005仍缺，AF-045 |
| `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 正在审计 | 原规划公共接口仍待适配，当前叶子/证明见三路报告；AF-046 |
| `docs/design/sampling_noise.md` | 正在审计 | 全量有序采样/RNG独立参考与测试；AF-047 |
| `docs/design/trajectory.md` | 正在审计 | 严格反向源异常/TR-C候选已核，独立L3/CTest仍缺；AF-048 |
| `docs/design/experiments.md` | 正在审计 | 五入口/save/Manifest/聚合，实际Runner仍缺；AF-049 |
| `docs/design/analysis_plotting.md` | 正在审计 | 曲线、分段/平滑、源插图与历史字段；完整L3/CTest仍缺；AF-050 |
| `docs/decisions/DEC-004_HISTORICAL_MAT_FIELDS.md` | 已审计 | 已选规则，历史来源未知不阻塞 |
| `docs/decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md` | 已审计 | 保留当时所有者技术决定 |
| `docs/golden/NO_MATLAB_EXECUTION_POLICY.md` | 已审计 | L1永久非必需 |
| `docs/golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | 正在审计 | 逐AC真实L2/L3/L4缺口；SH新增实证不代表全部完成；AF-060 |
| `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | 正在审计 | 精确等式不等于最终double容差；AF-054 |
| `docs/golden/analytic_golden_candidates.v1.json` | 正在审计 | 原AN-G01～12未改，不以新SH结果替代整体验收；AF-055 |
| `docs/TODO.md` | 正在审计 | 材料、SH专项实现/Wolfram与用户两次球谐native均有证据；代理执行权限/其它批准仍缺；AF-057 |
| `docs/MAT_DATA_SCHEMA.md` | 正在审计 | 历史582元数据与独立SHA-256/逐值区别；AF-056 |
| `docs/SOURCE_INVENTORY.md` | 已审计 | 45源文件静态身份，不等于软件验收 |
| `docs/MIGRATION_MATRIX.md` | 已审计 | 45文件/50函数历史声明映射；新叶子与公共适配分开 |
| `docs/approval/DECISION_IMPLEMENTATION_NOTES.md` | 已审计 | 历史权限/决定释义，不回写 |
| `docs/approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md` | 已审计 | 历史审批包，不回写 |
| `docs/reviews/STATIC_TRACEABILITY_2026-11-06.md` | 已审计 | 固定历史静态记录，只读 |
| `docs/reviews/DOC_CONSISTENCY_RECONCILIATION_2026-11-06.md` | 已审计 | 固定历史静态记录，只读 |
| `docs/reviews/LLD_MAT_IO_AND_CONSISTENCY_2026-11-07.md` | 已审计 | 固定历史静态记录，只读 |
| `docs/reviews/LLD_CONTRACTS_TARGETS_TESTS_ALIGNMENT_2026-11-07.md` | 已审计 | 固定历史静态记录，只读 |
| `docs/reviews/OWNER_TECH_DECISIONS_ALIGNMENT_2026-11-07.md` | 已审计 | 固定历史决定对齐记录，只读 |
| `docs/reviews/DOCUMENT_REVIEW_AND_SH_DECLARATIONS_2026-11-07.md` | 已审计 | 历史声明级记录，只读 |
| `docs/reviews/FULL_DOCUMENT_AUDIT_2026-11-07.md` | 已审计 | 历史全量扫描，不冒充当前HEAD |
| `docs/reviews/AUDIT_AF001_AF012_RESOLUTION_2026-11-07.md` | 已审计 | 历史整改证据，只读 |
| `docs/reviews/AUDIT_AF013_AF020_RESOLUTION_2026-11-07.md` | 已审计 | 历史整改证据，只读 |
| `docs/reviews/AUDIT_FOLLOWUP_SCAN_2026-11-07.md` | 已审计 | 历史增量扫描，只读 |
| `docs/reviews/AUDIT_AF021_AF025_RESOLUTION_2026-11-07.md` | 已审计 | 历史整改证据，只读 |
| `docs/reviews/AUDIT_SEVEN_MATH_DOCS_SCAN_2026-11-08.md` | 已审计 | 历史数学文档扫描，只读 |
| `docs/reviews/AUDIT_SEVEN_MATH_DOCS_RESOLUTION_2026-11-08.md` | 已审计 | 历史限定修订，只读 |
| `docs/reviews/AUDIT_FOUR_POLICY_DRIFTS_SCAN_2026-11-08.md` | 已审计 | 历史决策漂移扫描，只读 |
| `docs/reviews/AN_G_STATIC_FORMULA_REVIEW_2026-11-08.md` | 已审计 | 部分静态代数，不是L3/CTest通过 |
| `docs/reviews/AUDIT_FUNCTION_DECLARATIONS_AND_LEDGER_COMPACT_2026-11-08.md` | 已审计 | 45.m/50函数及治理压缩历史证据 |
| `docs/reviews/AN_G08_Z90_STATIC_CONVENTION_2026-11-08.md` | 已审计 | z90限定特例，旧结论不扩张 |
| `docs/reviews/AUDIT_ANALYSIS_SEGMENT_SMOOTHING_CORRECTION_2026-11-08.md` | 已审计 | 纠正旧窗口差异断言，不是软件测试 |
| `docs/reviews/AUDIT_ANALYSIS_ERROR_MAPPING_SCAN_2026-11-08.md` | 已审计 | AF-077发现证据 |
| `docs/reviews/AN_G_LEVEL0_FACE_ORDER_STATIC_2026-11-08.md` | 已审计 | 初始面有序符号推导，非完整递归L3 |
| `docs/reviews/AN_G_RECURSION_INDEX_STATIC_2026-11-08.md` | 已审计 | A/B/C/D索引证明，非全量数值参考 |
| `docs/reviews/AUDIT_BIAS_XWERTE_LAST_RUN_STATIC_2026-11-08.md` | 已审计 | Bias末轮x/均值y静态审查 |
| `AGENTS.md` | 已审计 | 既有安全规则与最新用户专项授权共同适用 |
| `README.md` | 已审计 | 原项目范围/历史证据；最新代码状态以PROGRESS为准 |
| `LICENSE` | 已审计 | MIT与上游作品边界 |
| `.gitignore` | 已审计 | 输出/构建忽略，不绕过排除规则 |
| `CMakeLists.txt` | 已审计 | 静态配置 C++17、Eigen3、3个SH-only独立CTest；固定 5e522198 用户原生 CMake/GCC16 成功、3/3 CTest通过，未授权其它模块 |
| `build.sh` | 已审计 | 一次 --verbose CTest、源外构建、日志、pipefail 由固定5e522198真实执行，3/3 PASS；新脚本以后变更仍需按SHA实测 |
| `scripts/check_docs_traceability.sh` | 已审计 | 仅脚本源码审阅，本轮未执行 |
| `docs/reviews/SH_PAPERS_SOURCE_AUDIT_2026-11-09.md` | 已审计 | 上轮三PDF/勘误/相位静态记录，保持历史范围 |
| `docs/golden/SH_PAPER_VERIFICATION_CASES.md` | 已审计 | 上轮25公式/10用例设计；新增实际执行另记，不回填为历史通过 |
| `docs/decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md` | 已审计 | 仅SH代码和Wolfram授权、其他模块及native边界 |
| `include/egomotion/harmonics/low_order.hpp` | 正在审计 | 局部Result/API已有GCC16球谐构建实证；公共contracts/ABI未冻结，仍在审 |
| `src/harmonics/low_order_detail.hpp` | 正在审计 | SO(3)/方向校验、舍入边界静态核；GCC16构建已成功，未全面覆盖浮点/异常边界 |
| `src/harmonics/low_order_basis.cpp` | 正在审计 | 九基/递推/重建/转换已符号对照，旧固定 HEAD 经用户球谐 CTest 运行；仍缺独立异常/全面浮点边界覆盖 |
| `src/harmonics/low_order_rotation.cpp` | 正在审计 | 张量与IR公式全域符号核查，旧固定 HEAD 已成功编译/运行单项 CTest；尚缺全部边界及接口冻结 |
| `src/harmonics/low_order_choi.cpp` | 正在审计 | 复递推/W共轭一般符号已核，旧固定 HEAD 已编译/运行单项 CTest；仍缺超常边界和高精度参考扩充 |
| `src/harmonics/low_order_source_projection.cpp` | 正在审计 | 固定 5e522198 已于用户 Ubuntu GCC16 编译并经单独 source_projection CTest 76次检查通过；仍非完整公共Fitter/ABI/DEC-005 |
| `tests/unit/test_low_order_harmonics.cpp` | 正在审计 | `601c74de` 已实测 1/1 CTest：53,532检查、624非零四元数、错误码/溢出、双类残差；仍缺更广输入/ABI/正式容差 |
| `tests/unit/test_low_order_archived_golden.cpp` | 正在审计 | AN-G06/07/08 的源CS布局、R3/R5固定值在5e522198 原生CTest 176次检查通过；完整L3/正式批准仍缺 |
| `tests/unit/test_low_order_source_projection.cpp` | 正在审计 | 原源7方向二阶逐项拟合及异常负例在5e522198 原生CTest 76次检查通过；其他场景/通用API尚缺 |
| `verification/harmonics/verify_low_order_basis.wl` | 已审计 | 已执行：九基定义/递推、Gram81、Laplacian和dyadic恒等式 |
| `verification/harmonics/verify_low_order_rotations.wl` | 已审计 | 已执行：一般四元数SO(3)、两递推各34项对定义精确零差 |
| `verification/harmonics/verify_low_order_layout.wl` | 已审计 | 已执行：任意系数同场性、往返和旧相位反例 |
| `verification/harmonics/verify_archived_sh_candidates.wl` | 已审计 | 独立CS多项式的AN-G06九基/AN-G07常数/AN-G08全R3/R5和场作用已实际Wolfram精确验证，allExact=True；不等于新CTest PASS |
| `verification/harmonics/verify_source_projection_order2.wl` | 已审计 | 受限SH源逐项投影7方向数学公式在Wolfram精确执行，9差为0且分母均正、离散Gram有非零交叉项；不等于C++运行 |
| `verification/harmonics/verification_results_2026-11-09.json` | 已审计 | 实际Wolfram输出/版本/固定输入/独立25值，native明确未执行 |
| `verification/harmonics/source_projection_reference.v1.json` | 已审计 | 七方向9个精确根式系数+固定近度与Wolfram版本来源已复核；标注CPP_NOT_RUN、未批准DEC-005 |
| `docs/reviews/LOW_ORDER_SH_THREE_ROUTE_VERIFICATION_2026-11-09.md` | 已审计 | 三路一般证明、C++逐式对应、纸面错误与实际执行限制 |

## 4. 现行结论与进度变化（2026-11-09 SH 3/3 实测回填）

- **最新固定结果**：用户在`5e5221988cf093dbfddb88f2f65f06632460f3a1` 上实际编译完整SH叶子（含原二阶逐系数投影），并通过三个独立CTest **3/3，0失败**；程序分别报告53,532、176、76次内部检查，最大残差和环境日志见[PROGRESS E-DOC-062](PROGRESS.md)。这更新了下文旧HEAD的“第二/第三测试未执行”历史快照；用户日志未提供原始文件SHA-256或正式DEC-005批准，不能将SH局部PASS解释为全工程验收。


- **文件三态：82个唯一对象＝50已审计＋32正在审计＋0空白。** 从上一轮 74/45/29 新纳入根构建文件2份，均限定为静态审核；6份球谐源/测试仍因公共接口与数值边界/正式验收未完成而在审。历史从62/39/23增加12份工件，再新增根构建文件2份，均已单独记录。分母扩大不表示此前工作倒退。
- **问题两态：97个唯一AF＝74已整改＋23未整改。** AF-096 已依据用户提供的固定 SHA 球谐构建/CTest 记录、跨文档修订和新提交定向复核结案；原 AF-045/046/051/055 的完整工程、数值、异常和批准义务仍全部开放。增强解析/相位测试在 `445ed151` 通过；后新增的624非零四元数、错误分类和误差报告已经在用户 `601c74de` 实测 1/1 Passed。**再后来修改的build.sh仅完成静态复核，尚无新版本运行结果**。 本轮直接推进AF-045/046/051/055/057的数学/实现子目标，不人为新造小问题并关闭以改变百分比。
- 九基值定义与递推、一般SO(3)三路线全矩阵和布局同场性已有 Wolfram 精确结果；**GCC 16.2/CMake4.3.4/Eigen3.4 下球谐旧版在用户VM实际构建成功且 1/1 CTest Passed**。增强解析值、dyadic符号、MATLAB相位锚点已在 `445ed151` 实测通过；624四元数/53,532检查/错态/残差在 `601c74de` 实测通过。未解决的卡点是更广浮点边界、正式公共接口、工程集成、DEC-005和新脚本运行证据。
- 2015 p.30的d1²/d2²展开h4符号错误已按定义证明并记录在AF-045子项；IR98残留减号被全域证明与区分性反例排除。不改原PDF，不开启其他模块代码。
- 原66REQ/66TC/98P/45.m/50函数/27设计边不变；MAT字典、其他模块参考与正式工程验收不被专项结果替代。
- 第一轮整体未清零，不启动条件性的第二轮全面审计。

本次原生结果的范围和剩余缺口见 [PROGRESS E-DOC-056](PROGRESS.md) 与 [AUDIT_PROCESS AF-096 结案](AUDIT_PROCESS.md)；上轮 74 文件、96 AF 的事实为旧 HEAD 快照，不能再充当动态统计。

更多当前事实与未来测试的界限见 [PROGRESS E-DOC-057/058](PROGRESS.md) 和 [TEST_PLAN](TEST_PLAN.md)；AF-045/046/051 等完整目标仍为开放状态。

**本轮新增固定证据**：用户 `601c74de` 的详细输出记录 **624个非零四元数输入、53,532次require检查、1/1 CTest Passed**，另有双类最大绝对残差。见 [PROGRESS E-DOC-059](PROGRESS.md)；`build.sh` 后续verbose调整**未运行**，参见 [AUDIT_PROCESS §7](AUDIT_PROCESS.md)。

**最新增量（AN-G06/07/08）**：已接入第二个独立球谐 CTest，并且另存重放 Wolfram 精确核对。20行方向与9基/15系数/R3九项/R5二十五项和归档候选的字面差异都为0；Wolfram返回allExact=True。**当前完整可执行CTest为2项，但最新版本尚未由用户执行，不能写2/2通过**。详见 [PROGRESS E-DOC-060](PROGRESS.md)、[TEST_PLAN 当前状态](TEST_PLAN.md) 与 [AUDIT_PROCESS §8](AUDIT_PROCESS.md)。

**新一期数学与代码分层（AF-045/051/055）**：原`koeff_aus_near.m` 的二阶**逐系数投影**现有单独SH实现、独立7方向Wolfram精确参考及第三个CTest（源代码尚未编译）。归档AN-G06/07/08第二CTest同样尚未原生运行；因此当前是3个已注册 SH CTest、仅历史旧版1/1有用户证据，不能标3/3通过。证明与差异详见 [PROGRESS E-DOC-061](PROGRESS.md)、[AUDIT_PROCESS §9](AUDIT_PROCESS.md)、[TEST_PLAN](TEST_PLAN.md)。

**本轮 3/3 验收核查**：本分支所有 82 个文件路径/97 个AF ID仍按原台账的唯一行计算：82文件（50已审计、32在审、0空白），97个整改目标（74已整改、23未整改）。没有新增/关闭原复合AF；包括AF-045/046/051/055/060的全业务/最终容差、异常广度等义务仍开放。新提交后的 C++ 修改必须重新测试，不继承`5e522198`旧HEAD结果。

## 5. 固定证据与历史访问

本轮起点`4fe7be6cc50a2c2b7dec1555edf9859c0497870b`；专项授权`ae8c094`，代码/三符号脚本检查点`7eb04ede3d6f381caad5be2f80bba88b16d21b3c`，结果`0985914`、报告`9a508c6`。具体已执行结果与未执行native分开见PROGRESS及结果JSON。

AF-092～095上轮先登记/修订/复读历史、此前完整AU/文件/目标集合均由Git固定版本及原reviews保留。旧“尚未上传”“全部未实施”仅属当时事实；不得再反向覆盖当前明确授权与新增代码/数学证据。
