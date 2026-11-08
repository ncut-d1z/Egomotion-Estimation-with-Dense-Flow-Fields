# CLI 参数、退出码与诊断协议：详细设计草案

> **候选 v0.6，未冻结、未编码、未执行 CTest。** 对应 REQ-F-CLI-001～005、REQ-I-CLI-001、REQ-I-RESULT-001 和 P0.8/P1.2。来源为 [SRS §6.1](../REQUIREMENTS.md)、[实验 LLD](experiments.md) 与 [跨模块错误合同](contracts.md)。本文是设计评审材料，不代替所有者批准。

## 1. 语法与基本约束

- 进程入口：`egomotion COMMAND [OPTIONS]`。`egomotion --help`、`egomotion COMMAND --help` 和 `egomotion --version` 不读取 MAT、不执行算法、退出 0。
- 未知命令/选项、缺参数、单值选项重复、非法枚举/整数/浮点数都退出 **2**，不采取“最后值胜出”。
- **写出根路径为固定内部规则**：本仓库所有新输出只能位于仓库根 `output/<RunId>/`，**CLI 不提供任何自定义输出根选项**。有新输出的命令可以指定 `--run-id ID`；缺省时由与实验 RNG **完全独立**的操作系统随机源创建 128 位随机身份，建议格式 `run_<32位十六进制>`，目录以独占/不覆盖方式创建；若无法取得安全随机值/独占目录则明确失败，不复用旧 RunId。不含新输出的 `inspect-mat`、纯数值 `analyze` 不强制生成 RunId。用户指定 ID 的候选校验为 1–64 个 ASCII 字母/数字/下划线/连字符，路径分隔、回溯和绝对路径一律拒绝。原始 `origin/**` 永远只读，`.gitignore` 的 `/output/` 不替代文件系统安全检查。
- 解析时不改变 MATLAB 原值：35/600 点、二十面体细分级别、8 次比较、四个更新间隔 1/5/11/21、噪声乘数不擅自改变。
- 路径保留 Debian/Ubuntu 的 UTF-8/中文/空格语义；不把路径拼接结果当成可靠的防符号链接绕过实现。

## 2. 命令级输入合同

| 命令 | 必需选项 | 可选选项 | 已知输出、选择与来源边界 |
|---|---|---|---|
| `inspect-mat` | `--file PATH` | `--variable NAME`、`--mode strict/compat`（默认 strict） | 未指定变量时仅列举元数据；strict+NAME 指定变量；compat 且不带 NAME 时模拟原 getflowfield 读首变量。compat+NAME 视为参数错误；**单变量文件**按现有归档合同读唯一变量，**多变量且没有获批准的独立顺序依据**返回 `MatErrorCode::AmbiguousVariable`，不采用字母序猜测；完整边界见 [mat_io.md §7.4](mat_io.md)，无需 MATLAB 运行 |
| `run-bias-occluded` | `--seed UINT32 --level NONNEG_INT` | `--run-id ID` | 使用缺损八面体、平均光流模长噪声；level 为包含端点的最大递归级别；写 xwerte 和 ywerte1…6 |
| `run-bias-proportional` | 同上 | `--run-id ID` | 使用完整八面体和逐分量噪声；保留原作者 xwerte 与真实 N 不同的行为 |
| `run-adaptive-init` | 无 | `--run-id ID` | 35 点、level=2；四类初始化误差 MAT |
| `run-adaptive-comparison` | `--noise-scale FINITE_DOUBLE` | `--run-id ID` | 600 点、level=4、8 次重复；含噪/无噪比较及 rottransratio，当前五类 MAT；噪声为 0 时仍消耗原有 randn |
| `run-adaptive-update-rates` | 无 | `--run-id ID` | 600 点、level=2，周期固定 1/5/11/21；当前脚本只写三个 MAT，不伪造历史额外数据 |
| `analyze` | `--experiment bias/adaptive-init/adaptive-comparison/adaptive-update-rates --input-root DIR` | 无 | 四种分析路径；只读取 MAT，不调用求解器；缺少字段显式失败 |
| `plot`（S） | 同 analyze | `--run-id ID`（仅当生成持久图像文件时适用） | 绘图后端可缺席（exit=7），不得影响数值与分析命令 |

### 2.1 参数类型与不变量（待 P0.8 签署）

