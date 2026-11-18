# R6：FlowField有限存储与方向独立拟合

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。接续[98项任务审计](R6_PLAN_TASK_AUDIT_2026-11-18.md)的D1/P2.1/P2.8/P10.9及REQ-DC-DOMAIN-001。起点`0dce8fd9fd4652a7d17666264d83769dc4984ec0`，本地/远端一致、工作区干净；用户授权生产代码、构建/CTest和方案文档。本轮Issue全状态检索仍为#1/#2/#3开放、无PR，未改Issue/PR状态。

## 实现范围

[FlowField](../../include/egomotion/domain/types.hpp)改为私有N×6列主序double和只读访问，默认0×6空集合。[工厂](../../src/domain/flow_field.cpp)先核六列，再逐行检查全部分量有限；错误使用既有InvalidShape/NonFiniteInput、Simulation阶段和实际0-based首个坏行。复制输入后自拥有，符号、尺度、signed zero和极端有限值均不改变。零/非单位方向、有限非切向光流仍可存储，空值/单位/切向条件沿用各算法原作用位置；没有归一化、投影或新容差。

create内部复制失败映射ResourceLimitExceeded；fromOwned只接受FlowMatrix原类型非const右值，删除模板阻止隐式Eigen转换，先检查再移交，失败保留原数据和地址。成功后调用者必须放弃旧可写别名。复制赋值先复制后交换，失败保留旧值；左值getter只读、普通右值移交、const右值复制。普通复制及进入工厂前的Eigen表达式/Ref临时构造仍可能抛异常，不宣称内存耗尽时错误载荷也必能分配。

[光流合成与噪声](../../src/simulation/flow.cpp)使用局部可写矩阵完成原算式与随机调用，末尾检查移交；NoNoise复制完整值。[Franz/KvD](../../src/estimators/franz_kvd.cpp)、[Adaptive单步](../../src/estimators/adaptive_step.cpp)与[Runner](../../src/experiments/adaptive_runner.cpp)只改getter。read-only-arithmetic-audit.json核对这些文件全文：前三个消费文件消去getter括号后完全一致；合成/噪声逆向还原列明的局部存储和检查返回变化后全文一致。原始资料、独立参考、低阶投影叶子、Session及系数策略无差异。

[HarmonicFitter](../../src/harmonics/harmonic_fitter.cpp)新增DirectionSet重载；FlowField重载只取前三列，两个入口共用原投影叶子。原N×K矩阵可显式提取前三列而无需构造光流，因此尾部Inf不妨碍方向拟合，完整FlowField则拒绝该输入。没有改变九通道求和/4pi/相位顺序、单位判据或原始叶子的K>=3前缀行为。原自由函数FlowField兼容入口仍保留。

[夹具](../../tests/support/flow_fixture.hpp)先检查再拥有原数值。KvD非有限流负例迁到工厂，非单位仍在规则入口拒绝；[Adaptive噪声替身](../../tests/integration/test_r06_adaptive_injection.cpp)的NaN产物同样在工厂拒绝，Runner沿既有数值失败路径报告IncompleteRun并保留NonFiniteInput/Simulation/row0，不能进入Step；空行/方向改动仍报告DataMismatch。这是不可变值边界前移，不是放宽失败合同；36091项和24个服务故障继续执行。

## 真实构建与测试

受测源码 **`b2628a97d1ca74844a2b57fab035387ce847670c`**，分支`dev-plan-P0-P1-mat-dictionary-probe`。运行前已原子提交、正常推送并用GitHub branch API核实同SHA；工作区干净。RunId **`r6-flow-value-20261118-dcfqcXP8`**，工作目录仓库根。

| 配置 | CMake/构建 | MAT优先 | 其余CTest | 起止时间（含时区） |
|---|---|---|---|---|
| SVG ON | exit0，无诊断 | 7/7 | 60/60，共67/67 | 2026-11-18T19:01:14+08:00 — 2026-11-18T19:07:20+08:00 |
| SVG OFF | exit0，无诊断 | 7/7 | 59/59，共66/66 | 2026-11-18T19:07:50+08:00 — 2026-11-18T19:13:22+08:00 |

[新增测试](../../tests/contracts/test_r06_flow_value.cpp)`r06_flow_value_invariants`两配置各 **1245项**：五种行数×六类有限输入共30组合，8种错误形状、NaN/±Inf遍历7×6共126个坏分量；核对列主序逐位保存、源销毁/复制/移动/完整赋值/自赋值/临时/const右值，首行错误优先及失败移交不消费输入。编译断言保证只读、禁止aggregate/未检查构造、原类型移交约束和无抛移动。

