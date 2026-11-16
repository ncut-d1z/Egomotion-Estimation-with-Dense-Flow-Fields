# 实施进度与证据登记（PROGRESS）

> 滚动证据登记 v0.6（截至E-DOC-069，2026-11-10），非正式基线批准。**这里的“本轮”是E-DOC-069历史数学批次，不是2026-11-11正在开展的全工程文档规范审计。** 该历史批次仅作理论推导和Wolfram验证，未实施生产C++；历史原生SH测试与数学运算的执行者、源码SHA及证据类型必须分开。本次文档审计最新状态见[AUDIT_PROGRESS.md](AUDIT_PROGRESS.md)，不得用本页旧97AF计数替代现行全工程审计的DOC/DAR口径；现行数字及延期分类只以[AUDIT_PROGRESS.md](AUDIT_PROGRESS.md)最新台账为准，历史9DAR属于旧批次而非当前总数。
>
> 为消除旧NOT_RUN、旧授权与动态计数反复堆叠，既有E-DOC-054～067详细记录完整保存在[9825375固定PROGRESS](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/982537556be1ac5a414af368a0082806a9179d32/docs/PROGRESS.md)。更早E-DATA-001、E-DOC-001～053保存在[98e95e2固定历史](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/98e95e2f867dc7142fa0d5ac0f1c79f5d6429ed1/docs/PROGRESS.md)。未改写任何Git历史或历史审批；下表保留定位和关键事实，E-DOC-068为上一轮、最新证据为§5的E-DOC-069。

## 0. 2026-11-15 R0/R1 新编程批次（尚无原生运行权限）

**新授权**：项目所有者已解锁H-10的全工程C++代码编写/提交权限（参见[TODO](TODO.md)和[ROADMAP](ROADMAP.md)）；H-11仍单独延期，**本次只编写/提交代码、CMake和测试源码，不进行CMake配置、编译、CTest或业务CLI运行**。以下§1～6是原E-DOC-069历史数学批次记录，不随本批次追溯改写。

**本批次交付**：R0解析`inspect-mat`、2种Bias、3种Adaptive、`analyze`和可选`plot`命令，校验必需/重复/未知参数、uint32/level/有限噪声及安全RunId；现无真实后端时返回有明确消息的失败而非伪造Complete。R1提供C++17的Eigen自拥有值类型、Result/Status（禁默认构造、明确逻辑错误）、`std::forward_list<ErrorFrame>`完整包装链与JSONL、Reader/Writer/RNG、Runner和Analyzer抽象接口。根CMake保留原SH静态库/3测试，同时增加CLI程序、公共接口与7个INTERFACE占位模块、2个新增合同测试源码，未实际执行测试。未实现matio生产Reader/Writer、采样/KvD/Adaptive、真实RunContext/Manifest及结果写入，这些属于R2～R5。

**静态CMake检查**：源文件路径存在、11模块目标命名及依赖非环、旧3+新2测试对象均已登记；**不等于CMake configure/compile通过**。没有实际运行环境、工具链或日志的成功证据，不登记虚构PASS。详细批次与依赖以[ROADMAP](ROADMAP.md)为准。

## 0.1 R2 MAT Reader复用与旧探针重构（2026-11-15）

**已编写/已推送，不是已编译/已运行**：`src/mat_data_dict_probe.cpp`仅解析旧位置参数和`--expected-count/--source-sha/--replace-generated`并返回诊断/退出码，不再调用`Mat_Open/Mat_GetDir/Mat_VarRead`；`src/io/mat_data_dictionary.cpp`负责遍历输入、FNV64、旧格式Markdown报告和生成文件保护；`src/io/mat_result_reader.cpp`统一负责matio RAII及两条输入能力：`inspect`用`Mat_VarReadInfo`**仅读结构**，`readNamed/readFirstCompatible`用`Mat_VarRead`**完整解码、列主序拷贝到自拥有double数组**。字典扫描复用同一Reader的`VerifyDecoded`模式，仍在单个Mat_Open句柄内逐变量验证读取状态。公共`IResultReader`无matio类型；现有`egomotion inspect-mat`经Composition Root装配真实Reader，不回读或展示数值。

**工程与测试源码**：CMake的`egomotion_matio`由空INTERFACE改为实际STATIC，私有链接`PkgConfig::MATIO`与`ZLIB::ZLIB`；新`mat_data_dict_probe`目标以及`r02_mat_reader`只读原MAT夹具测试已登记，保留既有5项CTest。历史手动`g++ src/mat_data_dict_probe.cpp`**单文件编译已不适用**；需编译/链接三个源文件（含`src/io/`）和Eigen/matio/zlib，或通过CMake构建。详细命令见[ROADMAP R2](ROADMAP.md)。

