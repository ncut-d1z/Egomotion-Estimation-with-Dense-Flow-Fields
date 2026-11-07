# 项目所有者的最新设计决定：MIT / Eigen / CLI / 数值与实验策略（2026-11-07）

> **效力**：本文件登记项目所有者在当前对话中作出的**明确技术方向决定**，对历史 `HUMAN_APPROVAL_REQUEST.json` 及先前仅供起草的方案中发生冲突的**设计口径**具有后续优先效力；不修改原审批记录。**选择技术方案不等于生产代码已实施、构建/CTest/MAT 写入已获执行权限或 AC 已通过。** 球谐正式实现须等待用户上传指定论文后才能开展。原 `origin/**` 只读，不运行 MATLAB/Octave、不要求 L1。

## 已确认：无需继续逐项请求选择的工程方案

**后续 DEC-004 补充决定**：现行 Runner 仅生成当前有效 `save` 字段；历史 MAT 额外字段标 `HistoricalOnly`、只读保留；新结果分析必需字段缺失时明确失败；来源未知不阻塞开发。详见 [DEC-004 独立决定](DEC-004_HISTORICAL_MAT_FIELDS.md)。本条只登记新决策的引用，不改变其他已选工程约束。

| 决策 | 当前工程方案 |
|---|---|
| 项目许可证 | 本仓库**自行编写的项目代码与文档采用 MIT License**，仓库根 [LICENSE](../../LICENSE) 已建立。MIT 不把第三方独立作品、原作者归档 MAT/.m 或论文自动重新许可；在实际引入第三方文件时保留其现有来源/许可告知即可，**不再因一般版权审查待办阻塞开发任务或反复要求所有者作版权选择**。 |
| 球谐公式与代码 | 保留 H-14 选择的 Ivanic–Ruedenberg 路线，以 **Ivanic & Ruedenberg (1996) 原文 + (1998) 勘误**为必须阅读的数学依据；Don Williamson `IvanicSHRotate.cpp/.h` 仅作为已选代码参考。**必须以 Eigen 重写，实现中完全禁止 `sdla` include、link、vendoring 和作为间接运行依赖**。只实现 l=0/1/2，核 m 顺序、相位、R3(9)/R5(25)、系数前后。论文尚待用户下载/上传，**在此之前不编写球谐递推生产实现**。详见 [球谐文献与 Eigen 实现设计](../design/SH_ROTATION_IVANIC_RUEDENBERG.md)。 |
| 错误返回 | `Result<T,E>`、`Status<E>` 采用**轻量封装类**，内部 C++17 `std::variant`；唯一 `Failure<E>` 含 `ErrorDetail`；`ErrorStage` 用稳定 `enum class`；`message` 为自由文本；**JSONL 为统一结构化诊断格式**，实现时要正确转义、处理非有限值与 UTF-8，不引入无必要的大型 JSON 依赖。 |
| 实验配置 | 每个 Runner 使用各自**独立的配置类型**；CLI 可以仅在统一分发层使用 `std::variant`，不得改变五入口原始默认值/顺序。 |
| 运行与失败 | 每次 RunContext **独占** RNG、RunId、计数和 Manifest，Writer 以接口注入；独立样本失败可记录后继续，**依赖状态的 Adaptive 单步失败不得提交无效主系数**，不能恢复时停止当前连续序列；8 次聚合少一次就标为 `Incomplete`，**不改成 7 次平均、不伪装为原八次输出**；保存完整日志与可追溯部分产物，但不得覆盖/自动清理已有文件。Manifest 至少区分 `Complete`、`Incomplete`、`Failed` 并标明实际有效分母。 |
| 数值验收 | 确认 `atol=0.01`、`rtol=0.1` 仅作为**第一轮宽松探索性筛查**；形状/顺序/迭代数/MAT 原值/RNG 调用数/失败状态为严格检查，单位向量、旋转正交性、理想无噪残差建议独立 `1e-6` 候选；5° 方向误差仅为探索性筛查上限。**DEC-005 的正式分模块最终验收阈值仍需根据实际 C++/独立 L3 误差证据确定，不自动冻结**。 |
| MAT 输入支持 | 第一版仅支持 **二维、密集、实数、非 logical double**；inspect 可保留其他类型元数据，不静默转换。多变量 compat 在无独立顺序证据时返回 `AmbiguousVariable`，strict 明确变量名时可读；已存 582 MAT 单变量合同保持。 |
| CLI 输出路径 | **删除 `--output-root` CLI 选项（包括 run、analyze、plot）**。固定新结果根是仓库根 `output/`，其下每次新建 `output/<RunId>/`；`--run-id` 可选且拒绝不合法/重复值，无参数时通过 **操作系统独立于算法 RNG 的随机源**生成新 ID；必须对目录和最终文件实施不覆盖/并发/路径防逃逸策略。解析命令如无新输出则保持只读，不强行创建 RunId。 |
| target 依赖 | `egomotion_harmonics` 公共 API 需要 `Result/ErrorStage`，因此增添对 `egomotion_contracts` 的显式依赖并同步 HLD target 表及 DAG；`egomotion_contracts` 不依赖 `egomotion_harmonics`。 `sdla` 不得在目标依赖闭包中出现。 |
| 开发与测试阶段 | 采用 **阶段一 C++ 修改 → 阶段二隔离构建/CTest/安全负例 → 阶段三数值验收/合并发布** 的顺序。**本轮只要求方案/许可更新**，且用户明确会在开始球谐实现时上传论文；因此本轮不启动生产 C++、不运行 CMake/CTest/Bash/MAT 写入、不合并 PR。后续具体执行仍按当时明示的任务/运行范围授权。 |

## 必读原始文献及可访问入口

1. Joseph Ivanic, Klaus Ruedenberg. **Rotation Matrices for Real Spherical Harmonics. Direct Determination by Recursion**. *The Journal of Physical Chemistry*, **100**(15), 6342–6347 (1996). DOI: [10.1021/jp953350u](https://doi.org/10.1021/jp953350u). [ACS PDF 入口](https://pubs.acs.org/doi/pdf/10.1021/jp953350u)（ACS 当前标为付费访问；可能需要机构权限）。
2. 同作者。**Rotation Matrices for Real Spherical Harmonics. Direct Determination by Recursion** (Addition/Correction). *The Journal of Physical Chemistry A*, **102**(45), 9099–9100 (1998). DOI: [10.1021/jp9833350](https://doi.org/10.1021/jp9833350). [ACS PDF 入口](https://pubs.acs.org/doi/pdf/10.1021/jp9833350)（ACS 网页标为 Free to Read；工具直接取 PDF 可能返回 403）。

**1998 年勘误为必读且优先于 1996 原文中的错误公式**：作者说明原文的 eqs. 6.7、7.9b、7.9c 中有负号应为正号，eq. 7.9a 的某下标从 1 更正为 -1，并更新了表 1/2。实现时不得仅依据未更正的 1996 打印版或单一第三方 C++ 译本确定符号。论文上传前此处只做来源记录和接口/测试规划，不臆造完整递推公式。

## 仍需实际证据，不属于再次讨论的技术选择

生产 C++/接口、真实 target 编译和 CTest、独立 L2 逐元素/SHA-256、一般输入与退化输入 L3、R3/R5 全元素、错误/输出非覆盖、RNG 次数及统计 L4、最终数值阈值、性能/版本记录都**尚未验收**。本次技术方向确定**不宣称基线已冻结**；项目根 MIT 已获明确同意而不需再次就许可证作审批，第三方已有许可证告知仍予保留。
