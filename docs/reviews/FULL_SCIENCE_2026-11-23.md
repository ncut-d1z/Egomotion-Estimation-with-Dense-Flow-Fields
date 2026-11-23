# 完整科学实验运行记录（2026-11-23）

`[HUMAN-EXECUTE/APPROVE]` 所有者已恢复完整实验。初始开发分支为 `dev-plan-P0-P1-mat-dictionary-probe`，本批授权检查点为 `b38b7ae85f33dca8be6e6b452e8d24df9f071038`。实际科学程序是已通过 SVG ON 全套 CTest 的 Release 二进制，内嵌源码 SHA 为 `a83682f2e493170521ec3192e9463e3b4fc132cd`。两个提交的生产代码、CMake、测试与归档输入相同。参考实验与数据集分别见 [Strübbe、Stürzl 与 Egelhaaf 的论文](https://doi.org/10.1371/journal.pone.0128413)和[原始归档](https://doi.org/10.4119/unibi/2736885)。

本机为 Ubuntu 24.04.5 x86_64，科学二进制使用 GCC 16.2.0、Eigen 3.4.0 和 matio 1.5.26。证据根为 `build/science-20261123-AXfvVUVJ/`。每次调用保存命令、工作目录、分支、起止时间、退出码、stdout、stderr 和 GNU time 的 wall、CPU、峰值 RSS。科学二进制的 SHA-256 在 `binary.sha256`，原始输入 SHA-256 在 `inputs.sha256`。图件及原生 Adaptive 结果位于仓库根 `output/science-20261123-AXfvVUVJ-<入口>/`。科学二进制的来源以它产生的 invocation 和 manifest 为准。早期记录模板也给 Bash、CMake 等工具写入了这一二进制的 SHA，这些工具自身的身份应查实际命令、Git 检查点及编译配置。

## Fig. 2、3、4

Fig. 2 初始化实际使用 35 个位置、33 个内帧，留下 66 条双槽观测、198 个逐单元均值和两幅 SVG。逐帧与四个历史 MAT 比较，Original / Adaptive 平移方向角的最大绝对差分别为 3.0951e-12° / 1.1965e-6°，旋转轴方向角为 6.8390e-14° / 3.0414e-10°。新的 SO(3) 测地角另行报告。独立复算确认两个图各有 66 个曲线点。

Fig. 3 和 Fig. 4 的独立生产调用都返回 0。生成的双面板 SVG 与前轮复核版逐字节相同，SHA-256 分别为 `e14cc393dd2b56d2efd66d02c094aa80d464574eca8e0f290ca41a93941b0daf`、`da0b766e6bb0c67c6dff464789abdb0672040e3c8c4930e07e9dbd62501cddcf`。[几何来源和出版图比较](PAPER_FIGURES_1_TO_5_2026-11-21.md)

## Fig. 5 与原生 Adaptive 交付

实际运行 `paper-figure --figure 5 --seed 1 --periods article|archive --format svg`。种子 1 是本批两套日程共用的固定输入。每套分别执行两个噪声条件，各 8 轮、每轮 598 个内帧、两种算法和 5,120 条视线。每个噪声条件保存 9,568 条样本。更新周期另有 2,392 条样本，论文周期为 1/5/10/20，归档周期为 1/5/11/21。两套任务均正常返回 0，相同噪声条件的 Comparison TSV 逐字节相同。独立复核每套 14,352 个均值单元及实际绘图窗口。

| 噪声倍率 | Original 平移均值 | Adaptive 平移均值 | Adaptive 八轮均值的样本标准差 |
|---|---|---|---|
| 0 | 0.80560° | 0.03206° | 0° |
| 0.1 | 1.15250° | 0.71160° | 0.12206° |

表中先算每轮 598 帧的平移均值，再取八轮均值。同一条件八轮均完成，零旋转帧 151 和 451 的方向轴无定义，两个槽合计产生 32 条轴角缺测。更新周期单轮有 8 条轴角缺测。相应八轮轴角均值保留缺测，测地角仍有定义。每套实际的轴角和测地角曲线分别有 9,788、10,020 个点，归档日程另有 3,800 个线性视图点。

独立解码发现，归档 `with_errors` 和 `without_errors` 的四个同名误差字段及 `rottransratio` 的文件身份不同，但每对的 600 个 double 载荷逐位相同。这两份归档无法独立证实噪声效应。上述表格描述新 C++ 实验的实际变化。逐字段身份、逐轮均值及所有缺测原因见 `audit-adaptive/stdout.log`，其 SHA-256 为 `0cc2b835e3a41d8a32ced9f012d1a9f37ea7473367c87acbed22a65187eacbdc`。

原生初始化、零噪声 Comparison、0.1 噪声 Comparison 和归档周期 UpdateRates 分别交付 9、11、11、7 个 MAT，四份 manifest 均为 Complete。独立解码检查了全部 38 个 MAT 的回执、字节数、SHA-256、类型和形状，复算了 11,540 个聚合值与其有效数及 53,540 个逐轮值。33,620 个可直接比较的确定性内帧值与专项输出相等。原生 0.1 条件实际种子为 2875821374，Original / Adaptive 平移均值分别为 1.14520° / 0.70610°。与专项种子 1 的比较仅报告样本统计。完整结果在 `audit-native-adaptive/stdout.log`，SHA-256 为 `f7106c2986eff75a38b659870b71a0112d3b7883994c4978d9b8aab06a44afdb`。

## 现场文档追踪

首次现场 Bash 核查返回 1。脚本把测试计划中两行场景引用也计作正式 TC，使正确的 66 条登记被计为 68 条。修复 `b3edf0099cf25b9d745969474ad7a2a42e4c2383` 将解析范围限定为第 3 节的登记表。开发分支复测返回 0，核对 66 项 REQ、66 项 TC、98 项 P、45 个源文件和 27 条 HLD 依赖边。另以三个独立负例确认重复登记、删行和 AC 错配均返回 1，额外引用表可正常通过。首次日志、修复复测和反例分别位于 `docs-initial/`、`docs-fixed/`、`trace-fix-evidence/`。[Issue #9](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/issues/9)
