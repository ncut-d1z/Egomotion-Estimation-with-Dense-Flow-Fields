# 实球谐、近度拟合及系数旋转——详细设计草案

> **LLD v0.8，未冻结、未实现、未数值验收。** 来源：winkelausvektor、kugelflfunc2、near_aus_koeff、koeff_aus_near、transfor_koeff、SHRotate及用户已提供的1996原文、1998勘误、1999论文。材料已齐，关键公式/表格静态审阅见[论文审计](../reviews/SH_PAPERS_SOURCE_AUDIT_2026-11-09.md)，不再等待上传；一般SO(3)语义、独立L3和真实C++/CTest仍待完成。关联REQ-F-SH-001/002、REQ-D-SOURCE-002、REQ-NF-COPYRIGHT-001及P7.1–P7.6/P7.10。

**既定路线不变**：IR96+IR98为实数递推主要依据，纯Eigen、直接/间接禁止sdla；Don Williamson仅辅助代码参考。1999 F/G路线用于独立交叉参考候选，不擅自替换主算法或扩大到l>2。材料到位不等于H-10/H-11开放。项目根MIT已选，正常原创开发不因重复版权审批阻塞；实际复用上游文件保留必要notice。

## 1. 坐标系、角度与原实球谐表达式

对原单位方向d=(x,y,z)，保持源`winkelausvektor`：

\[
\theta=\arccos z,\qquad\phi=\operatorname{atan2}(y,x),
\]

单位rad；非单位、非有限及acos越界的拒绝/校正规则须明确，不能静默改变输入。令q=|m|，`P_l^q`采用原MATLAB `legendre(l,cos(theta))`默认相位；不是任意库的同名函数。

\[
A_{lq}=\sqrt{\frac{2l+1}{4\pi}\frac{(l-q)!}{(l+q)!}},\qquad
Y_{lm}^{real}=\begin{cases}
\sqrt2 A_{lq}P_l^q(\cos\theta)\sin(q\phi),&m<0,\\
A_{l0}P_l^0(\cos\theta),&m=0,\\
\sqrt2 A_{lq}P_l^q(\cos\theta)\cos(q\phi),&m>0.
\end{cases}
\]

**此处记录的是原`kugelflfunc2.m`，不把论文的实基命名直接视为同一相位约定。** IR96式(5.3)/CIGR99式(8.15)与当前实基的一阶符号关系、IR96负m写法的核对边界见§7.1.2及审计记录。不得未经独立验证宣布所有基变换已一致。

### 1.1 3×5系数九个有效索引

| l | m | MATLAB 1-based(row,col) | C++ 0-based(row,col) | 方位分支 |
|---:|---:|---|---|---|
| 0 | 0 | (1,1) | (0,0) | 无φ |
| 1 | −1 | (2,1) | (1,0) | sinφ |
| 1 | 0 | (2,2) | (1,1) | 无φ |
| 1 | +1 | (2,3) | (1,2) | cosφ |
| 2 | −2 | (3,1) | (2,0) | sin2φ |
| 2 | −1 | (3,2) | (2,1) | sinφ |
| 2 | 0 | (3,3) | (2,2) | 无φ |
| 2 | +1 | (3,4) | (2,3) | cosφ |
| 2 | +2 | (3,5) | (2,4) | cos2φ |

其余位置不参与l≤2求和；原初始化C(1,1)=1，其余零。九项表示与八项非零阶信息不同；零阶系数1不等于重建近度恒1，后者需要a00=√(4π)。

## 2. 系数—近度重建与逐项拟合

原`near_aus_koeff.m`逐点：

\[
\mu(d_i)=\sum_{l=0}^{2}\sum_{m=-l}^{l}a_{lm}Y_{lm}^{real}(d_i).
\]

原`koeff_aus_near.m`对每个(l,m)独立：

\[
a^{num}_{lm}=\frac{4\pi}{N}\sum_i\mu_iY_{lm}(d_i),\quad
a^{den}_{lm}=\frac{4\pi}{N}\sum_iY_{lm}(d_i)^2,\quad
a_{lm}=a^{num}_{lm}/a^{den}_{lm}.
\]

