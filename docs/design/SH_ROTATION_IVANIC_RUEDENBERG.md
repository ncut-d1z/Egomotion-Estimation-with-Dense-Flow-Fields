# Ivanic–Ruedenberg 球谐旋转：论文、勘误与纯 Eigen 候选合同

> **专题 LLD v0.6，公共接口与全工程未冻结、未实现。** IR96、IR98、Choi99 论文已经审阅，低阶叶子 `egomotion::harmonics::low_order` 的 C++/Eigen 独立实现现已提交，且项目所有者提供固定 SHA `8eab5dd`、含增强回归的 `445ed151` 与624四元数压力的 `601c74de` 的 GCC16.2/Release **各1/1 CTest Passed**。本文件§5 中未来的公共类/头文件方案仍属未创建、未编译的候选，不能与已经构建的叶子混淆。新增的解析/相位断言在固定 `445ed151` 的 CTest 执行并通过；之后624非零四元数、错误分类、有限系数溢出在 `601c74de` 的 CTest执行并通过；完整 ABI/集成、最终 DEC-005 和广义异常测试未完成。详见 [PROGRESS E-DOC-056](../PROGRESS.md)。

**原生证据适用范围**：根 `CMakeLists.txt` 只生成 `egomotion_harmonics` 静态库，编译 `src/harmonics/low_order_basis.cpp`、`low_order_rotation.cpp`、`low_order_choi.cpp`，并执行 `tests/unit/test_low_order_harmonics.cpp`。用户观察到构建成功及 CTest 1/1 Passed，这并不表示下文规划的 `real_spherical_harmonics.hpp`、`harmonic_fitter.hpp`、`harmonic_rotator.hpp` 等公共/拟合器已实现，更不表示其它模块获准编码。每次新回归或源码变更需新 HEAD 的真实测试记录。

## 1. 原始依据及本轮已确认范围

| 资料 | 定位 | 本模块用途 |
|---|---|---|
| Ivanic & Ruedenberg 1996 | J. Phys. Chem. 100, 6342–6347；DOI `10.1021/jp953350u` | 实数递推，§5的轴/基定义、式(8.1)、Tables 1/2；Table 3为l=2显式参考 |
| Ivanic & Ruedenberg 1998勘误 | J. Phys. Chem. A 102, 9099–9100；出版社DOI `10.1021/jp9833350` | 修正式(6.7)、(7.9a/b/c)及P边界索引；Table 2残留符号冲突必须另行记录 |
| Choi / Ivanic / Gordon / Ruedenberg 1999 | J. Chem. Phys. 111, 8825–8831；DOI `10.1063/1.480229` | F/G实数运算实现复递推、W实复基转换；独立交叉参考候选，不自动替换生产路线 |

附件SHA-256、页数及IR98页脚DOI与出版社元数据不一致的来源说明见审计记录。**原PDF未上传公共仓库**，不要求用户重复提交。Don Williamson源码仍仅为辅助参考，不能替代用户材料，也不得引入其sdla依赖。

### 1.1 明确勘误，不静默修论文

IR98 p.9099将式(6.7)、(7.9b)、(7.9c)的特定负号改正号，将式(7.9a)第一括号第一项的下标1改成−1。其Table 2把P边界列明确为前阶`±(l−1)`。与`SHRotate.m`的M/U/P/V/W逐项对应见审计记录§3。

**重要保留问题**：IR98 p.9100 Table 2负m的V末因子印成`sqrt(1−δ_(m,−1))`；IR96原Table 2及原`SHRotate.m:119–123`为`sqrt(1+δ_(m,−1))`。在`l=2,m=−1,n=0,r=R_x(π/4)`时，前者给`√3/4`，后者及IR96 Table 3给`√3/2`。本LLD候选采用与原源码和显式表一致的**加号**，并保留“附件疑似残留排印错误”的明确标签；**不是声称作者另行更正或已经数值验收**。

## 2. 纯 Eigen 及作用域边界

| 边界 | 候选设计 | 禁止事项 |
|---|---|---|
| 输入 | `Eigen::Matrix3d` / domain `Matrix3` | sdla类型及隐式坐标改变 |
| R3 | 自拥有3×3，按源码实际九元素构造 | 将物理矩阵直接当R3或未经说明使用论文转置方向 |
| R5 | 自拥有`Eigen::Matrix<double,5,5>`，行列m/n映射0-based `m+2,n+2` | 直接移入sdla、未经核对的递推片段 |
| R0 | 恒为1，不新增可变成员 | 扩展l>2或修改有效系数位置 |
| 失败 | 公共Result/NumericalErrorCode/ErrorDetail | 默认矩阵或NaN伪装成功；静默投影SO(3) |
| 构建 | `egomotion_harmonics → domain + contracts + Eigen`，保持HLD27边 | 直接/间接include、link、vendor sdla |

