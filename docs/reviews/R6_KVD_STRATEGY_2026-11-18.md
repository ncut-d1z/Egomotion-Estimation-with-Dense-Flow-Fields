# R6：KvD迭代引擎与平移策略组合

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。所有者已授权代码、构建/CTest及方案文档修改；落实 [PLAN P4.9](../PLAN.md)、REQ/TC-DC-KVD-001及 [98项P审计D2](R6_PLAN_TASK_AUDIT_2026-11-18.md)，不修改数值阈值、原始资料或正式冻结/验收状态。起点本地与远端同`e78d608b7e6d0df4d533c928ecb0dcef561eadad`，工作区干净；关联任务“检查R5实现完成度”仍idle，最新用户要求继续R6，未向其发消息。

## 实现与保真边界

此前两入口使用同一内部函数和枚举，不能实际注入替换平移规则。本轮在`include/egomotion/estimators/franz_kvd.hpp`、`src/estimators/franz_kvd.cpp`实现`IKvDTranslationRule`、`OriginalKvDTranslationRule`、`ModifiedKvDTranslationRule`、`KvDIterationEngine`。两个具名Estimator各组合自己的无状态引擎和明确规则，既有estimate签名不变；引擎每次调用借用策略，不保存引用/流场/跨调用状态，复制Estimator不会形成指向旧对象的内部借用。

规则接收翻号后的near及旧t/omega，返回逐行求和后除N的a/b/c和未归一化raw t。Original保留near*p、near*d、near²投影项；Modified保留p、d、near投影项。共用引擎仍处理Franz初始化、near重估/翻号、唯一一次归一化、使用旧t的旋转更新、提交、预测残差、6/200及动态/绝对停止门限和最终旧near均值缩放。原方向检查1e-8仅提取为命名常量，不改变数值。

策略输入显式拒绝空/错near长度/非有限/非单位视线；策略输出非有限或零raw不会被当有效运动。策略返回的Failure完整透传，不附加虚假包装；分配失败沿既有ResourceLimitExceeded路径。新接口/Estimator布局属候选增量，需重编译依赖，不声称跨旧二进制ABI或正式接口冻结。

## 验证范围

新增`r06_kvd_strategy_composition`：固定六轴的标量求和参考；负near翻号及每轮调用记录；同一引擎交替两策略、与具名入口逐位比较、trace开关和输入只读；第三轮策略失败/非有限/零候选/分配故障及后续复用；保留内层来源/变量/索引/RunId；替换raw候选验证引擎确实调用策略并负责归一。逐位比较只证明接口编排；算法数值另由原有独立L3/完整trace/停止参考/Bias回归验证，不把自比较当独立Golden。

## 固定源码实测

受测源码 **`df511f9924b75ded66b437fd533ca888918420c1`**，分支`dev-plan-P0-P1-mat-dictionary-probe`。执行前完成原子提交、正常推送并用GitHub branch API核实同SHA；工作区干净。所有者授权范围内由Codex代执行，两模式分别配置/增量构建，先MAT7项再运行全部其余CTest。

| 配置 | 配置/构建 | MAT优先 | 其余CTest | 起止时间（2026-11-18，UTC+08:00） |
|---|---|---|---|---|
| SVG ON | exit0，无诊断 | 7/7 | 49/49，合计**56/56** | 01:47:59–01:51:19 |
| SVG OFF | exit0，无诊断 | 7/7 | 48/48，合计**55/55** | 01:51:13–01:54:43 |

四份JUnit逐名核对全部status=run，零失败/跳过/禁用，与各自保存的CTest登记完全一致；唯一配置差异仍是`r06_svg_source_windows`。新增策略组合测试两模式各**2712项检查**通过。既有独立KvD逐节点/完整trace、506次停止参考（75080项检查）、真实Bias数值（17695项）及发布（5560项）全部通过，Bias最大绝对残差仍为6.6613381477509392e-16。故障注入、错误链和重用断言是接口/生命周期证据，不替代独立公式参考或全输入域数值资格。

环境：Ubuntu24.04.5、Linux7.0.0-43-generic x86_64、GNU16.2.0、C++17 Release、CMake/CTest4.3.4、Eigen3.4.0、matio1.5.26、zlib1.3、Sophus1.24.6。仅此环境实测，不以构建耗时认定性能预算。策略夹具固定、无新增RNG；既有测试种子/来源保留在测试源码与`randomness-source.txt`，RunId仍使用独立OS随机源。

RunId **`r6-kvd-strategy-20261118-iQclVtbH`**，工作目录为仓库根。同名`build/`目录的`run.sh`保存完整命令，`evidence-ON/`、`evidence-OFF/`保存每步meta/stdout/stderr、环境、Git blob、输入SHA及后验、JUnit、Testing副本和ELF/链接证据；`read-only-junit-audit.json`保存逐名静态复核。原`.m`仅静态审阅：Original blob `d48048f37b859bfb19402ca85b0f4c20e76afbfd`、Modified blob `9ffdac1baaecac06db20bcc6849bf4cdd165a484`。

配置参数为`-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DEGOMOTION_ENABLE_SVG=ON|OFF`；构建`--parallel 4 -- -k`。CTest均带`-C Release --verbose --no-tests=error --output-junit ...`，MAT组`--parallel 2 -R ...`，其余`--parallel 4 -E ...`，完整正则在`run.sh`及meta。构建缓存分别复用`build/r6-20261117-KzjFHPDQ/cmake`和`build/r6-headless-20261117-Joe73gEd/cmake-off`，两套新证据目录独立，未覆盖旧失败记录。OFF链接/编译清单/符号检查无SVG实现。

两模式输入SHA后验exit0；原树628文件/582MAT、Git树`70ee155131c24a50169346d0ffbaff0c01b1c547`不变。all-state查询仍只有开放Issue #1/#2、没有PR；本次无新增失败，不新建或关闭Issue。未安装依赖、执行MATLAB/Octave、完整科学/L4或性能实验，未合并、发布或正式签署。后续文档提交没有重新构建，受测身份仍为上述源码SHA。

## 仍未完成的范围

P4.9的引擎/可替换平移规则结构缺口已补，候选接口仍未冻结；不把这一项的实现和回归称为R6整体完成。领域不变量D1仍未核销：DEC-008-03的工厂/尽量不可变决定针对Result/Status，不能据此声称FlowField等已改为构造验证；跨模块候选值接口仍需继续落实实际不变量。D2其它Session/球谐/分析职责和D3 Adaptive装配也独立保留。三真实Adaptive入口仍Incomplete、无完整MAT；完整实验/L4、病态数值资格及正式验收未完成。
