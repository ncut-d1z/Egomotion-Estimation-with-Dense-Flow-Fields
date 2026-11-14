# 实验 Runner 与结果写出——详细设计草案

> **长期参考边界**：本 Runner **不要求 MATLAB/Octave 运行或采集 L1**；原 .m 仅作只读源码依据，结果验收使用 L2 已存 MAT、L3 独立逐帧/逐轮参考与 L4 统计，参见 [长期政策](../golden/NO_MATLAB_EXECUTION_POLICY.md)。

> **LLD v0.15草案（明确CompositionRoot装配合法Bias噪声策略、Runner负责实参门禁、底层只用NumericalErrorCode；未运行CTest）。** 来源：bias_test/data_generation 中 2 个入口；adaptive_matched_filter_test/data_generation 中 3 个入口。对应 P5、P8、REQ-F-BIAS、REQ-F-EXP、REQ-F-CLI 与 REQ-I-RESULT。

## 1. 实验入口及源代码实际参数

| Runner（拟议） | MATLAB 函数 | 关键原始参数/范围 | 当前脚本实际写出 |
|---|---|---|---|
| BiasExperimentRunner / occluded | bias_test_nonspherical | corenumber seed、iterationnumber；3 个噪声因子 [0.6,2.4,9.6]；单个脚本 durchlaeufe=1；N=6·4^i | 每 run 子目录 xwerte、ywerte1…6 |
| BiasExperimentRunner / proportional | bias_test_spherical_prop_error | 其他参数近似但 N=8·4^i 且噪声不同 | xwerte、ywerte1…6 |
| InitializationRunner | agent_in_sphere_initialization_test | 圆轨迹 35 点、二十面体级别 2、每帧零阶重置与继承 | allemittransfehler、allemitrotfehler、alleohnetransfehler、alleohnerotfehler |
| AdaptiveComparisonRunner | agent_in_sphere_adaptive_matched_filter_test | 600 点正弦轨迹、二十面体级别 4、8 次重复、addederror 参数 | 上述四类平均误差、rottransratio（最后一次运行的缓冲） |
| UpdateRateRunner | agent_in_sphere_adaptive_matched_filter_many_parameter | 600 点正弦轨迹、二十面体级别 2、单次运行、周期 1/5/11/21 | allemittransfehler、allemitrotfehler、akrot |

**DEC-004 已由所有者选定**：[处理规则](../decisions/DEC-004_HISTORICAL_MAT_FIELDS.md)。当前 C++ Runner 只保存现有生成脚本实际有效的 save 字段；历史 MAT 完整只读保留，额外字段标 `HistoricalOnly`，来源未知不阻塞开发，不新增伪字段。

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
- **Bias 原始合成流接口的三种数值不可混淆**：两种 `fixed_*_randomnear.m` 的 `durchlaenge` 是**合成**理想光流的逐点模长平均 `mean(||transfluss+rotfluss||)`，它不能替换为 `mean(||transfluss||)+mean(||rotfluss||)`；后者两项均值只属于独立分量诊断（类似 `erzeugeflussausnear.m` 的两个分量输出）。原 `near` 输出形参在循环中为**每次覆盖、最后一条近度的标量**；`FlowSynthesisResult.nearness` 是新增的全部逐点内部值，`near_last` 才对应源返回标量。新版候选布局及精确抵消反例见 [sampling_noise.md §6/§7.1.5](sampling_noise.md)。Runner/噪声模型不得误以两个分量均值的和作为缺损视场的噪声尺度；标量 `near_last` 不能冒充 N×1 的 MAT 原作者保存数据（原 Bias save 不包括 near）。
- Bias 的 40 个历史子目录是 40 份结果来源，不能错误解释为一个源运行的 40 个内循环。
- AdaptiveComparisonRunner 的前四种结果累计后除以 **8**；rottransratio 按原脚本在循环中的覆盖次序保留。UpdateRateRunner 的输出系数状态分别保存，不和当前主 akkoeff 共享可写对象。
- RunContext 跨实验不可共享可变状态；**单步 Adaptive 求解失败不得提交无效主系数**；如果依赖状态无法安全延续，就停止本条连续序列。独立样本或独立实验可记失败后继续其他样本，前提是记录真实有效分母；不得将失败静默改成正常结果。

**2026-11-14 Bias噪声策略互斥的业务编排合同**：`BiasOccluded`仅调用`MeanMagnitudeGaussianNoise`，源`fixed_kugel_symmetric_perfectpoints_randomnear.m`逐分量乘`mean(||p_ideal||)`；`BiasProportional`仅调用`ComponentProportionalGaussianNoise`，源`fixed_propfehler_kugel_symmetric_perfectpoints_randomnear.m`逐分量乘相应`p_ideal,c`。任何**一次**场景不能两个模型叠加，否则会额外消耗高斯抽样、改变源分布；源每点x/y/z各抽一次`randn`，严格`3N`个normal（零系数、零分量、总流长零也不优化跳过）。`IFlowNoiseModel`调用方应以`ExperimentKind`确定**唯一模式**并注入对应实现，不能用两个布尔模型开关各自独立生效。**先加三维高斯、再用`P=I-dd^T`投影**是源实际运算次序；投影后的三个坐标噪声一般相关，即使原`randn`输入独立，也不得用额外两维噪声抽样替代`3N`次。固定`d=(1,1,0)/√2,p=(1,-1,0),g=(1,0,0),c=1`时，两个单独噪声模型的最终增量分别为`(1/√2,-1/√2,0)`和`(1/2,-1/2,0)`，与误选“双模型”产生的和不同。严格依据[采样LLD §2.1](sampling_noise.md)，该测试为[TEST_PLAN §2.2.1](../TEST_PLAN.md)的隔离数值输入，不把单点夹具冒充原6/8面Bias运行。**唯一合法模型必须由CompositionRoot按已解析实验入口装配并在Runner首次抽样前注入**：项目公开CLI中不存在可同时打开两种噪声的合法组合或双开参数；解析非法/矛盾实验请求时在RNG消费前由CLI/Runner层返回`ExperimentErrorCode::InvalidParameters`，按真实Parse/Simulation阶段和唯一exit映射分类。若CompositionRoot错误地注入另一个具体模型，由于窄接口无`kind()`，该错误只能通过装配契约/替身集成测试发现，**不得假称Runner存在实际无法实现的运行时类型识别机制**。下层`egomotion_simulation`仅调用已声明的`IFlowNoiseModel::apply(const FlowField&,double coefficient,IRandomSource&)`，失败为`Result<FlowField,NumericalErrorCode>`，不能依赖`ExperimentKind`或`ExperimentErrorCode`，否则产生架构反向依赖。源Noise `fehlerprozent`必须显式绑定为`coefficient`，而非从不存在的隐式状态读取。详见[采样LLD §2/§3](sampling_noise.md)；该接口分层修复不改源公式、不新增公共enum，未执行Runner/CTest。

