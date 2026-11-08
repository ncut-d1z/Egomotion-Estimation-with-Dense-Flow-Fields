# 视线采样、光流合成与噪声——详细设计草案

> **LLD v0.7 草案，未冻结；无生产实现或已执行的 L3 独立数值测试。** 对应 P2/P3、REQ-F-GEOM-*、REQ-F-FLOW-*、REQ-DC-RNG-001、REQ-DC-NOISE-001。

## 1. 来源与输入输出

| MATLAB 文件 | 输入与输出 | 必须保留的行为 |
|---|---|---|
| findeDreiecke.m | 三角顶点、当前/终止细分层级 → 单位球面面中心 | 每次以 A/B/C/D 顺序四分，递归终点对三角形中心归一 |
| erzeugerichtungsvektoren.m | 等级 i → N×6 (8·4^i) | 八面体 8 个初始三角面固定顺序 |
| erzeugerichtungsvektoren_loch.m | 等级 i → N×6 (6·4^i) | 只连接原始面编号 2、3、5、6、7、8 |
| richtungsvektoren_icosahedron.m | 等级 i → N×6 (20·4^i) | 原 12 顶点、20 面表、递归次序 |
| getvektor.m | 随机源 → 3×1 球面单位方向 | theta=(π/2)−asin(2rand−1)，phi=2πrand |
| erzeugeflussausnear.m | 方向、near、t、ω → N×6 流场、平均平移/旋转模长 | 三维切向球面光流与固定号数 |
| fixed_kugel_symmetric_perfectpoints_randomnear.m | i、噪声系数、真值 t/ω → 受遮挡流场 | 距离在 [1,4] 取样、除以 2.5 再取近度倒数；按全场平均模长加噪 |
| fixed_propfehler_kugel_symmetric_perfectpoints_randomnear.m | 同上 → 完整球面流场 | 噪声按每个流分量大小乘以 randn、系数 |
| nonpropfehler.m | 已有流场、系数、随机源 → 加噪流场 | 每个分量加全场**平均光流模长**尺度噪声，再投影到切平面 |

## 2. 几何公式与伪代码

\[
\boldsymbol p_i = -\mu_i\big(\boldsymbol t-(\boldsymbol t^\top\boldsymbol d_i)\boldsymbol d_i\big)-\boldsymbol\omega\times\boldsymbol d_i.
\]

先独立计算平移/旋转分量及其**模长的平均值**（不是平均向量的模长），最后得到合成光流。加噪后的向量都经 \( \boldsymbol p\gets\boldsymbol p-(\boldsymbol p^\top\boldsymbol d)\boldsymbol d \) 投影到球面切平面。

~~~text
sample_triangles(level, geometry):
    choose exact original vertices/faces
    for each original face IN MATLAB ORDER:
        recursively split into A, B, C, D
        normalize terminal face centroid
    return directions IN GENERATED ORDER

generate_bias_flow(level, truth_t, truth_omega, noise_kind, rng):
    directions = required_octahedral_variant(level)
    for each direction: normalized_distance = U(1,4)/2.5; near=1/normalized_distance
    perfect_flow = synthesize(directions, near, truth_t, truth_omega)
    mean_length = mean(norm(perfect_flow_i))
    for each flow component:
        if mean_length_noise: value += Normal()*coefficient*mean_length
        if component_noise:   value += Normal()*coefficient*original_component
    project_each_flow_to_tangent_plane()
    return flow and original statistical fields
~~~

原源文件将 [0.6, 2.4, 9.6] 用作 **randn 直接乘数**；不可解释为必须开方的方差。多个入口共用数学函数不表示其 RNG 初始化和调用次数相同；MATLAB/RNG 的逐调用序列须单独跟踪。

## 3. 候选 C++17 合同（未批准）

