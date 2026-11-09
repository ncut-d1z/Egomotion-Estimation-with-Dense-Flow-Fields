# 实球谐、近度重建及系数旋转——详细设计

> **LLD v0.11，未冻结、完整业务数值未验收。** 2026-11-09 用户仅开放低阶球谐 C++ 实现与 Wolfram 符号验证，`egomotion::harmonics::low_order` 叶子代码及专项测试已提交；九基直接/递推与 IR96/IR98、Choi1999 的**一般 SO(3) 精确符号验证已执行**。项目所有者又提供了固定 SHA `8eab5dd` 与增强回归 `445ed151` 的 Ubuntu/GCC16.2/Eigen3.4 Release **各1/1 CTest Passed** 原生结果；解析/相位锚点已在后者实际执行。正式三类公共接口、离散拟合、全业务集成、独立全部边界和最终容差尚未完成。权限见[专项授权](../decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md)，证据见[三路报告](../reviews/LOW_ORDER_SH_THREE_ROUTE_VERIFICATION_2026-11-09.md)、[PROGRESS E-DOC-056](../PROGRESS.md)。

关联REQ-F-SH-001/002、REQ-D-SOURCE-002、REQ-NF-COPYRIGHT-001及P7.1–P7.6/P7.10。原`winkelausvektor`、`kugelflfunc2`、`near_aus_koeff`、`koeff_aus_near`、`transfor_koeff`、`SHRotate`仍只读；四篇论文材料齐备，不再以缺PDF阻塞。纯Eigen、C++17、double，禁止sdla；Don Williamson仅辅助参考，无直接代码移入。

## 1. 区分论文的正相位基与原MATLAB-CS基

对单位方向d=(x,y,z)，theta=acos(z)为极角，phi=atan2(y,x)为方位角，单位rad。原MATLAB的`legendre(l,cos(theta))`含Condon–Shortley相位。令q=|m|，原`kugelflfunc2`为

\[
A_{lq}=\sqrt{\frac{2l+1}{4\pi}\frac{(l-q)!}{(l+q)!}},\quad
Y_{lm}^{source}=\begin{cases}
\sqrt2 A_{lq}P_l^q(\cos\theta)\sin(q\phi),&m<0,\\
A_{l0}P_l^0(\cos\theta),&m=0,\\
\sqrt2 A_{lq}P_l^q(\cos\theta)\cos(q\phi),&m>0.
\end{cases}
\]

本轮实现的canonical基按2015附录式(59)–(67)排序为`[g0,f1,f2,f3,h1,h2,h3,h4,h5]`，不默认认为它与原MATLAB九项同号同序。

| 论文项 | 直接Cartesian公式 | 原MATLAB有效位置（1-based）及系数转换 |
|---|---|---|
| g0 | `1/sqrt(4*pi)` | `a_g0=C(1,1)` |
| f1 | `sqrt(3/(4*pi))*x` | `a_f1=-C(2,3)` |
| f2 | `sqrt(3/(4*pi))*y` | `a_f2=-C(2,1)` |
| f3 | `sqrt(3/(4*pi))*z` | `a_f3=C(2,2)` |
| h1 | `sqrt(5/(16*pi))*(3*z²-1)` | `a_h1=C(3,3)` |
| h2 | `sqrt(15/(4*pi))*xz` | `a_h2=-C(3,4)` |
| h3 | `sqrt(15/(4*pi))*yz` | `a_h3=-C(3,2)` |
| h4 | `sqrt(15/(16*pi))*(x²-y²)` | `a_h4=C(3,5)` |
| h5 | `sqrt(15/(4*pi))*xy` | `a_h5=C(3,1)` |

因此原CS m序的一阶为sqrt(3/(4pi))*(-y,z,-x)，二阶为正xy、负yz、h1、负xz、h4；这本身是合法基约定，不是任意可删的负号。上述系数转换及逆转换**对任意九个系数保持同一个标量场**，已由Wolfram精确证明，不只验证往返。3×5无效位置不参与原重建；新转换输出将它们置零。原初始化C(1,1)=1意味着常数近度1/sqrt(4pi)，不是1。

### 1.1 已验证的基值递推

