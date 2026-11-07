# 轨迹、相机旋转与球内环境——详细设计草案

> **未冻结、未运行数值测试。** 来源：kreis_trajektorie.m、sinuskurve_trajektorie.m、trajektorie_aus_punkten.m、drehvektor_aus_zwei_vektoren.m、rodrigues.m、getnear_in_kugel_Drehung.m。对应 P3.6、P6.1–P6.7、REQ-F-TRAJ-001。

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

原始函数初始化数组为 N×3，首尾行未在主循环填写，C++ 不能擅自用外推补齐而声称原版等价。接近共线、反向或零方向的行为必须记录原 MATLAB 结果与拟议数值状态。

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
| v1≈v2 | 叉积接近零、γ≈0 | 原 1e−11 分支与 acos 数值边界要 Golden |
| **v1 与 v2 反向** | 叉积零但 γ≈π，原函数可能输出**零旋转向量而非有效 π 旋转** | 必须保留/标注原版行为，不能悄悄替换任意反向旋转轴 |
| 任一输入为零 | γ 的分母可为零 | 显式 NumericalErrorCode，同时记录原版 NaN/报错；未授权不称“等价” |
| dot/(norms) 超出 [-1,1] 浮点范围 | acos 非有限或复数 | 是否显式 clamp 为安全增强待决定，忠实模式不能未经同意更改 |

rodrigues.m 的实际主要调用方向是 **3×1 旋转向量→3×3 旋转矩阵**；原函数在角度小于 MATLAB double 的 `eps` 时返回单位阵，完整 MATLAB 文件还包含矩阵→向量、SO(3) 投影与 Jacobian 功能。**H-02 已选定 Sophus**，不复刻未调用的辅助分支；仍须确认所有实际调用方是否使用了这些分支及返回 Jacobian，不能因库 API 类似而宣布完全等价。

### 5.1 经 H-02 指定的 Sophus 适配方案（仅设计，未实施）

- **依赖来源**：[strasdat/Sophus](https://github.com/strasdat/Sophus)，[sophus/so3.hpp](https://github.com/strasdat/Sophus/blob/main/sophus/so3.hpp)，上游 `LICENSE.txt` 为许可参考；只使用 SO(3) API，不引入 ROS 运行依赖。计划于将来的 P1.1/P6.2 获授权实施时锁定版本和 CMake 查找方式；本轮不安装库。
- **数学调用**：给定原旋转向量 `omega = (ω_x, ω_y, ω_z)^T`（rad），候选计算为 `Sophus::SO3d::exp(omega).matrix()`，再返回自拥有的 `Eigen::Matrix3d`，不将 Sophus 类型泄露给其他业务层。
- **原版边界保留**：为了对齐 `rodrigues.m`，当 `omega.norm() < std::numeric_limits<double>::epsilon()` 时须**优先按原函数返回单位阵**，再考虑调用 `Sophus::SO3d::exp`；阈值是**原 MATLAB 已存在条件**，不是新数值容差。输入非有限/异常的返回类型仍遵循 [contracts.md](contracts.md) 的未冻结错误候选。
- **坐标/调用方向**：原 `trajektorie_aus_punkten` 使用 `Rᵀ` 把世界向量转换为局部运动；不能因为 Sophus 具有群操作就改变转置、轴角方向和单位。
- **验收设计**：固定 `ω=0`、小于 MATLAB eps、x/y/z 轴已知角、复合转动、接近 π、原反向向量路径；对比原 MATLAB 的 `R` 九元素、`Rᵀ` 下的局部 t/ω；若未来确实需要矩阵→向量或 Jacobian，再另列 TC，不扩大 H-02 所批准范围。
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
- 是否把近零角度强制置零、非法输入直接拒绝、是否支持球外位置，均属于 LLD 审批决定；禁止伪装成原 MATLAB 已经处理了这些异常。

## 7. 黄金样本与验收覆盖

| 黄金 ID（待创建） | 源数据与输入 | 固定预期 |
|---|---|---|
| TR-G01 | 35 点圆，给定半径/高度 | 每个 q_i 的 x/y/z；首尾与非重复端点规则 |
| TR-G02 | 600 点正弦、幅度 0.5、长度 1、高度 0.3、周期 2 | 每个 q_i；分母必须是 600，非 599 |
| TR-G03 | 相邻三点的非退化 v1/v2 | 绝对/相对旋转、Rᵀ 局部平移 |
| TR-G04 | 同向、反向、零向量 | 源函数的零轴/NaN 现象以及 C++ 失败分类 |
| TR-G05 | 球心、近边界位置与一般旋转 | 射线长度/near、判别式与有效性 |
| TR-G06 | Rodrigues 零向量和已知 x/y/z 轴角 | R 矩阵、正交性和旋转方向 |

关联 TC-F-TRAJ-001、TC-F-FLOW-004、TC-D-SOURCE-002；每个 Golden 必须记录源 SHA、MATLAB 实际数据、绝对/相对容差及版本。**当前只定义案例，没有固定数值或实验运行证据。**

## 附录 A：轨迹及旋转的来源等级（补充草案）

[统一 L1–L4 清单](../golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md) 说明：轨迹解析式与当前 `AN-G08` 的 3×3 旋转是 **L3 解析候选**，历史最终误差 MAT 是 **L2**。两者均不包含原 `trajektorie_aus_punkten` 在每一帧实际生成的 t/ω、`R^T` 局部坐标、端点 zeros、反向/近共线异常、`rodrigues.m` 在小角与边界的实际执行轨迹；这些 L1 仍缺失。

`AC-F-TRAJ-001` 的解析路径可先设计有限固定输入检验，但当前没有真实生产 C++、独立 MATLAB 采集或 CTest，不能以 L3 的符号/公式推导宣称原 MATLAB 全部行为等价。Sophus 路线的选择不改变 L1 取证需求。
