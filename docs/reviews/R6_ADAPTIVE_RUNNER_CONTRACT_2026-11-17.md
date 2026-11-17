# R6 Adaptive Runner 跨帧与发布合同验证

## 最新诊断与接续（2026-11-17）

**[HUMAN-EXECUTE/APPROVE] 第二轮配置/编译通过且无编译诊断，27项中25通过、2失败；新28项版本待运行。** 受测源码 `cc2f668f8ef49fb618650d110230cdb1ca15a842`，RunId `r6-adaptive-diagnostics-20261117-MMnO6TbY`，完整stdout/stderr/命令、JUnit及输入哈希在同名build目录 `evidence/`。MAT4/4、其余21/23，输入哈希核对exit0。初始化真实流水线已通过（99次替身Step，4个真实MAT）；首次第7个normal故障场景通过。

Comparison和UpdateRates均在 **run1/MATLAB frame151** 返回 `UndefinedAngle`。Comparison实际已完成149帧、调用448次Step、消耗2,304,000个normal；此时尚无完整重复。根因是默认600点正弦轨迹该帧真实相对旋转为零，而替身估计非零，Session依现有合同拒绝零范数夹角。静态原 `normrelfehler.m` 也直接除以向量范数；没有执行原脚本，不能推断其具体运行轨迹。**这不是第8次注入求解器故障，也不能通过填零、跳过帧、缩小样本、钳位或修改轨迹来伪造完整成功。** 全状态Issue检索仍为空，作为本地记录R6-ADAPT-001保存；真实默认求解器或其他工具链可能更早失败，不能把替身下的帧151当普遍保证。

