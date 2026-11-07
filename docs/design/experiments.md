# 实验 Runner 与结果写出——详细设计草案

> **长期参考边界**：本 Runner **不要求 MATLAB/Octave 运行或采集 L1**；原 .m 仅作只读源码依据，结果验收使用 L2 已存 MAT、L3 独立逐帧/逐轮参考与 L4 统计，参见 [长期政策](../golden/NO_MATLAB_EXECUTION_POLICY.md)。

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

## 4. L2/L3/L4 参考数据、错误和验收边界

- 新输出的所有变量必须通过 matio 回读并与原 MAT 的 shape、统计含义核对；不应要求不同语言在随机数不等价时逐样本完全一致。
- CLI 错误类含参数缺失、非法细分、零模向量、MAT 写失败/覆盖冲突、数值非有限和部分运行失败；**错误枚举→退出码的唯一候选定义是 [contracts.md §3.1](contracts.md)**，CLI 命令解析与诊断见 [cli.md §3](cli.md)，不由 Runner 重复定义。
- TC-F-BIAS-001/002/003、TC-F-EXP-001/002/003、TC-F-CLI-002/003、TC-I-RESULT-001、TC-NF-LOG-001；**候选测试实体及 CTest 前缀见 §8.1 和 TEST_PLAN §3**，相关异常/性能只能在后续获授权时执行。
- **真实历史 MAT 全量变量字典已完成读取**；独立 L3 逐节点参考、L2/L3/L4 限定范围的数值对照、总墙钟/CPU/RSS 预算、版权和新的 C++ 输出回读仍未完成，LLD 不可冻结。


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

### 6.1 RunContext 的最小所有权/接口候选（非编译代码）

RunContext 属于 **egomotion_experiments** 的一次性运行上下文，不与 `MatResultReader`、`RunMatWriter` 或 `MatDenseArray` 建立生产级直接链接。Composition Root 负责在执行前校验用户输出根与 RunId、构造具体 Writer 后以 **contracts 的 IResultWriter** 窄接口注入；RunContext 只持有一个经验证的 `RunId` 和所需上下文。

~~~cpp
// 文档候选；IRandomSource/IResultWriter/RunId 由 contracts.md 声明。
// 最终构造签名与 Result/Failure 携带方式均待 P0.8/DEC-008 冻结。
struct SourceProvenance {
    std::string repository_commit_sha;
    std::string matlab_source_blob_sha; // 只读源码身份，绝非 MATLAB 运行记录
};
class RunContext {
public:
    RunContext(RunId id,
               std::unique_ptr<IRandomSource> random,
               IResultWriter& writer, // 非自拥有：Composition Root 保证生命周期
               std::filesystem::path validated_output_root,
               SourceProvenance provenance);
    RunContext(const RunContext&) = delete;
    RunContext& operator=(const RunContext&) = delete;
    // 移动/重置策略暂不承诺，避免外部持有失效 writer/RNG 引用
    const RunId& runId() const noexcept;
    IRandomSource& randomSource() noexcept;
    IResultWriter& resultWriter() noexcept;
    const SourceProvenance& provenance() const noexcept;
    const std::filesystem::path& approvedOutputRoot() const noexcept;
    // 结果计数/Manifest 只可由当前运行写入；暴露方式待审
private:
    RunId run_id_;
    std::unique_ptr<IRandomSource> random_;
    IResultWriter& writer_;
    std::filesystem::path validated_output_root_;
    SourceProvenance provenance_;
    // 每次运行独占的计数和 Manifest 值对象，实际表示尚待决定
};
~~~

- **谁拥有谁**：Composition Root 保证 `IResultWriter&` 的有效期长于本次 Runner 调用；RunContext **独占 RNG**、RunId/结果计数/Manifest，Session 只持有其自身系数状态且不能跨 RunContext 共享。输出根由 CLI/Composition Root 验证，Runner 不按任意用户字符串拼接文件路径。
- **失败与释放**：固定 RNG 耗尽、子求解器失败、MAT 写失败必须在 `Result<RunOutcome, ExperimentErrorCode>` 的失败分支保留原底层错误及 `ErrorDetail.run_id`；不再以一个空 `RunOutcome` 表示失败。Manifest 是否允许 partial、失败状态能否继续下一样本，仍为待决定事项，不做默认回滚承诺。
- **边界**：`BiasExperimentConfig` 负责原入口 seed/最大 level/三个噪声乘数，`BiasFlowParameters`（采样 LLD）负责**单个流场**，不能彼此覆盖默认值。配置选择 `std::variant` 还是独立参数类型、RunContext 移动/并发限制、Writer 创建时机均待 P0.8 冻结。

### 6.2 输出 Manifest 推荐字段

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

## 8. L2/L3/L4 参考数据、验收和未批准合同

