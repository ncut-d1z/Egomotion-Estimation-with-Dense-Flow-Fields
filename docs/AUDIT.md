# 文档审计计划与文件进度（AUDIT）

> **范围**：规范、设计、测试及治理文件的静态审阅，不是软件实现、编译、数值验收或批准。SRS/HLD/LLD仍为草案。最新文件三态见§3，问题两态以[AUDIT_PROCESS](AUDIT_PROCESS.md)唯一AF行维护。

## 1. 规则

- 文件状态只用**空白 / 正在审计 / 已审计**；已审计只表示明确范围的文档静态审阅，不表示软件或数学测试通过。
- 发现问题立即逐目标登记路径、具体义务和未整改；实际修改目标并在新固定提交复读后才记已整改。一文件多目标分行。
- 先锁定分支/提交，再查SRS↔HLD↔LLD、REQ↔AC↔TC↔P、版本/链接、数据及错误合同；证据归[PROGRESS](PROGRESS.md)，冻结身份归[版本矩阵](DOCUMENT_BASELINE_REGISTER.md)。旧审批JSON和reviews不回写。
- 遵守[AGENTS](../AGENTS.md)及[TODO](TODO.md)：origin只读，未经授权不执行项目Bash/CMake/CTest、MAT写入或生产C++；MATLAB/Octave/L1永久非必需。**三篇论文已取得，不再使用材料未上传作为当前卡点；收到论文也不自动授权实施/运行。**
- 规范归口：SRS管REQ/AC；HLD/ADR管架构及27边设计DAG；对象模型管生命周期；LLD管接口/数学；TEST_PLAN管66TC；PLAN管98P；PROGRESS管实际证据；版本矩阵管SHA/签署；TODO管需要人类参与的事项。
- 持续推进文档及只读源码可完成的部分并及时提交。真实软件/独立参考/正式批准未齐的目标保持开放；只有第一轮全部适用目标实际清零后，才启动用户要求的第二轮全面审计。

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

原60个路径及其状态保留，新增两份论文相关文档。新增文档的“已审计”限于**来源标记、推导/候选合同、引用及未执行边界**；不能把候选公式文件的文字审阅当作其所有数值已验证。SRS/HLD/LLD仍非编译/冻结基线；历史reviews按固定时点事实审查。

| 仓库根相对路径 | 状态 | 核对重点 |
|---|---|---|
| `docs/AUDIT.md` | 已审计 | AU规则、完整路径/三态、现行与历史统计分开 |
| `docs/AUDIT_PROCESS.md` | 已审计 | 唯一AF、原目标、状态和固定提交结案证据 |
| `docs/REQUIREMENTS.md` | 正在审计 | 58正文+8附录REQ、AC/DEC与正式冻结；AF-037 |
| `docs/PLAN.md` | 正在审计 | 98P静态追踪已核；真实DEC-006/性能仍缺；AF-061 |
| `docs/TEST_PLAN.md` | 正在审计 | 原66TC及真实执行缺口；AF-051；论文测试候选另见SH-PAPER，不新增TC |
| `docs/design/README.md` | 已审计 | 10条覆盖/11份LLD；论文已提供、专题声明§5、真正剩余门槛 |
| `docs/DOCUMENT_BASELINE_REGISTER.md` | 正在审计 | 固定36项历史身份与实时集合分离；正式签署AF-058 |
| `docs/PROGRESS.md` | 正在审计 | 历史证据与本轮材料/静态成果分开；真实工程证据AF-053 |
| `docs/ARCHITECTURE.md` | 正在审计 | 27条设计边、部署/Writer；实际构建AF-038 |
| `docs/OBJECT_MODEL.md` | 正在审计 | 生命周期、45.m职责与批准；AF-039 |
| `docs/design/contracts.md` | 正在审计 | 统一错误/Result/所有权，分析/绘图映射已静态修订，实际ABI/异常AF-040 |
| `docs/design/cli.md` | 正在审计 | 参数、RunId、JSONL与真实负例；AF-041 |
| `docs/design/mat_io.md` | 正在审计 | Reader/Writer、RAII、TOCTOU、安全与L2；AF-042 |
| `docs/design/kvd.md` | 正在审计 | 两种公式/完整trace、六方向候选，独立逐轮L3/CTest仍缺；AF-043 |
| `docs/design/adaptive_mfa.md` | 正在审计 | M1～M4、MSE、Session与原子提交；AF-044 |
| `docs/design/harmonics.md` | 正在审计 | 论文已齐，转置/相位/全R5一般验证及实现仍缺；AF-045 |
| `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 正在审计 | 勘误冲突/P边界/短路/声明已细化，独立数值/真实实现仍缺；AF-046 |
| `docs/design/sampling_noise.md` | 正在审计 | 全量有序采样/RNG独立参考与测试；AF-047 |
| `docs/design/trajectory.md` | 正在审计 | 严格反向源异常/TR-C候选已核，独立L3/CTest仍缺；AF-048 |
| `docs/design/experiments.md` | 正在审计 | 五入口/save/Manifest/聚合，实际Runner仍缺；AF-049 |
| `docs/design/analysis_plotting.md` | 正在审计 | 曲线、分段/平滑、源插图与历史字段；完整L3/CTest仍缺；AF-050 |
| `docs/decisions/DEC-004_HISTORICAL_MAT_FIELDS.md` | 已审计 | 已选规则，历史来源未知不阻塞 |
| `docs/decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md` | 已审计 | 保留当时所有者技术决定 |
| `docs/golden/NO_MATLAB_EXECUTION_POLICY.md` | 已审计 | L1永久非必需 |
| `docs/golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md` | 正在审计 | 逐AC真实L2/L3/L4缺口；AF-060 |
| `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | 正在审计 | 筛查不等于最终DEC-005；AF-054 |
| `docs/golden/analytic_golden_candidates.v1.json` | 正在审计 | 原AN-G01～12保持候选，未因新论文修改数据；AF-055 |
| `docs/TODO.md` | 正在审计 | PAPER-SH材料已接受，其余执行/批准仍缺；完整AF-057未关闭 |
| `docs/MAT_DATA_SCHEMA.md` | 正在审计 | 历史582元数据与独立SHA-256/逐值区别；AF-056 |
| `docs/SOURCE_INVENTORY.md` | 已审计 | 45源文件静态身份，不等于软件验收 |
| `docs/MIGRATION_MATRIX.md` | 已审计 | 45文件/50函数声明级映射，数学/实现仍缺 |
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
| `docs/reviews/AN_G08_Z90_STATIC_CONVENTION_2026-11-08.md` | 已审计 | z90限定特例，不代表一般旋转方向/相位已正确 |
| `docs/reviews/AUDIT_ANALYSIS_SEGMENT_SMOOTHING_CORRECTION_2026-11-08.md` | 已审计 | 纠正旧窗口差异断言，不是软件测试 |
| `docs/reviews/AUDIT_ANALYSIS_ERROR_MAPPING_SCAN_2026-11-08.md` | 已审计 | AF-077发现证据 |
| `docs/reviews/AN_G_LEVEL0_FACE_ORDER_STATIC_2026-11-08.md` | 已审计 | 初始面有序符号推导，非完整递归L3 |
| `docs/reviews/AN_G_RECURSION_INDEX_STATIC_2026-11-08.md` | 已审计 | A/B/C/D索引证明，非全量数值参考 |
| `docs/reviews/AUDIT_BIAS_XWERTE_LAST_RUN_STATIC_2026-11-08.md` | 已审计 | Bias末轮x/均值y静态审查 |
| `AGENTS.md` | 已审计 | 权限/数据安全规则；现行材料状态以TODO与本轮记录为准 |
| `README.md` | 已审计 | 项目范围与证据/许可边界 |
| `LICENSE` | 已审计 | 项目MIT与上游作品区分 |
| `.gitignore` | 已审计 | 固定output忽略规则 |
| `scripts/check_docs_traceability.sh` | 已审计 | 仅脚本源码审阅，本轮未执行 |
| `docs/reviews/SH_PAPERS_SOURCE_AUDIT_2026-11-09.md` | 已审计 | 三PDF身份/页图、勘误与源码差异、纸面反例及证据限制；非全数值验收 |
| `docs/golden/SH_PAPER_VERIFICATION_CASES.md` | 已审计 | 25公式与10用例的候选设计/来源标记已审；数据生成和实际TC仍未执行 |

