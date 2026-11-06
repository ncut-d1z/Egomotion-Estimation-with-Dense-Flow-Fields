# Franz / Original KvD / Modified KvD 详细设计草案

> **LLD v0.3（未冻结、未实现、未验证）**。下列伪代码和公式根据源 MATLAB 函数重新核对，仍需有 MATLAB 固定输入 Golden 与经 DEC-005 批准的容差后才能冻结。需求：REQ-F-FILTER-001、REQ-F-KVD-001/002、REQ-F-ERROR-001、REQ-DC-KVD-001、REQ-NF-NUM-001/002；任务：P4.1–P4.10。

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

每个 TC 必须记录源码 SHA、固定输入来源、绝对/相对最大误差、被测矩阵/向量维度及异常状态。**DEC-005 数值容差未冻结，1e−10 只是候选。** 不能仅凭整体论文图形相似就声称逐轮算法等价。

## 6. 未决与冻结门禁

- 固定 MATLAB Golden 数据尚未产生并归档；各阶段仅有公式/行为的静态核对。
- 对原始 MATLAB 数值失败和新增显式错误返回的差异需列入合同，不能以“修复”之名改变忠实输出。
- C++17 结果类型、具体签名、迭代快照的保存策略、算法默认行为、数值容差和 CTest 名称均待 P0.8/P4 审批。
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
struct KvDIterationTrace {
    std::size_t k;
    Vector3 translation_before_sign;
    Vector3 translation_after_sign;
    Vector3 rotation_before;
    Eigen::VectorXd nearness_raw;
    Eigen::VectorXd nearness_after_sign;
    Vector3 translation_sum_a;
    Vector3 translation_sum_b;
    Vector3 translation_sum_c;
    Vector3 translation_candidate;
    Vector3 rotation_after;
    Scalar residual_mse;
    Scalar residual_rms;
    Scalar previous_rms;
    Scalar residual_change;
    Scalar active_threshold;
};
struct KvDResult {
    MotionEstimate motion;
    Eigen::VectorXd final_nearness;
    std::size_t iteration_count;
    std::vector<KvDIterationTrace> trace; // 可按设置输出；Golden 模式必须可见
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

**合同待定项**：NumericalErrorCode 单独 enum 是否足以携带上下文、Result 如何表达 NaN/Inf 的原版行为、trace 在性能模式是否可关闭，以及是否允许方法接收非单位视线。须依 P0.8 与实际 MATLAB Golden 冻结，不能仅凭该示例签名开始实现。

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
- 一部分原 MATLAB 输入会继续传播 NaN；显式返回 NumericalErrorCode 需要同时记录原版行为和安全模式的分歧，未经确认不可宣称逐值等价。

## 9. 固定 Golden 的分层生成与测试数据格式

建议 Golden 以**只读 MATLAB 参考导出**形式存为对照数据；不得把被测 C++ 结果拷贝成 Expected。每个测试案例至少保存：

| 样本 ID（计划） | 条件 | MATLAB 采集内容 | 目标 |
|---|---|---|---|
| KVD-G01 | i=0 小采样、无噪声/固定 near、非退化 t/ω | Franz 中间响应、每轮 near、Original/Modified t/ω、残差、停止 | 符号和更新顺序 |
| KVD-G02 | 符号翻转（sum near<0） | near_before/after、t_before/after 与最终 scale | 保留符号分支 |
| KVD-G03 | 非零固定 Gaussian、近似噪声输入 | 两个估计器逐轮误差和次数 | 对照统计数据与容差 |
| KVD-G04 | 人工零模或几何退化输入 | MATLAB 是报错、产生非有限值还是仍返回 | 明确 C++ 状态与忠实性差别 |
| KVD-G05 | 能区分最低 6 次、提前停止和 200 次上限的输入 | k、每轮 delta/threshold/残差 | 验证完整 while 语义 |

黄金记录字段应有源 Git blob SHA、原始函数名、MATLAB 版本、固定输入身份、输出每个字段的 shape/dtype/数值、允许误差和对应 TC-F-FILTER-001、TC-F-KVD-001/002、TC-F-ERROR-001。**上述 G01–G05 只是测试设计编号，当前没有数据文件或运行证据。**

## 10. 与架构及开发工作的绑定

- P4.1/P4.2 → FranzMatchedFilter，验证三轴六类响应；P4.3/P4.4/P4.5/P4.9 → 两条 KvD 平移规则；P4.6 → while 与停止统计；P4.7 → 两个不同误差计算；P4.8 → Golden；P4.10 → 多次调用的状态隔离。
- 内部公共引擎仅负责 near、旋转、残差与停止检查，不能在某个布尔选项中悄悄消去 near²。
- 方法接口与返回值的最终冻结由 [对象模型](../OBJECT_MODEL.md) 和 [测试计划](../TEST_PLAN.md) 一起审查；模块依赖保持 estimator → domain/math，不读取或写入 MAT。

**就绪状态**：数学步骤和逐轮字段已列出；仍缺原 MATLAB Golden、批准容差、实现/编译、具体测试函数/CTest。文档可用于技术审查，不可据此标记模块已通过。