新`basisRecurrence`使用去CS相位的关联Legendre递推，保留`P_l^m*cos(m phi)`与`P_l^m*sin(m phi)`的笛卡尔多项式形式。p11c=x、p11s=y，p20=(3z²−1)/2，p21c=3zx、p21s=3zy、p22c=3(x²−y²)、p22s=6xy；再乘定义给出的归一化系数。

[实际执行脚本](../../verification/harmonics/verify_low_order_basis.wl)已核：直接值与复球谐定义9项差全0；直接值与递推值9项差全0；Gram=I9共81项；对应实体调和多项式Laplacian9项全0。两极由Cartesian连续延拓覆盖，不除sin(theta)、不依赖atan2。

## 2. 近度重建与尚未实施的离散拟合

原`near_aus_koeff.m`的定义仍是

\[
\mu(d_i)=\sum_{l=0}^{2}\sum_{m=-l}^{l}a_{lm}Y_{lm}^{source}(d_i).
\]

新叶子的`reconstruct`用论文九基和论文序列系数计算同一个有限展开；使用旧C时必须先`fromMatlabLayout`。

原`koeff_aus_near.m`逐项拟合为

\[
a^{num}_{lm}=\frac{4\pi}{N}\sum_i\mu_iY_{lm}(d_i),\quad
 a^{den}_{lm}=\frac{4\pi}{N}\sum_iY_{lm}(d_i)^2,\quad a_{lm}=a^{num}_{lm}/a^{den}_{lm}.
\]

**HarmonicFitter、N点数据接口、与Adaptive的衔接本轮未实施。** 不把逐项投影改成联合最小二乘；不因4pi/N代数可消就宣称浮点次序等价。N=0、范数分母零、非有限的实际测试仍待完成。

AF-090仍适用：`spherical_harmonic_illustration`的rho=5+3yy/max(abs(yy))是独立图示，不作为核心九基或旋转验证依据，见[分析LLD](analysis_plotting.md)。

## 3. 新叶子的唯一旋转合同

\[
\mu_{new}(d)=\mu_{old}(A d),\quad A\in SO(3),\quad
 a'_{0}=a_0,\quad a'_1=D_1(A)a_1,\quad a'_2=D_2(A)a_2.
\]

系数列左乘；论文正xyz基给`D1=A^T`。对应组合为`D(A B)=D(B)D(A)`。具体主动旋转/相机反旋由上层明确传A或A^T，不在叶子猜测，不擅改轨迹或Session。

### 3.1 三条实际实现路线

| 路线/API | 数学来源和独立性 | 当前证据 |
|---|---|---|
| `rotationDirect` | 五个对称无迹张量T_i由定义构造，`D2_ij=2 Tr(T_i A^T T_j A)` | 全25项及场作用符号证明 |
| `rotationIvanic` | IR96式8.1、IR98修正P边界及经定义判别的V加号；B=P A^T P^T，显式行列重排 | 对一般非零实四元数，9+25项与定义路线差全0 |
| `rotationChoi` | Choi1999式5.4/5.5、6.1/6.9/6.14及W D W^dagger；独立复运算路线，不调用IR | 对同一一般域，9+25项与定义路线差全0 |

l=2的w对全部m精确为0，不求W；u=0先跳过辅助项再访问前阶数组，不用浮点小阈值替代精确零分支。当前只支持l<=2，不为测试W扩大范围。Choi用`adjoint()`，非普通transpose，虚部残留必须检查。

### 3.2 与原SHRotate行为的区别必须明示

原`SHRotate.m`实际R3=P r^T P^T，且`transfor_koeff`直接对原CS系数左乘。若把r解释成当前A而不作CS相位转换，它不等于当前指定的物理场作用。例mu_old=2+z、A=Rx(pi/2)，旧组合为2−y，新明确合同为2+y；在ey处分别1和3。Wolfram已验证这个全正近度反例。

这是**输入/基/系数约定的组合差异**，不据此宣称原实验全部无效，也不自动改写原`origin`或业务调用方。新实现以定义证明的明确合同为准，旧3×5仅通过保持同一场的转换接入。旧AN-G08 z90候选原样保留，它不能独自证明一般旋转。

## 4. 已核出的论文印刷问题

