# 实球谐、近度拟合及系数旋转——详细设计草案

> **LLD v0.7，未冻结、未实现。** 来源：winkelausvektor.m、kugelflfunc2.m、near_aus_koeff.m、koeff_aus_near.m、transfor_koeff.m、SHRotate.m。第三方许可/依赖与独立 L3 参考尚未完整核验。关联 REQ-F-SH-001/002、REQ-D-SOURCE-002、REQ-NF-COPYRIGHT-001、P7.1–P7.6/P7.10。

**项目所有者确定的实现路线**：[Ivanic–Ruedenberg 原始论文（1996）及勘误（1998）的纯 Eigen 实施方案](SH_ROTATION_IVANIC_RUEDENBERG.md) 已列明。实现的数学依据以**用户稍后上传的原论文和 1998 年勘误**为准，Don Williamson 源码只作参考；**完全禁止 sdla 依赖（直接和间接）**。仓库新代码 MIT，正常开发无需另外版权审核签字。论文上传前仅完善设计，不实施 R5 生产递推。

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

上式直接对应 kugelflfunc2.m 的 factorial、legendre、sin/cos 分支；未以独立 L3 固定方向参考验证每个负 m 的符号前，不得把显式多项式实现当作已验证。

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

- `RealSphericalHarmonics::evaluate(int degree, int order, double theta_rad, double phi_rad) const`：有限角输入、l≤2 和 `-l≤m≤l`；错误使用公共 `Result<double, NumericalErrorCode>`。
- `RealSphericalHarmonics::reconstruct(const HarmonicCoefficients&, const DirectionSet&) const`：依据 3×5 的九个有效项重建逐方向 near。
- `HarmonicFitter::fit(const DirectionSet&, const Eigen::VectorXd& nearness) const`：根据逐项投影/归一构建 3×5 系数，不引入联合最小二乘。
- `RealSphericalHarmonics::validDegreeOrder(int,int) noexcept`：为输入检查准备的**静态函数声明**，不预先规定具体数值计算实现。

**这些接口的拟议头文件、命名空间与唯一 C++17 声明已集中在 [球谐专题 LLD §3](SH_ROTATION_IVANIC_RUEDENBERG.md)**，本节不再维护重复 C++ 定义。上述签名只用于设计，生产头文件尚未创建。


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
- l=2 的 R5 必须以 Ivanic–Ruedenberg **1996 原文与 1998 勘误**为首要数学依据，在**Eigen** 上重写递推；Don Williamson `IvanicSHRotate` 仅作代码参考，**不得引入 sdla 头、库、源文件或间接依赖**。用户将上传论文后才启动 R5 生产实现，本轮不复制/移植第三方代码。
- 先用对 z 轴的已知旋转检查 m 正负的相位，再分别用 x/y 轴和复合旋转检查 R3/R5 全部元素、正交性、往返一致性和基函数变换方向。
- 以 **Ivanic–Ruedenberg 1996 原论文及 1998 勘误**为数学依据，规划纯 Eigen 的 R5 递推；Don Williamson 的 C++ 代码仅作为交叉阅读参考，必须建立独立 L3 参考表：旋转向量、输入 R、**独立推导/复核** R3/R5 全元素、输入/输出系数、误差及只读源 SHA；不得以被测 C++ 自验代替独立参考，**无需运行 MATLAB**，也不得在本轮复制第三方代码。
- 对第二阶旋转的任何替代生成算法，须证明与作者 SHRotate 数值行为和系数顺序一致，并有合法的原创或兼容授权基础。

### 3.1.1 Ivanic–Ruedenberg 原论文、Don Williamson 参考与 Eigen-only 构建边界