**接口定义唯一归属**：下面 IRandomSource、IDirectionSampler、IFlowNoiseModel 的 C++ 类声明仅用于展示**候选签名**，并不表示在 sampling/simulation 两个目标里重复定义接口类。按 [ARCHITECTURE.md §4](../ARCHITECTURE.md) 与 [contracts.md](contracts.md)，最终这三种可注入抽象应在 `egomotion_contracts` 的单一公共接口头提供；`egomotion_sampling`、`egomotion_simulation` 只 include 它们并实现具体类。Result/Failure 与 NumericalErrorCode 也只从公共合同引入，不在模块实现重复声明。正式头文件位置/具体 virtual 语义须 P0.8 审查，不代表已经建立 ABI。


~~~cpp
// 文档接口草案，不是实现。
struct DirectionSet; struct FlowField;
// 具体 BiasFlowParameters / FlowSynthesisResult 见 §6。
class IRandomSource {
public:
    virtual ~IRandomSource() = default;
    // 返回 Result 而不是无效 double，错误类型与上下文由 contracts.md 管理。
    virtual Result<Scalar, NumericalErrorCode> uniform() = 0;
    virtual Result<Scalar, NumericalErrorCode> normal() = 0;
};
class IDirectionSampler {
public:
    virtual ~IDirectionSampler() = default;
    virtual Result<DirectionSet, NumericalErrorCode> sample(int level) const = 0;
};
class IFlowNoiseModel {
public:
    virtual ~IFlowNoiseModel() = default;
    virtual Result<FlowField, NumericalErrorCode> apply(
        const FlowField&, double coefficient, IRandomSource&) const = 0;
};
~~~

### 3.1 三种具体采样器的候选类合同

~~~cpp
class OctahedralSampler final : public IDirectionSampler {
public:
    Result<DirectionSet, NumericalErrorCode> sample(int level) const override;
};
class OccludedOctahedralSampler final : public IDirectionSampler {
public:
    Result<DirectionSet, NumericalErrorCode> sample(int level) const override;
};
class IcosahedralSampler final : public IDirectionSampler {
public:
    Result<DirectionSet, NumericalErrorCode> sample(int level) const override;
};
~~~

| 类型 | 原 MATLAB 对应 | 固定输入/算法 | 输出与有效性 |
|---|---|---|---|
| OctahedralSampler | erzeugerichtungsvektoren.m + findeDreiecke.m | i ≥ 0；八面体原 8 面按顺序逐层 A/B/C/D 四分，末层面心归一 | `DirectionSet` 为 N×3，N=8·4^i；须保留逐行顺序 |
| OccludedOctahedralSampler | erzeugerichtungsvektoren_loch.m + findeDreiecke.m | 同一八面体但仅使用原 MATLAB 初始面编号 2/3/5/6/7/8 | N=6·4^i；不补回缺损面、不改变行序 |
| IcosahedralSampler | richtungsvektoren_icosahedron.m + findeDreiecke.m | 固定原 12 个顶点/20 个面及其递归顺序 | N=20·4^i；不以另一库的顶点面排列代替 |

**接口约束**：构造无需持有 RNG 或跨次状态；只返回自拥有的 N×3 方向，不自行添加噪声、距离或最终 N×6 流场。原 MATLAB 初始方向生成的 N×6 setofpoints 若包含后三列的占位值，须在 `FlowField` 转换边界保持该语义，不能把 N×3 类型误当最终 N×6。采样前验证 `level >= 0`、`base * 4^level` 不发生整数溢出及实际分配可行性。具体内存预算未批准，不引入任意固定最大 level。

**错误枚举与最小负例**：

| 场景 | 返回候选 NumericalErrorCode | 必须核对的 TC |
|---|---|---|
| level 为负 | InvalidSubdivisionLevel | TC-F-GEOM-002/003 |
| 采样数乘以 4 溢出 | SampleCountOverflow | TC-F-GEOM-002/003 |
| 请求内存无法满足 | ResourceLimitExceeded | TC-F-GEOM-001/002/003 |
| 面中心零范数、NaN/Inf/非单位方向 | ZeroNorm / InvalidDirection / NonFiniteInput | TC-F-GEOM-001/002/003 |
| FixedSequenceRandomSource 耗尽或返回非法值 | RandomSourceExhausted / InvalidRandomSample | TC-DC-RNG-001 |
| 噪声乘数非有限、光流非有限 | NonFiniteInput / NonFiniteResult | TC-F-FLOW-003、TC-DC-NOISE-001 |

