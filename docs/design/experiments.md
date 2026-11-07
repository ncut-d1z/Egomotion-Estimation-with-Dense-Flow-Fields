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
  -> (future only) write current-script-compatible MAT outputs to new output/<RunId>/
  -> record errors/invalid counts and manifest
Output: Result<RunOutcome, ExperimentErrorCode>（成功时为 RunOutcome 及记录的输出变量/来源，失败时保留明确错误上下文）
~~~

- 每个 RunId 拥有 RNG；**H-12 选定将来新输出目录为仓库根 `output/<RunId>/`**，由 `.gitignore` 忽略生成数据。失败/重试不自动覆盖旧输出，原始 MAT 永远只读；**H-10 暂缓代码实施，当前没有可运行的 MAT Writer 或生成结果。**
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
// 结构化运行结果仅使用 §6 的 RunOutcome，避免 RunReport/RunOutcome 并存。
class BiasExperimentRunner;
class InitializationRunner;
class AdaptiveComparisonRunner;
class UpdateRateRunner;
~~~

**历史 MAT 的类型和 shape 已由 matio 1.5.26 实测**：582 个源文件/变量均为 v5+zlib、double/double，按下方数据契约读取；新 C++ Runner 尚未生成任何结果，未来 MAT 写出仍需回读/数值验收。

## 4. Golden、异常、验收和未决

- 新输出的所有变量必须通过 matio 回读并与原 MAT 的 shape、统计含义核对；不应要求不同语言在随机数不等价时逐样本完全一致。
- CLI 错误类含参数缺失、非法细分、零模向量、MAT 写失败/覆盖冲突、数值非有限和部分运行失败；具体退出码在 SRS/TEST_PLAN 冻结。
- TC-F-BIAS-001/002/003、TC-F-EXP-001/002/003、TC-F-CLI-002/003、TC-I-RESULT-001、TC-NF-LOG-001；相关异常/性能在 P10 执行。
- **真实历史 MAT 全量变量字典已完成读取**；独立算法 Golden、MATLAB 数值等价、总墙钟/CPU/RSS 预算、版权和新的 C++ 输出回读仍未完成，LLD 不可冻结。


## 5. 五个入口的参数 schema 与源文件差异

**下表分离两个不同证据层次**：「当前脚本预分配/save」来自原 MATLAB 静态源码；「历史归档实测」来自早前 matio 582/582 变量读取，不是本轮 MATLAB 执行，更不能推断历史文件都由当前脚本版本生成。历史 MAT 详情以 §9 和 [MAT_DATA_SCHEMA.md](../MAT_DATA_SCHEMA.md) 为准。

| 入口 | 参数/原始默认值（MATLAB 代码） | MATLAB 循环/样本 | **当前脚本预分配与有效 save（源码静态核对）** | **历史归档的实际类型/shape（matio 实测）** |
|---|---|---|---|---|
| bias occluded | corenumber、iterationnumber、savestring；noise=[0.6,2.4,9.6]；durchlaeufe=1 | level=0..iterationnumber；N=6·4^level | `xwerte` 是 (iterationnumber+1)×1；`ywerte1…6` 各 (iterationnumber+1)×3；共七个有效 save | 40 个结果目录，各有 `xwerte` 10×1、六个 `ywerte` 10×3；均为 double |
| bias proportional | 同上，完整八面体 N=8·4^level | level=0..iterationnumber | 与 occluded 同样七个 save 与预分配；原脚本 `xwerte` 仍采用 6·4^level | 另 40 个结果目录，七个文件的形状同上；均为 double |
| adaptive init | savestring；stellen=35、kreisradius=0.6、hoehe=0.3、二十面体 level=2 | i=2..34；anzahlitfuerini=1、anzahldurchlaeufe=1 | 四类 `allemit*/alleohne*` 误差分别为 35×1，保存四个 MAT | initialization 四个 double[35×1] |
| adaptive comparison | addederror、savestring；stellen=600、sinus amp=0.5、length=1、periods=2、height=0.3、level=4、anzahldurchlaeufe=8 | i=2..599 | 四类误差各 600×1，另保存 `rottransratio` 600×1（最后一次运行的缓冲）；共五个 save | without_errors：五个 double[600×1]；with_errors：七个 double[600×1]，**另外两份 `alleadap*` 来源未证实** |
| adaptive rates | savestring；stellen=600、同一正弦轨迹、level=2、period=[1,5,11,21]、单运行 | i=2..599；四条周期 baseline | `allemittransfehler`、`allemitrotfehler` 各 600×4，`akrot` 600×1；**当前仅三个有效 save** | many_parameter：四个 double[600×4] 与两个 double[600×1]，包括当前未保存的 `alleadap*` 与 `rottransratio` |

§9 的「历史归档」仅支持已经存储的变量名、MAT class/dtype、shape 和文件数；**不能把它反写为当前源脚本的运行结果**，也不得补造 `alleadap*` / `rottransratio`。