**未运行的证据与限制**：H-11仍延期，本轮没有执行`cmake`、`g++`、CTest、旧582文件数据字典重建、独立L2逐值/SHA-256或任何MAT写入；当前原582/582扫描结果属于旧探针历史版本，不能自动继承为本次新代码PASS。CMake静态检查涵盖源文件存在、target命名及非循环边，不能保证编译/链接；正/负例和元数据压缩头行为需真实运行复核。**R2代码就绪待原生验证，R3尚未开始。**


## 0.2 最新顺序决定与R2代码闭合、R3源码推进（2026-11-15）

**所有代码先行**：用户本轮明确要求[ROADMAP](ROADMAP.md)把构建、测试、数值运行、测试源码收尾和验收全部集中R6。R0–R5只做**代码和静态设计复核**，不得以每阶段需先编译/CTest为前置；H-11仍没有原生运行及MAT写入授权。旧章节中“先构建/验证再做R3”的文字只反映形成时的历史建议，本段及ROADMAP的新顺序优先。

**R2代码复核与实修**：保留CLI分离、Reader单MAT句柄、结构`Mat_VarReadInfo`与数值`Mat_VarRead`独立职责；补上`readNamed`检测同名重复变量返回`AmbiguousVariable`及数值读取只接受MAT v5的拒绝门禁（结构元数据枚举仍可报告不同版本）。R2代码范围当前没有已知阻止继续实现R3的未处理缺陷；**这是静态源码判断，不是R2数值等价/原生通过**。

**R3已提交的计算源码**：
- `src/sampling/`：按源面表和A/B/C/D深度优先产生完整/缺损八面体与二十面体视线；`getvektor`两次uniform；独立`Mt19937RandomSource`支持seed重置。标准库normal分布**不承诺与MATLAB同seed逐样本一致**。
- `src/simulation/flow.cpp`：近度/理想流与分量平均、两种互斥三维噪声/切平面投影，严格N次uniform后3N次normal，零噪声系数仍消耗样本；结果保存理想/含噪流、全部近度和末次源标量，处理非有限/非法方向/内存失败。
- `src/estimators/franz_kvd.cpp`：Franz三轴响应/分母、Original与Modified KvD独立命名入口；near负和翻号、旧t算旋转、一次t归一、至少6/通常最多200次及终局mean(near)缩放，提供可选逐轮trace。
- `src/experiments/bias_runner.cpp`：通过构造注入**一组**采样器/噪声模型；每次Bias实验先生成truth t/ω再用同seed重置RNG，三档系数、0..level顺序、两种算法和六类`transfehler_div`归一误差保存至自拥有`xwerte/ywerte1..6`列主序内存，不触碰MAT文件。

**R3与R5的分界**：这一轮的`BiasExperimentRunner::compute`只返回内存`BiasRunData`，**没有**把新结果伪装为`RunOutcome::Complete`。CLI的`run-bias-*`到具体RunContext/Writer/Manifest装配与实际MAT保存由R5实现；本轮新C++文件还没有进入根CMake目标的编译源清单，按用户指令在**全部R0–R5代码完成后再于R6一次性接入并构建/测试**。当前不能报告R3/完整业务可运行或MAT结果正确；R4/R5未开始。

**执行证据**：所有新生产文件已提交、推送；本轮**没有**运行CMake、g++、CTest、程序、MATLAB、MAT读取/写入或任何数值/性能测试，也未修改`origin/**`。此前R2注册的只读CTest仍只是测试源码登记，不能算已通过。详细本轮Git提交见该分支提交历史。


## 0.3 R3完成静态源码缺口修复，进入R4实作（2026-11-15）

**范围判断**：按[ROADMAP](ROADMAP.md)定义，R3只要求**内存**Bias业务计算（不包含R5的持久化Writer、RunContext/Manifest及CLI完整装配）。从`ec0a3a74f9fe`复核后，对实际源码执行多次修正：

