# MAT 结果分析与可选绘图——详细设计草案

> **未冻结、未运行任何可视化程序。** 来源：bias_test/data_analysis/graphs_bias_test.m、adaptive_matched_filter_test/data_analysis/ 下三份 graph_*.m、help_functions/kurveglaetten.m、illustrations/ 下三份 .m。对应 P5.5、P9、REQ-F-BIAS-003、REQ-F-AN-001/002。

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
struct AnalysisError;
class BiasResultAnalyzer {
public:
    Result<AnalysisDataset, AnalysisError> aggregate(
        const std::filesystem::path& read_only_result_root) const;
};
class AdaptiveResultAnalyzer {
public:
    Result<AnalysisDataset, AnalysisError> analyze(
        ExperimentKind kind, const std::filesystem::path& read_only_result_root) const;
};
class IPlotExporter {
public:
    virtual ~IPlotExporter() = default;
    virtual Result<void, PlotError> exportPlot(
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
- 对缺失 rottransratio 或历史额外 MAT 的情况，报告缺字段及其来源，不为了生成图像而伪造比值。
- TC-F-BIAS-003、TC-F-AN-001/002、TC-I-ANALYSIS-001、TC-F-CLI-004/005；当前无 Golden 数值、无签名冻结、无实际 CTest。
