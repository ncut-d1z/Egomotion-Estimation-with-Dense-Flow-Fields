# Adaptive MFA 单步计算与轨迹状态——详细设计草案

> **LLD v0.4：未冻结、未实现、未测。** 已核对原 MATLAB 的 oneit_adaptiv_matched_filter.m 关键表达式，但仍须独立 L3 逐节点参考和真实测试、确定最终数值容差；**异常时不提交无效状态的原则已由所有者确定**。需求：REQ-F-ADAPT-001/002/003、REQ-F-EXP-001/002/003、REQ-DC-ADAPT-001；任务 P7.7–P7.11、P8.1–P8.9。

## 1. 单步输入/输出与接口候选

原函数签名：oneit_adaptiv_matched_filter(altkoeff, setofpoints) → (neukoeff, trans, rot, error)。

| 字段 | MATLAB 合同 | C++ 设计约束 |
|---|---|---|
| altkoeff | 3×5，l≤2 球谐系数 | 输入值对象，不修改原对象 |
| setofpoints | N×6，每行 d(3)、p(3) | FlowField，保留顺序 |
| neukoeff | 3×5 新系数 | 输出独立值对象 |
| trans、rot | 3×1 与 3×1 | 分别标记平移/旋转向量 |
| error | **平均平方流场残差（MSE）** | **不是 RMS**；不可仅凭变量名称推断单位 |

**源码关键细节**：函数内部计算 error=mean(||p−p_pred||²)，随后另行定义 oneit_error=sqrt(error)，但返回列表返回的是 error 而不是 oneit_error。此处先按源代码保留 MSE；如需要 RMS 展示，须由调用方显式计算新字段，且不能替代原返回变量。

## 2. 一次迭代的精确矩阵公式

令 \( \langle x\rangle=N^{-1}\sum_i x_i \)，方向为 \(\boldsymbol d_i\)，观测切向光流为 \(\boldsymbol p_i\)，恢复的近度为 \(\mu_i\)，旧 3×5 系数为 C。

### 2.1 近度与均值归一化（严格按源码顺序）

\[
\mu_i\leftarrow\mathrm{near\_aus\_koeff}(C,\boldsymbol d_i),\quad
s\leftarrow\langle\mu_i\rangle.
\]

原始代码：

~~~text
s = mean(near)
if s < 0: near = -near
near = near / s
~~~

**特别审计点**：s 在符号翻转之前求得，符号翻转后没有重新求 s；负均值时这与先取绝对值归一化不等价。不得无说明地把它“纠正”为 abs(s) 或修改代码顺序。需要为 s>0、s<0、s≈0 分别建立原版对照与异常状态。

### 2.2 6×6 块线性系统

记 \([\boldsymbol v]_\times\) 为叉乘矩阵，即 \([\boldsymbol v]_\times\boldsymbol x=\boldsymbol v\times\boldsymbol x\)，I 为 3×3 单位矩阵。原 MATLAB 右端：

\[
\boldsymbol b_t=-\langle\boldsymbol p_i\rangle,\qquad
\boldsymbol b_\omega=\langle\boldsymbol p_i\times\boldsymbol d_i\rangle.
\]

四个 3×3 块：

\[
M_1=I-\langle\mu_i\boldsymbol d_i\boldsymbol d_i^\top\rangle,
\qquad M_4=I-\langle\boldsymbol d_i\boldsymbol d_i^\top\rangle,
\]
\[
M_2=[-\langle\boldsymbol d_i\rangle]_\times,\qquad
M_3=[\langle\mu_i\boldsymbol d_i\rangle]_\times.
\]

联立：

\[
M=\begin{bmatrix}M_1&M_2\\M_3&M_4\end{bmatrix},\quad
\boldsymbol b=\begin{bmatrix}\boldsymbol b_t\\\boldsymbol b_\omega\end{bmatrix},
\qquad
\boldsymbol z=\operatorname{real}(M^{-1})\boldsymbol b.
\]

原版按以下方式解释：

\[
\boldsymbol t=\frac{\boldsymbol z_{1:3}}{\|\boldsymbol z_{1:3}\|},\qquad
\boldsymbol\omega=\boldsymbol z_{4:6}.
\]

