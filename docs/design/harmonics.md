# 实球谐、近度重建及系数旋转——详细设计

> **LLD v0.16，未冻结、完整业务数值未验收。** 2026-11-09 用户仅开放低阶球谐 C++ 实现与 Wolfram 符号验证，`egomotion::harmonics::low_order` 叶子代码及专项测试已提交；九基直接/递推与 IR96/IR98、Choi1999 的**一般 SO(3) 精确符号验证已执行**。项目所有者又提供了固定 SHA `8eab5dd`、增强回归 `445ed151` 和624四元数/错态压力 `601c74de` 的 Ubuntu/GCC16.2/Eigen3.4 Release **各1/1 CTest Passed** 原生结果；真实最大绝对残差分别为8.8817841970012523e-16（矩阵/向量）、1.0658141036401503e-14（标量），回归比较阈值9.0949470177292824e-13。正式三类公共接口、离散拟合、全业务集成、独立全部边界和最终容差尚未完成。权限见[专项授权](../decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md)，证据见[三路报告](../reviews/LOW_ORDER_SH_THREE_ROUTE_VERIFICATION_2026-11-09.md)、[PROGRESS E-DOC-056/057/059](../PROGRESS.md)。

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

## 2. 近度重建与低阶逐系数离散投影

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

**当前新增的球谐叶子功能**：`projectSourceCoefficientsOrderTwo(nearness,setofpoints)` 专门复现 `koeff_aus_near.m` 的固定 `ordnung=2` 分支。接受 N>0 的有限近度和 N×K (K≥3) 样本，仅解释前三列的有限单位方向；按原 m 排列的九个源 CS 基函数**分别**求和，先算 `a=(sum(near*Y)/N)*4pi` 与 `b=(sum(Y²)/N)*4pi` 再相除，不将其替换为九元联合 LS；返回 3×5 源布局且无效位置填零。错误返回：空/维度错 `InvalidSampleShape`，近度/方向非有限 `NonFiniteInput`，非单位方向 `NotUnitDirection`，零平方范数 `ZeroProjectionNorm`，累加/比值溢出 `NonFiniteResult`；原 MATLAB 在这些病态数据上的警告/NaN 传播未取证，因此不能主张等价。**一般化跨模块HarmonicFitter、动态阶数、Adaptive/Session接口仍未实现；但这个已存在的受限叶子本身，在固定提交`5e5221988cf093dbfddb88f2f65f06632460f3a1`下，已有用户原生编译及对应专项CTest通过证据。** 该证据不属于公共HarmonicFitter/DEC-005或后续动态HEAD的测试。

**独立精确参考（不是被测实现生成）**：取七个单位方向 `e_x,e_y,e_z,(3/5,4/5,0),(0,3/5,4/5),(4/5,0,3/5),(1/3,2/3,2/3)` 与近度 `(2,-1,4,1/2,-3,5/4,11/2)`。已实际执行 [Wolfram 精确计算](../../verification/harmonics/verify_source_projection_order2.wl)，九项系数差全部为0，所有平方范数分母严格为正；离散 Gram 的非对角项 `G[2,3]≈-0.2206948544` 非零，所以**此固定场能够区分源逐项比值与联合最小二乘**。精确有理/根式值单独保存于 [source_projection_reference.v1.json](../../verification/harmonics/source_projection_reference.v1.json)，其状态是独立数学参考、**非已运行的 C++/批准 Golden**；本模块专有测试 `test_low_order_source_projection.cpp` 覆盖所有九项、六处无效填充、额外流列忽略、零/维度/非有限/非单位/溢出等负例；**现已在5e522198用户原生CTest通过**。

AF-090仍适用：`spherical_harmonic_illustration`的rho=5+3yy/max(abs(yy))是独立图示，不作为核心九基或旋转验证依据，见[分析LLD](analysis_plotting.md)。

### 2.1 已存在的低阶叶子错误类型与未来公共错误链的适配边界（2026-11-14，真实接口交叉整改）