- `src/sampling/directions.cpp`/`src/simulation/flow.cpp`/`src/estimators/franz_kvd.cpp`：全局失败的`observation_index`不得默认伪造0；明确每个源uniform/normal失败的观测/分量与真实包装链，修正参数/资源超限分支并保留源MATLAB采样顺序。
- `generateBiasFlow`：距离抽样恢复源`entf=(rand*(4-1)+1)/2.5; near=1/entf`的浮点操作顺序，避免把代数等价的`2.5/(1+3*rand)`误当逐步骤复现；先N次uniform、再3N次normal不变。
- `BiasExperimentRunner`：明确`subdivision_level`和`noise_column_matlab_1_based`属于不同身份，不误用观测下标；失败带上`ywerte1..6`指标，零真值不记假0误差，资源分配失败保留完整数值根因。
- 仍由Composition Root持有/注入唯一相配采样器+噪声模型；R3源码范围已具备三种有序采样、两类互斥噪声、Franz、Original/Modified KvD以及两类Bias的六组内存输出。**“R3源码范围闭合”只是静态检查判断，不是已成功编译/运行；如果R6发现实际错误仍须重新打开修复。**

**R4新增生产源码**（都已提交推送、尚未编译）：`include/egomotion/trajectory/trajectory.hpp`与`src/trajectory/{trajectory,kinematics,sphere}.cpp`实现circle/sinusoidal源N分母、有效中间帧`Rᵀ`局部运动、Sophus SO(3) exp接口和严格球内正根；对于原`drehvektor_aus_zwei_vektoren`精确反向会返回伪零的问题，新路径采取**显式InvalidDirection失败**并标识这是C++安全边界而非MATLAB原有处理。

`src/harmonics/adapter.cpp`重用已有l≤2叶子的3×5↔9系数转换、重建、原source逐通道二阶拟合及SH旋转，不复制球谐公式。公共系数旋转参数的R/Rᵀ约定**必须由未来Runner按原调用方明确选择**，不可反向猜测。

`src/estimators/adaptive_step.cpp`按原`oneit_adaptiv_matched_filter`顺序计算near_raw/s/翻号与归一、6×6**一般非对称**M1–M4及RHS、FullPivLU求解、t单位化、原返回MSE、1e−11新near与二阶系数拟合；不用错误的LDLT对称化。FullPivLU与源`real(inv(M))*b`的浮点差异和病态阈值需在R6另行核验，DEC-005不能先虚设PASS。

`src/estimators/adaptive_session.cpp`将Initialization、Comparison、UpdateRates的主状态/比较状态分离：UpdateRates按MATLAB **1-based** i周期1/5/11/21刷新，比较调用的`dumi`新系数不提交；仅当全部比较与主求解完成后提交主系数及持久基线，失败保留之前状态与错误链。**R4尚未完成三类Runner的完整场景生成、八次聚合及实际内存输出；R5的MAT写入/CLI也未接入。**

**执行约束**：本批严格**没有**调用CMake、编译器、CTest、Wolfram/数值实验、MATLAB或读写任何MAT；`origin/**`原始文件未改。R0–R5仍只编写实现，R6最后统一准备构建/测试源码及执行；本次没有新PASS或代码验收结论。


## 0.4 R4内存计算源码闭合复核与R5第一批输出合同（2026-11-15）

**本轮起点**：`0ea0f0324514d7e06691fe45a155c8e774e1d3b3`。先读实际`src/trajectory`、球谐适配、Adaptive Step/Session，再逐行核对三份原只读MATLAB的默认场景、循环、当前`save`与错误统计。R4最大缺口为**三类Adaptive Runner完全不存在**，已新增：
- `include/egomotion/experiments/adaptive_runner.hpp`和`src/experiments/adaptive_runner.cpp`：Initialization用35点circle(0.6,0.3)/level2/单次；Comparison用600点sinus(.5,1,2,.3)/level4/八个完整run，依次累积四类角度误差，**仅全部八次完成后固定除8**，理想流平移/旋转平均模长之比`rottransratio`只取最后一轮，`nonpropfehler`即使scale=0仍消耗3N normal；UpdateRates用600点/level2，四周期1/5/11/21，对照求解器返回的`dumi`仍丢弃，当前save只有两组600×4误差及600×1的`akrot`（deg），绝不补造HistoricalOnly字段。所有输出为自拥有列主序double矩阵，首尾按原MATLAB的zeros保持0且不计有效估计。
- `include/egomotion/estimators/adaptive_session.hpp`与`src/estimators/adaptive_session.cpp`：新增MATLAB从i=2开始的严格连续帧资格检查，成功提交时才推进下一帧；所有比较及主求解完成前，主系数/四份周期基线均不改变。每次R4`compute*`只有完整内存结果才能返回成功，不允许在失败alternative假装返回完整`Complete`。
- 再修复Comparison**移动理想流后读取分量均值**的生命周期瑕疵，把`mean_rot/mean_trans`提前取得；累积完一轮后才增加`completed_runs`，第八次之外不提交末次比值。