## 4. 现行结论与进度变化（2026-11-09）

- **文件三态：62个唯一对象＝39已审计＋23正在审计＋0空白。** 较本轮开始60/37/23新增两份限定静态文档；没有把23个在审阶段文件直接转已审计。
- **问题两态：96个唯一AF＝73已整改＋23未整改。** 本轮开始92/69/23，新增AF-092～095先登记未整改、实际修订并固定复读后关闭；这些只关闭材料状态/候选文档缺项，不关闭原AF-045/046等完整数值目标。
- 材料缺失已解除：1996原文、1998勘误、1999论文全部收到并登记；当前主要球谐风险是**IR98表项冲突、论文/源码转置、基相位及物理旋转语义**。独立全元素数值参考、生产Eigen、CTest/批准仍缺，不再要求上传论文。
- 66REQ（58+8）、66TC、98P、45.m/50函数和27边设计DAG的既有范围不变；新SH-P01～10只是原TC内的验证候选，不新增TC。
- 历史582/582 MAT探针不等于独立SHA-256、逐值L2、Reader/Writer/TOCTOU或算法验收。附件PDF哈希也不能替代MAT数据检查；原AN-G JSON不变。
- 仍保持第一轮；原23个阶段目标未实际清零，不启动条件性的第二轮全面审计。文件三态与AF两态只反映各自明确范围，不可混成软件完成率。

## 5. 固定证据与历史访问

AF-092～095首先登记于`d0618b8f02ba35ff47ae8355eca8f319cde8b7ed`，目标修订和固定复读点见[AUDIT_PROCESS §2](AUDIT_PROCESS.md)。论文报告、测试候选、harmonics/专题/TODO/索引在`f2c3aca4205035d670df625d22d4b48935cb4a2f`复读后，`b2414382a326fa89aff9f00bebe97a211c942210`关闭四项。此证据不是实际执行日志。

本文件旧§6～21逐轮过程及原始措辞完整保留于[本轮前固定版本](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/98e95e2f867dc7142fa0d5ac0f1c79f5d6429ed1/docs/AUDIT.md)，与各reviews及PROGRESS历史证据共同使用；当前索引不重复堆积旧“最新”统计，也不改变旧快照内容。旧文档写“尚未上传”的句子只能按当时事实读取，现行材料状态由TODO和本轮审计覆盖；它不意味着其余旧目标已经通过。
