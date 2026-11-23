# 低阶球谐三路线一致性专项整改台账（AUDIT_PROCESS）

> 现行任务仍只有SH-01～SH-06。本轮根据用户“继续理论整改、允许Wolfram、禁止C++”从固定 `9af16680a2b73754ad3b09825faf2655ea45a052` 执行限定核对。原两份MATLAB、全部C++、CMake/CTest/build.sh及MAT保持不变。数学裁定已形成；**已整改表示此处规定的理论核对及更正公式完成，不表示原MATLAB已被修改或其原样组合无条件正确**。

> **2026-11-10 文档定位更新**：本文件仍为**历史低阶球谐SH-01～SH-06限定理论整改**，其6/6结案只涉及此前范围；原同范围的 `AUDIT.md` 固定版本见 [`cb3129b1` 历史快照](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/cb3129b1510f4ffc5fadeb94b8fd67bdff0d5e82/docs/AUDIT.md)。现行 [AUDIT.md](AUDIT.md) 已根据用户本轮新要求改为**全工程文档规范审计规则**，其36份评审对象、整改目标及进度只由 [AUDIT_PROGRESS.md](AUDIT_PROGRESS.md) 管理；**不把原SH-01～06或更早97项全工程旧AF重复计入新文档审核**。这仅修订交叉引用，不改变本文件任何SH目标的原状态或数学结论。

## 1. 专项唯一整改目标（当前有效）

唯一目的未改变：核对论文 *Insect-Inspired Self-Motion Estimation with Dense Flow Fields—An Adaptive Matched Filter Approach* 所用l=0,1,2九实基与旋转计算，比较A直接定义、B实递推、C复递推及两份MATLAB的准确对应。以下六项目标文字沿用范围修订版本，只更新状态和证据。

| 专项ID | 路线/对象 | 唯一整改与验收目标 | 状态 |
|---|---|---|---|
| SH-01 | 直接定义 | 依据数学定义及 Condon–Shortley/实基转换，明确并核对 l=0,1,2 九个 Y_lm 的解析式、归一化、符号及顺序；独立于递推提供基准 | 已整改 |
| SH-02 | 实球谐递推 | 核对 Ivanic–Ruedenberg 1996 及 1998 勘误、l=0/1/2 实球谐旋转递推的全部 R0/R1/R2 元素与一般 SO(3) 作用，逐式与 SH-01 一致 | 已整改 |
| SH-03 | 复球谐递推 | 核对 Choi 等 1999 复球谐递推及复基→实基转换（含共轭、m 排列与相位），使 l=0/1/2 的全部旋转块与 SH-01、SH-02 一致 | 已整改 |
| SH-04 | MATLAB 九基实现 | 对照 origin/.../help_functions/kugelflfunc2.m 的 legendre、相位、归一化、m<0/m=0/m>0 与九个解析定义；说明是否确实等价或定位真实差异 | 已整改 |
| SH-05 | MATLAB 旋转实现 | 对照 origin/.../help_functions/SHRotate.m 的 degree=0/1/2、M/U/P/V/W、索引、矩阵方向、系数布局及相位映射；分别判定与 SH-02、SH-03 和直接基准的对应关系 | 已整改 |
| SH-06 | 三路线最终交叉结案 | 在已核对的九基与 R0(1×1)/R1(3×3)/R2(5×5) 上，用独立一般 SO(3) 数学/符号证据复核 A=B=C；汇总 MATLAB 逐函数结论、论文印刷或1998勘误差异、适用约定及不能证明的范围，形成唯一最终结案判定 | 已整改 |

### 1.1 范围硬边界保持不变

仅审计上述九基、低阶旋转与 `kugelflfunc2.m`、`SHRotate.m` 内的M/U/P/V/W。禁止转入Franz/KvD/Adaptive运动矩阵或迭代、near符号/均值理论、离散重新拟合、采样/RNG/噪声、Session、MAT/CLI/Runner、绘图/性能/全工程审批。不研究任意高阶，不新增无关AF/TC/Golden/文件。后续只有用户明确授权才能扩大范围。

原MATLAB只读；本轮允许符号验证，不允许C++、头文件、构建脚本或生产测试改动。既有Wolfram和论文记录优先复用；旧3/3 SH CTest只是既有数值佐证，不代替一般符号证明。

## 2. 本轮逐目标证据与结案理由

统一完整报告为 [LOW_ORDER_SH_THREE_ROUTE_VERIFICATION_2026-11-09.md](reviews/LOW_ORDER_SH_THREE_ROUTE_VERIFICATION_2026-11-09.md)，现行核验日期2026-11-10。原PDF来源页图记录仍在 [SH_PAPERS_SOURCE_AUDIT](reviews/SH_PAPERS_SOURCE_AUDIT_2026-11-09.md)，未改写。

