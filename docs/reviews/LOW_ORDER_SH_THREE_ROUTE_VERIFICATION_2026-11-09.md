# 低阶球谐三路线与原 MATLAB 公式专项结论

> 现行专项核验日期2026-11-10，起点 `9af16680a2b73754ad3b09825faf2655ea45a052`。只讨论论文实际使用的 l=0,1,2 九实基与旋转块；不讨论离散拟合、Adaptive/KvD运动求解、采样、Session或工程实现。本轮仅理论/符号核验，未修改原 MATLAB/C++或运行原生测试。
>
> 原2026-11-09报告（包括当时权限、论文页图记录、初始C++状态及范围以外的历史段落）完整保存在[9af1668固定版本](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/9af16680a2b73754ad3b09825faf2655ea45a052/docs/reviews/LOW_ORDER_SH_THREE_ROUTE_VERIFICATION_2026-11-09.md)。本文不把其中旧NOT_RUN或全工程待办作为本专项卡点。

## 1. 最终裁定与六项目标

**三条计算路线在明确同一基、同一系数排序与同一旋转作用后，l=0/1/2 精确一致。** 但不能据此宣称两个原 MATLAB 文件可以不作任何约定转换而直接混用：`kugelflfunc2.m` 含 CS 相位，`SHRotate.m` 的实递推对应去CS相位的基；对同一个物理矩阵 A 和下述函数作用，必须有明确的相位共轭。原文件只读，数学更正意见不等于已修改源码。

| 目标 | 本轮结论 | 理论结案含义 |
|---|---|---|
| SH-01 直接定义 | 九个源CS基和论文正相位基均已逐项推导，归一化、m顺序与转换明确 | 已完成数学基准，不从递推输出反填 |
| SH-02 实递推 | 采用源码的正确V正号、1998更正的P边界，全部1+9+25元素与定义一致 | 原IR98 Table2残留减号另列，不能原样采用 |
| SH-03 复递推 | Choi一阶F+iG、二阶内部/边界递推，经W D W†及相位转换，与定义及IR一致 | 系数变换与基值变换的共轭方向明确 |
| SH-04 MATLAB九基 | `kugelflfunc2.m` 在声明输入域下与CS定义严格等价 | 合法相位差，不是九基计算错误 |
| SH-05 MATLAB旋转 | `SHRotate.m` 在无CS实基内低阶递推正确；与源CS系数混用需 S_l R_l S_l | 已定位并证明约定差异与数学修正，原MATLAB未改 |
| SH-06 交叉结案 | 一般非零实四元数证明全部SO(3)块等价，并给出不换基的明确反例 | 已完成限定理论判定；不是无条件原代码/浮点正确性声明 |

这里“理论结案”按当前AUDIT_PROCESS的标准包含**明确判定差异及给出正确公式**，不是强行把所有原式判为正确；没有通过改写六项目标来排除发现的问题。

## 2. 固定来源、范围与符号

三篇论文的原PDF身份、印刷页图与式号沿用已存[论文源审计](SH_PAPERS_SOURCE_AUDIT_2026-11-09.md)：IR96 DOI 10.1021/jp953350u；IR98 DOI 10.1021/jp9833350（附件页脚标识另有历史差异）；Choi等1999 DOI 10.1063/1.480229。本轮复用该原文/页图审阅记录与现有公式转录，没有声称重新上传或重新逐页阅读PDF。

原 MATLAB blob 固定为：

| 原文件 | Git blob | 本轮只读定位 |
|---|---|---|
| `origin/egomotion_estimation_dense_flow_fields_matlab_scripts/help_functions/kugelflfunc2.m` | `edc0caa1f88bf91d2a189e4d3a283334f809b905` | 9行legendre，12～14归一化，16～21三种m分支 |
| `origin/egomotion_estimation_dense_flow_fields_matlab_scripts/help_functions/SHRotate.m` | `7b73000411a9294f6cbed2e111bf74ce7fb7d4df` | 30行R0，33～45一阶，49～55高阶循环；M58、U87、P91、V108、W127起 |

数学域为整数 l=0,1,2、−l≤m≤l，标量实角 0≤theta≤pi、phi实数，单位方向 d=(x,y,z)=(sin(theta)cos(phi),sin(theta)sin(phi),cos(theta))。旋转 A∈SO(3)，固定系数列向量作用：

\[
f_{new}(d)=f_{old}(A d),\qquad a_{new}=D(A)a_{old},\qquad
b(A d)=D(A)^\top b(d).
\]