**本轮已转入R5代码**（非R6的测试实施）：
- `output_contract.cpp`按实验类型拒绝旧归档`HistoricalOnly`及跨入口错字段；在任何Writer调用前核对完整当前save集合、内部变量名、MAT double/shape/bytes、有限数值；
- `output_values.cpp`将R3的7个Bias矩阵和R4的4/5/3个Adaptive当前字段转换为自拥有公共领域值；Comparison必须保留**8/8**才具备发布资格；
- `run_context.cpp`独占RunId/RNG、持有CompositionRoot保障生命周期的Writer引用，保留全部错误链和独立文件确认记录。先准备当前save字段和确认列表容量，再逐个交给IResultWriter；部分写入只标内存`Incomplete`草稿，**不得产生假的正式Manifest/RunOutcome Complete**。真实安全文件发布、SHA校验、Manifest持久化和CLI全链接仍属R5未完成事项。

**残余资格与代码状态**：当前R4的三种在内存中的实验计算流程已按原入口结构静态闭合，但由于H-11未授权，本轮**没有**执行编译、CTest、真实MAT读写、MATLAB、Wolfram或数值实验，也未更改`CMakeLists.txt`。该判断不是原源与新物理球谐旋转作用逐值等价的声明（旧`transfor_koeff/SHRotate`按[球谐LLD](design/harmonics.md)存在明确相位/动作差异），亦不保证数值病态、异常和性能行为；R6必须独立限定验证，不通过应重开代码修正。真实CMake目标/测试源码收尾全部留R6，`origin/**`保持只读。

## 0.5 R5代码范围完成静态核查，继续R6测试源码（2026-11-16）

**本轮核查起点**：`0d3b6071844ac5799ded0fed8553d31e661cb40b`；依据真实文件与近十次R5/R6提交复核，而非沿用§0.4和ROADMAP中早期R5快照。核实R5的五种业务Runner全部连接到真实CLI Composition Root，MAT v5 Writer带Linux dirfd锚定、禁止覆盖、回读和SHA-256证据；RunContext具有全字段白名单与shape校验、Complete Manifest资格及独立Failed/Incomplete失败记录。四类只读Analyzer分别处理Bias、Adaptive Initialization、Comparison、UpdateRates，输出结构化JSON；SVG适配只消费数值曲线，不阻塞独立`analyze`。**R5在ROADMAP定义的“代码实现部分”静态闭合，尚不表示已编译或原生验收**。源码发现仍须在R6用真实测试核对的安全竞态、数据等价和异常行为，若失败则重新打开R5修复。

**R6本轮真实提交的源码工作（3个新测试文件，1次CMake登记）**：

- `tests/integration/test_r06_sampling_flow.cpp`：三类采样器level 0/1/2计数、单位视线和缺损八面体逐行保序、getvektor两次uniform、Bias内存近度与末项标量、零高斯3N正态抽样、NoNoise区分、理想流分量抵消与component proportional噪声、RNG耗尽错误链。
- `tests/contracts/test_r06_session_contract.cpp`：三种Adaptive Session在非法帧次序/非有限旋转/空流求解失败时不推进帧、不提交主球谐系数或四组周期基线；验证根因ErrorFrame和Comparison非法参数不消耗RNG。
- `tests/integration/test_r06_analysis_segments.cpp`：合成600行ratio与误差数组，检查Comparison固定四段边界、阈值等值不触发跳变、平移/ratio的501个连续点、UpdateRates旋转局部分段窗口及零边缘、平移全600行平滑后裁剪，以及非法分段和NaN ratio的明确拒绝。
- `CMakeLists.txt`：将以上3项登记为`r06_sampling_flow_contract`、`r06_adaptive_session_transaction`、`r06_analysis_segments_smoothing`，原有9项CTest登记保持不变，**共12项登记，不等于通过12项**。

**严格未执行项**：未运行CMake configure、编译、CTest、CLI、真实MAT读写、MATLAB/Octave、Wolfram、L2/L3/L4、性能或数值测试；没有MAT生成或改动`origin/**`。H-11执行权限未得到本轮授权。R6下一阶段仍须在获许可后统一原生构建并修复发现的实际错误，执行真实Writer路径/竞态负例与完整数据来源验证，补齐独立L3逐节点、历史L2/必要L4，核验SHRotate相位/动作差异及DEC-005/006、冻结和最终验收。此前固定SHA的3项SH CTest与旧582/582探针不能冒充本HEAD新增测试的PASS证据。

## 0.6 R6源码覆盖继续补充（2026-11-16，所有新测试未运行）