对应源码使用 real(inv(mgesamt)) * vekgesamt。Eigen 的 QR/LDLT/SVD 求解可以作为替代实现候选，但**必须先对固定输入的 M、b、z、t、ω 核对并另记改变**，不能暗中把原方程替换为正则化或其他模型。

### 2.3 原返回 error、近度重新估计和系数更新

给定已归一的近度和此前保存的 s，原版预测流：

\[
\widehat{\boldsymbol p}_i=
-\mu_i\,s\left(\boldsymbol t-(\boldsymbol t^\top\boldsymbol d_i)\boldsymbol d_i\right)
-\boldsymbol\omega\times\boldsymbol d_i.
\]

\[
\text{error}=\langle\|\boldsymbol p_i-\widehat{\boldsymbol p}_i\|^2\rangle
\quad(\text{MSE，源函数返回}),\qquad
\text{oneit\_error}=\sqrt{\text{error}}\quad(\text{未返回}).
\]

然后按新的 t、ω 用原 MATLAB 的公式重新计算每个 near：

\[
\mu_i^{new}=
-\frac{\boldsymbol t^\top(\boldsymbol p_i-\boldsymbol d_i\times\boldsymbol\omega)}
{1-(\boldsymbol t^\top\boldsymbol d_i)^2+10^{-11}},
\quad
C^{new}=\mathrm{koeff\_aus\_near}(\mu^{new},\{\boldsymbol d_i\},2).
\]

注意 10^−11 是 Adaptive 单步的分母稳定常量，**不同于**两种 KvD 中的 10^−13。内部 result 应可同时保存 errorMse 和经明确变换产生的 rmsResidual，但原始 error 字段语义必须保留。

## 3. 单步计算伪代码（仅设计）

~~~text
single_step(old_coeff, flow):
    near = reconstruct_nearness(old_coeff, flow.d)
    s = mean(near)
    if s < 0: near = -near
    near = near / s                 # 保留原版先翻 near 后除 s 的顺序
    b = concat(-mean(flow.p), mean(cross(flow.p, flow.d)))
    M = blocks(M1, M2, M3, M4)     # 确切定义见 §2.2
    z = real(inverse(M)) * b       # 精度与分解替换需先对照
    t = normalize(z[1:3])
    omega = z[4:6]
    error_mse = mean(square_norm(flow.p - predict(flow, near, s, t, omega)))
    near_new = estimate_nearness(flow, t, omega, epsilon=1e-11)
    new_coeff = fit_spherical_harmonics(near_new, degree=2)
    return {new_coeff, t, omega, error_mse, status}
~~~

发生 s≈0、t≈0、M 奇异或 NaN/Inf 时，不得假称返回成功；原 MATLAB 若产生非有限值，需保留数值行为证据，并在新 C++ 的显式失败状态中报告差异。

## 4. 候选 C++17 公开接口（未批准）

**单步和 Session 的唯一候选签名在本文件 §8**；其共用值类型、Result、NumericalErrorCode 和 ExperimentErrorCode 由 [contracts.md](contracts.md) 维护。避免在同一 LLD 中同时引入 AdaptiveMfaStepResult / AdaptiveStepResult 或 NumericalError / NumericalErrorCode 两套名称。

Result<T,E>、FlowField、FrameMotionContext、状态枚举及构造注入方式在 P0.7/P0.8 后冻结。单步求解器不强制继承 KvD，也不持有主 akkoeff 的跨帧可变状态。

## 5. 三个实验 Runner 的实际状态时序差异

**初始化入口** agent_in_sphere_initialization_test.m（35 个圆轨迹点）：
- 每帧构造 ohnekoeff（零阶常数 1）与 mitkoeff（继承上一轮的 akkoeff），分别调用 oneit；随后单独更新主 akkoeff。
- **当前这个入口没有调用 transfor_koeff 进行前一帧旋转**；不得因为其他两个入口有旋转就强行加入。

**带/不带噪声对比入口** agent_in_sphere_adaptive_matched_filter_test.m（600 位置、等级 4、8 次运行）：
- 每帧先按上一帧相对旋转执行 transfor_koeff(akkoeff, R)；然后设置 ohnekoeff/mitkoeff、分别执行单步，最后再更新主 akkoeff。
- 先生成合成光流，再按 addederror 调用 nonpropfehler；保留无噪时参数为零仍会消耗随机调用的事实（需核对执行细节）。
- 四个误差 MAT 为逐运行求和后除以 8；rottransratio 是循环内赋值的**最后一次运行值**，不在同样位置求八次平均。

