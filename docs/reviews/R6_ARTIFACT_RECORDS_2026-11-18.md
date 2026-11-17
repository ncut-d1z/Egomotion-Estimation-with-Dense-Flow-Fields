# R6：plot/SVG 插图运行记录

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。所有者已授权生产代码、构建/CTest和方案文档修改。本轮补 [98项P审计](R6_PLAN_TASK_AUDIT_2026-11-18.md) D4 中已分配 RunId 的两个导出入口，关联 REQ-NF-LOG-001、P5.3/P8.9/P9.6/P9.7/P10.6。字段为候选实现，未冻结；不改变算法、数值阈值或正式验收状态。

## 实现与边界

- `plot` 在只读 Analyzer 成功、后端存在和 RunId 预留后，先发布 `invocation.json`，确认后才调用导出器。`illustrate --format svg` 对已生成的几何数据采用相同顺序。`record_scope=artifact_export` 明确开始时间覆盖导出阶段，不冒称包含此前的读取/几何计算。
- 开始记录保留真实 argv（直接库 dispatch 为 null）、Config、RunId、源码 SHA、原脚本及辅助 blob、工具链/环境和输出目录。plot 按曲线保留 Analyzer 已绑定的输入路径、变量、1-based run ordinal、角色和 MAT SHA-256；不重开路径，未知摘要保留 null。
- 两入口不消耗科学 RNG，engine/seed 为 null、seed_origin=not_used；`calculation_manifest=null`，不创建计算 Manifest，不冒称实验 Complete。
- 结束记录保存 stdout/stderr 尝试全文、写入/flush状态及条件返回码。完整失败包装链原样序列化。开始记录未确认时不调用导出器；记录自身发布失败返回6，已发布文件保留、不覆盖已有记录。捕获的导出异常保留实际 RunId。
- inspect/analyze/JSON illustration 继续只读，不新增目录。分析/几何失败、后端缺席及 RunId 预留失败发生在本记录范围之外；信号/崩溃仍可能只有开始记录。它们依赖外部执行证据，本轮不关闭整个 REQ-NF-LOG 或 R6。

## 检查点验证准备

扩展现有 `r06_execution_record_contract`：plot和SVG各覆盖成功/直接dispatch、开始/结束linkat及发布后fsync故障、四类PlotErrorCode及嵌套输入身份、导出异常、结束记录冲突、stdout/stderr flush失败。替身测试必须在调用导出器时确认开始记录已经存在。

扩展已有真实CLI测试：历史Bias/Initialization、有限Comparison/UpdateRates、circle/sinusoidal/harmonic。使用CMake独立JSON解析核对真实参数、源码/环境/时间、MAT快照和辅助来源及完整响应；保留原曲线、几何、SVG、输入只读及无后端测试。未新增测试名，不减少原测试义务。

检查点提交并推送、核实远端后，执行 SVG ON/OFF 配置、编译，先MAT7项，再全量其余CTest。实际SHA、命令、RunId、工具链、退出码和结果待完成后登记，不能以本段预期当通过证据。

## 首轮失败及修正

`d2158e2bc30a6a67a7f76483a9c56f2ead12cd07`在远端同SHA检查点下完成SVG ON配置/构建，均exit0且无诊断。MAT7/7、其余47/48，合计54/55；`r06_execution_record_contract`异常终止，CTest exit8，日志为`Result::value(): no success value`。其它真实plot/illustrate的独立JSON检查通过。RunId `r6-artifact-records-20261118-YkxMQPcr`，01:25:57–01:27:37（UTC+08:00），完整证据在同名build目录的`evidence-ON/`，输入SHA后验exit0；未执行此SHA的OFF。

新测试夹具错误使用`--order -2`，而已存在CLI与设计要求`0≤order≤degree`；直接dispatch前未检查解析Result。修正夹具为合法order 2并增加解析成功断言，保留原全部发布/流/异常场景；不改变生产参数范围或算法。all-state检索无重复项后登记 [Issue #2](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/issues/2)，失败日志及沙箱保留。此修正待新远端检查点及双模式复测。