**核查起点**：`bd229ecf75e5af432fba9720eae3d486768d58c6`。按[ROADMAP](ROADMAP.md) R6仍需完成统一CMake、单测/集成/独立L3候选源码及获授权后的原生验收。R5代码静态闭合的声明保持限定；**R6代码实现与原生验证均未完成，不存在已批准R7批次**。本次实作不是补充空壳计数，实际新增下列C++测试文件：

- `tests/integration/test_r06_kvd_l3.cpp`：将既有 `verification/kvd/six_direction_exact_trace.v1.json` 与 `near_sign_flip_exact_trace.v1.json` 的独立Wolfram结果组织成三组6/8方向固定流。检查Franz各轴分母与初态、Original/Modified首轮不同的原始t更新、非零近度、符号翻转、六轮迭代停止/continue、预测光流shape、MSE与RMS分类、末次近度均值缩放；不借被测算法生成参考数据。
- `tests/integration/test_r06_adaptive_l3.cpp`：将 `verification/adaptive/asymmetric_signed_near_one_step.v1.json` 的正/负均值九方向、3×5原CS系数、切向流及独立M1/M2/M3/M4、rhs、6维解、归一t、旋转、MSE、新near、九新系数写成固定输入。负均值仅作非物理有符号对照，不宣称正距离普适失败；阈值是未批准L3回归诊断而非DEC-005最终容差。
- `tests/integration/test_r06_archive_readonly.cpp`：对现有582 MAT只读扫描、单变量double及列主序自拥有数据、5种历史shape分布和SHA-256可计算性进行原生测试准备；**不触碰原文件写入、不作独立逐值Golden或声称已核验预期SHA**。
- `tests/integration/test_r06_sh_adapter.cpp`：验证AN-G08的90°物理旋转、`near_new(d)=near_old(R*d)`参数作用、反方向Rᵀ鉴别及往返与反平行失败；不把旧源SHRotate相位/作用差异抹去或虚报等价。

**CMake实修**：上述四项独立CTest均已接线，登记条目由12增为**16**，但**本分支新CTest实际通过项仍为0（没有执行）**。同时核查 `tests/integration/test_r01_cli.cpp`，发现旧R0占位消息`not implemented`和`plot`退出码的过时断言，与R5现行Composition Root实际错误映射不一致；实际修正3处断言，另外加入具有假Analyzer但无SVG后端时`analyze`成功、`plot`退出7的隔离合同断言。不是只修改PROGRESS记录。

**执行与结案门禁**：本轮没有执行CMake/g++/CTest/业务CLI/MAT读写/数值/性能实验，也没有新运行日志或PASS；没有修改`origin/**`。H-11仍待明确授权。R6余下包括未完成的原生编译修复、真实Reader/Writer安全/权限竞态、L2独立逐值及完整可信文件哈希、剩余L3多轮/多帧和SHRotate独立相位/动作验证、必要L4统计、DEC-005/006与最终冻结/发布；真实执行若失败需回到业务源码修复。旧固定SHA的SH 3/3和旧582/582探针绝不能冒充本轮16项通过。

## 1. 当前边界

仓库 `ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields`，分支 `dev-plan-P0-P1-mat-dictionary-probe`；不合并main、不发布。`origin/**` 原45份.m与582份.mat只读；不要求执行MATLAB/Octave或获得L1。既有66REQ/66TC/98P、27条HLD设计边不变；数学子案例不新增正式TC。

本轮无C++、头文件、CMake、CTest、构建脚本或MAT的修改和原生构建运行。Wolfram中的代数与高精度数值是实际执行的数学检查，不称作生产Adaptive实现、C++单测或批准Golden。动态批准事项仍归 [TODO](TODO.md) 与 [DOCUMENT_BASELINE_REGISTER](DOCUMENT_BASELINE_REGISTER.md)，原E-DOC-069历史批次尚未完成DEC-005/006及正式接口冻结；之后DEC-008-01～06已作出设计决定（见[TODO §2.2](TODO.md)），但公共接口实现、原生测试及正式冻结仍未验证。

## 2. 固定里程碑与历史检索