| 类型 | 拟议的精确校验 | 待决定 |
|---|---|---|
| UINT32 | 完整十进制输入，0 至 4294967295；禁止负数、溢出和尾随字符 | 固定 seed 的跨 MATLAB/C++ 序列不承诺相同 |
| NONNEG_INT | 完整十进制非负整数；先检查 4 的幂、样本数量及分配溢出 | 不凭空设定人为最大 level/实验规模 |
| FINITE_DOUBLE | C locale 完整解析，拒绝 NaN、Inf、尾随内容 | 负噪声系数的 MATLAB 语义及是否接收需批准 |
| ID | 用户指定的 RunId 为 1–64 个 ASCII 字母/数字/下划线/连字符；拒绝点、空格、路径分隔与回溯 | 缺省由 OS 独立随机源生成 `run_<128bit十六进制>`，不消费实验 RNG；目录和文件独占创建，不覆盖。实现待验证 |
| 固定输出根（内部，不是 CLI 参数） | 唯一根 `<repo>/output/`，任何新结果仅能使用全新的 `output/<RunId>/` | 由 Composition Root/RunContext/Writer 分层校验，独占创建与 TOCTOU 测试尚未执行 |
| PATH / DIR（仅输入路径） | 单个非空输入实参，解析实际路径；不用于指定新结果输出根 | 输入保持只读；符号链接/TOCTOU 仍需安全负例验证 |

## 3. 退出码、错误阶段与错误优先级（候选）

**本表是面向命令行使用者的摘要，不是独立枚举映射表。** `MatErrorCode` / `NumericalErrorCode` / `ExperimentErrorCode` / **`AnalysisErrorCode` / `PlotErrorCode`** 到退出码的唯一候选来源是 [contracts.md §3.1](contracts.md)，不得从本表反向定义另一套错误编码；两表不一致时先记录待审差异，不擅自修改已批准的 SRS 义务。

| exit | 类别 | 示例 |
|---:|---|---|
| 0 | 成功 | help、version 或正常完成 |
| 2 | 参数错误 | 未知/重复/缺少参数、非法枚举、整数格式 |
| 3 | 输入或 MAT 读取错误 | 文件不存在/拒绝访问、坏 MAT 头、损坏压缩块、缺变量（含 Analyzer 发现必需的 rottransratio 缺失）、兼容多变量顺序不明（AmbiguousVariable） |
| 4 | 变量类型/布局错误 | 不支持的 MAT class/data_type、rank/dims 不匹配、分析变量 ShapeMismatch、长度溢出 |
| 5 | 数值、实验或分析失败 | SingularSystem、NonFiniteResult、SolverFailed、IncompleteRun；Analyzer 的 InvalidSegment / UndefinedStatistic / NonFiniteInput / UnsupportedExperiment |
| 6 | 输出路径/结果安全冲突 | origin 路径、已有 RunId、写失败、独立回读不一致 |
| 7 | 可选功能不可用 | 仅 `plot` 的 `PlotErrorCode::BackendUnavailable`；无效曲线或图表写入失败不属于此类 |

**可选绘图的错误边界（AF-084）**：`IPlotExporter` 的 `PlotErrorCode` 四项由 [分析 LLD §2](analysis_plotting.md) 定义，仅 [contracts §3.1](contracts.md) 决定对外 exit/stage。缺后端、无效曲线、禁止输出和写图失败不得一概报 exit 7；`analyze` 在无绘图后端时仍必须可返回数值。文件系统拒绝/写失败不应破坏原始 MAT 或前一步已产生的只读分析数据。全部为候选，不是实际 CTest 结果。

**分析路径跨层区分（AF-077）**：无效 `--experiment` 由 CLI parse 阶段拒绝（exit 2）；合法命令但缺少必需的分析输入/变量属 read(3)、shape 错误属 decode(4)，读入成功后段界/统计/非有限属 analyze(5)，严格按 [contracts §3.1](contracts.md) 的同一候选表，保留底层源错误身份，不能只按枚举同名就随意转换。

阶段检查顺序为 **parse（2）→ input/read（3）→ decode/schema（4）→ output safety（6）→ solve/analyze（5）→ plot capability（7）**。如果一个请求包含多个问题，报告实际先触发的失败，保留原始错误码及上下文。具体错误枚举映射直接引用 [contracts.md §3.1](contracts.md) 的候选表，待 P0.8/P1.2 人类批准后冻结。

失败诊断最少包含稳定的 `error_code`、`exit_code`、`stage`、`message`；涉及 MAT 需附 `path` 与已知 `variable`，涉及实验需附 RunId、算法与样本下标。正常输出使用 stdout、错误使用 stderr，文件里仍保存原 MATLAB 兼容 MAT。**现行设计已选 JSONL 为结构化诊断格式**：一行一个 UTF-8 JSON 对象，至少有 `stage`、`error_code`、`exit_code`、`message`，可选 `run_id/path/variable/observation_index/source_error_code`；stage 由 `contracts.md` 的稳定 `ErrorStage` 枚举序列化，字符串须正确转义，非有限数值不得输出不合法 JSON。无需大型 JSON 依赖，可实现小型可靠序列化器。**当前只有方案，未创建程序或 JSONL 测试**。