**必须保留的源码特殊行为（已核查）**：

两个 Bias 入口都在第 58 行设置 xwerte(i+1)=(8*4^i)*(6/8)=6*4^i。**对完整球面 Bias，真实采样 N=8*4^i，但作者存储的 xwerte 仍是 6*4^i。**这是原脚本的记录/横轴语义与实际样本数量不一致，不得为了“修正数量”而悄悄改写作者 xwerte MAT。设计应同时输出：
1. original_xwerte：忠实复制源脚本的存储值；
2. actual_flow_count：内部诊断元数据，按实际方向数量记录；
3. analysis 默认使用 original_xwerte 对照原图，任何使用 actual_flow_count 的图必须另行标为分析增强而非原版复现。

Bias 六组 ywerte 对应顺序依次是：Franz 平移、Franz 旋转、Original KvD 平移、Original KvD 旋转、Modified KvD 平移、Modified KvD 旋转，六者都调用原 transfehler_div 的**尺度对齐向量差**，不能将名称含 rot 的列改为测地角。实验分析脚本在现有代码中主要读取 ywerte3…6。

## 6. 拟议配置/结果接口与状态

~~~cpp
// 设计候选，非已实现代码；公共状态码见 contracts.md。
struct BiasExperimentConfig {
    std::uint32_t original_corenumber_seed;
    std::size_t maximum_subdivision_level;
    std::array<Scalar, 3> error_multipliers{{0.6, 2.4, 9.6}};
    std::size_t source_inner_repetitions = 1;
};
struct InitializationExperimentConfig {
    std::size_t trajectory_points = 35;
    Scalar circle_radius = 0.6;
    Scalar height = 0.3;
    std::size_t icosahedron_level = 2;
};
struct ComparisonExperimentConfig {
    Scalar added_noise_coefficient;
    std::size_t trajectory_points = 600;
    std::size_t icosahedron_level = 4;
    std::size_t source_repetitions = 8;
};
struct UpdateRateExperimentConfig {
    std::array<std::size_t, 4> refresh_periods{{1, 5, 11, 21}};
    std::size_t trajectory_points = 600;
    std::size_t icosahedron_level = 2;
};
struct RunOutcome {
    RunId run_id;
    std::vector<std::string> mat_variables_produced;
    std::size_t attempted_solver_calls;
    std::size_t valid_samples;
    std::size_t failed_samples;
    std::size_t undefined_error_samples;
};
class BiasExperimentRunner {
public:
    Result<RunOutcome, ExperimentErrorCode> runOccluded(
        const BiasExperimentConfig&, RunContext&);
    Result<RunOutcome, ExperimentErrorCode> runProportional(
        const BiasExperimentConfig&, RunContext&);
};
~~~

实际项目使用 std::variant 区分五类 ExperimentConfig，还是各命令拥有独立参数类型，须在 P0.8/P1.2 冻结。**单个源脚本的参数默认值不能因为复用 Runner 而外溢到另一实验**。RunContext 应独占 RNG、输出位置与本次结果 Manifest；没有全局状态。

### 6.1 输出 Manifest 推荐字段

| 字段 | 语义/约束 |
|---|---|
| run_id / experiment_kind / source_entry | 唯一运行身份与原 MATLAB 函数名称 |
| repo_sha / source_blob_sha / original_mat_root | 可追溯版本，原数据只读 |
| original_parameters / default_origin | 每个数值来自入口形参还是源文件固定常量 |
| subdivision_level / actual_flow_count / original_xwerte | 区分物理 N 与作者横轴存储值 |
| uniform_calls / normal_calls / seed | 观测生成和噪声调用统计 |
| mat_files | 文件名、变量名、dtype、rank/dims、输出 SHA；不能写 origin |
| attempted / valid / failed / undefined | 不隐藏失败、未定义或仅部分生成的结果 |
| environment / stdout / stderr / exit_code | Debian/Ubuntu、构建源 SHA、真实执行证据 |

## 7. 结果写入与多次运行的失败路径

- **Bias**：每个 corenumber 的结果相互独立，写入相应新 RunId 子目录；原始已有编号目录 1..40 是历史参考，不是 C++ 自动覆盖位置。
- **Comparison**：四种方向误差数组在八次内循环中累加后除以 8；rottransratio 在各次运行的循环里重新分配，最后仅写最后一次的数组。不得擅自对其求八次平均或把它与四个误差数组同样聚合。
- **Update rates**：比较的 mitkoeff 四组在源周期触发时更新；主 akkoeff 单独每帧更新。当前源 save **只有三个 MAT**；历史附加文件需要字段级差异报告。
- **初始化**：每次创建独立系数起点，不能重用此前 600 帧轨迹的状态；首尾行按原 zeros 预分配语义。
- **原始数据保护**：任何路径重叠、文件已存在、变量写入失败应返回带 RunId 的明确错误，不允许把部分成功伪装为完整。
- 对新实验在各入口运行过程中失败是否继续剩余参数组合，须经 P0.8 审核其与 MATLAB 行为差异；不能以异常吞掉而达到“全部通过”。

