# 低阶球谐三路线一致性专项审计（AUDIT）

> 当前唯一有效范围仍为用户指定的三路线低阶球谐理论审计。本轮从 `9af16680a2b73754ad3b09825faf2655ea45a052` 继续，根据用户最新许可开展理论推导和Wolfram符号核查；禁止C++实现，原MATLAB只读。上轮“仅修改两份范围文件、不做具体核验”的规定是当轮范围修订限制，完整版本固定在[9af1668](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/9af16680a2b73754ad3b09825faf2655ea45a052/docs/AUDIT.md)，不是禁止此次已授权的专项理论核验。

## 1. 唯一审计目的与范围硬边界

唯一目的：核对论文 *Insect-Inspired Self-Motion Estimation with Dense Flow Fields—An Adaptive Matched Filter Approach* 实际依赖的 l=0,1,2 九个实球谐函数及其旋转计算，比较以下三条独立数学路线，并逐式核对两份原MATLAB。

| 路线 | 唯一核对对象 |
|---|---|
| A 直接定义 | 由球谐定义/实基转换得到九基，以及从定义得到的R0(1×1)、R1(3×3)、R2(5×5)旋转作用 |
| B 实球谐递推 | Ivanic–Ruedenberg 1996、1998勘误和M/U/P/V/W在l≤2的实际公式 |
| C 复球谐递推 | Choi等1999的复递推、W共轭变换，与A/B在同一约定下的一般旋转一致性 |

原MATLAB核查仅限 `kugelflfunc2.m` 的Legendre、CS相位、归一化和m分支，以及 `SHRotate.m` 的degree=0/1/2、M/U/P/V/W、下标、转置、系数/基值作用。可以明确指出真实差异、论文印刷错误及需要的数学换基，不能为了“全部一致”隐瞒反例。

### 1.1 不得再次扩大的范围

Franz/KvD/Adaptive运动求解、M1～M4、near正/负/零均值、Session、收敛/退化、采样噪声/RNG、MAT/CLI/Runner/绘图、性能、全工程SRS/HLD/TC/P/审批均不属于本专项。`koeff_aus_near.m`/`near_aus_koeff.m` 的离散拟合、保正、幂等和最小二乘问题也不属于本专项。任意高阶和全部IEEE浮点正确性不在此证明范围。

每项行动必须直接对应SH-01～SH-06的某个低阶基函数、旋转公式或上述两份MATLAB的具体位置；否则不得开展。未经用户明确同意，不增加文件/目标/验证工件，不以研究价值或相关性为由扩张。

### 1.2 执行与结案规则

- 复用原论文页图来源审阅和现有低阶符号脚本，只补尚未闭合的三路线/源公式核对。本轮没有新建文件，只在原13个对象内修改已有报告、布局证明脚本和两份台账。
- 所有 `origin/**`、C++/头文件、CMake/CTest/build.sh、MAT均未修改，未执行MATLAB/原生构建。Wolfram数学执行不得写成C++或MATLAB运行。
- 六项目标的“已整改”指**本限定理论审计已明确判定、数学对应或更正公式已有证据**。若发现原源码约定差异，必须明示未修改源码，不能把结案理解成所有原代码已变得无误。
- 精确证明必须明确单位球、l≤2、SO(3)、相位/排列及函数作用；单个特例或旧SH 3/3 CTest不能替代一般证明。
- 数学结论和原论文印刷内容分开：IR98表中残留的减号因子不被暗改；源码正确的加号与定义及Choi交叉证据共同裁定。
- 第一轮六项目标结案后仅复核下面同一13对象及六项结论，禁止自动进入原全工程97AF的第二轮审计。第二次核查仍不增新目标；有直接差异则在原SH目标重开，不用外围问题拖延结案。

## 2. 专项唯一审计对象与状态

以下“已审计”均限定为本专项相关段落/公式及其证据身份；不是对同文件中历史工程接口、实现或性能文字作整体验收。