- IR98 p.9100 Table2负m的V末因子印成sqrt(1−delta_(m,-1))；源码、IR96 Table3、定义和Choi独立路线要求加号。本轮一般符号验证支持加号，Rx45对应元素为sqrt(3)/2，印刷减号给sqrt(3)/4。候选选择已提升为本限定数学域的精确验证，不伪称作者另有正式勘误。
- 2015 p.30最后d1²/d2²展开与式66定义的h4符号相反。正确xx用`+sqrt(4pi/15)*h4`，yy用负号；印刷xx实际上等于y²，印刷yy等于x²。**式59–67九基本身正确**。本轮不实施/修改耦合矩阵等其他模块。

详见[三路验证报告§7](../reviews/LOW_ORDER_SH_THREE_ROUTE_VERIFICATION_2026-11-09.md)，保留原页面，不静默改原文。

## 5. 本轮代码与未来公共接口分层

| 路径 | 状态与职责 |
|---|---|
| `include/egomotion/harmonics/low_order.hpp` | 已创建，论文9系数、有限叶子Result、基值/三旋转路线/重建/布局转换 |
| `src/harmonics/low_order_detail.hpp` | 已创建，输入SO(3)/单位方向检查、固定排列、有限输出 |
| `src/harmonics/low_order_basis.cpp` | 已创建，直接/递推基值、重建、旋转列系数与转换 |
| `src/harmonics/low_order_rotation.cpp` | 已创建，独立张量与IR实现 |
| `src/harmonics/low_order_choi.cpp` | 已创建，复递推与实基转换 |
| `tests/unit/test_low_order_harmonics.cpp` | 已创建，30个旋转、10方向、9基、3路线等确定性专项测试源码；未执行 |

命名空间为`egomotion::harmonics::low_order`，不冒充已实现[专题§5](SH_ROTATION_IVANIC_RUEDENBERG.md)中旧规划的RealSphericalHarmonics/HarmonicFitter/HarmonicRotator公共ABI。domain/contracts未获实施授权，新叶子不引入这些尚不存在的公共类型；局部Result封装variant，仅是本模块可独立验证的边界。

**已建立且通过用户首次本机编译的 `egomotion_harmonics` CMake 静态库 target，以及单个 `low_order_harmonics` CTest。** HLD 原27条设计DAG未修改、尚未接入其它模块；现有叶子仅依赖 C++17/Eigen、禁 sdla。未来公共三类接口、共享类型/生命周期/ABI/全业务适配仍须核对；增补解析/dyadic/MATLAB相位断言已在 `445ed151` CTest 实际通过，未来任何新变更不得借用旧 HEAD 的通过结果。

## 6. 独立参考、异常与验收

实际数学脚本位于`verification/harmonics/`：九基、全SO(3)三路矩阵、布局同场性各自有脚本；实际结果JSON包含内核版本、来源检查点、全元素零差和独立有理旋转参考。它们不是被测C++自行计算再复制为Expected。

代码拒绝非有限、非单位方向、非SO(3)及反射，不归一化或投影。`128*epsilon(double)`仅输入/虚部舍入检查；`4096*epsilon`仅专项测试源码的比较余量，不替代DEC-005最终阈值。输出矩阵、旋转系数、重建值均检查非有限；全部自拥有，不持有MAT/RNG/Session或可变全局缓存。

测试源码覆盖轴向、一般、近零/近pi、非交换组合、逐基场重建、独立25值参考、旧相位反例、非法输入及增强解析/dyadic/MATLAB相位锚点；**其所在固定版本 `445ed151` 已由用户原生执行，CTest 1/1 Passed**。源码逐式复读与Wolfram精确恒等证明不能自动证明编译器/浮点/所有异常行为。

关联TC-F-SH-001/002、TC-NF-NUM-001/002与TC-D-SOURCE-002；既有66TC总量不变，专项子用例不是新TC。AF-045/046/051/055的符号数学部分已有实证，但整体原生、公共接口、其他Golden与批准义务尚缺，继续开放。

## 7. 后续进入条件

H-10球谐实现与本次Wolfram已授权；**不再等待论文或笼统等待全部代码许可**。原生球谐专项编译/测试仍未执行；取得明确运行范围后才执行，不因此开放其他模块。正式DEC-005/008、ABI冻结、项目集成仍需独立证据。

按[长期政策](../golden/NO_MATLAB_EXECUTION_POLICY.md)，永久不要求MATLAB/Octave/L1。原论文、源码、MAT及旧审查记录保持不变；仅在本文件及PROGRESS/AUDIT维护当前的局部进展与真实限制。