首版测试“默认轨迹每帧夹角都有定义”的假定无效。接续按已有设计 [experiments §8.2](../design/experiments.md#82-第一轮-runnermanifest-的固定替身验收场景af-049设计补齐)拆分证据，不变更验收义务：

- 新 `r06_adaptive_default_undefined_contract` 保留两个原600帧配置和原采样级别，必须在当前固定工具链/替身下准确报告帧151、149有效帧、0完成重复、完整根因及Incomplete Manifest，禁止MAT输出；正例不会替换这个默认路径。
- `r06_adaptive_runner_complete_contract` 保留35帧初始化；Comparison直接输入8组**各不相同**的600×1四误差及比值，复用抽出的私有生产聚合与发布函数，独立闭式验证四误差平均和第8组比值，真实Writer/Reader/SHA/Manifest检查照常。UpdateRates以明确非零合成真值完成600帧Session/2,990次Step和原1/5/11/21周期，验证独立atan2误差和三个当前输出字段。**后两种不是原轨迹端到端科学实验；UpdateRates的akrot由夹具构造，不能据此宣称生产单位转换已经单独验证。**
- `r06_adaptive_runner_incomplete_contract` 验证0～7组的每个缺样本前缀均不除法/不发布，保存实际完成数；第8组缺比值、维度转置或NaN必须原子拒绝，再提交正确第8组可重试，第9组拒绝。RNG第7个normal失败场景仍使用原Runner。

生产聚合仍按原顺序加法、八轮全部成功后除以8，比值只复制最后一轮；前置检查补足字段/shape/非有限值的原子性。仅向私有头提取接缝，不加CLI参数或新配置合同。静态复核同时发现 `noteUndefined()` 原来没有任何生产调用，故Adaptive失败发布门禁现按完整错误链的 `NumericalErrorCode::UndefinedAngle` 增加一次真实undefined计数；不把它算成有效样本。最终门禁先拒绝planned_runs=0，避免检查计数时除零。以上修正及28项CTest须经新远端检查点后验证；下文保留最初目标和失败记录，不能据其旧措辞宣称已全部贯通。

**[HUMAN-EXECUTE/APPROVE] 首次构建成功，27项中25通过、2失败；正在定位/修正。** 起点 `3383b0c4075dc7f5abf214a851c1369a73cc7965`，关联任务最新要求继续R6代码与验证；用户已允许源码、构建/CTest和方案文档。首次读取的本地/远端SHA一致、工作区干净，GitHub全状态Issue/PR为空。上一批受测源码为 `dffddf3b6a62b906a5b20eba365a2087e9c86662`，25/25 CTest通过；新增2项没有通过证据。

## 范围及不变量

关联 PLAN P8.1～P8.8、P1.7/P1.9、TC-F-EXP-001/002/003、TC-F-ADAPT-002/003、TC-I-RESULT-001。静态核对三份原 `agent_in_sphere_*.m` 的35/600帧、级别2/4、Comparison八次重复、逐帧2/4比较调用加一次主调用、1/5/11/21刷新、两端预分配零、当前save字段和除以8的顺序；原文件仅静态读取。

`AdaptiveExperimentRunner` 与 `ProductionExperimentRunner` 允许借用已有 `IAdaptiveStepSolver`，生命周期由调用者保证；传递给每轮新建的Session。默认空指针与CLI仍使用真实求解器，未增加CLI参数或更改公式、帧数、采样级别、重复数。新增测试替身按run/frame/slot返回预定非对称非零运动，并核对精确调用顺序、每轮状态重置、基线刷新与丢弃比较dumi。替身只验证调度/聚合合同，不是科学算法或独立数值Golden。

| 新CTest | 必须核对 |
|---|---|
| `r06_adaptive_runner_complete_contract` | 三模式原帧数和采样数；Comparison全部8次/4,784有效帧；真实轨迹/环境/光流/噪声路径；零噪声仍完整消耗3N正态抽样；四类误差按独立atan2夹角逐行/逐列核对，八次严格平均；理想rottransratio不被再次缩放，UpdateRates只有当前3字段；真实Writer回读/SHA/Manifest/计数/端点零 |
| `r06_adaptive_runner_incomplete_contract` | 第8次重复第2帧主求解失败，保留7次完成及4,186有效帧，拒绝缺样本平均/任何MAT输出/Complete；首次噪声抽样第7个normal失败，保持0完成与原错误链；分别保存Incomplete/Failed审计 |

角度合同回归沿用既有Session测试 `1e-9` 度余量，参考为 `atan2(norm(cross),dot)`，未改变任何生产或已有测试阈值，不代替DEC-005。几何叶子的再调用用于核对Runner数据传递；完整独立轨迹/求解数值证明是不同证据。

运行只写build中中文/空格的独立RunId沙箱，每次留下 `CONTRACT_TEST_ONLY.txt`；Manifest的来源字段明确写替身标识，不能当MATLAB或真实科学实验结果。原始MAT不写；不执行MATLAB、依赖安装或完整真实求解器实验。失败文件不删除。每次构建/CTest仍先提交、推送、核实远端SHA。

## 待执行及剩余项

待新源码检查点后运行配置/编译、MAT4项及其余23项CTest。原日志保留，不把上批25/25继承为新HEAD结果。完整真实Runner的科学实验、全部L3/L4、原始SHRotate动作限定、其它Reader raw tag/资源负例、最终数值/性能和人类验收仍未完成。

## 首次运行与诊断

源码 `e77eb402ddcd9d4c8fa607999f6ff514cbe97a54` 已提交/推送/远端核实；RunId `r6-adaptive-contract-20261117-O6jKrbJC`，证据在同名build目录的 `evidence/`。构建exit0，但替身 `AdaptiveStepTrace::coefficients_next` 未初始化产生 `-Wmaybe-uninitialized`；修正为复制已构造的替身输出，不关闭诊断。MAT4/4通过，其余21/23通过（exit8），原始输入及参考/测试文件哈希核对exit0。

成功合同用例在Initialization输出检查中失败：测试把 `fileEvidence()` 的vector索引当保存顺序，而真实Writer以变量名map导出；公开合同不要求回执枚举顺序，修正为按变量身份查找，保留所有SHA/字节数/持久化/Manifest断言。第八轮故障用例在预定注入之前已经收到SolverFailed，原输出未保存底层错误；本轮先新增诊断与真实部分计数落盘，并独立收集全部模式，**该原因尚未定位，不猜测为求解器问题或改变断言**。失败日志SHA-256 `8776e862c3b07184f358b30b213bc5804b0921a10eef5f2495693896078dcfd7`。失败后检索全部open/closed Issue为空，未新建Issue；已生成的沙箱保留。
