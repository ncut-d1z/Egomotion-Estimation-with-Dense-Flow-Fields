# Franz / Original KvD / Modified KvD 详细设计草案

> **LLD v0.8（未冻结、无生产实现）**。原 MATLAB 的Franz/Original/Modified KvD公式和赋值次序已核对；§9.4/§9.5的固定方向夹具已用Wolfram精确算术分别验证**两组六方向4条×6轮、一次8方向翻号2条×6轮，共36次数学迭代**。其中精确轨迹覆盖near正均值、Original/Modified近度加权差异、首轮near翻号、RMS/delta/动态门限及最低六次停止；但不覆盖全部一般运动、200轮极端、生产C++/CTest与最终DEC-005。

## 1. 精确来源与函数合同

| MATLAB 文件 | 输入 | 输出 | 原版重要行为 |
|---|---|---|---|
| help_functions/zellendefinieren.m | 无 | 3×10 模板表 | 三个单位笛卡尔轴；响应与范数累积位初始为零 |
| help_functions/franzeigen.m | N×6 setofpoints | trans (3×1)、rot (3×1) | 平移、旋转分别用三个轴模板；按模板响应/范数归一 |
| help_functions/norm_koenderink_iteration.m | N×6 setofpoints | trans、rot、near (N×1)、x 次数 | 原始 KvD，平移更新使用 near 及 near² |
| help_functions/mue_norm_koenderink_iteration.m | N×6 setofpoints | trans、rot、near、x | 修正 KvD，平移更新取消相应额外 near 因子 |
| help_functions/transfehler_div.m | truth、estimated 3D 向量 | 向量差模长 | 先把估计向量模长缩放至真值模长 |
| help_functions/normrelfehler.m | truth、estimated 3D 向量 | 方向夹角（度） | 先归一化再 acos；零向量/越界需单独报告 |

定义：第 i 个样本的单位视线为 \(\boldsymbol d_i\)，切向光流为 \(\boldsymbol p_i\)，近度为 \(\mu_i\)，平移为 \(\boldsymbol t\)，旋转向量为 \(\boldsymbol\omega\)；\(\langle x\rangle=N^{-1}\sum_i x_i\)。

## 2. Franz 基线的三轴模板

对于三个笛卡尔轴 \(\boldsymbol e_j\)（j=1,2,3），原 MATLAB 对每个光流样本累积：

\[
\boldsymbol q^t_{ij}=-(\boldsymbol e_j-(\boldsymbol e_j^\top\boldsymbol d_i)\boldsymbol d_i),
\qquad
\boldsymbol q^\omega_{ij}=-(\boldsymbol e_j\times\boldsymbol d_i).
\]

\[
s^t_j=\sum_i(\boldsymbol q^t_{ij})^\top\boldsymbol p_i,\qquad
n^t_j=\sum_i\|\boldsymbol q^t_{ij}\|^2,
\quad
s^\omega_j=\sum_i(\boldsymbol q^\omega_{ij})^\top\boldsymbol p_i,\qquad
n^\omega_j=\sum_i\|\boldsymbol q^\omega_{ij}\|^2.
\]

\[
\boldsymbol t_{\rm Franz}
=\frac{3}{3}\sum_{j=1}^{3}\boldsymbol e_j\frac{s^t_j}{n^t_j},
\qquad
\boldsymbol\omega_{\rm Franz}
=\frac{3}{3}\sum_{j=1}^{3}\boldsymbol e_j\frac{s^\omega_j}{n^\omega_j}.
\]

式中的 3/3 是现有 zellendefinieren（3 个模板）的原始代码因子，不能擅自把它泛化为不相等的响应权重。某个 n_j=0 时原版直接除法；C++ 应输出明确数值失败状态，同时记录与原版的行为差异，不能伪造零响应。

## 3. KvD 两条路径的逐步公式与更新次序

原始两函数共用的初始流程：

1. 调用 Franz 基线求得初始 t、ω，然后 \( \boldsymbol t\gets\boldsymbol t/\|\boldsymbol t\| \)。
2. 常量（核对自源文件）：**nulldiv=1e−13、genauigkeit=1000、初始 grenze=1e−4、absgrenze=1e−7、最多 200 次、至少 6 次**。
3. 迭代开始时，依据当前 t、ω 按如下公式重估 near：

\[
\mu_i=
-\frac{\boldsymbol t^\top\left(\boldsymbol p_i-\boldsymbol d_i\times\boldsymbol\omega\right)}
{1-(\boldsymbol t^\top\boldsymbol d_i)^2+10^{-13}}.
\]

若 \(\sum_i\mu_i<0\)，同时将全部 near 和 t 乘以 −1。随后计算 **下一轮 t**，再计算 **本轮 ω**；ω 的更新使用旧 t，之后才能提交新 t。

### 3.1 Original KvD：含 near² 项的平移更新

\[
\boldsymbol a=\langle\mu_i\boldsymbol p_i\rangle,\quad
\boldsymbol b=\langle\mu_i\boldsymbol d_i\rangle,\quad
\boldsymbol c=\langle\mu_i^2(\boldsymbol t^\top\boldsymbol d_i)\boldsymbol d_i\rangle,
\]
\[
\widetilde{\boldsymbol t}_{new}
=-(\boldsymbol a+\boldsymbol\omega\times\boldsymbol b-\boldsymbol c),\qquad
\boldsymbol t_{new}=\widetilde{\boldsymbol t}_{new}/\|\widetilde{\boldsymbol t}_{new}\|.
\]

### 3.2 Modified KvD：取消额外 near 的平移更新

