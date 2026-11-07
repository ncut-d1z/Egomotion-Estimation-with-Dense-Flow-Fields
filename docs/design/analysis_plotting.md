# MAT 结果分析与可选绘图——详细设计草案

> **LLD v0.1 草案，未冻结、未运行任何可视化程序。** 来源：bias_test/data_analysis/graphs_bias_test.m、adaptive_matched_filter_test/data_analysis/ 下三份 graph_*.m、help_functions/kurveglaetten.m、illustrations/ 下三份 .m。对应 P5.5、P9、REQ-F-BIAS-003、REQ-F-AN-001/002。

## 1. 只读分析合同

| 原分析 | 输入 MAT 字段 | 数值操作与期望数据 |
|---|---|---|
| graphs_bias_test | 每个 1–40 目录 xwerte、ywerte3…6（ywerte1/2 在源脚本中注释） | 按每运行逐数组求平均；理论参考 100/sqrt(xwerte)；取源起始索引 3 及后续范围，log-log 横纵轴 |
| graph_agent_in_sphere_initialization_test | allemittransfehler、allemitrotfehler；也读取 ohne* | mittrans 第 1–13 项与 mitrot 全序列（semilogy）；其他读取字段不意味着都参与该图 |
| graph_agent_in_sphere_adaptive_matched_filter_test | allemittransfehler、allemitrotfehler、alleohnetransfehler、alleohnerotfehler、rottransratio | x=50:550；按 rottransratio <0.01 分段；旋转/平移角度误差与右侧比值双纵轴、对数刻度 |
| graph_agent_in_sphere_adaptive_matched_filter_many_parameter | allemittransfehler、allemitrotfehler、rottransratio | x=50:550；阈值 0.02 分段，四个更新率曲线；平滑窗口半宽 2/5/10 |

kurveglaetten.m 使用窗口内均值；原代码在头尾保留零值。该行为必须先核对，不得为了平滑曲线外观更好而改成延拓/镜像边界。

## 2. 结构化曲线数据（候选接口）

建议 Analyzer 输出包含：
- curve_id、source_mat_path、source_variable、source_sha、source_index_start/end、axis-x、axis-y、axis-unit、raw_values、smoothing_window；
- 对分段值另带 segment_start/end、threshold、invalid_flag，输出明确有效样本数及数据缺失原因；
- 图像后端只消费这些不可变值，不负责重新读取 MAT、选择随机种子或触发求解。

~~~cpp
// 候选 LLD 约束，不是已实现类。
struct CurveSeries;
struct AnalysisDataset;
enum class AnalysisErrorCode;
enum class PlotErrorCode { BackendUnavailable, InvalidSeries, OutputForbidden, WriteFailed };
class BiasResultAnalyzer {
public:
    Result<AnalysisDataset, AnalysisErrorCode> aggregate(
        const std::filesystem::path& read_only_result_root) const;
};
class AdaptiveResultAnalyzer {
public:
    Result<AnalysisDataset, AnalysisErrorCode> analyze(
        ExperimentKind kind, const std::filesystem::path& read_only_result_root) const;
};
class IPlotExporter {
public:
    virtual ~IPlotExporter() = default;
    virtual Status<PlotErrorCode> exportPlot(
        const AnalysisDataset&, const std::filesystem::path& output_file) = 0;
};
~~~

实际签名需在模块间的值对象、异常枚举和 CLI 子命令冻结后确认；IPlotExporter 为可选组件，不应强迫 numerical-only 构建携带绘图库。

## 3. 展示脚本与选择性图形复现

- circle_curve_in_the_sphere：绘制单位球与圆轨迹；
- sinosodial_curve_in_the_sphere：绘制单位球与正弦轨迹；
- spherical_harmonic_illustration：在球面角网格上绘制球谐函数形状。
- 图形几何和曲线数值/范围需与原 MATLAB 相符；无需声称像素级一致。禁止引入 Python/MATLAB Runtime。
- 若无法使用 gnuplot 或其他可选后端，analyze 命令必须仍能完成数值输出，且清楚报告图形不可用状态。