**更新频率入口** agent_in_sphere_adaptive_matched_filter_many_parameter.m（600 位置、等级 2、单次运行）：
- 每帧旋转主 akkoeff 和四组 mitkoeff；在 mod(i, [1,5,11,21])==0 时把对应 mitkoeff 刷新为主 akkoeff，然后分别调用单步，再更新主 akkoeff。
- 当前源脚本实际保存 allemittransfehler、allemitrotfehler、akrot 三类 MAT；历史目录还含 alleadaprotfehler、alleadaptransfehler、rottransratio，**历史额外文件不得伪造为当前运行输出**。

### 5.1 会话状态/异常提交合同

~~~text
Per-run Session (one owner of akkoeff)
    Setup(constant coefficient at (1,1))
    for each valid trajectory index:
        optionally rotate coefficient state according to entry-specific script
        apply entry-specific comparison coefficient policy
        run comparator oneit calls
        run main oneit and record next akkoeff
        record errors/ratios and preserve file-output contract
~~~

**项目所有者已确定失败状态规则**（见 [最新决定](../decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md) 与 [experiments.md §8.1](experiments.md)）：

- StepSolver 返回非成功，不产生可提交的 `coefficients_next`；Session 仅在所有必须计算节点成功且有限时**原子提交下一帧主系数**，否则保持上一次有效主/基线系数，不将无效 near/系数写入状态。
- 若是依赖上一帧状态的连续序列，且没有经证实安全的恢复路径，则**停止当前轨迹**并标记 `Incomplete`/`Failed`；独立运行/独立样本可以由 Runner 记录失败后继续，不改变有效样本分母。
- 此策略是**明确的 C++ 安全合同**，不声称已验证原 MATLAB 特定异常时 NaN/Inf 传播；原 MATLAB 无统一事务机制不是强制复现该缺陷的要求。独立 L3 失败输入、状态 before/after 和真实 CTest 仍须完成。

## 6. 每一步 Golden 跟踪表及验收

| 计算节点 | 必需量 | 数值对照来源 |
|---|---|---|
| near 原始与归一 | C_old、near_before、s、near_after_sign、near_normalized | oneit 第 19–29 行 |
| 右端、矩阵 | b_t、b_ω、M1/M2/M3/M4、M(6×6)、det/condition（新诊断） | oneit 第 32–91 行 |
| 线性求解 | z(6)、t_raw、t_norm、ω、失败状态 | oneit 第 95–105 行 |
| 残差 | 每个 p_pred、error_mse、sqrt(error_mse) 的独立诊断 | oneit 第 108–117 行 |
| 更新 | near_new、C_new(3×5) | oneit 第 120–132 行 |
| 会话 | 前一帧 R、主/基线系数 before/after、频率策略、i、输出误差 | 三个 Adaptive 生成脚本 |

所有黄金值应有**固定输入与独立 L3 数值参考**、原只读 .m 源 SHA、参考生成过程及输出 SHA-256、批准容差和测试记录；**不要求原 MATLAB 实际输出，也不能用被测 C++ 自算值验证自身**。

关联 TC-F-ADAPT-001/002/003、TC-F-EXP-001/002/003、TC-DC-ADAPT-001、TC-NF-SESSION-001；更完整的证据设计见 [TEST_PLAN.md](../TEST_PLAN.md)。

## 7. 阻塞与批准事项

- 精确 C++ 接口与错误码实现、状态原子提交的真实测试、独立 L3 参考、DEC-005 最终数值容差和 CTest 尚未完成；**异常不提交原则已确定，不需要再次人类选择**。
- 本文的伪代码和公式是静态源码审计设计材料；没有编译、MAT 变量全量验证或数值运行证据。
- **源函数返回 error=MSE，而非 RMS**，未来任何与之不符的计划/SRS/TC 都必须修正后才能作为对照基线。

**本文件不能直接宣称达到 LLD 的最终冻结标准。**


## 8. StepSolver / Session 的精确输入输出和状态合同

公共 Result<T,E>、ErrorCode、FlowField、HarmonicCoefficients 候选定义由 [contracts.md](contracts.md) 统一维护。这里仅声明 Adaptive 特有中间量，不增加全局隐式 RNG、MAT I/O 或跨帧求解状态。