\[
\boldsymbol a=\langle\boldsymbol p_i\rangle,\quad
\boldsymbol b=\langle\boldsymbol d_i\rangle,\quad
\boldsymbol c=\langle\mu_i(\boldsymbol t^\top\boldsymbol d_i)\boldsymbol d_i\rangle,
\]
\[
\widetilde{\boldsymbol t}_{new}
=-(\boldsymbol a+\boldsymbol\omega\times\boldsymbol b-\boldsymbol c),\qquad
\boldsymbol t_{new}=\widetilde{\boldsymbol t}_{new}/\|\widetilde{\boldsymbol t}_{new}\|.
\]

这两个公式不能靠未注明数学含义的 bool 模式切换；候选设计使用明确的 Original/Modified 类型或策略。

### 3.3 共享旋转更新、残差和终止

\[
\boldsymbol a_\omega=\langle \boldsymbol p_i\times\boldsymbol d_i\rangle,\quad
\boldsymbol b_\omega=\langle\mu_i\boldsymbol d_i\rangle,\quad
\boldsymbol c_\omega=\langle(\boldsymbol\omega^\top\boldsymbol d_i)\boldsymbol d_i\rangle,
\]
\[
\boldsymbol\omega_{new}
=\boldsymbol a_\omega+\boldsymbol t_{\rm old}\times\boldsymbol b_\omega+\boldsymbol c_\omega.
\]

保存 ω_new 之后才将 t←t_new。预测流与 RMS：

\[
\widehat{\boldsymbol p}_i=-\mu_i
(\boldsymbol t-(\boldsymbol t^\top\boldsymbol d_i)\boldsymbol d_i)
-\boldsymbol\omega\times\boldsymbol d_i,\quad
r=\sqrt{\langle\|\boldsymbol p_i-\widehat{\boldsymbol p}_i\|^2\rangle}.
\]

源码每轮计算 delta=abs(previous_RMS−current_RMS)，令动态门限 grenze=r/(1000N)。循环条件**按原始布尔优先级**为：

\[
((\Delta>grenze)\land(k<200)\land(\Delta>10^{-7}))\lor(k<6).
\]

原版在循环前将 grenze=1e−4、delta=10*grenze；循环后将最终 t 乘以最终 near 均值（near 是**最后一次循环早期计算**的值，并非必然针对最终 t、ω 再求一次）。必须记录 k 的计数起点、首次/末次误差与动态门限，验证 6 次最低循环和 200 次常规上限的行为。

### 3.4 迭代伪代码（非 C++ 实现）

~~~text
(t, omega) = Franz(flow)
t = normalize(t)
delta = 1e-3; threshold = 1e-4; absolute = 1e-7
k = 0; previous_rms = 0; current_rms = 0
while ((delta > threshold && k < 200 && delta > absolute) || k < 6):
    k += 1
    near = estimate_nearness(flow, t, omega, epsilon=1e-13)
    if sum(near) < 0: near = -near; t = -t
    t_next = translation_rule_original_or_modified(flow, near, t, omega)
    omega_next = mean_cross_p_d + cross(t_old, mean_near_d) + mean_rot_dot_d_d
    omega = omega_next
    t = normalize(t_next)
    current_rms = rms(flow - predict_flow(near, t, omega))
    delta = abs(previous_rms - current_rms)
    previous_rms = current_rms
    threshold = current_rms / (1000 * N)
return (mean(near) * t, omega, near, k)
~~~

此伪代码用于实现步骤核对；和原 MATLAB 精确的变量覆盖顺序应在 P4 Golden 中核验，如发现不同，以原始源码为准并修订本草案。

## 4. 候选 C++17 接口合同（尚未批准）

**唯一接口候选定义在本文件 §7**，公共 Result<T,E> / NumericalErrorCode 以 [contracts.md](contracts.md) 为准。此处不重复声明 KvDParameters、KvDResult 和估计器类型，以避免签名矛盾。

**前置条件候选**：N>0；每个 d 可归一化且流场列数为 6；数据有限；合法源精度。**后置条件**：所有返回的 t、ω、near、残差及迭代次数有明确状态；无效输入及零范数/NaN/Inf 不返回伪造的成功值。错误类别建议 InvalidShape、ZeroNorm、SingularGeometry、NonFinite，需与 MatError 分离并由 P0.8 正式定义。

## 5. 逐轮 Golden 与验收字段

| 层级 | 必须保存的字段 | 来源/冻结状态 |
|---|---|---|
| Franz | 每轴平移/旋转模板响应、norm 分母、初始 t 和 ω | zellendefinieren / franzeigen；参考数值未生成 |
| near | t_old、ω_old、原始 near、符号翻转、分母 | 两种 KvD 原函数；参考数值未生成 |
| KvD 平移 | 原始/修正 a、b、c、t_next、归一化范数 | 两种平移规则；参考数值未生成 |
| KvD 旋转 | a_ω、b_ω、c_ω、ω_new、使用旧 t 的时刻 | 两种 KvD；参考数值未生成 |
| 退出判定 | k、RMS、previous_RMS、delta、动态 grenze、absgrenze、最终 t 尺度 | 原 while 表达式；参考数值未生成 |
| 指标 | transfehler_div 的尺度对齐结果、normrelfehler 的角度（度） | 独立计算，不可互换 |

每个 TC 必须记录源码 SHA、固定输入来源、绝对/相对最大误差、被测矩阵/向量维度及异常状态。**历史 `rtol=1e-10` 不再是现行候选；项目所有者已确定 `atol=0.01`、`rtol=0.1` 仅用于首轮宽松探索性筛查，不能作为最终 TC/AC PASS 门槛。DEC-005 的最终逐模块阈值及严格几何/状态/迭代次数判据仍须以真实 L3/CTest 证据确认。** 不能仅凭整体论文图形相似就声称逐轮算法等价。

## 6. 未决与冻结门禁

