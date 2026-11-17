# R6 Bias Runner有界集成验证

## 首轮准备：尚未运行

**[HUMAN-EXECUTE/APPROVE]** 起点 `96a695d14b05a19482fc9ee74ac445a43b8e95e8` 本地/远端一致，工作区干净；关联任务仍要求继续R6，全部Issue/PR查询为空。用户授权生产修改及构建/CTest，仍遵守执行前提交/推送/远端SHA规则。前批31/31仅属于 `632139b`；新登记两项后共33项，不能继承通过状态。

本批覆盖TC-F-BIAS-001/002、TC-DC-RNG-001、TC-NF-RNG-001、TC-I-RESULT-001与TC-NF-RECOVERY-001的Runner编排/发布。固定C++ seed7/11、原合法level0..2，保留三档[0.6,2.4,9.6]及全部三个真实求解器；不运行40份level9论文实验、不改变原配置默认值或最终阈值。每样本6或8乘4^level方向，七个输出仍是原save集合。

`r06_bias_runner_contract`用透明采样/噪声包装观察生产调用：验证noise-major顺序、第二次seed重置、N个距离uniform后3N个XYZ normal连续序列、原xwerte在两模式均为6*4^i、六指标及列主序、完成回调、7→11→7可重入。独立std随机流只核本C++工具链，不声称MATLAB同seed同序列；真值以z=2u−1构造，理想流按静态源公式复核。已捕获流的三个求解器重放只检验Runner插槽/编排，误差用独立弦长表达式核对，**不是新的独立求解器L3**。数学L3仍引用原独立KvD测试。

`r06_bias_runner_publication`运行真实生产Runner/采样/噪声/求解器/Writer/Reader/Manifest，在build独立RunId保存有界测试标记，核七个MAT/逐值/shape/SHA/来源种子、1次完成9有效样本。另注入ywerte3写出失败并验证前三个文件留存/Incomplete，非法level拒绝且Failed，无原始数据写入。

负例在第一/第五样本的第三个normal注入失败，保留0/4已完成样本；依据[实验LLD §2](../design/experiments.md)外层应为IncompleteRun，不能把流生成错误称作SolverFailed；当前源码静态可见疑点，先保存真实失败再修复。首轮尚未编译/执行。比较余量沿用既有KvD/Runner的1e-9，不改DEC-005。原始入口blob：occluded=`00a3e974fbf068ccd1af4e541a659862beffcad5`，proportional=`fc3c4f0a7a417171415fb5e3d869a744ba24d04f`。
