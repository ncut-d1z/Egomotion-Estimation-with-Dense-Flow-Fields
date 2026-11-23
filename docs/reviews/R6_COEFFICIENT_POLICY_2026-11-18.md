# R6：Session比较系数更新策略

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。继续 [PLAN P8.8](../PLAN.md) 与REQ/TC-DC-ADAPT-001、TC-F-ADAPT-003、TC-NF-SESSION-001，补 [98项复核D2](R6_PLAN_TASK_AUDIT_2026-11-18.md) 的策略缺口。所有者已授权代码、构建/CTest和方案文档修改；起点本地/远端`7a8459aa6025e8fe4ac95293a9870c22024b5392`、工作区干净。关联“检查R5实现完成度”仍idle，最新请求继续实现R6；未发送消息。all-state仍为开放Issue #1/#2，无PR。

## 实现边界

新增`coefficient_update.hpp/.cpp`：`ICoefficientUpdatePolicy::prepare(i,primary,baseline,source_rotation) const`返回自拥有比较系数候选。Reset每帧重置常数项1；Inherit直接复制已完成入口旋转、尚未本帧主求解的系数；Periodic先调用既有`rotateSourceCoefficients`旋转上一帧基线，再按MATLAB帧号模周期从当前主旧系数刷新，默认周期严格为1/5/11/21。刷新帧也不能跳过基线旋转的失败检查。策略不调用求解器、不消费RNG、不拥有/修改主或跨帧基线状态。

Session仍为唯一持久状态拥有者，原构造入口保持可用并拥有默认策略；另可显式注入策略指针数组，复制数组、借用策略对象，后者须比Session活得更久。初始化/普通Comparison使用前两槽，UpdateRates使用四槽；所用空槽在任何策略/求解之前拒绝。Session先按原模式处理主旋转，再准备全部比较候选，依序完成比较及主求解，构造完整成功结果后才提交主/周期基线与帧号。策略失败/非有限候选/分配故障不提交；完整错误链保留。比较`coefficients_next`在原固定单次比较中不作为持久状态，UpdateRates的`dumi`继续丢弃。

周期策略逐槽完成独立旋转/刷新，延续既有C++逐槽顺序；原.m将四次旋转和四次刷新分两循环。所有策略都只读同一主旧值和各自旧基线，所有准备在比较求解前完成，数值依赖没有改变；不声称原MATLAB运行时异常轨迹相同。未新增比较迭代次数、周期CLI参数或数值阈值。接口为候选增量、未冻结，布局变化需重编译，不承诺旧二进制ABI。

原脚本仅静态审查，blob：Initialization `a3154ae391be82ccf48c9f3a03c55109dde85f65`、Comparison `c9548c83fd0a2ce13d96394e98f6c7573119a7d5`、UpdateRates `4629901c1233cdee5d6d22c917603720ffd1fab4`。

## 验证范围（首次准备）

新增`r06_coefficient_policy_composition`：2…43帧、非平凡z旋转、三模式默认/注入策略逐位对照；常数/继承与模周期选择、每槽调用输入/顺序、替换候选实际生效、主状态不可越权、共享策略下Session隔离；每槽Failure/NaN/bad_alloc/length_error、每个比较/主求解失败及同帧重试；空槽/错误帧号/零周期/刷新帧坏旋转拒绝，错误链保留来源/变量/索引/RunId。数值自比较只用于组合证据，不冒称独立Golden；继续保留原独立SH/单步L3、真实Session交错、周期哨兵、Runner与实际CLI测试。

预期SVG ON57、OFF56项CTest。按AGENTS先原子提交、正常推送并确认远端SHA，再分别配置/编译，先MAT7项、后全部其余测试，保存独立RunId/命令/环境/种子来源/退出码/stdout/stderr/输入SHA。当前不得把本段预期记为通过。未安装依赖、运行MATLAB/Octave、完整科学/L4或性能实验，原始资料保持只读。

D1构造不变量、D2其它球谐/分析职责、D3 Adaptive装配及D4只读/预留前记录边界仍保留。真实Adaptive的acos越界/零旋转和完整产物缺口未由本轮改动消除；R6未完成，未冻结/签署/合并/发布。

## 首轮构建失败与修复

固定`4efcf0e395b05801e7a51a582b5f60794999a352`完成推送与远端核验后，SVG ON配置exit0、构建exit2；新增测试三处修改`Result::value() const &`的只读载荷，GNU报`discards qualifiers`/`assignment of read-only location`。生产策略已编译，CTest尚未执行，OFF未执行。RunId `r6-coefficient-policy-20261118-HxGMtusL`，其`evidence-ON/`完整meta/stdout/stderr与输入SHA保留；构建后另行只读SHA后验exit0，未覆盖日志。

