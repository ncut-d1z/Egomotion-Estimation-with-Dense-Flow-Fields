# R6 MAT 名称选择、字典指纹与源码范围复核（2026-11-20）

`[HUMAN-EXECUTE/APPROVE]`。按所有者“继续代码实现，推迟构建、测试”，从 Draft [PR #6](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/pull/6) 的 `88f856780e65f726ef4fcf287ef01b7193e40173` 继续。前批见 [输出目录异常所有权记录](R6_DIRECTORY_OWNERSHIP_CONTINUATION_2026-11-20.md)。**本批仅生产源码、必要回归源码和状态文档，未配置、未编译、未做语法编译、未执行测试、CLI、MAT I/O 或数值实验。**

## 1. MAT 原始载荷与后端的变量选择一致性

[mat_payload_validation.cpp](../../src/io/mat_payload_validation.cpp) 原来用原始名字的完整长度和字节判断是否选中。[MatResultReader](../../src/io/mat_result_reader.cpp) 拒绝带 NUL 的请求名称，但文件内的名字由 matio 读取并作为 C 字符串使用。两层对文件内嵌 NUL 的解释因此可能不同。

独立读取 matio **v1.5.26** 的上游源码可确认：

- [mat5.c](https://github.com/tbeu/matio/blob/v1.5.26/src/mat5.c) 的普通/压缩、normal/small 名称路径复制原始字节后补末尾 NUL；文件 blob 为 `fefe4304df6ea53bb455fafa8a83b8dd6160392f`。
- [mat.c](https://github.com/tbeu/matio/blob/v1.5.26/src/mat.c) 的 `Mat_GetDir` 使用 `strdup` 保存目录名，`Mat_VarReadInfo` 使用 `strcmp` 查找；文件 blob 为 `a3e6008ff74eb9a8388a6572db854a9a9a91bd7d`。

静态反例：原始名称为 `value\0suffix`、MATLAB class 为 double、实数载荷为 `miSINGLE`，请求为 `value`。旧预检因全名长度不同把它视为未选中，跳过既有 single 存储拒绝规则；后端却按 `value` 选择并可隐式转换。**本轮没有生成或运行这个反例；上述判断来自两层源码的选择路径。**

现将非空请求的选择改为：原始名称首个 NUL 前的字节与请求完全相同，且有效长度相等；无 NUL 时比较全部名称。仍使用 128 字节固定缓冲，不按文件声明长度分配字符串；首个 NUL 后的原始字节和 padding 仍全部消费、检查。空请求保持全选；不相关的 single 邻居继续不影响具名读取支持类型。没有新增全局拒绝 NUL 的命名政策、支持类型、资源预算或公共 API。

[test_r06_mat_payload.cpp](../../tests/integration/test_r06_mat_payload.cpp) 继续使用既有 `r06_mat_raw_payload_boundaries`，新增 13 个 fixture：NUL 偏移 1、5、127、128、129 各包含合法 double 和被禁止 single；另有不相关名字、请求长于前缀及短于前缀的 single 邻居。覆盖 small/normal 名称及 128 字节比较块边界，检查保留请求名；沿用 `Mat_VarRead` 调用计数断言，在数值分配/解码前拒绝无效存储。既有大小端 × 普通/压缩循环对应 **52 种新增编码组合**，这是源码设计数量，**不是已执行或通过数量**。

## 2. 字典 FNV 指纹不受调用者 locale 影响

[mat_data_dictionary.cpp](../../src/io/mat_data_dictionary.cpp) 的主报告流已有 classic locale，但 `checksum()` 的 FNV64 十六进制流未固定 locale。调用者若设置数字分组，全局 locale 会影响新建流，可能在指纹中插入分隔符，破坏既有 16 位小写零填充格式。

本批在该局部流设置 `std::locale::classic()`，保留原 FNV 算法、异常掩码及单文件失败处理。[test_r06_dictionary_streams.cpp](../../tests/integration/test_r06_dictionary_streams.cpp) 在既有 `r06_dictionary_output_streams` 中新增自定义三位分组 facet，以 `0x1234` 必须产生 `1_234` 确认注入有效；同一四文件输入分别在 classic 和分组 locale 下生成独立报告，比较完整报告字节与 CLI 状态文本，并用 RAII 恢复调用者的原全局 locale。输出文件名不进入报告正文。

这些是两次新增调用的回归源码，未执行；不沿用旧测试的检查数或通过数。未扩展字典报告的并发写入及持久性合同。

## 3. 当前已列明的代码实现范围

按 [ROADMAP 既有代码批次完成定义](../ROADMAP.md)，本轮结合 [PLAN](../PLAN.md)、[D1–D5 任务审计](R6_PLAN_TASK_AUDIT_2026-11-18.md) 和 [迁移矩阵](../MIGRATION_MATRIX.md) 核对生产入口与装配：

| 核对项 | 源码结论与入口 |
|---|---|
| 生产源文件登记 | Git 跟踪的 62 个 `src/*.cpp` 与 [CMakeLists.txt](../../CMakeLists.txt) 的 62 个唯一源路径一致，无遗漏、无不存在项、无重复项；两个 SVG 实现按既有选项启用。 |
| 五个实验入口 | [production_runner.cpp](../../src/experiments/production_runner.cpp) 的两个 Bias 入口装配实际采样/噪声/估计服务，三个 Adaptive 入口调用对应 Runner。 |
| 实际应用和分析 | [application_main.cpp](../../src/cli/application_main.cpp) 装配 Reader、Analyzer、Production Runner 与 CLI；[result_analyzer.cpp](../../src/analysis/result_analyzer.cpp) 分派到真实分析实现。 |
| DEC-R6-01、02 | [source_motion_errors.hpp](../../src/numerics/source_motion_errors.hpp) 的稳定方向角和 SO(3) 测地角由 [adaptive_session.cpp](../../src/estimators/adaptive_session.cpp) 实际调用。 |
| DEC-R6-03、04 | Session 结果进入 Production Runner 和 [run_context.cpp](../../src/experiments/run_context.cpp) 的专用封装、次数和交付路径；[adaptive.cpp](../../src/analysis/adaptive.cpp) 与 [dataset_json.cpp](../../src/analysis/dataset_json.cpp) 保留指标版本及部分有效结果语义。 |

在本次限定审查范围内，没有发现其它确定的未实现入口、无条件成功占位或漏接生产源文件。`BackendUnavailable` 的现有分支用于缺少注入组件或未启用可选 SVG，不是完整生产装配的无条件空壳。迁移矩阵明确排除的原始功能不重新计为当前未实现项。

**当前已列明的 R6 代码范围具备源码级闭合条件；这是实现状态，不是正确性或验收结论。** 历史实验中的 acos 越界和零旋转失败属于旧指标版本；本轮没有重跑实验，不能把旧失败改写为当前已通过。后续真实验证若暴露缺陷，仍须重开相应实现修复。

## 4. 交付、延期与人类决定

本轮检查方式为源码交叉审阅、Git 状态/差异检查及 `git diff --check`；没有运行仓库验证脚本。原始 `origin/**` 树保持 `70ee155131c24a50169346d0ffbaff0c01b1c547`，旧分支、工作树和失败记录保留；两个既有测试目标继续使用原 CMake 登记。

DEC-R6-01～04、方案 B、固定八轮分母和末轮 ratio 保持；**没有新增需要所有者立即决定的事项**。当前源码的构建/测试、完整 Bias/Adaptive 科学实验与必要 L4、独立 L3 的更广覆盖、病态数值/跨环境/性能资格，以及最终容差、冻结、REQ/AC 验收和发布继续按既有决定延期。旧固定 SHA 的通过记录不外推到本批。PR #6 保持 Draft，R6 总体验收仍未完成。