**最新项目所有者决定**：CLI 输出根选项已明确删除；固定 `output/<RunId>/`、独立 RunId、JSONL、稳定错误阶段和轻量封装 Result 为已选择的设计方向（见 [最新决定](../decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md)）。其余 CLI 具体错误码与完整 CTest identity 仍属于 P0.8/DEC-008 的未冻结接口细节；本轮不执行任何命令，也不能把候选接口称为已测试。

### 3.1 第一轮 CLI 冻结前边界矩阵（AF-041，部分文档工作）

| 输入/情境 | 已确定或当前候选 | 未完成的决定/证据 |
|---|---|---|
| `--noise-scale` | `FINITE_DOUBLE` 拒绝 NaN/Inf/尾随内容；零噪声**仍按原源消耗 RNG** | 负系数在原 MATLAB 语义下应接收还是拒绝，需独立源码/设计批准与负例测试，不能无依据更改 |
| `--seed` / `--level` | `UINT32`、`NONNEG_INT` 精确解析；不凭空设置人为规模上限 | 端值、溢出、资源失败状态与异常回退需实际测试 |
| 手工/自动 RunId | 手工 ASCII 白名单；自动 OS 独立 128 bit、不得消耗实验 RNG；新目录独占，不复用/覆盖 | CSPRNG 失败、重复 RunId、软链接/TOCTOU、路径和回读负例待验证 |
| 错误与退出码 | 唯一映射在 [contracts.md §3.1](contracts.md)，检查顺序 parse→read→decode→output→solve/analyze→plot | 多错并发出现时先到达阶段、跨层 code/stage 保留与所需退出码需 CTest 确认 |
| 诊断 JSONL | UTF-8 每行对象、稳定字段、转义引号/反斜杠/控制字符、非有限数值不输出非法 token | 无有效 RunId 时字段省略/空值约定、stderr 与文件日志失败后的状态需批准与真实测试 |

这些是冻结前核对项，**不授权**实现/执行；DEC-008/P0.8/P1.2 尚未批准，`AF-041` 保持「未整改」。

### 3.2 负噪声尺度的原始源码静态边界（AF-041，非批准）

只读原 `help_functions/nonpropfehler.m` 中，每个视线对 p 的三个分量**分别**执行 `randn * fehlerprozent * durchlaenge`，随后将 p 投影回切平面；`agent_in_sphere_adaptive_matched_filter_test.m` 仅将 `addederror` 传入该系数，没有显式的“必须非负”分支。因此：

- `--noise-scale 0` 的数值噪声贡献为零，但 **3N 次正态随机抽样仍发生**；负有限系数以负号乘相同的随机抽样，会改变**固定注入序列**的具体噪声符号，不能为了统计分布相似而默默转成绝对值。
- **候选忠实行为**：`FINITE_DOUBLE` 校验有限性，不额外对负号作非负拒绝；如将来选择“非负业务限制”，必须作为经 DEC-008 明确批准的外部语义变更，记录相应 TC 差异，不能声称来自原 `.m`。
- 必须用零/正/负尺度、固定 3N 抽样、精确切向投影及被拒绝的 NaN/Inf 负例验证 CLI/噪声层；当前没有获得正式接口批准，也没有代码/CTest/独立 L3 执行。参见 [采样 LLD §7.2](sampling_noise.md) 与 [静态证明记录](../reviews/AN_G_STATIC_FORMULA_REVIEW_2026-11-08.md)。

## 4. 验收映射与冻结界线

| TC | 必要测试 | 证据状态 |
|---|---|---|
| TC-F-CLI-001 | 有效/无效 MAT、变量、strict/compat、源哈希不变 | 只有独立 matio 探针 582/582 读取的子项证据 |
| TC-F-CLI-002 | 两类 Bias、seed/level、输出安全与变量形状 | 未编写/未运行 |
| TC-F-CLI-003 | 三种 Adaptive、noise-scale、原更新间隔与当前写出的 MAT | 未编写/未运行 |
| TC-F-CLI-004 | analyze 四条路径独立运行、不触发估计器、缺字段失败 | 未编写/未运行 |
| TC-F-CLI-005 | 缺绘图后端 exit=7，核心命令不受影响 | 未编写/未运行 |
| TC-I-CLI-001 | 帮助、必填与默认值、错误优先级和诊断字段 | 未编写/未运行 |
| TC-I-RESULT-001 | RunId 不覆盖、变量名/类型/形状、新 MAT 回读 | 未编写/未运行 |

**冻结前必须完成**：项目所有者批准该候选 SHA 与待决定项，SRS/HLD/contracts 同步确认；P1.2 有实际 CLI 代码及 tests/integration/test_cli.cpp/CTest 证据；输出路径负例与原始 MAT 哈希核验。没有这些证据时本文继续是设计草案。