| 目标 | 实际核对/修订 | 明确裁定 |
|---|---|---|
| SH-01 | 从Rodrigues关联Legendre定义、标准CS复球谐和现有论文九基分别推导；九差全0，明确北/南极连续延拓与原m顺序/论文顺序 | 九基定义正确；原CS负号是合法约定，不可省略 |
| SH-02 | 固定旧IR一般四元数脚本重新执行，一阶9/二阶25差全0；扩展现有布局脚本直接翻译原M/U/P/V并核IR96 Table3全部25项 | 采用正确P边界和源码V加号与定义等价；IR98印刷V减号因子反例被拒绝，不判其正确 |
| SH-03 | 固定Choi复递推与W共轭脚本重新执行，9+25差全0；另核W1/W2酉性及基值b=conj(W)Y、系数a_real=W a_complex，4/4成立 | 统一轴矩阵R=Aᵀ、排序和相位后与A/B完全一致；不能混淆基值与系数的共轭 |
| SH-04 | 原源码9行legendre、12～14归一、16～21m分支与Rodrigues/复定义均9差为0；核官方legendre默认CS定义 | 标量theta/phi及合法l,m域下九个函数正确；第5行degree/order注释对调不改变实际公式 |
| SH-05 | R0=1、R1=P Aᵀ Pᵀ；原l=2 M/U/P/V与直接张量25差为0，25个w均零且源码先短路；CS相位共轭后基作用3+5差为0 | 无CS基内原递推正确；与kugelflfunc2基直接混用一般不等价，数学纠正为S_l R_l S_l；degree=0仍额外创建R1亦已记录 |
| SH-06 | 汇总一般SO(3)恒等式、完整源对应、IR98差异、CS共轭及Rx90明确反例 | 三路线在明确转换后精确一致；原脚本原样组合不能无条件保证正确；理论裁定完成且原源码未修改 |

### 2.1 最重要的数学更正，不以结案掩盖

固定 `f_new(d)=f_old(A d)`、源CS系数列左乘时，正确源基旋转为

\[
D_{CS}^{(l)}(A)=S_l R_{SHRotate}^{(l)}(A)S_l,
\quad S_0=1,
\quad S_1=\operatorname{diag}(-1,1,-1),
\quad S_2=\operatorname{diag}(1,-1,1,-1,1).
\]

取f_old=2+z、A=Rx(pi/2)、d=e_y，定义要求3；原R未经CS共轭直接作用源系数得到1。这个差异没有被删掉或用容差放行。**本轮仅证明并记录所需换基，没有修改原MATLAB，也没有检查其它业务调用者是否已事先变换坐标。** 不据此否定论文全部实验，不把“已整改”写成“原组合已无错误”。

IR98 Table2负m的V印刷因子sqrt(1−delta)对同一x45二阶元素给sqrt(3)/4，正确源码加号、定义和Choi给sqrt(3)/2。保留原印刷内容与不同结果，不声称作者另有正式修正。

### 2.2 真实执行与固定身份

本轮在Wolfram Language 15.0.1重新执行起点固定的 `verify_low_order_basis.wl`、`verify_low_order_rotations.wl`、旧布局脚本，全部返回allExact=True。随后只在已有 `verify_low_order_layout.wl` 内补原MATLAB逐式验证并重新执行：**13/13**通过，Rodrigues/复定义九差各0、原二阶25差0、IR96显式表25差0、CS修正基作用一阶3和二阶5差0。独立W对偶检查另4/4成立。

| 来源 | 固定身份 |
|---|---|
| 原kugelflfunc2.m | blob `edc0caa1f88bf91d2a189e4d3a283334f809b905` |
| 原SHRotate.m | blob `7b73000411a9294f6cbed2e111bf74ce7fb7d4df` |
| 定义脚本（未改） | blob `498dda40eef44aeb4e7e9e914ce73d891f330d88` |
| 三路线旋转脚本（未改） | blob `e40ca439254b8860a03d0f265eb9bf1959c6acc5` |
| 扩展布局脚本 | 提交 `7f083e2e51d2f30126a9f6e72c27c196240fe977`，blob `644eaa967316d52a3769ef1508f2db80d6607c89`，SHA256 `eb6871fe023e0d6c694ec5b81d75fd78bb75da6230237807e3e3d7fca04d4468` |

历史 `verification_results_2026-11-09.json` 保持原样；它关联的原始运行时点、源检查点和旧C++未执行状态不能当成本轮新状态。当前精确输出与范围由现行报告和已提交脚本重放核对，没有虚称MATLAB/C++运行。

## 3. 仍只限本专项的第二次复核

六项目标在上述数学判定完成后，可按AUDIT规定的同一13个对象复读来源/公式/报告/台账，再重放已有符号证明核对是否一致；不增加文件、不增目标、不进入原97AF跨模块任务。最终远端一致性检查应确认：六项目标原文未变、状态有对应证据；13个文件集合不变；原两份MATLAB及全部C++/构建文件无修改；明确保留上述条件/反例与源码未改限制。

范围之外仍可能有工程待办或高阶/浮点问题，但不成为这六项理论审计的未办结条件，也不启动新的研究。若以后发现直接推翻本报告某个低阶公式的证据，只能重开对应SH项，不另造外围项目。

## 4. 本轮专项七项计数

| 指标 | 本轮开始 | 理论核对结束 | 净变化 |
|---|---:|---:|---:|
| 整改目标总数 | 6 | 6 | 0 |
| 已整改 | 0 | 6 | +6 |
| 未整改 | 6 | 0 | -6 |
| 审计文件总数 | 13 | 13 | 0 |
| 已审计文件 | 9 | 13 | +4 |
| 正在审计文件 | 2 | 0 | -2 |
| 未审计文件 | 2 | 0 | -2 |

此为限定理论核对的进度，不是原Matlab实现修改率。没有增加审计文件数量，不以全工程审批未完成拖延本专项结案；也不以审计结案宣称源码运行无条件正确。

## 5. 原全工程台账仅作历史

原97项AF的完整编号/目标/74已整改和23未整改状态，仍保存在[9af1668 AUDIT_PROCESS历史附表](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/9af16680a2b73754ad3b09825faf2655ea45a052/docs/AUDIT_PROCESS.md)以及[c38b4b6原工程版本](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/c38b4b63307c5f3febf2bee0aac95b565f44a46c/docs/AUDIT_PROCESS.md)。本轮只以不可变链接代替当前文件中无效的重复历史表，没有改变旧状态，也不允许据其继续扩大专项。
