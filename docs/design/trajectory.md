# 轨迹、相机旋转与球内环境——详细设计草案

> **LLD v0.5 草案，未冻结、未运行原生数值测试。** 来源：kreis_trajektorie.m、sinuskurve_trajektorie.m、trajektorie_aus_punkten.m、drehvektor_aus_zwei_vektoren.m、rodrigues.m、getnear_in_kugel_Drehung.m。对应 P3.6、P6.1–P6.7、REQ-F-TRAJ-001。

## 1. 轨迹点列与离散化

令 MATLAB 的 i=1…stellen，对应 C++ 0-based j=i−1。

**圆轨迹**（半径 r=parameter·kugelradius，源半径为 1）：

\[
w_j=2\pi j/\mathrm{stellen},\quad
\boldsymbol q_j=(r\cos w_j,r\sin w_j,\mathrm{hoehe}).
\]

**正弦轨迹**：

\[
w_j=2\pi j\,\mathrm{anzahlperioden}/\mathrm{stellen},
\quad
q_x=\mathrm{laenge}\,j/\mathrm{stellen}-\mathrm{laenge}/2,
\quad q_y=\mathrm{amplithude}\sin w_j,\quad q_z=\mathrm{hoehe}.
\]

注意分母是 **stellen**，不是 stellen−1，不能自行补上“重复的末点”。原函数绘图 line() 属于显示副作用，C++ 轨迹模块必须只返回点列。

## 2. 轨迹到局部运动

对应 trajektorie_aus_punkten.m 的有效索引 i=2…N−1：

\[
\boldsymbol v_1=\boldsymbol q_i-\boldsymbol q_{i-1},\qquad
\boldsymbol v_2=\boldsymbol q_{i+1}-\boldsymbol q_i.
\]

相对旋转向量由 drehvektor_aus_zwei_vektoren(v1,v2) 得到；绝对朝向旋转向量由起始前向 (1,0,0)^T 到 v1 得到。以该朝向的 Rodrigues 旋转矩阵 R，将世界平移/相对旋转转换到原脚本的局部坐标：
\[
\boldsymbol t_i=R^\top\boldsymbol v_2,\quad
\boldsymbol\omega_i^{rel}=R^\top\mathrm{rotationVectorBetween}(\boldsymbol v_1,\boldsymbol v_2).
\]

原始函数初始化数组为 N×3，首尾行未在主循环填写，C++ 不能擅自用外推补齐而声称原版等价。接近共线、反向或零方向的行为必须依据只读原源码静态逻辑和独立 L3 参考记录可验证结果与拟议 C++ 状态（未知运行行为注明限制）。

## 3. 单位球内射线近度

将视线 d 通过 Rodrigues 转动得到 n，位置 o 在单位球内。源码：
\[
b=\boldsymbol n^\top\boldsymbol o,\quad
c=\boldsymbol o^\top\boldsymbol o,\quad
\Delta=b^2-c+1,\quad
\rho=-b+\sqrt\Delta,\quad \mu=1/\rho.
\]
测试 o=0、接近球面、反向视线和非法球外位置；不能静默返回非有限 near。

## 4. 候选类型、失败及测试

~~~cpp
// 候选签名；§6.1 定义 TrajectoryPoints / FrameKinematics 值对象。
struct TrajectoryPoints;
struct FrameKinematics;
class TrajectoryGenerator {
public:
    Result<TrajectoryPoints, NumericalErrorCode> circle(
        std::size_t count, Scalar radius, Scalar height) const;
    Result<TrajectoryPoints, NumericalErrorCode> sinusoidal(
        std::size_t count, Scalar amplitude, Scalar length,
        std::size_t periods, Scalar height) const;
};
class TrajectoryKinematics {
public:
    Result<std::vector<FrameKinematics>, NumericalErrorCode>
        fromPoints(const TrajectoryPoints&) const;
};
~~~

责任边界：TrajectoryGenerator 不计算 MAT 数据或绘图；TrajectoryKinematics 不管理 Adaptive Session。**依据用户审批 JSON H-02，后续以 Sophus SO(3) 接口替代原 `rodrigues.m` 的实际旋转向量→矩阵调用，不再计划逐行复制 Perona/Bouguet MATLAB 代码。** 当前尚未引入 Sophus 或运行 C++，对应依赖与权利说明见 [审批落实记录](../approval/DECISION_IMPLEMENTATION_NOTES.md)。

TC-F-TRAJ-001：固定参数点列、局部 t/ω、首尾行为、奇异方向状态对照；实际 Golden 数值、精确签名与容差待审定。


