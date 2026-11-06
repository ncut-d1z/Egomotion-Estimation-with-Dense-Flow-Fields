# 实球谐、近度拟合及系数旋转——详细设计草案

> **LLD v0.3，未冻结、未实现。** 来源：winkelausvektor.m、kugelflfunc2.m、near_aus_koeff.m、koeff_aus_near.m、transfor_koeff.m、SHRotate.m。第三方版权与 Golden 尚未完整核验。关联 REQ-F-SH-001/002、REQ-D-SOURCE-002、REQ-NF-COPYRIGHT-001、P7.1–P7.6/P7.10。

## 1. 坐标系、角度与原实球谐表达式

由单位方向 \(\boldsymbol d=(x,y,z)^\top\) 计算：

\[
\theta=\arccos(z),\qquad \phi=\operatorname{atan2}(y,x)
\]
（\(\theta\)：极角；\(\phi\)：方位角；单位：rad）。本公式依原 MATLAB 原样记录；对于非单位输入和 acos 越界须单独定义失败状态，不能静默改变原样本。

令 \(q=|m|\)，\(P_l^q(\cos\theta)\) 是 **MATLAB legendre(l,cos(theta)) 返回的 q 阶值**，并保留其相位/符号约定（不能直接采用另一库默认的无 Condon–Shortley 相位定义）。

\[
A_{lq}=\sqrt{\frac{2l+1}{4\pi}\frac{(l-q)!}{(l+q)!}},
\]
\[
Y_{lm}^{real}(\theta,\phi)=
\begin{cases}
\sqrt 2\,A_{lq}\,P_l^q(\cos\theta)\sin(q\phi), &m<0,\\
A_{l0}\,P_l^0(\cos\theta),&m=0,\\
\sqrt 2\,A_{lq}\,P_l^q(\cos\theta)\cos(q\phi),&m>0.
\end{cases}
\]

上式直接对应 kugelflfunc2.m 的 factorial、legendre、sin/cos 分支；未以 MATLAB 固定黄金值验证每个负 m 的符号前，不得把显式多项式实现当作已验证。

### 1.1 3×5 系数的九个有效索引

| l | m | MATLAB 的 (row,col)，1-based | C++ 0-based (row,col) | 基函数类型 |
|---:|---:|---|---|---|
| 0 | 0 | (1,1) | (0,0) | \(Y_{00}\) |
| 1 | −1 | (2,1) | (1,0) | \(\sin\phi\) |
| 1 | 0 | (2,2) | (1,1) | 无 φ 因子 |
| 1 | +1 | (2,3) | (1,2) | \(\cos\phi\) |
| 2 | −2 | (3,1) | (2,0) | \(\sin(2\phi)\) |
| 2 | −1 | (3,2) | (2,1) | \(\sin\phi\) |
| 2 | 0 | (3,3) | (2,2) | 无 φ 因子 |
| 2 | +1 | (3,4) | (2,3) | \(\cos\phi\) |
| 2 | +2 | (3,5) | (2,4) | \(\cos(2\phi)\) |

其余 3×5 存储位置不参与 l≤2 有效系数求和。原脚本初始化 (1,1)=1、其余为零，**不能将九项完整表示与八项可独立更新的非零阶信息混淆**。

## 2. 从系数到近度与从近度拟合系数

**near_aus_koeff.m** 逐点重建：

\[
\mu(\boldsymbol d_i)=\sum_{l=0}^{2}\sum_{m=-l}^{l}a_{lm}Y_{lm}^{real}(\theta_i,\phi_i).
\]

**koeff_aus_near.m** 对每个有效 (l,m) **独立计算**：

\[
a_{lm}^{num}=\frac{4\pi}{N}\sum_{i=1}^{N}\mu_iY_{lm}^{real}(\theta_i,\phi_i),\qquad
a_{lm}^{den}=\frac{4\pi}{N}\sum_{i=1}^{N}(Y_{lm}^{real}(\theta_i,\phi_i))^2,
\]
\[
a_{lm}=a_{lm}^{num}/a_{lm}^{den}.
\]

