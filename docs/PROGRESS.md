# 实施进度与证据登记（PROGRESS）

> 滚动证据登记 v0.5，非正式基线批准。本轮用户明确限定**只做理论推导，允许符号计算，禁止C++实现**。因此当前修改仅为理论专题、Wolfram脚本、独立数学结果和审计登记；旧SH实现授权不扩大到本轮。原生运行与数学运算的执行者、固定源码及证据类型必须区分。
>
> 为消除旧NOT_RUN、旧授权与动态计数反复堆叠，既有E-DOC-054～067详细记录完整保存在[本轮起点固定PROGRESS](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/982537556be1ac5a414af368a0082806a9179d32/docs/PROGRESS.md)。更早E-DATA-001、E-DOC-001～053保存在[98e95e2固定历史](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/98e95e2f867dc7142fa0d5ac0f1c79f5d6429ed1/docs/PROGRESS.md)。未改写任何Git历史或历史审批；下表保留定位和关键事实，最新证据在E-DOC-068。

## 1. 当前边界

仓库 `ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields`，分支 `dev-plan-P0-P1-mat-dictionary-probe`；不合并main、不发布。`origin/**` 原45份.m与582份.mat只读；不要求执行MATLAB/Octave或获得L1。既有66REQ/66TC/98P、27条HLD设计边不变；数学子案例不新增正式TC。

本轮无C++、头文件、CMake、CTest、Bash、MAT写入或构建运行。Wolfram中的代数与高精度SVD是实际执行的数学检查，不称作生产Adaptive实现、C++单测或批准Golden。动态批准事项仍归 [TODO](TODO.md) 与 [DOCUMENT_BASELINE_REGISTER](DOCUMENT_BASELINE_REGISTER.md)，当前没有批准DEC-005/006/008或冻结接口。

## 2. 固定里程碑与历史检索

| 记录 | 已取得的限定证据 | 不扩张为 |
|---|---|---|
| E-DATA-001及早期记录 | 582/582独立MAT类型探针READ_OK，原数据只读 | 正式Reader/Writer或582份独立SHA-256/逐值L2验收 |
| E-DOC-054 | 1996/1998/1999论文材料、勘误和转置/相位核查 | 软件运行或全域数值稳定性证明 |
| E-DOC-055 | 九实基、IR/Choi/定义三路线一般SO(3)精确数学；SH叶子源码 | 全部业务模块或任意IEEE输入正确 |
| E-DOC-056 | 用户在8eab5dd原生1/1 SH CTest通过 | 后续提交自动通过 |
| E-DOC-057/058 | 用户在445ed151增强解析/相位回归1/1通过，随后扩大测试 | 全66TC通过 |
| E-DOC-059 | 用户在601c74de完成53,532检查/624非零四元数的1/1 SH CTest，双类残差有日志 | 全SO(3)浮点路径或最终DEC-005 |
| E-DOC-060/061 | AN-G06/07/08独立归档参考、原二阶逐系数投影实现与数学参考 | 当时尚未运行的新CTest通过 |
| E-DOC-062 | 用户固定5e522198的SH源与三个独立CTest全部通过，53,532+176+76检查 | Adaptive/KvD/采样业务软件通过 |
| E-DOC-063/064 | 三类采样i=0/1/2全714行参考，随后独立Wolfram逐元素核2142坐标，最大差1.11e-16 | 生产采样器/RNG或正式Golden批准 |
| E-DOC-065 | AN-G12对称20方向独立数学单步；+1e-11使near_new仅近似1 | 全部Adaptive输入、Session或真实CTest |
| E-DOC-066 | Original/Modified KvD三组输入共36轮精确数学trace；最少6/最多200的正常控制流界 | 真实模型可达200轮或生产软件验证 |
| E-DOC-067 | 非对称九方向正/负均值独立数学；正支恢复、负支M1与加权投影差2I，条件数有结果 | 所有物理正距离输入失败或可擅改负支 |

上述记录的全部原始数值、源Git blob、SHA-256、日志路径与纠偏过程均在本页顶部的固定历史版本及各 `verification/` 工件。当前不再使用旧“SH源尚未编译/三个CTest未运行”描述：用户已于固定 `5e5221988cf093dbfddb88f2f65f06632460f3a1` 在 Ubuntu 24.04/GCC16.2/CMake4.3.4/Eigen3.4、Release/C++17下完成 **3/3 CTest Passed**。这来自用户终端文本，不是代理在其机器执行。

## 3. E-DOC-068：零均值、抵消、几何退化和最终平移归一的分层证明（2026-11-09）

### 3.1 固定起点和真实源

本轮起点 `982537556be1ac5a414af368a0082806a9179d32`。开始时97个唯一AF（74已整改、23未整改）；94个文件（62已审计、32正在审计、0空白）。重新读取 `AGENTS.md`、两份审计台账和原 `oneit_adaptiv_matched_filter.m`（Git blob `48e51bcd52fb8f27cf35b3fce38b28507af36e55`），只依据原先取均值、负支翻号、仍除旧均值、M1～M4、最后t归一的实际表达式开展推导。

### 3.2 本轮实际提交的三个工件

