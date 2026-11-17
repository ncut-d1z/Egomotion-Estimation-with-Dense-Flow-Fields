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
