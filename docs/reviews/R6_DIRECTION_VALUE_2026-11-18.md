# R6：DirectionSet的构造、所有权与异常边界

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。起点`f178c3875fdb763e14d9c108f3f989f07e89e134`，本地/远端一致、工作区干净。接续[98项D1](R6_PLAN_TASK_AUDIT_2026-11-18.md)的P2.1/P2.8/P10.9和REQ-DC-DOMAIN-001。所有者已授权生产代码、构建/CTest和方案文档。Issue #1/#2/#3开放，无PR。

## 实现与兼容范围

[DirectionSet](../../include/egomotion/domain/types.hpp)从公开aggregate改为私有N×3列主序double存储和只读访问，默认明确为0×3空集合。[create工厂](../../src/domain/direction_set.cpp)先检查列数，再逐行检查有限性；InvalidShape不伪造位置，NonFiniteInput以Simulation阶段和实际0-based观测行返回，多个错误时保留首个坏行。复制输入后自拥有，原行序、符号、signed zero和极端有限值均不改变。

空集合、零向量和非单位有限方向仍可存储；FlowSynthesizer/Sphere按各自既有规则拒绝不适用的物理输入。没有归一化、新单位容差或切向门槛，构造成功不等于物理求解成功。Result仍禁止默认成功。

`fromOwned`用于采样器移交现有DirectionMatrix：校验成功才移动缓冲、失败保留输入及原地址；调用者成功移交后须放弃旧可写别名。删除模板拒绝其它Eigen矩阵类型的隐式转换及左值/const右值直接移交，未知形状应走create。左值getter只读，非const右值getter移交缓冲，const右值getter复制。深拷贝可能抛分配异常；复制赋值先复制后交换，失败保留旧值；工厂内部复制失败映射ResourceLimitExceeded。调用前Eigen表达式/Ref临时构造仍遵循C++异常规则，不承诺所有内存不足都能创建错误消息。

[采样器](../../src/sampling/directions.cpp)在局部DirectionMatrix中执行原normalizeSource/emitFace，完整生成后移交；[光流](../../src/simulation/flow.cpp)和[球环境](../../src/trajectory/sphere.cpp)只改访问语法。[CMake](../../CMakeLists.txt)加入领域实现和测试，无新第三方依赖。静态read-only-arithmetic-audit.json核对：Flow/Sphere全文消去getter括号后与起点一致；采样全文仅有列明的局部存储/成员路径/检查返回变化，面表、A/B/C/D递归、sqrt(dot)及运算顺序保持。

[方向夹具](../../tests/support/direction_fixture.hpp)显式核对合法输入；[Adaptive注入测试](../../tests/integration/test_r06_adaptive_injection.cpp)的Inf采样产物改由工厂拒绝，仍不得到达噪声/Step。每模式新增一条错误链行索引断言，共由36088增至36091项；其它24个服务故障和数值测试保留。非单位值仍在原消费端拒绝，未删除负例或改变数值通过阈值。

## 首轮与最终检查点

初始实现`20f15b5ad457b3655af2e272f1c44048ccae3350`在`build/r6-direction-value-20261118-vkJuECfL/`完成SVG ON66/66，无构建诊断，901项新检查通过。编译期间静态复核发现Eigen Matrix具有非explicit的通用表达式构造，单独DirectionMatrix右值形参仍可触发隐式转换；记录在eigen-implicit-conversion-source.txt。后续`7f4d248`添加删除模板和编译断言后重新建立检查点；首轮日志/first-checkpoint-audit.json保留，未用首轮结果代替最终双模式。此为静态发现的API加固，没有发生构建/CTest失败。

最终受测源码 **`7f4d248255a1502b65e1f8210071a5d47b919839`**，分支`dev-plan-P0-P1-mat-dictionary-probe`。每次运行前原子提交、正常推送、GitHub branch API核实同SHA及工作区干净。RunId **`r6-direction-final-20261118-SiB24fFB`**，工作目录仓库根。

