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

## 固定源码复测结果

实现提交`d2158e2`，测试修正及实际受测源码 **`00d174258f502d6c458fb26b8e0727d148b98672`**。每次运行前正常推送并通过GitHub branch API核对同SHA，工作区干净；分支`dev-plan-P0-P1-mat-dictionary-probe`。本次只改测试夹具，生产记录实现没有因首轮失败而改变；另外新增真实CLI负order拒绝用例，保留该非法输入的检查。

| 配置 | 配置/增量构建 | MAT优先 | 其余CTest | 起止时间（2026-11-18，UTC+08:00） |
|---|---|---|---|---|
| SVG ON | exit0，无诊断 | 7/7 | 48/48，合计**55/55** | 01:29:52–01:31:08 |
| SVG OFF | exit0，无诊断 | 7/7 | 47/47，合计**54/54** | 01:31:07–01:32:40 |

四份JUnit逐项静态核对全部status=run，零失败/跳过/禁用，唯一配置差异仍为`r06_svg_source_windows`。每模式运行记录专项 **394项检查/38次调用**（原106/12，新增288/26），新增的两入口各13场景全部通过；故障实际命中、调用顺序、原有文件保留、条件退出码和完整嵌套诊断均有断言。OFF使用注入导出器验证合同，不冒称运行了生产SVG后端；实际OFF符号/链接检查未发现SVG实现，真实CLI的后端缺席及不分配RunId检查通过。

实际进程独立JSON验证覆盖两组历史Bias及Initialization、有限Comparison/UpdateRates的五次plot和三种SVG插图；记录的MAT输入逐曲线与已独立核对的Analyzer数据比较，保留路径/变量/1-based序号/角色/摘要及辅助blob。两模式插图CLI均29次调用：ON检查真实产物、OFF检查既定后端缺席；新增负order拒绝不创建输出。只读分析重复调用不增加RunId。原曲线/几何/SVG断言、真实Bias数值、五生成入口记录、输出流/SIGPIPE及MAT73回归均通过。

环境：Ubuntu24.04.5、Linux7.0.0-43-generic x86_64、GNU16.2.0、C++17 Release、CMake/CTest4.3.4、Eigen3.4.0、matio1.5.26、zlib1.3、Sophus1.24.6。仅此工具链实测，不据构建耗时声明资源预算或性能验收。

RunId **`r6-artifact-retest-20261118-f3PrRXse`**；同名`build/`目录的`run.sh`保存完整命令，`evidence-ON/`和`evidence-OFF/`保存每步meta/stdout/stderr、环境、Git blob、种子来源、输入SHA及后验、JUnit、Testing副本和ELF/链接结果；`read-only-junit-audit.json`保存四份JUnit逐名复核。配置使用`-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DEGOMOTION_ENABLE_SVG=ON|OFF`，增量构建`--parallel 4 -- -k`；先MAT七项`ctest --verbose --no-tests=error --parallel 2 -R ...`，其余`--parallel 4 -E ...`。构建目录分别复用`build/r6-20261117-KzjFHPDQ/cmake`和`build/r6-headless-20261117-Joe73gEd/cmake-off`，运行/失败证据目录独立且未覆盖。

两模式输入SHA后验exit0，原树628文件/582MAT、Git树`70ee155131c24a50169346d0ffbaff0c01b1c547`不变。未安装依赖、运行MATLAB/Octave、完整科学/L4或性能实验；没有合并、发布或正式签署。Issue #2保持开放供复核；本报告的后续文档提交不冒称重新构建。D4的已分配RunId导出记录缺口已补；只读/预留前/信号边界仍如上，R6整体未结案。