## 8. Golden/验收/尚未批准的合同

| 用例 | 最少黄金/预期字段 | 具体 TC |
|---|---|---|
| EX-G01 | 两类 Bias i=0/1/2 的正确 N、作者 xwerte、六项 ywerte、seed/随机轨迹 | TC-F-BIAS-001/002/003 |
| EX-G02 | 初始化 35 点，四类误差与相同原 MATLAB 参数 | TC-F-EXP-001 |
| EX-G03 | comparison 600 点 8 次的结果矩阵、单次 rottransratio | TC-F-EXP-002 |
| EX-G04 | 更新率 1/5/11/21、600×4/600×1、三个现有源 save 字段 | TC-F-EXP-003 |
| EX-G05 | 输出目录冲突、MAT 写失败、无效近度/solver 失败 | TC-NF-SAFETY-001、TC-NF-RECOVERY-001 |

历史 **MAT 类型/shape 字典已实测**；但算法 Golden 中间量、容差、CLI schema、生产写入/回读与 CTest 仍未建立，不能以最终结果文件能够被读取替代算法真实性测试。

**状态**：五类入口参数与当前 save 契约已文档化，**历史输出 dtype/shape 已由 582/582 matio 数据字典确认**；C++ Runner、新输出、逐节点 Golden、误差容差及接口冻结仍未完成。

## 9. 原作者归档字段的实测数据合同

本节仅引用已在 [MAT_DATA_SCHEMA.md](../MAT_DATA_SCHEMA.md) 中经 `Mat_VarRead` 读取成功的**历史 MAT 文件内容**。`MATLAB class=double`、`data_type=double`、压缩 MAT v5/zlib；以下尺寸不再是作者 `zeros` 预分配的推测。

| 历史结果组 | 实测变量 | 实测形状与文件数 | 与当前 MATLAB 生成脚本的关系 |
|---|---|---|---|
| occluded_regions | 40 个 xwerte；每个 ywerte1…6 各 40 个 | xwerte: 40×(10×1)，ywerte: 240×(10×3) | 40 个独立结果目录，与两种算法的误差统计关联 |
| proportional_error | 同上 | xwerte: 40×(10×1)，ywerte: 240×(10×3) | 真实输入方向是完整球面，但原 xwerte 仍写为 6·4^i；不得更改 |
| initialization | allemittransfehler、allemitrotfehler、alleohnetransfehler、alleohnerotfehler | 4×(35×1) | 与当前源 save 四种误差字段名一致 |
| without_errors | 四误差变量、rottransratio | 5×(600×1) | 与当前对比实验 save 五字段对应 |
| with_errors | 四误差变量、rottransratio；另有 alleadaprotfehler/alleadaptransfehler | 7×(600×1) | **后两字段是归档额外结果，当前生成入口无对应 save**，来源待归因 |
| many_parameter | allemitrotfehler、allemittransfehler、alleadaprotfehler、alleadaptransfehler | 4×(600×4) | 只有前两项在当前生成脚本保存，后两项为历史额外结果 |
| many_parameter | akrot、rottransratio | 2×(600×1) | akrot 为当前脚本输出，rottransratio 是历史额外结果，当前分析脚本依赖它 |

**验证边界**：这些历史文件被 matio 读取成功，但没有证明 C++ 新生成的对应数值、RNG 顺序或误差曲线与 MATLAB 一致。仍需 TC-F-EXP-001/002/003、TC-I-RESULT-001 的具体结果回读、比较和差异归因。

## 附录 A：本模块无需运行 MATLAB 的验收参考

按 [长期无需 MATLAB 执行政策](../golden/NO_MATLAB_EXECUTION_POLICY.md) 与 [L2/L3/L4 来源矩阵](../golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md)：原 MATLAB .m 文件仅作为**只读公式、数据布局和顺序规范**；**不要求执行 MATLAB/Octave、采集 L1 或以缺少 L1 阻塞冻结/编码/验收**。

现行实验验收从历史 L2 MAT 对照结果字段、shape 和统计；另以 L3 固定输入独立模拟 8 次聚合、rottransratio 末次值、周期 1/5/11/21 的触发及 Session 主/基线状态。原 MATLAB 运行时的每次 RNG/每帧系数并未保存，也**不需要**运行原脚本采集；仍需真实 Runner、Writer、安全/CTest 和 L4 统计。

本附录不取消真实 C++/CTest、Golden 独立复核、DEC-005/008 以及源数据保护门禁；当前无新的软件测试通过证据。