这不是把九个系数一起求解的正规方程/联合最小二乘；如果改变了基函数间的耦合关系，可能改变原始实验结果。即使两个 4π 因子可在代数上相消，也应先对照 MATLAB 原先的乘除顺序及浮点误差。

### 2.1 拟议输入输出与不变量

~~~cpp
// 仅设计草案，不是可调用的已实现 API。
using HarmonicMatrix = Eigen::Matrix<double, 3, 5, Eigen::ColMajor>;
// HarmonicCoefficients 与 HarmonicMatrix 的唯一合同见 contracts.md。
// 使用共享 NumericalErrorCode，不另建重复错误类型。
class RealSphericalHarmonics {
public:
    Result<double, NumericalErrorCode> evaluate(int l, int m,
                                            double theta_rad, double phi_rad) const;
    Result<Eigen::VectorXd, NumericalErrorCode> reconstruct(
        const HarmonicCoefficients&, const DirectionSet&) const;
};
class HarmonicFitter {
public:
    Result<HarmonicCoefficients, NumericalErrorCode> fit(
        const DirectionSet&, const Eigen::VectorXd& nearness) const;
};
~~~

冻结前需决定 DirectionSet 定义、shape 校验、N=0、基函数范数零、非有限 near、非单位方向及异常时数据保持策略。上述 Result<T,E> 是项目建议的 C++17 显式结果约定，非标准库 std::expected。

## 3. l≤2 的球谐系数旋转

**SHRotate.m** 的现有 MATLAB 输入是 3×3 旋转矩阵 r 和 degree=2，输出包含 l=0 的 1×1、l=1 的 3×3、l=2 的 5×5 块；transfor_koeff.m 使用：

\[
a'_{0,0}=a_{0,0},\quad
\boldsymbol a'_1=R_3(r)\boldsymbol a_1,\quad
\boldsymbol a'_2=R_5(r)\boldsymbol a_2.
\]

两个系数向量以 MATLAB 原存储顺序读取并写回：\((-1,0,+1)\) 与 \((-2,-1,0,+1,+2)\)。不能因为 SHRotate 的 R1 是原矩阵的一个排列，就直接把 R1 替换为通常坐标轴顺序下的 r。

### 3.1 转动块生成规则的可验证边界

- 先处理 l=0 常数块，l=1 依据源 SHRotate 的轴排列构造 R3（注意行、列和符号）。
- l=2 的 R5 使用 SHRotate 递归辅助函数 M/U/V/W/P 与上一阶旋转块生成；**第三方代码复制与重写的授权状态尚未确认**。
- 先用对 z 轴的已知旋转检查 m 正负的相位，再分别用 x/y 轴和复合旋转检查 R3/R5 全部元素、正交性、往返一致性和基函数变换方向。
- “R3/R5 元素/递推系数的逐式复刻”必须建立 Golden 表：旋转向量、输入 R、源输出 R3/R5、输入/输出系数、误差及源 SHA；在版权确认前不从第三方程序逐行照搬公式或实现。
- 对第二阶旋转的任何替代生成算法，须证明与作者 SHRotate 数值行为和系数顺序一致，并有合法的原创或兼容授权基础。

### 3.2 旋转类接口候选

~~~cpp
struct HarmonicRotationBlocks {
    Eigen::Matrix3d degree_one;
    Eigen::Matrix<double, 5, 5> degree_two;
};
class HarmonicRotator {
public:
    Result<HarmonicRotationBlocks, NumericalErrorCode> blocks(
        const Eigen::Matrix3d& rotation) const;
    Result<HarmonicCoefficients, NumericalErrorCode> rotate(
        const HarmonicCoefficients&, const Eigen::Matrix3d&) const;
};
~~~

需明确是否接受不严格正交的 r、是否允许投影回 SO(3)、奇异输入与未定义旋转的返回状态；**不应未经许可擅自修改原算法**。

