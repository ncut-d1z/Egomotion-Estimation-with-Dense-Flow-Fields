# R6 Bias Runner有界集成验证

## 最终复测：33/33 CTest通过

**[HUMAN-EXECUTE/APPROVE] 受测源码 `c2187539b8cb16fe9d53e770d231a48c2451c0b5` 配置/编译成功，33/33 CTest通过，构建无诊断。** 本段为 `[AI-EDIT]` 事实登记，不代签AC/DEC或冻结。测试前已正常推送并通过GitHub分支API确认远端 `dev-plan-P0-P1-mat-dictionary-probe` 包含该SHA。本地缺陷R6-BIAS-001的两项原失败断言均通过，新增真实零流KvD失败仍为SolverFailed的两项检查亦通过；未改数值公式、阈值、随机消耗或有效样本计数。

RunId `r6-bias-fix-20261117-ZRqFNqCL`，证据目录 `build/r6-bias-fix-20261117-ZRqFNqCL/evidence/`；复用 `build/r6-20261117-KzjFHPDQ/cmake/` 并重新配置源码身份，属于增量构建。环境Ubuntu24.04.5、GCC16.2.0、CMake/CTest4.3.4、Eigen3.4.0、Sophus1.24.6、matio1.5.26、zlib1.3，C++17 Release，无依赖安装。构建前一次SSH推送因临时DNS失败退出128；解析恢复后正常推送成功，确认远端后才运行测试，未将网络问题登记为源码Issue。

| 步骤 | 开始/结束（2026-11-17 UTC+08:00） | 退出码 | 实际结果 |
|---|---|---:|---|
| configure | 18:09:55 / 18:09:56 | 0 | 配置中的源码身份与受测SHA一致 |
| build | 18:09:56 / 18:10:54 | 0 | 全目标通过，无编译诊断 |
| MAT CTest | 18:10:54 / 18:10:55 | 0 | 5/5，0.74秒 |
| 其余CTest | 18:10:55 / 18:10:57 | 0 | 28/28，1.27秒 |
| 输入完整性 | 18:10:57完成 | 0 | 原始资料、独立参考及测试文件SHA一致 |

配置参数 `-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`；构建 `cmake --build <build-dir> --parallel 4 -- -k`。CTest均使用 `-C Release --verbose --no-tests=error --output-junit <evidence>.xml`；先 `--parallel 2 -R '^(r02_mat_reader|r06_historical_mat_readonly_schema|r06_mat_independent_storage_values|r06_mat_reader_boundaries|r06_mat_raw_payload_boundaries)$'`，再 `--parallel 4 -E` 同表达式，覆盖全部33项。完整cwd、命令/参数、时间、工具链、退出码、stdout/stderr、JUnit、输入SHA/Git blob及Testing快照均保留。所有launcher stderr为空。

| 新增CTest | 内部检查 | 最大绝对残差 | 已验证范围 |
|---|---:|---:|---|
| `r06_bias_runner_contract` | 17695/17695 | 6.6613381477509392e-16 | 两种真实采样/噪声/求解器，seed7→11→7、三列噪声、level0..2、六指标/列主序/完成回调、RNG故障前缀及真实求解失败分类 |
| `r06_bias_runner_publication` | 5558/5558 | 6.6613381477509392e-16 | 两种生产Runner各七个MAT逐值/形状/来源/SHA、完整Manifest，第三个y写出失败时保留前三文件并记Incomplete，无效配置记Failed |

每次完整缺损/完整球采样分别消耗1134/1512次normal；两种模式各9个有效样本。复测第一/第五样本故障分别保留0/4有效前缀，外层均为IncompleteRun，真实零流求解失败保持SolverFailed。沿用既有1e-9比较余量，不作DEC-005批准；标准库随机流对照仅对本C++工具链有效。已有31项仍通过，包括原582 MAT/33,340值逐位/SHA、94载荷/29Reader边界、29发布故障及源SH/Adaptive合同。

成功输出位于构建目录 `r06_bias_runner/Bias 中文 合同/output/`，缺损/完整球Complete RunId分别为 `run_d97829d1745571d769952f6c60ab1158`、`run_0db9293d14f57f8778f6303f362e0d0a`；相应部分发布失败RunId为 `run_914779f92eb8dac8a644ce989f31fead`、`run_33d8b8c5cc902f9ee7696e599df9fb73`。所有输出及先前失败记录保留且未提交，原始资料只读。MAT stdout SHA-256 `3a408c6688a97b756a0b02464487d1170866c59f7b9c888709eaad55b123e666`；其余CTest stdout SHA-256 `d0f501802ea7d5d6d3c8c79f99c8b65189d3426c49c9f61341550d9a9a1d36c2`。原 `origin` Git树仍为 `70ee155131c24a50169346d0ffbaff0c01b1c547`。

实际修改为 `src/experiments/bias_runner.cpp` 的错误包装、`tests/integration/test_r06_bias_runner.cpp` 与CMake登记；源码/测试提交依次为 `ff18a3a584c76bb2b63489732968b07f53d78d89`、`c2187539b8cb16fe9d53e770d231a48c2451c0b5`。未创建/关闭Issue或PR，未合并/发布。