不改为九系数联合最小二乘。虽4π/N在代数上可约掉，实际乘除次序变化仍需真实浮点对照；分母零、N=0和非有限不得伪装成功。

**AF-090展示职责分离**：`illustrations/spherical_harmonic_illustration.m`使用未归一化Legendre与cos项，再做`rho=5+3yy/max(abs(yy))`，不调用本模块重建/拟合，也没有负m的sin分支。其行为见[分析LLD §3.1](analysis_plotting.md)；TC-F-AN-002不能替代TC-F-SH-001/002。

### 2.1 候选接口

`RealSphericalHarmonics::evaluate`、`reconstruct`、静态`validDegreeOrder`与`HarmonicFitter::fit`的**唯一完整声明及预计头文件**见[专题LLD §5](SH_ROTATION_IVANIC_RUEDENBERG.md)。公共Result、NumericalErrorCode、DirectionSet、3×5 HarmonicCoefficients归contracts/domain，本文件不重复定义ABI。有限角、l∈{0,1,2}、|m|≤l、方向/shape及失败保持策略须在接口冻结时明确。

## 3. l≤2系数旋转

原`SHRotate(r,2)`返回R0=1、R3、R5；`transfor_koeff`保持a00并分别左乘原序列系数列：

\[
a'_{00}=a_{00},\qquad a'_1=R_3(r)a_1,\qquad a'_2=R_5(r)a_2.
\]

一阶m序列为(−1,0,+1)，二阶为(−2,−1,0,+1,+2)。不能把R3直接替换为通常xyz顺序的物理矩阵。

### 3.1 递推与验证边界

R0为常数；R3按§7.1原实际九项；R5采用IR96/IR98经来源冲突检查后的候选。**IR98 Table2负m的V平方根存在印刷冲突**：候选加号来自原源码、IR96 Table3和明确代数反例，不能声称是无争议逐字照抄勘误。全部P/U/V/W数学分支、零系数短路与l=2的w恒零见[专题§4](SH_ROTATION_IVANIC_RUEDENBERG.md)。

需要全9/25元素、系数前后、基函数重建、x/y/z与不交换复合旋转的独立参考；仅正交性、往返或z轴特例不足。一般方向的物理语义不能由“源矩阵已复刻”自动推出。

### 3.1.1 纯Eigen和来源约束

| 项目 | 保留合同 |
|---|---|
| 数学材料 | IR96、IR98及CIGR99均已收到；原式/勘误/源码映射见审计，不再以缺PDF停工 |
| 主实现路线 | IR96/IR98实数递推、纯Eigen，现范围l≤2 |
| 辅助代码 | Williamson的IvanicSHRotate.cpp/.h和SHRotateMatrix.h仅参考；其SHRotation.h为带sdla的应用类，不能直接作为核心API |
| 依赖 | 不引入sdla头、库、源文件、桥接或间接依赖；实际Eigen/Result/API仍待授权实施和编译 |
| 来源/许可 | 根MIT不授予原作者作品；实际复用保留notice，不重复创建版权门禁 |
| 交叉参考 | IR96 Table3非递归25项及1999 F/G→W转换；与被测递推分离，不取其输出反填Expected |

### 3.2 值对象与方法归属

`egomotion::harmonics::HarmonicRotationBlocks`自拥有`Eigen::Matrix3d degree_one`和`Eigen::Matrix<double,5,5> degree_two`；R0不保存可变成员。`HarmonicRotator::blocks/rotate`无跨帧状态，私有`degreeOneBlock`和`detail::IvanicRuedenbergEigen::degreeTwoBlock`声明见专题§5。

3×5系数值归domain，Session管理其历史；本模块不持有MAT/RNG/Session句柄或可变全局缓存。失败携带公共NumericalErrorCode和ErrorDetail，不返回默认矩阵冒充成功。非正交输入是否接受/拒绝、是否允许投影等不得由本次审查擅定。