| 记录 | 已取得的限定证据 | 不扩张为 |
|---|---|---|
| E-DATA-001及早期记录 | 582/582独立MAT类型探针READ_OK，原数据只读 | 正式Reader/Writer或582份独立SHA-256/逐值L2验收 |
| E-DOC-054 | 1996/1998/1999论文材料、勘误和转置/相位核查 | 软件运行或全域数值稳定性证明 |
| E-DOC-055 | 九实基、IR/Choi/定义三路线一般SO(3)精确数学；SH叶子源码 | 全部业务模块或任意IEEE输入正确 |
| E-DOC-056 | 用户在8eab5dd原生1/1 SH CTest通过 | 后续提交自动通过 |
| E-DOC-057/058 | 用户在445ed151增强解析/相位回归1/1通过，随后扩大测试 | 全66TC通过 |
| E-DOC-059 | 用户在601c74de完成53,532检查/624非零四元数的1/1 SH CTest，双类残差有日志 | 全SO(3)浮点路径或最终DEC-005 |
| E-DOC-060/061 | AN-G06/07/08独立归档参考、原二阶逐系数投影实现与数学参考 | 当时尚未运行的新CTest通过 |
| E-DOC-062 | 用户固定5e522198的SH源与三个独立CTest全部通过，53,532+176+76检查 | Adaptive/KvD/采样业务软件通过 |
| E-DOC-063/064 | 三类采样i=0/1/2全714行参考，随后独立Wolfram逐元素核2142坐标，最大差1.11e-16 | 生产采样器/RNG或正式Golden批准 |
| E-DOC-065 | AN-G12对称20方向独立数学单步；+1e-11使near_new仅近似1 | 全部Adaptive输入、Session或真实CTest |
| E-DOC-066 | Original/Modified KvD三组输入共36轮精确数学trace；最少6/最多200的正常控制流界 | 真实模型可达200轮或生产软件验证 |
| E-DOC-067 | 非对称九方向正/负均值独立数学；正支恢复、负支M1与加权投影差2I，条件数有结果 | 所有物理正距离输入失败或可擅改负支 |

上述记录的全部原始数值、源Git blob、SHA-256、日志路径与纠偏过程均在本页顶部的固定历史版本及各 `verification/` 工件。当前不再使用旧“SH源尚未编译/三个CTest未运行”描述：用户已于固定 `5e5221988cf093dbfddb88f2f65f06632460f3a1` 在 Ubuntu 24.04/GCC16.2/CMake4.3.4/Eigen3.4、Release/C++17下完成 **3/3 CTest Passed**。这来自用户终端文本，不是代理在其机器执行。

## 3. E-DOC-068：零均值、抵消、几何退化和最终平移归一的分层证明（2026-11-09，历史批次）

### 3.1 固定起点和真实源

该轮起点 `982537556be1ac5a414af368a0082806a9179d32`。开始时97个唯一AF（74已整改、23未整改）；94个文件（62已审计、32正在审计、0空白）。重新读取 `AGENTS.md`、两份审计台账和原 `oneit_adaptiv_matched_filter.m`（Git blob `48e51bcd52fb8f27cf35b3fce38b28507af36e55`），只依据原先取均值、负支翻号、仍除旧均值、M1～M4、最后t归一的实际表达式开展推导。

### 3.2 该轮实际提交的三个工件

| 路径 | 内容与限定状态 |
|---|---|
| `verification/adaptive/verify_zero_mean_conditioning_families.wl` | 实际Wolfram精确恒等式及70位SVD验证脚本；固定完成版Git blob `36eedaf6477d0255b7ab0f12572506970da6847c` |
| `verification/adaptive/zero_mean_conditioning_evidence.v1.json` | 保留实际25/25检查、6组SVD样例、精确公式、原源blob与限制，非批准Golden |
| `docs/design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md` | 现有Adaptive LLD的理论专题补充；推导定义域、扰动界、恒定行列式反例、正深度窄锥谱、纯旋转平移归一失败及TC映射 |

新增推导属于该轮从源代码出发的研究结论，不声称论文已经给出这些定理。没有以其它算法替换源行为，也没有实施新的阈值、求解器或错误处理。

### 3.3 原始near与翻号后near的准确区分

令r是原始raw near、s=mean(r)、r_flip=sign(s)*r，源最终近度mu=r_flip/s。对于s不为0，**mu=r/abs(s)**，均值为sign(s)。这不是把“已经翻号的near”再除以abs(s)：后者在负支确实会改变原结果。旧文档有关abs(s)的警告必须连同变量阶段解读。

在不跨过零均值的扰动域内，精确相减得到：`delta_mu=(delta_r-sign(s)*mu*delta_s)/(abs(s)+sign(s)*delta_s)`。据此可分离相对尺度与抵消。若所有raw近度非负且s>0，则归一mu在[0,N]；s绝对值小本身不等于发生正负抵消。无量纲chi=abs(s)/mean(abs(r))可辅助记录抵消程度，但本轮没有为它规定生产阈值。

### 3.4 精确参数族及实际Wolfram结果

