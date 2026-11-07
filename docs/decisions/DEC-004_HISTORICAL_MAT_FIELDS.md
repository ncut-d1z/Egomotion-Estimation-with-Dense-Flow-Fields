# DEC-004：历史 MAT 额外字段与当前 C++ 生成结果的处理规则

> **已由项目所有者明确选择的设计规则，尚未实际实现或测试。** 与 [SRS](../REQUIREMENTS.md)、[实验 LLD](../design/experiments.md)、[分析 LLD](../design/analysis_plotting.md)、[TEST_PLAN](../TEST_PLAN.md) 及现有 582 个只读 MAT 的格式审计对应。该规则**不要求运行 MATLAB/Octave**、不改变 `origin/**`、不新增必须生成的虚假字段。

## 1. 决定（不再作为待选方案）

1. **新 C++ Runner 只保存当前原始 MATLAB 数据生成脚本的实际有效 `save` 字段**；不以“历史归档中曾经存在”为理由增写字段。保留原算法的参数、shape、循环、八次平均及刷新时序。
2. **历史 MAT 完整保留、原样只读**，Reader/Analyzer 可照原有文件、字段名及 shape 读取，也可继续按证据等级 L2 使用它们。
3. 当前有效 `save` 未产生、但历史 MAT 已存在的字段，统一标记 **`HistoricalOnly` /「历史存在、当前生成流程不产生」**。该标签是来源分类，不是虚构新 MAT 变量、数据修复或软件错误。
4. **分析器按所选数据源判断必需字段**：对历史归档正常存在的数据按 L2 读取；对新的 C++ 运行结果，如果某个分析入口需要 `rottransratio` 等当前并不保存的字段，应明确返回**缺失字段**及源类别和可用变量清单，不能猜测、补零、拼接其他运行的数据，不能从其它误差数组伪造 `rottransratio`。
5. 生成新的字段缺失报告和来源差异报告允许，但**不改变原作者 MAT 内容或在 MAT 文件内添加兼容伪字段**。未识别历史字段的实际来源**不再构成继续实现的阻塞**；如以后发现可复核的源版本再追加证据，但不追溯改造历史 MAT。
6. **验收必须分开**：当前 C++ 保存合同（L3 固定输入与真实 Writer/CTest）和历史 MAT 读取/分析合同（L2 与 L3 独立分析），不得将“新输出不含历史额外字段”判为失败，也不得将“旧字段存在”误认为当前脚本已保存。

## 2. 已知字段与数据场景

| 场景 | 当前生成脚本保存 | 历史归档额外字段 | C++ 输出和分析策略 |
|---|---|---|---|
| `adaptive comparison / without_errors` | 四类方向误差 + `rottransratio`，共五类 | 无由现有审计确认的额外字段 | 按当前五类输出，读取现存历史五类 |
| `adaptive comparison / with_errors` | 同上，当前保存五类 | `alleadaprotfehler`、`alleadaptransfehler`（各 600×1） | 仅输出当前五类；历史七类保留，额外二类标 HistoricalOnly |
| `adaptive rates / many_parameter` | `allemittransfehler`、`allemitrotfehler`（各 600×4）与 `akrot`（600×1），共三类 | `alleadaptransfehler`、`alleadaprotfehler`（各 600×4）及 `rottransratio`（600×1） | 仅输出三类；历史六类原样读；对新结果要求 `rottransratio` 的历史分析路径**明确报告 MissingVariable/数据不可用**，不得生成虚假比值 |
| `adaptive initialization`、两类 Bias | 以现有各生成脚本的有效 save 合同为准 | 未确认以上同类差异 | 保持现有输出合同，不从其他实验套用缺失/附加字段 |

> **证据来源与边界**：当前脚本 `save` 集合由 `origin/**/data_generation/*.m` 的静态文本核对，历史 582 个 MAT 的变量与 shape 来自过去的独立 matio 探针。**没有证据证明**历史额外字段的生成脚本、提交版本或加工来源，因此不对成因作确定判断。

## 3. 实施接口与测试（未执行）

- 元数据中用只读的 `FieldProvenance`（候选：`CurrentGenerated` / `HistoricalOnly` / `Unknown`）和 `MissingField` 报告：列出 `experiment_kind`、数据来源、`required_variable`、可用字段、适用的原脚本 blob SHA/历史 MAT SHA（能取得时）；**该枚举只是文档元数据候选，不要求存入原 MAT**。
- **错误归属**：真正在本入口必需的分析输入缺失，沿用公共 Reader 的 `MatErrorCode::MissingVariable` 或 Analyzer 现有显式失败映射（最终形式见 `contracts.md`/DEC-008），不擅自新增新的 CLI 退出码。
- **测试**：TC-F-EXP-002/003、TC-D-MAT-003、TC-F-AN-001/002、TC-I-ANALYSIS-001。至少包含“旧 seven/six 正常读取”、“当前 five/three 新结果不多写”、“新 rates 分析要求缺失 `rottransratio` 时明确失败”的独立 L2/L3 用例。所有测试仍需未来授权与真实 C++ 实现，不以本记录宣称 PASS。
- **文档状态**：DEC-004 **设计决定已选定**；代码/数值/端到端验收仍未完成。该决定不授权 MATLAB/Octave 运行，不恢复 L1 门禁，不自动允许 Bash/CMake/CTest/MAT 写入或合并发布。