| 路径 | 内容与限定状态 |
|---|---|
| `verification/adaptive/verify_zero_mean_conditioning_families.wl` | 实际Wolfram精确恒等式及70位SVD验证脚本；固定完成版Git blob `36eedaf6477d0255b7ab0f12572506970da6847c` |
| `verification/adaptive/zero_mean_conditioning_evidence.v1.json` | 保留实际25/25检查、6组SVD样例、精确公式、原源blob与限制，非批准Golden |
| `docs/design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md` | 现有Adaptive LLD的理论专题补充；推导定义域、扰动界、恒定行列式反例、正深度窄锥谱、纯旋转平移归一失败及TC映射 |

新增推导属于本轮从源代码出发的研究结论，不声称论文已经给出这些定理。没有以其它算法替换源行为，也没有实施新的阈值、求解器或错误处理。

### 3.3 原始near与翻号后near的准确区分

令r是原始raw near、s=mean(r)、r_flip=sign(s)*r，源最终近度mu=r_flip/s。对于s不为0，**mu=r/abs(s)**，均值为sign(s)。这不是把“已经翻号的near”再除以abs(s)：后者在负支确实会改变原结果。旧文档有关abs(s)的警告必须连同变量阶段解读。

在不跨过零均值的扰动域内，精确相减得到：`delta_mu=(delta_r-sign(s)*mu*delta_s)/(abs(s)+sign(s)*delta_s)`。据此可分离相对尺度与抵消。若所有raw近度非负且s>0，则归一mu在[0,N]；s绝对值小本身不等于发生正负抵消。无量纲chi=abs(s)/mean(abs(r))可辅助记录抵消程度，但本轮没有为它规定生产阈值。

### 3.4 精确参数族及实际Wolfram结果

1. **零均值输入有完整SH表示**：六轴方向与原CS系数 `C00=a*sqrt(4pi), C1,-1=-b*sqrt(4pi/3)` 重建raw near=a+b*y。a=0,b=1给非零向量(0,0,1,-1,0,0)但均值0；a=b=0给全零输入。两者的原均值归一都无定义。没有执行MATLAB，因此不推断其具体警告/异常传播。
2. **全正同比缩小**：b=0、a>0时M=(2/3)I6、条件数严格1；按原光流合成运动解出raw平移a*e_x。若raw平移被横向绝对扰动eta*e_y影响，方向角为atan(abs(eta)/a)。故矩阵良态与最终方向稳定性不是同一条件。
3. **有符号抵消**：a>0、b固定时源M的两个对角块均2I/3、左下块为(b/(3a))*[e_y]叉乘矩阵。det(M)=64/729、六个特征值恒2/3，但kappa2=((sqrt(gamma^2+4)+abs(gamma))/2)^2，gamma=b/(2a)，随a趋0按b²/(4a²)发散。b=1、a=1e-6时约2.50000000002e11。a<b时含负近度，明确不是严格正深度场景。
4. **严格正深度的窄锥几何**：四个单位方向采用有理半角参数h，raw near全1、s=1；在0<h<=1/2域，精确谱三对特征值为2h⁴/(1+h²)²、4h²/(1+h²)²、2/(1+h²)²，所以kappa2=h^-4。h=1/10、1/100、1/1000分别为1e4、1e8、1e12；h=0极限矩阵秩2。此例不靠负近度即可发生严重几何退化。
5. **纯旋转的后置失败**：六轴与raw near全1、M条件数1，纯旋转光流使六维解前三项精确为0；原源码随后t_raw/norm(t_raw)无定义。旋转解与无平移方向的定义域必须分别记录，不能把6×6可逆当成整个单步成功。

完成版Wolfram Language 15.0.1实际执行 **25项检查，25项为True**；包含完整特征多项式恒等式，不仅是有限样本。另对抵消族3组、窄锥族3组输入做70位SVD与公式交叉检查；以相对1e-35作为本次诊断阈值，JSON中差值显示0.0代表该工作精度下结果，不宣称任意机器逐位相同。初始未化简的极限曾产生中间Power::infy提示，完成版先化简增长比再取极限，最后重跑没有这些提示；失败/中间版本不算正式通过。

### 3.5 证据归口和剩余卡点

专题映射现有TC-F-ADAPT-001/002/003、TC-DC-ADAPT-001；没有新增正式TC，也未修改 `TEST_PLAN.md` 中已注册的实体或编写单元测试。新增数学证明补充AF-044/051/054/055/060的若干定义域和病态反例子义务；这些AF仍包括更多一般/近奇异/异常、Session原子提交、真实生产C++/CTest、独立参考覆盖和DEC-005/008批准，因此保持未整改。当前理论不能提供经过真实软件误差校准的阈值，不能自动修正原负支。

本轮只有理论文件与治理记录变动；**无需重跑旧SH 3/3测试**，因为C++、CMake、CTest和build.sh都没有修改。所有历史原生测试仍绑定各自SHA。

## 4. 本轮七项指标

| 指标 | 开始 | 结束 | 净变化 |
|---|---:|---:|---:|
| 整改目标总数 | 97 | 97 | 0 |
| 已整改 | 74 | 74 | 0 |
| 未整改 | 23 | 23 | 0 |
| 审计文件总数 | 94 | 97 | +3 |
| 已审计文件 | 62 | 65 | +3 |
| 正在审计文件 | 32 | 32 | 0 |
| 未审计文件 | 0 | 0 | 0 |

三份新数学文件的已审计状态只在上述限定证明范围内有效；未降低原生产/批准义务。第一轮未清零，不进入用户条件性的第二轮全面审计。唯一权威计数由 [AUDIT_PROCESS](AUDIT_PROCESS.md) 的AF表与 [AUDIT](AUDIT.md) 的路径表复核。