| 核查项 | 已确认的上游事实或用户决定 | 实施要求（未执行） |
|---|---|---|
| 用户选定路线 | Ivanic–Ruedenberg 原论文（1996）及其 1998 勘误 **必须作为实现前依据**，Don Williamson C++ 是算法参考 | 使用 Eigen 自行重写 l≤2 的 R0/R3/R5，不依赖 sdla，论文上传之前不开始生产递推 |
| 上游实际算法文件 | [`Source/IvanicSHRotate.cpp`](https://github.com/dwilliamson/SHRotation/blob/master/Source/IvanicSHRotate.cpp)、[`Source/IvanicSHRotate.h`](https://github.com/dwilliamson/SHRotation/blob/master/Source/IvanicSHRotate.h)；配套 `Source/SHRotateMatrix.h` | **`Source/SHRotation.h` 是依赖 sdla 的示例应用类，不是独立的核心递推 API**，不能只复制该头文件就宣称解决 R5 |
| 底层依赖 | Williamson 参考源码包含 `sdla/Matrix.h` 和 `sdla/Exception.h`；本项目使用 Eigen3 | **禁用并移除 sdla 的所有 include、link、vendor 依赖，不做 sdla 包装或桥接**；用 Eigen::Matrix3d / Eigen::Matrix<double,5,5> 和统一 ErrorDetail 重写数学递推，须核 1998 勘误及源相位映射 |
| 项目许可与来源记录 | 仓库新代码已采用根 [MIT LICENSE](../../LICENSE)，Don Williamson 上游代码同为独立来源 | 正常开发**不因版权审批待办阻塞**，保留实际引入上游文件必要的原许可/署名；不将项目 MIT 扩展到原作者归档数据 |
| 旋转块接口 | MATLAB `SHRotate(r,2)` 输出 R0=1×1、R1=3×3、R2=5×5 | 由**本项目依据原论文及 1998 勘误编写的纯 Eigen 递推**生成旋转块，再由 `HarmonicRotator` 按原实球谐约定映射 `(-1,0,1)` 和 `(-2,-1,0,1,2)` 的次序/相位；逐项核对 R3 九元素与 R5 全 25 元素、转置和正交性。Don Williamson 仅供代码阅读参考，不引入 sdla |

**冻结条件**：必须先核对用户上传的 1996 原论文和 1998 勘误，完成 Eigen-only 的 R5 数学实现并对 R0/R3/R5 独立 L3 全元素验证；还需正式接口/CTest 和 DEC-005 最终阈值。项目 MIT 与不再以一般版权审查阻塞开发的方向已确定，但不意味着目前实现或验收完成。

### 3.2 旋转类接口候选

`egomotion::harmonics::HarmonicRotationBlocks` 以 **`Eigen::Matrix3d degree_one`、`Eigen::Matrix<double,5,5> degree_two`** 两个自拥有成员保存 R3/R5；R0=1 不设可修改成员。无状态的 `HarmonicRotator` 公开 `blocks(physical_rotation)` 和 `rotate(coefficients,physical_rotation)`，私有静态函数可检查有限旋转和构造 R3；`egomotion::harmonics::detail::IvanicRuedenbergEigen` 仅在内部负责预定 R5 返回接口。

**唯一声明文本、预计头文件名与源文件路径见 [专题 LLD §3](SH_ROTATION_IVANIC_RUEDENBERG.md)**；禁止在此保留另一套类声明、R5 实现公式或 sdla 接口。

**值对象归属与最小状态合同（候选）**：`HarmonicCoefficients` 的 3×5、九有效项、列主序由 [contracts.md §2](contracts.md) 的 domain 值对象维护；`HarmonicRotationBlocks` 归本球谐模块，`degree_one` 与 `degree_two` 是自拥有、定尺寸 Eigen 3×3/5×5，允许按值复制/移动，R0=1 是显式数学不变量，无需持久存储新矩阵。三类计算接口不持有 MAT 句柄或跨帧状态；Adaptive Session 负责历史系数。

**Result/Status 错误上下文**：`evaluate/fit/reconstruct/blocks/rotate` 使用 [contracts.md §3.0](contracts.md) 的统一 Failure/Result 候选，失败要保留 NumericalErrorCode、ErrorDetail 和已知输入条件（stage 候选 solve），**不能返回默认 R3/R5/空系数伪装成功**；错误码、const、复制和异常返回的最终 API 仍待 P0.8 冻结。

需明确是否接受不严格正交的 r、是否允许投影回 SO(3)、奇异输入与未定义旋转的返回状态；**不应未经许可擅自修改原算法**。

## 4. 依赖、资源和可复现测试

- RealSphericalHarmonics、HarmonicFitter、HarmonicRotator 均不持有 MAT 文件句柄和跨帧轨迹状态；Adaptive Session 独占系数历史。
- **L3 固定球面方向参考**：所有 9 个 (l,m) 配对值、均匀/非均匀采样下拟合的系数、旋转前后近度差异；由独立解析或非被测参考实现生成。
- **L3 多组旋转参考**：单位 R、绕 x/y/z 不同角度、复合旋转和逆变换；在指定系数顺序下逐元素检查 R3 的 9 个和 R5 的 25 个元素，另核 R0=1。
- 测试必须绑定只读原源码 SHA、独立 L3 参考输出/独立复核者与经 DEC-005 批准的数值容差；原始数据或独立 L3 参考若无法合法提交，可记录受控外部存储位置与独立 SHA-256。

### 4.1 CMake target、头文件与测试映射（候选）

| 边界 | 拟议内容 | 未完成条件 |
|---|---|---|
| CMake target | [ARCHITECTURE.md §4](../ARCHITECTURE.md) 的 `egomotion_harmonics` 显式依赖 **egomotion_domain + egomotion_contracts + Eigen**；Eigen-only 的 Ivanic–Ruedenberg l≤2 递推实现 | target 尚未创建/编译；**不能包含 sdla**，须等用户上传原文+勘误后逐式实现 |
| 公共头文件 | 候选 `include/egomotion/harmonics/{real_spherical_harmonics,harmonic_fitter,harmonic_rotator}.hpp` 三个公共头；R3/R5 值对象归 harmonic_rotator，3×5 系数归 domain，精确声明见 [专题 §3](SH_ROTATION_IVANIC_RUEDENBERG.md) | `egomotion_harmonics → egomotion_contracts` **已在 HLD §4 和 27 边 DAG 中明确**；具体头文件的自包含性、签名/ABI 仍需 P0.8 与真实编译验证；**只有未来实际改变依赖边界时**才须同步修改 HLD/ADR/DAG |
| 私有实现 | `src/harmonics/{real_spherical_harmonics,harmonic_fitter,harmonic_rotator}.cpp` 与私有 `src/harmonics/detail/ivanic_ruedenberg_eigen.{hpp,cpp}` 仅作**预定路径设计**；无 C++ 实现，禁用 sdla | 无实际代码/构建证据，确认编译依赖闭包不含 sdla |

| TC / AC | TEST_PLAN §3 的候选实体/CTest | L3 逐项核验 |
|---|---|---|
| TC-F-SH-001 / AC-F-SH-001 | `tests/unit/test_harmonics.cpp` / `egomotion.harmonics.*` | 九个实基的相位、归一化、3×5 有效项、拟合/近度 |
| TC-F-SH-002 / AC-F-SH-002 | 同上 | R0=1、R3 九元素、R5 25 元素、系数 before/after、x/y/z、复合/逆旋转 |
| TC-D-SOURCE-002 / AC-D-SOURCE-002 | `tests/golden/test_mat_archive.cpp` / `egomotion.mat.*` 与人工来源审查 | 上游 SHA、许可、依赖闭包；**不将来源审核等同算法测试** |

所需参考为独立解析或独立非被测实现的 **L3 固定输入数值**；原 `SHRotate.m` 仅是只读静态数学/索引依据。不允许运行 MATLAB/Octave，也不能用被测实现输出当参考。所有上述代码、测试实体、CMake target 与参考数据仍未创建/执行；DEC-003/005/007/008 未全部关闭。

## 5. 冻结与授权阻塞

- 仍需对源关联勒让德相位、SHRotate 块递推、独立 L3 固定输入样本做独立核验。
- **项目许可和数学来源已确定**：仓库新代码 MIT；球谐 l≤2 按 Ivanic–Ruedenberg 1996 原文＋1998 勘误以 Eigen 重写，Don Williamson 只作辅助参考，禁止 sdla；正常开发不因重复版权审查阻塞。实际引入第三方文件时保留必要原许可告知。论文上传前不做生产递推实现；数值与 CTest 仍未验收。
- C++17 精确签名、错误枚举、异常行为、目标 CMake target 和测试用例未冻结。

**本文件是详细设计草案；没有生产 C++ 实现、完整独立 L3 参考或真实数值/CTest 验收证据；无需 MATLAB 运行。**


## 6. l≤2 九个实球谐分量的显式公式（待独立 L3 参考核符号）

取归一化方向 \(x=\sin\theta\cos\phi\)、\(y=\sin\theta\sin\phi\)、\(z=\cos\theta\)，并**按原 MATLAB 的 legendre 默认相位**（含 Condon–Shortley 相位）展开。下列是从 §1 通式得到的数学等价表示，只有经过独立 L3 逐元素检查、相位核对与批准数值测试才能成为已验证的公式 Golden：

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

#### 7.1.1 AN-G08 的 z 轴 90° 系数方向（只读特例）

对原 `SHRotate.m` 的九处 R3 赋值和 `transfor_koeff.m` 的**左乘列向量**，可直接静态代入 `r=R_z(+π/2)` 得 `R3=[[0,0,-1],[0,1,0],[1,0,0]]`。对本项目实球谐 `m=-2,-1,0,1,2` 的 sin/cos 排序，按源代码的系数操作方向，角变量的相应变换是 **`φ→φ+π/2`**，而不是未经核对就写成 `φ−π/2`：m=±2 各变号；m=−1 与 +1 按 `a'_{-1}=−a_{+1}, a'_{+1}=a_{−1}` 交叉。因此本**单个特例**的 `R5` 与 AN-G08 的负号、行列及系数 before/after 一致。

这只证明候选特殊角度与原实基系数约定**代数一致**，不证明任意 SO(3) 下 Ivanic–Ruedenberg 递推、主动/被动相机坐标约定、一般旋转下 R5 25 元素或任何独立 L3/CTest 已通过。详见 [AN-G08 z 轴静态核对](../reviews/AN_G08_Z90_STATIC_CONVENTION_2026-11-08.md)；AF-045/055 仍开放。

### 7.2 第 2 阶 R5 的递推合同

**数学依据、代码参考与行为约定的区分**：R5 的正式实现依据为 **Ivanic–Ruedenberg 1996 原始论文和 1998 勘误**；生产代码必须用 **Eigen 独立重写**，不得引入 sdla。Don Williamson 的 `Source/IvanicSHRotate.cpp/.h` 仅是辅助代码参考，**不是待复制或直接适配的生产实现来源**。原只读 `SHRotate.m` 用于静态核对原作者 `SHRotate(r,2)` 的实球谐基底、索引及变换约定，并构造独立 L3 参考，不运行 MATLAB，也不逐行复刻源代码。该 MATLAB 函数在 degree=2 时遍历 \(m,n\in\{-2,-1,0,1,2\}\)，由前一阶 \(R_3\) 生成 25 个 \(R_5(m,n)\)。

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

U/V/W 在不同 m 与 n 边界上组合上一阶 \(R_3\) 条目与递推辅助 P。本节公式来自对原 MATLAB **输出数学行为的静态梳理**，不构成可直接编译的 R5 算法；完整递推分段、索引/相位映射须先阅读用户上传的 **Ivanic–Ruedenberg 1996 原文及 1998 勘误**，以 Eigen 实现，**不允许 sdla 依赖**；Don Williamson 仅为辅助代码参考。实现的数值验收必须用独立 L3 Golden 核对原静态源码所规定的**全部 25 个 R5 元素**及其索引/符号，不能只检查正交性或往返；H-10 暂缓下本轮不复制代码。

### 7.3 旋转样本与方向性判定

| 案例 | 预期数学性质 | 需要的**独立 L3 参考**（由原只读源码约定推导，不运行 MATLAB） |
|---|---|---|
| SH-R00 | r=I 时 R3=I3、R5=I5 | 34 个元素，源 SHA |
| SH-RZ | 物理 r 绕 z 轴旋转小角及 90° | 输入 r、R3、R5、九个系数 before/after |
| SH-RXY | 绕 x/y 轴和两轴复合旋转 | 各矩阵 34 元素、向量范数、近度值 |
| SH-INV | 先 r 后 r⁻¹ | 九个系数和方向采样下近度变化 |
| SH-BAD | 非正交或非有限输入 | 原只读源码的静态风险与独立 L3 非法输入；C++ 明确失败状态，不冒称已实测 MATLAB 异常 |

**实施时序**：用户已确定纯 Eigen / 不依赖 sdla，原始 1996 论文和 1998 勘误尚待上传；因此本轮只完成方案。上传后逐式确认并进入单独获准的 C++ 实现/测试，不要求 MATLAB/Octave/L1。

## 8. 异常、生命周期与接口合同

- 输入 l∈{0,1,2} 且 |m|≤l；方向有限、单位化规则经批准；输入 3×5 系数的非有效位置不应参与计算。
- 近度拟合遇 \(b_{lm}=0\) 时明确失败；数学输入为 NaN、Inf、零向量或非正交 r 时须输出具体 NumericalErrorCode/细节，不得伪造通过。
- RealSphericalHarmonics、HarmonicFitter、HarmonicRotator 可为无跨帧可变状态服务；球谐系数是自拥有值对象，由 Session 管理历史。
- 正式的 Result、HarmonicCoefficients、NumericalErrorCode 以 [contracts.md](contracts.md) 为单一候选；具体 API 签名和 const/复制行为由 P0.8 确认。
- TC-F-SH-001、TC-F-SH-002、TC-D-SOURCE-002 需记录**原只读 .m blob SHA、独立 L3 生成器/审查者身份、输入/参考 SHA-256**、逐元素 atol/rtol、有效条件和未定义样本数；DEC-003/005/007 未关闭时不得冻结，且不要求 MATLAB 运行。

**本模块的已知内容：9 项索引与数学公式、R3 九项映射、R5 的递推结构；缺口：R5 分段递推合法实现、全量独立 L3 参考、批准容差与真实代码/CTest。**

## 附录 A：无需 MATLAB 运行的证据边界

遵守 [长期无需 MATLAB 执行政策](../golden/NO_MATLAB_EXECUTION_POLICY.md) 与 [L2/L3/L4 验收矩阵](../golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md)。**本模块不要求运行 MATLAB/Octave、取得 L1 逐节点输出或追溯原脚本运行异常**；现行独立参考需覆盖原静态数学公式、旋转/坐标/相位、固定输入、源 SHA 及全部有定义元素，并经 DEC-005 容差核准及真实 C++/CTest 比较。无法确定的原 MATLAB 特定运行结果须明确限制声称范围，不能冒充原脚本已实测。当前尚无正式实现/验收证据。