## 4. 依赖、资源与可复核测试

固定方向至少包含六坐标轴、一般方向(1,2,3)/√14及方位角正负边界；拟合/重建需均匀和非均匀输入。旋转需单位、x/y/z一般角、复合和逆变换，逐项绑定来源SHA、独立参考身份和最终批准容差。没有合法可提交参考文件时保留受控来源及SHA，不复制论文全文进仓库。

### 4.1 target、头文件与测试映射

| 范围 | 候选实体 | 当前限制 |
|---|---|---|
| egomotion_harmonics | HLD既有domain+contracts+Eigen依赖 | 保持27边设计DAG，未创建/编译target |
| 三个公共头 | include/egomotion/harmonics/{real_spherical_harmonics,harmonic_fitter,harmonic_rotator}.hpp | 唯一声明在专题§5；未创建，头自包含/ABI待验 |
| 私有实现 | src/harmonics/对应三个.cpp与detail/ivanic_ruedenberg_eigen.hpp/.cpp | 仅预计路径，无实现，不含sdla |

| 原TC/AC | 候选测试文件/CTest | 独立参考范围 |
|---|---|---|
| TC-F-SH-001 / AC-F-SH-001 | tests/unit/test_harmonics.cpp / egomotion.harmonics.* | 九实基相位、3×5有效项、逐项拟合与重建 |
| TC-F-SH-002 / AC-F-SH-002 | 同上 | R0、R3全9、R5全25、系数/场重建、边界/非交换旋转 |
| TC-D-SOURCE-002 / AC-D-SOURCE-002 | tests/golden/test_mat_archive.cpp / egomotion.mat.*及人工来源审查 | 原源身份、必要notice、依赖闭包；来源审核不等于数值测试 |

增补的[SH-PAPER验证用例](../golden/SH_PAPER_VERIFICATION_CASES.md)只展开既有TC的参考资格，不增加TC、不建立CTest、不修改现有AN-G JSON。

## 5. 当前已解除与仍未解除的卡点

- **已解除材料缺失**：三篇全文、附件身份、指定公式/表格阅读记录已经存在。
- **仍有数学问题**：IR98 Table2印刷冲突的全元素验证；源码转置/现有实基相位与一般物理函数关系必须明确，不以原z90候选覆盖全部SO(3)。
- **仍无真实验收**：独立L3全量数值、生产Eigen实现、真实ABI/CTest、最终DEC-005/008及签署尚缺。
- **授权不变**：H-10/H-11仍适用；论文上传、静态推导、文档提交不自动授权生产编码/运行。

AF-045/046/051/055保持完整开放目标；不是因为缺MATLAB/L1，也不再因为缺论文。

## 6. l≤2九实基的原代码相位多项式

下表保持原`kugelflfunc2`通式展开，不因论文矩阵约定不同而改写：

| (l,m) | Y_lm |
|---|---|
| (0,0) | `1/√(4π)` |
| (1,−1) | `−√(3/(4π)) y` |
| (1,0) | `+√(3/(4π)) z` |
| (1,+1) | `−√(3/(4π)) x` |
| (2,−2) | `+√(15/(4π)) xy` |
| (2,−1) | `−√(15/(4π)) yz` |
| (2,0) | `+√(5/(16π))(3z²−1)` |
| (2,+1) | `−√(15/(4π)) xz` |
| (2,+2) | `+√(15/(16π))(x²−y²)` |

这些是静态解析候选；通式、相位和具体方向数值仍需来源独立的L3及真实测试，不能只验证向量长度。

## 7. 旋转块的具体映射与判别

### 7.1 原SHRotate的R3九项

\[
R_3(r)=\begin{bmatrix}r_{22}&r_{32}&r_{12}\\r_{23}&r_{33}&r_{13}\\r_{21}&r_{31}&r_{11}\end{bmatrix}.
\]

对r=R_z(γ)，原矩阵为`[[cosγ,0,−sinγ],[0,1,0],[sinγ,0,cosγ]]`。这里每一项来自源赋值，不是通常xyz的r。

