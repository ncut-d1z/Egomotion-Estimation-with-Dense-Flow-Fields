# R6 Adaptive Runner 跨帧与发布合同验证

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