~~~cpp
namespace egomotion {
struct AdaptiveStepTrace {
    Eigen::VectorXd near_raw, near_after_sign, near_normalized;
    Scalar original_near_mean;
    Matrix3 m1, m2, m3, m4;
    Vector6 rhs, solved_motion;
    Matrix6 system_matrix;
    Vector3 translation_raw;
    Scalar error_mse;              // 返回变量 error，不是 RMS
    Eigen::VectorXd recomputed_near;
    HarmonicCoefficients coefficients_next;
};
struct AdaptiveStepResult {
    MotionEstimate motion;
    HarmonicCoefficients coefficients_next;
    Scalar error_mse;
    AdaptiveStepTrace trace;
};
class AdaptiveMfaStepSolver {
public:
    Result<AdaptiveStepResult, NumericalErrorCode> solve(
        const HarmonicCoefficients& coefficients_old,
        const FlowField& observations) const;
};
struct AdaptiveFrameInput {
    std::size_t matlab_frame_index;  // 原 MATLAB 1-based，而非 C++ 下标
    Matrix3 rotation_from_previous_frame;
    FlowField observations;
};
struct FrameResult {
    std::size_t matlab_frame_index;
    AdaptiveStepResult primary_step;
    std::vector<MotionEstimate> comparison_motion;
    std::vector<Scalar> comparison_translation_error_deg;
    std::vector<Scalar> comparison_rotation_error_deg;
};
class AdaptiveMfaSession {
public:
    Result<FrameResult, ExperimentErrorCode> processFrame(
        const AdaptiveFrameInput& frame);
};
}
~~~

Candidate interface only, not compiled: FrameResult, Session construction policy, Result error details and trace storage are pending P0.8 sign-off. A StepSolver invocation does not need to construct Session or access previous frames.

### 8.1 Failure classifications and state commit

| 故障 | 最少保存的原 MATLAB 证据 | 候选数值状态 | 主系数是否提交 |
|---|---|---|---|
| FlowField/coeff 维度或非有限 | N×6、3×5 实际输入 | InvalidShape、NonFiniteInput | **禁止提交，无效输入不能产生新系数** |
| near 的原始均值 s 为零、接近零或负数 | near_raw、s、符号翻转、最终归一 | Zero/near-zero 退化返回 InvalidNearness；**负 s 按原公式处理，不自动当失败** | **仅真正非成功时不提交**；有效负 s 不改变符号算法 |
| 6×6 退化、数值条件很差 | M1–M4、b、M、z、原版 real(inv(M)) | SingularSystem、IllConditionedSystem（判据待实测） | **非成功不提交；不能安全恢复则终止当前连续序列** |
| 线性解的 t 长度为零 | raw t、范数、归一化后状态 | ZeroNorm | **非成功不提交** |
| error_mse、near_new 或系数非有限 | 独立极值/病态输入及明确 C++ 状态 | NonFiniteResult | **非成功不提交；原 MATLAB 运行细节不作强制对照** |
| MAT 写入中断 | 最后成功帧与独立输出目录 | OutputFailed（Runner 层） | 不由 StepSolver 决定 |

**现行所有者决定优先于旧 H-13 仅起草状态**：StepSolver 计算失败、Session 主系数提交、Runner 整体 RunId 状态是三种不同层次；单步错误不得提交无效系数，不能安全恢复时停止当前连续序列，部分结果可保留审查但不伪装为完成。真实原版异常行为不可证明时按来源限制记录，不运行 MATLAB/Octave。H-10 的实际代码/测试运行仍须另行授权，最终数值容差和状态回归仍待实证。

## 9. 三个原入口的不同状态语义

| 入口 | 主循环（MATLAB 1-based） | 旋转系数 | 对比基线 | 原 save 的矩阵预分配形状 |
|---|---|---|---|---|
| agent_in_sphere_initialization_test | i=2…34，35 行 | **没有调用 transfor_koeff** | 每帧重置 ohne=常数，mit=akkoeff；各自单步后主 akkoeff 更新 | 四种误差：35×1 |
| agent_in_sphere_adaptive_matched_filter_test | i=2…599，8 次完整运行，600 行 | 上一帧相对 R 旋转主系数 | 每帧 ohne/mit 求解，然后主 akkoeff | 四误差 600×1 均值；rottransratio 600×1 最后一次运行值 |
| agent_in_sphere_adaptive_matched_filter_many_parameter | i=2…599，1 次运行，600 行 | 主 akkoeff 和四份 mitkoeff 均旋转 | mod(i,[1,5,11,21])==0 时复制当前主系数；比较用 oneit 返回的新系数**不写回 mitkoeff** | allemittrans/rotfehler：600×4，akrot：600×1 |