上述枚举及错误到退出码的映射**统一引用 [contracts.md §3.1–3.2](contracts.md)**，不在此文件独立定义重复的 enum。随机源耗尽、无效方向或非有限输出的**当前采样/求解路径**必须立即显式传播失败并附上位置/参数，不能伪造正常值。**实验整体失败策略已由所有者确定**（参见 [experiments.md §6–8](experiments.md)）：独立样本可以记录失败并继续后续独立单位；依赖状态的 Adaptive 单步失败不提交无效主系数，无法安全恢复时终止当前连续序列；保留可审计的部分输出，Manifest 区分 Complete/Incomplete/Failed，八次聚合不改分母。**上述是已选设计而非待 P0.8/Golden 重新选型，实际 C++/CTest 失败回归尚未进行**。显式错误与原 MATLAB NaN/Inf 传播的差异须在未来证据中说明。

### 3.2 RNG 抽样合同与对象职责

`IRandomSource::uniform()` 成功输出有限 [0,1) double；`normal()` 成功输出有限 double。固定序列测试源超出已提供样本返回 RandomSourceExhausted；超范围 uniform、NaN/Inf 返回 InvalidRandomSample，**不得自动重抽或返回 0**。Runner 根据原脚本在固定运动生成后进行重新播种：由 RunContext 为下一阶段建立相同 seed 的新流，保留调用和重置轨迹。真实随机源的分布、固定序列异常及性能预算均属待批准/待执行内容。

**噪声策略区别**：MeanMagnitudeGaussianNoise(coefficient=0) 在对应 `nonpropfehler(...,0)` 的运行路径仍然消耗 3N 个 normal；作为“完全不调用噪声函数”的独立 NoNoise 策略可以不消耗样本，但**不能用它替代该 MATLAB 无噪入口**。这两个语义不同，须分别通过固定源消费计数验收。

具体对象由 Composition Root 创建。FlowField 拥有 N×6 列主序，方向只读，算法不得修改外部 MAT 文件。无随机全局单例，固定 RNG 测试替身用于确定性 L3 参考核验。

## 4. 失败、安全与验证

- 不允许负细分、N=0、样本数溢出、零模方向、非法 near、非有限噪声或异常随机源；失败信息须保留位置与参数。
- 边界测试：i=0,1,2 的方向**顺序**、具体缺损面、范数及数目，与**独立 L3 解析/独立重实现的完整有序方向参考**逐行比较；不要求运行原 MATLAB、不得由被测 C++ 输出生成参考。
- 固定人工流场、固定 uniform/normal 序列，在噪声系数 0、0.6、2.4、9.6 下验证全场平均与逐分量模型不被互换。
- 对有待解的随机结果，与原版随机试验**比较统计而非跨语言同种子逐位相同**。
- 对应 TC-F-GEOM-001/002/003、TC-F-FLOW-001/002/003/004、TC-DC-RNG-001、TC-DC-NOISE-001。独立 L3 输入/参考数据、容差、实际类和测试实体仍未冻结。


## 5. 原随机调用轨迹和可确定性测试合同

**原 Bias 入口** bias_test_nonspherical.m / bias_test_spherical_prop_error.m 的实际随机源顺序是：

~~~text
seed(corenumber)
fixedtrans = getvektor()  # 2 次 rand；theta/phi
fixedrot   = getvektor()  # 2 次 rand
seed(corenumber)          # 再次重置随机流
for each noise_coefficient and each subdivision level:
    for i=1..N: distance_i = rand(...)       # 先连续 N 次均匀分布调用
    for i=1..N:
        p_i.x += randn() * coefficient * scale_x
        p_i.y += randn() * coefficient * scale_y
        p_i.z += randn() * coefficient * scale_z
        p_i   -= dot(p_i, d_i) * d_i
~~~