## 4. 依赖、资源和可复现测试

- RealSphericalHarmonics、HarmonicFitter、HarmonicRotator 均不持有 MAT 文件句柄和跨帧轨迹状态；Adaptive Session 独占系数历史。
- 固定球面方向数组 Golden：所有 9 个 l,m 配对值、均匀/非均匀采样下拟合的系数、旋转前后近度差异。
- 多组旋转 Golden：单位 R、绕 x/y/z 不同角度、复合旋转和逆变换；在指定系数顺序下逐元素检查 R3、R5 的全部 34 个块元素。
- 测试必须绑定源脚本 SHA、MATLAB 参考输出和经 DEC-005 批准的数值容差；原数据/Golden 若无法合法提交可使用受控外部存储与可审查哈希。

## 5. 冻结与授权阻塞

- 仍需对源关联勒让德相位、SHRotate 块递推、真实黄金样本做独立核验。
- 第三方 SHRotate.m、rodrigues.m 的代码来源与权利范围待 DEC-003/007；不能仅凭资料页数据库许可推断这些代码可直接复制。
- C++17 精确签名、错误枚举、异常行为、目标 CMake target 和测试用例未冻结。

**本文件是详细设计草案；没有 C++ 实现、MATLAB 运行或数值验收证据。**


## 6. l≤2 九个实球谐分量的显式公式（待 MATLAB Golden 核符号）

取归一化方向 \(x=\sin\theta\cos\phi\)、\(y=\sin\theta\sin\phi\)、\(z=\cos\theta\)，并**按原 MATLAB 的 legendre 默认相位**（含 Condon–Shortley 相位）展开。下列是从 §1 通式得到的数学等价表示，只有经过源 MATLAB 数值检查才能成为冻结的公式 Golden：

| (l,m) | 按源代码约定的 \(Y_{lm}^{real}\) |
|---|---|
| (0,0) | \(1/\sqrt{4\pi}\) |
| (1,−1) | \(-\sqrt{3/(4\pi)}\,y\) |
| (1,0) | \(+\sqrt{3/(4\pi)}\,z\) |
| (1,+1) | \(-\sqrt{3/(4\pi)}\,x\) |
| (2,−2) | \(+\sqrt{15/(4\pi)}\,xy\) |
| (2,−1) | \(-\sqrt{15/(4\pi)}\,yz\) |
| (2,0) | \(+\sqrt{5/(16\pi)}(3z^2-1)\) |
| (2,+1) | \(-\sqrt{15/(4\pi)}\,xz\) |
| (2,+2) | \(+\sqrt{15/(16\pi)}(x^2-y^2)\) |

**注意**：MATLAB 的函数调用约定含 φ 的 sin/cos 正负号，不可用默认的量子力学 \(Y_l^m\) 次序替换。测试应包含六个坐标轴方向、一般方向 \((1,2,3)/\sqrt{14}\) 和方位角正负边界，逐项核对九个结果及正交/符号关系。

## 7. SHRotate 的旋转块元素与递推生成合同

### 7.1 第 1 阶 R3 的**实际位置映射**

令输入物理旋转矩阵为 \(r=[r_{ij}]_{3\times3}\)，使用 MATLAB 1-based 的 \(r_{ij}\) 表示原始元素。根据 SHRotate.m 对 R{2} 的九处赋值：

\[
R_3(r)=
\begin{bmatrix}
r_{22}&r_{32}&r_{12}\\
r_{23}&r_{33}&r_{13}\\
r_{21}&r_{31}&r_{11}
\end{bmatrix}.
\]

