# R6 原脚本视窗与 SVG 几何验证

`[HUMAN-EXECUTE/APPROVE]`：生产修复、构建及CTest已授权；首轮已编译，36/37 CTest通过，新测试确认25处几何/裁剪断言失败；修复待新检查点复测。关联P9.2/P9.4/P9.6、TC-F-BIAS-003、TC-F-CLI-005、TC-F-AN-002、TC-DC-ANALYSIS-001。此次修复既有源行为，不改变数值公式、CLI/JSON合同、容差或冻结状态。

## 来源与缺口

原 `graphs_bias_test.m` blob `112aa9eca1fa9728f493d31fc8e59620a8445230` 第70/71行固定 xlim=[100,3000000]、ylim=[0.06,200]，两轴log。Comparison分析 blob `3924038f0ccd70de5ea7b60c589afe166da9c8fb` 对旋转/平移两图右侧ratio轴固定[0.01,100]，左侧角度轴仍自动范围，两侧log。UpdateRates不使用上述固定窗口。

首轮 `svg_plot.cpp` 自动采用数据极值，且没有viewport clip。范围外数据会改变坐标，而log全零Bias会被拒绝。新增第37项测试 `r06_svg_source_windows`：直接解析实际SVG坐标及clipPath绑定，记录6个独立合成情境的输入/输出JSON和SVG。不运行原MATLAB、修改归档、运行完整科学实验或安装图形依赖。

## 可独立复核的几何断言

画布绘图区为左95、右1080、上70、下630（现有实现的像素几何，不是科学数值门槛）。Bias的固定窗口两角应为(95,630)、(1080,70)，几何中心输入 `(sqrt(100*3000000),sqrt(0.06*200))` 应为(587.5,350)。Comparison右轴的ratio=0.01/1/100分别对应y=630/350/70；ratio=0.001/1000应保留投影y=770/−70，再由viewport裁剪；不能由这些离群点扩大右轴，也不能影响左轴。

正的越界端点保留在SVG折线上，使穿过窗口的线段仍可见；clipPath仅裁剪显示，不改动AnalysisDataset。零值在log轴断开曲线，不截断为正数；Bias全零仍有明确固定坐标轴，可输出空曲线图。未知来源即使面板同名也不可套用Bias范围；UpdateRates线性零值仍显示。对现有3位小数SVG坐标按上面可表示的像素值精确比较，不新增或放宽DEC-005科学容差。每情境前后dataset JSON必须逐字节相等。

当前只验证源固定窗口和几何/裁剪语义；不据此声称MATLAB后端默认自动刻度、图例绑定、字体或三份illustration视觉等价。每次测试使用新目录且保留失败SVG，原始资料只读。

## R6-SVG-001 首轮实测与修复

分支 `dev-plan-P0-P1-mat-dictionary-probe`，受测SHA `80ae1acf7caf083fa97aeff76f4f228a56f6dc03` 已提交/推送并经GitHub远端核实。RunId `r6-svg-baseline-20261117-9zMh13gd`，日志位于 `build/<RunId>/evidence/`，源外增量构建目录 `build/r6-20261117-KzjFHPDQ/cmake`。Ubuntu24.04.5/GCC16.2.0/CMake4.3.4/Eigen3.4.0/matio1.5.26，C++17 Release；固定合成输入无算法随机数，隔离目录独立随机命名。

2026-11-17 UTC+08:00：配置19:02:56…19:02:58、编译19:02:58…19:03:38均exit0，无编译诊断；MAT测试19:03:38…19:03:40为5/5、exit0；其余19:03:40…19:03:44为31/32、exit8。新SVG测试60项断言中25项失败；数字数据保持不变、零值断线、未知来源自动缩放及UpdateRates线性坐标本身通过。全零Bias不产生图，另外五个情境实际SVG保留于 `build/r6-20261117-KzjFHPDQ/cmake/r06_svg_windows/svg-windows-uPwJRF/`。原始/测试输入完整性exit0。

MAT stdout SHA-256 `2c9b6336cd04ea3f9ec8683715abf6d656b3f520ac6dfcfc6065cbf4884453bd`；其余stdout `f3072ab7436dc6eaf819fe27d6ad8410fd31eabde790d6e627e3bc523d78e6db`。2026-11-17 GitHub all-state Issue/PR检索为空；以本文R6-SVG-001记录复现→诊断→修复→复测，不创建/关闭Issue/PR。

修复在内部Exporter按已知原分析blob及面板应用上述固定log范围，不新增JSON/公共接口字段；所有曲线绑定viewport clipPath，有限越界点及零值断线规则保留。固定Bias轴在全零情况下仍成立，允许正确的空图，不伪造正值。下一轮必须在新远程SHA下复测全部37项；保留首次失败断言，不改变参考坐标。