- 独立 L3 逐轮数值参考尚未完整产生/复核；仅有公式/行为的静态审计和解析候选。**不再等待或要求 MATLAB 运行 L1**。
- 对原始 MATLAB 数值失败和新增显式错误返回的差异需列入合同，不能以“修复”之名改变忠实输出。
- **已选**轻量 `Result/Status` 封装及显式失败传播（见 [contracts.md](contracts.md)）；仍待 P0.8/P4 结合真实头文件/编译核对**具体签名、trace 选择及实现是否忠实保留原公式与迭代次序**。首轮筛查值已选，但 DEC-005 最终数值阈值、CTest 注册及数值验收仍未完成。
- 需求/测试：REQ-F-KVD-001/002、TC-F-KVD-001/002、TC-F-ERROR-001、TC-DC-KVD-001、TC-NF-NUM-001；详细执行计划见 [TEST_PLAN.md](../TEST_PLAN.md)。

**本文件是更具体的 LLD 草案，不是可冻结的编码基线。**


## 7. 统一类型与精确签名候选

[contracts.md](contracts.md) 为值对象与 Result<T,E> 的单一候选定义；本节只定义 KvD 额外参数和逐轮记录，避免不同求解器私建互斥错误类型。

~~~cpp
namespace egomotion {
struct KvDParameters {
    Scalar nearness_denominator_epsilon = 1e-13;
    Scalar initial_threshold = 1e-4;
    Scalar absolute_threshold = 1e-7;
    Scalar accuracy_factor = 1000.0;
    std::size_t minimum_iterations = 6;
    std::size_t nominal_maximum_iterations = 200;
};
// Franz 初始化的每轴原始响应和分母必须单独观察，不能只存最终运动。
struct FranzInitializationTrace {
    Vector3 translation_responses;
    Vector3 translation_denominators;
    Vector3 rotation_responses;
    Vector3 rotation_denominators;
    MotionEstimate before_translation_normalization;
    Scalar initial_translation_norm;
};
struct KvDIterationTrace {
    std::size_t k;  // MATLAB 循环内 x：1-based；不是 C++ 容器下标
    Vector3 translation_before_sign;
    Vector3 translation_after_sign;  // 此状态是旋转更新使用的 old t
    Vector3 rotation_before;
    Eigen::VectorXd nearness_denominators; // N 项，含 1e-13
    Eigen::VectorXd nearness_raw;
    Scalar nearness_sum_raw;
    bool sign_flipped;  // 源码严格 sum < 0
    Eigen::VectorXd nearness_after_sign;
    Vector3 translation_sum_a;
    Vector3 translation_sum_b;
    Vector3 translation_sum_c;
    Vector3 translation_candidate_raw; // 归一化前的 -(a+omega×b-c)
    Scalar translation_candidate_norm;
    Vector3 translation_candidate_normalized;
    Vector3 rotation_sum_a;
    Vector3 rotation_sum_b;
    Vector3 rotation_sum_c;
    Vector3 rotation_after; // 必须用 translation_after_sign 计算
    Vector3 translation_after_commit; // 旋转更新完成后才提交
    Eigen::MatrixXd predicted_flow_xyz; // N×3，逐点 pstrich，仅 Golden trace
    Scalar residual_mse;
    Scalar residual_rms;
    Scalar previous_rms;
    Scalar residual_change;
    Scalar active_threshold; // 本轮 residual_rms/(1000*N)
    bool continue_after_iteration; // 精确复算源 while 的 (A&&B&&C)||k<6
};
struct KvDResult {
    MotionEstimate motion;  // translation 已乘 mean(final_nearness)
    Eigen::VectorXd final_nearness; // 最后一轮开头生成，不重新估计
    Scalar final_nearness_mean;
    Vector3 final_translation_direction_before_scale;
    std::size_t iteration_count;
    FranzInitializationTrace initial_franz; // Golden 逐轴诊断候选
    std::vector<KvDIterationTrace> trace; // 可按设置输出；Golden 模式必须可见
};
class FranzMatchedFilter {
public:
    Result<MotionEstimate, NumericalErrorCode> estimate(
        const FlowField&) const;
};
class OriginalKvDEstimator {
public:
    Result<KvDResult, NumericalErrorCode> estimate(
        const FlowField&, const KvDParameters&) const;
};
class ModifiedKvDEstimator {
public:
    Result<KvDResult, NumericalErrorCode> estimate(
        const FlowField&, const KvDParameters&) const;
};
}
~~~

**AF-087 逐轮证据字段与原源码对应（仅未冻结候选）**：原 `norm_koenderink_iteration.m:46–60,62–112,116–146` 和 `mue_norm_koenderink_iteration.m:43–57,59–109,113–143` 每轮有如下顺序。`nearness_denominators` 使用翻号前 t 的平方项（翻号不改变平方），`nearness_sum_raw/sign_flipped` 在求 a/b/c **之前**确定；`translation_candidate_raw/norm/normalized` 是源码先归一的新候选 t，但 `rotation_sum_a/b/c` 和 `rotation_after` 必须使用 `translation_after_sign`，**随后**才记录 `translation_after_commit`。残差预测 `predicted_flow_xyz` 使用的是**更新后的 t/ω 与本轮旧 near**，不能换成下一轮重算 near。记录 `previous_rms`、本轮 MSE/RMS、delta、动态门限、`k` 和 `continue_after_iteration`，方可逐轮验证最低 6 次和常规 200 次上限及 `||` 优先级。

`FranzInitializationTrace` 单独保存三轴的响应和归一分母，以及开始 KvD 之前归一 t 的模；终局 `final_nearness_mean` 与归一 t 和 `motion` 的比例应可独立核验。**这些是逐轮/逐轴诊断字段的拟议存储 schema，不是改动求解数学或宣称算法性能模式必须长期保存 N×3 数据**；性能模式 trace 能否关闭、溢出/非有限状态的诊断以及内存限制仍由 P0.8/DEC-008 和真实测试决定。由于原 .m 对病态数值没有稳定错误码，C++ 的非成功路径应另外保存上次有效节点与错误来源，不应假造完整成功 trace。

