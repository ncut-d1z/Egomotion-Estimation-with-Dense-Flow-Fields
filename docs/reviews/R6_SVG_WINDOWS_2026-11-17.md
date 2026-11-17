# R6 原脚本视窗与 SVG 几何验证

`[HUMAN-EXECUTE/APPROVE]`：生产修复、构建及CTest已授权；新测试当前仅写源码，尚未运行。关联P9.2/P9.4/P9.6、TC-F-BIAS-003、TC-F-CLI-005、TC-F-AN-002、TC-DC-ANALYSIS-001。此次修复既有源行为，不改变数值公式、CLI/JSON合同、容差或冻结状态。

## 来源与缺口

原 `graphs_bias_test.m` blob `112aa9eca1fa9728f493d31fc8e59620a8445230` 第70/71行固定 xlim=[100,3000000]、ylim=[0.06,200]，两轴log。Comparison分析 blob `3924038f0ccd70de5ea7b60c589afe166da9c8fb` 对旋转/平移两图右侧ratio轴固定[0.01,100]，左侧角度轴仍自动范围，两侧log。UpdateRates不使用上述固定窗口。

当前 `svg_plot.cpp` 自动采用数据极值，且没有viewport clip。范围外数据会改变坐标，而log全零Bias会被拒绝。将新增37号测试 `r06_svg_source_windows`：直接解析实际SVG坐标及clipPath绑定，记录6个独立合成情境的输入/输出JSON和SVG。不会运行原MATLAB、修改归档、运行完整科学实验或安装图形依赖。

## 可独立复核的几何断言

画布绘图区为左95、右1080、上70、下630（现有实现的像素几何，不是科学数值门槛）。Bias的固定窗口两角应为(95,630)、(1080,70)，几何中心输入 `(sqrt(100*3000000),sqrt(0.06*200))` 应为(587.5,350)。Comparison右轴的ratio=0.01/1/100分别对应y=630/350/70；ratio=0.001/1000应保留投影y=770/−70，再由viewport裁剪；不能由这些离群点扩大右轴，也不能影响左轴。

正的越界端点保留在SVG折线上，使穿过窗口的线段仍可见；clipPath仅裁剪显示，不改动AnalysisDataset。零值在log轴断开曲线，不截断为正数；Bias全零仍有明确固定坐标轴，可输出空曲线图。未知来源即使面板同名也不可套用Bias范围；UpdateRates线性零值仍显示。对现有3位小数SVG坐标按上面可表示的像素值精确比较，不新增或放宽DEC-005科学容差。每情境前后dataset JSON必须逐字节相等。

当前只验证源固定窗口和几何/裁剪语义；不据此声称MATLAB后端默认自动刻度、图例绑定、字体或三份illustration视觉等价。每次测试使用新目录且保留失败SVG，原始资料只读。
