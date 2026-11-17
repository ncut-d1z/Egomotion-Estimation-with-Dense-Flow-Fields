# R6：Adaptive场景装配与八轮合同

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。接续[98项任务D3](R6_PLAN_TASK_AUDIT_2026-11-18.md)、P6.7/P8.9/P10.9、REQ-F-TRAJ-001/REQ-DC-EXPERIMENT-001/REQ-NF-SESSION-001。起点本地与GitHub分支API为`f638cf4b3cbd79f0521eed600bfa32511dc71263`，工作区干净；all-state Issue #1/#2/#3开放、PR为空。所有者已授权修改、构建/CTest及方案文档。

## 实现

轨迹模块新增三个窄接口`ITrajectoryGenerator`、`ITrajectoryKinematics`、`ISphereEnvironment`，原TrajectoryGenerator/TrajectoryKinematics/SphereEnvironment实现接口；原数值函数体不变。Adaptive内存Runner必须接收`AdaptiveScene`与既有`AdaptiveSampling`两个借用包，ProductionExperimentRunner拥有原默认场景服务或借用外部实现并传入三个入口。借用对象须长于Runner，参数包本身按引用成员复制；不保存跨帧系数、不操作RunContext。旧直接构造内存Runner的测试显式传入原服务，生产适配默认构造保留。候选接口未冻结，类布局变化须重编译，不承诺旧ABI。

轨迹success先检查点数与全部坐标有限，再调用运动学；运动学success先检查帧数和全部三类运动向量有限，再调用采样。原有效内部帧标志仍在各帧检查；环境返回的近度继续由实际FlowSynthesizer校验行数、正值与有限性。这样非法服务success无法造成点索引越界或向下一依赖传递无效载荷。失败Result保留原ErrorFrame，异常沿用既有Runner分类。初始化/正弦轨迹参数、采样级别2/4/2、35/600/600帧、8次重复、固定除数、原比较/主状态时序及数学阈值不变。FlowSynthesizer仍是局部无状态公式实现，不为简单数学运算添加继承。

## 合同夹具及首次计划

新增`r06_adaptive_scene_composition`使用明确合成的轨迹点、固定真值运动、六轴视线及每轮常数近度，经过真实FlowSynthesizer、零系数的真实MeanMagnitude噪声、真实Session/更新策略、真实Runner聚合与RunContext发布逻辑；Step和Writer/Publisher为内存替身。三入口完整35/600/600行，Comparison确实经过八轮4784个内部帧、14352次Step及86112次normal；核每轮主/比较全系数重置、各周期候选、首尾零、列主序、角度0/90/180、误差八轮平均45、末轮rottransratio=1/8和akrot的弧度转度。期望为独立简单几何/整数关系，角度比较沿用既有Runner的1e-9工程余量，不修改DEC-005。

54个场景服务故障覆盖三个入口的失败Result、bad_alloc、length_error、错尺寸、非有限、近度零/负及缺有效运动标志，核在后续服务/Step/RNG之前停止。另在Comparison第5轮MATLAB帧17注入环境失败，预期planned8、attempted5、completed4、valid2407，后续无Step/抽样、不发布部分均值或MAT，完整错误来源/帧身份及Incomplete审计保留。

夹具Complete及内存receipt只证明组合合同，不是MAT持久化或原科学实验成功；真实三Adaptive入口原有acos越界/零旋转失败仍需如实报告。原独立数值、真实默认入口、MAT/Writer/CLI及此前注入回归全部保留。

首次尚未执行：先提交、正常推送并核实远端SHA，随后新RunId双模式配置/构建，先MAT7项再全部其余CTest，预期SVG ON60、OFF59。完整环境、命令、Git SHA、输入哈希、固定RNG来源、stdout/stderr、退出码及JUnit分别存档。不安装依赖、不运行MATLAB/Octave/完整科学或性能实验，不修改原始资料/阈值，不签署/冻结/合并/发布。

## 固定源码双模式结果

实际受测源码 **`74dbccd01afe23465d7914444917f7f357e69f9a`**，分支`dev-plan-P0-P1-mat-dictionary-probe`；正常推送并用GitHub branch API核实同SHA、工作区干净后执行。RunId **`r6-adaptive-scene-20261118-BLAFVtQC`**，工作目录为仓库根。

| 配置 | 配置/构建 | MAT优先 | 其余CTest | 起止时间（2026-11-18，UTC+08:00） |
|---|---|---|---|---|
| SVG ON | exit0，无诊断 | 7/7 | 53/53，共60/60 | 03:05:35–03:08:54 |
| SVG OFF | exit0，无诊断 | 7/7 | 52/52，共59/59 | 03:09:02–03:11:53 |

新增测试两模式各 **88121项检查**，三模式全部完成合成场景；Comparison实际完成8轮、4784有效帧、14352次Step、86112次normal，逐轮主/比较系数全部重置、误差平均和末轮比值符合独立简单几何/整数期望。54个服务故障及第5轮第17帧失败的2407有效帧前缀均通过。真实默认场景、独立SH/单步/KvD、Session、MAT/Writer/CLI及此前36088项采样/噪声注入回归全部执行；真实三Adaptive入口仍按原数值行为失败并保留Incomplete、无MAT，未改变公式或裁剪acos。

四份JUnit与每模式CTest登记逐名一致，全部status=run，零failure/error/skip/disabled；ON唯一多出`r06_svg_source_windows`。OFF编译源、链接列表、ELF符号及静态库检查确认无SVG实现。两模式各720项输入SHA后验全匹配，原628文件/582MAT树仍为`70ee155131c24a50169346d0ffbaff0c01b1c547`。`src/trajectory/`原数值函数体与本轮起点无差异。

仅本轮环境有证据：Ubuntu24.04.5、Linux7.0.0-43 x86_64、GNU16.2、C++17 Release、CMake/CTest4.3.4、Eigen3.4、matio1.5.26、zlib1.3、Sophus1.24.6。场景夹具RNG固定返回0.5，无reseed；其它固定seed来源单独存档，RunId使用独立OS随机源。启动命令为`bash build/r6-adaptive-scene-20261118-BLAFVtQC/run.sh 74dbccd01afe23465d7914444917f7f357e69f9a ON`及同参数`OFF`。各模式`evidence-ON/`、`evidence-OFF/`保存完整环境/版本、步骤命令与参数、开始结束时间、退出码、独立stdout/stderr、JUnit、Testing快照、输入blob/SHA、随机来源、CMake缓存、编译/链接列表和ELF；复用独立构建缓存，没有覆盖旧证据。后验逐名核对存于`read-only-junit-audit.json`。

D3目前列出的Bias/Adaptive依赖装配源码缺口已补，包括轨迹、运动学、球环境；不再把这些已完成接口列为后续缺口。总体对象边界验收仍未签署，D1构造不变量、D2其余职责、D4只读/预留前记录范围、完整科学实验及正式资格仍未完成，R6未结案。此次无新构建/测试缺陷或Issue；#1/#2/#3仍开放，无PR。后续文档提交仅同步事实，不代表重新构建或人类冻结。

文档静态复核：98个P与PLAN逐项同序，66个TC及45源路径/50声明与变更前身份和顺序一致；TEST_PLAN除顶部事实注记外正文完全相同。11份更新文档的508处相对文件引用均有目标，结果存于`read-only-document-audit.json`。这不是Bash追踪脚本现场执行或新增CTest记录。