| 配置 | 配置/构建 | MAT优先 | 其余CTest | 起止时间（含时区） |
|---|---|---|---|---|
| SVG ON | exit0，无诊断 | 7/7 | 59/59，共66/66 | 2026-11-18T18:39:45+08:00 — 2026-11-18T18:45:33+08:00 |
| SVG OFF | exit0，无诊断 | 7/7 | 58/58，共65/65 | 2026-11-18T18:45:49+08:00 — 2026-11-18T18:51:48+08:00 |

新增[方向值测试](../../tests/contracts/test_r06_direction_value.cpp)`r06_direction_value_invariants`两模式各 **901项**：0/1/2/7/19行×六类有限输入共30组合，源销毁后仍有效、逐位保存/列主序索引、复制/移动/完整赋值/自赋值/临时与const右值。8类列数负例、NaN/±Inf遍历7×3位置共63个坏分量，复制和移交两入口均拒绝，错误首行及失败移交不消费输入。编译断言验证只读、禁止aggregate/未检查构造、nothrow移动及移交类型限制。

测试仅在自身链接目标用--wrap=malloc单次注入三次失败：复制工厂、复制赋值、const右值getter；分别验证ResourceLimitExceeded、旧值/缓冲不变及常量源不变。有效所有权工厂和非const右值getter在故障开关保持未消费时成功，并按地址核对真实缓冲移交。另通过真实Flow/Sphere核对空集合、零/非单位方向拒绝及行索引。这是确定性故障测试，不是实际内存耗尽实验、性能预算或普遍异常安全证明。

既有714行/2142坐标独立采样L3、光流/球/轨迹、运动839项、系数178项及全部旧回归执行。各模式15组既有stdout数值/合同摘要与9e4b60f检查点逐字相同（除CTest编号），另Adaptive注入只增加上述3项。球谐服务484项最大绝对差仍1.7763568394002505e-15，入口隔离172962项和场景88121项保留。

四份JUnit逐名对应两配置全部注册项，无failure/error/skip/disabled，ON唯一额外项为r06_svg_source_windows。各模式728项输入SHA后验匹配；原628文件/582MAT树仍为`70ee155131c24a50169346d0ffbaff0c01b1c547`。OFF编译源/链接/ELF/静态库审计无SVG实现，原始资料只读。

实际环境Ubuntu24.04.5、Linux7.0.0-43 x86_64、GNU16.2、C++17 Release、CMake/CTest4.3.4、Eigen3.4、matio1.5.26、zlib1.3、Sophus1.24.6。新测试无RNG，既有种子来源另存，RunId仍独立OS随机。命令为`bash build/r6-direction-final-20261118-SiB24fFB/run.sh 7f4d248255a1502b65e1f8210071a5d47b919839 ON`及同参数OFF。evidence-ON/OFF保存命令、SHA/分支/环境、起止/退出码、stdout/stderr、JUnit/Testing、输入blob/SHA和编译/链接/ELF；只读后验为read-only-junit-audit.json及read-only-regression-summary-audit.json。复用隔离构建缓存，新旧日志分别保存。

文档只读检查确认：98项P和66项TC身份/顺序、45个原.m路径及50个声明数保持，TEST_PLAN正文未变，13份文档的605条相对链接有效。证据为read-only-document-audit.json；这不是Bash追踪脚本现场执行或规范冻结。

## 剩余边界

MotionEstimate、HarmonicCoefficients和DirectionSet的构造/存储实现已补；FlowField尚为aggregate，尤其须保留球谐拟合只读前三列的源合同。D4只读/预留前记录、完整科学实验和正式对象/数值/资源资格仍未完成。真实Adaptive仍因既有acos越界/零旋转Incomplete，无MAT，不能把规定失败/合成八轮成功称为完整实验成功。未安装依赖、运行MATLAB/Octave/完整科学/性能实验、改原始资料/阈值、冻结、合并或发布；无新Issue/PR，R6未结案。后续文档提交只登记事实，不代表再次构建。
