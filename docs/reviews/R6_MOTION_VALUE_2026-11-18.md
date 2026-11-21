# R6：MotionEstimate的构造不变量与值语义

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。起点`43581790c473771f9cde18a05a90df99ac615872`，本地/远端一致、工作区干净。接续[98项任务D1](R6_PLAN_TASK_AUDIT_2026-11-18.md)的P2.1/P2.8/P10.9及REQ-DC-DOMAIN-001，所有者已允许生产代码、构建/CTest和方案文档。Issue #1/#2/#3保持开放、无PR；关联任务继续要求检查/实现R6。

## 源码与兼容边界

[MotionEstimate](../../include/egomotion/domain/types.hpp)从公开Eigen aggregate改为两个私有Vector3和只读分量访问；默认值明确为静止，而非未初始化向量。静态[create工厂](../../src/domain/motion_estimate.cpp)接受动态矩阵只读引用，在固定3×1转换前检查两输入形状，随后检查六分量有限性，以既有InvalidShape/NonFiniteInput及Solve阶段返回失败。成功复制到自有固定矩阵，不借用输入。左值只读引用；右值返回Vector3值，避免临时对象销毁后的悬空引用。完整值复制/移动/赋值仍支持，不能局部写入非法分量；Result/Status仍禁止默认成功。

静止、单个零向量、非单位运动、极大/极小有限值和signed zero均合法，不归一化、不改尺度/坐标/单位、不计算模长或引入容差。零范数、不可定义角和可观测性继续由原数值模块判断。默认静止值不等于求解成功；任何Result仍需显式success/failure。此实现只落实MotionEstimate的不变量；FlowField、DirectionSet和HarmonicCoefficients仍是待处理的领域边界，不能宣称全部D1完成，也不能把DEC-008-03对Result的决定扩大为已批准所有领域工厂。

[Franz/KvD实现](../../src/estimators/franz_kvd.cpp)中，Franz以局部三维向量按原轴次序累计后构造；KvD在传入平移策略及最终尺度恢复后构造；[Adaptive实现](../../src/estimators/adaptive_step.cpp)在原6×6求解/平移归一化后构造；Runner真值在已有轨迹有限性检查后构造。算法的非有限输出继续保留原NonFiniteResult与原有错误位置；新防御分支显式失败，不返回伪成功。Session/误差/测试仅使用只读访问。[构建目标](../../CMakeLists.txt)中的`egomotion_domain`由interface目标改为含此实现的静态库，依赖仍为Eigen/C++17，未增加第三方运行库；类型声明前向引用Result/错误枚举，避免头文件循环。该候选源码/API改变要求重编译，不承诺旧二进制兼容或正式冻结。

[共享测试夹具](../../tests/support/motion_fixture.hpp)显式检查固定合法运动的工厂状态。原KvD NaN旧旋转负例改在工厂边界验证NonFiniteInput；非法MotionEstimate已不能通过正常构造/分量赋值进入规则，原非有限near/flow、非法方向和其他失败断言全部保留。此变更不是删去失败输入或降低通过阈值。

## 固定源码双配置实测

受测源码 **`1588ebb97b0c3b0984e5b21335f1e67af9184e62`**，分支`dev-plan-P0-P1-mat-dictionary-probe`。原子提交/正常推送并以GitHub branch API核实同SHA、干净工作区后执行。RunId **`r6-motion-value-20261118-9j2NQZaD`**，工作目录为仓库根。

| 配置 | 配置/构建 | MAT优先 | 其余CTest | 起止时间（2026-11-18，UTC+08:00） |
|---|---|---|---|---|
| SVG ON | exit0，无诊断 | 7/7 | 57/57，共64/64 | 17:40:13–17:45:36 |
| SVG OFF | exit0，无诊断 | 7/7 | 56/56，共63/63 | 17:45:43–17:51:39 |

新增[运动值测试](../../tests/contracts/test_r06_motion_value.cpp)`r06_motion_value_invariants`两模式各 **839项检查**：49组有限运动逐位保持、默认静止、释放/缩放源矩阵后仍有效、复制/移动/赋值独立性、移动后固定向量有限、临时及const右值返回自拥有值；16种形状负例、18个NaN/±Inf分量负例及形状优先顺序。编译断言拒绝aggregate/公开未检查两向量构造/可写分量，核对左右值返回类型和Result禁止默认构造。不以模长或单位方向约束运动，max/denorm/signed zero均按位保存。

原独立Franz/KvD/完整迭代trace、Adaptive L3/conditioning/单步/Session、球谐/Reader/Writer/分析/CLI等测试全部执行；172962项三Runner隔离、88121项场景及既有策略/误差回归保留。`read-only-arithmetic-audit.json`对比Franz逐轴算式（只消去成员路径差异）、KvD旋转更新/残差/停止段和Adaptive投影段（只消去getter括号），结果相同；未声称整个函数体完全不变。真实三Adaptive仍按原acos越界/零旋转边界Incomplete、无MAT，不能把规定失败测试通过称为科学成功。

四份JUnit与两配置CTest登记逐名一致，全为status=run，无failure/error/skip/disabled，仅ON多出`r06_svg_source_windows`。各模式724项输入摘要后验全部匹配；原628文件/582MAT树仍为`70ee155131c24a50169346d0ffbaff0c01b1c547`。OFF源/链接/ELF/静态库检查无SVG实现。

实际环境为Ubuntu24.04.5、Linux7.0.0-43 x86_64、GNU16.2、C++17 Release、CMake/CTest4.3.4、Eigen3.4、matio1.5.26、zlib1.3、Sophus1.24.6。新测试纯固定值无RNG，原测试seed来源另存，自动RunId仍用独立OS随机源。命令为`bash build/r6-motion-value-20261118-9j2NQZaD/run.sh 1588ebb97b0c3b0984e5b21335f1e67af9184e62 ON`及同参数`OFF`。`evidence-ON/`和`evidence-OFF/`保存命令/参数、分支/SHA、环境、起止/退出码、stdout/stderr、JUnit与Testing快照、输入blob/SHA、编译/链接/ELF；后验核对为`read-only-junit-audit.json`。复用原隔离构建缓存，旧失败证据未覆盖。

本轮没有新构建/测试缺陷或新Issue/PR。没有安装依赖、运行MATLAB/Octave/完整科学/性能实验、改原始文件/阈值、冻结、合并或发布。D1其余值边界、D4只读/预留前记录、完整科学实验及正式对象/数值/资源资格仍缺，R6未结案。后续文档提交只登记事实，不代表再次构建。

附加只读核对：两配置各13组原有数值/合同摘要与源码`34fe509`检查点逐字一致（去除CTest编号前缀），包括球谐残差、Bias统计、Session和Runner计数，保存为`read-only-regression-summary-audit.json`；该摘要核对不替代各测试的原断言。

文档只读复核：98个P与PLAN同序，66个TC及45源路径/50声明身份和顺序不变；TEST_PLAN仅更新顶部事实，其余正文完全相同。12份更新文档的566处相对引用均有目标，见`read-only-document-audit.json`；不是Bash追踪脚本现场执行或新增CTest。