**2026-11-14 实验/仿真分层与错误码所有权的静态反例**：原`generate_bias_flow`伪代码漏写`coefficient`形参，新增非法模式判断时还一度将`ExperimentErrorCode::InvalidParameters`放到仿真层，违反拟议[ARCHITECTURE §4](../ARCHITECTURE.md)的“simulation→contracts/sampling，不依赖experiments”。已在[采样LLD](sampling_noise.md)纠正并与本Runner保持一致：**解析ExperimentKind并选取/拥有具体IFlowNoiseModel实现**属于`ApplicationCompositionRoot`，与本Runner此前的“构造注入”所有权规则一致；**Runner负责已有参数及调用前置条件的门禁**，非法输入由上层在RNG消费前返回`ExperimentErrorCode::InvalidParameters`。现有`IFlowNoiseModel`不提供`kind()`，因此Runner不能仅靠一个抽象引用证明注入实现的具体类型；正确组合必须在CompositionRoot完成并用集成替身测试固定校验，而不是在Runner内部自行new或加RTTI；**实际3N抽样、单模型噪声及投影**属于仿真具体实现，通过`IFlowNoiseModel::apply`返回`NumericalErrorCode`。对下层失败先记录其`NumericalErrorCode`与`ErrorStage::Simulation`及RNG样本索引；若该样本可按既定独立样本纪律继续，不必伪造整次Runner失败；若因必需场景不能完成而中止运行，外层可用**已有`ExperimentErrorCode::IncompleteRun`**包装并按RunContext实际可靠产物判断`Incomplete`或`Failed`。**不要把生成随机噪声的错误随意命名为`SolverFailed`**，也不得把底层数值错误强转成Experiment枚举；每次真实包装依DEC-008-04在`std::forward_list<ErrorFrame>`保留原错误帧，不可强行转换枚举/改变根因。实验控制流在做完模式校验前不能消费任何RNG。给出固定RNG计数和模拟下层错误传播的既有TC输入见[TEST_PLAN §2.2.2](../TEST_PLAN.md)。这是源与架构一致性的设计合同，未实现/构建。

## 3. 候选实验和报告结构

~~~cpp
// 仅候选合同，不是生产 C++ 文件。
enum class ExperimentKind {
    BiasOccluded, BiasProportional,
    AdaptiveInitialization, AdaptiveComparison, AdaptiveUpdateRates
};
// 五个入口各自独立 Config 类型；CLI 层若需统一分发才用 variant。
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
| adaptive comparison | addederror、savestring；stellen=600、sinus amp=0.5、length=1、periods=2、height=0.3、level=4、anzahldurchlaeufe=8 | i=2..599 | 四类误差各 600×1，另保存 `rottransratio` 600×1（最后一次运行的缓冲）；共五个 save | without_errors：五个 double[600×1]；with_errors：七个 double[600×1]，额外两份 `alleadap*` 为 **`HistoricalOnly`**，只读保留、不纳入新 `save` |
| adaptive rates | savestring；stellen=600、同一正弦轨迹、level=2、period=[1,5,11,21]、单运行 | i=2..599；四条周期 baseline | `allemittransfehler`、`allemitrotfehler` 各 600×4，`akrot` 600×1；**当前仅三个有效 save** | many_parameter：四个 double[600×4] 与两个 double[600×1]；**额外 `alleadap*`、`rottransratio` 均为 `HistoricalOnly`**，只读保留；新输出仅含当前三个有效 `save` |

§9 的「历史归档」只反映已存 MAT 变量名、MAT class/dtype、shape 和文件数，**不得反写为现行脚本的有效 `save`**。按 [DEC-004](../decisions/DEC-004_HISTORICAL_MAT_FIELDS.md)，对当前未写出的额外字段统一标记 `HistoricalOnly` 并只读保留；**无需查清历史生成原因**。新 Runner 不补写这些字段；新结果缺少分析必需字段时明确报错（见 [分析 LLD](analysis_plotting.md)）。

**必须保留的源码特殊行为（已核查）**：