## 5. 旋转向量构造的原版奇异行为

原 drehvektor_aus_zwei_vektoren.m 的核心合同：

\[
\boldsymbol c=\boldsymbol v_1\times\boldsymbol v_2,\quad
n_c=\|\boldsymbol c\|,\quad
\gamma=\arccos\frac{\boldsymbol v_1^\top\boldsymbol v_2}{\|\boldsymbol v_1\|\|\boldsymbol v_2\|}.
\]
\[
\boldsymbol a=\begin{cases}\boldsymbol c/n_c,&n_c>10^{-11},\\\boldsymbol c,&n_c\le 10^{-11},\end{cases}
\qquad \boldsymbol r=\gamma\boldsymbol a.
\]

这产生三种必须区分的情形：

| 几何条件 | MATLAB 潜在结果 | C++ 候选处理/回归 |
|---|---|---|
| v1、v2 正常非平行 | 单位旋转轴×角度 | 对照每个分量、rotvec 范数和 Rodrigues 方向 |
| v1≈v2 | 叉积可能接近零、γ≈0；**原判别量是未归一化叉积范数，不是角度/单位方向之差** | 原 1e−11 分支与 acos 数值边界要 Golden，长度改变也可能触发不同分支 |
| **非零 v1 与 v2 严格反向** | 对精确反向输入叉积**必为 0**、γ=acos(−1)=π，因源代码 `drehvektor=0*π` **确定返回零旋转向量**，不是有效 π 旋转；近反向但叉积非零另受 1e−11 分支影响 | 不得把此源返回当“真实零旋转”的有效估计；安全的 `InvalidDirection` 失败是候选 C++ 行为，需按已选 Result/Failure 与真实测试确认 |
| 任一输入为零 | γ 的分母可为零 | 显式 NumericalErrorCode，同时记录原版 NaN/报错；未授权不称“等价” |
| dot/(norms) 超出 [-1,1] 浮点范围 | acos 非有限或复数 | 是否显式 clamp 为安全增强待决定，忠实模式不能未经同意更改 |

**新确认的尺度依赖退化（2026-11-11，原源码静态复核）**：第5节的 `normfaktor3` 为 **`norm(cross(v1,v2)) = norm(v1)*norm(v2)*abs(sin(gamma))`**，`if normfaktor3>1e-11` 的比较发生在叉积**未归一化**之前。这不是“只有两向量近共线才退化”的角度检测。完全垂直的两方向也会因任一向量长度足够小而走原始未单位化轴分支，使 `drehvektor = gamma*cross(v1,v2)` 与实际夹角的幅度严重不符。不能在忠实原版合同中无说明地将它换成单位向量叉积、直接调用 Sophus `SO3::log`、或者将原始旋转幅度当作可信运动。未来可选的**几何安全模式**应单独审批是否报 `InvalidDirection`、拒绝近零输入还是采用归一化旋转；源保真和安全增强必须明确分列。

| 手算输入（全为非零、夹角均为90°） | 原判别/原源精确符号输出 | 核验含义 |
|---|---|---|
| `v1=e_x, v2=1e-11*e_y` | `cross=1e-11*e_z`，与源码阈值相等，**严格 > 不成立**；`gamma=pi/2`，返回 `(pi/2)*1e-11*e_z` | 输出幅度几乎为零，**不是**90°旋转；阈值是绝对量纲而非角度 |
| `v1=e_x, v2=2e-11*e_y` | `cross=2e-11*e_z`，**严格 > 成立**；轴单位化为 `e_z`，返回 `(pi/2)*e_z` | 两例几何方向相同，仅位移长度改变就跨越巨大数值跳变 |
| `v1=1e-6*e_x, v2=1e-6*e_y` | `cross=1e-12*e_z`；`gamma=pi/2`，返回 `(pi/2)*1e-12*e_z` | 即使两个速度模长相等，低模长仍导致“几乎零旋转”错态 |

以上结果是以源码比较和 `acos(0)=pi/2` 做出的**数学/静态预测**，数值浮点的边界恰等比较要在实际 C++/数值环境复核；未执行 MATLAB/原生 CTest。特别是 `trajektorie_aus_punkten.m` 的内部 `v1/v2` 来自相邻点差分，随着步长变小，绝对叉积可按步长**平方量级**下降，故并非罕见的抽象极端向量情形。关联原 `TC-F-TRAJ-001`、`TR-G04` 与 §6.3 的 `TR-C05`，不新建正式测试编号。

