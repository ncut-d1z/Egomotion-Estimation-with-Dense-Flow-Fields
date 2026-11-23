# Adaptive 均值归一、几何条件数与平移归一的分层理论审计

> 数学专题 v0.2，未冻结、非生产实现。关联 AF-044/051/054/055/060；既有 TC-F-ADAPT-001/002/003、TC-DC-ADAPT-001 的数学输入与诊断义务，不新增 TC。以下是**根据原源码开展的本轮推导**，不声称原论文已经给出这些定理，也不授权改写原算法或新增 C++。
>
> 原始依据：只读 `origin/egomotion_estimation_dense_flow_fields_matlab_scripts/help_functions/oneit_adaptiv_matched_filter.m`，Git blob `48e51bcd52fb8f27cf35b3fce38b28507af36e55`。旧源球谐相位沿用 [adaptive_mfa.md](adaptive_mfa.md) 与 [harmonics.md](harmonics.md)。实际验证脚本为 [verify_zero_mean_conditioning_families.wl](../../verification/adaptive/verify_zero_mean_conditioning_families.wl)，完整实际结果为 [zero_mean_conditioning_evidence.v1.json](../../verification/adaptive/zero_mean_conditioning_evidence.v1.json)。Wolfram 精确等式及70位数 SVD 合计25/25检查通过，**不是 C++/CTest 或 MATLAB 运行**。

## 1. 固定原变量身份，避免将 raw 与翻号后的 near 混为一谈

令 \(r_i\) 为调用 `near_aus_koeff` 后、尚未符号翻转的 **raw near**；\(s=\langle r_i\rangle\)。用 \(r_i^{\rm flip}\) 表示执行原 `if s<0: near=-near` 之后的值，用 \(\mu_i\) 表示再除以**原始** \(s\) 后的值。源码在 \(s\ne0\) 时精确等价于

\[
r_i^{\rm flip}=\operatorname{sign}(s)r_i,\qquad
\mu_i=\frac{r_i^{\rm flip}}s=\frac{r_i}{|s|},\qquad
\langle\mu_i\rangle=\operatorname{sign}(s).
\]

这澄清了旧文档“不能换成 near/abs(s)”的变量歧义：**原 raw r 除以 |s| 与源码相同；已经翻号的 r_flip 再除以 |s| 在负分支则不同**。本式只是对既有源语义的数学重写，不提出修改源负均值分支。上一轮的恒等式仍成立：

\[
M_1-\langle\mu_i(I-d_i d_i^\top)\rangle
=(1-\langle\mu_i\rangle)I,
\]

所以正分支差0、负分支差2I。变量身份澄清不推翻该结论。

### 1.1 s=0 是定义域失败，不是“先求一个 M 看看”

取六个有序方向 \((e_x,-e_x,e_y,-e_y,e_z,-e_z)\)，原 CS 系数只取

\[
C_{00}=a\sqrt{4\pi},\qquad C_{1,-1}=-b\sqrt{4\pi/3},
\]

便严格重建 \(r_i=a+b y_i\)。在 \(a=0,b=1\) 时，raw near 为 \((0,0,1,-1,0,0)\)，均值为0而输入不全为0；\(a=b=0\) 又给全零输入。原归一除法两者都无定义。不能把某参数族的有限极限当成原源码在 s=0 的求值，不能声称实际 MATLAB 必然抛出哪一种错误/警告，因为本次没有运行它。

原 `nulldiv=1e-11` 只位于**6×6求解及平移归一之后**的 near_new 分母，不保护这个初始的 `near/s`，也不保护 `t_raw/||t_raw||`。

### 1.2 有限扰动式与尺度无关的抵消量

假设 \(s\) 与 \(s+\delta s\) 非零且同号，\(\sigma=\operatorname{sign}(s)\)、\(\delta s=\langle\delta r_i\rangle\)。直接相减得到

\[
\widetilde{\boldsymbol\mu}-\boldsymbol\mu
=\frac{\delta\boldsymbol r-\sigma\boldsymbol\mu\,\delta s}
{|s|+\sigma\delta s}.
\]

当 \(|\delta s|<|s|\) 时，对任一向量范数有

\[
\|\widetilde{\boldsymbol\mu}-\boldsymbol\mu\|
\leq\frac{\|\delta\boldsymbol r\|+\|\boldsymbol\mu\|\,|\delta s|}
{|s|-|\delta s|}.
\]

这是同一符号分支的界；跨过零均值时不能沿用。微分形式为

\[
D\boldsymbol\mu=\frac1{|s|}
\left(I-\frac{\sigma}{N}\boldsymbol\mu\boldsymbol1^\top\right).
\]

若 \(r_i\ge0\)、\(s>0\)，则 \(0\le\mu_i\le N\)：全体正近度一起变小不会造成正负抵消。可另记录无量纲诊断 \(\chi=|s|/\langle|r_i|\rangle\)（分母非零），由三角不等式 \(0\le\chi\le1\)；全正场为1，正负抵消可趋0。**本轮不为 chi 或 s 指定新的生产阈值，也不把此诊断替换原归一公式。**