这一结构表明**uniform 与 normal 的调用分成两个完整 N 循环**，不能在 C++ 中改成“为单个点依次取 distance、三个 noise”而声称随机序列等价。两个入口中不同的尺度选择应保留：
- 缺损视场：scale_x=scale_y=scale_z=全场原始光流**模长的算术平均**；
- 完整球面：scale_c=当前未加噪光流的相应分量 p_i,c（符号可以为负）。

无噪比较入口的原源码依旧调用 nonpropfehler(setofpoints,0)，因此其 randn 调用数不会因为噪声系数零而归零。测试要分别检查最终数值与 RNG 消耗次数。

### 5.1 日志/测试替身应提供的字段

| 字段 | 数学含义 | 约束 |
|---|---|---|
| seed_initial / seed_reset_count | getvektor 前初始化与固定运动之后的重置 | 来源以 Bias 入口为准，不能跨不同实验统一假设 |
| rng_uniform_calls / rng_normal_calls | uniform、normal 实际消耗次数 | 对每个原入口单独统计 |
| sample_index / subdivision_level | 源 MATLAB 1-based 索引和细分级别 | 保留递归叶顺序，不允许排序去重 |
| distance_raw / distance_normalized / near | [1,4] 距离、/2.5、倒数 | N 个独立样本，有效单位 |
| p_translation / p_rotation / p_original | 三类三维向量 | 同一方向 d_i 对应 |
| noise_scale_xyz / normal_sample_xyz | 每个分量扰动的真实因子与随机样本 | 噪声种类可追踪 |
| p_noisy_before_projection / after_projection | 未投影与投影后的三维流 | 验证切向性 |

## 6. 候选参数对象、入口/返回和数值失败

~~~cpp
// 设计合同草案，公共类型见 contracts.md。
struct BiasFlowParameters {
    std::size_t subdivision_level;
    Vector3 truth_translation;
    Vector3 truth_rotation;
    Scalar noise_coefficient;
    // 具体种子由 RunContext/IRandomSource 独立持有
};
struct FlowSynthesisResult {
    FlowField noisy;
    FlowField noiseless;
    Eigen::VectorXd nearness; // C++ 新增的逐点内部记录，不等同原 fixed_* 返回的 near 标量
    Scalar mean_translational_magnitude;
    Scalar mean_rotational_magnitude;
};
// 对应 getvektor.m：无状态函数，不制造多余的可变对象。
namespace sampling {
Result<Vector3, NumericalErrorCode> randomUnitDirection(IRandomSource& source);
}

// erzeugeflussausnear.m 的职责：纯理想流场及独立平移/旋转平均模长。
// 不采样距离、不消耗随机数、不选择 Bias 实验类型。
struct IdealFlowResult {
    FlowField flow;
    Scalar mean_translational_magnitude;
    Scalar mean_rotational_magnitude;
};
class FlowSynthesizer {
public:
    Result<IdealFlowResult, NumericalErrorCode> synthesize(
        const DirectionSet& directions,
        const Eigen::VectorXd& nearness,
        const Vector3& truth_translation,
        const Vector3& truth_rotation) const;
};
// fixed_* 的场景编排分别由 experiments.md 中同一
// BiasExperimentRunner 的 occluded/proportional 参数模式承担。
// Runner 注入 IDirectionSampler、IFlowNoiseModel、IRandomSource
// 与 FlowSynthesizer，按 §5 的原始 RNG 顺序生成 FlowSynthesisResult。
~~~

**原 `fixed_*_randomnear.m` 的输出契约与 C++ 内部记录必须区分**：在只读源 `fixed_kugel_symmetric_perfectpoints_randomnear.m` 的逐点循环（第 47–55 行）和 `fixed_propfehler_kugel_symmetric_perfectpoints_randomnear.m` 的逐点循环（第 45–53 行）中，`near=1/entf` 每次**覆盖单个标量**，而没有写 `near(i)`；因此这两个源函数的返回形参 `near` 实际代表**末次抽样近度标量**（仅在 N 大于 0 且调用完成时有定义），**不是一个 N×1 近度数组**。本 LLD 的 `FlowSynthesisResult.nearness` 是为了便于忠实模拟/独立诊断而计划提供的 **C++ N 维逐点内部值对象**：允许保留全部 N 次原抽样值，但其数学/输入行为与源码返回值**不能混同**；若单独核验旧 MATLAB 返回 `near`，必须明确使用最后一个采样值作为 `near_last` 标量并独立标记来源。不得以新增内部向量修改 RNG 的 N 次 uniform/3N 次 normal 调用时序、当前生成脚本的 MAT `save` 字段集合，亦不得把该向量宣称为作者 MAT 已有的变量。

