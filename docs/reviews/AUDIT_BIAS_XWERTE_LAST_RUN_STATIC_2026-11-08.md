# Bias 历史分析：xwerte 取末运行、四组误差取跨运行平均（静态审计，2026-11-08）

**固定输入 commit**：`19157a21221a8fde4447e3c96ff97a8da1a5746f`。原始只读源 `origin/egomotion_estimation_dense_flow_fields_matlab_scripts/bias_test/data_analysis/graphs_bias_test.m`，Git blob SHA `112aa9eca1fa9728f493d31fc8e59620a8445230`。仅对源码表达式和计划合同进行核对，不运行 MATLAB/C++、不读取 MAT 逐值、不生成独立 L3/CTest。

## 原函数赋值次序

- 在 `for k=von:bis` （`von=1`、`bis=number_of_cores`）内，作者每次执行 `xwerte=getflowfield([str 'xwerte'])`，该赋值**覆盖**先前的 xwerte。
- 同一循环中 `allywerte3…6=allywerte3…6+ywerte3…6`；循环后 `anzahl=(bis-von)+1`，**只有这四个 y 矩阵**除以 `anzahl`。源代码没有聚合 `xwerte`，也不比较不同运行的 x。
- `anfang=3`、`ende=iterationnumber+1`；原 `ygross(i)=100/sqrt(xwerte(i))` 且所有 loglog 横轴使用最后一次读取的 `xwerte`。若原数据正常且每次 x 一样，数值上难以察觉此更新次序差异。

## 不能通过正常一致数据辨别的负例

设人工 K=2，`x^(1)≠x^(2)`、`Y_3^(1)≠Y_3^(2)`，各数组具有源要求的相同 shape。**静态预期**为末轮 `x_plot=x^(2)`，四组 `Ybar_j=(Y_j^(1)+Y_j^(2))/2`，理论线取 `x^(2)`；不是 `mean(x^(1),x^(2))`。不同 x 为**特意构造的合同区分性负例**，不推断真实 40 组历史 MAT 有这种差异。K=0 或缺/坏数据的 C++ 显式错误另候选，原 MATLAB 的对应运行行为未实测。

## 改动范围与结案门槛

只修订 [分析 LLD](../design/analysis_plotting.md) 的 Bias 数据流、§5.3/§5.3.1、APL-G01 与 §8.1 预期，以及 [TEST_PLAN](../TEST_PLAN.md) 对 TC-F-BIAS-003/TC-I-ANALYSIS-001 的未来夹具。**AF-083 是文档合同问题，可在固定提交静态复核后结案**。AF-050/051 的实际 L2→L3 曲线和真实业务测试、DEC-005/008、H-10/H-11 仍未完成；旧历史报告、原作者归档及任何批准状态保持原状。