## 2. 正尺度近零，M仍可完美条件化

在上述六个方向取 \(r_i=a>0\)。则 \(s=a\)、\(\mu_i=1\)，源四块给出

\[
M=\frac23I_6,\qquad\kappa_2(M)=1
\]

对任意 \(a>0\) 成立。用原几何公式生成

\[
p_i=-a(e_x-d_{ix}d_i)-\omega\times d_i,
\quad\omega=(1/5,-1/7,1/11)^\top,
\]

源6×6解精确为 \(z=(a,0,0,1/5,-1/7,1/11)^\top\)，归一后平移为 \(e_x\)。因此“s很小必然使6×6近奇异”不成立。

但这并不表示全流程抗绝对噪声：若原始平移解由 \(ae_x\) 变为 \(ae_x+\eta e_y\)，归一化方向偏差恰为 \(\arctan(|\eta|/a)\)。M条件数与原始平移范数必须分开记录。\(a=0\) 时依旧先遇到源码初始均值除零，不能借助有限的 M 极限宣布可执行。

## 3. 抵消近零：行列式与全部特征值恒定，条件数仍发散

在同一六方向取 \(r_i=a+b y_i\)、\(a>0,b\ge0\)。源均值为a，归一 near 为 \(1+(b/a)y_i\)。四块精确得到

\[
M=\begin{bmatrix}
\tfrac23 I&0\\
\tfrac{b}{3a}[e_y]_\times&\tfrac23 I
\end{bmatrix}.
\]

对所有非零a，\(\det M=64/729\)，六个特征值全为 \(2/3\)。该矩阵一般不是正规矩阵，**不能把最大/最小特征值之比当成其2-范数条件数**。

令 \(\gamma=|b|/(2a)\)，则 \(M^\top M\) 的特征多项式由Wolfram验证为

\[
(x-4/9)^2\left[x^2-\frac49(2+\gamma^2)x+\frac{16}{81}\right]^2.
\]

所以 \(M\) 的奇异值为 \(2/3\)（两重）及

\[
\frac23\frac{\sqrt{\gamma^2+4}\pm\gamma}{2}
\quad\text{（各两重）},
\]

从而

\[
\boxed{\kappa_2(M)=\left(\frac{\sqrt{\gamma^2+4}+\gamma}{2}\right)^2
\sim\frac{b^2}{4a^2}}\quad(a\to0^+,\ b>0\text{固定}).
\]

| a，b=1 | det M | kappa2，精确公式经70位SVD交叉检查 |
|---:|---:|---:|
| 0.1 | 64/729 | 26.96291201783626 |
| 0.001 | 64/729 | 250001.99999600003 |
| 0.000001 | 64/729 | 250000000002 |

这里 \(0<a<b\) 时有一个或多个负 raw near，属于**有符号/非正深度的数学输入**，不得把这条发散结论直接外推为所有物点正深度的场景。此区间 \(\chi=3a/(2a+b)\) 可明确辨别均值抵消；其值不是额外批准门槛。

### 3.1 一般矩阵不是对称阵：LDLT 不具有无条件求解资格（2026-11-11）

原源码的两块对角矩阵 \(M_1,M_4\) 本身对称，两个非对角块则为 \(M_2=[-\langle d\rangle]_\times\)、\(M_3=[\langle\mu d\rangle]_\times\)。使用叉积矩阵恒等式 \([v]_\times^T=-[v]_\times\)，严格得到

\[
M=M^T\quad\Longleftrightarrow\quad M_3=M_2^T
\quad\Longleftrightarrow\quad\langle\mu d\rangle=\langle d\rangle
\quad\Longleftrightarrow\quad\boxed{\langle(\mu-1)d\rangle=0}.
\]

由 \(s=\langle r\rangle>0\) 归一只可保证 \(\langle\mu\rangle=1\)，**不能推出** \(\langle(\mu-1)d\rangle=0\)；因此“均值=1/正近度”并不保证 M 对称。本文§4的全常数 \(\mu=1\) 四向量样例满足该额外条件，属于特例，不能把其中的对称性推广到任意场。

完整的正近度反例见[Adaptive LLD §2.2.1](adaptive_mfa.md)：六条 \(\pm e_x,\pm e_y,\pm e_z\)，raw near \(r(d)=1+x/2\)，得到 \(s=1\)、\(M_2=0\)、\(M_3=[e_x]_\times/6\ne0\)，但 \(\det M=64/729\) 且原线性解有定义。通用 C++ 求解器须支持**一般非对称**线性系统，不能将自伴矩阵的 LDLT 作为默认替换，也不能简单对 M 对称化。若要将某个子集单独使用 LDLT，至少先证明这一附加块条件并审查分解性质/条件数/失败语义；不由此认定现有 Adaptive 公共接口或 CTest 已实现。

## 4. 真正正深度的视场退化：s=1也不能保护M

取四个方向，\(0<h\le1/2\)，设 \(q=2h/(1+h^2)\)、\(c=(1-h^2)/(1+h^2)\)：