## 4. 验收、Golden 和阻塞

- 每一种输出曲线必须从具体 MAT 变量、原始 MATLAB 索引（1-based）和读出 shape 追踪到 C++ 结构化曲线；逐元素误差与数值统计独立于颜色/图例验证。
- **DEC-004 已确定**：历史归档原样只读读取；新 C++ 输出只生成当前有效 save 字段。对于当前生成流程不写、但历史分析需要的 `rottransratio` 等字段，必须报告 `MissingVariable`（或由 Analyzer 按 contracts 明确映射的非成功）及 `HistoricalOnly` 来源说明，**绝不补零、跨 RunId 借用、推导虚假比值或伪造图像**。详见 [决策](../decisions/DEC-004_HISTORICAL_MAT_FIELDS.md)。
- TC-F-BIAS-003、TC-F-AN-001/002、TC-I-ANALYSIS-001、TC-F-CLI-004/005；当前无 Golden 数值、无签名冻结、无实际 CTest。


## 5. 四种分析入口的可编码索引/统计算法合同

| 来源分析函数 | MATLAB 索引与算法 | 结构化结果（建议） | 特别验证 |
|---|---|---|---|
| graphs_bias_test | 按 k=1..number_of_cores 读取 xwerte 和 ywerte3…6，四组求和再除以个数；从 anfang=3 到 ende=iterationnumber+1；参考线 y=100/sqrt(xwerte) | 每个噪声系数、Original/Modified 的平移/旋转四条原曲线，x=原 xwerte 和理论参考 | 不能把独立运行 40 份混成单次内部循环；ywerte1/2 在分析里被注释 |
| graph_agent_in_sphere_initialization_test | mittrans 的 MATLAB 1..13 行、mitrot 的全部行；semilogy | curve_id、x 的 MATLAB 1-based 下标、y 数组、log-y 语义 | 这两条曲线长度不同，不能强行补齐尾部 |
| graph_agent_in_sphere_adaptive_matched_filter_test | x=50:550（501 点），rottransratio 参与右纵轴与阈值 0.01 分段；绘制 mit/ohne 的旋转及平移误差 | 数值曲线 + 左轴 angle_deg/右轴 ratio、段边界及原始样本 | 不能用 SO(3) 测地角替换原脚本方向角度误差 |
| graph_agent_in_sphere_adaptive_matched_filter_many_parameter | x=50:550，rottransratio 阈值 0.02，四种刷新频率对应的曲线；后 3 组用半窗 2、5、10 平滑 | 每个频率一组 curve、应用的窗口/段范围/非有限标志 | 多更新率当前源码不保存 rottransratio，但**现有历史 MAT 包含**，两种数据来源必须区分 |

### 5.1 分段选择逻辑（保留 MATLAB 规则）

原两个 Adaptive 分析脚本均采用 3 个分段区域的设置和 4 组 begin/end 数组，先设置 begin(1)=50，然后逐个 i=50..550 检查：
- ratio(i) < threshold **且**先前 zone==0：将当前 i 写入 end(j)，j+1，zone←1；
- ratio(i) > threshold **且**先前 zone==1：将当前 i 写入 begin(j)，zone←0；
- 将第四段 end(4)=550；threshold=0.01（comparison）或 0.02（many_parameter）。
- **等于阈值不会触发任一分支**，不能自行改成 ≤/≥。

该脚本假设预期的阈值穿越次数和有效段数；当历史数据不满足条件时，部分 begin/end 可能为零，导致 MATLAB 索引无效。忠实模式需首先记录真实分段和失败行为，不能悄悄使用“自动发现所有片段”的新算法替代作者的四区域逻辑；改进策略属于单独需求。

### 5.2 平滑窗口的完整边界行为

原 kurveglaetten(kurve,fenster)：

\[
y_i=\operatorname{mean}(x_{i-f:i+f}),\quad
i=f+1,\ldots,N-f;
\qquad
y_i=0\text{ （原始 zeros 预分配的其余边缘）}.
\]