两个 Bias 入口都在第 58 行设置 xwerte(i+1)=(8*4^i)*(6/8)=6*4^i。**对完整球面 Bias，真实采样 N=8*4^i，但作者存储的 xwerte 仍是 6*4^i。**这是原脚本的记录/横轴语义与实际样本数量不一致，不得为了“修正数量”而悄悄改写作者 xwerte MAT。设计应同时输出：
1. original_xwerte：忠实复制源脚本的存储值；
2. actual_flow_count：内部诊断元数据，按实际方向数量记录；
3. analysis 默认使用 original_xwerte 对照原图，任何使用 actual_flow_count 的图必须另行标为分析增强而非原版复现。

**2026-11-12 AC-F-BIAS-003 源数据契约纠偏**：原分析图的参考线是 `100/sqrt(xwerte_last(j))`，不是 `100/sqrt(N_actual(j))`，其中 `xwerte_last` 来自**最后一个独立运行目录**而不是多个 RunId 的均值。以完整球面 level=2 为例，`N_actual=128`、`xwerte=96`；原理论参考值 `100/sqrt(96)≈10.206207`，按实际N替换得到 `100/sqrt(128)≈8.838835`，相差比值 `sqrt(4/3)`。由[分析LLD §5.3](analysis_plotting.md)维护准确的单坐标轴 `loglog`、显示窗口与13条曲线/仅5个图例文本的约束；Runner 只保存原七个 `save` 字段，`actual_flow_count` 为内部诊断元信息，**不添加新的历史 MAT 变量，也不在生成阶段重算或修补原图**。这项合同修复未建立真实 Runner/Analyzer或通过CTest。

Bias 六组 ywerte 对应顺序依次是：Franz 平移、Franz 旋转、Original KvD 平移、Original KvD 旋转、Modified KvD 平移、Modified KvD 旋转，六者都调用原 transfehler_div 的**尺度对齐向量差**，不能将名称含 rot 的列改为测地角。实验分析脚本在现有代码中主要读取 ywerte3…6。

### 5.0.1 Bias 六类 ywerte 对零真值与原误差函数的定义域（2026-11-12）

两份原 Bias 生成入口 `bias_test_nonspherical.m:72–77` 和 `bias_test_spherical_prop_error.m:72–77` 对六个平移/旋转估计**全部调用** `transfehler_div(theotrans/theorot, estimate)`，而**没有**调用 `normrelfehler`。所以六个 `ywerte1..6` 均是**按真值模长缩放估计后的欧氏向量差**，不是差的平方、方向角（deg）、相对百分误差或测地角；源函数注释的 *pointwise squared error* 与实际执行的 `sqrt` 返回值不同。由 [KvD LLD §1.1](kvd.md) 与 [SRS 的 AC-F-ERROR-001](../REQUIREMENTS.md) 确定其准确数学/单位归属。

**候选安全失败与原版数值须分开**：在原 `transfehler_div(0,2e_x)` 的代数路径中，真值范数为0且估计范数非零，函数会得到**数值0**；这并不说明估计方向正确。C++ 对零真值/零估计应依既有 `ZeroNorm/UndefinedAngle` 合同显式拒绝和记录 undefined/failed，而不能把这种“源零结果”写入已完成的有效样本均值。原 Bias 正常入口用 `getvektor` 产生随机单位真值方向，此零真值是**注入的负例**，不宣称原标准40组归档一定含这类样本。输出 `ywerte` 均值的有效分母/预期重复次数不应因拒绝而偷偷改变；独立样本失败按现行 `RunOutcome Incomplete/Failed` 与底层失败原因处理，并保存可审计部分记录，不能将空白 y 值填0伪装完整结果。该规则属于已选失败纪律的具体边界，非原 MATLAB 异常输出实测。

维持 `TC-F-ERROR-001`、`TC-F-BIAS-001/002/003` 与 `TC-NF-RECOVERY-001` 原编号，不引入新的MAT字段，也不把候选显式拒绝当成原程序实际行为或已经完成的生产C++/CTest。

### 5.1 Adaptive 两个输出字段的物理单位与加噪前来源（AF-088/089，静态审阅）

**`akrot` 的单位**：原只读 `adaptive_matched_filter_test/data_generation/agent_in_sphere_adaptive_matched_filter_many_parameter.m:79,91–93,199–203` 先将 `akrot=zeros(600,1)`，在帧 `i=2:599` 对 `bestrot=reldrehvektoren(i,:)'` 计算：

\[
\mathrm{akrot}(i)=\frac{360}{2\pi}\sqrt{\boldsymbol\omega_i^\top\boldsymbol\omega_i}=\frac{180}{\pi}\|\boldsymbol\omega_i\|\quad\text{(degree)}.
\]

这里 `bestrot` / `reldrehvektoren` 为**当前轨迹的真实相对旋转向量（rad）**；`akrot` 为其**角位移模长的度数（deg）**，不是 Adaptive 的估计旋转误差、不是 rad，也不是在没有时间步长时可解释的角速度。第 1、600 行保持源预分配的 0。原对比入口也在内循环计算 `akrot(i)`，但**未 `save akrot`**，故新 Comparison 仍只保存前述五类字段，不能错误扩展当前 save。未来可手算的固定输入候选：`bestrot=(0,0,0)` → 0°，`(0,0,π/2)` → 90°，`(0,0,π)` → 180°；实际轨迹逐帧 `bestrot` 与输出值仍须独立 L3/CTest。

**`rottransratio` 的计算来源**：原 `agent_in_sphere_adaptive_matched_filter_test.m:83–90` 在 `erzeugeflussausnear` 中以原理想流场得到 `durchtransfluss`、`durchrotfluss` 两个**逐样本模长的平均值**；随后第 86 行调用 `nonpropfehler` 修改 `setofpoints`，最后第 90 行才赋值 `rottransratio(i)=durchrotfluss/durchtransfluss`。因此虽然**赋值发生在加噪之后**，用于除法的两个量都在**加噪之前已固定**，不允许从含噪观测重新估计比值，也不能把比值误写为 `mean(rot_component_norm/translation_component_norm)`：

