# R6：HarmonicCoefficients的构造不变量与存储

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。起点`d5818244c9d291f685d23fce982125a38749a91e`，本地/远端一致、工作区干净。接续[98项任务D1](R6_PLAN_TASK_AUDIT_2026-11-18.md)的P2.8/P10.9及REQ-DC-DOMAIN-001。所有者已允许生产代码、构建/CTest和方案文档；Issue #1/#2/#3保持开放，无PR。

## 源码与保真范围

[HarmonicCoefficients](../../include/egomotion/domain/types.hpp)改为私有3×5列主序double矩阵及只读访问。[create工厂](../../src/domain/harmonic_coefficients.cpp)先检查形状、再检查全部15项有限性，以既有InvalidShape/NonFiniteInput和Solve阶段返回失败；不伪造观测索引或叶子错误。1+3+5个有效项之外的六个位置允许有限非零值，按位保存、不清零。默认值明确全零；`zeroOrderUnit`单独提供原脚本(1,1)=1、其余零的初始值，不改成sqrt(4π)归一化。

工厂复制输入，自拥有固定矩阵；左值getter只读，右值及const右值按值返回，避免借用临时对象。整体复制/移动/赋值允许，普通调用不能写入非法分量。有限极值、denorm和signed zero仍合法；存储有效不等于近度可观测或重建成功。Result仍禁止默认成功，未新增数值容差或批准API冻结。

[拟合](../../src/harmonics/harmonic_fitter.cpp)、[旋转](../../src/harmonics/harmonic_rotator.cpp)在原叶子成功后构造值；相位仍仅反转(1,0)/(1,2)/(2,1)/(2,3)。[单步](../../src/estimators/adaptive_step.cpp)、[Session](../../src/estimators/adaptive_session.cpp)和[策略](../../src/estimators/coefficient_update.cpp)迁移到只读访问/显式初始化。[CMake](../../CMakeLists.txt)将新工厂加入既有egomotion_domain静态库并注册新测试，无新增第三方依赖。源归一化、求解顺序、刷新模数和状态提交次序不变。低阶叶子、独立参考及origin树在本轮无差异；静态审计确认单步全文除getter/零值初始化外相同、Session processFrame除getter外相同、四个相位位置相同，不声称整个拟合/旋转文件完全未改。

[共享夹具](../../tests/support/coefficient_fixture.hpp)检查合法固定输入的构造结果。原来先写入NaN/Inf再调用服务的负例，现迁到工厂拒绝，并独立调用原始矩阵low_order布局叶子核对错误身份；未以const_cast伪造非法类型。三组件服务检查由482增为484，原旋转错误链仍测。策略的非有限候选在构造时转为带底层NonFiniteInput的NonFiniteResult失败，Session停止/回滚/同帧重试检查保留。原“刷新帧不得跳过坏基线”用有限DBL_MAX系数经45°旋转真实溢出验证，避免不可构造的非法对象；反射旋转负例保留。

## 双配置实测

受测源码 **`9e4b60f065ecb5625bc30429d59607d9b002cfda`**，分支`dev-plan-P0-P1-mat-dictionary-probe`。原子提交、正常推送、GitHub branch API核实同SHA及干净工作区后运行。RunId **`r6-coefficient-value-20261118-SQJ9gcFR`**，工作目录为仓库根。

| 配置 | 配置/构建 | MAT优先 | 其余CTest | 起止时间（含时区） |
|---|---|---|---|---|
| SVG ON | exit0，无诊断 | 7/7 | 58/58，共65/65 | 2026-11-18T18:08:06+08:00 — 2026-11-18T18:14:46+08:00 |
| SVG OFF | exit0，无诊断 | 7/7 | 57/57，共64/64 | 2026-11-18T18:15:06+08:00 — 2026-11-18T18:21:30+08:00 |

新增[系数值测试](../../tests/contracts/test_r06_coefficient_value.cpp)`r06_coefficient_value_invariants`两模式各 **178项**：六组有限存储、默认零/源单位零阶、行主序来源转为列主序、全部15项按位保存及非零填充；释放输入后仍有效、复制/移动/整体赋值独立性、临时/const右值自拥有。8种形状及NaN/±Inf遍历15位置共45分量负例，形状优先；编译断言拒绝aggregate/公开未检查矩阵构造/可写分量，保持Result无默认构造。

原MotionEstimate839项、球谐53532项、源相位adapter4886项/Session10006项、真实Session交错9579项、策略48523项、入口隔离172962项及场景88121项保留。各模式13组既有数值/合同stdout摘要与前一受测源码1588ebb逐字相同（去除CTest编号）；三组件最大绝对残差仍1.7763568394002505e-15。完整原独立L3/MAT/分析/CLI等回归执行，规定失败、替身成功均不冒充科学成功。

四份JUnit逐名匹配两配置登记，无failure/error/skip/disabled；唯一ON专有项为r06_svg_source_windows。各模式726项输入SHA后验匹配；原628文件/582MAT树仍为`70ee155131c24a50169346d0ffbaff0c01b1c547`。OFF源/链接/ELF/静态库审计无SVG实现。原始资料始终只读。

实际环境Ubuntu24.04.5、Linux7.0.0-43 x86_64、GNU16.2、C++17 Release、CMake/CTest4.3.4、Eigen3.4、matio1.5.26、zlib1.3、Sophus1.24.6。新测试无RNG；原测试seed来源另存，RunId使用独立OS随机源。命令为`bash build/r6-coefficient-value-20261118-SQJ9gcFR/run.sh 9e4b60f065ecb5625bc30429d59607d9b002cfda ON`及同参数OFF。`evidence-ON/`和`evidence-OFF/`保存命令、环境、SHA、起止/退出码、stdout/stderr、JUnit/Testing、输入blob/SHA和编译/链接/ELF。后验核对为read-only-junit-audit.json、read-only-regression-summary-audit.json；源码对照为read-only-arithmetic-audit.json及空的unchanged-leaf-and-input.diff。复用隔离构建缓存，本轮日志独立，旧失败记录未覆盖。

## 尚未完成

没有新构建/CTest缺陷或新Issue/PR，未安装依赖、运行MATLAB/Octave/完整科学/性能实验、改原始资料/阈值、冻结、合并或发布。MotionEstimate与HarmonicCoefficients的D1源码缺口已补；FlowField/DirectionSet构造、D4只读/预留前记录、完整实验及正式对象/数值/资源资格仍缺。真实Adaptive仍按已知acos越界/零旋转边界Incomplete、无MAT；R6未结案。文档后的提交只登记事实，不代表再次构建。

文档只读核对：98个P与PLAN同序，66个TC、45源路径/50声明身份与顺序不变；TEST_PLAN仅顶部事实更新，其余正文完全相同。12份更新文档的576处相对引用有实际目标，见read-only-document-audit.json；不是Bash追踪脚本现场执行或新增CTest。