对 many_parameter 的源函数，每个比较调用接收返回形式 (dumi, mittrans, mitrot, error)，而不是把第一个返回值赋回 mitkoeff。**不能**把四条 baseline 都包装成“每帧根据求解结果自动更新系数”的同一种 Session。初始化入口则在当前帧比较循环内更新 ohne/mit 的局部系数，但后续帧重新构造。

comparison 入口在 addederror=0 时仍调用 nonpropfehler；即使噪声乘数为零，其内部仍会调用 randn，不能擅自删去随机消耗。

### 9.1 正常逐帧顺序（语义伪代码）

~~~text
for i in original MATLAB valid frames:
    scene = synthesize_nearness_and_flow(i)
    if source_entry_calls_noise: apply_noise_even_when_scale_zero(scene)
    if source_entry_rotates_coeff:
        akkoeff = rotate(akkoeff, previous_frame_rotation)
        if many_parameter: rotate(each mitkoeff, previous_frame_rotation)
    if many_parameter: refresh each mitkoeff when mod(i, period) == 0
    else: form comparison coefficients according to entry's initialization
    for every comparison: solve one step, record direction-angle errors
    primary = solve_one_step(akkoeff, scene)
    akkoeff = primary.coefficients_next [MATLAB success-path semantics]
    record source-specific outputs
~~~

C++ 从 0 开始存帧时必须显式转换为源 MATLAB 的 1-based i，周期刷新直接用 C++ 下标会导致偏移一帧。

## 10. 逐节点 Golden、统计与批准门槛

| Golden ID（计划） | 固定输入及必须保留的值 | 检查点 |
|---|---|---|
| AD-G01 | N×6、旧 3×5、near_raw/s/near_after、M1–M4、b、z、t/ω、error_mse、新 near/coeff | 单步的每个数值节点 |
| AD-G02 | near 平均 s<0 的符号/归一化 | **先翻 near，后除原 s**，不得自动改成 abs(s) |
| AD-G03 | near 平均≈0、奇异或病态 M | 原版失败/非有限值与 C++ 显式状态分别记录 |
| AD-G04 | init 帧 i=2、3 | 入口没有球谐系数旋转，比较状态每帧处理 |
| AD-G05 | comparison 的 i=2、3，addederror=0/非零 | 先上一帧旋转、再比较、再主更新；rottransratio 不取八次平均 |
| AD-G06 | many_parameter i=5、11、21 邻近帧 | 四个周期准确触发，mitkoeff 比较单步不回写 |

每个 Golden 应记录原只读 MATLAB 函数 blob SHA、**独立固定输入/L3 参考生成方式与 SHA-256**、各节点 shape/dtype、数值基准和经 DEC-005 批准的容差；**不要求运行 MATLAB 或记录 MATLAB 版本**。现有 produced_data 不含 M1…M4 的原始逐节点值，**不可把只对照最终 MAT 当作单步数学等价证据**。

**状态**：此文档定义了算法逐节点检查与入口专属状态合同；独立 L3 参考、**错误提交策略的实际实现/测试**、正式 API 及 CTest 均尚未实际形成；设计原则已明确。

## 附录 A：无需运行 MATLAB 的证据边界

按 [当前 L2/L3/L4 证据矩阵](../golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md) 和 [长期无需 MATLAB 执行政策](../golden/NO_MATLAB_EXECUTION_POLICY.md)，**不要求原 MATLAB 逐节点/逐调用的 L1，也不因没有 L1 阻塞本模块的设计/编码/测试/验收**。原 `.m` 作为只读公式、分支、时序和数据合同依据；当前应准备模块适用的 **L3 独立固定输入逐节点参考**，L2 历史 MAT 用于最终结果范围，L4 用于统计。需要保留真实 C++/CTest、DEC-005 容差、错误与来源限制记录，不能把 L3 宣称为原 MATLAB 实际运行的完整等价证据。本文及该矩阵均未宣布模块验收通过。