\[
(q,0,c),\ (-q,0,c),\ (0,q,c),\ (0,-q,c).
\]

四条方向都单位，全部 raw near 恒为1，可由原 \(C_{00}=\sqrt{4\pi}\) 表示；因而s=1、chi=1。这是合法的正距离几何配置。

令 \(A=\operatorname{diag}((1+c^2)/2,(1+c^2)/2,q^2)\)、\(K=[c e_z]_\times\)，源矩阵为

\[
M=\begin{bmatrix}A&-K\\K&A\end{bmatrix}=M^\top.
\]

Wolfram从原四块重新构造矩阵，并验证完整特征多项式。三个特征值分别各两重：

\[
\lambda_{\min}=\frac{2h^4}{(1+h^2)^2},\quad
\lambda_{\rm mid}=\frac{4h^2}{(1+h^2)^2},\quad
\lambda_{\max}=\frac{2}{(1+h^2)^2}.
\]

在本声明域上严格有 \(0<\lambda_{\min}\le\lambda_{\rm mid}\le\lambda_{\max}\)，所以

\[
\boxed{\kappa_2(M)=h^{-4}},\qquad
\det M=\frac{256h^{12}}{(1+h^2)^{12}}.
\]

| h | s | kappa2 精确值 |
|---:|---:|---:|
| 1/10 | 1 | 10,000 |
| 1/100 | 1 | 100,000,000 |
| 1/1000 | 1 | 1,000,000,000,000 |

弱特征向量可取 \(v=(1,0,0,0,-1,0)^\top\)，满足 \(Mv=\lambda_{\min}v\)：平移x分量与反向旋转y分量的组合变得难分辨。\(h=0\) 时四视线重合，源M秩为2；但此极限不改变旧 near 的均值1。该例从严格正深度输入证明：**均值非零甚至恒1，不等于运动矩阵可稳定求解**。

## 5. M可逆也不保证最终平移方向有定义

在六轴方向和raw near=1时，给定纯旋转光流 \(p_i=-\omega\times d_i\)。源矩阵仍为 \((2/3)I_6\)，均值1，条件数1；源线性解精确为

\[
z=(0,0,0,\omega_x,\omega_y,\omega_z)^\top.
\]

源码随后却将 \(z_{1:3}\) 除以自身范数，所以**平移方向归一无定义**。这里证明的是原表达式的定义域，不是实际 MATLAB 的警告/NaN传播，也不意味着纯旋转下旋转本身不可估。不能把近度或矩阵测试通过，直接当成完整单步成功。

## 6. 一般矩阵判别的来源推导与边界

源 \(M_4=\langle I-d_i d_i^\top\rangle\)。对任意x，

\[
x^\top M_4x=\langle\|d_i\times x\|^2\rangle\ge0.
\]

因此对于单位方向、有限非空样本，M4可逆当且仅当不存在非零x与所有方向共线（正负反向也视为同一直线）。在M4可逆时，用精确分块消元可得

\[
\det M=\det M_4\det S,\qquad S=M_1-M_2M_4^{-1}M_3.
\]

这只给出精确算术下的等价奇异性条件；不能仅以 det(M) 或 det(S) 的绝对大小制定浮点阈值，第3节的恒定行列式反例说明了原因。文中S是理论 Schur 补，不是新增生产求解路线；本轮未修改任何矩阵求解器。

## 7. 对既有测试与验收目标的具体整改

| 必须区分的数学情形 | 已形成证据 | 尚不能宣称 |
|---|---|---|
| s=0，raw不全为0或全为0 | CS系数可重建的精确输入，源码均值归一无定义 | MATLAB实际警告或C++错误码测试已执行 |
| s趋零而raw全正且同比缩小 | M=(2/3)I、条件数1；raw平移小，方向对绝对扰动敏感 | 所有小s都是矩阵退化，或小s没有风险 |
| 有符号抵消造成s趋零 | M行列式及特征值恒定、奇异值条件数发散的完整参数证明 | 已覆盖正深度实际场景或所有病态输入 |
| raw=1但四方向窄锥 | M完整谱、条件数h^-4及正距离例 | 均值有效即可放行，或已批准某条件数阈值 |
| 纯旋转、M良态、s=1 | z_t=0，最终方向归一无定义 | 旋转不可估，或整个源单步返回值已取得 |

映射既有 TC-F-ADAPT-001/002/003、TC-DC-ADAPT-001；Session的失败不提交原则保留，但此轮没有编程或运行Session。AF-044/051/054/055/060仍需生产实现、实际C++负例/逐节点验证、跨帧状态和正式DEC-005/008。理论上应分别记录 raw near、s、符号、均值抵消量、M的奇异值尺度/条件数、raw t及其范数；**不在本轮引入未经批准的固定阈值、代替原负支、取消失败或改变状态提交**。

本专题三参数族的精确谱与定义域结论属于本轮新增推导；不是先前原论文的直接引文。结果JSON保留25项实际Wolfram检查及6组高精度SVD数值比较。所有证明域都明列；不以有限样本或某个条件数数字宣称整个算法100%正确。
