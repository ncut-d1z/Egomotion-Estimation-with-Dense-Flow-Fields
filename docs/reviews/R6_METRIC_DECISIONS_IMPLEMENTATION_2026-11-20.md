# R6 指标决定与部分有效结果实现（2026-11-20）

`[HUMAN-EXECUTE/APPROVE]`。依据项目所有者在当前会话明确确认的 DEC-R6-01～04 和“继续代码实现”，在 Draft [PR #6](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/pull/6) 的现有开发分支接续。起点为 `8f6d46c0fb98ca4b45304bc86876ec35376778bb`；决定记录见 [2026-11-20 所有者决定](../decisions/PROJECT_OWNER_DECISIONS_2026-11-20.md)。本记录不代替 R6 总体验收或正式冻结。

## 已实现的合同

| 决定 | 生产实现 | 数值与兼容性边界 |
|---|---|---|
| DEC-R6-01 | `sourceDirectionErrorDegrees` 对有限非零向量先按最大分量缩放，再用 `atan2(norm(cross(u,v)),dot(u,v))` | 保留零向量无定义；不再因 `acos` 端点舍入越界中断。Bias `normalizedVectorError` 未改。 |
| DEC-R6-02 | 将每帧弧度旋转向量变换为矩阵，计算 `Exp(truth)^T Exp(estimate)` 的 SO(3) 主测地角，输出度 | 零向量为单位旋转；相同旋转轴而幅值不同不再被误判零误差。**这是经批准的指标变更，不是历史旋转轴方向角等价实现。** |
| DEC-R6-03 | `AdaptiveFrameResult` 的平移/旋转指标分别返回 `Result`；纯指标无定义仍完成主求解并提交有效状态 | 策略失败、比较/主求解失败、非法输入/状态、资源失败仍停止并回滚该帧，不跳过失败帧。 |
| DEC-R6-04(B) | 保存部分有效字段、逐轮原值、缺测坐标/原因和有效重复次数；完整求解但统计缺测的交付状态为 `Incomplete` | 未采用方案 A。八轮误差均值只在八次均有效时产生，任一次缺测即保留 NaN；绝不改除数。 |

### 数据发布

原当前 save 变量名、形状、列主序、35/600 帧、8 次重复和首尾结构零保持；没有新增 HistoricalOnly 字段。每次新 Adaptive 输出还包含：

- `egomotion_metric_policy.mat`：变量同名，`1×4`，值为 `[1,1,1,planned_repetitions]`。它先于其它字段发布，防止中途失败的新目录被误认作历史结果。
- 每个原字段的 `valid_<source>.mat`：形状与原字段相同，值为有效重复次数。首尾结构零计数为 0；它们不是有效观测，也不是缺测。Comparison 的四个误差字段内部次数为 0～8，`rottransratio` 保持末轮值及 0/1 次数；UpdateRates 的 `akrot` 为 0/1。
- `egomotion-run-manifest-v2`：含指标公式/单位、固定分母、计算/统计完整性、每个聚合字段的次数与值、每轮原值，以及按 MATLAB 1-based 轮/帧/列定位的完整缺测原因链。JSON 只用 `null` 表示缺测，禁止 bare NaN/Inf。

原未掩码 Writer 仍严格拒绝非有限值；新掩码入口只接受 `defined=1` 的有限值与 `defined=0` 的 NaN，任何 Inf、形状/长度错误或矛盾掩码均拒绝。新入口复用既有安全发布、回读、摘要与持久化确认流程。Adaptive 的旧 Context 发布接口显式拒绝，不能绕过政策和证据。原始 MAT 均只读。

只有全部求解轮次完成、逐轮证据与聚合值/次数一致、没有求解失败、全部预期 MAT 回执及 Manifest 发布成功，才返回交付成功。统计完整返回 `Complete`；部分指标无定义返回 `Incomplete`。**CLI 退出 0 表示交付成功，不能单独当作统计完整；应读取状态和 Manifest。** 求解失败/输出失败仍分别返回 5/6，求解失败不会通过方案 B 伪装为完整求解轨迹。

### 分析与图形

Analyzer 先读取政策 marker，仅确实缺文件才按历史严格有限值路径解释。未知版本、错误重复次数、损坏 marker、缺计数、计数与 NaN 不一致均明确拒绝。新分析输出为 `egomotion-analysis-v2`，保留指标语义、有效标志、结构端点和输入来源；缺测为 `null`。SVG 对声明缺测断线，无声明 NaN/Inf 仍失败。平滑窗口含任一缺测时整窗缺测，不重加权、不缩小窗口。

UpdateRates 原分析需要但当前 save 未包含的 `rottransratio` 仍是已知 HistoricalOnly 限制：新结果直接执行该原分析会明确报告缺变量，不补造科学字段；Comparison 已声明缺测的 ratio 若阻止原阈值分段，也明确失败。

## 验证状态

源代码与测试源码已静态核对，实际执行结果将在完成远程检查点后登记。当前工作区可见 C++ 编译器及已有 Eigen 头文件；未发现 CMake、matio 或 Sophus 的可用开发依赖。没有安装或下载依赖，也没有用替身替换依赖来声称全工程通过。

计划在现有工具范围执行独立指标、Session、部分有效聚合/Manifest、Analyzer/SVG 的有限测试；不把这些结果标为全工程 CMake/CTest 或原科学实验复现。此前固定 `3985b056...` 的 ON 73/73、OFF 72/72 仅是旧版本证据，本轮不继承。

## 延期项与后续边界

所有其它可推迟决定按所有者要求继续延期；当前四项无需重复批准。完整科学实验/L4、病态与跨环境资格、最终容差/性能决定、基线签字及 Draft 转 Ready/合并/发布均未执行或宣称完成。完整构建验证需要已有合适依赖的环境，或另行决定是否安装依赖；在此之前先保留源码、有限验证及具体限制，不以未知结果填“通过”。
