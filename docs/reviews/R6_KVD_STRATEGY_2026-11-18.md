# R6：KvD迭代引擎与平移策略组合

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。所有者已授权代码、构建/CTest及方案文档修改；落实 [PLAN P4.9](../PLAN.md)、REQ/TC-DC-KVD-001及 [98项P审计D2](R6_PLAN_TASK_AUDIT_2026-11-18.md)，不修改数值阈值、原始资料或正式冻结/验收状态。起点本地与远端同`e78d608b7e6d0df4d533c928ecb0dcef561eadad`，工作区干净；关联任务“检查R5实现完成度”仍idle，最新用户要求继续R6，未向其发消息。

## 实现与保真边界

此前两入口使用同一内部函数和枚举，不能实际注入替换平移规则。本轮在`include/egomotion/estimators/franz_kvd.hpp`、`src/estimators/franz_kvd.cpp`实现`IKvDTranslationRule`、`OriginalKvDTranslationRule`、`ModifiedKvDTranslationRule`、`KvDIterationEngine`。两个具名Estimator各组合自己的无状态引擎和明确规则，既有estimate签名不变；引擎每次调用借用策略，不保存引用/流场/跨调用状态，复制Estimator不会形成指向旧对象的内部借用。

规则接收翻号后的near及旧t/omega，返回逐行求和后除N的a/b/c和未归一化raw t。Original保留near*p、near*d、near²投影项；Modified保留p、d、near投影项。共用引擎仍处理Franz初始化、near重估/翻号、唯一一次归一化、使用旧t的旋转更新、提交、预测残差、6/200及动态/绝对停止门限和最终旧near均值缩放。原方向检查1e-8仅提取为命名常量，不改变数值。

策略输入显式拒绝空/错near长度/非有限/非单位视线；策略输出非有限或零raw不会被当有效运动。策略返回的Failure完整透传，不附加虚假包装；分配失败沿既有ResourceLimitExceeded路径。新接口/Estimator布局属候选增量，需重编译依赖，不声称跨旧二进制ABI或正式接口冻结。

## 验证准备（尚未执行）

新增`r06_kvd_strategy_composition`：固定六轴的标量求和参考；负near翻号及每轮调用记录；同一引擎交替两策略、与具名入口逐位比较、trace开关和输入只读；第三轮策略失败/非有限/零候选/分配故障及后续复用；保留内层来源/变量/索引/RunId；替换raw候选验证引擎确实调用策略并负责归一。逐位比较只证明接口编排；算法数值另由原有独立L3/完整trace/停止参考/Bias回归验证，不把自比较当独立Golden。

预计SVG ON56、OFF55项CTest。检查点提交、正常推送和远端SHA核实后，先MAT7项再运行全部其余CTest；配置/编译/执行日志、真实SHA、RunId、退出码及输入哈希将在实测后登记。没有运行MATLAB/Octave或完整科学/L4/性能实验，R6整体未完成。

领域不变量D1仍未核销：DEC-008-03的工厂/尽量不可变决定针对Result/Status，不能据此声称FlowField等已改为构造验证；跨模块候选值接口仍需继续落实实际不变量。D2其它Session/球谐/分析职责和D3 Adaptive装配也独立保留。
