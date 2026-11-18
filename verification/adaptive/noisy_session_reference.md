# 固定噪声输入与真实Session组合参考

`[AI-DRAFT + HUMAN-APPROVE]`，2026-11-18，L3候选/未冻结。补充SN-G06及AD-G05的有界多帧数值子范围，关联P3.5/P3.7/P8.3、TC-F-FLOW-003、TC-F-ADAPT-002和TC-NF-RNG-001；不修改TC/AC或批准DEC-005。真实运行状态另记PROGRESS。

在[既有多帧参考](session_multiframe_reference.md)的两条12视线固定流上，使用Comparison模式的MATLAB帧2–43。每种系数分别创建新Session和独占固定样本源，系数为入口binary64值`0.0`、`0.1`、`-0.1`，两流合计6条/252帧。原始脚本`help_functions/nonpropfehler.m`只读静态核对，Git blob为`c25d4743f5dc99b4825b738d822457d541813350`；其余单步/源旋转/状态来源与前述说明一致。

每流固定样本按0-based序号k定义：`g[k]=(((17*k+13*stream) mod 29)-14)/8`，k=0…1511。这些是可辨认的测试输入，不是分布合格样本、MATLAB RNG输出或L4统计；两流偏移不同，每个系数分支复用相同输入值以显露符号差异。每帧需36个样本，位置为`36*(matlab_frame-2)+3*row+axis`，不得reseed、取uniform、改为整列x后整列y/z或按噪声符号改变消费。即使系数为0，也必须消耗完整1512样本/流，合计9072。

独立参考先用long double标量计算全部无噪流向量模长的平均q，再对每行做`p'=p+g*coefficient*q`，最后做`p''=p'-(p'·d)d`。顺序和输入值明确来自源公式；它不调用生产噪声、Eigen或生产RNG。参考自身先经两行模长5、尺度0/±1/8、dyadic样本的精确投影值校准。原始固定视线/理想光流已舍入binary64，噪声参考保留long double中间值；真实服务消费相同binary64理想场、系数及样本，自行产生输出。参考Session消费独立投影后的场，不读取真实噪声结果或生产主状态作为Expected。

真实`MeanMagnitudeGaussianNoise`输出每个方向值必须保持，每个流分量按既有sampling_flow绝对`1e-12`工程余量核对；全部Step节点/主和基线状态沿用原`2e-8*(1+abs(reference))`，比较角度沿用`1e-9*(1+abs(reference))`。没有新增/放宽阈值、钳制角度或改变源算法。每个i=5/11/21/42还复用已生成的观测注入真实零旋转角度失败，再用相同观测同帧重试，验证24次Session逐位回滚；**这不声称失败Runner会回滚RNG或允许恢复完整原实验**。

CTest名称为`r06_noisy_session_independent_multiframe`；源为`tests/integration/test_r06_session_l3.cpp`的`--noise`分支，独立标量公式在`tests/support/adaptive_scalar_reference.hpp`。每条序列首帧另调用默认未注入Session，其余调用通过透明记录器返回真实Step结果。原`r06_session_independent_multiframe`继续覆盖三模式源旋转与四周期，不缩减原样本。

沙箱保存reference.jsonl及actual.jsonl的全部252帧。新增字段记录noise_coefficient、first_normal_draw、normal_inputs和ideal_observations；两文件的reference_mean_magnitude均明确是独立参考q，不伪装成生产内部trace。observations分别为独立噪声场与生产真实噪声场，随后保存全部比较/主求解节点、状态和角度。输入样本、输出文件和源码hash随运行证据保存，旧记录不覆盖。

本测试不驱动原轨迹/完整8轮Runner、不输出新科学MAT，也不构造与历史L2同分布的统计样本；AD-G05完整addederror/8-run/最后一轮ratio、真实Adaptive科学成功及正式Golden/容差仍另行验证。负有限系数遵循现有源/生产API合同，不改变CLI参数规则。
