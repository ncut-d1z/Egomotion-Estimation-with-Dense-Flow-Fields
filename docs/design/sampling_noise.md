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