**合同待定项**：NumericalErrorCode 单独 enum 是否足以携带上下文、Result 如何表达 NaN/Inf 的原版行为、trace 在性能模式是否可关闭，以及是否允许方法接收非单位视线。须依 P0.8 与经独立复核的 L3 Golden 冻结，不能仅凭该示例签名开始实现。

## 8. 每轮状态、停止判据与异常路径表

| 顺序 | 输入/可变状态 | 必须留下的证据 | MATLAB 对应关键代码 |
|---|---|---|---|
| I0 | 固定 N×6 流场 | Franz 的每轴积分、归一分母、t0、ω0 | franzeigen 的初始模板 |
| I1 | t_before、ω_before | near_raw、分母是否接近 0 | norm_koenderink 的 near 循环 |
| I2 | near_raw 与 t_before | sum(near_raw)、sign_flip、near_after、t_after_sign | near 和 t 同时翻转 |
| I3 | near_after、t_old、ω_old | Original/Modified **分别**记录 a、b、c、t_candidate 与范数 | 两个不同的平移分支 |
| I4 | t_old、near、ω_old、flow | a_omega、b_omega、c_omega、ω_new、所用旧 t 的副本 | 旋转更新必须先于 t_new 提交 |
| I5 | t_new、ω_new、**本轮 near** | p_pred(i)、residual_mse、residual_rms | MATLAB 求平均平方再取根 |
| I6 | previous_rms、current_rms | delta、threshold=rms/(1000N)、absolute_threshold、k 与 bool 判定 | 原 while 的 OR/AND 优先级 |
| I7 | final t、final near | mean(final_near) 和最终 scale-adjusted t、x 次数 | 循环后缩放，不擅自重算 near |

**特别检查**：原函数计算 near 时用的是该迭代开始的 t/ω；循环末尾不会再次计算 near。若重构为“先更新最终状态，再统一重算 near”，会改变输出，不能作为忠实模式的实现。

候选失败集合：
- 输入 N=0、非 6 列、非有限 d/p、非单位 d（默认拒绝还是校正待约定）；
- Franz 各模板归一分母为 0；初始或后续 t 范数为 0；near 分母极小；输出出现 NaN/Inf；
- 达到原最大预算：**不等于出错**，必须记录终止原因及最后有效性，而非统一当作失败；
- 原源码对部分非有限值没有显式拒绝，**可能**传播 NaN；C++ 显式返回 NumericalErrorCode 时记录静态可证公式分歧及来源限制，**不要求运行 MATLAB 验证警告或具体传播**，也不宣称原版异常逐值等价。

## 9. 固定 Golden 的分层生成与测试数据格式

建议 Golden 以**独立 L3 参考推导/非被测程序输出**形式存为可审查数据；只读原 MATLAB 源码作公式依据，**不执行原 MATLAB**。不得把被测 C++ 结果拷贝成 Expected。每个测试案例至少保存：

| 样本 ID（计划） | 条件 | 独立 L3 参考及异常合同 | 目标 |
|---|---|---|---|
| KVD-G01 | i=0 小采样、无噪声/固定 near、非退化 t/ω | Franz 中间响应、每轮 near、Original/Modified t/ω、残差、停止 | 符号和更新顺序 |
| KVD-G02 | 符号翻转（sum near<0） | near_before/after、t_before/after 与最终 scale | 保留符号分支 |
| KVD-G03 | 非零固定 Gaussian、近似噪声输入 | 两个估计器逐轮误差和次数 | 对照统计数据与容差 |
| KVD-G04 | 人工零模或几何退化输入 | 静态标注原源可能传播非有限的分支；独立输入与 C++ 显式错误分类 | 明确可验证异常状态，不冒充原 MATLAB 运行行为 |
| KVD-G05 | 能区分最低 6 次、提前停止和 200 次上限的输入 | k、每轮 delta/threshold/残差 | 验证完整 while 语义 |

黄金记录字段应有源 Git blob SHA、原始函数名、独立参考生成器/推导来源、固定输入身份、输出每个字段的 shape/dtype/数值、允许误差和对应 TC-F-FILTER-001、TC-F-KVD-001/002、TC-F-ERROR-001。**上述 G01–G05 只是测试设计编号，当前没有数据文件或运行证据。**

### 9.1 源码逐段核对与可判别的数学性质（AF-043 第一轮）

**已对照的只读来源**：`origin/.../help_functions/franzeigen.m`、`norm_koenderink_iteration.m`、`mue_norm_koenderink_iteration.m`。下表是**源码和代数的静态核对**，不是新增 L3 逐轮数值结果。

| 源函数位置（MATLAB 1-based 行号） | 必须区分的行为 | 后续 L3 最小核验点 |
|---|---|---|
| `franzeigen.m:13–35` | 每轴累积平移/旋转模板响应与各自平方范数分母，最后按 `3/anzahlzellen` 缩放 | 三轴各自的响应、分母、Franz `t₀/ω₀`；分母为零时记录显式失败 |
| `norm_koenderink_iteration.m:46–85` | near 估计、总和负时 near/t 同翻；Original 平移 `a=⟨μp⟩, b=⟨μd⟩, c=⟨μ²(t·d)d⟩` | 分别保留 `a/b/c` 及翻转前后的 t/near |
| `mue_norm_koenderink_iteration.m:43–82` | Modified 平移 `a=⟨p⟩, b=⟨d⟩, c=⟨μ(t·d)d⟩` | 与 Original 使用**同一份固定样本和初态**，逐轮比较三项，不能仅比较最终 t |
| Original `:88–112` / Modified `:85–109` | `ω_new=a_ω+t_old×b_ω+c_ω`，**用旧 t 算 ω 后才提交新 t** | 记录旧 t、副本、ω_new 与提交顺序 |
| Original `:116–146` / Modified `:113–143` | 先求 MSE→RMS→delta→动态门限，循环后 `t *= mean(last_near)`；不重算最终 near | `k`、每轮 MSE/RMS/delta/grenze、最终近度均值与尺度；至少 6 次、常规上限 200 次 |

