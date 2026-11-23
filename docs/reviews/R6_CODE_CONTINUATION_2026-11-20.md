# R6 代码接续：聚合证据与分析边界（2026-11-20）

> **同日后续**：所有者再次要求继续代码实现，后续 Session/Manifest/实验异常边界修复见 [失败边界接续](R6_FAILURE_BOUNDARY_CONTINUATION_2026-11-20.md)。构建、测试仍延期，两批均没有新运行证据。

`[HUMAN-EXECUTE/APPROVE]`。所有者在 DEC-R6-01～04 实现之后明确要求“继续代码实现，推迟构建、测试”。本次从已核实的 Draft [PR #6](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/pull/6) HEAD `9e6ea3ae314426d67beb275374739eb111efa0f4` 接续，保持分支 `dev-plan-P0-P1-mat-dictionary-probe`。**本次只有代码、必要回归源码、构建登记和静态审阅；没有执行配置、编译、语法编译、CTest、测试程序、CLI、MAT I/O 或科学实验。**

此前 `594a1b11...` 的 12 项及 `2ed5c2d...` 的补充 1 项仅为其固定版本的实测记录，见 [前阶段报告](R6_METRIC_DECISIONS_IMPLEMENTATION_2026-11-20.md)。本次生产源码已有修改，不能继续称新 HEAD 已通过这些用例。四项指标决定仍适用，不增加数值阈值或恢复其它延期事项。

## 本次修复与补齐

| 项目 | 静态定位的问题 | 本次源码行为 |
|---|---|---|
| 聚合证据与原子提交 | 原追加入口只检查部分形状/次数及缺测轮号，遗漏“每个 NaN 都有唯一原因”和逐帧有效/缺测账本的交叉核对；未声明 NaN 的轮次可先被记为聚合成功。下游完整输出门禁原已拒绝这种数据，不能据此声称错误 MAT 已发布。 | 复用同一套严格证据验证，区分累计和与最终结果；接受轮次前、提交候选前和最终化时均核对原值、次数、原因、帧计数。失败保留调用者的原聚合，最终均值仍固定除 8。 |
| 平滑结果溢出 | `smoothSource` 的输入即使全部有限，连续加法仍可能溢出；此前会把该结果作为已定义值交给 Analyzer 的成功返回。 | 完整窗口求和非有限时返回 `UndefinedStatistic`，携带源 MAT 路径、变量及 MATLAB 帧号。原求和顺序、固定窗口、分母、边缘零值和缺测传播保持。 |
| SVG 坐标溢出 | 原 X/Y 有限不能保证坐标范围相减和像素换算仍有限；例如 `{-DBL_MAX,0,DBL_MAX}` 的范围相减溢出，可能生成 `nan` 坐标并发布。 | 换算后的像素坐标必须有限，否则返回 `InvalidSeries`；利用既有全部面板先准备、后发布的顺序，在发布前拒绝，不改写原数值数据。 |
| 分析运行记录的指标身份 | `analysis_ready` 与导出 `invocation.json` 只保存原分析脚本及辅助脚本身份，没有直接保留已经存在于数值 JSON 中的新旧指标政策、次数和每条曲线语义。 | 两种记录共用字段序列化，保存实际 `metric_policy`、`planned_repetitions`、`metric_semantics` 和 `y_unit`；未知来源仍原样保留 0/空字段，不从命令、文件名或源 blob 推测。 |

### 聚合仍遵守方案 B

缺测值继续保存 NaN、0～8 有效次数及 1-based 轮/帧/列原因；同一帧多个指标缺测只计一个缺测帧。任一轮缺测时，八轮均值仍缺测，不能改成对剩余有效轮数取平均。`rottransratio` 继续取末轮，早轮缺测不被隐去，末轮缺测不回退到旧轮结果。完整输出仍要求所有计划求解轮次完成；本次没有把求解失败变成可发布的完整轨迹。

累计阶段与最终阶段共用证据规则，避免上游接受、下游拒绝之间存在两套不一致的缺测解释。原材料、运动误差公式、分段阈值和科学采样规则不变；本次修复的是已批准合同的实现边界。

最终化在除以 8 **之前**再次核对累计和，之后才核对最终均值。静态复核发现，仅验证除法后的结果会漏掉“八轮原值均为零，但受损累计和为 `double::denorm_min()`”的情况：常用舍入模式下该非零错值除以 8 会归零。已增加前置检查及拒绝后数据不变、精确零仍可接受的回归源码；没有执行该用例或增加容差。

## 生产代码和待执行回归源码

| 范围 | 生产代码 | 回归源码与预期覆盖（未执行） |
|---|---|---|
| 聚合 | [adaptive_aggregation.cpp](../../src/experiments/adaptive_aggregation.cpp)、[adaptive_evidence.hpp](../../src/experiments/adaptive_evidence.hpp)、[output_values.cpp](../../src/experiments/output_values.cpp) | [test_r06_adaptive_aggregation.cpp](../../tests/contracts/test_r06_adaptive_aggregation.cpp)：缺原因、重复/错误坐标、帧账本不一致、损坏前缀、拒绝后原状态保持、固定分母及末轮 ratio。 |
| 平滑 | [adaptive.cpp](../../src/analysis/adaptive.cpp) | [test_r06_partial_analysis.cpp](../../tests/integration/test_r06_partial_analysis.cpp)：旧/新格式分别覆盖旋转局部和平移全局窗口的有限输入溢出，以及原帧定位。 |
| SVG | [svg_plot.cpp](../../src/analysis/svg_plot.cpp) | [test_r06_svg_windows.cpp](../../tests/integration/test_r06_svg_windows.cpp)：X/主 Y/右 Y 溢出、前置面板零发布和原数值 JSON 不变。 |
| 运行记录 | [execution_record.cpp](../../src/cli/execution_record.cpp) | [test_r06_execution_records.cpp](../../tests/integration/test_r06_execution_records.cpp)：观察事件与导出记录均保留新政策，旧默认未知信息不被填造，既有故障和输入快照断言保留。 |

聚合新增独立用例 `r06_adaptive_aggregation_evidence`，其余回归沿用既有用例；CMake 和有限子集脚本只更新登记，不执行。所有新增预期都是源码中的断言，**不是本轮已得到的运行结果或检查数量**。

## 静态核对与交付边界

- 阅读现行 AGENTS、PLAN、ROADMAP、最新证据与 PR 身份后实施；已检索仓库全部状态 Issue，既有 #1～#5 与本轮新增问题不同。本轮通过此记录跟踪静态诊断及修复，未新建/关闭 Issue 或发送评论。
- 原子提交按逻辑分开，使用隔离工作树并行；汇总时逐项审阅补丁、接口、调用顺序和错误传播。`git diff --check` 只作为文本差异检查，不记作构建或测试。
- 对新掩码 Writer、RunContext 和 Manifest 发布链的静态复核未确认新缺陷，保持既有代码；不能把这一结果称为真实 MAT 回读或发布故障验证。
- 原 `origin/**` 树保持 `70ee155131c24a50169346d0ffbaff0c01b1c547`。未安装依赖，未改写历史、强推、转 Ready、合并或发布。

**本批状态：代码已提交，构建与测试延期。R6 总体验收未完成。** 目前没有需要所有者立即重新决策的新事项。后续恢复验证时，应针对本次新 SHA 执行聚合、平滑、SVG、运行记录及既有全工程回归；此前测试通过不能代替该工作。完整科学实验/L4、最终容差/性能资格、正式冻结和发布继续延期。UpdateRates 缺少 HistoricalOnly `rottransratio` 的已知限制保持明确错误，不在本轮补造该字段。