**与对象模型的一致性**：上述 `sampling::randomUnitDirection(IRandomSource&)` 与 [OBJECT_MODEL.md §7.1](../OBJECT_MODEL.md) 保持一致；不再另设 `SphericalRandomDirection` 或 `BiasSceneGenerator`。前者不拥有 RNG；后两项场景生成的唯一编排者是 `BiasExperimentRunner`，其类的完整入口/返回合同在 [experiments.md](experiments.md)。`IdealFlowResult` 为本模块的中间值对象，最终保存结果仍用 `FlowSynthesisResult`；两者不可混淆。

**前置条件**：细分级别非负、N=base·4^level 在 size_t 范围内且实际分配受资源限制；真值有限；所需方向可归一；生成的 near 有定义。对于 N=0、非有限方向、距离无效、数值溢出应返回可识别失败类别。原脚本即便在数值边界产生 NaN/Inf，也不能偷偷替换为看似成功的零值。

**值对象的具体拥有关系（候选）**：`DirectionSet`/ `FlowField` 按 [contracts.md §2/§4](contracts.md) 自拥有 Eigen 列主序存储，`IdealFlowResult` 按值持有理想 `FlowField` 与两个 double 均值，`FlowSynthesisResult` 按值持有含噪/无噪 `FlowField`、自拥有 `Eigen::VectorXd nearness` 与对应均值；**不保存指向 matio 数据、函数局部矩阵或 RNG 内部缓存的 Eigen::Map/裸引用**。构造完成的值对象允许明确深拷贝/移动，计算中临时只读 Map 只在被映射对象生命周期内使用；是否限制大拷贝属于性能策略而非数学行为变更。FlowSynthesizer/三种方向采样器没有跨调用可变状态；`IRandomSource` 唯一可变源由 RunContext 独占，`uniform/normal` 返回的 Result 在失败时按 [contracts.md §3.0](contracts.md) 保留 code、stage（simulation）、observation_index 与已分配 RunId；不得靠 0、重新抽样或失败后复用上一次值掩盖错误。

公共 DirectionSet、FlowField 的生命周期与 const 规则由 [contracts.md](contracts.md) 管理。采样器无跨调用状态，RNG 由一次 RunContext 独占；`BiasFlowParameters` 只承载**单层级流场合成参数**，`experiments.md` 的 `BiasExperimentConfig` 独立负责实验 seed、最大细分级别、三个噪声乘数及输出。这两者是不同层次的类型，不能再重复声明同名 Config。 `IdealFlowResult` 是 FlowSynthesizer 的理想结果；`FlowSynthesisResult` 是包含近度与含噪/无噪数据的场景输出，实际组合由 BiasExperimentRunner 完成。不得把全局 rand/randn 复用到多个实验。

## 7. 独立 L3 参考数据、RNG 消耗及浮点顺序审查

| L3 参考 ID（拟议） | 依据只读源公式独立生成的参考内容（无需执行 MATLAB） | 预期检查范围 |
|---|---|---|
| SN-G01 | 八面体 i=0/1/2 的有序 N×3 方向 | 每个叶中心单位化顺序、坐标与整体 N |
| SN-G02 | 缺损八面体 i=0/1/2 | 明确排除初始面 1、4，并核对 6·4^i |
| SN-G03 | 二十面体 i=0/1/2 | 核对 12 顶点/20 面表、20·4^i 顺序 |
| SN-G04 | getvektor 的固定两次 uniform 输入 | theta/phi 与三维单位方向逐分量 |
| SN-G05 | Bias 在固定 uniform/normal 序列下的原流/噪声/投影 | N 次 rand 后 3N 次 randn，2 个不同噪声模型 |
| SN-G06 | nonpropfehler 噪声系数 0 与非零 | 即使系数 0 仍消耗 3N 个 normal 样本 |
| SN-G07 | 一般固定 near/t/ω 输入的 erzeugeflussausnear | 切向内积≈0、平移/旋转平均模长 |