**剩余范围**：本批已补足两种Bias有界真实生产链，不能继续将其列为完全未测；但未执行40份level9科学实验、真实完整Adaptive实验或L4。默认Adaptive轨迹在既有Step替身下第151帧UndefinedAngle限制仍存在。静态核查发现 `r01_cli_contract` 仍使用替身Reader/Analyzer、未调用实际可执行文件；当前分析测试亦用合成Reader，尚缺实际CLI装配及历史MAT→Analyzer→JSON/SVG的集成证据。这是下一项具体可执行覆盖缺口，与剩余独立L3、资源资格、DEC-005/006/008及正式验收分开登记。

## 首轮实测与修复候选（历史）

已提交/推送/远端核实 `ff18a3a584c76bb2b63489732968b07f53d78d89`；RunId `r6-bias-baseline-20261117-V6NJqqUX`。配置/编译exit0且无诊断，MAT先5/5、其余27/28，合计32/33。新编排17693项中仅2失败：第一/第五样本的normal故障外层均为SolverFailed而不是既有合同的IncompleteRun；有效前缀0/4、精确样本身份和根因索引均正确。真实发布5558项全部通过，七个MAT和部分写出/无效参数状态正确；两组最大数值残差均6.6613381477509392e-16。每次完整occluded/proportional分别消耗1134/1512个normal，7→11→7输出逐位一致，全部原始输入哈希未变。

**本地缺陷R6-BIAS-001**：`bias_runner.cpp` 共用 `fromSolver` 将RNG/场景生成与真正求解器失败都命名为SolverFailed。保存证据后再次查询全部open/closed Issue/PR为空，未创建远端Issue。完整证据在 `build/r6-bias-baseline-20261117-V6NJqqUX/evidence/`；其余CTest stdout SHA-256 `b82ab6f15b1520b4fb64656b854b55ce5a4792c61765a0eb50cc8eb4ca39d091`。生成MAT、失败Manifest和日志全部保留。

修正将私有包装函数命名为 `fromNumerical`，显式传入外层枚举：真值RNG/场景生成用IncompleteRun，Franz/Original/Modified求解和最终指标仍为SolverFailed；保留Simulation阶段、全部cause链、MATLAB噪声列和level，未改变生成/求解数值、控制流或样本计数。输出的Failed/Incomplete仍由RunContext依据可靠进度独立决定，IncompleteRun并不强制将零进度Manifest写成Incomplete。原两项失败断言不变，另用成功生成的合成零流触发真实KvD的ZeroNorm，确认真正求解失败仍为SolverFailed；待新检查点后复测全33项。

## 首轮准备（历史）：尚未运行

**[HUMAN-EXECUTE/APPROVE]** 起点 `96a695d14b05a19482fc9ee74ac445a43b8e95e8` 本地/远端一致，工作区干净；关联任务仍要求继续R6，全部Issue/PR查询为空。用户授权生产修改及构建/CTest，仍遵守执行前提交/推送/远端SHA规则。前批31/31仅属于 `632139b`；新登记两项后共33项，不能继承通过状态。

本批覆盖TC-F-BIAS-001/002、TC-DC-RNG-001、TC-NF-RNG-001、TC-I-RESULT-001与TC-NF-RECOVERY-001的Runner编排/发布。固定C++ seed7/11、原合法level0..2，保留三档[0.6,2.4,9.6]及全部三个真实求解器；不运行40份level9论文实验、不改变原配置默认值或最终阈值。每样本6或8乘4^level方向，七个输出仍是原save集合。

`r06_bias_runner_contract`用透明采样/噪声包装观察生产调用：验证noise-major顺序、第二次seed重置、N个距离uniform后3N个XYZ normal连续序列、原xwerte在两模式均为6*4^i、六指标及列主序、完成回调、7→11→7可重入。独立std随机流只核本C++工具链，不声称MATLAB同seed同序列；真值以z=2u−1构造，理想流按静态源公式复核。已捕获流的三个求解器重放只检验Runner插槽/编排，误差用独立弦长表达式核对，**不是新的独立求解器L3**。数学L3仍引用原独立KvD测试。

`r06_bias_runner_publication`运行真实生产Runner/采样/噪声/求解器/Writer/Reader/Manifest，在build独立RunId保存有界测试标记，核七个MAT/逐值/shape/SHA/来源种子、1次完成9有效样本。另注入ywerte3写出失败并验证前三个文件留存/Incomplete，非法level拒绝且Failed，无原始数据写入。

负例在第一/第五样本的第三个normal注入失败，保留0/4已完成样本；依据[实验LLD §2](../design/experiments.md)外层应为IncompleteRun，不能把流生成错误称作SolverFailed；当前源码静态可见疑点，先保存真实失败再修复。首轮尚未编译/执行。比较余量沿用既有KvD/Runner的1e-9，不改DEC-005。原始入口blob：occluded=`00a3e974fbf068ccd1af4e541a659862beffcad5`，proportional=`fc3c4f0a7a417171415fb5e3d869a744ba24d04f`。
