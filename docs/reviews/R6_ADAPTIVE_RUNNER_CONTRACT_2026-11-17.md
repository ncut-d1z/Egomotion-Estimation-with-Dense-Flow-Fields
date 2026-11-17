# R6 Adaptive Runner 跨帧与发布合同验证

## 最新结果：28/28 CTest通过（2026-11-17）

**[HUMAN-EXECUTE/APPROVE] 受测源码 `a5336b3e9072be16dcf296b927a24de3e39cdc06` 已提交、正常推送、核实远端并通过配置/编译及全部28项CTest。** 文档登记为 `[AI-EDIT]`；没有改变AC、数值阈值或冻结状态。远端分支 `dev-plan-P0-P1-mat-dictionary-probe`，没有新增/关闭Issue或PR、没有合并/发布。原始 `origin` Git树仍为 `70ee155131c24a50169346d0ffbaff0c01b1c547`，原始资料始终只读。

RunId `r6-adaptive-errors-20261117-BxUi6ya8`，完整证据在 `build/r6-adaptive-errors-20261117-BxUi6ya8/evidence/`，包括环境、原始输入/固定参考/测试文件SHA、来源Git blob、命令与cwd、stdout/stderr、JUnit和CTest日志快照。工作目录为仓库根，复用 `build/r6-20261117-KzjFHPDQ/cmake/`，是重新配置后的增量构建。Ubuntu24.04.5、GCC16.2.0、CMake/CTest4.3.4、Eigen3.4.0、Sophus1.24.6、matio1.5.26、zlib1.3，C++17 Release；无依赖安装。数值输入及RNG替身固定，无统计种子；RunId使用独立OS随机源。所有生成物在build沙箱，未提交MAT或原始运行日志。

| 步骤 | 开始/结束（2026-11-17 UTC+08:00） | 退出码 | 实测结果 |
|---|---|---:|---|
| configure | 16:50:20 / 16:50:22 | 0 | 受测SHA正确 |
| build | 16:50:22 / 16:51:41 | 0 | 全目标通过，无编译诊断 |
| MAT CTest | 16:51:41 / 16:51:42 | 0 | 4/4，0.61秒 |
| 其余CTest | 16:51:42 / 16:51:43 | 0 | 24/24，0.81秒 |
| 输入SHA核对 | 16:51:43完成 | 0 | 原始资料/固定参考/测试输入均未改变 |

配置参数 `-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`；编译 `cmake --build <build-dir> --parallel 4 -- -k`。CTest使用 `-C Release --verbose --no-tests=error --output-junit <evidence>.xml`，先 `--parallel 2 -R '^(r02_mat_reader|r06_historical_mat_readonly_schema|r06_mat_independent_storage_values|r06_mat_reader_boundaries)$'`，再 `--parallel 4 -E` 同表达式，无跳过登记项。所有步骤stderr为空。MAT日志SHA-256 `82d59a9e593ece1f5e8a4c6c9ef999b698af176d9fe1472a31392b203ceb8311`，其余CTest日志SHA-256 `d9c5b0cccc974b8ce1d093e73bd2600ac45f8496e195303d6a8abf9598084dcf`。

三项新增CTest的**实际证据边界**：

| CTest | 实测通过内容 |
|---|---|
| `r06_adaptive_runner_complete_contract` | 原35帧初始化/99次替身Step/4 MAT（独立atan2最大差 `1.42109e-14` 度）；8组独立固定600×1结果经实际生产聚合及发布门禁，四误差闭式平均精确相等、比值只取第8组；非零合成真值的600帧UpdateRates Session共2,990次Step、原周期刷新、3个当前MAT字段；全部真实Writer/Reader/SHA/Manifest核对 |
| `r06_adaptive_runner_incomplete_contract` | 0～7个完整结果的全部缺样本前缀禁止均值/Complete/MAT，保留实际次数；第8次缺比值/转置/NaN原子拒绝并允许合法重试、第9次及零计划次数拒绝；原Runner的第7个normal故障准确为IncompleteRun→InvalidRandomSample，索引2及帧2保留 |
| `r06_adaptive_default_undefined_contract` | 原600点轨迹+原级别2/4+替身Step下，两模式在第151帧为零旋转真值失败；attempted=1、completed=0、valid=149、undefined=1，保存Incomplete、无MAT。Comparison 448次Step/2,304,000次normal，UpdateRates 746次Step/0normal |

源修改包括两Runner头的可选借用StepSolver、`adaptive_runner.cpp`、私有 `adaptive_aggregation.{hpp,cpp}` / `adaptive_publication.hpp`、`production_runner.cpp`、测试及CMake。修复了undefined漏计、RNG失败被误标求解失败，新增聚合整轮预检和零计划次数保护；原始公式、默认轨迹、采样级别、帧数、八次分母不变。28项通过包括对**预期非成功**的验证，绝不能写成原默认600帧实验已成功。

**R6仍未结案**：原轨迹端到端科学结果不能从固定替身推断；第151帧无定义角仍是原配置在本工具链的可见限制，并未通过改变规则消除。真实完整科学实验、剩余独立L3/L4、其它Reader raw tag/资源负例、源SHRotate动作限定、DEC-005/006及最终人类验收仍待完成。未运行MATLAB/Octave，不要求L1；未签署验收或冻结。下文按历史顺序保留失败、诊断与当时待测状态。