每组 **L3 独立参考**须记录原只读 .m **Git blob SHA**、独立生成器代码/审查人身份及其 SHA、固定输入/注入 RNG 序列、输出 shape/数值与 SHA-256、计算次序、适用容差和退化状态；**不要求 MATLAB 运行或 L1**。初级采样通过不能替代完整实验/分布统计，以上参考尚未全部生成或测试，DEC-005 容差未批准。

**模块状态**：采样/噪声的算法和 RNG 调用次序已有可审查合同；实现、实际 C++ 签名、独立 L3 逐值参考的验证、CTest 和性能预算均未完成。

### 7.1 第一轮有序采样/固定 RNG 的可执行前置用例（AF-047，设计补齐）

**只规定输入、应观察的顺序与来源，不生成独立 L3 数值或运行模拟。** 将来固定参考必须与原只读 .m 的面/子面顺序逐行比较，不能只核点数与无序集合。

| 固定输入 | 离散预期/核对对象 | 当前证据缺口 |
|---|---|---|
| 完整八面体 `level=0/1/2` | 行数依次 `8/32/128`，各层完整**行序/方向**另由独立 L3 提供 | 有数量公式，缺 `i=1/2` 全量有序参考 |
| 缺损八面体 `level=0/1/2` | 行数依次 `6/24/96`；必须保留原取面顺序，不能先生成完整球面再按排序结果删除 | 缺每层完整逐行独立 L3 |
| `getvektor()` 注入两次 `uniform` | 单次调用确切消耗 **2 次** `uniform`；应记录调用顺序、方向/归一化结果 | 还未通过 C++ 固定源替身/穷尽负例验证 |
| Bias 固定 `N`、提供可辨认的 `uniform` 和 `normal` 序列 | 固定平移/旋转方向先消耗两组 `getvektor()`，重新设种后先连续 **N 次 uniform** 产生距离，再连续 **3N 次 normal** 产生各点噪声；不可逐点交错 | 需按入口独立比较抽样 trace 与样本位置，不要求 MATLAB 同 seed 相同随机数 |
| 噪声倍数为 `0`、正常源用尽、返回 NaN/Inf | 零倍数仍执行源 `normal` 调用，不能优化跳过；耗尽/非有限值显式报错并保留 sample_index/来源 | 无实际异常/CTest 证据；非成功样本不擅改统计分母 |

**原始返回差异**：`fixed_*_randomnear.m` 返回的 `near` 为逐点循环最后写入的 **标量**；C++ 候选 `FlowSynthesisResult.nearness` 的 N 维向量是新内部记录，不能伪称为原 MATLAB 返回向量。按 `TC-F-GEOM-002/003`、`TC-NF-RNG-001`、`TC-DC-RNG-001`、`TC-F-FLOW-001～004` 关联现有 TEST_PLAN，不新增 TC。**AF-047 仍「未整改」**，独立有序 L3、实现及 CTest 待授权。

#### 7.1.1 AN-G01～03 零级面顺序的符号核对（AF-047/055，非数值 L3）

已根据原始三份采样器与 `findeDreiecke.m` 的静态公式，按初始面表逐行核对 `AN-G01` 8 行、`AN-G02` 6 行、`AN-G03` 20 行的 **level=0 符号方向与次序**。推导和完整 20 行顶点索引见 [专项静态说明](../reviews/AN_G_LEVEL0_FACE_ORDER_STATIC_2026-11-08.md)。源 `findeDreiecke` 的 `A/B/C/D` 递归到 **level=1/2** 的全部行、独立数值 L3、浮点误差/异常以及 C++/CTest 均未核验；这只是对旧候选 JSON 初始有序面表的静态证据，不将任何 TC/AC 标为 PASS。AF-047/055 保持「未整改」。

