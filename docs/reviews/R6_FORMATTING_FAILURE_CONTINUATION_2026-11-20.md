# R6 停止阈值、输出完整性与诊断接续（2026-11-20）

> **同日后续**：插图法向/光照数值防护及进程记录目录身份修复见 [后续接续](R6_ILLUSTRATION_RECORD_CONTINUATION_2026-11-20.md)。构建、测试仍延期，两批均没有新运行证据。

`[HUMAN-EXECUTE/APPROVE]`。所有者再次要求继续代码实现，既有“推迟构建、测试”继续有效。本批从 Draft [PR #6](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/pull/6) 的 `1ab26a3c25cd320bb43ca6c63af8ad0c6b2b2f21` 接续，沿用 `dev-plan-P0-P1-mat-dictionary-probe`。**只修改生产源码、必要回归源码与状态文档；没有执行配置、编译、编译器语法检查、CTest、测试程序、CLI、MAT I/O 或数值实验。**

前批 RunContext 封存、Reader 身份与轨迹分类见 [发布封存与错误身份接续](R6_LIFECYCLE_DIAGNOSTICS_CONTINUATION_2026-11-20.md)。前后批次均没有新的运行证据，不能继承旧固定 SHA 的通过结论。

## 四类确定缺口与实现

| 边界 | 基线源码的具体问题 | 本批实现 |
|---|---|---|
| KvD 动态停止阈值 | `accuracy_factor` 允许有限正数，但 `accuracy_factor * count` 仍可溢出为 Inf。有限 RMS 除以 Inf 得到零，原有最终有限性检查因此无法发现中间计算失败。 | 在原乘法位置保存并检查分母；非有限时返回 `NonFiniteResult@Solve`，不继续执行除法或把零当作有效阈值。 |
| 内存格式化结果完整性 | 默认 `ostringstream` 的插入操作可能把缓冲区异常转为错误状态而不重新抛出；直接 `success(stream.str())` 或返回片段字符串可能把截断内容当作完整载荷。 | 两个 JSON 序列化入口在返回成功前检查流状态；分析 SVG 的整体及坐标流、插图 SVG 的整体/三角形/轨迹片段流，以及字典指纹流启用错误异常，交给对应失败边界处理。 |
| 分析分段诊断身份 | 四段划分失败直接传播时缺少 ratio 路径/变量；可定位的额外阈值穿越和已声明 ratio 缺测丢失原帧；短旋转平滑段失败也缺少来源。 | 补齐实际 MAT 路径、变量与可以确定的 MATLAB 1-based 帧号；扫描结束后才确认的不完整段界、整体短段不虚构单帧位置。 |
| MAT 暂存哈希失败身份 | `sha256Fd` 失败的 Writer 包装已包含 RunId、变量、Output 阶段和原因链，但没有输出路径；接收文件描述符的内层 helper 也不能补回该路径。 | 外层补入 `outputRoot()/runId`，与相邻暂存回读失败保持同一目录身份。普通/掩码写入共用此处。 |

以上是源码可达路径与失败合同的静态分析，没有执行反例、故障注入或数值复现，不声称默认科学输入曾触发这些边界。

## 输出失败的准确行为

- 数值分析 JSON 流失败返回既有 `UndefinedStatistic@Analyze`；插图 JSON 返回既有 `ResourceLimitExceeded@Analyze`。正常 JSON 的字段、精度、掩码和 `null` 表达不变。
- 分析 SVG 的所有面板仍先准备完毕，再开始调用 Publisher。内存格式化失败会在这一准备阶段退出；`ios_base::failure` 映射为 `WriteFailed@Plot`，已有 `bad_alloc` 和 `length_error` 分类保持。
- 插图仍先完成 JSON 和 SVG 两个载荷，再开始发布。片段或整体 SVG 格式化异常经原 `std::exception` 边界返回 `WriteFailed@Plot`；JSON 的失败继续沿用原数值原因包装。
- 上述发布前保证只针对载荷准备失败。如果 Publisher 自身在后续发布阶段失败或抛异常，已确认的前序文件仍按原协议保留，不声称整个多文件发布具有回滚能力。分析 SVG 的新增流异常消息保持中性，不错误断言失败一定发生于发布前。
- 字典的 FNV-1a 指纹格式化失败由原 `scanFile` 边界记录为该文件的 `ERROR`，不会把不完整指纹认作成功。字典报告仍可按原设计包含失败记录；本次没有将单文件失败改成整份报告停止生成。
- 这些防护不承诺在持续内存耗尽时仍能分配完整错误对象，也没有新增全局分配器、公共故障注入接口或输出 schema。

## 文件与待执行回归

| 实现文件 | 本批回归源码或验证边界 |
|---|---|
| [franz_kvd.cpp](../../src/estimators/franz_kvd.cpp) | [test_r06_kvd_strategy.cpp](../../tests/integration/test_r06_kvd_strategy.cpp)：Original/Modified × trace 开关，正常六方向流与最大有限 accuracy factor；检查首轮失败、错误码/阶段/消息、输入逐位保持，以及同一 engine 恢复参数后可复用。沿用 `r06_kvd_strategy_composition`。 |
| [adaptive.cpp](../../src/analysis/adaptive.cpp) | [test_r06_analysis_segments.cpp](../../tests/integration/test_r06_analysis_segments.cpp)：Comparison/UpdateRates 两入口的不完整段界、额外穿越帧 520，以及短旋转段来源；[test_r06_partial_analysis.cpp](../../tests/integration/test_r06_partial_analysis.cpp)：ratio 声明缺测帧 151 的准确身份。沿用 `r06_analysis_segments_smoothing`、`r06_partial_analysis`。 |
| [dataset_json.cpp](../../src/analysis/dataset_json.cpp)、[svg_plot.cpp](../../src/analysis/svg_plot.cpp) | 新增流失败保护；既有分析与 `r06_svg_source_windows` 场景保留。没有为此新增内存分配故障注入，故新流失败分支没有本批执行覆盖。 |
| [data_json.cpp](../../src/illustrations/data_json.cpp)、[svg.cpp](../../src/illustrations/svg.cpp) | 新增插图 JSON/SVG 完整性保护；既有 `r06_illustrations_independent_geometry` 及 CLI 场景保留。没有本批格式化故障注入执行证据。 |
| [mat_data_dictionary.cpp](../../src/io/mat_data_dictionary.cpp) | 指纹流失败归入原单文件错误记录；不改变 FNV 算法、MAT 解码或报告 schema。未执行字典/分配故障场景。 |
| [run_mat_writer.cpp](../../src/io/run_mat_writer.cpp) | 仅补一行输出目录身份；没有为该诊断扩展共享故障注入器或 CMake，哈希失败分支的新增执行覆盖待后续验证。 |

本批只扩充三个既有测试源码文件，没有新 CMake 目标或测试登记。**测试源码已准备不表示断言执行或通过。** 恢复验证后，应基于新的固定 SHA 核对上述回归、相关输出合同与真实 MAT/CLI 路径；如需针对流/哈希故障新增定点注入，应先按具体未覆盖风险设计，不能用正常输出用例替代故障命中证据。

## 保持的合同与当前状态

KvD 仅检查原求值阶段的中间结果，没有重排公式、修改默认参数、停止条件或科学阈值。分析分段的严格比较、原阈值、窗口和求值次序不变。诊断使用实际已知上下文，没有新增包装层、丢弃原因链或猜测单帧。MAT 发布顺序、无覆盖约束、回执与持久化规则不变。

DEC-R6-01～04、固定八轮分母、末轮 ratio、方案 B 的部分有效数据及全部延期决定继续有效。静态检查包括源码交叉审阅和 Git 文本差异检查；原始 `origin/**` 树保持 `70ee155131c24a50169346d0ffbaff0c01b1c547`。没有运行软件、安装依赖、改写历史、强推、发送评论、改变 Issue 状态、转 Ready、合并或发布。

**本批四类确定源码问题已修复；新代码未构建、未测试，R6 总体验收仍未完成。没有需要所有者立即介入的新决定。** 科学复现、完整数值资格、最终阈值/性能、基线签署与发布继续延期，不设置自动恢复时间。