普通版权选型不再是开发门禁；仅保留实际复用文件的必要notice。项目根MIT不重新许可原PDF、原MAT或他人作品。**本轮无生产源码或构建动作**。

## 3. 原源码行为与论文约定的精确分离

令P把xyz重排为(y,z,x)：

\[
P=\begin{bmatrix}0&1&0\\0&0&1\\1&0&0\end{bmatrix}.
\]

CIGR99式(8.15)：`B_paper(R)=P R P^T`；原`SHRotate.m:33–43`：

\[
B_{source}(r)=P r^T P^T=
\begin{bmatrix}r_{22}&r_{32}&r_{12}\\r_{23}&r_{33}&r_{13}\\r_{21}&r_{31}&r_{11}\end{bmatrix}.
\]

因此对应论文输入必须写清`R=r^T`。原`transfor_koeff.m:24–25`对`(-1,0,1)`和`(-2,-1,0,1,2)`系数列左乘。**转置差异不是可以直接删除的源码错误**，而是输入轴定义需要显式统一。

当前`kugelflfunc2`的一阶实基又为`√(3/(4π))(-y,z,-x)`。若验证指定的`f_new(d)=f_old(r d)`，还需J=diag(−1,1,−1)的相位共轭`J B_source J`；这与忠实原系数输出一般不同。绕x90°/旧系数(0,1,0)能区分，绕z的AN-G08不能覆盖此问题。详见 [审计§5](../reviews/SH_PAPERS_SOURCE_AUDIT_2026-11-09.md)。**保留原算法，不在设计修订中擅自改成物理修正版**；AF-045/046仍需该语义与一般输入验证闭合。

## 4. l=2递推候选：明确来源、索引、短路

以下是数学合同，不是C++函数体。记`B`为§3确定的一阶块，`L`为前一阶块，所有下标用数学m标签。令

\[
d=\delta_{m0},\quad D=\begin{cases}2l(2l-1),&|n|=l,\\l^2-n^2,&|n|<l,\end{cases}
\]
\[
u=\sqrt{(l^2-m^2)/D},\quad
v=\frac{1-2d}{2}\sqrt{(1+d)(l+|m|-1)(l+|m|)/D},\quad
w=-\frac{1-d}{2}\sqrt{(l-|m|-1)(l-|m|)/D}.
\]

原辅助P的边界对应IR98 Table 2：

\[
P(i,l,a,n)=\begin{cases}
B_{i0}L_{a,n},&|n|<l,\\
B_{i1}L_{a,l-1}-B_{i,-1}L_{a,-l+1},&n=l,\\
B_{i1}L_{a,-l+1}+B_{i,-1}L_{a,l-1},&n=-l.
\end{cases}
\]

`U=P(0,l,m,n)`。V的候选与原源码一致（负m平方根冲突已在§1.1标记）：

- m=0：`P(1,l,1,n)+P(−1,l,−1,n)`。
- m>0，δ=δ_(m,1)：`sqrt(1+δ)P(1,l,m−1,n)−(1−δ)P(−1,l,−m+1,n)`。
- m<0，δ=δ_(m,−1)：`(1−δ)P(1,l,m+1,n)+sqrt(1+δ)P(−1,l,−m−1,n)`。

源W在m>0时为`P(1,l,m+1,n)+P(−1,l,−m−1,n)`，m<0时为`P(1,l,m−1,n)−P(−1,l,−m+1,n)`；m=0不得求值。组装`R_l(m,n)=uU+vV+wW`时，必须**先判精确零系数，再决定是否调用辅助项**，不能先计算越界/未定义项后乘0。原M第72～82行已经如此处理。

**当前只需l=2**：m=0时w因(1−d)为0，|m|=1或2时乘积为0，所以25个R5元素的w都为0，W不应被调用。保留原函数追踪关系不等于要求生产实现支持任意l。u=0的极端行也不得先访问前阶不存在的行。

零系数、m顺序、边界索引属离散合同，不设置额外小量阈值。非有限/非正交输入的处理与最终容差仍待P0.8/DEC-005/008和真实测试。

## 5. C++17文件与唯一声明（仍无实现）

