# R6：P8.9三个Adaptive入口的编排与隔离

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。起点`79f304aaf5b4f78aca9e5c2167ba560821bb52b6`，本地/远端分支一致且工作区干净。承接[98项P复核D2](R6_PLAN_TASK_AUDIT_2026-11-18.md)及P8.9、REQ-F-ADAPT-002、REQ-DC-ADAPT-001、REQ-NF-LOG-001、REQ-NF-SESSION-001；用户已授权生产修改、构建/CTest和方案文档。Issue #1/#2/#3开放、无PR，关联任务继续要求检查/实现R6。

## 实现及保真范围

[公共头](../../include/egomotion/experiments/adaptive_runner.hpp)新增InitializationRunner、AdaptiveComparisonRunner、UpdateRateRunner，各只接受自己Config的compute/run。[入口实现](../../src/experiments/adaptive_entries.cpp)由对应类负责Context种类检查、进度绑定、Comparison的Context RNG和既有完整发布门禁；[Production](../../src/experiments/production_runner.cpp)仅装配/分派三个Adaptive入口。旧AdaptiveExperimentRunner保留内存兼容分派，不持有算法状态。

[共用计算](../../src/experiments/adaptive_runner.cpp)仍只实现一份源逐帧流程；每次调用创建自己的聚合，每一轮创建自己的Session及当前结果。Runner只借用场景/采样/噪声/Step，不保存Context、RNG、可写系数或结果；外部服务生命周期必须覆盖Runner，不能推论任意可变替身线程安全。35/600/600帧、级别2/4/2、8轮固定除数、末轮ratio、端点零行、原比较/主状态更新顺序及错误链保持。`read-only-body-audit.json`确认整个共用计算函数体、辅助代码和进度函数体逐字相同；只变归属/分派，不新造第二套算法。候选公共API/对象模型未冻结。

## 实际验证

受测源码 **`34fe509240167a357afc43da71c588534bee005c`**，分支`dev-plan-P0-P1-mat-dictionary-probe`；原子提交、正常推送、GitHub branch API核实SHA且工作区干净后才运行。RunId **`r6-adaptive-entries-20261118-gBryoJ8Q`**，工作目录为仓库根。

| 配置 | 配置/构建 | MAT优先 | 其余CTest | 起止时间（2026-11-18，UTC+08:00） |
|---|---|---|---|---|
| SVG ON | exit0，无诊断 | 7/7 | 56/56，共63/63 | 17:26:12–17:28:16 |
| SVG OFF | exit0，无诊断 | 7/7 | 55/55，共62/62 | 17:28:25–17:30:50 |

新增`r06_adaptive_entry_isolation`使用[既有场景夹具](../../tests/integration/test_r06_adaptive_scene.cpp)，两模式各 **172962项检查**：三个具名对象借用同一组服务，按Init/Comparison/Rates/Comparison/Init/Rates顺序完整交错调用，逐次新Context/RNG/Writer；12种错误Context在场景/RNG/计数/输出前拒绝，非法Comparison Config同样拒绝。第五轮第17帧失败精确保留planned8/attempted5/completed4/valid2407，未发布部分平均；保存Incomplete后复用原Comparison对象，用新Context完整完成八轮，旧错误Manifest、计数、RNG及空结果保持。夹具每轮检查所有主/比较初始系数；周期1/5/11/21、输出列主序、零端点、八轮均值45度及末轮ratio=1/8均按固定输入检查。重置的是外部计数替身，未重建或访问被测Runner内部状态。

原`r06_adaptive_scene_composition`各88121项及54服务故障仍通过，原采样/噪声36088项、独立球谐/运动/单步/Session、MAT/历史分析/CLI/发布等所有测试均执行。新场景使用真实FlowSynthesizer、Session、策略、聚合和RunContext，但Step与Writer/Publisher为替身；它不是科学结果、MATLAB Golden或真实MAT持久化证据。真实Adaptive三入口仍在原acos越界/零旋转定义域返回Incomplete、无MAT；未裁剪、跳样本、放宽阈值或改变除数。

四份JUnit和两模式CTest登记逐名一致，全部status=run、零failure/error/skip/disabled，仅ON多出`r06_svg_source_windows`。OFF源/链接/ELF/静态库检查无SVG实现。每模式 **722项**输入SHA后验全匹配，原628文件/582MAT树仍为`70ee155131c24a50169346d0ffbaff0c01b1c547`；原始文件只读。

环境：Ubuntu24.04.5、Linux7.0.0-43 x86_64、GNU16.2、C++17 Release、CMake/CTest4.3.4、Eigen3.4、matio1.5.26、zlib1.3、Sophus1.24.6。新测试固定输入/固定Random替身，不进行随机统计实验；其它seed来源和独立OS RunId规则另存。命令为`bash build/r6-adaptive-entries-20261118-gBryoJ8Q/run.sh 34fe509240167a357afc43da71c588534bee005c ON`及同参数`OFF`。`evidence-ON/`、`evidence-OFF/`保存环境、命令/参数/起止/退出码、stdout/stderr、JUnit、Testing快照、输入blob/SHA、固定seed来源、缓存、编译/链接与ELF检查；后验为`read-only-junit-audit.json`。复用既有独立构建缓存，未覆盖任何旧失败证据。

P8.9源码职责已落实，D2列明源码结构缺口已补；不代签总体对象/数值验收。D1构造不变量、D4只读/预留前记录范围、真实完整Adaptive成功、完整Bias/L4、资源及正式资格仍未完成，R6未结案。无新缺陷/Issue/PR，没有安装依赖、运行MATLAB/Octave/完整科学或性能实验、改变阈值、冻结、合并或发布。后续文档提交只同步证据，不代表重新构建。

文档静态复核：98个P与PLAN逐项同序，66个TC、45源路径/50声明与修改前身份及顺序一致；TEST_PLAN除顶部事实注记外正文完全相同。12份更新文档的575处相对文件引用均有目标，结果保存为`read-only-document-audit.json`。这不是Bash追踪脚本现场执行或新增CTest记录。
