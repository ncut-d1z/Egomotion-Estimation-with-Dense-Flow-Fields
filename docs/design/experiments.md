# 实验 Runner 与结果写出——详细设计草案

> **未冻结，未创建 C++ Runner。** 来源：bias_test/data_generation 中 2 个入口；adaptive_matched_filter_test/data_generation 中 3 个入口。对应 P5、P8、REQ-F-BIAS、REQ-F-EXP、REQ-F-CLI 与 REQ-I-RESULT。

## 1. 实验入口及源代码实际参数

| Runner（拟议） | MATLAB 函数 | 关键原始参数/范围 | 当前脚本实际写出 |
|---|---|---|---|
| BiasExperimentRunner / occluded | bias_test_nonspherical | corenumber seed、iterationnumber；3 个噪声因子 [0.6,2.4,9.6]；单个脚本 durchlaeufe=1；N=6·4^i | 每 run 子目录 xwerte、ywerte1…6 |
| BiasExperimentRunner / proportional | bias_test_spherical_prop_error | 其他参数近似但 N=8·4^i 且噪声不同 | xwerte、ywerte1…6 |
| InitializationRunner | agent_in_sphere_initialization_test | 圆轨迹 35 点、二十面体级别 2、每帧零阶重置与继承 | allemittransfehler、allemitrotfehler、alleohnetransfehler、alleohnerotfehler |
| AdaptiveComparisonRunner | agent_in_sphere_adaptive_matched_filter_test | 600 点正弦轨迹、二十面体级别 4、8 次重复、addederror 参数 | 上述四类平均误差、rottransratio（最后一次运行的缓冲） |
| UpdateRateRunner | agent_in_sphere_adaptive_matched_filter_many_parameter | 600 点正弦轨迹、二十面体级别 2、单次运行、周期 1/5/11/21 | allemittransfehler、allemitrotfehler、akrot |

所有历史 MAT 应与当前脚本的 save 行为**分别记录**；with_errors 和 many_parameter 目录含当前脚本未写出的额外变量，不能自行补造。

## 2. Runner 时序、对象拥有与可重入性

~~~text
Input: experiment-kind + validated script parameters + RunId
  -> new RunContext(own RNG, output directory, build/source SHA)
  -> create independent sampling/trajectory and noise model
  -> synthesize flow or per-frame scene
  -> call Franz/KvD or Adaptive StepSolver via independent Session
  -> calculate source-specific metrics and statistics
  -> write current-script-compatible MAT outputs to new results/RunId
  -> record errors/invalid counts and manifest
Output: RunReport(status, output variables, run provenance)
~~~

- 每个 RunId 拥有 RNG，失败/重试不自动合并到旧输出；不得用当前脚本写入的结果覆盖任何原始 MAT。
- Bias 的 40 个历史子目录是 40 份结果来源，不能错误解释为一个源运行的 40 个内循环。
- AdaptiveComparisonRunner 的前四种结果累计后除以 **8**；rottransratio 按原脚本在循环中的覆盖次序保留。UpdateRateRunner 的输出系数状态分别保存，不和当前主 akkoeff 共享可写对象。
- RunContext 跨实验不可共享可变状态；Session 异常时继续还是终止需冻结来源行为与结果合同。

## 3. 候选实验和报告结构

~~~cpp
// 仅候选合同，不是生产 C++ 文件。
enum class ExperimentKind {
    BiasOccluded, BiasProportional,
    AdaptiveInitialization, AdaptiveComparison, AdaptiveUpdateRates
};
struct ExperimentConfig;    // 五种原入口参数分开表达
struct RunReport {
    ExperimentKind kind;
    std::string run_id;
    std::vector<std::string> produced_mat_variables;
    std::size_t valid_samples;
    std::size_t failed_samples;
    // 实际参数、seed、source SHA、输出目录和错误记录由后续合同细化
};
class BiasExperimentRunner;
class InitializationRunner;
class AdaptiveComparisonRunner;
class UpdateRateRunner;
~~~

原始输入/输出维度与类型需要 MAT 数据字典核定，不可仅根据 zeros 预分配断定历史文件的真实 dtype/rank。

## 4. Golden、异常、验收和未决

- 新输出的所有变量必须通过 matio 回读并与原 MAT 的 shape、统计含义核对；不应要求不同语言在随机数不等价时逐样本完全一致。
- CLI 错误类含参数缺失、非法细分、零模向量、MAT 写失败/覆盖冲突、数值非有限和部分运行失败；具体退出码在 SRS/TEST_PLAN 冻结。
- TC-F-BIAS-001/002/003、TC-F-EXP-001/002/003、TC-F-CLI-002/003、TC-I-RESULT-001、TC-NF-LOG-001；相关异常/性能在 P10 执行。
- 当前 MAT 全量变量字典、独立 Golden、总墙钟/CPU/RSS 预算和版权核查未完成，LLD 不可冻结。
