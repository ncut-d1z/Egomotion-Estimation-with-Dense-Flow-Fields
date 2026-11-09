# 审计计划与文件进度（AUDIT）

> **范围**：规范、设计、测试、治理及已明确授权的球谐实现/验证工件。文件三态见§3，问题两态以[AUDIT_PROCESS](AUDIT_PROCESS.md)唯一AF行维护。数学符号证明、C++源码审阅、实际编译/浮点测试和正式验收必须分别陈述。

## 1. 规则

- 文件状态只用**空白 / 正在审计 / 已审计**；已审计仅表示该行核对重点所限定的范围，不自动表示软件测试或批准通过。
- 发现问题登记路径、义务和未整改；既有目标的新子问题可在原AF下登记，不以新增文档条目虚增结案数。实际修改并在固定提交复读/实际验证后才能按完整义务结案。
- 先锁定分支/提交，再查SRS↔HLD↔LLD、REQ↔AC↔TC↔P、版本/链接、数据及错误合同；证据归[PROGRESS](PROGRESS.md)，冻结身份归[版本矩阵](DOCUMENT_BASELINE_REGISTER.md)。旧审批JSON和历史reviews不回写。
- 遵守[AGENTS](../AGENTS.md)、[TODO](TODO.md)和[球谐专项授权](decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md)：**仅球谐C++及其专项测试源码允许实现，Wolfram符号验证允许执行**；其他模块不开工。用户已亲自执行一次限于球谐的 CMake/CTest（固定 HEAD 8eab5dd，1/1 Passed），但代理仍没有其他模块/全工程/MAT写回读的执行授权。origin只读，MATLAB/Octave/L1永久非必需。
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

保留此前62个路径，新增本轮12个球谐专项工件。其中6份代码/测试源码保持正在审计（原生执行未验）；授权/报告/三符号脚本/结果共6份在限定范围内已审阅。**符号脚本实际执行通过不同于C++测试源码仅已编写。**

| 仓库根相对路径 | 状态 | 核对重点 |
|---|---|---|
| `docs/AUDIT.md` | 已审计 | AU规则、完整路径/三态、现行与历史统计分开 |
| `docs/AUDIT_PROCESS.md` | 已审计 | 唯一AF、完整目标、状态及本轮数学子项证据 |
| `docs/REQUIREMENTS.md` | 正在审计 | 58正文+8附录REQ、AC/DEC与正式冻结；AF-037 |
| `docs/PLAN.md` | 正在审计 | 98P静态追踪已核；真实DEC-006/性能仍缺；AF-061 |
| `docs/TEST_PLAN.md` | 正在审计 | 原66TC；SH固定 HEAD 8eab5dd 的 1/1 CTest 用户实测已记录；此后新SH断言未执行，AF-051仍开放 |
| `docs/design/README.md` | 已审计 | LLD索引的限定静态范围；球谐新进展由harmonics/PROGRESS覆盖，非全软件通过 |
| `docs/DOCUMENT_BASELINE_REGISTER.md` | 正在审计 | 固定36项历史身份与实时集合分离；正式签署AF-058 |
| `docs/PROGRESS.md` | 正在审计 | 实际Wolfram与未执行native分开；完整工程证据AF-053 |
| `docs/ARCHITECTURE.md` | 正在审计 | 27条设计边；本轮未创建业务target；AF-038 |
| `docs/OBJECT_MODEL.md` | 正在审计 | 生命周期、45.m职责与批准；叶子不冒充公共类；AF-039 |
| `docs/design/contracts.md` | 正在审计 | 实际公共ABI/异常AF-040；未获准开启该模块代码 |
| `docs/design/cli.md` | 正在审计 | 参数、RunId、JSONL与真实负例；AF-041 |
| `docs/design/mat_io.md` | 正在审计 | Reader/Writer、RAII、TOCTOU、安全与L2；AF-042 |
| `docs/design/kvd.md` | 正在审计 | 两种公式/完整trace、六方向候选，独立逐轮L3/CTest仍缺；AF-043 |
| `docs/design/adaptive_mfa.md` | 正在审计 | M1～M4、MSE、Session与原子提交；AF-044；本轮没有实现 |
| `docs/design/harmonics.md` | 正在审计 | 九基与一般三路精确证明已完成，叶子代码已提交；native/集成未验；AF-045 |
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
| `docs/TODO.md` | 正在审计 | 材料、SH代码/Wolfram权限已获得；native/其他批准仍缺；AF-057 |
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
| `CMakeLists.txt` | 已审计 | 静态检查：C++17/Eigen3、仅球谐静态库、单项 CTest；实际 GCC16 构建证据来自用户，不是代理运行 |
| `build.sh` | 已审计 | 静态检查：Release/编译器选择、源码外构建、日志/pipefail、CTest；Git 模式 100755；实际运行来自用户终端 |
| `scripts/check_docs_traceability.sh` | 已审计 | 仅脚本源码审阅，本轮未执行 |
| `docs/reviews/SH_PAPERS_SOURCE_AUDIT_2026-11-09.md` | 已审计 | 上轮三PDF/勘误/相位静态记录，保持历史范围 |
| `docs/golden/SH_PAPER_VERIFICATION_CASES.md` | 已审计 | 上轮25公式/10用例设计；新增实际执行另记，不回填为历史通过 |
| `docs/decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md` | 已审计 | 仅SH代码和Wolfram授权、其他模块及native边界 |
| `include/egomotion/harmonics/low_order.hpp` | 正在审计 | 局部Result/API已有GCC16球谐构建实证；公共contracts/ABI未冻结，仍在审 |
| `src/harmonics/low_order_detail.hpp` | 正在审计 | SO(3)/方向校验、舍入边界静态核；GCC16构建已成功，未全面覆盖浮点/异常边界 |
| `src/harmonics/low_order_basis.cpp` | 正在审计 | 九基/递推/重建/转换已符号对照，旧固定 HEAD 经用户球谐 CTest 运行；仍缺独立异常/全面浮点边界覆盖 |
| `src/harmonics/low_order_rotation.cpp` | 正在审计 | 张量与IR公式全域符号核查，旧固定 HEAD 已成功编译/运行单项 CTest；尚缺全部边界及接口冻结 |
| `src/harmonics/low_order_choi.cpp` | 正在审计 | 复递推/W共轭一般符号已核，旧固定 HEAD 已编译/运行单项 CTest；仍缺超常边界和高精度参考扩充 |
| `tests/unit/test_low_order_harmonics.cpp` | 正在审计 | 旧固定 SHA 8eab5dd 的单项 CTest 由用户报告 Passed；新增解析锚点、dyadic、MATLAB相位回归尚待再次原生执行 |
| `verification/harmonics/verify_low_order_basis.wl` | 已审计 | 已执行：九基定义/递推、Gram81、Laplacian和dyadic恒等式 |
| `verification/harmonics/verify_low_order_rotations.wl` | 已审计 | 已执行：一般四元数SO(3)、两递推各34项对定义精确零差 |
| `verification/harmonics/verify_low_order_layout.wl` | 已审计 | 已执行：任意系数同场性、往返和旧相位反例 |
| `verification/harmonics/verification_results_2026-11-09.json` | 已审计 | 实际Wolfram输出/版本/固定输入/独立25值，native明确未执行 |
| `docs/reviews/LOW_ORDER_SH_THREE_ROUTE_VERIFICATION_2026-11-09.md` | 已审计 | 三路一般证明、C++逐式对应、纸面错误与实际执行限制 |

