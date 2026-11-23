# R6：P9.8分析组件职责分离

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。起点为`c12a6613715528a3d8ddce0fff2016bd54096db1`，工作区干净，远端分支同SHA。接续[98项任务D2](R6_PLAN_TASK_AUDIT_2026-11-18.md)的P9.8及REQ-F-AN-002/REQ-DC-ANALYSIS-001；用户已授权生产修改、构建/CTest和方案文档。Issue #1/#2/#3开放，无PR，关联任务仍要求检查并继续R6。

## 实现

`result_analyzer.hpp`增加BiasResultAnalyzer和AdaptiveResultAnalyzer两个无状态具体组件，分别以aggregate和analyze借用IResultReader；在原`bias.cpp`、`adaptive.cpp`拥有实际分析函数体，私有源方法按原次序执行，未增加第二套统计/分段公式。较长Adaptive函数保留各源分支的连续时序，避免把片段状态散落到多个可变对象。原自由入口从私有Reader辅助头移除；`MatResultAnalyzer`拥有两个组件，只分派既有四种kind，CLI和IAnalyzer签名保持。

共同私有`analysis_guard.hpp`保留旧入口的空路径拒绝、bad_alloc/length_error/filesystem_error/out_of_range映射与消息，使直接组件调用也有同样的明确失败。原目录顺序、最后x、跨run分母、MAT形状/有限性、35/600行、严格分段阈值、平滑窗口和原零边缘、历史ratio缺失拒绝均不变。无Writer、RNG、求解器或图像后端依赖；Exporter仍只消费自拥有AnalysisDataset。候选公共API未冻结，不将类名存在视为总体对象验收。

## 首次验证计划

新增`r06_analyzer_service_contract`：四入口两次交替运行，内存Reader固定输入、两份Bias目录及Unicode路径；精确读取顺序、13/2/12/20曲线、最后x与两run均值、Initialization源窗口、完整JSON与既有分派对照、Reader数据释放后的结果自拥有，以及导出替身失败不改变数值。40个Reader故障覆盖四入口首/末输入的Failure及四类异常，保留完整MatError路径/变量/索引/RunId，验证不继续读取；另检空路径、Adaptive收到Bias或非法kind时不访问Reader。对照分派仅证明路由一致，独立历史400文件/10400值与多项式平滑/分段/CLI/实际SVG回归保留。

先原子提交、正常推送并核实远端SHA，再新RunId双模式配置/编译，先MAT7项、后其余全部CTest，预期ON62/OFF61；首次尚未执行。记录完整环境、命令、源SHA、输入摘要、固定seed来源、stdout/stderr、起止时间、退出码及JUnit。新夹具仅读取内存值、创建隔离空目录，无实际MAT写入；全套原I/O测试在既有授权隔离沙箱运行。不安装依赖、不运行MATLAB/Octave/完整科学或性能实验，不修改阈值/原始资料、不冻结/合并/发布。

## 固定源码双模式实测

受测源码 **`03f7f81b6605a4492890a44e9e28290b8f3d785d`**，分支`dev-plan-P0-P1-mat-dictionary-probe`，正常推送并经GitHub branch API核实同SHA、工作区干净后执行。RunId **`r6-analyzer-services-20261118-0WFb9nRp`**，工作目录为仓库根。

| 配置 | 配置/构建 | MAT优先 | 其余CTest | 起止时间（2026-11-18，UTC+08:00） |
|---|---|---|---|---|
| SVG ON | exit0，无诊断 | 7/7 | 55/55，共62/62 | 17:09:35–17:12:10 |
| SVG OFF | exit0，无诊断 | 7/7 | 54/54，共61/61 | 17:12:19–17:14:30 |

新组件测试两模式各 **727项检查**，四入口两轮交替、完整读取顺序和JSON、40个Reader故障前缀、叶子来源以及自拥有/导出隔离全部通过。新夹具仅有内存数组和隔离空目录；其固定SHA字符串只是Reader提供的来源传播夹具，不能冒称真实MAT身份。已有历史400文件/10400值独立统计、有限多项式平滑、分段、真实分析CLI/SVG、球谐/单步/Session/八轮合同及原MAT/Writer等回归全部执行。真实三Adaptive仍Incomplete、无MAT，未改变原acos越界/零旋转处理，不宣称完整科学成功。

四份JUnit与两模式CTest登记逐名一致，全部status=run，零failure/error/skip/disabled；ON唯一多出`r06_svg_source_windows`。OFF编译源、链接列表、ELF符号及静态库检查无SVG实现。各722项输入SHA后验全匹配，原628文件/582MAT树仍为`70ee155131c24a50169346d0ffbaff0c01b1c547`。`read-only-body-audit.json`核对两套原分析函数体完整保持，只改变方法归属；原始资料没有改动。

实际环境：Ubuntu24.04.5、Linux7.0.0-43 x86_64、GNU16.2、C++17 Release、CMake/CTest4.3.4、Eigen3.4、matio1.5.26、zlib1.3、Sophus1.24.6。新测试固定输入、无RNG；其它seed来源另存，RunId独立OS随机。启动命令为`bash build/r6-analyzer-services-20261118-0WFb9nRp/run.sh 03f7f81b6605a4492890a44e9e28290b8f3d785d ON`及同参数`OFF`。`evidence-ON/`、`evidence-OFF/`保留环境、所有步骤命令/参数、起止时间、退出码、stdout/stderr、JUnit、Testing快照、输入blob/SHA、随机来源、CMake缓存、编译/链接和ELF；后验逐名核对为`read-only-junit-audit.json`。复用原独立构建缓存，没有覆盖旧失败记录或其它运行证据。

P9.8源码职责缺口已补，正式对象/数值验收保持未签署。D1构造不变量、D2其余Runner职责、D4记录范围及完整实验/资格仍未完成，R6未结案。本轮无新构建/测试缺陷；再次all-state核实Issue #1/#2/#3开放，无PR。后续文档提交只同步事实，不代表重新构建。

文档静态复核：98个P与PLAN逐项同序，66个TC及45源路径/50声明与变更前身份和顺序一致；TEST_PLAN除顶部事实注记外正文完全相同。11份更新文档的522处相对文件引用均有目标，结果为`read-only-document-audit.json`。这不是Bash追踪脚本现场执行或新增CTest记录。