| 仓库根相对路径 | 状态 | 本轮限定结论 |
|---|---|---|
| `docs/AUDIT.md` | 已审计 | 同一13对象、六目标和禁止扩张边界；理论结案不冒充代码修正 |
| `docs/AUDIT_PROCESS.md` | 已审计 | 六个目标逐项给出证据与明确裁定；不再统计旧工程AF |
| `docs/design/harmonics.md` | 已审计 | 仅§1九基/CS映射、§3三路线作用及§4勘误；本轮一般符号结果支持，外围拟合/工程内容未纳入 |
| `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md` | 已审计 | 仅§1/3/4的文献公式、M/U/P/V/W、相位与方向；旧全工程待办不阻塞本专项 |
| `docs/golden/SH_PAPER_VERIFICATION_CASES.md` | 已审计 | 原25元素参考表及W变换与定义重新核对；旧候选/未运行身份作为历史，不宣称新数值软件测试 |
| `docs/reviews/SH_PAPERS_SOURCE_AUDIT_2026-11-09.md` | 已审计 | 复用原PDF页图/式号及源码对应；本轮重新证明勘误差异和相位反例 |
| `docs/reviews/LOW_ORDER_SH_THREE_ROUTE_VERIFICATION_2026-11-09.md` | 已审计 | 当前六项专项的统一理论结案报告，明确原脚本直接混用不满足同一CS场作用 |
| `verification/harmonics/verify_low_order_basis.wl` | 已审计 | 固定原脚本重新执行；定义与论文九基、基值递推差全0 |
| `verification/harmonics/verify_low_order_rotations.wl` | 已审计 | 固定原脚本重新执行；IR/Choi的R1各9、R2各25差全0，一般SO(3) |
| `verification/harmonics/verify_low_order_layout.wl` | 已审计 | 复用并扩展原脚本：Rodrigues、原MATLAB低阶M/U/P/V、Table3、CS共轭及反例，13/13通过 |
| `verification/harmonics/verification_results_2026-11-09.json` | 已审计 | 原历史精确输出/论文身份未改；旧NOT_RUN不当作新执行状态，本轮结果在现行报告 |
| `origin/egomotion_estimation_dense_flow_fields_matlab_scripts/help_functions/kugelflfunc2.m` | 已审计 | 9行CS Legendre、12～14归一、16～21的m分支与定义九项严格等价，源未修改 |
| `origin/egomotion_estimation_dense_flow_fields_matlab_scripts/help_functions/SHRotate.m` | 已审计 | R0及无CS基内R1/R2递推正确；与源CS系数需S_l R_l S_l；degree=0仍创建额外R1，源未修改 |

**现行13个对象＝13已审计＋0正在审计＋0空白。** 文件总数没有增加。旧文件中的其它模块/正式API/完整工程状态不参与本专项完成率。

## 3. 第一轮六项结论与限定复核

详见[当前三路报告](reviews/LOW_ORDER_SH_THREE_ROUTE_VERIFICATION_2026-11-09.md)。本轮数学裁定：

1. 九个源CS基、论文正相位九基之间的显式映射正确；原kugelflfunc2在标量合法输入域内计算正确。
2. 采用源码V正号与正确P边界后，定义、IR实递推、Choi复递推在统一基及作用下，一般SO(3)的R0/R1/R2精确一致。IR98印刷减号版本存在明确反例，不被判作正确。
3. 原SHRotate无CS基与kugelflfunc2的CS基不同。固定 f_new(d)=f_old(A d) 时，正确源CS系数矩阵为 S_l R_SHRotate(A) S_l，其中S1=diag(-1,1,-1)、S2=diag(1,-1,1,-1,1)。未经转换的直接混用一般错误，Rx90例输出1而定义要求3。
4. 以上是数学对应和更正建议，不是原MATLAB源码已修正；没有审计调用者是否预先变换坐标，不能推广为论文所有实验无效，也不能宣称全IEEE输入100%通过。

第二次限定复核只验证这些结论、固定源/脚本身份、六目标和13对象一致性；只读重放已有符号证明，不启动任何外围研究。本轮最终远端复读与数学重放作为限定复核记录，不以增加审计文件计功。

## 4. 专项七项指标

| 指标 | 本轮开始 | 理论核对结束 | 净变化 |
|---|---:|---:|---:|
| 整改目标总数 | 6 | 6 | 0 |
| 已整改 | 0 | 6 | +6 |
| 未整改 | 6 | 0 | -6 |
| 审计文件总数 | 13 | 13 | 0 |
| 已审计文件 | 9 | 13 | +4 |
| 正在审计文件 | 2 | 0 | -2 |
| 未审计文件 | 2 | 0 | -2 |

完成数字表示限定理论核对完成且差异已有明确裁定，**不表示原两个MATLAB原样组合已经无条件正确**。不将需要单独代码授权的后续实现默默重新纳入理论审计。

## 5. 历史全工程表只读存档

原100个文件（68已审计/32正在审计/0空白）、原97个AF（74已整改/23未整改）不参与现行统计，也不作为下一项任务来源。完整原表和其状态仍保存在[9af1668 AUDIT附录](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/9af16680a2b73754ad3b09825faf2655ea45a052/docs/AUDIT.md)及[c38b4b6原工程表](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/c38b4b63307c5f3febf2bee0aac95b565f44a46c/docs/AUDIT.md)。本版仅将不再有效的重复历史表替换为不可变链接；未改写任何旧记录、源码或审批。