1. **零均值输入有完整SH表示**：六轴方向与原CS系数 `C00=a*sqrt(4pi), C1,-1=-b*sqrt(4pi/3)` 重建raw near=a+b*y。a=0,b=1给非零向量(0,0,1,-1,0,0)但均值0；a=b=0给全零输入。两者的原均值归一都无定义。没有执行MATLAB，因此不推断其具体警告/异常传播。
2. **全正同比缩小**：b=0、a>0时M=(2/3)I6、条件数严格1；按原光流合成运动解出raw平移a*e_x。若raw平移被横向绝对扰动eta*e_y影响，方向角为atan(abs(eta)/a)。故矩阵良态与最终方向稳定性不是同一条件。
3. **有符号抵消**：a>0、b固定时源M的两个对角块均2I/3、左下块为(b/(3a))*[e_y]叉乘矩阵。det(M)=64/729、六个特征值恒2/3，但kappa2=((sqrt(gamma^2+4)+abs(gamma))/2)^2，gamma=b/(2a)，随a趋0按b²/(4a²)发散。b=1、a=1e-6时约2.50000000002e11。a<b时含负近度，明确不是严格正深度场景。
4. **严格正深度的窄锥几何**：四个单位方向采用有理半角参数h，raw near全1、s=1；在0<h<=1/2域，精确谱三对特征值为2h⁴/(1+h²)²、4h²/(1+h²)²、2/(1+h²)²，所以kappa2=h^-4。h=1/10、1/100、1/1000分别为1e4、1e8、1e12；h=0极限矩阵秩2。此例不靠负近度即可发生严重几何退化。
5. **纯旋转的后置失败**：六轴与raw near全1、M条件数1，纯旋转光流使六维解前三项精确为0；原源码随后t_raw/norm(t_raw)无定义。旋转解与无平移方向的定义域必须分别记录，不能把6×6可逆当成整个单步成功。

完成版Wolfram Language 15.0.1实际执行 **25项检查，25项为True**；包含完整特征多项式恒等式，不仅是有限样本。另对抵消族3组、窄锥族3组输入做70位SVD与公式交叉检查；以相对1e-35作为本次诊断阈值，JSON中差值显示0.0代表该工作精度下结果，不宣称任意机器逐位相同。初始未化简的极限曾产生中间Power::infy提示，完成版先化简增长比再取极限，最后重跑没有这些提示；失败/中间版本不算正式通过。

### 3.5 证据归口和剩余卡点

专题映射现有TC-F-ADAPT-001/002/003、TC-DC-ADAPT-001；没有新增正式TC，也未修改 `TEST_PLAN.md` 中已注册的实体或编写单元测试。新增数学证明补充AF-044/051/054/055/060的若干定义域和病态反例子义务；这些AF仍包括更多一般/近奇异/异常、Session原子提交、真实生产C++/CTest、独立参考覆盖和DEC-005/008批准，因此保持未整改。当前理论不能提供经过真实软件误差校准的阈值，不能自动修正原负支。

该轮只有理论文件与治理记录变动，所有历史原生测试仍绑定各自SHA。

## 4. E-DOC-068历史起止指标

| 指标 | 开始 | 结束 | 净变化 |
|---|---:|---:|---:|
| 整改目标总数 | 97 | 97 | 0 |
| 已整改 | 74 | 74 | 0 |
| 未整改 | 23 | 23 | 0 |
| 审计文件总数 | 94 | 97 | +3 |
| 已审计文件 | 62 | 65 | +3 |
| 正在审计文件 | 32 | 32 | 0 |
| 未审计文件 | 0 | 0 | 0 |

## 5. E-DOC-069：低阶SH离散重新拟合的往返与正值性（2026-11-10，历史数学批次）

### 5.1 核查起点、数学依据及新工件

本轮读取并固定远端起点 `21ad0c34f3c43fd1bbf33c847406dfd302e239ea`；97个AF（74已整改、23未整改），97个审计路径（65已审计、32正在审计、0空白）。重新核对原 `koeff_aus_near.m`、`near_aus_koeff.m`、`kugelflfunc2.m` 和相关采样/Adaptive源规则，原始文件不改。