#### 7.1.2 递归路径与最终行号的精确静态索引（AF-081）

原始 [`findeDreiecke.m` 的独立静态行号审计](../reviews/AN_G_RECURSION_INDEX_STATIC_2026-11-08.md) 明确：每个非终止三角面先按 `xy=normalize((x+y)/2)`、`xz=normalize((x+z)/2)`、`yz=normalize((y+z)/2)` 构造中点，再**顺序递归** `A=(x,xz,xy)`、`B=(y,xy,yz)`、`C=(z,xz,yz)`、`D=(xy,xz,yz)`；终止面返回 `normalize((x+y+z)/3)`。原函数也在终止层先计算这些中点，但不使用它们构造返回面心；不能把该额外中间运算误称为另一层细分或已验证的退化异常。

- 原面排列按完整八面体 `1…8`、缺损八面体 `2,3,5,6,7,8`、二十面体源 `F(1,:)…F(20,:)`。**缺损的原始面 ID 与输出块 ID 不相同**：缺损首块是原面 2，第二块是原面 3，依此类推。
- 深度 `L` 的每个原面输出 `4^L` 个相邻行。令**输出块序号** `b=1…B`（`B∈{8,6,20}`）及沿根到叶的子面数字 `d_j∈{0,1,2,3}` 分别表示 `A/B/C/D`，则最后结果的 **MATLAB 1-based 行号**为

  `row(b,d_1,…,d_L) = 1 + (b−1)·4^L + Σ_{j=1}^L d_j·4^(L−j)`。

  `L=0` 时求和为空、每块仅一行；`L=1` 每块 `A,B,C,D`；`L=2` 每块 `AA,AB,AC,AD,BA,BB,BC,BD,CA,CB,CC,CD,DA,DB,DC,DD`，**深度优先而非按所有面广度优先**。
- `L=1` 行数为完整/缺损/二十面体 `32/24/80`；`L=2` 为 `128/96/320`。原面块 `b` 的 1-based 行范围是 `[(b−1)·4^L+1, b·4^L]`。所有行的前三列是终止面心单位向量，原 `setofpoints` 的后三列初始化为零；不得通过全局排序、点集相等或对称性代替逐行校验。

**验证界限**：本节是原源码的**递归路径/索引合同静态推导**，没有实际计算 `level=1/2` 的全部数值、没有生成独立 L3、没有运行 MATLAB/C++/CTest。未来须提供有来源身份的每行三维参考、浮点误差/非法输入检查。AF-081 可按文本合同独立复核；AF-047/055 继续「未整改」。

### 7.2 原始噪声系数正负与 RNG 抽样数（AF-047，静态合同）

原 `nonpropfehler.m` 先用无噪声流场计算平均模长 `durchlaenge`，随后对每个 N×6 行，依次进行 x/y/z 三个 `randn * fehlerprozent * durchlaenge`，最后应用 `p - (p·d)d` 切向投影；不存在按 `fehlerprozent==0` 跳过抽样的分支。故：

- 零噪声尺度仍为 **3N 次 normal 抽样**，只令加到 p 的值为零；不得据“输出无噪声”推断未消耗 RNG。
- 负的有限尺度**在源码中没有显式拒绝**，固定注入相同的法向样本时，噪声项符号反转；其概率分布可能相同，**固定序列样本值不相同**。这与外部 CLI 是否最终允许负系数的 DEC-008 决策须分别标明。
- 两种 Bias 原 `fixed_*randomnear.m` 均先有 N 次 uniform 近度，再各有 3N 次 normal；一个版本乘平均流模长，另一个乘逐分量原始 p。两者不得在复用函数时互换。

这仅是对只读源码的调用顺序审查，不是注入随机源实际回归；相关现有 TC 为 TC-NF-RNG-001、TC-F-FLOW-003/004、TC-F-BIAS-001/002，独立 L3 和 CTest 未完成。