**无需运行即可证明的判别性约束**（只针对给定同一轮近度、t、ω 和输入）：

1. 若所有 `μ_i=1`，Original/Modified 的平移 `a/b/c` **逐项相等**，因此它们的本轮 `t_new` 相同（归一化有定义时）。**只有常数一近度的用例不能验证是否误删了 Original 的 `μ²`**。
2. 为判别两条路径，独立 L3 固定输入必须覆盖**非恒一、非均匀近度**及非退化视线/流场，使至少一项 `a_O-a_M`、`b_O-b_M` 或 `c_O-c_M` 非零；只有最终方向不同而中间项不可见仍不充分。由于 near 来自源码估计而非任意注入，输入需同时记录 Franz 初态与 near 估计过程。
3. `sum(near)<0` 的翻转先于 `a/b/c` 求和；`sum(near)=0` 不触发翻转。near 是**本轮开始**的 t/ω 计算结果，循环后也不重新计算；这是独立于连续值容差的离散合同。
4. 上述等式是静态源码与代数性质，**并非** KVD-G01～05 数值 Golden、真实 C++ 等价或 CTest 通过。需获授权后产生独立来源的完整逐轮数据及失败证据，`AF-043` 仍为「未整改」。


### 9.2 六方向固定流场的逐轮代数候选（AF-043 增量，**非已执行独立 L3**）

为避免“Original/Modified 只有一个最终方向差异”这种不充分的断言，使用**不执行任何 MATLAB/C++ 程序**即可在纸面逐项核对的固定 N×6 输入，给未来 `TC-F-FILTER-001`、`TC-F-KVD-001/002` 提供**可辨认的分支、逐轮 trace 与终止顺序**。这不是新的全局 AN-G ID，也不是原 `erzeugerichtungsvektoren(0)` 的有序采样输出。方向按表中人工定义的 `±e_x,±e_y,±e_z` 顺序，观测光流单位与原接口一致：

| MATLAB 行 i | `d_i`（方向前三列） | `p_i`（切向流后三列） |
|---:|---|---|
| 1 | (1,0,0) | (0,−1/2,0) |
| 2 | (−1,0,0) | (0,+1/2,0) |
| 3 | (0,+1,0) | (−1/2,0,0) |
| 4 | (0,−1,0) | (−3/2,0,0) |
| 5 | (0,0,+1) | (−2,0,0) |
| 6 | (0,0,−1) | (−2,0,0) |

这六行可独立解释为 `t=(1,0,0)`、`omega=(0,0,1/2)`、y 方向成对近度 1、z 方向成对近度 2 的理想球面流，且每行 `d_i·p_i=0`。x 方向的平移切向分量为零，**不能**从该方向识别任意真实近度；本夹具只对源迭代实际重算的近度作断言。

令 `epsilon=10^(-13)` 为原 KvD 分母项，`h=1/(1+epsilon)`。将此表代入**原** `franzeigen.m:13–36`：三个平移模板的范数分母均为 4，旋转模板的范数分母也均为 4；响应为平移 `(6,0,0)`、旋转 `(0,0,2)`，故 `t_Franz=(3/2,0,0)`，`omega_Franz=(0,0,1/2)`。KvD 先把初始平移归一，得 `t_old=(1,0,0)`，再按源近度公式重算：

\[
\mu^{raw}=\mu^{after}=(0,0,h,h,2h,2h)^\top,\quad
\sum_i\mu_i=6h>0,\quad \bar\mu=h.
\]

因此不发生符号翻转。对**同一轮输入/初态**，源两个分支的平移和（`a,b,c` 定义见 §3.1/3.2）分别为：

\[
(a_O,b_O,c_O)=(-\tfrac53h\,e_x,0,0),\qquad
(a_M,b_M,c_M)=(-e_x,0,0).
\]

候选归一化前向量分别为 `+(5h/3)e_x` 与 `+e_x`，其模长**不同**，归一后的 `t_new` 却都为 `e_x`。因此**仅核最后的平移方向会漏检 Original 的 near 权重与 near² 分支**；必须核每项 `a/b/c` 和归一前的向量（本例 `c=0`，相应非零 `mu²` 区分见下文 §9.3 的第二份独立静态夹具）。

两条路径共有的旋转项为：

\[
a_\omega=(0,0,1/3),\quad b_\omega=0,\quad
c_\omega=(0,0,1/6),\quad
\omega_{new}=a_\omega+t_{old}\times b_\omega+c_\omega=(0,0,1/2).
\]

按照原代码，先计算该旋转，再提交 `t_new`；不能重算 near 后才算残差。此时**每一轮**的预测流误差（原观测减预测）在 x 两行是零、y 两行是 `-(1-h)e_x`、z 两行是 `-2(1-h)e_x`，因此纸面解析得到：

\[
\mathrm{MSE}=\tfrac53(1-h)^2,\quad
\mathrm{RMS}=\sqrt{5/3}(1-h),\quad
\mathrm{grenze}=\mathrm{RMS}/(1000\cdot6).
\]

`h` 很接近 1；以源 `absgrenze=10^{-7}` 和每轮都稳定的符号公式，`k<6` 分支确保**至少 6 次**，第六次末满足停止并返回 `x=6`（这是**数学候选**，真实浮点路径仍须独立验证）。源循环后不重新估计 near，输出 `near=(0,0,h,h,2h,2h)`、`trans=h e_x`、`rot=(0,0,1/2)`。该夹具**没有**覆盖 200 次上限、near 翻号、退化零分母、MSE/浮点舍入一般性或带噪数据，不能用本例关闭 AF-043。