**静态读码事实**：已实际读取固定分支上的`include/egomotion/harmonics/low_order.hpp`（Git blob `8b2a8723d22fec0c7e544cab57952173c8daacfb`）、`low_order_source_projection.cpp`（blob `7d3c7aadc90841c5c66d41ccebcfdfdca11306e5`）及其专项测试文件。受限叶子已有**局部**`egomotion::harmonics::low_order::ErrorCode`和`Result<T>`，不是公共`NumericalErrorCode`及`Result<T,NumericalErrorCode>`。叶子`Result::error() const`自拥有返回局部`Error{code,const char* message}`；访问错误alternative时由`std::get`抛`std::bad_variant_access`。这只是**历史已实现叶子的局部事实**，不能将用户对未来跨模块Result所决定的`std::logic_error`、`ErrorDetail.causes`错误链**追溯声明为叶子已有行为**；未来业务桥接必须正面处理这道类型边界，不能偷偷把局部类型当公共API强转。

| 实际`low_order::ErrorCode`成员（源码有定义） | 错误源事实及适配注意事项 |
|---|---|
| `NonFiniteInput`、`NonFiniteResult` | 公共`NumericalErrorCode`有同名候选，但必须保留局部枚举家族`low_order::ErrorCode`作为根因帧，不能只通过字符串同名假设枚举ABI相同 |
| `InvalidSampleShape`、`NotUnitDirection` | 公共候选分别为`InvalidShape`、`InvalidDirection`，仅语义相近，不能使用`static_cast`按枚举整数映射；由桥接函数逐项检查后转成业务层错误 |
| `ZeroProjectionNorm` | 原离散投影某通道平方范数为零；公共有`ZeroNorm`/`SingularSystem`，但需明确业务失败资格，不能未经定义就选其一并删除局部Code |
| `NotProperRotation`、`ComplexResidue`、`InvalidMethod` | 公共`NumericalErrorCode`没有明确一一对应的同义项；特别是**有限复数虚部残余不等于`NonFiniteResult`**，非法旋转矩阵也不自动等于非法单位视线。不可压成模糊`InvalidShape`或自动改成`Result::success` |

**确定的集成边界**（不增加新枚举、不替用户授权实施）：低阶叶子出错时，适配器必须先`if (!leaf_result.hasValue())`再读取`leaf_result.error()`（自拥有局部Error），严禁让局部`std::bad_variant_access`替代正常业务失败；新业务层需要包装Failure时，创建`ErrorFrame{domain="egomotion::harmonics::low_order::ErrorCode",code=local_code,...,stage=Solve,message=leaf_message}`，按已决定的`std::forward_list`规则保留这一真实原始故障层。对于**Runner层**可以将外层类型定为既有`ExperimentErrorCode::SolverFailed`并保留叶子Frame；如未来`HarmonicFitter`直接承诺返回`NumericalErrorCode`，须在接口LLD明确合法一一映射和无法无损映射项的处理，**此处不擅自新建公共ErrorCode或由外层静默硬转**。正常业务失败走Failure，程序员错用叶子Result的`bad_variant_access`也不应被掩盖为科学数值失败。

**已有证据不可混用**：固定`5e522198`用户专项3/3 CTest支持已有叶子的原错误码等局部行为，但**没有**当前公共Result工厂/`std::logic_error`/`std::forward_list<ErrorFrame>`桥接、RunContext、MAT集成的原生测试。建议复用既有`TC-F-SH-001/002`、`TC-NF-OOP-001`、`TC-I-RESULT-001`做未来跨层故障注入；要求对八个局部ErrorCode逐个验证来源家族、Code、stage、message及根因不丢，并对`ComplexResidue/InvalidMethod/NotProperRotation`禁止猜测式公共枚举转换。不会新增正式第67个TC或实施C++，H-10/H-11仍无限期延期。

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
| `src/harmonics/low_order_source_projection.cpp` | 受限二阶源逐系数投影；固定5e522198下用户实际GCC16编译，专项76次检查的CTest通过；原MATLAB并未运行 |
| `tests/unit/test_low_order_harmonics.cpp` | 已创建，旧版在固定601c74de完成53,532项检查/624四元数及三路线，1/1 CTest通过；更广验证/批准仍开放 |
| `tests/unit/test_low_order_archived_golden.cpp` | AN-G06/07/08原CS相位与归档R3/R5/3×5候选；5e522198用户原生CTest 176次检查通过 |
| `tests/unit/test_low_order_source_projection.cpp` | 七方向精确九系数、分母异常/维度/非有限/额外列回归；5e522198用户原生CTest 76次检查通过 |

