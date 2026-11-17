# R6 Bias Runner有界集成验证

## 首轮实测与修复候选

已提交/推送/远端核实 `ff18a3a584c76bb2b63489732968b07f53d78d89`；RunId `r6-bias-baseline-20261117-V6NJqqUX`。配置/编译exit0且无诊断，MAT先5/5、其余27/28，合计32/33。新编排17693项中仅2失败：第一/第五样本的normal故障外层均为SolverFailed而不是既有合同的IncompleteRun；有效前缀0/4、精确样本身份和根因索引均正确。真实发布5558项全部通过，七个MAT和部分写出/无效参数状态正确；两组最大数值残差均6.6613381477509392e-16。每次完整occluded/proportional分别消耗1134/1512个normal，7→11→7输出逐位一致，全部原始输入哈希未变。

**本地缺陷R6-BIAS-001**：`bias_runner.cpp` 共用 `fromSolver` 将RNG/场景生成与真正求解器失败都命名为SolverFailed。保存证据后再次查询全部open/closed Issue/PR为空，未创建远端Issue。完整证据在 `build/r6-bias-baseline-20261117-V6NJqqUX/evidence/`；其余CTest stdout SHA-256 `b82ab6f15b1520b4fb64656b854b55ce5a4792c61765a0eb50cc8eb4ca39d091`。生成MAT、失败Manifest和日志全部保留。

修正将私有包装函数命名为 `fromNumerical`，显式传入外层枚举：真值RNG/场景生成用IncompleteRun，Franz/Original/Modified求解和最终指标仍为SolverFailed；保留Simulation阶段、全部cause链、MATLAB噪声列和level，未改变生成/求解数值、控制流或样本计数。输出的Failed/Incomplete仍由RunContext依据可靠进度独立决定，IncompleteRun并不强制将零进度Manifest写成Incomplete。原两项失败断言不变，另用成功生成的合成零流触发真实KvD的ZeroNorm，确认真正求解失败仍为SolverFailed；待新检查点后复测全33项。

## 首轮准备（历史）：尚未运行

**[HUMAN-EXECUTE/APPROVE]** 起点 `96a695d14b05a19482fc9ee74ac445a43b8e95e8` 本地/远端一致，工作区干净；关联任务仍要求继续R6，全部Issue/PR查询为空。用户授权生产修改及构建/CTest，仍遵守执行前提交/推送/远端SHA规则。前批31/31仅属于 `632139b`；新登记两项后共33项，不能继承通过状态。

本批覆盖TC-F-BIAS-001/002、TC-DC-RNG-001、TC-NF-RNG-001、TC-I-RESULT-001与TC-NF-RECOVERY-001的Runner编排/发布。固定C++ seed7/11、原合法level0..2，保留三档[0.6,2.4,9.6]及全部三个真实求解器；不运行40份level9论文实验、不改变原配置默认值或最终阈值。每样本6或8乘4^level方向，七个输出仍是原save集合。

`r06_bias_runner_contract`用透明采样/噪声包装观察生产调用：验证noise-major顺序、第二次seed重置、N个距离uniform后3N个XYZ normal连续序列、原xwerte在两模式均为6*4^i、六指标及列主序、完成回调、7→11→7可重入。独立std随机流只核本C++工具链，不声称MATLAB同seed同序列；真值以z=2u−1构造，理想流按静态源公式复核。已捕获流的三个求解器重放只检验Runner插槽/编排，误差用独立弦长表达式核对，**不是新的独立求解器L3**。数学L3仍引用原独立KvD测试。

`r06_bias_runner_publication`运行真实生产Runner/采样/噪声/求解器/Writer/Reader/Manifest，在build独立RunId保存有界测试标记，核七个MAT/逐值/shape/SHA/来源种子、1次完成9有效样本。另注入ywerte3写出失败并验证前三个文件留存/Incomplete，非法level拒绝且Failed，无原始数据写入。

负例在第一/第五样本的第三个normal注入失败，保留0/4已完成样本；依据[实验LLD §2](../design/experiments.md)外层应为IncompleteRun，不能把流生成错误称作SolverFailed；当前源码静态可见疑点，先保存真实失败再修复。首轮尚未编译/执行。比较余量沿用既有KvD/Runner的1e-9，不改DEC-005。原始入口blob：occluded=`00a3e974fbf068ccd1af4e541a659862beffcad5`，proportional=`fc3c4f0a7a417171415fb5e3d869a744ba24d04f`。