## 失败诊断与修正过程（历史）

**[HUMAN-EXECUTE/APPROVE] 第二轮配置/编译通过且无编译诊断，27项中25通过、2失败；新28项版本待运行。** 受测源码 `cc2f668f8ef49fb618650d110230cdb1ca15a842`，RunId `r6-adaptive-diagnostics-20261117-MMnO6TbY`，完整stdout/stderr/命令、JUnit及输入哈希在同名build目录 `evidence/`。MAT4/4、其余21/23，输入哈希核对exit0。初始化真实流水线已通过（99次替身Step，4个真实MAT）；首次第7个normal故障场景通过。

Comparison和UpdateRates均在 **run1/MATLAB frame151** 返回 `UndefinedAngle`。Comparison实际已完成149帧、调用448次Step、消耗2,304,000个normal；此时尚无完整重复。根因是默认600点正弦轨迹该帧真实相对旋转为零，而替身估计非零，Session依现有合同拒绝零范数夹角。静态原 `normrelfehler.m` 也直接除以向量范数；没有执行原脚本，不能推断其具体运行轨迹。**这不是第8次注入求解器故障，也不能通过填零、跳过帧、缩小样本、钳位或修改轨迹来伪造完整成功。** 全状态Issue检索仍为空，作为本地记录R6-ADAPT-001保存；真实默认求解器或其他工具链可能更早失败，不能把替身下的帧151当普遍保证。

首版测试“默认轨迹每帧夹角都有定义”的假定无效。接续按已有设计 [experiments §8.2](../design/experiments.md#82-第一轮-runnermanifest-的固定替身验收场景af-049设计补齐)拆分证据，不变更验收义务：

- 新 `r06_adaptive_default_undefined_contract` 保留两个原600帧配置和原采样级别，必须在当前固定工具链/替身下准确报告帧151、149有效帧、0完成重复、完整根因及Incomplete Manifest，禁止MAT输出；正例不会替换这个默认路径。
- `r06_adaptive_runner_complete_contract` 保留35帧初始化；Comparison直接输入8组**各不相同**的600×1四误差及比值，复用抽出的私有生产聚合与发布函数，独立闭式验证四误差平均和第8组比值，真实Writer/Reader/SHA/Manifest检查照常。UpdateRates以明确非零合成真值完成600帧Session/2,990次Step和原1/5/11/21周期，验证独立atan2误差和三个当前输出字段。**后两种不是原轨迹端到端科学实验；UpdateRates的akrot由夹具构造，不能据此宣称生产单位转换已经单独验证。**
- `r06_adaptive_runner_incomplete_contract` 验证0～7组的每个缺样本前缀均不除法/不发布，保存实际完成数；第8组缺比值、维度转置或NaN必须原子拒绝，再提交正确第8组可重试，第9组拒绝。RNG第7个normal失败场景仍使用原Runner。

生产聚合仍按原顺序加法、八轮全部成功后除以8，比值只复制最后一轮；前置检查补足字段/shape/非有限值的原子性。仅向私有头提取接缝，不加CLI参数或新配置合同。静态复核同时发现 `noteUndefined()` 原来没有任何生产调用，故Adaptive失败发布门禁现按完整错误链的 `NumericalErrorCode::UndefinedAngle` 增加一次真实undefined计数；不把它算成有效样本。最终门禁先拒绝planned_runs=0，避免检查计数时除零。以上修正及28项CTest须经新远端检查点后验证；下文保留最初目标和失败记录，不能据其旧措辞宣称已全部贯通。

**[HUMAN-EXECUTE/APPROVE] 首次构建成功，27项中25通过、2失败；正在定位/修正。** 起点 `3383b0c4075dc7f5abf214a851c1369a73cc7965`，关联任务最新要求继续R6代码与验证；用户已允许源码、构建/CTest和方案文档。首次读取的本地/远端SHA一致、工作区干净，GitHub全状态Issue/PR为空。上一批受测源码为 `dffddf3b6a62b906a5b20eba365a2087e9c86662`，25/25 CTest通过；新增2项没有通过证据。

## 范围及不变量

**首次28项通过及后续修正**：`0b33cf7d2a5d85a9a9cf1c706f6953ff839a763d` 已完成配置/编译和28/28 CTest，输入哈希不变。复核真实RNG失败JSON时发现其外层仍为SolverFailed；[实验LLD §1](../design/experiments.md)明确禁止把噪声生成失败命名为求解失败，要求保留原NumericalErrorCode并用已有IncompleteRun包装。接续仅为 `nonpropfehler` 分支选择该既有码，根因、阶段、ray索引和run/frame身份不改，更新负例断言验证既定合同。修正后需要新检查点和全量复测，不能继承0b33cf7的PASS。

**接续构建失败记录**：`3349d33a1e736a75441f7f8ce3564c0fb46bca3b` 已推送并核实；RunId `r6-adaptive-aggregation-20261117-nP4IGdxp` 配置exit0、编译exit2，CTest未启动。新私有 `adaptive_publication.hpp` 缺少定义 `ExperimentKind` 的直接include，导致声明不可编译；修复为显式包含已有 `output_contract.hpp`，不依赖包含顺序。原失败stdout/stderr保留，修复版须另建检查点并复测。

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