新增[离散拟合理论专题](design/SH_DISCRETE_REFIT_CONSISTENCY.md)、[Wolfram完整脚本](../verification/harmonics/verify_discrete_refit_roundtrip.wl)、[实际数值证据](../verification/harmonics/discrete_refit_roundtrip_evidence.v1.json)。源3份SH文件Git blob依次为 `5ff91a2210a20e5acf79dec6e59ddd33b55529d3`、`7373c4376dfd5bbf281136ced39657d411a10ffa`、`edc0caa1f88bf91d2a189e4d3a283334f809b905`。完成版脚本blob `41a8d9c034d739702a7e867de1f52b1c1865c38f`，UTF-8 SHA-256 `6a212ec37201c741e33550b47a11d28b8381269df6a97d16745b7d69c6641c9e`。已从提交 `9c6b263903150742b452c0cfe0ff11d132194748` 重新读取并实际执行该脚本，而非只执行未保存的临时代码。

### 5.2 一般代数推导与实际35项辅助检查

原离散系数规则对每一通道分别除以平方范数。设 B 是原CS归一基的N×9采样矩阵，G=BᵀB、D=diag(G)；在D全部严格正的定义域，精确源拟合为c_hat=D⁻¹Bᵀr，系数往返T=D⁻¹G，样本算子P=BD⁻¹Bᵀ。因此对任意九系数精确往返，以及P为幂等投影，都以**当前离散G为对角矩阵**为充要条件。一般证明写入专题；连续球面正交不能替代该离散条件。

Wolfram35/35固定例/恒等式检查实际通过，包含以下结果：

- **24方向正向对照**：全部不同的(1,2,2)/3带符号排列形成秩9、离散G对角且分母正的采样；源逐项拟合对所有九系数精确还原。
- **短小精确正近度反例**：在24行后附加12个v=(1,2,2)/3，所有36个near均为1、每通道分母正且rank(B)=9；按原逐项规则拟合、再按原基重建，在已观测d*=(2,-2,-1)/3得到**−9/140**。多项式缩放系数beta=(1,−3/5,3/5,−3/7,9/10,−9/8,3/5,−9/10,−9/7)；beta=Sc，不能混称为原归一基系数c。全部样本的重建均值是383/210，不是输入均值1。
- **排除重复视线**：把12行换成有理立体投影psi(1/5+j/10^6,2/5)，j=1..12；36条方向全部互异且精确单位，包含rank9基集，每个分母正。d*处结果为精确有理负数，并严格位于(−65/1000,−64/1000)，近似−0.06427848307778179；分子2853位且负、分母2855位且正。结论不依赖浮点误差，也不依赖输入负近度。
- **最后投影阶段的独立定义域**：八个立方体角方向使Y20、Y22对全体样本为0，原平方范数分母为0，九基rank7；但常数近度下运动矩阵为(2/3)I6、条件数1。四方向窄锥则使Y2,-2恒为0，即使选择h=1/4令运动矩阵可逆，末端拟合仍有数学零分母。不能用前置运动矩阵条件替代最后每通道资格。

### 5.3 该发现的边界及真实卡点

本轮不是证明“球谐定义或IR/Choi旋转错误”，也不是推翻固定5e522198的3/3来源忠实性测试；它发现的是**正确基函数之上的离散逐项系数估计并不具有任意采样下的精确逆或保正保证**。正近度反例可对应所有物点距离均为1的输入，但不是现有实验默认方向集已出现相同失败的实测声明。

H=D⁻¹/²GD⁻¹/²的trace为9、对角线全1，G非对角时存在大于1的特征值；这只说明单独重复T存在放大模态，不外推完整Adaptive必然发散。精确零分母在浮点中可能表现为极小非零，不声称取得原MATLAB警告或新C++错误码。

本轮只新增数学材料并更新三份治理记录，未修改主harmonics/Adaptive LLD、原AN-G候选或正式TC注册；新专题明确映射已有SH/Adaptive/采样相关义务，不新增TC。AF-044/045/047/051/055/060仍需各自完整公共/软件/异常/批准条件，不能因该局部理论结案而关闭。没有替换联合最小二乘、裁剪负值、增加阻尼、改变权重或通过扩大容差掩盖差异。

## 6. 本轮E-DOC-069七项指标

| 指标 | 开始 | 结束 | 净变化 |
|---|---:|---:|---:|
| 整改目标总数 | 97 | 97 | 0 |
| 已整改 | 74 | 74 | 0 |
| 未整改 | 23 | 23 | 0 |
| 审计文件总数 | 97 | 100 | +3 |
| 已审计文件 | 65 | 68 | +3 |
| 正在审计文件 | 32 | 32 | 0 |
| 未审计文件 | 0 | 0 | 0 |

三份新数学文件仅在声明的数学范围内完成审阅；原97个路径/状态、97条AF义务/状态保留。第一轮仍23项开放，未启动条件性的第二轮全面审计。C++、CMake、CTest、build.sh未改，**不需要重跑旧SH构建**来验收本轮文档/数学更新。