改变为主动旋转或逆作用时必须相应换A/Aᵀ，不能只换一处转置。

## 3. SH-01/04：从定义直接核对原九基

令 k=|m|，使用 Rodrigues 定义及CS相位：

\[
P_l^k(u)=(-1)^k(1-u^2)^{k/2}
\frac{1}{2^l l!}\frac{d^{l+k}}{du^{l+k}}(u^2-1)^l,
\quad N_{lk}=\sqrt{\frac{2l+1}{4\pi}\frac{(l-k)!}{(l+k)!}}.
\]

MATLAB默认 `legendre(l,u)` 是带CS相位的未归一关联Legendre；官方定义见 [MathWorks legendre](https://www.mathworks.com/help/matlab/ref/legendre.html)。原代码先取第k+1项，再乘N，m<0乘sqrt(2)sin(k phi)、m>0乘sqrt(2)cos(k phi)、m=0不乘sqrt(2)，恰好给出：

| (l,m)，原有效项顺序 | 原 MATLAB/CS 实基 | 对应论文正相位项 |
|---|---|---|
| (0,0) | 1/sqrt(4pi) | g0 |
| (1,−1) | −sqrt(3/(4pi)) y | −f2 |
| (1,0) | sqrt(3/(4pi)) z | f3 |
| (1,+1) | −sqrt(3/(4pi)) x | −f1 |
| (2,−2) | sqrt(15/(4pi)) xy | h5 |
| (2,−1) | −sqrt(15/(4pi)) yz | −h3 |
| (2,0) | sqrt(5/(16pi))(3z²−1) | h1 |
| (2,+1) | −sqrt(15/(4pi)) xz | −h2 |
| (2,+2) | sqrt(15/(16pi))(x²−y²) | h4 |

原文件第5行注释把l/m的degree/order文字对调，但实际运算 `legendre(l,...)` 与abs(m)索引的数学身份正确；不能把注释用词当作数值公式错误。

完整Rodrigues九项与上表的差精确为0；另与标准CS复球谐的sqrt(2)Re/Im组合独立核对，九差也为0。计算先在0<theta<pi进行，两极由这些笛卡尔多项式连续延拓；北/南极只有m=0项非零，l=1在南极变号，l=0/2不变。

论文顺序 [g0,f1,f2,f3,h1,h2,h3,h4,h5] 与源m序的关系为

\[
b_{CS}=(g_0,-f_2,f_3,-f_1,h_5,-h_3,h_1,-h_2,h_4)^\top.
\]

对应任意源系数c的论文系数为 `(c00,−c1,+1,−c1,−1,c1,0,c2,0,−c2,+1,−c2,−1,c2,+2,c2,−2)`，此处每个c带(l,m)标签而非数组算式；Wolfram同时验证9项往返和任意系数的同一标量场。对3×5布局的简明赋值沿用 harmonics.md §1，不涉及本专项以外的系数拟合。

## 4. SH-02：实球谐递推与原 M/U/P/V/W

为避免把“九个基函数的数值计算”与“旋转矩阵的阶次递推”混淆：IR和Choi递推的对象是每阶旋转块，不是直接替换上述角函数定义。路线A的旋转参考由基多项式的二次张量独立获得。

令无CS的m序为一阶 `(y,z,x)` 和二阶 `(xy,yz,h1,xz,x²−y²)`，每项都乘第3节对应的正归一系数。P将xyz重排为yzx，原SHRotate第33～43行精确等于

\[
B=P A^\top P^\top,\qquad
P=\begin{bmatrix}0&1&0\\0&0&1\\1&0&0\end{bmatrix}.
\]

故对IR/Choi论文的轴矩阵应使用 `R=Aᵀ`。这是明确的输入/系数作用约定，不是漏转置。

对二阶、m,n∈[−2,2]，源码M采用

\[
\Delta_n=\begin{cases}12,&|n|=2,\\4-n^2,&|n|<2,\end{cases}
\quad u=\sqrt{(4-m^2)/\Delta_n},
\quad v=\frac{1-2\delta_{m0}}2
\sqrt{\frac{(1+\delta_{m0})(1+|m|)(2+|m|)}{\Delta_n}}.
\]

原w对全部25个(m,n)精确为0。M第72～82行先检查系数是否非零才调用U/V/W；对u=0的极端行，这避免访问不存在的一阶行；l≤2不会调用W，本专项不为测试W而扩展高阶。

P(第91～106行)在n=−2访问前阶列−1/+1并相加，n=+2访问+1/−1并相减；内列用B_i0 B_an。U=P(0,l,m,n)；V的m=0、m>0、m<0分支完整对应原代码，特别是第123行使用 `sqrt(1+delta_(m,-1))`。本轮把这些原分支逐式翻译到Wolfram，不调用MATLAB。

独立定义基准：把二阶无CS基写作 b_i(d)=dᵀQ_i d，则

\[
D^{(2)}_{ij}(A)=\frac{8\pi}{15}\operatorname{Tr}(Q_i A^\top Q_j A).
\]

Q_i从每个定义多项式Hessian/2直接求出，不调用递推。原MATLAB二阶M/U/P/V公式与此参考的25项一般旋转差全0；既有 IR96 p.6347 Table3 全25元素转录也与该参考完全一致（其常数1在齐次清分母时为(q·q)²）。

### 4.1 1998印刷表的残留差异必须保留

已存论文页图审计记录明确：IR98 p.9100 Table2的负m V末因子印作 `sqrt(1−delta_(m,-1))`，原MATLAB第123行和IR96 Table3对应正确为加号。本轮一般符号核验再次拒绝减号版本；在 A=Rx(pi/4)、l=2,m=−1,n=0，正确值sqrt(3)/2，印刷减号给sqrt(3)/4。

**因此本报告不是认定1998表每个印刷符号均正确。** 采用有独立定义、IR96显式表及Choi路线一致支持的加号，并列明印刷冲突；不伪称作者另发布了勘误，不修改原PDF。1998对P边界±(l−1)等已明确的修订仍应采用。

### 4.2 degree=0的实际返回容器

原SHRotate第30行设置R{1}=1，但第33～45行无条件创建一阶块；即使degree=0，也有额外R{2}。零阶公式仍正确，但不能宣称“返回单元数恰好degree+1”。degree=1不进入二阶循环，degree=2计算完整5×5块。这里只记录原函数行为，未修改源代码。

## 5. SH-03：Choi复递推、W与共轭的准确角色

复一阶使用Choi1999式(5.4)/(5.5)的F+iG，输入轴矩阵R=Aᵀ。二阶内部列用(6.1)，两端列用(6.9)/(6.14)，权重来自(6.2)/(6.3)/(6.10)/(6.11)；与实递推为独立运算式。零权重先短路，避免访问不存在的前阶m。

按m从−l到+l，对每个正整数m，W在行/列(−m,+m)的非零2×2块为

\[
W_{\{-m,+m\},\{-m,+m\}}=
\frac1{\sqrt2}\begin{bmatrix}-i&i(-1)^m\\1&(-1)^m\end{bmatrix},\quad W_{00}=1.
\]

这里必须分清**系数**与**基值**。对于标准CS复基Y和无CS实基b，

\[
b=\overline W Y,\qquad a_{real}=W a_{complex},\qquad
D_{real}=W D_{complex} W^\dagger.
\]

Wolfram另行检查W1、W2酉性及上式两个基值关系，4/4为True；不能把基值关系误写成b=WY，也不能以普通转置代替共轭转置。转换到源CS实基时再作 S_l D_real S_l。一般SO(3)下Choi的9个一阶元素和25个二阶元素均与直接定义及IR一致，且虚部精确抵消。

## 6. SH-05：两个原MATLAB基约定不能不加说明地混用

源CS基与无CS基满足 b_CS=S_l b_noCS，其中

\[
S_0=(1),\quad S_1=\operatorname{diag}(-1,1,-1),\quad
S_2=\operatorname{diag}(1,-1,1,-1,1).
\]

**在同一A、同一方向d以及 f_new(d)=f_old(A d) 下，原CS列系数所需的旋转必须为**

\[
\boxed{D_{CS}^{(l)}(A)=S_l\,R_{\text{SHRotate}}^{(l)}(A)\,S_l},\qquad l=0,1,2.
\]

这是数学换基，不是本轮实现代码。Wolfram对任意非零实四元数验证：一阶3个基函数作用多项式差为0；二阶5个作用多项式差为0；与直接参考的矩阵全元素一致。原R不作相位共轭时一般不满足源CS基的同一场作用。

最小反例保留旧脚本：f_old(d)=2+z、A=Rx(pi/2)、d=e_y。直接定义f_old(A d)=3，未经换基的原SHRotate结果在原CS基内为1（场2−y）；共轭后为正确场2+y。绕z的特例对该相位差不敏感，不能取代一般检查。

**判定范围**：这证明两个函数在相同坐标解释下直接混用的约定缺口；本轮没有进入其它业务调用者判断其是否已另行变换坐标，因此不宣称论文所有实验无效。原SHRotate的无CS递推本身没有因这一组合问题变错，原kugelflfunc2的CS基也不是错误。真实问题是缺少显式统一，而非“所有实球谐都天然同号”。

## 7. 一般SO(3)证据与本轮实际结果

固定既有旋转脚本用四个任意实q构造齐次矩阵N，s=q·q，验证NᵀN=s²I、detN=s³，A=N/s。所有非零实q覆盖SO(3)，包括pi旋转。二阶表达是A的齐次二次式，清除非零s²后比较全部多项式，而不是随机采样。

本轮先按起点提交重新读取/执行三份旧脚本，均返回allExact=True；随后只扩展现有布局脚本补原MATLAB逐式验证，没有新建验证文件：

| 实际检查 | 本轮结果 |
|---|---|
| 定义与论文九基、低阶基值递推 | 各9差全0 |
| IR与定义的R1/R2 | 9/25元素差全0 |
| Choi与定义的R1/R2 | 9/25元素差全0 |
| Rodrigues与原CS九基；复球谐与原CS九基 | 各9差全0 |
| 原SHRotate低阶源分支与直接二阶张量 | 25差全0 |
| IR96 Table3与直接定义 | 25差全0 |
| CS相位共轭后的基函数作用 | 一阶3差、二阶5差全0 |
| 原系数到论文序转换 | 9项往返差0、任意标量场差0 |
| l=2全部w与短路索引 | 25个w精确0；已调用的前阶行均有效 |
| 旧直接混用的反例 | 原结果1，指定定义3；被正确识别而非忽略 |
| 扩展布局脚本 | 13/13检查、allExact=True |
| 系数/基值W对偶与酉性单独复核 | 4/4检查True |

执行器Wolfram Language 15.0.1，精确符号算术。最初探索调用使用大写局部符号触发工具的undefined提示；最后从**已提交的全部局部变量脚本**重新读取执行，13/13通过且无该提示，不把初次提示当作唯一证据。

固定文件身份：

| 工件 | Git blob / SHA-256 |
|---|---|
| `verify_low_order_basis.wl` | blob `498dda40eef44aeb4e7e9e914ce73d891f330d88`；SHA256 `b361450ee30d6ef0f3763dd71fe09f0c7e3a67955e2dad6ecb2df09b80a10565` |
| `verify_low_order_rotations.wl` | blob `e40ca439254b8860a03d0f265eb9bf1959c6acc5`；SHA256 `c49008db0a1e1c90d9ec588b80aba32283fde0e22f4a7e070f6146ccba5d9a51` |
| 本轮扩展 `verify_low_order_layout.wl` | 提交 `7f083e2e51d2f30126a9f6e72c27c196240fe977`，blob `644eaa967316d52a3769ef1508f2db80d6607c89`；SHA256 `eb6871fe023e0d6c694ec5b81d75fd78bb75da6230237807e3e3d7fca04d4468` |
| `kugelflfunc2.m` UTF-8 SHA256 | `8801e0439ca0ee60ed074ccf6ae944523f20b3d69809000af1220aaa8dba43de` |
| `SHRotate.m` UTF-8 SHA256 | `05494d975fd4eae62ed8531e8e7ffbedafaa4aa535b99059c496f7a67b5991b9` |

旧 `verification_results_2026-11-09.json` 作为历史原始输出保留，不把其中旧NOT_RUN改写成今天的结果；现行输出由本报告与已提交脚本的真实重放记录。用户旧SH 3/3 CTest仅作既有数值佐证，本轮没有编译或修改任何C++。

## 8. 结案与未证边界

六项专项的**理论核对和数学差异裁定已完成**，原97项全工程AF和性能/ABI等不再阻塞。允许作出的结论是：低阶九基正确；采用定义所确认的勘误/相位/方向后A=B=C；两个原MATLAB的基约定不同，必须显式换基，不能断言其未经转换的直接组合无误。

没有证明或实施：任意高阶、全IEEE浮点行为、源码运行时异常、所有调用者坐标约定、全工程集成、原MATLAB代码修正。未执行MATLAB/Octave/C++，未更改原文件，未扩展到离散拟合或运动估计理论。第二次专项复核也只检查AUDIT规定的13个对象与六个目标，不增加外围任务。