命名空间为`egomotion::harmonics::low_order`，不冒充已实现[专题§5](SH_ROTATION_IVANIC_RUEDENBERG.md)中旧规划的RealSphericalHarmonics/HarmonicFitter/HarmonicRotator公共ABI。domain/contracts未获实施授权，新叶子不引入这些尚不存在的公共类型；局部Result封装variant，仅是本模块可独立验证的边界。

**固定 `5e5221988cf093dbfddb88f2f65f06632460f3a1` 上三个独立低阶球谐CTest已由用户在Ubuntu GCC16.2实际编译并3/3 Passed**：原回归53,532次检查、归档AN-G06/07/08参考176次、二阶逐系数投影76次。用户提交的完整可见日志记录真实误差，详见[PROGRESS E-DOC-062](../PROGRESS.md)。**这不是通用公共ABI/动态阶数/最终DEC-005验收**，下一提交的新代码不能沿用此前PASS。 HLD 原27条设计DAG未修改、尚未接入其它模块；现有叶子仅依赖 C++17/Eigen、禁 sdla。未来公共三类接口、共享类型/生命周期/ABI/全业务适配仍须核对；增补解析/dyadic/MATLAB相位断言已在 `445ed151` 实测，624非零四元数/错误码/溢出又在 `601c74de` 实测；未来任何新变更不得借用旧 HEAD 通过结果。

## 6. 独立参考、异常与验收

实际数学脚本位于`verification/harmonics/`：九基、全SO(3)三路矩阵、布局同场性各自有脚本；实际结果JSON包含内核版本、来源检查点、全元素零差和独立有理旋转参考。它们不是被测C++自行计算再复制为Expected。

代码拒绝非有限、非单位方向、非SO(3)及反射，不归一化或投影。`128*epsilon(double)`仅输入/虚部舍入检查；`4096*epsilon`仅专项测试源码的比较余量，不替代DEC-005最终阈值。输出矩阵、旋转系数、重建值均检查非有限；全部自拥有，不持有MAT/RNG/Session或可变全局缓存。

测试源码覆盖轴向、一般、近零/近pi、非交换组合、逐基场重建、独立25值参考、旧相位反例、非法输入及增强解析/dyadic/MATLAB相位锚点；**增强版 `445ed151` 和压力版 `601c74de` 均由用户原生执行，CTest各1/1 Passed**。源码逐式复读与Wolfram精确恒等证明不能自动证明编译器/浮点/所有异常行为。

关联TC-F-SH-001/002、TC-NF-NUM-001/002与TC-D-SOURCE-002；既有66TC总量不变，专项子用例不是新TC。AF-045/046/051/055的符号数学部分已有实证，但整体原生、公共接口、其他Golden与批准义务尚缺，继续开放。

**归档候选交叉校验补充（2026-11-09）**：当前 AN-G06 原CS 9基、AN-G07 20个归档方向近度1、AN-G08 原m顺序z90的R3/R5与3×5系数都有**独立字面参考**，不从被测Eigen递推中构造Expected。对其候选值已用独立 Wolfram CS多项式计算核对（全域表达式相应的限定恒等式 `allExact=True`），脚本位于 [verify_archived_sh_candidates.wl](../../verification/harmonics/verify_archived_sh_candidates.wl)。第二个归档CTest已在5e522198实际通过176次检查，但不把该局部证据扩展为全部Golden/TC通过；不代表已实现完整原MATLAB近度离散拟合或全业务源忠实移植，正式 DEC-005/008 仍需批准。对应 [TEST_PLAN](../TEST_PLAN.md) 和 [PROGRESS E-DOC-060](../PROGRESS.md)。

## 7. 后续进入条件

H-10球谐实现与本次Wolfram已授权；**不再等待论文或笼统等待全部代码许可**。球谐叶子三次原生编译/CTest已由用户实际执行并报告通过；代理自行执行全部项目/其它模块仍须独立授权，不因此开放其他模块。正式DEC-005数值验收、跨模块错误适配/ABI与项目集成仍需独立证据；**DEC-008-01～06的设计决定已作出，但未在低阶叶子与公共Result之间实施或验证桥接**。

按[长期政策](../golden/NO_MATLAB_EXECUTION_POLICY.md)，永久不要求MATLAB/Octave/L1。原论文、源码、MAT及旧审查记录保持不变；仅在本文件及PROGRESS/AUDIT维护当前的局部进展与真实限制。
