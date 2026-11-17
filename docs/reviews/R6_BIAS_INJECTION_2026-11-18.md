# R6 Bias 求解器与随机源注入

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。按用户明确授权修改生产代码、构建/CTest和方案文档；关联 P5.7/P10.9、REQ-DC-EXPERIMENT-001、TC-F-BIAS-001/002/003、TC-NF-SESSION-001。实现接口仍是候选、未冻结，不改变数值公式、阈值或正式验收状态。

## 检查点准备

从本地/远端一致的 `4039bfd18cd1f554c4c6e0932cec67f826e59c2a` 接续，分支 `dev-plan-P0-P1-mat-dictionary-probe`。GitHub all-state Issue/PR 列表为空。原始 `origin/**` 保持只读；MATLAB 两入口的第16/22行两次 seeding、第51/52行噪声列/细分层循环仅静态核对，没有运行 MATLAB/Octave。

- `IFranzMatchedFilter` 和 `IKvDEstimator` 分别保留 MotionEstimate/KvDResult；Franz、Original、Modified 三个槽通过借用的 `BiasEstimators` 注入。生产默认服务由 ProductionExperimentRunner 拥有，计算函数不再创建具体求解器。
- Bias compute 接收 `IRandomSource&`，生产调用传入 RunContext 独占的 RNG。`reseed(seed)` 返回 Status，明确不支持重置的流失败，不替换成隐藏 RNG；Mt19937 重置引擎和分布缓存。初始化前、真值后各一次重置，Adaptive 的连续流调用不新增重置。
- 保持唯一采样/噪声配对、原 KvD 参数、六指标/列主序、`xwerte=6*4^level`、callback 与失败传播位置。原 Bias 数值/真实 MAT 发布回归只调整装配调用，不改期望值和阈值。
- 新 `r06_bias_dependency_injection` 使用固定求解替身及可观察 RNG：两模式完整事件序列，三求解槽/六指标映射，seed7/11交错运行，首/次重置、真值/近度 uniform、normal 及各求解器失败的精确前缀，非重置流拒绝，奇数 normal 后缓存重置。Production/RunContext 另用内存 Writer/Publisher 验证七字段、RunId、计数和部分交付；这些替身不构成真实 MAT 持久化证据。

新源码尚未构建/执行。计划完成原子提交、正常推送与远端 SHA 核实后，在两个隔离构建目录执行 SVG ON/OFF 配置、编译和 MAT 优先全量 CTest，预期54/53项。原始文件摘要、命令、环境、开始/结束时间、stdout/stderr/退出码和JUnit保存于独立忽略目录；实测后在下节登记。

## 实测证据

待检查点后执行，不将准备状态写作通过。

## 未覆盖边界

本次补齐任务复核D3的 Bias 求解/RNG 注入缺口；Adaptive其它候选装配边界、D1/D2公共结构、D4非生成日志和D5真实MAT v7.3负例继续保留。完整Bias/L4、三个Adaptive真实完整成功、正式数值/资源资格及冻结未完成。保留既有失败样本，不变更容差，不宣称原MATLAB运行轨迹等价。