这里的 i 是 MATLAB 1-based，f 是**半窗宽**而不是总窗长。分析脚本再对部分段选择 begin+f..end−f 进行绘图。C++ 数值输出要保留原 y 边缘零值和“被实际绘制的有效子段”，不能默默改为镜像延拓、全长卷积或在边缘计算更短窗口。

### 5.3 Bias xwerte 的原版兼容约束

两个 Bias **生成脚本**都写 xwerte(i+1)=6·4^i，然而完整球面生成函数的实际视线数量为 8·4^i。分析脚本对原存 xwerte 求 100/sqrt(xwerte)。因此：
- **原始复现视图**必须使用历史文件的 xwerte 和理论曲线，不把 x 替换成真实 N；
- 如另行输出物理实际样本数，应使用 separate actual_flow_count 元信息及新的、明确标注的图表，不能把新图伪装成原论文结果；
- 需求与结果解释中应记录这一差异，待项目所有者决定是否保留另一种对照图。

## 6. 候选 CurveSeries 输出 schema 与 C++17 错误合同

~~~cpp
// 设计合同，不是已实现类；公共 Result/Error 在 contracts.md。
enum class AxisScale { Linear, Log10 };
struct SeriesProvenance {
    std::string original_mat_relative_path;
    std::string original_mat_variable;
    std::string source_blob_sha;
};
struct CurveSeries {
    SeriesProvenance provenance;
    std::string curve_id;
    std::string x_unit, y_unit;
    AxisScale x_scale, y_scale;
    Eigen::VectorXd x_values, y_values;
    std::size_t source_index_first_matlab;
    std::size_t source_index_last_matlab;
    std::size_t smoothing_half_window;
    std::vector<std::pair<std::size_t, std::size_t>> plotted_segments;
    std::vector<std::size_t> invalid_indices;
};
struct AnalysisDataset { std::vector<CurveSeries> curves; };
enum class AnalysisErrorCode {
    MissingResult, MissingVariable, ShapeMismatch, InvalidSegment,
    NonFiniteInput, UndefinedStatistic, UnsupportedExperiment
};
~~~

前置条件：源目录可只读加载、每条曲线要求的变量存在且形状可核实；任何 1-based→0-based 的转换必须在 Analyzer 内显式处理。后置条件：曲线数据自拥有且可在 MAT 文件关闭后独立消费；错误不得诱发求解器运行或写回源 MAT。

PlotExporter 仅消费 AnalysisDataset，输出图形位置必须在独立 RunId 中且不得覆盖原数据；禁用绘图或没有图形终端时 CLI analyze 仍可返回全部数值与分析状态。

## 7. 对应 Golden、边界数据与测试映射

| 计划 Golden | 输入来源与核验内容 | 对应 TC |
|---|---|---|
| AN-G01 | 40-run 两组 Bias 的 xwerte、ywerte3…6，四组平均和 100/sqrt(x) | TC-F-BIAS-003 |
| AN-G02 | init 的 1..13 与 1..35 原样索引，两个不等长度序列 | TC-F-AN-001 |
| AN-G03 | comparison x=50..550、阈值 0.01、旋转/平移曲线、右轴 ratio 和各分段 | TC-F-AN-001 |
| AN-G04 | many_parameter 的四组曲线、阈值 0.02、窗口 f=2/5/10、分段裁剪 | TC-F-AN-001 |
| AN-G05 | rottransratio 缺失、NaN/Inf、穿越区域不足、f 超过数据长度、MAT 形状错误 | TC-I-ANALYSIS-001、TC-NF-HEADLESS-001 |
| AN-G06 | 单位球圆/正弦轨迹、l≤2 实球谐三种插图数据 | TC-F-AN-002 |

所有参考数据须来自原只读 .m 源公式/历史 MAT L2 及独立 L3 曲线实现，带原源码 SHA、输入 MAT SHA、参考生成过程与明确 Golden 版本；**不要求运行原 MATLAB 分析脚本**。图像像素一致性不要求，但**数据值、MATLAB 索引、绘图分段、单位和有效数量必须可审查**。