| 参考用例 | 证据来源等级 | 最少独立参考/预期字段（不是被测 C++ 自算） | 具体 TC |
|---|---|---|
| EX-G01 | **L3** 采样/固定注入 RNG；**L2** 作者归档 xwerte/ywerte；**L4** 跨 40 runs 统计 | 两类 Bias i=0/1/2 的 N、原存储 xwerte、六项 ywerte 的 shape/指标；固定 RNG 序列的消费次序与独立统计分母 | TC-F-BIAS-001/002/003 |
| EX-G02 | **L2** 历史四类 35×1；**L3** 独立帧/状态 | 初始化 35 点、四类误差 shape、首尾数组与主/比较系数更新的独立参考 | TC-F-EXP-001 |
| EX-G03 | **L3** 固定 8 次聚合；**L2** 历史五字段；**L4** 汇总 | 600 点、四误差先累加再除以 8，rottransratio 仅保留**最后一次运行**值；不假设 MATLAB RNG 同 seed 输出相同 | TC-F-EXP-002 |
| EX-G04 | **L3** 固定帧号状态机；**L2** 历史结果 | 更新率 1/5/11/21、600×4/600×1、三个当前源 save 字段；主 akkoeff 和周期基线状态不串扰 | TC-F-EXP-003 |
| EX-G05 | **L3** 独立构造错误/安全合同 | 输出路径冲突、MAT 写失败、近度/solver 非有限；日志含 RunId、原错误码和独立失败数，不能把异常计为有效 | TC-NF-SAFETY-001、TC-NF-RECOVERY-001 |

历史 **MAT 类型/shape 字典已实测**；但算法的**独立 L3 逐节点参考**、完整 L2 逐值及 SHA-256、L4 统计、批准容差、CLI schema、生产写入/回读与 CTest 仍未建立，不能以最终结果文件能够被读取替代算法真实性测试。

**状态**：五类入口参数与当前 save 契约已文档化，**历史输出 dtype/shape 已由 582/582 matio 数据字典确认**；C++ Runner、新输出、独立 L3 逐节点参考、误差容差及接口冻结仍未完成。

### 8.1 具体错误触发、CTest 候选与依赖边界

`ExperimentErrorCode::DataMismatch` 的候选范围与 [contracts.md §3.3](contracts.md) 对齐：在**可验证的**参数、预期变量名/shape、帧数/有效样本总数、Manifest 与实际数据不一致时报告 DataMismatch（候选 exit 5）；不能将 MAT 底层非法 class/rank/压缩解码错报为 Runner DataMismatch，也不能因不同 RNG 的逐样本数值不等报错。 `SolverFailed` 用于求解器实际失败，`OutputFailed` 用于 Writer/路径/回读失败（保留底层 `MatErrorCode` 和 ErrorDetail），`IncompleteRun` 用于已记录的运行中断或无法完整聚合；Runner 不新增其他退出码。全部 CLI 映射继续以 contracts.md §3.1 为准。

| TC/AC 组 | [TEST_PLAN.md §3](../TEST_PLAN.md) 中的拟议实体/CTest | 未来必须验证 |
|---|---|---|
| TC-F-BIAS-001/002/003 / 对应 AC | `tests/integration/test_bias.cpp` / `egomotion.bias.*` | 两 Bias 的 N、xwerte、六类 ywerte、固定 RNG/reset、独立 L3 与 L2/L4 |
| TC-F-EXP-001/002/003 / 对应 AC | `tests/integration/test_adaptive_experiments.cpp` / `egomotion.experiments.*` | 35/600 点、8 次平均与末次比值、周期 [1,5,11,21]、Session 隔离 |
| TC-I-RESULT-001 / AC-I-RESULT-001 | `tests/integration/test_adaptive_experiments.cpp` / `egomotion.experiments.*` | 只写新 output/RunId 下的当前 save 字段、MAT dtype/shape/回读 |
| TC-NF-LOG-001 / AC-NF-LOG-001 | `tests/contracts/test_runlog.cpp` / `egomotion.logging.*` | RunId、来源、seed、失败/无效分母、错误上下文与真实日志 |
| TC-NF-SAFETY-001 / AC-NF-SAFETY-001 | `tests/contracts/test_output_safety.cpp` / `egomotion.safety.*` | 重复 RunId、路径逃逸及已有文件非覆盖 |

**CMake/头文件归属候选**：按 [ARCHITECTURE.md §4](../ARCHITECTURE.md)，`egomotion_experiments` 依赖 `egomotion_domain`、`egomotion_contracts`、`egomotion_sampling`、`egomotion_simulation`、`egomotion_estimators`；**不链接 `egomotion_matio`、不包含 `matio.h`**。Runner 的必要 facade/RunOutcome 接口可候选放在 `include/egomotion/experiments/`，输出 Manifest/实验编排和入口配置解析属于 `src/experiments/` 私有层（最终公开头与参数位置待 P0.8）。MAT I/O 具体 Writer 只在 CLI Composition Root 装配。上述测试文件及 target 均未创建，真实测试/头文件/依赖版本未冻结。

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
