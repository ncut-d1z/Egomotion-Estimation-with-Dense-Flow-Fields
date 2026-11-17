# R6 Adaptive 均值与矩阵条件参考接入

任务标签 `[HUMAN-EXECUTE/APPROVE]`。用户已授权生产代码、构建/CTest 及方案文档修改；本批仅固定输入，不执行完整科学实验、不安装依赖、不改原始数据或数值门限，不代签 DEC-005/008。

## 首轮检查点准备

新增 `tests/integration/test_r06_adaptive_conditioning.cpp`，注册 `r06_adaptive_conditioning_reference`；总计 40 项 CTest。**尚未构建或执行，40 项登记不能继承之前 39/39 的结果。** 提交、正常推送并核实远端 SHA 后，先运行五项 MAT 测试，再运行其余测试；日志保存在新的忽略目录。

## 独立依据与固定输入

- 只读原源码 `oneit_adaptiv_matched_filter.m`，blob `48e51bcd52fb8f27cf35b3fce38b28507af36e55`。
- 既有独立参考 [zero_mean_conditioning_evidence.v1.json](../../verification/adaptive/zero_mean_conditioning_evidence.v1.json)，blob `be9aa64cb454fb434b407814f8cfa1310a7c90b7`；其 Wolfram 生成器 blob `36eedaf6477d0255b7ab0f12572506970da6847c`。本批不重新运行 Wolfram，也不改该历史证据状态。
- 理论推导见 [均值与条件数专题](../design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md)。测试独立用 `long double` 标量公式构造预期矩阵、右端和奇异值，不调用生产矩阵装配/流生成函数生成预期值。实际生产计算仍为 double；只在诊断/参考计算使用较高精度。

十二个运动系统可解的固定输入（完整投影另有定义域约束，见实测记录）：

1. 六轴均匀正近度，`a=1,0.1,0.001,0.000001`，真实平移 `e_x`、旋转 `(1/5,-1/7,1/11)`；预期 `M=(2/3)I6`、raw 平移 `a e_x`、条件数 1。另取 `a=1e-100,omega=0`，验证绝对小均值本身不会触发任意阈值拒绝；旋转置零避免把不可分辨的小平移叠加到有限旋转上。
2. 六轴 `r=a+y`，同四个 a，CS 系数 `C00=a sqrt(4pi)`、`C1,-1=-sqrt(4pi/3)`；下左块为 `[e_y]_cross/(3a)`，两个对角块为 `(2/3)I3`。`a<1` 有负近度，明确是数学输入，不能声称全部是正深度场景。固定行列式不代表良态，条件数使用既有解析式 `((sqrt(gamma²+4)+gamma)/2)², gamma=1/(2a)`。
3. 四条正深度窄锥视线，`h=0.1,0.01,0.001`、raw near=1；独立解析矩阵、右端与三对奇异值对应既有 `condition=h^-4`。保留真实平移/旋转及源公式，记录解的前向误差、后向残差和 double SVD 条件估计。

另四个定义域负例：全部 near 为零、非零 near 正负抵消至均值零、纯旋转导致 raw 平移为零、四条重合视线导致奇异矩阵。分别要求既有 `InvalidNearness`、`InvalidNearness`、`ZeroNorm`、`SingularSystem`，stage 为 Solve；不推断 MATLAB 的异常/警告轨迹。

## 断言与诊断边界

沿用 `test_r06_adaptive_l3.cpp` 的回归 allowance `2e-8*(1+abs(expected))`，没有新增生产条件数阈值，也没有启用候选 IllConditionedSystem 分类。直接断言输入不变、raw mean 相对比例、raw/sign/normalized near、完整 6×6 矩阵和 RHS、奇异值的绝对/相对尺度、有限输出及归一平移、后向残差；轴向族另核 raw 解、最终运动、MSE 和含原 `1e-11` 分母的 near_new。

窄锥族的**前向运动误差不作为本批数值资格通过项**，仅如实保存；病态问题的普通小残差不能证明运动准确。最小奇异值很小时，现有混合 allowance 只约束其绝对尺度，不能据此声称相对条件数精度达标。归档数学条件数与实际 double SVD 估计分开输出；正式误差/失败阈值仍需独立审查。

## 实测结果

首轮 `3ef34a20aef074af997a5110e7530b44274c7647`，RunId `r6-conditioning-20261117-XX2hlboO`，无诊断构建，MAT 5/5、其余 34/35，合计 **39/40**。新测试36项中12项失败，均是上述运动系统可解输入在完整单步末尾被 `koeff_aus_near` 拒绝；四个早期定义域负例符合预期。输入 SHA 保持不变，日志/失败完整保留。

### 首轮诊断与修订 R6-COND-001

这是测试把“运动系统可解”误认成“完整单步可成功”，不是生产求解器故障。六轴及四锥视线都有 `x*y=0`，原 `koeff_aus_near.m` 对源 `(l=2,m=-2)` 通道逐项计算的 `sum(Y²)` 为零，原静态公式发生 `0/0`；已有 C++ 按合同返回 `SingularSystem`，保留叶子 `LowOrderHarmonicsErrorCode / ZeroProjectionNorm`。未运行 MATLAB，不能声称原运行时抛相同错误。全状态 Issue/PR 检索为空，本地覆盖事项记录为 `R6-COND-001`。

现将原运动重建、归一、矩阵装配/求解及 near_new 阶段提取为内部 `detail::solveAdaptiveMotion` 与不同类型 `AdaptiveMotionResult`（头文件在 `src/estimators/adaptive_motion.hpp`）。生产公共 `AdaptiveMfaStepSolver::solve` 始终继续原投影，只有完整成功才产生公共 `AdaptiveStepResult`；Session 仍只接收该公共完整结果。表达式、运算顺序、FullPivLU、错误分类/消息与投影拒绝保持不变，内部阶段成功不能提交系数或宣称完整成功。

修订测试直接检查生产内部阶段的独立矩阵/运动参考，并逐一重新调用公共入口，要求十二例全部因零投影范数失败、错误链保留；原四负例仍通过公共入口检查。这样同时覆盖数学阶段与完整流程的失败义务，不删除场景、跳过失败或修改生产门限。等待新检查点复测；R6 未结案。

### 第二轮与内部占位初始化

`7bbf8df89d1f4332f5d733e718498a1b1006b658`，RunId `r6-conditioning-phase-20261117-jZusEjpG`，无诊断构建、**40/40 CTest**（MAT5/5、其余35/35），新专项 **1077项、0失败**，所有输入摘要不变。十二个运动阶段与独立参考一致，同时十二个完整公共单步均按预期保留投影失败；四个早期负例正确分类。窄锥 h=0.001 的前向最大分量误差实际为 `4.1634284445323425e-6`，后向残差约 `2.36e-17`，证明不能由小残差宣称前向准确；这仍是诊断，未批准病态误差阈值。

静态复核另发现新内部阶段会在投影前移动 trace，其中 `coefficients_next` 的 Eigen 固定矩阵原来要到公共投影后才赋值。现显式置零作为**无效占位**，避免复制未初始化成员；完整公共入口仍用真实拟合覆盖。该风险不是本轮测试触发的数值失败，不把40/40视为内存安全证明。补充提交后再次执行检查点与全量测试，最终受测SHA以下一记录为准。