\[
q_i=\frac{N^{-1}\sum_{j=1}^{N}\|\boldsymbol p^{\omega}_{ij}\|}{N^{-1}\sum_{j=1}^{N}\|\boldsymbol p^{t}_{ij}\|},\qquad \boldsymbol p_{ij}^{\rm ideal}=\boldsymbol p^{t}_{ij}+\boldsymbol p^{\omega}_{ij}.
\]

`rottransratio` **无量纲**，在外层 8 次运行中只是**最后一次运行的缓冲，不求平均**；其 600×1 首尾零是源 `zeros` 预分配/有效帧范围的离散行为，不应把那两行解释为有定义的分量比值。若 `durchtransfluss=0`、输入非有限、实际分量均值无定义，则 C++ 应按现有 `NumericalErrorCode` / `ExperimentErrorCode` 候选合同返回带原始原因的非成功；原 MATLAB 的除零传播并没有本轮运行证据，不得伪称已逐值验证。无噪/加噪同一固定理想场的测试应检查 **ratio 不依赖后来注入的噪声样本**；如改动理想流场本身则不能要求相等。

**范围限定**：以上只补足当前 Comparison / Update rates 的 `TC-F-EXP-002/003`、`TC-F-AN-001` 的候选字段来源及单位；旧 `many_parameter` MAT 中的 `rottransratio` 仍为 `HistoricalOnly`，不得借这段公式给新 Update rates 输出补造该字段。独立 L3 数值、Reader/Writer、真实 C++/CTest、DEC-005/008、AF-049/050/051 依然未完成。

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
enum class RunCompleteness { Complete, Incomplete, Failed }; // Manifest 最终状态
struct RunOutcome {
    RunId run_id;
    RunCompleteness completeness;
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

**项目所有者确定每类 Runner 使用独立强类型配置**；CLI Composition Root 仅在必要时用 `std::variant` 作命令分发，不把它泄漏到算法与 Session 入口。**单个源脚本的参数默认值不能因为复用 Runner 而外溢到另一实验**。RunContext 应独占 RNG、输出位置与本次结果 Manifest；没有全局状态。

### 6.1 RunContext 的最小所有权/接口候选（非编译代码）

RunContext 属于 **egomotion_experiments** 的一次性运行上下文，不与 `MatResultReader`、`RunMatWriter` 或 `MatDenseArray` 建立生产级直接链接。Composition Root 负责确认**仓库根固定的 `output/`** 与可选的用户 `--run-id`（未指定时由独立 OS 随机源产生 RunId），独占创建 `output/<RunId>/`，并构造具体 Writer 后以 **contracts 的 IResultWriter** 窄接口注入；RunContext 只持有已验证的 `RunId` 和所需上下文。**CLI 不提供、也不解析自定义输出根参数**。

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

- **谁拥有谁**：Composition Root 保证 `IResultWriter&` 的有效期长于本次 Runner 调用；RunContext **独占 RNG**、RunId/结果计数/Manifest，Session 只持有其自身系数状态且不能跨 RunContext 共享。**固定 `output/` 根由 Composition Root 从已确认的仓库位置确定并验证**，CLI 只接受可选 `--run-id`；RunMatWriter 仍须二次校验固定根、RunId 和最终文件路径，Runner 不按任意用户字符串拼接文件路径。
- **失败与释放（DEC-008-04/06已决定）**：固定RNG耗尽、子求解器失败、MAT写失败必须在`Result<RunOutcome, ExperimentErrorCode>`的**失败alternative只保留`Failure<ExperimentErrorCode>`与全部`std::forward_list<ErrorFrame>`因果包装历史**；不能用半成功或默认RunOutcome冒充成功。`RunContext`独占持久化所需的attempted/completed/valid/failed/undefined、文件SHA及`Complete/Incomplete/Failed` Manifest。部分真实已验证产物允许保留供审计但不得作为完整实验结果，亦不自动删除或覆盖已有结果。Manifest落盘失败时不得报告Complete或谎称日志已持久化。独立样本失败可继续后续独立样本；依赖状态错误不提交，不能安全恢复则停止当前轨迹。
- **边界**：`BiasExperimentConfig` 负责原入口 seed/最大 level/三个噪声乘数，`BiasFlowParameters`（采样 LLD）负责**单个流场**，不能彼此覆盖默认值。配置采用**每入口独立类型**，CLI 层可选择 variant 分发；RunContext 默认不可复制、单次会话不可并发，Writer 在 Composition Root 装配；构建期的类移动能力/线程合同尚需真实验证。

**RunId 自动命名（所有者已选择）**：不再有输出根 CLI 选项，固定 `output/<RunId>/`。由 Composition Root 使用**独立于算法随机源的 Linux OS 随机源**生成 128 位随机标识，推荐 `run_<32位十六进制>`，以独占目录创建确认唯一性；用户传入 `--run-id` 则严格校验 ASCII 字符 `[A-Za-z0-9_-]`（1–64）并拒绝重复。不能使用 RunContext 的 uniform/normal 来生成 RunId，避免改变科学实验 RNG 调用顺序；无法获取随机源/创建目录时直接返回错误。Writer 仍需做文件系统层二次验证。

### 6.1.1 DEC-008-06 所有者已决定：单次失败与RunContext部分结果分离（2026-11-12）

此节是**已决定设计**（`DecisionRecorded`），不是只供讨论的候选失败策略。`Result<RunOutcome,ExperimentErrorCode>`仅在整次实验的必要条件全部满足时返回`success(RunOutcome{Complete,...})`；任一必要阶段失败则返回`failure(Failure<ExperimentErrorCode>{code,detail})`，不允许在失败alternative里暗藏“部分成功结果”，也不能`success`返回一个语义上不完整的RunOutcome伪称成功。`Failure`记录SolverFailed/OutputFailed/IncompleteRun等现有代码及实际底层原因；Runner不改变原`ErrorStage`或丢失RunId。

| 情形 | 单次Result / RunContext责任 | 可对外宣称的持久状态 |
|---|---|---|
| 全部必需计算/统计与MAT结果已独立回读核实，Manifest可靠写入 | Runner返回成功的`RunOutcome{Complete}`；RunContext保存已核字段数、hash、完整分母 | `Complete` |
| 已完成一部分**可核实身份**的MAT/帧，但缺剩余计算/某次聚合失败 | Runner返回`Failure<ExperimentErrorCode>`；RunContext保持已完成/失败/无定义计数及已核部分文件；不改变8轮聚合分母 | Manifest如能可靠写入则`Incomplete`，不产生完整结果 |
| 参数或输入先失败、无任何可靠产物；或输出身份/一致性不可证明 | 返回具体Failure，记录可知的错误上下文；不能填0伪造成功值 | 若有合法的运行上下文且Manifest可写则`Failed`；CLI解析前尚无RunId时仅报parse错误，不制造Manifest |
| 已发布数据但最终回读/Manifest写入故障 | Writer仅负责每个文件的Status；RunContext不得把完整运算宣布成功，Runner返回`OutputFailed`并附原因；不能删除/覆盖旧产物 | 只有实际写出并可核实的状态可在Manifest中声称；Manifest写失败时须报告**持久化未确认**，不能虚称`Complete/Incomplete`已落盘 |
| Adaptive主/比较状态在本帧任一必需步骤失败 | 本帧改动先存于暂存，主系数/周期baseline均不局部提交；依赖轨迹中断，保留先前有效状态 | 记录`Incomplete/Failed`及真实完成帧数、失败原因 |

**实务约束**：`RunContext`负责计数和Manifest事务状态，`RunMatWriter`只负责**单文件**非覆盖发布/回读的成功或失败，`AdaptiveMfaSession`只负责自身系数暂存/后验提交。不能在Writer成功后自动把整个Runner标`Complete`，也不能依靠Result对象来保存部分MAT。对于Comparison原8次循环，第5次失败时expected=8、completed=4；拒绝“改除以4并将原8次结果标成功”。任何异常导致Manifest不能持久化，应尽可能向独立stderr发送诊断并如实说明日志/Manifest是否成功，不保证诊断渠道也永远可用。

**状态**：所有者决议已记录，但`IResultWriter`/RunContext/Manifest/Result的真实C++接口、异常/TOCTOU/CTest并未获得实施许可；H-10/H-11等六项保持无限期延期。此条决议不解除它们。

### 6.1.2 DEC-008-04：每一层包装都必须保留到RunContext/Manifest（2026-11-13）

DEC-008-06的`RunContext`只管理**运行级**部分结果/Manifest，不改变DEC-008-04确定的**完整错误链表示**。在`Result<RunOutcome, ExperimentErrorCode>::failure`中，`Failure.code`记录最外层例如`SolverFailed/OutputFailed/IncompleteRun`；其`detail.causes`是`std::forward_list<ErrorFrame>`，**最近包装层在链首、最深根因在链尾**。无新包装时原样传递，不把每一次函数调用都当作包装。每次真正换外层ErrorCode时，必须先创建直接内层的`ErrorFrame`、外层字段，并在**尚未移动原链**的新链中成功`emplace_front`；仅在分配完成之后，才用分配器相容的`splice_after(new_head, inner.causes)`把旧内层链插到新节点后面（含枚举家族/code/stage/message/已知RunId/path/variable/index）；不允许清空链再只填最深`source_error_code`。

**真实流程例子（仅设计事实）**：

~~~text
Reader:   MatErrorCode::InvalidShape@Decode                causes=[]
Analyzer: AnalysisErrorCode::ShapeMismatch@Analyze         causes=[Mat]
Runner:   ExperimentErrorCode::DataMismatch@Simulation     causes=[Analysis, Mat]
CLI:      serialize same Failure, no new wrapping          causes=[Analysis, Mat]
~~~

`RunContext`对**每一次失败事件**保存当前完整最外Failure及其全部causes；可用独立顺序容器保存若干Failure事件（每个事件内部的错误链唯一采用`std::forward_list<ErrorFrame>`），不在失败`Result`里隐藏`RunOutcome`。Manifest成功落盘时按同样的外→内顺序序列化`causes`数组，并**始终同时保存顶层`error_domain`、`error_code`、stage及真实RunId**。若没有任何包装帧，`causes=[]`仍必须保存顶层家族，不因同名`InvalidShape`或`OutputFailed`而误判来源。若先有SolverFailed，随后**独立发生**Manifest写入失败，必须记录为**两个独立错误事件**，而非伪造一条不真实的因果单链。无法落盘时应如实返回`OutputFailed`及可得的错误信息，不能假报“错误链已持久化”；`std::forward_list::push_front`分配内存也可能异常，不能在内存失败时假造成功记录。

**异常安全负例（2026-11-13）**：旧的“先`std::move(inner.causes)`、再`push_front`分配新帧”如果发生`std::bad_alloc`，会使调用方失去旧链。现在Runner必须遵守[公共合同 §3.0.2](contracts.md)与[TODO §2.2 F](../TODO.md)的**先分配新节点、后`splice_after`转移旧节点**次序；新帧分配故障时旧`inner.causes`应保持原样。`splice_after`本身要求相同allocator且不抛，但以后真正保存RunContext/Manifest、序列化JSONL仍可能失败，不能说“完整链保证已落盘”。对应已有`TC-NF-RECOVERY-001/TC-NF-LOG-001`待测。

**对Writer/Session的映射**：Writer仅提供单文件`Status<MatErrorCode>`与其错误链；Runner包装为`ExperimentErrorCode::OutputFailed`时保留前一`Failure`为第一个`ErrorFrame`并继承旧链。Adaptive帧中某比较或主求解出错时，暂存主/比较系数不能部分提交，但已经发生的错误包装链需保留，不得因回滚状态而销毁失败原因。Comparison八次第5次失败仍expected=8、completed=4，若Manifest可写则Incomplete，若写失败则明确持久化未确认；不能仅在`RunContext`存最初根因而丢掉中间包装。

以上是**已决定的文档行为合同**，实际Writer、RunContext、链分配/拷贝/移动、JSONL/Manifest持久化顺序与原生CTest未实施。复用已有`TC-NF-RECOVERY-001`、`TC-I-RESULT-001`、`TC-NF-LOG-001`、`TC-NF-SESSION-001`，不新建TC。参见[contracts `3.0.2](contracts.md)及[TODO `2.2 F/H](../TODO.md)。

### 6.2 输出 Manifest 推荐字段

| 字段 | 语义/约束 |
|---|---|
| run_id / experiment_kind / source_entry | 唯一运行身份与原 MATLAB 函数名称 |
| completeness / expected_repetitions / completed_repetitions | **DEC-008-06已决定**：Complete仅在全计算/回读/Manifest可靠发布成功；部分已核产物Incomplete、无可靠结果或前置失败Failed；八次聚合完整不变分母；对应的每个失败事件均依DEC-008-04保存整个ErrorFrame前插链 |
| repo_sha / source_blob_sha / original_mat_root | 可追溯版本，原数据只读 |
| original_parameters / default_origin | 每个数值来自入口形参还是源文件固定常量 |
| subdivision_level / actual_flow_count / original_xwerte | 区分物理 N 与作者横轴存储值 |
| uniform_calls / normal_calls / seed | 观测生成和噪声调用统计 |
| mat_files | 文件名、变量名、dtype、rank/dims、输出 SHA；不能写 origin |
| attempted / valid / failed / undefined | 不隐藏失败、未定义或仅部分生成的结果 |
| environment / stdout / stderr / exit_code | Debian/Ubuntu、构建源 SHA、真实执行证据 |

## 7. 结果写入与多次运行的失败路径

- **Bias**：每个 corenumber 的结果相互独立，写入相应新 RunId 子目录；原始已有编号目录 1..40 是历史参考，不是 C++ 自动覆盖位置。
- **Comparison**：四种方向误差数组仅在**八次有效完成**时按原算法先累计后除以 8；rottransratio 仅保留**第八次**（最后一次预定运行）的数组。若某次失败，整次聚合标记 `Incomplete`，不改为除以 7 或替代缺失的最后一次；部分结果可留证，但不作为原算法完整输出发布。
- **Update rates**：四组 `mitkoeff` 只允许先旋转、按 `mod(i,[1,5,11,21])==0` 从**本帧主求解前**的主 `akkoeff` 周期复制；对照的 `oneit` 第一输出在原脚本绑定给**废弃变量 `dumi`**，绝不可写回 `mitkoeff`。只有主 `oneit` 返回值每帧提交 `akkoeff`。这不同于初始化/Comparison入口对照 `ohnekoeff/mitkoeff` 的同帧迭代返回覆盖语义。当前源 save **只有三个 MAT**；历史附加文件标 `HistoricalOnly` 并保留字段级差异报告，**不得在新结果补造这些文件**。
- **初始化**：每次创建独立系数起点，不能重用此前 600 帧轨迹的状态；首尾行按原 zeros 预分配语义。
- **原始数据保护**：任何路径重叠、文件已存在、变量写入失败应返回带 RunId 的明确错误，不允许把部分成功伪装为完整。
- **失败策略已由所有者确认**：独立采样/运行失败记录并可继续其他独立单位；Adaptive 连续状态不提交失败步，无法保证状态可用则停止该轨迹；损坏输入或输出写失败导致依赖任务失败；保留失败日志与可追踪产物，不自动删除旧文件/重试覆盖。需在未来真实 C++/CTest 下验证该策略，不声称原 MATLAB 的异常传播已实测。

## 8. L2/L3/L4 参考数据、验收和未批准合同

| 参考用例 | 证据来源等级 | 最少独立参考/预期字段（不是被测 C++ 自算） | 具体 TC |
|---|---|---|
| EX-G01 | **L3** 采样/固定注入 RNG；**L2** 作者归档 xwerte/ywerte；**L4** 跨 40 runs 统计 | 两类 Bias i=0/1/2 的 N、原存储 xwerte、六项 ywerte 的 shape/指标；固定 RNG 序列的消费次序与独立统计分母 | TC-F-BIAS-001/002/003 |
| EX-G02 | **L2** 历史四类 35×1；**L3** 独立帧/状态 | 初始化 35 点、四类误差 shape、首尾数组与主/比较系数更新的独立参考 | TC-F-EXP-001 |
| EX-G03 | **L3** 固定 8 次聚合；**L2** 历史五字段；**L4** 汇总 | 600 点、四误差先累加再除以 8，rottransratio 来自**加噪前两种理想流分量的平均模长之比（无量纲）**且只保留**最后一次运行**值；另核分母为零/非有限状态，不假设 MATLAB RNG 同 seed 输出相同 | TC-F-EXP-002 |
| EX-G04 | **L3** 固定帧号状态机；**L2** 历史结果 | 更新率 1/5/11/21、600×4/600×1、三个当前源 save 字段；`akrot=(180/π)·norm(bestrot)` 单位 **deg**，第一/末行源预分配为 0；period比较的`coefficients_next`绑定原`dumi`并**弃置**，周期刷新用主求解前系数；比较状态只可旋转/刷新，主akkoeff每帧提交；固定哨兵预期见§8.0.1 | TC-F-EXP-003 |
| EX-G05 | **L3** 独立构造错误/安全合同 | 输出路径冲突、MAT 写失败、近度/solver 非有限；日志含 RunId、原错误码和独立失败数，不能把异常计为有效 | TC-NF-SAFETY-001、TC-NF-RECOVERY-001 |

历史 **MAT 类型/shape 字典已实测**；但算法的**独立 L3 逐节点参考**、完整 L2 逐值及 SHA-256、L4 统计、批准容差、CLI schema、生产写入/回读与 CTest 仍未建立，不能以最终结果文件能够被读取替代算法真实性测试。

**状态**：五类入口参数、独立 Config、RunContext/Manifest 失败策略与当前 save 契约已文档化，**历史输出 dtype/shape 已由 582/582 matio 数据字典确认**；C++ Runner、新输出、独立 L3 逐节点参考、误差容差及接口冻结仍未完成。

**2026-11-11 Bias源结果边界的静态数学夹具**：在 `d=e_z, mu=1, t=e_x, omega=-e_y` 的独立单方向流场中，`p_trans=-e_x`、`p_rot=+e_x`，故合成流的源式 `durchlaenge=0`，两个单分量平均模长各为1。检查在这个输入下不可使用`1+1=2`替代原合成模长：MeanMagnitudeGaussianNoise即便噪声系数非零也产生0扰动，`nonpropfehler` 路径仍要消耗3个normal。此为**求解器前流场合成/噪声数学边界**而非原Bias的6/8面集合实际运行结果；复用 `TC-F-FLOW-002/003`、`TC-DC-NOISE-001`，正式软件测试与独立L3尚未实施。

### 8.0.1 四种更新周期不是四套每帧自适应系数状态（2026-11-12 原源码合同）

只读原 `agent_in_sphere_adaptive_matched_filter_many_parameter.m:134–182` 定义更新率算法。若未来使用 `AdaptiveMfaSession` 共用封装，必须保持返回值弃置语义：第134–138行先旋转主`akkoeff`与四组`mitkoeff(:,:,k)`，第147–150行按`mod(i,zeitkonstanten(k))==0`从**已旋转但本帧主求解尚未提交**的主系数复制给周期组；第164行`[dumi mittrans mitrot error]=oneit_adaptiv_matched_filter(mitkoeff(:,:,k),setofpoints)`只使用`mittrans/mitrot`分别与真值计算`normrelfehler`，`dumi`不写回周期基线，而返回的`error`也被下一求解调用覆盖、并非保存误差数组的来源；第182行`[akkoeff aktuelltrans aktuellrot error]=oneit_adaptiv_matched_filter(akkoeff,setofpoints)`才唯一更新主系数。这些精确行为要在[Adaptive LLD §5.0.1](adaptive_mfa.md)中维护，Runner不应维护第二套自相矛盾的更新规则。

**可区分正确和错误实现的状态替身**：旋转始终I；主状态和四组对照的源初始常数项均为`A=B_k=1`；模拟每次求解返回系数`C_next=C+100`，运动值不影响状态。帧`i=2/3/5/6`的比较输入必须分别为`period1:[1,101,301,401]`、`period5:[1,1,301,301]`；本帧主求解后的主状态分别为`[101,201,401,501]`（i=4仍执行主更新，故i=5前主为301）。如果错误地把`dumi`写回period5，对照在i=3的输入会变成101；如果先更新主系数再周期复制，则i=5的period5输入会变成401，均能在不调用真实MATLAB或随机源的条件下由**计划中的隔离替身测试**判别。上述标量仅是系数对象的哨兵值，不冒充3×5真实球谐矩阵或原始轨迹L3。

**失败原子性**：保持既定已选Session纪律，在本帧临时状态旋转/复制/比较之后若有必需节点失败，则**主系数和四组对照均不能局部提交旋转/刷新**；否则下一帧可能消费半更新状态。连续序列不可恢复则停止本轨迹，明确`Incomplete/Failed`，不改变统计分母。这个事务保护是**额外的C++安全合同**，不应反向描述为MATLAB已实现原子事务。对应既有`TC-F-ADAPT-003`、`TC-F-EXP-003`、`TC-NF-SESSION-001`，候选测试文件仍为计划，未执行。

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

### 8.2 第一轮 Runner/Manifest 的固定替身验收场景（AF-049，设计补齐）

以下通过**未来独立替身**输入可审查的结果序列，而非调用 MATLAB；此轮**没有**生成结果、编写测试或执行测试。

| 固定场景 | 期望的输出/状态合同 | 最小证据与关联 TC |
|---|---|---|
| Comparison 八次皆有定义；八次提供各不相同的四误差数组和 `rottransratio` | 前四数组分别累加八次、**最后统一除以 8**；`rottransratio` 保留**第 8 次**，绝不求均值或采用之前一次 | 逐次独立输入、期望 600×1、每次计数/最后一轮身份；`TC-F-EXP-002` |
| Comparison 在第 k 次失败或第 8 次没有可用比值 | 不能用七次或 k−1 次修改分母；不伪造“最后一次比值”，整体非 Complete 并保留真实已完成次数与失败阶段 | `RunOutcome` + Manifest 的 attempted/valid/failed/undefined 及底层 Failure；`TC-NF-RECOVERY-001` |
| Update rates 的第 1/5/11/21 周期（含 `akrot` 角位移单位核验） | 每种对照配置独立刷新，主 `akkoeff` 按原帧规则更新，输出仅 **两个 allemit*（600×4）+ akrot（600×1）** | 固定帧号、刷新前/后状态与当前 save 三变量；`TC-F-EXP-003` |
| 新 Update rates 输出被旧分析入口读取 | 没有 `rottransratio` 即 `MissingVariable`，不自动计算/填零或制造历史 `alleadap*` | 新/旧数据来源分类、分析失败状态；`TC-I-ANALYSIS-001` |
| 两个 Bias 独立运行或两个 `RunContext` 交错 | seed/reset、随机流、输出 RunId、Manifest 和结果不可交叉污染；`origin/**` 始终只读 | 固定 RNG trace、独立 RunId/目录和来源 SHA；`TC-NF-SESSION-001` |
| Writer 已存在/失败，或中断后仅有部分 MAT | 使用固定 output 根，非覆盖；失败不能宣称 `Complete`，不得删除旧 RunId 或复制历史额外字段 | `OutputExists/OutputFailed` 底层 cause、剩余产物/日志、只读源不变；`TC-I-RESULT-001`、`TC-NF-SAFETY-001` |

**DEC-008-04/06边界**：还必须区分整次运行`RunContext`状态与单个`Result<RunOutcome,E>`成功/失败，所有真实包装层以`std::forward_list<ErrorFrame>`保留到RunContext可核的事件记录；`Incomplete/Failed`Manifest不意味着Runner返回成功。严格区分“原源码的更新次序/当前 save”、“独立 L3 固定输入参考”、“历史 MAT 实际已有字段”与“已执行 C++/CTest”四类证据。只有最后一类实测并取得相应数值/安全批准时才能关闭相关 AC。**AF-049 仍「未整改」**。

## 9. 原作者归档字段的实测数据合同

本节仅引用已在 [MAT_DATA_SCHEMA.md](../MAT_DATA_SCHEMA.md) 中经 `Mat_VarRead` 读取成功的**历史 MAT 文件内容**。`MATLAB class=double`、`data_type=double`、压缩 MAT v5/zlib；以下尺寸不再是作者 `zeros` 预分配的推测。

| 历史结果组 | 实测变量 | 实测形状与文件数 | 与当前 MATLAB 生成脚本的关系 |
|---|---|---|---|
| occluded_regions | 40 个 xwerte；每个 ywerte1…6 各 40 个 | xwerte: 40×(10×1)，ywerte: 240×(10×3) | 40 个独立结果目录，与两种算法的误差统计关联 |
| proportional_error | 同上 | xwerte: 40×(10×1)，ywerte: 240×(10×3) | 真实输入方向是完整球面，但原 xwerte 仍写为 6·4^i；不得更改 |
| initialization | allemittransfehler、allemitrotfehler、alleohnetransfehler、alleohnerotfehler | 4×(35×1) | 与当前源 save 四种误差字段名一致 |
| without_errors | 四误差变量、rottransratio | 5×(600×1) | 与当前对比实验 save 五字段对应 |
| with_errors | 四误差变量、rottransratio；另有 alleadaprotfehler/alleadaptransfehler | 7×(600×1) | **后两字段为 HistoricalOnly，当前生成入口无对应 save**；未知来源无需先归因，保留原始归档 |
| many_parameter | allemitrotfehler、allemittransfehler、alleadaprotfehler、alleadaptransfehler | 4×(600×4) | 仅前两项在当前生成脚本保存，后两项为 HistoricalOnly；新 C++ 输出不可增写 |
| many_parameter | akrot、rottransratio | 2×(600×1) | akrot 为当前输出，rottransratio 为 HistoricalOnly；新运行结果如需要比值进行该分析则明确报告缺失，旧 MAT 可正常分析 |

**验证边界**：这些历史文件被 matio 读取成功，但没有证明 C++ 新生成的对应数值、RNG 顺序或误差曲线与 MATLAB 一致。仍需 TC-F-EXP-001/002/003、TC-I-RESULT-001 的具体结果回读、比较和差异归因。

## 附录 A：本模块无需运行 MATLAB 的验收参考

按 [长期无需 MATLAB 执行政策](../golden/NO_MATLAB_EXECUTION_POLICY.md) 与 [L2/L3/L4 来源矩阵](../golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md)：原 MATLAB .m 文件仅作为**只读公式、数据布局和顺序规范**；**不要求执行 MATLAB/Octave、采集 L1 或以缺少 L1 阻塞冻结/编码/验收**。

现行实验验收从历史 L2 MAT 对照结果字段、shape 和统计；另以 L3 固定输入独立模拟 8 次聚合、rottransratio 末次值、周期 1/5/11/21 的触发及 Session 主/基线状态。原 MATLAB 运行时的每次 RNG/每帧系数并未保存，也**不需要**运行原脚本采集；仍需真实 Runner、Writer、安全/CTest 和 L4 统计。

本附录不取消真实 C++/CTest、Golden 独立复核、DEC-005/008 以及源数据保护门禁；当前无新的软件测试通过证据。