## 4. 现行结论与进度变化（2026-11-09球谐构建回填）

- **文件三态：76个唯一对象＝47已审计＋29正在审计＋0空白。** 从上一轮 74/45/29 新纳入根构建文件2份，均限定为静态审核；6份球谐源/测试仍因新增回归及完整验收未完成而在审。 从62/39/23新增12份专项工件，其中6份源码/测试待原生验收、6份文档/符号证据已限定审阅。分母扩大不表示此前工作倒退。
- **问题两态：97个唯一AF＝74已整改＋23未整改。** AF-096 已依据用户提供的固定 SHA 球谐构建/CTest 记录、跨文档修订和新提交定向复核结案；原 AF-045/046/051/055 的完整工程、数值、异常和批准义务仍全部开放。新测试代码在成功构建之后提交，尚未再次执行，不能计为通过。 本轮直接推进AF-045/046/051/055/057的数学/实现子目标，不人为新造小问题并关闭以改变百分比。
- 九基值定义与递推、一般SO(3)三路线全矩阵和布局同场性已有 Wolfram 精确结果；**GCC 16.2/CMake4.3.4/Eigen3.4 下球谐旧版在用户VM实际构建成功且 1/1 CTest Passed**。新测试增加解析值、dyadic 符号和 MATLAB 相位锚点，尚未在新 HEAD 重跑；其他集成/DEC-005/全量异常验证仍为卡点。
- 2015 p.30的d1²/d2²展开h4符号错误已按定义证明并记录在AF-045子项；IR98残留减号被全域证明与区分性反例排除。不改原PDF，不开启其他模块代码。
- 原66REQ/66TC/98P/45.m/50函数/27设计边不变；MAT字典、其他模块参考与正式工程验收不被专项结果替代。
- 第一轮整体未清零，不启动条件性的第二轮全面审计。

本次原生结果的范围和剩余缺口见 [PROGRESS E-DOC-056](PROGRESS.md) 与 [AUDIT_PROCESS AF-096 结案](AUDIT_PROCESS.md)；上轮 74 文件、96 AF 的事实为旧 HEAD 快照，不能再充当动态统计。

## 5. 固定证据与历史访问

本轮起点`4fe7be6cc50a2c2b7dec1555edf9859c0497870b`；专项授权`ae8c094`，代码/三符号脚本检查点`7eb04ede3d6f381caad5be2f80bba88b16d21b3c`，结果`0985914`、报告`9a508c6`。具体已执行结果与未执行native分开见PROGRESS及结果JSON。

AF-092～095上轮先登记/修订/复读历史、此前完整AU/文件/目标集合均由Git固定版本及原reviews保留。旧“尚未上传”“全部未实施”仅属当时事实；不得再反向覆盖当前明确授权与新增代码/数学证据。