**核查与保存资格**：未来独立 L3 需将六行 `double[6×6]` 的精确值、上述每轴 Franz 响应/分母、每轮 `near/a/b/c/t_raw/t_norm/omega/MSE/RMS/grenze/k` 写成有 SHA-256 的完整参考数据，再用**真实已授权**的 C++/CTest 验证，并由 §9.3 的非零 c 候选开展独立数值核验，再补一般/异常案例。此处只有只读源码约定下的代数推导，**没有运行、没有数据文件、没有正式容差或批准**；AF-043、AF-051 保持「未整改」。

### 9.3 另一组六方向非轴对齐夹具：`mu²` 与 `mu` 的非零 c 项（AF-043，静态代数）

§9.2 的 `c_O=c_M=0` **不能**捕获 Original 误用 Modified 的 `c` 更新式。为覆盖这一缺口，再定义一个独立的**纸面候选**：令 `u=1/sqrt(2)`，依下表人工组成 6 行切向 N×6 输入。前四行取全部 `(±u,±u,0)` 组合，末两行取 `±e_z`：

| i | `d_i` | `p_i` |
|---:|---|---|
| 1 | (+u,+u,0) | (−1,+1,0) |
| 2 | (+u,−u,0) | (−1,−1,0) |
| 3 | (−u,+u,0) | (−1,−1,0) |
| 4 | (−u,−u,0) | (−1,+1,0) |
| 5 | (0,0,+1) | (−1,0,0) |
| 6 | (0,0,−1) | (−1,0,0) |

这相当于 `t_true=e_x,omega_true=0` 的理想平移光流：斜向四行 `near_true=2`，末两行 `near_true=1`。六行均满足 `d_i·p_i=0`；其单位向量和几何对称性使 Franz 的**所有方向轴分母非零**、平移初始化为 `(3/2)e_x`、旋转初始化为 `0`，归一化后 `t_old=e_x`。这里仍令原 `nulldiv=epsilon=10^{-13}`，定义

\[
\eta=\frac1{1+2\epsilon},\qquad \beta=\frac1{1+\epsilon}.
\]

斜向 `(t·d)^2=1/2`、`t·p=-1`，故按**原 near 分母**得到 `near=2eta`；z± 方向则得到 `near=beta`。六行的源近度为 `(2eta,2eta,2eta,2eta,beta,beta)`，均值 `(4eta+beta)/3`，不触发翻号。直接按 §3.1/3.2 计算平移中间量：

\[
\begin{aligned}
\boldsymbol a_O&=-\frac{4\eta+\beta}{3}\,\boldsymbol e_x,
&\boldsymbol b_O&=0,
&\boldsymbol c_O&=\frac43\eta^2\,\boldsymbol e_x,\\
\boldsymbol a_M&=-\boldsymbol e_x,
&\boldsymbol b_M&=0,
&\boldsymbol c_M&=\frac23\eta\,\boldsymbol e_x.
\end{aligned}
\]

在两种更新式中 `omega=0`，故 `t_candidate_O=((4eta+beta+4eta²)/3)e_x`、`t_candidate_M=(1+2eta/3)e_x`；它们的 `c` **均非零且不同**，而归一化方向仍同为 `e_x`。因此必须用本例的 `a/b/c` 逐项和归一前候选值区分两种实现；只检查最终方向/角度将**不能**检测错误使用 `mu` 或 `mu²`。

由于成对样本的平移流关于 `d→-d` 为偶，旋转模板与 `p×d` 成对抵消，纸面 `omega_new=0`。每轮 near 不重算终点；固定点状态下误差可进一步按每个样本求得：

\[
\mathrm{MSE}=\frac43(1-\eta)^2+\frac13(1-\beta)^2,\qquad
\mathrm{RMS}=\sqrt{\mathrm{MSE}},\quad
\mathrm{grenze}=\mathrm{RMS}/6000.
\]

精确算术下源 `k<6` 仍令这组稳定场完成 6 轮，并在循环后返回 `trans=((4eta+beta)/3)e_x`、`rot=0` 和当前六行近度；真实 `double` 逐轮误差、`absgrenze` 分支与退出次数**仍须独立 L3 和真实运行核对**，不能用纸面推导当 PASS。

**适用边界**：这不是原作者 i=0 球面有序采样，不是 Matlab 实测，不是 C++ 算法输出，也不替代一般/噪声/near 翻号/200 次上限/异常测试。本例只把先前 `c=0` 的测试盲区变成了可辨认的固定公式、输入/trace 候选；`KVD-G01～05` 和 `TC-F-KVD-001/002` 的独立 L3 数值身份、真实测试、DEC-005/008 及 AF-043/051 的完整验收仍开放。

### 9.4 精确Wolfram计算的四条六轮KvD轨迹（AF-043，限定数学参考，非生产软件）

此前§9.2/9.3是**手工/静态候选**。现在已用独立 [Wolfram可重放脚本](../../verification/kvd/verify_six_direction_exact_trace.wl) 根据只读 `zellendefinieren.m`、`franzeigen.m`、`norm_koenderink_iteration.m`、`mue_norm_koenderink_iteration.m` 的精确赋值先后在符号算术中执行：两组表内 **N=6** 输入分别走 Original/Modified，得四条**各六轮、共24轮的完整数学 trace**（不是 C++ 或源 MATLAB 实际运行）。固定输入、模板响应/平方范数、每轮 near/均值/翻号、平移 a/b/c/原始候选、旋转 aω/bω/cω、归一化姿态、MSE/RMS/差分/动态门限、停止判定与最终输出，均保存于 [逐轮参考JSON](../../verification/kvd/six_direction_exact_trace.v1.json)。生成器 Git blob `5e288035ede1775a8a16dd5abf177a378cb37a1f`，其关联四个源文件 Git blob/局部 `epsilon=10^{-13}` 明列于参考JSON，正式SHA-256/DEC-005仍不批准。