#### 7.1.1 既有AN-G08的适用范围

r=R_z(+π/2)的R3为`[[0,0,−1],[0,1,0],[1,0,0]]`；现有z90的R5/系数静态候选不改。对该特例，重建角变量φ→φ+π/2可与现有基核对。但它**不证明一般x/y旋转、主动/被动方向或论文相位一致**；[旧z90报告](../reviews/AN_G08_Z90_STATIC_CONVENTION_2026-11-08.md)保持当时限定结论。

#### 7.1.2 新的论文映射与相位区分（AF-094）

令`P=[[0,1,0],[0,0,1],[1,0,0]]`。CIGR99式(8.15)为`P R P^T`，源为`P r^T P^T`，故**论文输入对应r^T**。另外一阶源基为`√(3/(4π))(-y,z,-x)`；针对明确指定的`f_new(d)=f_old(r d)`，候选系数映射为`J R3(r) J`，`J=diag(-1,1,-1)`，一般不同于源码直接R3。

r=R_x(+π/2)、旧系数(0,1,0)时，源左乘给(1,0,0)，按§6重建是负y；指定物理关系是正y。此**静态反例**要求分别记录“与源系数一致”和“与指定物理旋转一致”；不能为了修物理含义悄改原r/基/系数。论文原始负m写法、式(5.3)、Table3与源相位的完整对应仍需审查。候选测试见SH-P03～07。

### 7.2 R5递推与非递归参考

保留IR式(8.1) `R_l=uU+vV+wW`；D在内部列为l²−n²、边界为2l(2l−1)。完整候选公式及P/U/V/W分支归[专题§4](SH_ROTATION_IVANIC_RUEDENBERG.md)，**IR98负m的sqrt(1−δ)不能无说明照抄**。源码使用+δ且与Table3的低阶反例一致；l=2时w恒零，U/V/W必须按零权重短路。

[SH-PAPER §2](../golden/SH_PAPER_VERIFICATION_CASES.md)从IR96 Table3重排出全25元素非递归公式，作为不同计算形式的参考候选；1999 F/G及W转换提供另一条计算路径。两者都未执行，不得称已生成Golden或测试通过。

### 7.3 保留的样本类别

| 类别 | 原检查目标 | 本轮补充 |
|---|---|---|
| SH-R00 | 单位R的34个元素 | 常数/有效位置；不足以发现非一般问题 |
| SH-RZ | z小角和90°，九系数前后 | 保持AN-G08，但不能外推相位正确 |
| SH-RXY | x/y和非交换复合旋转、近度值 | x45勘误因子反例、x90源基符号、完整25项 |
| SH-INV | r与r⁻¹，系数/重建 | 往返只是必要检查，错误正交矩阵也可通过 |
| SH-BAD | 非正交/非有限等失败 | 明确SO(3)工程范围与论文O(3)理论不同，不自动接收反射 |

## 8. 异常、生命周期与接口合同

输入l∈{0,1,2}且|m|≤l；方向有限且单位规则明确；3×5非有效位置不参与运算。拟合分母零、非有限、零方向、非正交矩阵须有显式非成功候选与上下文，不返回默认结果。三个类均无跨帧状态，系数按值拥有、历史归Session。

公共类型及Result以[contracts](contracts.md)为准，具体API/异常/const/复制及真实测试身份由P0.8确认。来源、输入与参考SHA-256、逐元素误差、有效/失败样本、真正批准仍需留证，不将普通版权选择重新列为研发阻塞。

## 附录 A：无需 MATLAB 运行的证据边界

按[长期政策](../golden/NO_MATLAB_EXECUTION_POLICY.md)与[L2/L3/L4矩阵](../golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md)，本模块不要求原MATLAB/Octave/L1。需独立固定输入参考、完整元素/坐标/相位证明、实际授权的C++/CTest和最终批准；未知原运行行为如实标记。本次PDF阅读、文本/静态代数不等于上述数值验收。
