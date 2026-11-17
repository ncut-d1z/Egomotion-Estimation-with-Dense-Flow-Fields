# R6 Session / RunContext 交替隔离验证

任务标签 `[HUMAN-EXECUTE/APPROVE]`。在已授权生产代码、构建/CTest 及文档范围内补测试；合成 MAT 仅写独立构建沙箱，不执行完整科学实验、修改原始资料或变更数值阈值。

## 首轮准备（尚未执行）

新增 `r06_real_session_interleaving`、`r06_run_context_interleaving`，CTest 登记由40增至42。**登记不等于运行或通过。** 原子提交、正常推送并核实远端 SHA 后，先执行五项 MAT 测试，再运行其余测试；实际日志独立保存，失败不覆盖。

### Session：真实生产单步与不同步时钟

`tests/integration/test_r06_session_interleaving.cpp` 使用默认拥有的真实 `AdaptiveMfaStepSolver`，没有替身。Initialization、Comparison、UpdateRates 三种模式各构造两个不同固定流场。每个实例使用12条二十面体方向、随帧变化的正近度、不同平移/旋转信号，以及不同的正交系数旋转；比较用参考轴与信号不共线，明确是状态验证输入，不声称原论文轨迹或科学真值。

每条流先独立执行 MATLAB 帧2..22保存结果，再在两个新实例上交替重放，A先走两帧使时钟不同步。逐帧按位比较 primary/四个baseline、完整运动/系数/误差、trace 的 near、四块矩阵、完整矩阵/RHS/raw解和比较角度，并检查未被调用的另一实例及输入不变。独立重放是**调度不变性**参考，不替代独立数学 Golden。

在A的第5帧注入空观测、B的第11帧注入零旋转参考，要求分别保留 EmptyInput/UndefinedAngle 原因，所有系数回滚、另一实例不变；尝试跳到下一帧必须拒绝。另一实例可继续，失败实例重试原帧后与独立记录一致。若全部完成，三模式应合计252次成功Session调用、12次明确拒绝（6次数值失败与6次越序拒绝）。此处是待验证计数，不预填测试通过。

### RunContext：真实隔离输出与故障局部化

`tests/integration/test_r06_run_context_interleaving.cpp` 在 `build/.../r06_run_context_interleaving/交替 实例/` 下创建两个新的自动 RunId，分别拥有真实 `Mt19937RandomSource`、`RunMatWriter`、`RunTextPublisher` 和 RunContext。仅B的第二次Writer调用由窄包装器注入 WriteFailed，其已成功部分仍由真实Writer发布并回读；不伪造回执或Manifest。

- A使用初始化save合同（四个35×1合成数组）、seed7、一次内存完成；B使用更新率save合同（两个600×4与一个600×1数组）、seed11、预期8次但实际不完整。所有输出都有 `SYNTHETIC_TEST_ONLY.txt`，Complete状态只表示该合同fixture发布成功，不是算法实验完成。
- 交替RNG调用与各自独立控制实例逐值一致，覆盖输出故障/Manifest发布前后；验证计数、错误链、来源/RunId、回执不会串入另一实例。
- B第二次写入失败后仅保留第一份真实MAT；A仍可发布四MAT和Complete Manifest，B发布Incomplete Manifest。A拒绝B的schema后仍可正确重试；B不能借A的RunId写文件，也不能从Incomplete转成Complete。
- 两个RunId使用相同MAT文件名但不同维度/值；全部已发布数值逐值回读并核对SHA-256，缺失字段不能被另一方补齐。重复写入/重复Manifest被拒绝后，两边文件摘要及状态保持不变。

这是**同进程交替操作**的隔离证据，不是线程竞态压力测试，也不承诺一个Session/RunContext可被多个线程同时调用；已有目录竞态/故障测试另见IO报告。原有TC/AC、最终资格及冻结状态不变。

## 实测记录

待首轮检查点后填写；R6仍未结案。
