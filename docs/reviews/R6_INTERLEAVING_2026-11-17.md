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

源码检查点 `412f6aba63ee08b5390604008b01728819ea2300` 已正常提交/推送并通过GitHub分支API核实，然后配置、增量构建和42/42 CTest全部通过。分支为 `dev-plan-P0-P1-mat-dictionary-probe`；RunId `r6-interleaving-20261117-xIQZ6xzO`，独立日志为 `build/r6-interleaving-20261117-xIQZ6xzO/evidence/`，复用构建缓存 `build/r6-20261117-KzjFHPDQ/cmake/`。工作目录为仓库根，所有合成输出保留在构建沙箱中。

| 操作 | 时间（UTC+08:00） | 退出码/结果 |
|---|---|---|
| CMake配置 | 2026-11-17 20:14:04–20:14:06 | 0 |
| 增量构建 | 20:14:06–20:14:27 | 0，无诊断 |
| 五项MAT CTest | 20:14:27–20:14:28 | 0，5/5 |
| 其余CTest | 20:14:28–20:14:31 | 0，37/37 |
| 输入摘要复核 | 结束20:14:31 | 0，未改变 |

环境为Ubuntu24.04.5、Linux7.0.0-43、GCC16.2.0、CMake/CTest4.3.4、Eigen3.4.0、matio1.5.26、zlib1.3及已有Sophus1.24.6。配置为Release/C++17、`BUILD_TESTING=ON`，构建并行4；先按MAT五项正则并行2运行，再排除该正则并行4运行其余。完整命令/参数、开始结束时间、退出码分别在四个`.meta`，环境在`environment.txt`；stdout/stderr和JUnit单独保存，全部stderr为0字节。输入Git blob在`input-git-blobs.txt`，运行前后文件SHA-256在`inputs.sha256`和`inputs-after.txt`；没有安装依赖或运行统计实验。

实测Session **9579项检查、252次成功调用、12次拒绝、0失败**；三模式各42次独立成功、42次交替成功及4次拒绝。RunContext **282项检查、96个RNG样本、2540个真实MAT回读值、0失败**。A的RunId为`run_84975c6a91ecf9d8c1c8aa69f5944fd7`（seed7），B为`run_fc6527554cff2c0714f9f7fedbeffad3`（seed11）；输出位于上述缓存的`r06_run_context_interleaving/交替 实例/output/`。对应Manifest SHA-256分别为`8b3753459651641194c75f0e842fe4ff2d5e1e164852ced027b09cb2067f399f`与`8e0f174d8de2cd32c28bf864101d13cdc36f70081c26ecb49b9282bf212001f3`。

CTest stdout SHA-256：MAT `20bd60e70104e954df003e9c6a4c76d84c6f0b9a7ef8ae89f9894e777a2d50b3`；其余 `0789126fda27e2233ac4a5dda5d40d0be421201bfd42baba7bab507be0548c4e`。原始628文件/582MAT的Git树仍为`70ee155131c24a50169346d0ffbaff0c01b1c547`。R6仍未结案。

## 同批补充：acos舍入端点（尚未执行）

沿用`TC-F-ERROR-001`和`TC-NF-NUM-002`，在现有`r06_source_motion_errors`追加固定向量`(1,1,1)`及其反向。独立标量binary64计算应略越过`+1/-1`，测试显式核对该前提，并要求真实角度函数返回`UndefinedAngle`且诊断命中`acos argument outside [-1,1]`；同一输入的尺度对齐距离仍为解析值0或`2*sqrt(3)`。保留输入字节和既有回归容差，无钳制/新生产门限，也不把C++拒绝行为宣称为MATLAB运行结果。待新检查点后复测42项。
