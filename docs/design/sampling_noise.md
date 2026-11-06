# 视线采样、光流合成与噪声——详细设计草案

> **LLD 草案，未冻结、无实现/Golden 数值。** 对应 P2/P3、REQ-F-GEOM-*、REQ-F-FLOW-*、REQ-DC-RNG-001、REQ-DC-NOISE-001。

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

~~~cpp
// 文档接口草案，不是实现。
struct DirectionSet; struct FlowField;
struct BiasSimulationInput;
struct BiasSimulationOutput;
class IRandomSource {
public:
    virtual ~IRandomSource() = default;
    virtual double uniform() = 0;
    virtual double normal() = 0;
};
class IDirectionSampler {
public:
    virtual ~IDirectionSampler() = default;
    virtual Result<DirectionSet, SamplingError> sample(int level) const = 0;
};
class IFlowNoiseModel {
public:
    virtual ~IFlowNoiseModel() = default;
    virtual Result<FlowField, NoiseError> apply(
        const FlowField&, double coefficient, IRandomSource&) const = 0;
};
~~~

存在三种采样器、两种原加噪模型及 NoNoise；具体对象由 Composition Root 创建。FlowField 拥有 N×6 列主序，方向只读，算法不得修改外部 MAT 文件。无随机全局单例，固定 RNG 测试替身用于确定性 Golden。

## 4. 失败、安全与验证

- 不允许负细分、N=0、样本数溢出、零模方向、非法 near、非有限噪声或异常随机源；失败信息须保留位置与参数。
- 边界测试：i=0,1,2 的方向**顺序**、具体缺损面、范数及数目，与原 MATLAB 黄金数组逐行比较。
- 固定人工流场、固定 uniform/normal 序列，在噪声系数 0、0.6、2.4、9.6 下验证全场平均与逐分量模型不被互换。
- 对有待解的随机结果，与原版随机试验**比较统计而非跨语言同种子逐位相同**。
- 对应 TC-F-GEOM-001/002/003、TC-F-FLOW-001/002/003/004、TC-DC-RNG-001、TC-DC-NOISE-001。输入/Golden、容差、实际类和测试路径未冻结。


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
    Eigen::VectorXd nearness;
    Scalar mean_translational_magnitude;
    Scalar mean_rotational_magnitude;
};
class BiasSceneGenerator {
public:
    Result<FlowSynthesisResult, NumericalErrorCode> makeOccluded(
        const BiasFlowParameters&, IRandomSource&) const;
    Result<FlowSynthesisResult, NumericalErrorCode> makeProportional(
        const BiasFlowParameters&, IRandomSource&) const;
};
~~~

**前置条件**：细分级别非负、N=base·4^level 在 size_t 范围内且实际分配受资源限制；真值有限；所需方向可归一；生成的 near 有定义。对于 N=0、非有限方向、距离无效、数值溢出应返回可识别失败类别。原脚本即便在数值边界产生 NaN/Inf，也不能偷偷替换为看似成功的零值。

公共 DirectionSet、FlowField 的生命周期和 const 规则由 [contracts.md](contracts.md) 管理；采样器可能无状态，RNG 必须是一次运行私有状态，不得把全局 rand/randn 偷偷复用到其他实验。

## 7. 逐层 Golden 和浮点顺序审查

| Golden ID（拟议） | 对照 MATLAB 的内容 | 预期检查范围 |
|---|---|---|
| SN-G01 | 八面体 i=0/1/2 的有序 N×3 方向 | 每个叶中心单位化顺序、坐标与整体 N |
| SN-G02 | 缺损八面体 i=0/1/2 | 明确排除初始面 1、4，并核对 6·4^i |
| SN-G03 | 二十面体 i=0/1/2 | 核对 12 顶点/20 面表、20·4^i 顺序 |
| SN-G04 | getvektor 的固定两次 uniform 输入 | theta/phi 与三维单位方向逐分量 |
| SN-G05 | Bias 在固定 uniform/normal 序列下的原流/噪声/投影 | N 次 rand 后 3N 次 randn，2 个不同噪声模型 |
| SN-G06 | nonpropfehler 噪声系数 0 与非零 | 即使系数 0 仍消耗 3N 个 normal 样本 |
| SN-G07 | 一般固定 near/t/ω 输入的 erzeugeflussausnear | 切向内积≈0、平移/旋转平均模长 |

每个 Golden 应记录原 MATLAB blob SHA、固定输入序列、输出 shape、计算顺序、被测容差及是否退化；抽样初步通过不能代替最终批量实验。**目前没有以上 Golden 数值与批准容差。**

**模块状态**：采样/噪声的算法和 RNG 调用次序已有可审查合同；实现、实际 C++ 签名、数值 Golden、CTest 和性能预算均未完成。