检索all-state与本地记录后登记 [Issue #3](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/issues/3)。修正测试为`std::move(result).value()`移出自拥有候选，再修改局部值并重新构造成功Result；不修改生产Result接口、策略代码、阈值或断言范围。待新提交/推送/远端核验后双模式复测，Issue保持开放供复核。

第二轮`daf6c5e91a742ae3b161349f9834cb156b5334fb`在新远端检查点完成SVG ON配置/构建exit0、CTest 7+50=57/57，策略测试46507项通过；但GNU发出`-Wmaybe-uninitialized`，定位同一新增替身`Solver`未初始化返回trace中的`coefficients_next`。不将该构建标为无诊断，也不关闭Issue #3。RunId `r6-coefficient-retest-20261118-Pu5TeopC`，输入SHA后验exit0，OFF尚未执行。追加修复为显式将已构造候选复制到trace，并加入返回trace与候选一致的逐位断言；保留所有已有检查，不关闭诊断选项，待新检查点双模式复测。

## 最终固定源码验证

实现提交`4efcf0e`，只读Result测试修复`daf6c5e`，trace初始化修复及实际受测源码 **`5318cd817aa781345c159bd4ba22ee00fe926ee2`**。每轮配置/构建前均正常推送、GitHub branch API核实同SHA、工作区干净；分支`dev-plan-P0-P1-mat-dictionary-probe`。两次修复只改新增测试及记录，没有变更生产策略、Result接口或数值判据。

| 配置 | 配置/构建 | MAT优先 | 其余CTest | 起止时间（2026-11-18，UTC+08:00） |
|---|---|---|---|---|
| SVG ON | exit0，无诊断 | 7/7 | 50/50，合计**57/57** | 02:15:46–02:16:28 |
| SVG OFF | exit0，无诊断 | 7/7 | 49/49，合计**56/56** | 02:16:35–02:17:57 |

四份JUnit逐名复核全部status=run、零失败/跳过/禁用，与保存的CTest登记一一对应；两配置唯一差别是SVG专用`r06_svg_source_windows`。新策略测试两模式各**48523项检查**：三模式各42帧默认/注入对照，32次策略故障及11个比较/主求解故障位置、同帧重试、独立/共享策略与替换候选等均通过。大部分是精确状态/调用/逐位检查，不把检查数当科学样本数；没有新增或放宽容差。

原独立SH/单步L3、原周期哨兵、真实Session交错9579项（252成功帧/12拒绝帧）、KvD策略与停止/Bias、Runner及真实CLI均回归通过。真实Adaptive仍按既有规则在初始化帧14的acos越界或Comparison/UpdateRates帧151零旋转处返回Incomplete，没有完整MAT；错误合同通过不等于实验成功。

实际环境：Ubuntu24.04.5 / Linux7.0.0-43-generic x86_64，GNU16.2.0，C++17 Release，CMake/CTest4.3.4，Eigen3.4.0，matio1.5.26，zlib1.3，Sophus1.24.6。仅此环境实测，耗时不作为资源预算或性能验收。新增策略测试为固定输入、无RNG；既有种子与调用来源保存于测试源码及日志中的`randomness-source.txt`，RunId继续用独立OS随机源。

RunId **`r6-coefficient-final-20261118-A1AX7yeP`**；工作目录为仓库根，同名`build/`目录的`run.sh`保存完整参数，`evidence-ON/`、`evidence-OFF/`保存环境/Git blob/输入SHA及后验、每步meta/stdout/stderr、JUnit、Testing副本及ELF/链接证据。`read-only-junit-audit.json`保存逐名复核。配置采用`-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DEGOMOTION_ENABLE_SVG=ON|OFF`，构建`--parallel 4 -- -k`；CTest均`-C Release --verbose --no-tests=error --output-junit ...`，MAT7项`--parallel 2 -R ...`、其余`--parallel 4 -E ...`，完整正则与路径在meta/run.sh中。

缓存分别复用`build/r6-20261117-KzjFHPDQ/cmake`、`build/r6-headless-20261117-Joe73gEd/cmake-off`；新证据目录隔离，旧失败/警告记录没有覆盖。OFF编译清单/链接/符号无SVG实现，两模式输入SHA后验exit0，原628文件/582MAT及Git树`70ee155131c24a50169346d0ffbaff0c01b1c547`不变。Issue #3保持开放供复核；后续文档提交没有重新构建。

P8.8的可替换策略缺口已补，候选接口仍未冻结。D1、D2其它职责、D3其它装配与D4记录范围仍待收敛；完整实验/L4/性能/正式资格未执行或完成。本轮未安装依赖、运行MATLAB/Octave、改动原始资料、合并或发布，R6未结案。