rodrigues.m 的实际主要调用方向是 **3×1 旋转向量→3×3 旋转矩阵**；原函数在角度小于 MATLAB double 的 `eps` 时返回单位阵，完整 MATLAB 文件还包含矩阵→向量、SO(3) 投影与 Jacobian 功能。**H-02 已选定 Sophus**，不复刻未调用的辅助分支；仍须确认所有实际调用方是否使用了这些分支及返回 Jacobian，不能因库 API 类似而宣布完全等价。

### 5.1 经 H-02 指定的 Sophus 适配方案（仅设计，未实施）

- **依赖来源**：[strasdat/Sophus](https://github.com/strasdat/Sophus)，[sophus/so3.hpp](https://github.com/strasdat/Sophus/blob/main/sophus/so3.hpp)，上游 `LICENSE.txt` 为许可参考；只使用 SO(3) API，不引入 ROS 运行依赖。计划于将来的 P1.1/P6.2 获授权实施时锁定版本和 CMake 查找方式；本轮不安装库。
- **数学调用**：给定原旋转向量 `omega = (ω_x, ω_y, ω_z)^T`（rad），候选计算为 `Sophus::SO3d::exp(omega).matrix()`，再返回自拥有的 `Eigen::Matrix3d`，不将 Sophus 类型泄露给其他业务层。
- **原版边界保留**：为了对齐 `rodrigues.m`，当 `omega.norm() < std::numeric_limits<double>::epsilon()` 时须**优先按原函数返回单位阵**，再考虑调用 `Sophus::SO3d::exp`；阈值是**原 MATLAB 已存在条件**，不是新数值容差。输入非有限/异常的返回类型仍遵循 [contracts.md](contracts.md) 的未冻结错误候选。
- **坐标/调用方向**：原 `trajektorie_aus_punkten` 使用 `Rᵀ` 把世界向量转换为局部运动；不能因为 Sophus 具有群操作就改变转置、轴角方向和单位。
- **验收设计**：固定 `ω=0`、小于 MATLAB eps、x/y/z 轴已知角、复合转动、接近 π、原反向向量路径；对比独立 L3 解析的 `R` 九元素、`Rᵀ` 下的局部 t/ω（不执行原 MATLAB）；若未来确实需要矩阵→向量或 Jacobian，再另列 TC，不扩大 H-02 所批准范围。
- **状态**：**方案选择已确认，代码/CTest/Golden 尚未运行**。H-10 暂缓生产实施、H-01/DEC-005 容差待定；H-02 不等于数值等价签署。

## 6. 位姿与球内近度的边界约定

| 字段 | 源 MATLAB 形状与坐标 | 约束 |
|---|---|---|
| trajectory points | N×3，世界中相机位置 | 按索引 i=1…N 存点，圆/正弦参数不能混用 |
| alletrans | N×3，局部平移 | i=2…N−1 根据 Rᵀ 和 v2 计算；首末行原为零 |
| absdrehvektoren | N×3，世界前向参考至当前的绝对旋转向量 | 保留原 (1,0,0) 参考向量与角度方向 |
| reldrehvektoren | N×3，局部相对旋转向量 | 用 Rᵀ 变换由 v1→v2 的旋转向量 |
| near | N×1、单位球内相机向球面各视线的距离倒数 | 只对处在单位球内的位置与有效正根定义 |

球内距离公式使用已**旋转后的视线** n=R(drehvektor)d；对单位球与 \(\|\boldsymbol o\|<1\) 的假设，判别式 \(\Delta=b^2-\|\boldsymbol o\|^2+1\) 严格非负。由于浮点误差或球外位置发生负判别式时，须返回可识别错误而非把 sqrt(NaN) 填成有效近度。

### 6.1 拟议 C++17 生命周期与返回语义

~~~cpp
// 设计合同，不是已实现代码。
struct TrajectoryPoints { Eigen::Matrix<Scalar, Eigen::Dynamic, 3> points; };
struct FrameKinematics {
    Vector3 local_translation;
    Vector3 absolute_rotation;
    Vector3 relative_rotation;
    bool has_original_matlab_motion;  // i=2..N-1 为 true
};
class SphereEnvironment {
public:
    Result<Eigen::VectorXd, NumericalErrorCode> nearnessInUnitSphere(
        const DirectionSet& directions,
        const Vector3& camera_position,
        const Vector3& absolute_rotation_vector) const;
};
~~~

- 值对象不借用外部 MAT 的短生命周期内存。TrajectoryGenerator 不绘图，不修改 Session，也不产生全局随机状态。
- 对不能形成前后两段位移的端点，返回原来的预分配零值，同时标注没有有效运动；不能用“端点真值零”掩盖其非估计状态。
- **原始小角合同已确定**：对 `rodrigues.m` 的旋转向量→矩阵分支，原源码在 `theta < eps` 时返回**单位旋转矩阵**；§5.1 规划 `omega.norm() < std::numeric_limits<double>::epsilon()` 优先返回单位阵，否则采用已选定的 Sophus `SO3d::exp`。这不是将任意“近零角度”强制置零的新增可选策略，也不等于 `drehvektor_aus_zwei_vektoren.m` 的叉积范数 **1e−11** 判断门槛。**是否扩大原 eps 阈值、对非法/非有限输入如何显式失败、是否支持球外位置**仍须依据源代码与独立 L3/异常测试审查；不得把未知 MATLAB 行为写成已验证实现。

### 6.2 第一轮轨迹—采样—光流边界用例（AF-048，设计补齐）

| 场景 | 明确接口/数值语义 | 待独立核验的条件 |
|---|---|---|
| 圆轨迹 35 点、正弦轨迹 600 点 | `TrajectoryGenerator` 仅输出世界坐标的 N×3 点；原源以 **N** 而非 `N−1` 作为参数分母，首尾按源预分配规则 | 逐行 L3 轨迹、首末点与对偶差分的真实 C++ 对照 |
| 第 1 和第 N 帧 | 原 `alletrans` 预分配的端点为零，`has_original_matlab_motion=false`；不得作为“真实零运动”的有效测量 | 端点状态/误差分母的 Session/Runner 验证 |
| 零旋转与 `norm(omega) < eps` | 仅对**旋转向量→旋转矩阵**分支优先返回单位阵；否则应用 Sophus `SO3d::exp`，不扩大 eps 门槛 | 单位阵/三轴/近 π/组合矩阵的独立 L3 全元素参考 |
| 反向、近同向及零长度方向 | 源 `drehvektor_aus_zwei_vektoren` 的叉积 `1e−11` 条件与 Rodrigues eps **不是同一阈值**；非定义结果应按已选 Result/Failure 报告而非伪造有效旋转 | 向量方向、符号、非有限/零模负例及源无法实测的 MATLAB 行为分列 |
| 单位球内相机位置、射线指向球面 | `SphereEnvironment` 消费 `DirectionSet` 与自拥有轨迹/旋转输入，先按既定 R 旋转方向、取**正向射线**距离的正根再倒数得到 near；不在此层读取 MAT 或生成随机数 | 球心/近边界/负判别式或球外失败分类的独立参考 |
| 世界运动转局部运动 | `TrajectoryKinematics` 使用原 `R^\top` 对向量变换；不向 `sampling` 模块输出 MAT 句柄/Session 状态 | 矩阵转置方向与平移、旋转量的固定输入 L3 |

本合同明确 **TrajectoryGenerator/TrajectoryKinematics/SphereEnvironment** 与采样 `DirectionSet`、流场 `FlowSynthesizer` 的值传递职责；具体公共签名与 target 以 [contracts.md](contracts.md)、[sampling_noise.md](sampling_noise.md) 及 HLD 为准。关联 `TC-F-TRAJ-001`、`TC-F-FLOW-004`。**AF-048 仍「未整改」**：Sophus 生产依赖、近退化 L3、异常/CTest 及最终冻结尚缺。

### 6.3 可手算的固定输入与源退化核对（AF-080；非已生成 Golden）

以下逐项来自只读 `drehvektor_aus_zwei_vektoren.m:5–16`、`trajektorie_aus_punkten.m:9–30`、`getnear_in_kugel_Drehung.m:11–20`；分数写法用于保持符号与坐标方向可审查。**均是静态公式推导的候选，不是运行过的 MATLAB 或独立 L3/CTest PASS。**

| 候选 | 输入及源定义 | 手算必得的结果／未来需检查项 |
|---|---|---|
| TR-C01：局部转置方向 | 三点 `q1=(0,-1/5,0)`，`q2=(0,0,0)`，`q3=(-1/5,0,0)`；原 `vabs=e_x` | `v1=(0,1/5,0)`、`v2=(-1/5,0,0)`；`v1×v2=(0,0,1/25)`，两条旋转向量 `absrot=(0,0,π/2)`、`absdrehvektoren(2)=(0,0,π/2)`；`R=R_z(π/2)`，`Rᵀv2=(0,1/5,0)`，`Rᵀabsrot=(0,0,π/2)`。如果误用 `R v2`，平移会变为 `(0,-1/5,0)`，可区分转置方向。首尾行原 zeros 预分配不代表有效零运动。 |
| TR-C02：严格反向异常 | `v1=(1/5,0,0)`、`v2=(-1/5,0,0)`；可取 `q1=(-1/5,0,0), q2=(0,0,0), q3=(-1/5,0,0)` | `v1×v2=(0,0,0)` 且 `cosγ=-1`，`γ=π`；原函数**确定返回零向量**。由于由 `e_x` 到 `v1` 的绝对 rotvec 亦为零，原路径在中间行给 `R=I`、`alletrans=(-1/5,0,0)`、`reldrehvektoren=(0,0,0)`；但真实折返非零旋转，不能冒充正常输出。C++ 的显式失败类别/调用方继续策略仍须独立批准与测试。 |
| TR-C03：旋转后的射线近度 | 单位视线 `d=(0,1,0)`，旋转向量 `ω=(0,0,-π/2)`，相机位置 `o=(1/4,0,0)` | `n=R_z(-π/2)d=(1,0,0)`，`b=1/4`、`c=1/16`、`Δ=1`，正向射线距离 `ρ=3/4`，`near=4/3`；若遗漏视线旋转，误得 `near=4/√15`。 |
| TR-C04：反向射线根号 | 单位视线 `d=(-1,0,0)`，无旋转、`o=(1/4,0,0)` | `b=-1/4,c=1/16,Δ=1`，`ρ=5/4`，`near=4/5`；不能偷换成 `+b+√Δ` 或负根。 |
| TR-C05：绝对叉积阈值尺度断层 | `v1=e_x`；分别取 `v2=1e-11 e_y` 和 `v2=2e-11 e_y`；夹角同为 `π/2` | 原函数严格 `>1e-11` 而不是 `>=`，两组候选旋转向量分别是 `(π/2)·1e-11 e_z` 与 `(π/2) e_z`，**不能把前者解释为几何上几乎无旋转**；真实 double 边界与独立安全策略待验证。 |

**验收身份**：这些条件分别补充原 `TR-G03/04/05`（含 TR-C05）、`TC-F-TRAJ-001`、`TC-F-FLOW-004` 的输入资格，不新增 TC/G ID。实际 L3 必须有独立来源、完整输入/输出/状态、SHA-256、批准容差与真实代码/CTest 对照，且至少另测零方向、非有限、球边界与一般旋转；源 MATLAB 的退化返回与 C++ 安全失败应分别记录。AF-048 仍「未整改」，AF-080 仅以**纠正明确的静态源码语义与输入候选**为整改范围。

## 7. 黄金样本与验收覆盖

| 黄金 ID（待创建） | 源数据与输入 | 固定预期 |
|---|---|---|
| TR-G01 | 35 点圆，给定半径/高度 | 每个 q_i 的 x/y/z；首尾与非重复端点规则 |
| TR-G02 | 600 点正弦、幅度 0.5、长度 1、高度 0.3、周期 2 | 每个 q_i；分母必须是 600，非 599 |
| TR-G03 | 相邻三点的非退化 v1/v2 | 绝对/相对旋转、Rᵀ 局部平移 |
| TR-G04 | 同向、反向、零向量 | 源函数的零轴/NaN 现象以及 C++ 失败分类 |
| TR-G05 | 球心、近边界位置与一般旋转 | 射线长度/near、判别式与有效性 |
| TR-G06 | Rodrigues 零向量和已知 x/y/z 轴角 | R 矩阵、正交性和旋转方向 |

关联 TC-F-TRAJ-001、TC-F-FLOW-004、TC-D-SOURCE-002；每个 Golden 必须记录只读原源 blob SHA、独立 L3 参考值/来源和 SHA-256、绝对/相对容差及版本。**当前只定义案例，没有固定数值或实验运行证据。**

## 附录 A：无需 MATLAB 运行的证据边界

遵守 [长期无需 MATLAB 执行政策](../golden/NO_MATLAB_EXECUTION_POLICY.md) 与 [L2/L3/L4 验收矩阵](../golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md)。**本模块不要求运行 MATLAB/Octave、取得 L1 逐节点输出或追溯原脚本运行异常**；现行独立参考需覆盖原静态数学公式、旋转/坐标/相位、固定输入、源 SHA 及全部有定义元素，并经 DEC-005 容差核准及真实 C++/CTest 比较。无法确定的原 MATLAB 特定运行结果须明确限制声称范围，不能冒充原脚本已实测。当前尚无正式实现/验收证据。