**当前状态**：分析的索引、分段和平滑合同已列出；**历史 582 个 MAT 已由 matio 实测 READ_OK，真实形状与类型见下表**。实际曲线数值 Golden、批准容差、绘图后端、C++ 分析程序与 CTest 仍未完成。

## 8. matio 已确认的历史分析输入合同

[MAT_DATA_SCHEMA.md](../MAT_DATA_SCHEMA.md) 是当前作者归档中 MAT 变量名、class/type、真实 dims 和读取状态的唯一事实来源。所有历史文件为 MAT v5/zlib、class=double/data_type=double，**582/582 文件/变量读取成功**；不能因此直接判定曲线计算正确。

| MATLAB 分析脚本 | 历史 MAT 输入变量及真实 shape | 约束 |
|---|---|---|
| graphs_bias_test | 两类归档各 40 运行；每运行 xwerte `10×1`，ywerte1…6 各 `10×3` | 原图分析主要使用 ywerte3…6，且 xwerte 按作者存储值而非实际完整球面 N |
| graph_agent_in_sphere_initialization_test | 四个 allemit/alleohne trans/rot 变量，均 `35×1` | 1..13 与全行索引按源脚本控制，不因 dtype/shape 合法而省略索引测试 |
| graph_agent_in_sphere_adaptive_matched_filter_test | without_errors 的四误差和 rottransratio 均 `600×1`；with_errors 另有 alleadaprot/alleadaptrans，亦 `600×1` | 当前源码不写 `alleadap*`，历史额外数据来源仍需追踪 |
| graph_agent_in_sphere_adaptive_matched_filter_many_parameter | 现存 allemittrans/allemitrot/alleadaptrans/alleadaprot 均 `600×4`；akrot/rottransratio 均 `600×1` | 当前多频率生成脚本只保存两个 allemit 和 akrot；历史 rottransratio 属 HistoricalOnly。新输出进行该图分析时缺少比值即明确失败，不虚构数据；保留旧历史完整分析 |

**DEC-004 的最新所有者决定优先于历史 H-07 的仅起草状态**：当前生成字段正常输出，历史额外字段标 `HistoricalOnly` 并只读保留；未知来源无需先归因。分析需要却缺失的字段明确非成功，不补造/不调整统计口径。三态来源分类可保留 `CurrentGenerated/HistoricalOnly/Unknown`，具体报告类型待 LLD/CTest 验证。

后续验收要分别进行：(1) `Mat_VarRead` 正常读取、rank/dims 解析（**已有独立探针证据**）；(2) 业务 Reader 将 MAT 列主序数据正确映射到分析数组；(3) 分段、窗口、指标及数值曲线与从 L2 MAT 依据原静态索引/公式**独立重算的 L3 数值参考**逐项相符；(4) 可选绘图输出。只有 (1) 有实际完成证据，其余仍未实施。

## 附录 A：本模块无需运行 MATLAB 的验收参考

按 [长期无需 MATLAB 执行政策](../golden/NO_MATLAB_EXECUTION_POLICY.md) 与 [L2/L3/L4 来源矩阵](../golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md)：原 MATLAB .m 文件仅作为**只读公式、数据布局和顺序规范**；**不要求执行 MATLAB/Octave、采集 L1 或以缺少 L1 阻塞冻结/编码/验收**。

采用原始 MAT 的 L2 列主序数值作为只读输入，独立生成 L3 begin/end/zone、平滑数组、曲线/分段和 L4 统计；不能从原绘图脚本取得的 L1 中间数组**不是必需取证项**。不要求执行 graph_*.m；实际独立分析器、参考 SHA、批准容差和真实 CTest 尚未完成。

本附录不取消真实 C++/CTest、Golden 独立复核、DEC-005/008 以及源数据保护门禁；当前无新的软件测试通过证据。
