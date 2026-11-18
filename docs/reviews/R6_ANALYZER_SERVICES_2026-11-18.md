# R6：P9.8分析组件职责分离

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。起点为`c12a6613715528a3d8ddce0fff2016bd54096db1`，工作区干净，远端分支同SHA。接续[98项任务D2](R6_PLAN_TASK_AUDIT_2026-11-18.md)的P9.8及REQ-F-AN-002/REQ-DC-ANALYSIS-001；用户已授权生产修改、构建/CTest和方案文档。Issue #1/#2/#3开放，无PR，关联任务仍要求检查并继续R6。

## 实现

`result_analyzer.hpp`增加BiasResultAnalyzer和AdaptiveResultAnalyzer两个无状态具体组件，分别以aggregate和analyze借用IResultReader；在原`bias.cpp`、`adaptive.cpp`拥有实际分析函数体，私有源方法按原次序执行，未增加第二套统计/分段公式。较长Adaptive函数保留各源分支的连续时序，避免把片段状态散落到多个可变对象。原自由入口从私有Reader辅助头移除；`MatResultAnalyzer`拥有两个组件，只分派既有四种kind，CLI和IAnalyzer签名保持。

共同私有`analysis_guard.hpp`保留旧入口的空路径拒绝、bad_alloc/length_error/filesystem_error/out_of_range映射与消息，使直接组件调用也有同样的明确失败。原目录顺序、最后x、跨run分母、MAT形状/有限性、35/600行、严格分段阈值、平滑窗口和原零边缘、历史ratio缺失拒绝均不变。无Writer、RNG、求解器或图像后端依赖；Exporter仍只消费自拥有AnalysisDataset。候选公共API未冻结，不将类名存在视为总体对象验收。

## 首次验证计划

新增`r06_analyzer_service_contract`：四入口两次交替运行，内存Reader固定输入、两份Bias目录及Unicode路径；精确读取顺序、13/2/12/20曲线、最后x与两run均值、Initialization源窗口、完整JSON与既有分派对照、Reader数据释放后的结果自拥有，以及导出替身失败不改变数值。40个Reader故障覆盖四入口首/末输入的Failure及四类异常，保留完整MatError路径/变量/索引/RunId，验证不继续读取；另检空路径、Adaptive收到Bias或非法kind时不访问Reader。对照分派仅证明路由一致，独立历史400文件/10400值与多项式平滑/分段/CLI/实际SVG回归保留。

先原子提交、正常推送并核实远端SHA，再新RunId双模式配置/编译，先MAT7项、后其余全部CTest，预期ON62/OFF61；首次尚未执行。记录完整环境、命令、源SHA、输入摘要、固定seed来源、stdout/stderr、起止时间、退出码及JUnit。新夹具仅读取内存值、创建隔离空目录，无实际MAT写入；全套原I/O测试在既有授权隔离沙箱运行。不安装依赖、不运行MATLAB/Octave/完整科学或性能实验，不修改阈值/原始资料、不冻结/合并/发布。
