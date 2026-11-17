# R6：Adaptive采样与噪声依赖注入

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。接续[98项P复核D3](R6_PLAN_TASK_AUDIT_2026-11-18.md)、P2.9/P3.8/P8.9/P10.9、REQ-DC-EXPERIMENT-001/REQ-DC-NOISE-001/REQ-NF-SESSION-001。起点本地与GitHub分支API均为`e942764aa08183092f54d1a61050a10615434a44`，工作区干净。关联“检查R5实现完成度”最新要求继续检查/实现R6，仍idle，未发送消息；Issue #1/#2/#3开放，无新Issue/PR。

## 实现边界

内存Adaptive Runner原来在compute内部直接创建IcosahedralSampler和MeanMagnitudeGaussianNoise，现经`AdaptiveSampling`借用两个既有窄接口。ProductionExperimentRunner拥有原具体默认服务，并提供显式借用构造；三个入口都传入同一已选服务组合。服务的生命周期须长于Runner，临时参数包按引用成员复制，不借用参数包本身。未增加第二噪声接口、RTTI识别或CLI模型选择。内存Runner的无参数构造删除，既有真实初始化取证及Session测试改为显式装配相同服务；生产适配默认构造保留。

配置失败在调用依赖之前返回；级别仍为Initialization=2、Comparison=4、UpdateRates=2，采样先于RunStarted和逐帧计算。仅Comparison调用噪声模型，显式传noise_scale和同一外部RNG，系数0也由原模型消费3N次normal，不重置或替换随机流。噪声服务返回success后检查行数、全部元素有限性及前三列与理想场严格相同，失配以DataMismatch终止，不能把畸形成功值送入Session。失败服务保留原有ErrorFrame链，bad_alloc/length_error沿用既有Runner异常映射。

每次compute仍新建Session；策略/主系数不由采样或噪声服务持有。35/600/600帧、8次聚合/固定除数、末次rottransratio、轨迹/球环境/理想流计算、误差公式和阈值均保持。轨迹生成/运动学/球环境是现有无跨帧状态的具体数学服务，本轮没有把它们伪称已具可替换接口。未更改三个真实Adaptive入口的acos越界/零旋转结果。

## 领域合同复核范围

SRS的REQ-DC-DOMAIN-001和PLAN P2.8要求形状、有限性、失败状态及拥有值语义；contracts §2仍列可变aggregate候选，§3.B明确`success(T)`调用者负责T的数学不变量。DEC-008-03的禁止默认构造/不可变工厂只约束Result/Status，不能扩称全部领域值已有相同决定。本轮补足可替换噪声success进入Adaptive Session前的实际形状/有限性边界；这不是构造时全不变量保证。D1的领域访问、默认值、几何不变量及完整合同测试仍保留，不用新文档取消义务，也不自行添加切向性数值阈值。

## 首次验证计划

新增`r06_adaptive_dependency_injection`：三模式×三噪声系数的顺序/同Runner重放；采样级别、完整帧计数、实际替换流进入求解器、主/基线首帧重置；18个采样失败/分配/长度/空/非有限/非单位场景和6个噪声失败/分配/长度/空/非有限/改方向场景；非法配置零依赖调用；三生产入口默认/显式真实采样噪声装配逐位对照，RunContext RNG消费、失败计数、嵌套MatError路径/变量/索引/RunId及不发布结果。固定Step仅验证组合，不冒称独立数值Golden或完整实验。

先原子提交、正常推送并核实远端SHA，再在新RunId下配置/构建SVG ON/OFF，先MAT7项，后全部其余CTest；预期ON59、OFF58，首次尚未执行不能写通过。保留独立命令、环境、源SHA、seed来源、输入哈希、stdout/stderr、退出码和JUnit。既有数值/真实失败/Writer/CLI回归保留；无依赖安装、MATLAB/Octave、完整科学/L4、性能、阈值变更或验收/冻结/合并/发布。