**共同的严格解析性质**：两个流场的平移模板平方范数为 `(4,4,4)`，平移响应为 `(6,0,0)`，因此 Franz 未归一化结果**均**为 `(3/2)e_x`。第一组旋转模板响应 `(0,0,2)`、旋转 `(1/2)e_z`；第二组旋转响应全零。这里原 §9.3 的 `(3/2)e_x` 是正确值：斜向平移模板的内积必须计算所有 x/y 分量，不得只算光流 `p_x`。

定义 `h=1/(1+epsilon)`，`eta=1/(1+2epsilon)`，`beta=1/(1+epsilon)`。Wolfram 逐条输出与两组手工候选的精确形式相符：

| 固定流场与分支 | 第一轮 raw t（未归一化） | 各轮 MSE | `omega_new` | 第六轮后的最终 trans |
|---|---|---|---|---|
| §9.2 轴向 Original | `(5h/3)e_x` | `(5/3)(1-h)^2` | `(1/2)e_z` | `h e_x` |
| §9.2 轴向 Modified | `e_x` | 同上 | `(1/2)e_z` | `h e_x` |
| §9.3 斜向 Original | `[(4eta+beta+4eta²)/3]e_x` | `(4/3)(1-eta)^2+(1/3)(1-beta)^2` | `0` | `[(4eta+beta)/3]e_x` |
| §9.3 斜向 Modified | `[1+2eta/3]e_x` | 同上 | `0` | `[(4eta+beta)/3]e_x` |

**真实数学迭代停止分支**：轴向两算法首轮RMS均 `1.2909944487356765e-13`，动态门限 `2.1516574145594608e-17`；斜向两算法首轮RMS `2.3804761428471544e-13`，动态门限 `3.967460238078591e-17`。四组首轮 `delta=RMS<10^{-7}`，第2～6轮 `delta=0`，且两类归一化 `t=e_x`、初始旋转和 near 均为固定点；因此`x<6`维持前五次继续，第六次执行完退出并返回 `x=6`。归一化前平移不同，却得到相同的末次方向与尺度，证明**必须观察原始 a/b/c/tRaw**，而非仅最终t或误差。

**证据范围与未决**：本次有独立Wolfram准确输入与实际逐轮数字、14项静态/数值自检（14/14），但不是源码 MATLAB、经批准的生产 C++/CTest 或正式Golden；只包含固定的两组六方向和 `near` 正均值、没有 near 翻号、病态 Franz 模板、噪声、200轮上限或一般连续状态。迭代恒定来自本固定数学场景，不能把它扩展成所有输入均在6轮停止的结论。AF-043仍须更多一般/失败逐轮参考、真实实现与DEC-005批准，继续保持「未整改」。

### 9.5 首轮 near 负总和的判别性八方向精确夹具（AF-043，理论数学执行）

§9.2/9.3 的两个固定流场的 near 总和均为正，**没有覆盖原源码 sum(near)<0 时在平移项求和之前同时反转 near 与 trans 的路径**。现构造混合正负的**数学光流输入**，只验证这一离散分支，并不宣称所有物点距离为正。

令 \(s=1/10\)、\(c=\sqrt{99}/10\)、\(\epsilon=10^{-13}\)，按行输入八个单位视线

\[
(c,s,0),\ (c,-s,0),\ (-c,s,0),\ (-c,-s,0),
\quad \boldsymbol e_y,-\boldsymbol e_y,\boldsymbol e_z,-\boldsymbol e_z.
\]

对前四行给 \(\boldsymbol p_i=10(\boldsymbol e_x-d_{ix}\boldsymbol d_i)\)，后四行给 \(\boldsymbol p_i=-(\boldsymbol e_x-d_{ix}\boldsymbol d_i)\)；全部满足 \(\boldsymbol d_i^\top\boldsymbol p_i=0\)。这等效于第一组近轴方向的**有符号**近度 \(-10\)，以及末四个方向的近度 \(+1\)，仅适用于非物理的控制流测试。

Franz 的三轴平移响应 \((18/5,0,0)\)，平方范数分母 \((101/25,149/25,6)\)，所以

\[
\boldsymbol t_{\mathrm{Franz}}=\frac{90}{101}\boldsymbol e_x,\qquad
\boldsymbol\omega_{\mathrm{Franz}}=\boldsymbol 0.
\]

设 \(\alpha=1/(1+100\epsilon)\)、\(\beta=1/(1+\epsilon)\)。第一次估计的近度为四个 \(-10\alpha\) 和四个 \(+\beta\)，总和 \(-40\alpha+4\beta<0\)。因此原源码**先将 near 翻转为四个 \(10\alpha\) 与四个 \(-\beta\)，再将旧 t 从 \(+\boldsymbol e_x\) 翻为 \(-\boldsymbol e_x\)，最后才求平移 \(a,b,c\)**。翻号后的 near 均值为 \((5\alpha-\beta)/2>0\)。直接代入两条源码规则：

\[
\begin{aligned}
\boldsymbol a_O&=\frac{\alpha+\beta}{2}\boldsymbol e_x,&
\boldsymbol b_O&=\boldsymbol 0,&
\boldsymbol c_O&=-\frac{99}{2}\alpha^2\boldsymbol e_x,\\
\boldsymbol a_M&=-\frac9{20}\boldsymbol e_x,&
\boldsymbol b_M&=\boldsymbol 0,&
\boldsymbol c_M&=-\frac{99}{20}\alpha\boldsymbol e_x.
\end{aligned}
\]

从而得到归一化之前的候选平移

\[
\widetilde{\boldsymbol t}_O
=-\frac{\alpha+\beta+99\alpha^2}{2}\boldsymbol e_x,\qquad
\widetilde{\boldsymbol t}_M
=-\frac{99\alpha-9}{20}\boldsymbol e_x .
\]

二者均归一为 \(-\boldsymbol e_x\)，旋转保持零。模型的单轮误差为