## 8. CMake 归属、头文件与 TC/CTest 候选（无实际实现）

按 [ARCHITECTURE.md §4–4.1](../ARCHITECTURE.md) 现有的目标依赖边界：

| 拟议 target | 本文职责 | 依赖与禁止事项 |
|---|---|---|
| `egomotion_sampling` | 三个有序采样器、`sampling::randomUnitDirection(IRandomSource&)` 的数学/无状态实现 | `egomotion_domain` + `egomotion_contracts`（随机接口与采样抽象）；不依赖 Runner、MAT I/O 或绘图 |
| `egomotion_simulation` | `FlowSynthesizer`、理想/加噪流场、噪声模型与 `IdealFlowResult/FlowSynthesisResult` | `egomotion_domain`、`egomotion_sampling`、`egomotion_contracts`；不把 matio 或实验输出管理引入数学层 |
| `egomotion_experiments` | BiasExperimentRunner 的种子阶段、场景/噪声组合与 RunContext/输出编排 | 只调用以上窄接口；不重复数值公式，不向 sampling/simulation 形成反向依赖 |

公共头文件候选放在 `include/egomotion/sampling/` 和 `include/egomotion/simulation/`，实现可放 `src/sampling/`、`src/simulation/`；共享 `DirectionSet/FlowField` 和 `Result/Failure` 由其唯一公共合同提供，不在两处重新声明 enum 或 ABI。**头文件路径/接口、CMake target 链接与错误类型最终由 P0.8 冻结；目前并未编译或验证**。

| TC / AC | TEST_PLAN §3 的拟议测试文件/CTest | 独立 L3 参考内容 |
|---|---|---|
| TC-F-GEOM-001/002/003 / 对应 AC | `tests/unit/test_sampling.cpp` / `egomotion.geometry.*` | i=0/1/2 三类采样的 N、全量有序方向、面/叶顺序、范数及非法 level/资源 |
| TC-F-FLOW-001/002/003/004 / 对应 AC | `tests/unit/test_flow_simulation.cpp` / `egomotion.flow.*` | 切向流公式、近度、两个噪声模型、固定 normal 顺序、0 系数仍消耗、边界/不合法输入 |
| TC-DC-RNG-001 / AC-DC-RNG-001 | TEST_PLAN 当前是**人工合同评审**；拟在上述两个测试文件增添固定随机替身检查，CTest 归属待 DEC-008 | uniform/normal 值、消耗计数、耗尽/无效输入、RunId 隔离 |
| TC-DC-NOISE-001 / AC-DC-NOISE-001 | TEST_PLAN 当前是**人工合同评审**；拟在 `test_flow_simulation.cpp` 中补充噪声替身检查，CTest 归属待 DEC-008 | 模型不互换、非有限输入的 NumericalErrorCode/上下文与统计分母 |

本节对 DC-RNG/NOISE 仅提议**追加测试实现位置**，不新增、替换或宣称已通过任何 TC/AC。全部实例、L3 参考、CTest 注册/运行与资源预算仍缺。原 MATLAB 为静态依据，不执行 MATLAB/Octave。

## 附录 A：本模块无需运行 MATLAB 的验收参考

按 [长期无需 MATLAB 执行政策](../golden/NO_MATLAB_EXECUTION_POLICY.md) 与 [L2/L3/L4 来源矩阵](../golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md)：原 MATLAB .m 文件仅作为**只读公式、数据布局和顺序规范**；**不要求执行 MATLAB/Octave、采集 L1 或以缺少 L1 阻塞冻结/编码/验收**。

现行采样验收使用原只读源码定义的面表/递归与独立 L3 全量有序方向（i=0,1,2），固定注入 uniform/normal 样本及消费次数。原 MATLAB 的 seed→具体浮点抽样序列不作为等价门槛；不要声称已实际验证其 RNG 流。

本附录不取消真实 C++/CTest、Golden 独立复核、DEC-005/008 以及源数据保护门禁；当前无新的软件测试通过证据。
