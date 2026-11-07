# 七份正在审计文档的原源码静态复核与新增问题（2026-11-08）

> **只读文档/源码数学合同审计**。固定审计输入：`e190ed74240bf7b3b32a65c8fb6099fbdedc46b9`，分支 `dev-plan-P0-P1-mat-dictionary-probe`。本报告只核对 GitHub 中现存原 MATLAB `.m` 文本、设计 LLD 与现行用户决定；**未执行 MATLAB/Octave、C++、Bash、CMake/CTest、MAT 读写、模拟或独立 L3 数值运行**。用户指定的 Ivanic–Ruedenberg 1996 原文及 1998 勘误 PDF **尚未上传**，不能宣称完整核对 R5 原论文公式或数值忠实性。

## 一、七份仍在审计文件的逐项来源检查

| 文件（仓库根相对路径） | 本次对照的原始 MATLAB 源与文档合同 | 本轮结论/证据限制 |
|---|---|---|
| `docs/design/kvd.md` | `zellendefinieren.m` 的 3×10/三轴与 `franzeigen.m` 六类模板响应；`norm_koenderink_iteration.m` 与 `mue_norm_koenderink_iteration.m` 的近度分母 1e−13、near 符号翻转、Original 的 a/b/c 含 near/near² 与 Modified 的取消因子、旋转更新使用**旧 t**、迭代下界 6/上界 200、动态 grenze、结束后的 near 均值尺度反馈 | **可用源码的逐段数学/时序文档对照完成，未发现新的当前文本公式冲突**；独立 L3 逐轮值、有限精度/退化与真实测试未进行，不因而声称算法验证 |
| `docs/design/adaptive_mfa.md` | `oneit_adaptiv_matched_filter.m`：near_raw→原均值 s→s<0 时先取负→除以原 s、右端 `-mean(p)` 与 `mean(p×d)`、M1–M4、`real(inv(M))*rhs`、平移归一、`error` 返回 **MSE** 而 `oneit_error=sqrt(error)` 仅内部变量、重新近度分母 1e−11、逐项拟合系数。对照三个 data_generation 入口的初始化/比较/周期刷新 | **可用源的合同/时序文档审计完成，未发现新的当前公式冲突**；实际 C++ Solver/Session、失败提交状态和独立 L3 多帧未执行 |
| `docs/design/harmonics.md` | `winkelausvektor.m` 的 theta/phi、`kugelflfunc2.m` 的 MATLAB legendre 默认相位与 m 正负 sin/cos、`near_aus_koeff.m` 九项逐点重建、`koeff_aus_near.m` 的各项独立投影与各自分母、`SHRotate.m` 的 R3 逐元素索引和 R5 递推入口、`transfor_koeff.m` 的 3×5 旋转后列块 | **现存原 `.m` 的静态接口/公式索引审计完成，未发现新的当前文本冲突**；但 Ivanic–Ruedenberg 原文/勘误尚未逐式审查，R5 的全部 25 项独立 L3 及 Eigen 实现/编译仍未验证 |
| `docs/design/sampling_noise.md` | `erzeugerichtungsvektoren.m` 的八面体 8 面、`erzeugerichtungsvektoren_loch.m` 仅用第 2/3/5/6/7/8 面、`findeDreiecke.m` 的中点归一与 A/B/C/D 深度优先次序、`richtungsvektoren_icosahedron.m` 固定 20 面；`getvektor.m` 两次 rand、Bias 两次 seeding、`fixed_*_randomnear.m` 的 N 次 rand/3N 次 randn、末次 near 标量与候选 C++ N 维诊断数据区分；`erzeugeflussausnear.m` 两类平均模长、`nonpropfehler.m` 系数为 0 仍消费 3N randn | **可用源的采样、RNG 顺序与返回数据合同静态审计完成，未发现新的当前文本冲突**；i=1/2 的全部有序方向独立数值 L3 和 RNG 运行计数/真实测试未执行 |
| `docs/design/trajectory.md` | `kreis_trajektorie.m` 和 `sinuskurve_trajektorie.m` 分母均为 `stellen`；`trajektorie_aus_punkten.m` 只写 i=2..N−1，世界→局部使用 Rᵀ；`drehvektor_aus_zwei_vektoren.m` 在叉积范数超过 1e−11 时才归一，反向共线可能给零旋转向量；`rodrigues.m` 在 norm(omega)<eps 返回单位阵；`getnear_in_kugel_Drehung.m` 使用旋转视线和球内正根倒数 | **可用源的公式、端点和退化分支文档审计完成**，未发现新文本冲突；Sophus 与源码的小角、π/反向及球边界 L3/CTest 仍需真实验证 |
| `docs/design/analysis_plotting.md` | `graph_agent_in_sphere_adaptive_matched_filter_test.m` 与 `...many_parameter.m` 的 x=50:550、固定四组 begin/end、阈值 0.01/0.02、=阈值不切换、缺少历史 ratio 则非成功；`kurveglaetten.m` 半窗 2/5/10 与边缘零值；Bias 原 xwerte 和完整球面实际方向 N 的区别 | **有一项实质性的参考案例编号冲突 AF-026**（见下）；核心分段/平滑数学合同的静态审核已完成，真实 L2 数据整列独立曲线 L3 仍未取得 |
| `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | AN-G01～12 定义独立固定输入参考候选，比较规则 `atol=0.01, rtol=0.1` 仅首轮筛查；核对对应 `getvektor.m`、八面体索引、`erzeugeflussausnear.m`、`kurveglaetten.m`、`oneit_adaptiv_matched_filter.m`、SH 九项与 R3 来源约定等文档事实 | **有一项参考来源等级混淆 AF-027**（见下）；AN-G01～12 的具体数字仍只是设计候选，尚未完成**独立的数值/运行证明**，最终 DEC-005 仍开放 |

## 二、新发现——必须先登记「未整改」

| 问题 ID | 文件 | 原句/位置 | 具体整改目标与依据 | 扫描时状态 |
|---|---|---|---|---|
| **AF-026** | `docs/design/analysis_plotting.md` §7（原 140–145 行） | 分析案例 `AN-G01～AN-G06` | 这些编号**与** `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` 及 `docs/golden/analytic_golden_candidates.v1.json` 已使用的全局 **AN-G01～AN-G12** 冲突，但内容不同（例如分析的 AN-G01 是 Bias 曲线，而全局 AN-G01 是八面体 8 行）。应为分析模块采用**独立前缀**（建议 `APL-G01～APL-G06`），同时说明 AN-G01～12 专属于全局数学候选，避免测试身份碰撞；不重命名原 JSON 的 12 个 ID。 | **未整改** |
| **AF-027** | `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` §1（原 13 行） | “同量纲、有限且有唯一对应关系的**原 MATLAB 数值 x_ref** 和 C++ 结果” | 用户已永久取消运行 MATLAB/Octave 与 L1 原版中间值为强制证据，`x_ref` 应描述为**可追溯的 L2 原 MAT 参考或独立 L3 数值参考**（适用时 L4 统计另行处理），不能让首轮筛查公式错误地暗示“原 MATLAB 实际运行数值必须先获得”；公式本身及 0.01/0.1 不改变。 | **未整改** |

两项均可用既定文档规范解决，**无需用户新技术决策**。不能因为本轮已阅读七份文档，就提前标为软件/数学数值验证通过。

## 三、审计完成与证据缺口分离

- **可完成**：以上七份文件在已获得的原始 `.m` 和现行设计决定范围内的**文档静态一致性/公式/索引审计**。AF-026/027 处理并复核后，`AUDIT.md` 可按其既定定义将本轮文档审计进度标为“已审计”。
- **不能声称已完成**：不存在的 C++/MAT Writer/CTest、原 MATLAB 实际 L1、未上传的 Ivanic–Ruedenberg 两篇 PDF 的**全文逐式审查**、R5 全部独立数值 L3、KvD/采样/Adaptive/轨迹/分析真实输入数值执行、最终 DEC-005/006 阈值与正式基线签署。它们应在 [TODO.md](../TODO.md) 或后续测试/证据追踪中注明**未验证**，而不是反向将已经完成的文档审阅误记为软件 PASS。
- **仅文档任务的范围**：原 `origin/**` 永远只读；本轮不会运行 MATLAB/Octave、CMake/CTest/Bash，也不会创建生产 C++ 或修改源 MAT。历史 `AF-001～025` 的原报告和各自固定 SHA 不修改。