\[
\mathrm{MSE}=\frac12(1-\alpha)^2+\frac12(1-\beta)^2,\qquad
\mathrm{RMS}\approx7.071421356346968\times10^{-12}.
\]

首轮 \(\delta=\mathrm{RMS}<10^{-7}\)，第2～6轮 \(\delta=0\)，动态门限约 \(8.839276695433711\times10^{-16}\)。由于条件 **x<6**，前五轮仍继续，第六轮后退出。首轮翻号为真，后五轮为假，最后返回

\[
\boldsymbol t_{\mathrm{out}}
=-\frac{5\alpha-\beta}{2}\boldsymbol e_x
\approx-4.49999999995005\boldsymbol e_x,\qquad
\boldsymbol\omega=\boldsymbol 0.
\]

这些结果已由 [Wolfram精确运算脚本](../../verification/kvd/verify_near_sign_flip_exact_trace.wl) 分别对 Original/Modified 执行6轮、累计12轮，并保留在 [固定逐轮参考](../../verification/kvd/near_sign_flip_exact_trace.v1.json)，11/11数学自检通过。**这不是MATLAB或生产C++运行、正式DEC-005批准的Golden，也不是正距离场景的物理结果**。仍缺200轮可达性、一般噪声、Franz零分母和病态情形；AF-043继续开放。

### 9.6 原循环最少6轮和最多200轮的严格布尔界限（AF-043，纯理论）

原版两条 KvD 函数共同使用的 while 条件为

\[
B_k=((\Delta_k>g_k)\land(k<200)\land(\Delta_k>10^{-7}))
\lor(k<6).
\]

其中 \(k=x\) 表示**检查下一轮之前已完成**的次数，初值 \(k=0\)，每个循环体首先执行 x=x+1。末尾的逻辑或与前三项的逻辑与优先级不能混淆。

**下界**：当 \(k=0,1,\ldots,5\) 时，第二个析取项 \(k<6\) 必真。因此若没有异常或提前失败，至少运行6轮。

**上界**：当 \(k\geq200\) 时，两支均为假，下一轮不执行。每轮从0自增1，所以对任何正常返回，

\[
\boxed{6\leq k_{\mathrm{return}}\leq200}.
\]

为了仅验证这一**布尔控制结构**能够允许200轮，可以给定人工 RMS 序列 \(r_0=0\)，奇数轮 \(r_k=1\)，偶数轮 \(r_k=2\)。于是 \(\Delta_k=|r_k-r_{k-1}|=1>10^{-7}\)；对任意正整数样本数 \(N\)，\(g_k=r_k/(1000N)\leq0.002<1\)。所以当 \(k<200\) 第一支继续成立，完成第200轮后两支都为假。

**人工残差序列并不证明实际固定光流场中的 KvD 运动迭代可产生该序列**；这既非原 MATLAB 运行，也不是生产 C++ 的200轮数值测试。§9.4/9.5 的三组源公式输入分别在6轮停止，而理论上下界适用于所有没有失败的正常控制流；不能以这两种证据直接关闭 AF-043/DEC-005。

## 10. 与架构及开发工作的绑定

- P4.1/P4.2 → FranzMatchedFilter，验证三轴六类响应；P4.3/P4.4/P4.5/P4.9 → 两条 KvD 平移规则；P4.6 → while 与停止统计；P4.7 → 两个不同误差计算；P4.8 → Golden；P4.10 → 多次调用的状态隔离。
- 内部公共引擎仅负责 near、旋转、残差与停止检查，不能在某个布尔选项中悄悄消去 near²。
- 方法接口与返回值的最终冻结由 [对象模型](../OBJECT_MODEL.md) 和 [测试计划](../TEST_PLAN.md) 一起审查；模块依赖保持 estimator → domain/math，不读取或写入 MAT。

**H-13 历史记录与后续所有者决定（分开适用）**：原 H-13 仅允许对 MATLAB NaN/Inf、奇异、停止判定和变量覆盖次序作静态审查并起草失败方案，**当时并未授权遇错继续/回滚/终止的生产实施**。其后项目所有者已另行**选定 C++ 显式失败纪律**：无状态 KvD 单次求解失败即传播非成功，不返回伪造运动；独立实验样本可在 Runner 记录失败后继续其它独立样本；Adaptive 失败步不提交主系数，不能安全恢复的连续序列须停止，保留可追踪部分结果且八次聚合不改变分母（参见 [contracts.md](contracts.md)、[experiments.md](experiments.md) 及 [所有者决定](../decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md)）。这些是**后续设计原则，不等于原 MATLAB 异常传播已实测**，也**不解除 H-10/H-11** 的实际代码和现场运行暂缓。历史 H-01 仍为 draft；0.01/0.1 仅是后来确认的首轮筛查，最终 DEC-005 阈值未冻结。

**就绪状态**：数学步骤和逐轮字段已列出；仍缺覆盖一般输入的独立 L3 逐轮 Golden、批准容差、实现/编译、具体测试函数/CTest。文档可用于技术审查，不可据此标记模块已通过。

## 附录 A：无需运行 MATLAB 的证据边界

按 [当前 L2/L3/L4 证据矩阵](../golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md) 和 [长期无需 MATLAB 执行政策](../golden/NO_MATLAB_EXECUTION_POLICY.md)，**不要求原 MATLAB 逐节点/逐调用的 L1，也不因没有 L1 阻塞本模块的设计/编码/测试/验收**。原 `.m` 作为只读公式、分支、时序和数据合同依据；当前应准备模块适用的 **L3 独立固定输入逐节点参考**，L2 历史 MAT 用于最终结果范围，L4 用于统计。需要保留真实 C++/CTest、DEC-005 容差、错误与来源限制记录，不能把 L3 宣称为原 MATLAB 实际运行的完整等价证据。本文及该矩阵均未宣布模块验收通过。
