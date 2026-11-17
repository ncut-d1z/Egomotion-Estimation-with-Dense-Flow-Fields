# R6：Session比较系数更新策略

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。继续 [PLAN P8.8](../PLAN.md) 与REQ/TC-DC-ADAPT-001、TC-F-ADAPT-003、TC-NF-SESSION-001，补 [98项复核D2](R6_PLAN_TASK_AUDIT_2026-11-18.md) 的策略缺口。所有者已授权代码、构建/CTest和方案文档修改；起点本地/远端`7a8459aa6025e8fe4ac95293a9870c22024b5392`、工作区干净。关联“检查R5实现完成度”仍idle，最新请求继续实现R6；未发送消息。all-state仍为开放Issue #1/#2，无PR。

## 实现边界

新增`coefficient_update.hpp/.cpp`：`ICoefficientUpdatePolicy::prepare(i,primary,baseline,source_rotation) const`返回自拥有比较系数候选。Reset每帧重置常数项1；Inherit直接复制已完成入口旋转、尚未本帧主求解的系数；Periodic先调用既有`rotateSourceCoefficients`旋转上一帧基线，再按MATLAB帧号模周期从当前主旧系数刷新，默认周期严格为1/5/11/21。刷新帧也不能跳过基线旋转的失败检查。策略不调用求解器、不消费RNG、不拥有/修改主或跨帧基线状态。

Session仍为唯一持久状态拥有者，原构造入口保持可用并拥有默认策略；另可显式注入策略指针数组，复制数组、借用策略对象，后者须比Session活得更久。初始化/普通Comparison使用前两槽，UpdateRates使用四槽；所用空槽在任何策略/求解之前拒绝。Session先按原模式处理主旋转，再准备全部比较候选，依序完成比较及主求解，构造完整成功结果后才提交主/周期基线与帧号。策略失败/非有限候选/分配故障不提交；完整错误链保留。比较`coefficients_next`在原固定单次比较中不作为持久状态，UpdateRates的`dumi`继续丢弃。

周期策略逐槽完成独立旋转/刷新，延续既有C++逐槽顺序；原.m将四次旋转和四次刷新分两循环。所有策略都只读同一主旧值和各自旧基线，所有准备在比较求解前完成，数值依赖没有改变；不声称原MATLAB运行时异常轨迹相同。未新增比较迭代次数、周期CLI参数或数值阈值。接口为候选增量、未冻结，布局变化需重编译，不承诺旧二进制ABI。

原脚本仅静态审查，blob：Initialization `a3154ae391be82ccf48c9f3a03c55109dde85f65`、Comparison `c9548c83fd0a2ce13d96394e98f6c7573119a7d5`、UpdateRates `4629901c1233cdee5d6d22c917603720ffd1fab4`。

## 验证准备（尚未执行）

新增`r06_coefficient_policy_composition`：2…43帧、非平凡z旋转、三模式默认/注入策略逐位对照；常数/继承与模周期选择、每槽调用输入/顺序、替换候选实际生效、主状态不可越权、共享策略下Session隔离；每槽Failure/NaN/bad_alloc/length_error、每个比较/主求解失败及同帧重试；空槽/错误帧号/零周期/刷新帧坏旋转拒绝，错误链保留来源/变量/索引/RunId。数值自比较只用于组合证据，不冒称独立Golden；继续保留原独立SH/单步L3、真实Session交错、周期哨兵、Runner与实际CLI测试。

预期SVG ON57、OFF56项CTest。按AGENTS先原子提交、正常推送并确认远端SHA，再分别配置/编译，先MAT7项、后全部其余测试，保存独立RunId/命令/环境/种子来源/退出码/stdout/stderr/输入SHA。当前不得把本段预期记为通过。未安装依赖、运行MATLAB/Octave、完整科学/L4或性能实验，原始资料保持只读。

D1构造不变量、D2其它球谐/分析职责、D3 Adaptive装配及D4只读/预留前记录边界仍保留。真实Adaptive的acos越界/零旋转和完整产物缺口未由本轮改动消除；R6未完成，未冻结/签署/合并/发布。