只在测试目标链接--wrap=malloc，确定性单次注入复制工厂、复制赋值、const右值getter三次分配失败；核错误分类或异常、旧值/地址保留。移交工厂和普通右值getter在故障开关未消费时按地址验证无复制分配。真实NoNoise/Franz/Adaptive检验空/零/非单位方向原拒绝；NoNoise保留有限径向分量且零次RNG，不新增切向拒绝。此为有限故障证据，不是实际耗尽内存、性能或普遍异常安全证明。

[球谐服务测试](../../tests/integration/test_r06_harmonic_services.cpp)由484增至494项，新增方向重载、尾部Inf原始叶子与完整流场拒绝、空/非单位方向错误链，最大绝对差仍1.7763568394002505e-15。各模式19组既有数值/合同stdout摘要与7f4d248逐字相同（除CTest编号）；含KvD策略2712、停止75080、系数策略48523、真实Session交错9579、Adaptive注入36091、入口隔离172962、场景88121、方向901、运动839、系数178项。原L2/L3、MAT、分析和CLI全部注册测试均执行。

四份JUnit逐名对应全部注册项，无失败/错误/跳过/禁用；ON唯一多出的测试是r06_svg_source_windows。每配置730项输入SHA后验匹配，原628文件/582MAT树仍为`70ee155131c24a50169346d0ffbaff0c01b1c547`。OFF编译源/链接/ELF/静态库审计未含SVG实现。

实际环境Ubuntu24.04.5、Linux7.0.0-43 x86_64、GNU16.2、C++17 Release、CMake/CTest4.3.4、Eigen3.4、matio1.5.26、zlib1.3、Sophus1.24.6。命令为`bash build/r6-flow-value-20261118-dcfqcXP8/run.sh b2628a97d1ca74844a2b57fab035387ce847670c ON`及同参数OFF。复用各模式隔离缓存，evidence-ON/OFF分别保存命令/参数、Git身份、环境、起止/退出码、stdout/stderr、JUnit/Testing及输入blob/SHA；新测试无随机抽样，既有种子来源另存，RunId仍独立OS随机。read-only-junit-audit.json和read-only-regression-summary-audit.json为只读后验核对。

## 修改清单

本轮源码检查点包括以下29个文件（报告及进度同步另作文档提交）：

```text
CMakeLists.txt
docs/design/contracts.md
include/egomotion/domain/types.hpp
include/egomotion/harmonics/services.hpp
src/domain/flow_field.cpp
src/estimators/adaptive_step.cpp
src/estimators/franz_kvd.cpp
src/experiments/adaptive_runner.cpp
src/harmonics/harmonic_fitter.cpp
src/simulation/flow.cpp
tests/contracts/test_r06_adaptive_periods.cpp
tests/contracts/test_r06_flow_value.cpp
tests/contracts/test_r06_session_contract.cpp
tests/integration/r06_adaptive_init_trace.cpp
tests/integration/test_r06_adaptive_conditioning.cpp
tests/integration/test_r06_adaptive_injection.cpp
tests/integration/test_r06_adaptive_l3.cpp
tests/integration/test_r06_adaptive_runner.cpp
tests/integration/test_r06_adaptive_scene.cpp
tests/integration/test_r06_bias_injection.cpp
tests/integration/test_r06_bias_runner.cpp
tests/integration/test_r06_harmonic_services.cpp
tests/integration/test_r06_kvd_l3.cpp
tests/integration/test_r06_kvd_stopping.cpp
tests/integration/test_r06_kvd_strategy.cpp
tests/integration/test_r06_sampling_flow.cpp
tests/integration/test_r06_session_interleaving.cpp
tests/integration/test_r06_source_sh.cpp
tests/support/flow_fixture.hpp
```

文档只读核查保留98项P、66项TC的身份/顺序、45个.m路径及50个声明；TEST_PLAN验收正文未变，14份文档的634条相对链接有效。记录为read-only-document-audit.json，不是Bash追踪脚本现场执行或人类冻结。

## 剩余工作

D1列明的运动、系数、方向和流场四类构造/存储已补有限实测；总体对象与公共API仍是未冻结候选。D4只读/预留前记录、完整科学实验和正式数值/资源/基线资格仍缺。真实Adaptive仍因既有acos越界/零旋转Incomplete、无MAT；合成八轮成功及规定失败不能当作完整科学实验成功。未安装依赖、运行MATLAB/Octave/完整科学或性能实验、改原始资料/阈值、冻结、合并或发布。后续文档提交仅登记事实，不代表重新构建；R6未结案。