| 计划路径（尚未创建） | 职责 |
|---|---|
| `include/egomotion/harmonics/real_spherical_harmonics.hpp` | evaluate/reconstruct与degree/order检查 |
| `include/egomotion/harmonics/harmonic_fitter.hpp` | 源逐项投影/归一拟合，不改为联合最小二乘 |
| `include/egomotion/harmonics/harmonic_rotator.hpp` | 自拥有R3/R5及旋转接口 |
| `src/harmonics/real_spherical_harmonics.cpp` | 无状态实基实现 |
| `src/harmonics/harmonic_fitter.cpp` | 无状态逐项拟合实现 |
| `src/harmonics/harmonic_rotator.cpp` | 校验/映射/系数变换 |
| `src/harmonics/detail/ivanic_ruedenberg_eigen.hpp` | 私有递推接口 |
| `src/harmonics/detail/ivanic_ruedenberg_eigen.cpp` | 未来纯Eigen递推，禁sdla |
| `tests/unit/test_harmonics.cpp` | 原TC-F-SH-001/002及独立参考；尚未创建 |

~~~cpp
namespace egomotion::harmonics {
class RealSphericalHarmonics final {
public:
    static bool validDegreeOrder(int degree, int order) noexcept;
    Result<double, NumericalErrorCode> evaluate(
        int degree, int order, double theta_rad, double phi_rad) const;
    Result<Eigen::VectorXd, NumericalErrorCode> reconstruct(
        const HarmonicCoefficients& coefficients,
        const DirectionSet& directions) const;
};
class HarmonicFitter final {
public:
    Result<HarmonicCoefficients, NumericalErrorCode> fit(
        const DirectionSet& directions,
        const Eigen::VectorXd& nearness) const;
};
struct HarmonicRotationBlocks final {
    Eigen::Matrix3d degree_one;
    Eigen::Matrix<double, 5, 5> degree_two;
};
class HarmonicRotator final {
public:
    Result<HarmonicRotationBlocks, NumericalErrorCode> blocks(
        const Eigen::Matrix3d& physical_rotation) const;
    Result<HarmonicCoefficients, NumericalErrorCode> rotate(
        const HarmonicCoefficients& coefficients,
        const Eigen::Matrix3d& physical_rotation) const;
private:
    static bool finiteRotationElements(
        const Eigen::Matrix3d& physical_rotation) noexcept;
    static Result<Eigen::Matrix3d, NumericalErrorCode> degreeOneBlock(
        const Eigen::Matrix3d& physical_rotation);
};
}
namespace egomotion::harmonics::detail {
class IvanicRuedenbergEigen final {
public:
    static Result<Eigen::Matrix<double, 5, 5>, NumericalErrorCode>
    degreeTwoBlock(const Eigen::Matrix3d& physical_rotation,
                   const Eigen::Matrix3d& degree_one);
};
}
~~~

公共Result、NumericalErrorCode、DirectionSet、HarmonicCoefficients仍只由contracts/domain定义。本文件是上述候选声明的单一维护位置；其他LLD/对象模型引用类/方法名，不复制第二套ABI。三计算类无跨帧状态、不持有MAT/RNG/Session、无可变全局缓存；矩阵按值拥有。物理输入参数名的最终语义必须与§3一致并在接口冻结时明确，当前不因参数名推定旋转方向。

## 6. 已取得的证据与真正剩余工作

| 工作 | 当前状态 |
|---|---|
| 三篇PDF取得、身份登记、关键公式/表格图像阅读 | 已完成材料与静态审阅，不再等待上传 |
| 原式/勘误/源M/U/P/V/W对应及Table2冲突反例 | 已形成候选；不是作者额外勘误或数值PASS |
| R3转置与源基相位风险 | 已明确，仍需一般SO(3)和物理语义闭合 |
| R5全25项非递归公式、1999独立路线 | 见 [验证候选](../golden/SH_PAPER_VERIFICATION_CASES.md)，未执行 |
| Eigen生产实现、头自包含/ABI、构建/CTest | 未实施，H-10/H-11仍适用 |
| 独立L3文件/逐元素差异、DEC-005/008、正式签署 | 未完成；AF-045/046/051/055仍开放 |

当前检验路线为源忠实性 → 论文映射/相位语义 → 非递归Table3/1999独立参考 → 获授权的实际C++/CTest。文献高阶性能数字不替代本项目实测。MATLAB/Octave/L1始终非必需，首轮0.01/0.1不自动成为最终PASS标准。