旋转系数按原顺序 \((m=-1,0,1)\) 组成列向量，再执行 \(a'_1=R_3a_1\)。例如输入
\(r=R_z(\gamma)=\left[\begin{smallmatrix}\cos\gamma&-\sin\gamma&0\\\sin\gamma&\cos\gamma&0\\0&0&1\end{smallmatrix}\right]\)，得到

\[
R_3(R_z(\gamma))=
\begin{bmatrix}\cos\gamma&0&-\sin\gamma\\0&1&0\\\sin\gamma&0&\cos\gamma\end{bmatrix}.
\]

这是一组**可由源码逐元素交叉检查**的预期矩阵，不能未经说明交换 R3 的行/列。

### 7.2 第 2 阶 R5 的递推合同

源 SHRotate.m 对 degree=2 循环 \(m,n\in\{-2,-1,0,1,2\}\)，以先构造的 \(R_3\) 与下一阶递推函数生成所有 25 个 \(R_5(m,n)\)。

\[
R_5(m,n)=u_{2mn}U_{2mn}+v_{2mn}V_{2mn}+w_{2mn}W_{2mn}.
\]

其中 \(D_{ln}=2l(2l-1)\)（当 \(|n|=l\)），否则 \(D_{ln}=l^2-n^2\)；\(d_m=1\) 当 \(m=0\)，否则 \(0\)，相应系数：

\[
u=\sqrt{(l^2-m^2)/D_{ln}},\quad
v=\frac{1-2d_m}{2}\sqrt{\frac{(1+d_m)(l+|m|-1)(l+|m|)}{D_{ln}}},
\]
\[
w=-\frac{1-d_m}{2}\sqrt{\frac{(l-|m|-1)(l-|m|)}{D_{ln}}}.
\]

U/V/W 在不同 m 与 n 边界上组合上一阶 \(R_3\) 条目与递推辅助 P；其**完整分段定义**仍须在 P7.5 审阅源 SHRotate 的第三方版权并建立允许使用的数学描述之后冻结。不能仅把这三行权重当作已经足以实现 R5。验收需对**全部 25 个元素**及其索引和符号提供源 MATLAB Golden，不能只检查正交性或往返。

### 7.3 旋转样本与方向性判定

| 案例 | 预期数学性质 | 需要的源 Golden |
|---|---|---|
| SH-R00 | r=I 时 R3=I3、R5=I5 | 34 个元素，源 SHA |
| SH-RZ | 物理 r 绕 z 轴旋转小角及 90° | 输入 r、R3、R5、九个系数 before/after |
| SH-RXY | 绕 x/y 轴和两轴复合旋转 | 各矩阵 34 元素、向量范数、近度值 |
| SH-INV | 先 r 后 r⁻¹ | 九个系数和方向采样下近度变化 |
| SH-BAD | 非正交或非有限输入 | 原函数实际结果及 C++ 明确状态 |

**授权状态**：只可以在合法来源和已审核实现范围内实现算法；SHRotate.m 和 rodrigues.m 的第三方文件头仍是版权审核项目。无需复制原程序的代码文本即可描述这些数学关系，但不能用数学等价的猜测代替实测。

## 8. 异常、生命周期与接口合同

- 输入 l∈{0,1,2} 且 |m|≤l；方向有限、单位化规则经批准；输入 3×5 系数的非有效位置不应参与计算。
- 近度拟合遇 \(b_{lm}=0\) 时明确失败；数学输入为 NaN、Inf、零向量或非正交 r 时须输出具体 NumericalErrorCode/细节，不得伪造通过。
- RealSphericalHarmonics、HarmonicFitter、HarmonicRotator 可为无跨帧可变状态服务；球谐系数是自拥有值对象，由 Session 管理历史。
- 正式的 Result、HarmonicCoefficients、NumericalErrorCode 以 [contracts.md](contracts.md) 为单一候选；具体 API 签名和 const/复制行为由 P0.8 确认。
- TC-F-SH-001、TC-F-SH-002、TC-D-SOURCE-002 均需记录 MATLAB 来源函数 SHA、固定输入/输出、逐元素 atol/rtol、条件数和未定义样本数；DEC-003/005/007 未关闭时不得冻结。

**本模块的已知内容：9 项索引与数学公式、R3 九项映射、R5 的递推结构；缺口：R5 分段递推合法实现、全量 Golden、批准容差与真实代码/CTest。**
