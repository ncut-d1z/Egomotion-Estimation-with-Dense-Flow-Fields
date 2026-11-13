# 测试设计与验收计划（TEST PLAN）

> **测试计划草案 v0.31，未冻结。** 66 项 TC→候选测试实体/CTest→AC 的唯一规范性登记；当前 SRS/HLD/LLD 精确身份见 [文档控制矩阵](DOCUMENT_BASELINE_REGISTER.md)，真实执行进度见 [PROGRESS.md](PROGRESS.md)，不能以单个已执行 CTest 代替其他 TC。

> **候选核对材料**：见 [HAR-2026-11-06-001 材料包](approval/HAR-2026-11-06-001_REVIEW_PACKAGE.md)；历史提出的 MAT 类型/安全、Golden/容差仍要逐项执行。当前参考等级为 L2/L3/L4，不要求 MATLAB/Octave/L1；`atol=0.01`/`rtol=0.1` 仅首轮探索筛查。**低阶球谐已在 `8eab5dd`、`445ed151`、含624非零四元数/错误码检查的 `601c74de` 三个固定提交获得用户报告的 1/1 CTest Passed**，其他模块与完整验收未因此批准。

> **长期约束**：[无需运行 MATLAB 政策](golden/NO_MATLAB_EXECUTION_POLICY.md) 取消 MATLAB/Octave/L1 强制前提；该决定不等于所有模块的 C++/CTest/MAT 运行授权，也不批准 DEC-005 最终数值门槛。

> **当前实现现状（以固定 SHA 为准）**：现有低阶球谐三个独立 CTest：`low_order_harmonics`、`low_order_harmonics_archived_golden`、`low_order_harmonics_source_projection`。用户在固定 HEAD `5e5221988cf093dbfddb88f2f65f06632460f3a1` 的 Ubuntu 24.04/GCC16.2.0/Release 环境下提供了实际 **3/3 Passed**（分别 53,532/176/76 次内部检查）的完整可见日志；其它大多数业务 TC 仍是候选，正式 DEC-005/008、MAT 全量 L2/完整 L3/L4 未批准。

**权限与验收分层**：球谐 C++/专项测试已取得用户的局部实现授权，用户本人运行了本次球谐编译/CTest；其他模块代码、代理自行运行项目命令、MAT 写回读、DEC-005/006、基线及发布仍按 [TODO](TODO.md) 的具体限制办理。

### 2026-11-10 全工程文档审计的静态 TC 清单核查（DAR-004，未批准）

- **确实检查的静态范围**：重新读取本文件§3的66条 `TC-*` → `REQ-*` / `AC-*` 映射表，与 `REQUIREMENTS.md` 中66条唯一 `REQ-*` 登记逐编号比对；均为**66条唯一**，没有重复或编号缺失，且§3每行均有拟议测试实体/CTest列和AC列。这个结果仅是追踪**编号和映射位置**核对，不代表66条条款的完整语义/输入/误差均已深审。
- **仅有的业务原生验收范围**：根`CMakeLists.txt`已注册三个`low_order_harmonics*` CTest，用户在固定提交`5e5221988cf093dbfddb88f2f65f06632460f3a1`的Ubuntu GCC16.2/Release下提供 **3/3 Passed**；各程序内部检查数53,532／176／76。它们是**三个独立CTest可执行程序**，不能将内部53,784次检查、三个CTest或数学Wolfram证明误写成66个TC正式通过。
- **未验收范围**：本文件§3绝大多数业务目标仍是尚未实现/尚未运行的候选实体；582 MAT只读字典探针不是正式业务Reader/Writer的TC；AN-G/SH-P等候选Golden不自动属于已批准数值参考。DEC-005/006/008、L2逐值与必要L3/L4、软件负例、人员签署均未完成。
- **历史时点不可倒置**：下文分段记录的`NOT_RUN`和“新增测试未原生执行”属于彼时提交的事实；其后已有固定5e522198的真实3/3测试，所以当前判断以对应新证据为准。自这次固定SHA以来的纯文档修订不视作重新测试，也不推断以后任何新增C++版本已自动通过。

### 2026-11-09 首次低阶球谐原生构建及 CTest 证据（AF-096）

用户提供了 Ubuntu 终端从 `./build.sh` 开始至末尾 `PASS` 的完整文本输出，**不是代理本人在用户机器执行的命令，也没有取得原始日志文件字节**。受测 Git HEAD：`8eab5dd7971f9ba9263528d62e9dccdeb926d638`；Ubuntu 24.04、GCC 16.2.0、CMake 4.3.4、Eigen 3.4.0、Release/C++17、4 jobs。CMake 配置/生成成功，`egomotion_harmonics` 静态库及 `test_low_order_harmonics` 可执行文件构建成功；用户文本未显示 warning；CTest 单项 `low_order_harmonics` 为 **1/1 Passed、0 failed**，测试本身 0.07 秒，CTest 墙钟总耗时 0.11 秒。脚本打印 `PASS`，但未另行提供独立的 `$?` 数字或日志文件 SHA-256，不能伪称已经独立核验这些信息。

用户机器日志位置：`build/low-order-harmonics/logs/build-20261109T071118Z-451487.log`。这说明**一个 CTest 程序**运行并通过，其内部实际包含多项断言，但不等于 66 TC 均通过。Wolfram 全域精确符号证明另有来源，不混同本次 C++ double 运行。**现有测试通过的固定提交与后续新增测试源码必须区别对待**：新增加的独立基函数锚点、2015 论文第 30 页 `h4` 符号/dyadic 断言、MATLAB CS 相位重建断言，还没有实际运行证据，须在新 HEAD 上重跑。其它模块仍未实现/未获相同授权；DEC-005/006/008 和基线未批准。

### 2026-11-09 第二次低阶球谐增强回归实测（E-DOC-057）

用户在本轮终端输出 `git pl && ./build.sh`，fast-forward 至 **`445ed151e5fc1faaad34b920f86363e29fb0a484`**。环境为 Ubuntu 24.04 VirtualBox、GCC 16.2.0、CMake 4.3.4、Eigen 3.4.0、Release/C++17、4 jobs。静态库复用上次构建，球谐测试源新增的 87 行被实际重编译并链接成功；`low_order_harmonics` **1/1 Passed，0 Failed**，单项 0.06 秒，CTest 墙钟 0.07 秒；未见编译器警告行。脚本最终打印 PASS。完整原始日志文件仍在用户 VM 路径 `build/low-order-harmonics/logs/build-20261109T082009Z-457660.log`，**代理只持有用户提供的完整终端文本，并未读到该日志文件字节或独立获取 shell exit/SHA-256**。

本次通过的源码版本**确实包含** `testAnalyticBasisAndDyadic` 与 `testMatlabSourcePhase` 两项新增函数的主程序调用，覆盖独立解析九基、2015 p.30 的 `h4` 符号、全部六个二次张量独立表达式，以及 3×5 MATLAB Condon–Shortley 相位及填充隔离。单项 CTest 并不能证明所有输入/全部业务 TC/正式容差；测试源码在这个提交之后若再次改动应重新运行并留新证据。实际记录归 [PROGRESS E-DOC-057](PROGRESS.md)。

### 2026-11-09 球谐624四元数与错误码增量（初始设计状态已保留；**现已在601c74de实测**）

在用户刚刚实测通过的 `445ed151` 之后，球谐专项测试新增了以下两组确定性数值/错误分支覆盖，不增加 66 个既有 TC 编号，也不改变任何生产求解公式：

| 增量测试（唯一源 `tests/unit/test_low_order_harmonics.cpp`） | 可复核的静态目标 | 实际状态 |
|---|---|---|
| `testDeterministicQuaternionGrid` | 对每个 `(w,x,y,z)` 分量各取 `{-3,-1,0,1,3}`、排除零四元数，**5⁴−1=624** 个有限非零四元数（并非624个不同旋转），逐组校验三路线的一阶/二阶块、二阶正交性、固定九系数场作用 | **已在 `601c74de` 原生编译/CTest通过**；用户详细输出确认 `quaternion_grid=624` |
| `testFailureTaxonomyAndFiniteOverflow` | 检查非单位/非有限方向、反射矩阵、非法旋转算法值、MATLAB无效填充的Inf输入是否返回正确 ErrorCode；有限 `DBL_MAX` 正系数产生非有限重建结果须报 `NonFiniteResult` | **所在 `601c74de` CTest实际通过**；未取得逐断言错误码单独统计，不冒称覆盖所有异常路径 |
| 现有所有 `requireNear` 与标量比较 | 单独汇总 `max_matrix_vector_abs_residual`、`max_scalar_abs_residual` 和回归比较阈值；不冒充最终DEC-005 | **已有用户详细实测：8.8817841970012523e-16、1.0658141036401503e-14，阈值9.0949470177292824e-13** |

**初次试验事实**：`601c74de` 的 `build.sh` 尚未采用 CTest verbose，因此用户确实另外执行了 `ctest --test-dir build/low-order-harmonics -V` 才看到全部诊断。之后根 `build.sh` 已改为**一次 CTest 内启用 `--verbose`**，通过时也留存测试程序输出，不再要求重复运行；**此新脚本尚未得到下一次用户原生运行证据**。原来三次成功始终绑定各自固定 SHA。

### 2026-11-09 第三次低阶球谐压力回归实测（E-DOC-059）

受测固定 HEAD 为 `601c74de315fdeb7b913cecd2eceeadee56fb6b2`。用户在 Ubuntu 24.04/VirtualBox、GCC16.2.0、CMake4.3.4、Eigen3.4.0、Release/C++17 下运行 `./build.sh`，configure/build 成功、单个 `low_order_harmonics` CTest **1/1 Passed、0 failed**（CTest 总0.08秒）；随后执行 `ctest --test-dir build/low-order-harmonics -V`，仍为 **1/1 Passed、0 failed**（总0.11秒），测试程序真实输出：

```text
checks=53532
quaternion_grid=624
max_matrix_vector_abs_residual=8.8817841970012523e-16
max_scalar_abs_residual=1.0658141036401503e-14
regression_tolerance=9.0949470177292824e-13
```

两个最大绝对残差都小于本专项回归比较阈值。该阈值是代码中 `4096*epsilon(double)`，**不是正式批准的 DEC-005**；624个非零四元数中允许 `q` 与 `-q` 等价，同一测试程序的53,532条检查不是53,532个独立样本。错误码/有限大数溢出路径为该提交的测试内容，实际单项CTest通过；尚无逐断言独立失败/运行时浮点异常全面取证。成功输出来自**用户的命令文本**，未获取用户VM日志文件的字节/哈希，且脚本后续加入verbose输出的**新提交仍未实测**。链接 [PROGRESS E-DOC-059](PROGRESS.md)；原 AF-045/046/051/055 的广义数学/公共接口/完整TC义务仍开放。

### 2026-11-09 AN-G06/07/08 归档候选专项独立测试（已提交，**未执行原生 CTest**）

- **新增而非覆盖**：`tests/unit/test_low_order_archived_golden.cpp` 独立验证归档 [12个解析候选 JSON](golden/analytic_golden_candidates.v1.json) 中的 `AN-G06`、`AN-G07`、`AN-G08`；原 `test_low_order_harmonics.cpp` 与 `low_order_harmonics` CTest 保持原样。根 `CMakeLists.txt` 额外注册 `low_order_harmonics_archived_golden`，使球谐现有 CTest 条目为 **2 个，均属同一个 SH 叶子模块**；新目标没有用户 VM 原生编译或 CTest 结果，不能将旧 1/1 变成新 2/2 Passed。
- **候选输入原样保留**：以归档 JSON 固定 Git blob `1e6499eff3ce5b55503f8cd913b9f492629da499` 为唯一字段/字面值来源。AN-G06 在 `d=e_x` 的九个 MATLAB CS 基值逐个核对；AN-G07 使用 `AN-G03` 原始 20 个有序单位方向与系数 `C(1,1)=3.5449077018110318`，只测试近度恒为 1，**不声称本模块实现了源采样器或球谐拟合器**；AN-G08 按源 `m=-l,...,+l` 的3×5布局，对 `R_z(π/2)` 的输出系数以及完整 R3 的9项、R5的25项分别由三路线核对。原 JSON 状态仍为 `DRAFT_UNAPPROVED_UNEXECUTED`，不能擅自变更为已批准 Golden。
- **独立性与验证边界**：C++ 期望值由归档字面数值固定提供，不从被测递推矩阵求 Expected；来源静态核查覆盖方向20行、九基9项、系数15值、R3九元素、R5二十五元素，**全部与 JSON 精确字面一致**；方向的最大双精度平方范数偏差约为 `3.33e-16`。新增可重放 [Wolfram 脚本](../verification/harmonics/verify_archived_sh_candidates.wl) 在独立 CS 球谐多项式上验证：AN-G06 9项差为0，AN-G07 恒值为1，AN-G08 3项R3/5项R5基函数作用差为0、整体场多项式差为0，结果 `allExact=True`。此符号检查已运行，**新 C++ 测试仍是 NOT_RUN**。
- **下一阶段的真实证据**：用户在最新提交运行 `./build.sh` 后应看到 `low_order_harmonics`、`low_order_harmonics_archived_golden` 两个独立 CTest 的实际结果；新脚本已在单次 CTest 调用中设置 `--verbose`，会保留两者详细输出。必须再获取真实 exit/日志/误差并逐案处理失败，才能标记新测试 PASS。这里不增加 66 TC、不开其它生产 C++，也不冻结 DEC-005/008。

### 2026-11-09 原 koeff_aus_near 的二阶逐系数投影专项（AF-045/051/055，已提交、NOT_RUN）

- **仅限获授权的球谐叶子**：新增 `projectSourceCoefficientsOrderTwo`（`src/harmonics/low_order_source_projection.cpp`，声明在 `low_order.hpp`），映射原 `koeff_aus_near.m` 固定 `ordnung=2`。按原 `(l,m)` CS 顺序对每项独立计算 `a=(sum(near*Y)/N)*4pi`、`b=(sum(Y²)/N)*4pi` 再 `a/b`；**不求九项联立最小二乘**，不新增跨模块 HarmonicFitter、Adaptive/Session、采样或MAT接口。返回源形状 3×5 和无效位置零填充；输入限有限非空 N×K (K≥3)、N近度、单位方向，异常用本叶子明确错误状态。
- **独立参考**：`verification/harmonics/verify_source_projection_order2.wl` 已用 Wolfram 15.0.1 实际精确运行，按 7 个非对称单位方向与近度 `(2,-1,4,1/2,-3,5/4,11/2)` 导出九项确切有理×根式系数；九个差全精确 0、所有九个分母正，Gram 的非对角项约 -0.2206948544207615 非零，具有与联合LS不等价的判别力。完整值与输入另存 `verification/harmonics/source_projection_reference.v1.json`，状态 **INDEPENDENT_WOLFRAM_EXACT_CPP_NOT_RUN_UNAPPROVED**；来源 `koeff_aus_near.m` 在 `origin/**` 保持只读。
- **新测试范围及限制**：`tests/unit/test_low_order_source_projection.cpp` 与 CMake 新的第三个 `low_order_harmonics_source_projection` CTest 逐项核九个原CS系数、无效列置零、只读前三列、不合格 shape、非单位方向、NaN/Inf、分母零和极大有限近度溢出拒绝。**该受限源和单独CTest已在固定提交`5e5221988cf093dbfddb88f2f65f06632460f3a1`由用户实际Ubuntu GCC16编译、3/3低阶球谐专项CTest通过，其中source_projection为76次专项检查**；本段旧“尚未原生编译”的时点性描述已按真实证据修复。后续当前分支的C++或未来公共API、NumericalErrorCode/Result错误适配、DEC-005仍未执行/未验收，不得把历史通过推广到新源码。
- **仍缺的完整义务**：动态阶数、通用 HarmonicFitter/公共 ABI、真实源输入/独立 MAT L2、全面L3/L4、最终 DEC-005/008 及其它模块测试；AF-045/051/055仍开放。新源码和额外 CTest 是低阶SH小范围实现，不扩展 H-10/H-11 到 MAT/CLI/KvD/Adaptive/Runner。

### 2026-11-09 低阶SH三项CTest首次全通过（E-DOC-062，当前状态）

用户提供固定 Git HEAD `5e5221988cf093dbfddb88f2f65f06632460f3a1` 的 `./build.sh` 完整终端输出，系统 Ubuntu 24.04、GCC 16.2.0、CMake 4.3.4、Eigen 3.4.0、Release/C++17；源投影SH生产CPP与三个测试程序全部成功编译链接，CTest采用脚本**一次 verbose**调用，结果 **3/3 Passed，0 failed，墙钟总0.10秒**。数值诊断如下：

| 真实CTest | 检查次数 | 最大绝对残差 |
|---|---:|---:|
| `low_order_harmonics` | 53,532（含624个非零四元数输入） | 矩阵/向量 `8.8817841970012523e-16`、标量 `1.0658141036401503e-14` |
| `low_order_harmonics_archived_golden` | 176 | `8.8817841970012523e-16` |
| `low_order_harmonics_source_projection` | 76 | `1.7763568394002505e-15` |

全部结果均低于专项回归阈值 `9.0949470177292824e-13`；这是回归余量，不是正式批准的 DEC-005/008。三项共 53,784 次内部断言检查，**不是53,784个独立CTest**。该结果来自用户 VM 的可见终端日志，原始文件路径为 `build/low-order-harmonics/logs/build-20261109T104655Z-470926.log`；没有得到日志文件本身的SHA-256、独立 shell `$?` 数值，不能伪称代理在用户主机执行。此提交之后新增任何SH源码/测试，需重新在该新SHA下测试，不能承接当前PASS。

**历史时点说明**：下文初次登记 AN-G06/07/08 与二阶逐系数投影时的 NOT_RUN 为真实历史快照，当前执行状态由本节及 [PROGRESS E-DOC-062](PROGRESS.md) 覆盖；不得将历史 NOT_RUN 当成现在，也不得将局部SH的3/3等同全部66 TC/AC通过。

## 1. 验证分层和前提

- 顺序：源文件审计及 MAT 格式核验 → 单元测试 → 合同测试 → 入口集成测试 → L2 最终 MAT/L3 独立逐节点 Golden 比对 → L4 论文统计复现。
- 确定性数值验收：同一固定输入比较逐步计算和误差；随机统计：独立统计有效/失败样本、抽样配置及噪声；不要求 MATLAB 与 C++ 同 seed 的 randn 逐位一致。
- 所有测试须记录 RunId、源 Git SHA、数据文件哈希、实际 Debian/Ubuntu/编译器/依赖版本、命令、参数、随机种子、预期与实测、误差、退出码及日志。
- 每条 M 级需求应有验收条件与至少一个真实测试或经批准的人工核查；任何尚未执行的用例不能宣称通过。



**2026-11-07 历史设计决定（不是当前执行状态）**：固定 output/<RunId>/、轻量 Result/Status、ErrorStage/JSONL、独立 Runner 配置、Eigen-only/禁 sdla 等路线当时已确定；当时尚待论文及球谐实现。现行三篇论文已阅读、球谐叶子已实现，已有用户提供的三次 CTest 通过证据；其余模块仍未编程或实际验收。本文件不改写历史审批 JSON。

## 2. 首批关键测试的具体条件

### 2.1 CLI 契约（TC-F-CLI-001）

候选子命令必须分别核验帮助、必填参数、默认值、正常退出、缺参数、非法参数、输入文件错误、输出覆盖保护。**具体候选语法、参数类型/范围、退出码与诊断字段已写入 [design/cli.md](design/cli.md)**；P0.8/P1.2 仍需批准并以实际 CLI/CTest 验证，不得把文档视为实现。

| 子命令 | 候选必填参数 | 输出/验收 |
|---|---|---|
| inspect-mat | --file PATH；可选--variable NAME（仅筛选元数据） | **只枚举/打印MAT结构**：变量名、MAT类型、rank/dims、元素数等；不得解码或打印double值；--mode/--format/--all等为未知参数parse(2)；见design/cli.md §2.0.1 |
| run-bias-occluded | --seed UINT32 --level NONNEG_INT | xwerte、ywerte1…6 的 MAT 结果 |
| run-bias-proportional | --seed UINT32 --level NONNEG_INT | 独立完整球面/逐分量噪声结果 |
| run-adaptive-init | 无 | 原始初始化四类误差结果 |
| run-adaptive-comparison | --noise-scale FINITE_DOUBLE | 带噪/无噪的原始五类 MAT 结果 |
| run-adaptive-update-rates | 无 | 1/5/11/21 帧更新频率结果 |
| analyze | --experiment NAME --input-root DIR | 独立读取历史 MAT，计算统计序列 |
| plot（可选） | --experiment NAME --input-root DIR | 图表结果；无图形后端不影响 analyze |

**DEC-008-05新决定的`inspect-mat`结构检查测试（源需求纠偏，未执行）**：原先将结构检查擅自扩展成strict/compat完整数值读取、NumPy/pandas预览和全量打印，是**未经用户要求的虚构需求，现已撤销**。沿用`TC-F-CLI-001`：准备单变量/多变量/含不同MAT class/rank的只读或隔离MAT，调用`inspect-mat --file F`仅报告真实变量名称、class/dtype、rank/dims与元素计数；`--variable NAME`只过滤结构信息（不存在返回`MissingVariable/read(3)`），不要求值为dense double或能通过`MatDenseArray`的业务shape验证。`--mode strict/compat`及`--format/--rows/--cols/--precision/--all`在本命令均按未知参数拒绝`Parse/exit(2)`。验证**不会调用`Mat_VarRead`或`MatResultReader::readNamed/readFirstCompatible`，不会输出任何数值元素**，原MAT SHA不变、无RunId/无写入。`MatResultReader::readNamed/readFirstCompatible`仍单独用原`TC-F-MAT-001/003`测试完整值读取与单句柄身份；绝不可因删除`inspect-mat`数值打印要求而删除算法所需Reader能力。

**2026-11-11 兼容首变量的同一输入身份/TOCTOU负例**：**仅对真实数值Reader而非inspect-mat**，在已有`TC-F-MAT-003`、`TC-I-MAT-001`、`TC-NF-SAFETY-001`下，准备仅含 `alpha` 的合法单变量 MAT A 和仅含 `beta` 的合法单变量 MAT B，在兼容读取的**元数据枚举后、完整数值解码前**将同一可变路径从A切换到B。要求同一成功结果的变量选择、真实解码数据与源文件身份来自**同一次 Mat_Open 句柄**；不允许旧伪代码 `inspect(path)`（句柄A）后 `readNamed(path,name)`（新句柄B）混合构造虚假成功。若发生同一 inode 原地写入或身份不一致而无法证明一致性，必须明确非成功或记录尚未解决的稳定快照边界，不得以 canonical 字符串相同作通过证据。来源见 [MAT LLD §7.4.1](design/mat_io.md) 与原 `getflowfield.m` 一次 `load` 再 `fieldnames/getfield` 的流程。此为隔离文件竞态的测试**设计**，不创建/改写真实 `origin/**`，尚未执行真实 Reader/CTest，具体公共错误码留待 DEC-008 冻结。

**统一 CLI 输出约定（项目所有者已决定）**：用户配置输出根的 CLI 选项已删除；新结果固定写入仓库根 `output/<RunId>/`。`--run-id ID` 可选，缺省独立 OS 随机生成，不消费实验 RNG；每个目录与文件均独占创建，重复 RunId/路径逃逸/指向 `origin/**` 均拒绝。纯读取/分析命令保持只读；`plot` 只有保存结果时才分配 RunId。真实 C++/CTest/安全负例仍待后续获准实施。

建议退出码类别（尚未批准）：0 成功、2 参数错误、3 MAT/输入 I/O 错误、4 格式/类型/形状错误、5 数值无效、6 输出目录保护、7 可选功能不可用。实施前冻结状态优先级与错误含义。

### 2.1.1 错误指标的有效域与“零真值零误差”假成功负例（2026-11-12，尚未执行）

沿用 `TC-F-ERROR-001 / AC-F-ERROR-001`，依据原 `transfehler_div.m:8–16`、`normrelfehler.m:5–8` 与 [KvD LLD §1.1](design/kvd.md)，要求未来真实 `test_estimators.cpp` 独立校验**两个不同指标及非成功分类**，并记录其输入/输出维度和分母来源。`transfehler_div` 的源注释虽提 `squared error`，**返回语句是 `sqrt(dot(div,div))`，不是残差平方**；此处不能因为名为 error 就把距离平方后作为作者记录的 Bias ywerte。

| 真值 u、估计 v | 按源静态公式的尺度对齐距离 | 按源静态公式的方向角（deg） | 正式 C++ 候选资格 |
|---|---:|---:|---|
| `u=e_x; v=2e_x` | 0 | 0 | 合法、幅值被剔除 |
| `u=e_x; v=2e_y` | √2（**不是2**） | 90 | 合法；已有 AN-G09 候选 |
| `u=e_x; v=-2e_x` | 2 | 180 | 合法；估计方向不能取无符号轴 |
| `u=0; v=2e_x` | **源距离表达式=0** | **未定义（源归一零向量）** | **新实现必须非成功**，不能把源退化零值计入有效平均 |
| `u=e_x; v=0` | 源除零后非有限/无定义 | 未定义 | 非成功，不默认填0或NaN为成功 |

对两个非零向量，要求单独核对解析恒等式 `E_scale = 2*norm(u)*sin(pi*angleDeg/360)`，例如估计幅度成倍但方向不变时 E_scale=0；零参考时这一恒等式**不再是允许通过数据资格的依据**。零真值、零估计、内积比率略超出 [-1,1] 的源码警告/复值在**未运行原 MATLAB**的条件下不能推称已实测；当前 C++ 的明确错误返回与源公式计算要双栏区分。未来须报告输入有效数/拒绝数、失败原因和最终 DEC-005/008 的判定，不得把无定义样本当原脚本有效的零残差。

此节只对原 `TC-F-ERROR-001` 添加区分性输入，不创建第67个 TC、不改 AN-G09 JSON 的既有候选数值，不执行 C++、MATLAB 或 CTest。

### 2.1.2 KvD 候选错误枚举必须来自同一公共合同（2026-11-12）

本次核对[KvD LLD §4](design/kvd.md) 时发现其旧失败建议写了 `SingularGeometry` 与泛化 `NonFinite`，但唯一候选 [contracts.md §3](design/contracts.md) 的 `NumericalErrorCode` **不存在这两个枚举值**。原 LLD 已纠正为只引用现有类型，禁止 C++ 开发者照旧 LLD 新建重复的本地枚举、使用不存在的标识导致编译失败或丢失 JSONL/CLI 错误含义。

| 固定数值或输入异常 | 现有唯一候选 `NumericalErrorCode` | 需要验证的差异 |
|---|---|---|
| `setofpoints` 非 N×6 | `InvalidShape` | Solver来源 `solve(5)`，不得错当 MAT Decoder 的 `decode(4)` |
| 所有 `d_i=e_z`（N>0、p可取0） | `SingularSystem` | 原 `franzeigen.m` 中轴z的平移、旋转模板均为零，两个平方范数分母为0；新C++不返回伪零运动 |
| Franz给出纯旋转、初始平移范数为0 | `ZeroNorm` | KvD旧t归一无定义，不能因omega仍可算就宣称完整KvD状态成功 |
| 原始输入含 NaN/Inf | `NonFiniteInput` | 原值必须保持来源，不能随意转成一般 `NonFinite` |
| 计算中形成无穷/NaN | `NonFiniteResult` | 与输入错误区分，保留发生的迭代和对应样本 |
| 满秩但病态的系统 | `IllConditionedSystem`（候选） | 不凭空设置阈值；真实数值证据及 DEC-005 批准后方可判定 |

此处所有映射只复用已有 `TC-F-FILTER-001`、`TC-F-KVD-001/002`、`TC-DC-KVD-001` 与 `TC-F-ERROR-001`。原 MATLAB 的奇异/NaN 警告传播没有实测 L1，不写成“原 MATLAB 返回以上枚举”；上述是**待实现的 C++ 安全行为**，没有真实 CTest/编译或已批准 DEC-008 错误码输出。

### 2.2 Bias 噪声（TC-F-BIAS-001、TC-F-BIAS-002）

原脚本三项系数是 [0.6, 2.4, 9.6]。非球面情形按 randn × 系数 × 全场平均光流模长逐分量扰动；完整球面情形按 randn × 系数 × 对应光流分量扰动；两者都要将流重新投影到切向。**不得将系数开平方**。用固定 Gaussian 序列（含零、正负非零值）对每个分量验证，核对六组误差和 xwerte MAT 变量尺寸。

### 2.3 球面采样顺序（TC-F-GEOM-002、TC-F-GEOM-003）

分别对完整八面体 8·4^i、缺损八面体 6·4^i、二十面体 20·4^i 在 i=0、1、2 核对数量、单位范数、缺损面及**相同顺序下的每行三维方向**；不能只比较无序集合或哈希。Golden 输入应由原只读源码面表/递归规则**独立生成并交叉核验 L3**，覆盖 i=0、1、2 全部有序方向；不得由被测 C++ 自算，数值容差经 DEC-005 批准，**无需运行 MATLAB**。

**2026-11-09 已取得的有序参考证据（AF-047/055，非已执行采样 CTest）**：[完整方向数据](../verification/geometry/source_direction_levels_0_2.v1.json) 已覆盖八面体8/32/128、缺损6/24/96、二十面体20/80/320，共 **714 行**。已逐值核对现存 AN-G01/02/03 的 i=0 全部34行，最大差约1.11e-16；独立 [Wolfram 源序签名](../verification/geometry/verify_source_direction_levels.wl) 在九组输出中对每行的 i·(x,y,z) 和 i²·(x,y,z) 求加权和，与另一语言重建的最大相对差1.50e-15，最大绝对差9.31e-10（加权平方、部分和量级约3e6）。这两层**属于早期阶段的加权签名证据**；随后另用 [完整714行独立Wolfram逐元素复核](../verification/geometry/verify_full_source_direction_rows.wl) 与 [固定执行结果](../verification/geometry/full_ordered_row_verification.v1.json) 对比全部2,142个坐标，最大绝对差 `1.1102230246251565e-16`、超过诊断阈值 `1e-12` 的坐标数为0，**已解除此前“未做跨语言全行逐值比较”的数学证据缺口**。参考 JSON 和新脚本的原始字节 SHA-256 已独立计算并记录在执行结果；但依然未经DEC-005批准，未生成实际采样 C++/CTest，也未触及原MAT或源 `.m` 文件。

**AN-G12 对称单步的数学实证增量（AF-044/055，未运行Adaptive CTest）**：原 `AN-G03` 的20个二十面体方向、常数近度1、理想 `p=-e_x+(d_x)d` 已通过 [Wolfram独立单步计算](../verification/adaptive/verify_AN_G12_symmetric_one_step.wl) 和 [固定数值结果](../verification/adaptive/AN_G12_symmetric_one_step_evidence.v1.json) 对照：`M1/M4` 对 `2I/3` 最大差 `1.1102230246251565e-16`，`M2/M3` 零，右端 `(2/3,0,0,0,0,0)`，`t=e_x`、`omega=0`、MSE=0。**由于源 near_new 分母有 `+1e-11`，返回近度范围 `[0.999999999921459,0.99999999999]`、系数相对原恒一近度最大差 `9.216760687991155e-11`，不能用严格等于1/初始C作为真实输出断言。** 该例只补对称理想输入的独立L3数学候选，不覆盖负/零均值、非对称6×6/退化异常、Session/原子提交，也未获Adaptive生产实现与CTest、DEC-005批准。见 [Adaptive LLD §10.2](design/adaptive_mfa.md)。

**非对称九方向 Adaptive 单步：正/负均值近度数学资格（AF-044/055/060）**：在 [Adaptive LLD §10.3](design/adaptive_mfa.md) 与 [独立Wolfram结果](../verification/adaptive/asymmetric_signed_near_one_step.v1.json) 中，以九个有理单位方向、旧源 CS 3×5 系数和固定生成运动构造两个 N×6 切向流场。正均值分支是**非恒一正近度**且 M2/M3 非零、6×6 非奇异：精确有理均值 s=2819/2700，求得 z=(s,0,0,1/5,−1/7,1/11)、归一化 t=e_x、ω=(1/5,−1/7,1/11)、**MSE=0**，条件数约2.46656。负均值分支将源系数及带符号流场一并取反：s=−2819/2700，源流程将 near 先翻正但再除旧负 s，故归一化后的近度均值为 **−1**，并满足严格恒等式 M1−⟨μ(I−ddᵀ)⟩=2I；6×6 可逆（条件数约2.61270），该构造场的源单步返回 MSE≈0.21140062、运动偏离生成值。

本案例完整保存输入、M1～M4、rhs、解、更新 near/C 和原 Condon–Shortley 相位的数值 reference。**负分支只用于原始近度均为负的非物理有符号控制流**，不得因此宣称所有物理正距离场景失败；若未来改变源归一规则，须另行审查并批准，不应在移植时暗改。当前这两例均仅为独立 Wolfram 数学执行，**没有生产 Adaptive C++/CTest/Session、没有 DEC-005 正式阈值**，归现有 TC-F-ADAPT-001/002/003 的待测数学参考，不新增 TC。
### 2.4 600 点 Adaptive 轨迹输出（TC-F-EXP-002）

原始 agent_in_sphere_adaptive_matched_filter_test.m 采用 stellen=600、anzahliterationen=4、anzahldurchlaeufe=8、帧循环 i=2:599。当前代码明确 save 五个变量：allemittransfehler、allemitrotfehler、alleohnetransfehler、alleohnerotfehler、rottransratio，均保存为同名 MAT 文件。前四项为八次运行平均得到的 600×1 缓冲；rottransratio 仅保留代码在运行循环中最后赋值的 600×1 缓冲，不能擅自平均。with_errors 历史 `alleadap*` 按 [DEC-004](decisions/DEC-004_HISTORICAL_MAT_FIELDS.md) 标为 `HistoricalOnly`、只读保留；新 C++ 结果严格只生成五项当前有效 save 字段，**不以历史来源未明阻塞开发**。

### 2.5 原始 MAT、数值容差和失败判定

**数据字典已通过独立 C++ matio 探针实际扫描**：Ubuntu 24.04 / matio 1.5.26，582/582 源 MAT 全部被逐变量 `Mat_VarRead` 解码，582 个变量均 `READ_OK`，失败 0。原始目录以 `MAT_ACC_RDONLY` 打开；见 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md)。P0.5/0.6 的文档合同、正式业务 Reader/Writer 与负例仍需测试；P10.4 独立复核尚未完成。

对 Original/Modified KvD 的 near、t、ω、终止条件逐轮比对；Adaptive 比对 6×6 系统各子块、右端、旋转、残差与球谐系数更新。 **特别核验：Adaptive oneit 返回的 error 是 MSE，sqrt(error) 在 MATLAB 内部另算但没有返回；测试记录必须分列 error_mse 和仅用于诊断的 rms，不得将二者互作。** `double` 为数值精度要求；**历史** `rtol=1e-10` 不是现行容差候选。已决定的首轮探索筛查为 `atol=0.01`、`rtol=0.1`，另对物理不变量、近零理想残差和离散行为保留严格检查；参阅 [Golden/容差提案](golden/GOLDEN_TOLERANCE_PROPOSAL.md)。须按模块记录绝对/相对误差、条件数及样本有效性，DEC-005 审批前不得作为通过阈值。不对零向量、NaN、Inf 或未定义方向误差计入成功。

**第一轮静态分支判别补充（不新增 TC/CTest）**：TC-F-KVD-001/002 除常数 near 场景，还须以同一固定流场/Franz 初态设计**非恒一近度**，分别比较 Original 的 `⟨μp⟩/⟨μd⟩/⟨μ²(t·d)d⟩` 与 Modified 的 `⟨p⟩/⟨d⟩/⟨μ(t·d)d⟩`，并记录 `ω_new` 使用旧 t 的顺序；参阅 [KvD §9.1](design/kvd.md)。TC-F-ADAPT-001/002 须覆盖正/负/零原始 near 均值，特别在 `s<0` 时归一后均值应为 **−1**（按原先翻 near、再除负 s 公式）；参阅 [Adaptive §10.1](design/adaptive_mfa.md)。这些是源码静态推导的**未来夹具资格条件**，并非已形成完整独立 L3 或通过测试。


**KvD 逐轮 trace 合同的定向断言（AF-087，仍待独立 L3/真实测试）**：针对现有 `TC-F-FILTER-001`、`TC-F-KVD-001/002`、`TC-DC-KVD-001`，先核 Franz 每轴的平移/旋转响应与分母及初始 t 范数；再以相同固定 N×6 输入对 Original/Modified 的**每个迭代 k**分别核 `near_denominator/raw/sum/sign_flip/after_sign`、`t_raw/norm/normalized`、平移 `a/b/c`、旋转 `aω/bω/cω`、使用旧 t 的时序、每个样本的预测光流及 MSE/RMS、上一轮 RMS、delta、动态门限和原 `(A&&B&&C)||k<6` 的下一轮布尔判定。最终核 **近度不重算**、`t_direction * mean(last_near)`、实际循环次数/失败类型。具体拟议存储字段由 [KvD §7–9](design/kvd.md) **单一归口**，此处不另造测试函数、Golden 数值或修改既有 TC 编号；**只有静态合同已补齐，AF-043/051 的独立数据与 CTest 仍不满足**。

### 2.6 已执行 matio 全量探针的证据与限制

| 证据项 | 实测结果 | 当前验收边界 |
|---|---|---|
| 平台与工具 | Ubuntu 24.04，GCC 13，matio 1.5.26；源参数 SHA `48c590ba2564e4bdd293d2cbecd2eef83b291452` | 独立源文件 `src/mat_data_dict_probe.cpp` 已编译运行；非正式 CMake/CTest |
| 读取 | 582/582 MAT、582/582 变量、失败 0 | `Mat_Open(MAT_ACC_RDONLY)` + `Mat_GetDir` + `Mat_VarRead`；未做数据逐元素 L2 原 MAT 值与 L3 独立参考对照 |
| 格式与类别 | 全部 MAT v5 / zlib / MATLAB double、存储 double | 当前数据不需要 HDF5、cell、struct、complex、sparse 适配；新增数据另评审 |
| 形状分布 | 10×1:80，10×3:480，35×1:4，600×1:14，600×4:4 | 真实历史结果 shape；不代表所有未来输出或算法形状 |
| 文件身份 | 582 个文件各有 FNV-1a 64 位指纹 | FNV 非 SHA-256；尚须 P10.4 独立目录/文件身份审核 |
| 未运行的验收 | 严格/兼容读取、类型拒绝负例、写回读、RAII 下异常、原文件哈希比对、C++ 求解器/CTest | 不能用探针的 exit=0 将这些 TC 标为通过 |

详细记录以 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) 的 582 条逐变量表为准，以下的 TC 状态仅表明**相关子能力有实测证据**，而非整个接口合同正式验收。

### 2.7 更新频率实验与四种分析入口（TC-F-EXP-003、TC-F-AN-001）

- 更新率当前脚本：600 点、二十面体等级 2、**1/5/11/21 帧**，输出 600×4 的 allemittransfehler、allemitrotfehler，以及 600×1 的 akrot；历史 many_parameter 额外 `alleadaprotfehler`、`alleadaptransfehler`、`rottransratio` 按 **DEC-004** 标 `HistoricalOnly`，新结果只保存两个 `allemit*` 与 `akrot` 三项；针对新结果进行需要 `rottransratio` 的分析时必须**明确报告缺失而不是构造比值**。
- 初始化分析图：allemittransfehler(1:13,1) 与 allemitrotfehler(:,1) 的数值、索引、对数纵轴语义；数据被读取不代表一定进入图线。
- 比较分析图：x=50:550 的旋转与平移方向角度误差、rottransratio 双纵轴，阈值 0.01，分段规则精确依据原 MATLAB。
- 更新频率分析图：x=50:550、阈值 0.02，四条曲线与不同半窗 2/5/10 的平滑；kurveglaetten 的边缘零值语义不替换。
- 每条曲线的**L2 输入/L3 独立计算参考**应记录原始 source MAT+variable 和 SHA-256、原只读源码中的 1-based 索引规则、x/y 数组、axis/units、分段、窗宽及有效样本状态；**不运行 MATLAB**，只比较图片像素不能算通过。

**Adaptive 输出源时序与物理单位的未执行夹具（AF-088/089，不新增 TC）**：`TC-F-EXP-003` 须从原真实相对旋转向量 `bestrot` **弧度值**独立计算 `akrot=(180/π)*||bestrot||`，输出单位 **deg**，非角速度/误差；`bestrot=(0,0,0)`、`(0,0,π/2)`、`(0,0,π)` 分别期待 **0°/90°/180°**，而来源 600×1 数组的首/末帧另按原 `zeros` 保留 0。`TC-F-EXP-002` 应记录 `erzeugeflussausnear` 的加噪前 `durchrotfluss` 与 `durchtransfluss` 并对其**平均模长之比**核对 `rottransratio`（无量纲），`nonpropfehler` 是这两项计算之后的流程：给定相同理想流的不同固定噪声样本，ratio 应一致；将 AN-G05 的单点 `mean_rot=1,mean_trans=2` 作为待独立复核候选，ratio=0.5；分母为零/非有限要单列非成功而不能被宽松容差吞没。仍核其为**第 8 次缓冲**而非八次均值。分析 `TC-F-AN-001` 只使用归档中真实存在的 ratio 变量，不给新 Update rates 补造历史字段。原源码行与期望边界见 [实验 LLD §5.1](design/experiments.md)；以上手算输入只是**测试设计、非生成/执行的 L3 或 CTest**。

**DEC-004 已选的历史字段兼容验收**（参见 [决定原文](decisions/DEC-004_HISTORICAL_MAT_FIELDS.md)）：以 L2 的 `with_errors` 七份文件与 `many_parameter` 六份文件证明历史字段只读存在；以独立 L3 小规模生成案例和未来 C++ 运行验证**新 comparison 仅五类 save、新 update-rates 仅三类 save**，两类历史 `alleadap*` 以及新 update-rates 不保存的 `rottransratio` 必须标 `HistoricalOnly`，不得写入新的 MAT。对新 update-rates 结果运行需要 `rottransratio` 的分析必须**确定性失败并报告 MissingVariable/来源类别**，旧历史目录仍能读取。关联 TC-F-EXP-002/003、TC-D-MAT-003、TC-F-AN-001/002、TC-I-ANALYSIS-001；**没有新增 TC ID，且所有新测试均未执行**。这套处理规则已经确定，不再等待历史额外字段归因。

### 2.7.1 Update rates 对照单步新系数必须丢弃的固定时序回归（2026-11-12）

**源级合同**：原只读 `agent_in_sphere_adaptive_matched_filter_many_parameter.m:134–182` 在帧首旋转`akkoeff`及四组`mitkoeff(:,:,k)`，并在 `mod(i,[1,5,11,21])==0` 时把**本帧主单步前**的`akkoeff`复制给对应组；在比较处调用`[dumi,mittrans,mitrot,error]=oneit(...)`，其中`dumi`并不写回`mitkoeff`，即**比较仅用t/omega计算`normrelfehler`，`error`返回量被覆盖而不进入保存的对照误差列；新的系数被有意丢弃**。随后`[akkoeff,aktuelltrans,aktuellrot,error]=oneit(...)`才写回主状态。与初始化和普通Comparison中的`[mitkoeff,...]=oneit(...)`不同，不能共用“所有比较都写回新coeff”的代码路径；精确对照见 [Adaptive §5.0.1](design/adaptive_mfa.md)、[Runner §8.0.1](design/experiments.md)。

**原参数哨兵替身**：四组和主系数都从源常数项`A=B_k=1`开始，帧旋转设置为I，`oneit(C)` 返回候选`C+100`，并提供可识别的运动值但不干扰主/比较状态。按 i=2..6 顺序逐帧调用（包含第4帧），未来独立替身测试必须按下表得到：

| MATLAB 帧 i | 比较 period=1 的求解输入 | 比较 period=5 的求解输入 | 本帧主单步写回后 A |
|---:|---:|---:|---:|
| 2 | 1 | 1 | 101 |
| 3 | 101 | 1 | 201 |
| 4 | 201 | 1 | 301 |
| 5 | 301 | **301（周期刷新于主单步之前）** | 401 |
| 6 | 401 | **301（上一周期快照保持）** | 501 |

对照求解每次返回的`C+100`都不得变成下一帧的比较状态。两个必杀错误变体分别是：①错误提交`dumi`，将 period5 在i=3的输入从1变成101；②主单步先提交再作周期刷新，将period5在i=5的输入从301变成401。period11/21也要用`mod(i,period)`的实际 MATLAB 1-based i 测试刷新点和前/后邻帧，不能把C++的0-based行号直接取模。上述`C`仅代表可注入系数状态，不是将3×5球谐系数约简成物理标量。

**失败事务负例**：在有旋转且i=5刷新的帧分别注入period5比较失败、主单步失败，保留`akkoeff/mitkoeff[1..4]`帧前快照；任何必需节点失败时不得**仅提交部分旋转/刷新/主系数**，应按现行Session既定策略停止不可恢复连续序列，Runner标`Incomplete/Failed`，保留错误来源和有效样本数。这是新增C++安全保障，非声称原MATLAB具有事务语义。绑定现有`TC-F-ADAPT-003`、`TC-F-EXP-003`、`TC-NF-SESSION-001`、`TC-NF-RECOVERY-001`，**未编写/执行真实测试、未生成 L3/CTest PASS、不新增第67项TC**。

### 2.8 用户审批 JSON 的数值与执行验证边界（仅规划）

**已补充具体 Golden/容差候选，但仍未测试或批准**：查看 [Golden 参考样例与宽松容差建议](golden/GOLDEN_TOLERANCE_PROPOSAL.md) 和 [12 组解析候选 JSON](golden/analytic_golden_candidates.v1.json)。其中给出 12 组根据原 MATLAB 公式静态推导的固定输入/输出（含有序采样、光流、球谐、90° 转动、误差计算、Adaptive 单步）和第一轮探索性建议 `atol=0.01, rtol=0.1`。**这些是未执行的独立 L3 候选，不代表最终 DEC-005 PASS 阈值**；离散结构、错误状态、球面/旋转不变量另有严格条件。一般 KVD/Adaptive 逐轮及 RNG 测试须补齐 L3 独立固定输入、逐节点与调用次数参考，最终结果/统计使用 L2/L4；不要求 MATLAB 采集，H-10/H-11 暂缓状态不变。

审批记录见 [DECISION_IMPLEMENTATION_NOTES.md](approval/DECISION_IMPLEMENTATION_NOTES.md)。**历史 H-01 在原审批记录中是 `draft`、`referenceConfirmed=false`，并未批准任何数值阈值**；H-03 也只是 CLI/错误/TC→参考数据方案草案，历史 H-09 不再要求 MATLAB 运行。项目所有者**后来另行确定** `atol=0.01`、`rtol=0.1` 仅为**首轮探索性筛查**，不等于 H-01 已签字或 DEC-005 最终模块 PASS 门槛已获批准；不能把被测 C++ 输出当独立真值或将未创建的测试标为通过。

| 将来需要的对照 | 预定输入/观察量 | 当前状态 |
|---|---|---|
| H-02 Sophus 取代 Rodrigues | 原 MATLAB 固定 `ω=0`、小角低于 eps、x/y/z、复合、接近 π；核对 3×3 物理旋转、Rᵀ 局部运动 | **已选路线，未运行** |
| Ivanic–Ruedenberg 原论文/勘误驱动的低阶 Eigen 球谐 | 核对九基、R0/R3/R5 全元素、相位/转置、三条路线与独立参考 | **论文已取得，Wolfram一般SO(3)精确证明已执行；低阶叶子在 `8eab5dd`、`445ed151`、`601c74de` 均由用户实测CTest 1/1 Passed。最近一次执行624非零四元数、53,532个require检查、错态与双类残差汇总；更广浮点输入、公共API/ABI、全工程/DEC-005尚未验收** |
| H-12 `output/<RunId>/` 结果及负例 | 原 `origin/**` 前后哈希不变；重复 RunId、软链接越界、异常读写、MAT 写后回读、运行输出排除在 Git 上传之外 | **只完成忽略规则的文档/配置，H-10 暂缓、writeMat=false，未运行** |
| H-04 Ubuntu VM 资源快照 | 用户提供的 `uname/df/free`；将来记录实际 CMake/CTest 和 peak RSS/wall/CPU | **只有环境资料，未批准时间/性能门槛** |

> **审批历史与当前用户授权分开**：旧 JSON 的 H-10/H-11 曾为 deferred；后来已专门允许球谐 C++，用户又提供该叶子模块一次真实 Ubuntu CMake/CTest 结果。其他模块的代码/代理自行执行项目命令/全工程集成与 MAT 写回读仍未授权；不要回写旧 JSON 或把本次输出当作全项目执行权限。

**参考术语的唯一解释**：本文件中历史标题/测试名如出现 `Golden`，除明确标明“历史 L1 分类”外，都必须逐个注明为 **L2（原作者现存 MAT）/L3（独立固定输入数值）/L4（统计）**；**L1 原 MATLAB 运行数据永久不作为强制取得/通过条件**。有编号不表示已有测试函数，唯一候选实体矩阵见 §3，真实执行证据只写入 [PROGRESS.md](PROGRESS.md)/[reviews/](reviews/) 的指定 Git SHA 记录。

### 2.9 第一轮新增的可复核测试夹具规范（AF-051，尚未建真实测试）

下表仅把模块 LLD 中新增的固定输入/预期映射到**既有 TC**，不改变 §3 的 66 项编号或将建议的测试文件说成已创建。

| TC 组 | 应准备的独立夹具/重要断言 | 预期证据 |
|---|---|---|
| `TC-F-MAT-003/004`、`TC-NF-SAFETY-001`、`TC-DC-IO-001` | 多变量歧义、兼容元数据与解码**同一 Mat_Open 身份**且路径置换不能混合两个文件、固定 `double[2×3]` 列主序、变量名白名单与路径拒绝、符号链接父目录/并发 RunId/原子非覆盖；**关闭暂存→发布前严格回读→非覆盖发布→发布后身份验证→RunContext记录Manifest**，分别注入前/后失败 | 隔离测试目录、源文件不变、具体 MatErrorCode、manifest/log 与逐元素比较；详见 `mat_io.md §8.1` |
| `TC-F-GEOM-002/003`、`TC-NF-RNG-001`、`TC-DC-RNG-001` | 完整/缺损八面体 levels 0/1/2，有序行；固定注入时先 N 次 uniform 后 3N 次 normal；原Bias的`near_last`只取最后一次uniform近度标量；0 噪声仍抽样 | 原源码 SHA、独立方向 L3/固定 RNG 输入、调用数/顺序、非有限与耗尽负例；详见 `sampling_noise.md §7.1` |
| `TC-F-KVD-001/002`、`TC-F-FILTER-001` | KvD负近度总和翻号后立即缓存`translation_after_sign`供旋转更新；平移分支先计算`raw_t_next`，仅归一化一次，再以旧t计算rot并提交新t；终端near保持本轮早期值、不得重算 | 原只读`norm_koenderink_iteration.m`/`mue_norm_koenderink_iteration.m`变量覆盖顺序、`raw_t_next`与唯一归一事件、旧t/新t及rms trace；见`kvd.md §3.4`，未做原生CTest |
| `TC-F-TRAJ-001`、`TC-F-FLOW-004` | 35/600 轨迹、N 分母、端点无有效运动、`R^\top`、小角 epsilon/反向/近 π、**同角度不同向量长度跨绝对叉积阈值**、球内正根 | 全矩阵独立 L3、状态/维度、球外失败分类；详见 `trajectory.md §6.2` |
| `TC-F-EXP-002/003`、`TC-I-RESULT-001`、`TC-NF-RECOVERY-001` | 8 次四误差平均；加噪前理想流均值比 ratio 只留第 8 次；更新率 `akrot` 由原相对 rotvec 由 rad 换算为 deg；单轮失败/Manifest、周期刷新和当前三个 save | 独立固定八轮和帧号、RunId、保存字段/源只读确认；详见 `experiments.md §8.2` |
| `TC-F-AN-001/002`、`TC-I-ANALYSIS-001` | 50:550 的 501 点、阈值等值不触发；多更新率**旋转逐段平滑且裁剪**、**平移先全长平滑且不分段**；comparison 平移不分段；缺 ratio 明确报错 | 独立 L3 对比段级完整中间数组、绘制 x/y、连续平移 501 点和原值来源；不能以“正确裁剪后的旋转内部值不同于全局平滑”作假断言。详见 `analysis_plotting.md §5.2.1/§8.1` |

**2026-11-11 MAT Writer 失败时点的具体门槛（仅文档设计）**：`TC-F-MAT-004/TC-NF-SAFETY-001/TC-I-RESULT-001` 须分别注入：（a）暂存写出/关闭失败，最终文件名**不存在**；（b）暂存回读逐值不一致时返回 `ReadbackMismatch` 且**不得发布**；（c）两个Writer竞争同一文件的非覆盖发布，一成功一 `OutputExists`，原有内容和SHA不变；（d）**发布后**最终检验或RunContext写Manifest失败，RunOutcome仍为`Incomplete/Failed`，记录已发布文件并不得把全部运行标`Complete`；（e）恶意 `name='../origin/...'` 或输出父组件在检查/创建间变为symlink时失败且原始文件未改变。Writer的 `Status` 只证明单文件操作，整个运行的完整性只由Runner/RunContext在全部期望产物验证后确定。上述均为**将来需执行的隔离负例**，不得把文字或已有582/582只读探针当成安全通过。详见 [mat_io.md §7.2–7.2.1](design/mat_io.md)。

**执行范围限界**：`8eab5dd` 与 `445ed151` 的同一球谐 CTest 均已由用户现场运行并报告通过，后者包括独立基函数、dyadic 符号及 MATLAB 相位回归；并不代表全部 66 TC 或 MAT L2/更广 L3/DEC-005 验收完成。其它模块的实际测试、负例和日志仍缺；凡在 `445ed151` 以后修改的测试必须重新实测。AF-051 保持「未整改」。

### 2.10 已可手算但未实际执行的数值/抽样负例（AF-041/047/055）

根据 [AN-G 静态公式审阅](reviews/AN_G_STATIC_FORMULA_REVIEW_2026-11-08.md) 可用于未来独立 L3 的固定条件：

| 原 TC/验证目标 | 固定条件及不可忽略的判据 | 当前实际状态 |
|---|---|---|
| TC-F-GEOM-001 / TC-NF-RNG-001 | `getvektor` 注入两个 uniform `(0.5,0)`，方向严格按 θ/φ 公式算，消耗 2 次；还须覆盖边界输入 0/1、耗尽与非法样本 | 仅静态推导；未运行 |
| TC-F-FLOW-001～004 | `d=e_z, near=2, t=e_x, ω=e_y` 的生成光流 `(-3,0,0)`；另对 `nonpropfehler` 的 **0/+k/−k** 固定正态注入，记录每行 3 次调用与最终切向投影 | 数值点已手算；独立 L3/CTest 缺 |
| TC-F-BIAS-001/002 | `iterationnumber=2` 的原 `xwerte=[6,24,96]`，缺损模式 N=6/24/96，完整球面 N=8/32/128；必须记录实际 N 与显示 x 分别来源 | 仅源表达式/形状复核；未运行 |
| TC-F-SH-001、TC-F-ADAPT-001 | 对称AN-G12与非对称九方向两类输入的正近度M1～M4/rhs、t/ω/MSE、新 near/C | 已有独立Wolfram真实数学计算：九方向正均值非对称场M2/M3非零、z精确恢复、MSE=0，另有负均值源归一与M1差2I的反例；仍无Adaptive生产CTest/Session、近奇异/退化覆盖或正式批准 |

**KvD旋转更新对翻号时点的独立检验（2026-11-11，仅设计）**：核对 `norm_koenderink_iteration.m` 与 `mue_norm_koenderink_iteration.m` 的实际语句先 `if sum<0` 同时翻转 `near/trans`，再依原/修正公式求 `transneu_raw`，按 `transneu=transneu/norm(transneu)` **一次**归一，随后 `rot=a+cross(trans,b)+c` 仍引用**翻号后、更新前**的旧`trans`，最后才 `trans=transneu`。拒绝错误变体：旋转使用刚计算的新`transneu`、旋转使用翻号前的旧trans配翻号后near、或 `translation_rule` 已归一后再次`normalize`。固定源跟踪要求 `t_before_sign/t_after_sign/raw_t_next/norm_raw_t_next/t_next/omega_before/omega_next/t_after_commit`，其中二次归一会造成数值路径不同，不能因方向理论等价而忽略。映射 `KVD-G02/G05`、`TC-F-KVD-001/002`，见[ KvD LLD §3.4](design/kvd.md)。这只是源码对照后的实现约束，尚未产生实际L3/CTest。

**Bias近度返回及合成流模长的真实源码分层（2026-11-11静态合同）**：来自 `fixed_kugel_symmetric_perfectpoints_randomnear.m` 与 `fixed_propfehler_kugel_symmetric_perfectpoints_randomnear.m`，源`near=1/entf`在N次循环中覆盖标量，**仅最后一个值**作为原返回的`near`；C++候选 `FlowSynthesisResult.nearness` 全数组是新增独立诊断，`near_last` 需与其最后一项相等。以缺损八面体 `level=0,N=6` 的注入uniform `[0,0,0,0,0,1/2]`，按源 `entf=(3u+1)/2.5`，应得到内部 `nearness=[5/2,5/2,5/2,5/2,5/2,1]`、原返回`near_last=1`，**不新增任何作者MAT字段**。同时取一条固定 `d=e_z, mu=1, t=e_x, omega=-e_y`，分别有`p_trans=-e_x`、`p_rot=e_x`、合成`p=0`；原`durchlaenge=mean(||p||)=0`，但两个分量模长平均值各为1。若`nonpropfehler`采用该理想流，**噪声系数非零但平均合成光流模长为零时仍调用3次normal**，输出噪声为零。此用例验证`durchlaenge`不能错用两均值之和，不承诺该单方向反例等于原Bias六/八面集合。对应 `TC-F-BIAS-001/002`、`TC-F-FLOW-002/003`、`TC-DC-NOISE-001`、`TC-NF-RNG-001`；见 [采样LLD §6/§7.1.5](design/sampling_noise.md)，均**未生成正式L3或实际CTest**。

**Adaptive 非对称且可逆的矩阵求解资格（2026-11-11，静态数值参考）**：使用已有 `TC-F-ADAPT-001/002`、`TC-DC-ADAPT-001` 与 `TC-NF-NUM-001/002`，不新增正式 TC。在固定六条方向 `d=[+ex,-ex,+ey,-ey,+ez,-ez]` 上令原 raw near `r(d)=1+x/2`；原CS系数是 `C(1,1)=sqrt(4*pi)`、`C(2,3)=-(1/2)*sqrt(4*pi/3)`（其余零），所以原近度严格为 `(3/2,1/2,1,1,1,1)`、`s=1`，全部正且无归一除零。令源一致的切向光流 `p_i=-r_i*(I-d_i*d_i^T)*ey`。应逐字段得到 `M1=M4=(2/3)I3`、`M2=0`、`M3=(1/6)[ex]_cross`，完整 `M != M^T`，同时 `det(M)=64/729 != 0`，`rhs=[(2/3)ey;(1/6)ez]`、`z=[ey;0]`，故 `omega=0`。这排除把原一般块阵当作自伴/对称阵直接调用 Eigen `LDLT` 的候选方案；**禁止**只用对称三角部分、无声明地对称化 M 或把逆换成不同的正规方程。数值测试必须比较**完整** `M,b,z`、`||M*z-b||` 和错误状态，并对一般非对称/病态输入验证分解方案。原源码矩阵来自 [`adaptive_mfa.md §2.2.1`](design/adaptive_mfa.md)，对称充要条件见 [`ADAPTIVE_MEAN_ZERO_CONDITIONING.md §3.1`](design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md)。已核的是有理数学恒等式，不是生产 Eigen/CTest、MATLAB运行或DEC-005批准。

**负均值近度的代数资格补充**：`near_raw=[-1,-3]`，`s=-2`，原源先翻成`[1,3]`再除旧`s`，得 `near_normalized=[-1/2,-3/2]`；这**等价于** `near_raw/abs(s)`，**不等价于** `near_after_sign/abs(s)`。对`s=0`另设明确非成功，保持主系数未提交。该手算仅为分支数值真值，尚不能当作完整 N×6/3×5 单步L3；见[Adaptive LLD §2.1/§10.1](design/adaptive_mfa.md)和[均值理论](design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md)。沿用 `TC-F-ADAPT-001`、`TC-DC-ADAPT-001`。

所有例子不新增 TC ID，不替代现有 66 TC 的文件创建、测试执行、L2/L3/L4 独立证据及 DEC-005/008 正式审批。负有限噪声的 CLI 接收策略仍须批准；原 `.m` 未显式禁止该数值。

### 2.11 分析分段与平滑数据域的区分性测试（AF-075/076，规划）

同一组有 **4 个有效旋转区间**的合成 ratio 与非恒定误差输入，应建立两类独立 L3：

1. 多更新率**旋转图**：分段时读取每个区间切片（列 2/3/4），每段分别调用窗口半宽 2/5/10 的平滑；保存完整的局部零边界和裁剪后 x/y，不允许将源逻辑写成全局预处理。若全局平滑后按 **同样有效内部区间**裁剪，内部均值应**一致**，故不得将差异作为正确性断言。要查的是窗口调用域、完整临时数组、段边界与异常/日志身份。
2. 多更新率**平移图**：四列各以全部 600 行构造数据（**第一列原样，第二/第三/第四列**分别平滑半宽 2/5/10），统一绘制 **50:550 共 501 点**、无分段掩码；comparison 实验平移 mit/ohne 亦全区间不分段，旋转图另按阈值分段。必须选有阈值穿越和间隙的 ratio，确保平移线**不会丢掉中间 x**。

**2026-11-11补充的可复核固定输入（静态推导，尚未执行）**：沿用原 [analysis_plotting.md §5.1.1](design/analysis_plotting.md) 源分支，令 `t=0.02`（comparison改`0.01`）、`δ=t/2`，600行 `rottransratio(i)=t+δ`，仅 `i=120..179,260..319,400..459` 为 `t−δ`，预期 `begin=[50,180,320,460]`、`ende=[120,260,400,550]`；四段长度71/81/81/91。改 `i=121` 为严格 `t` 时分段不变；全高于阈值时 `begin(2..4)` 和 `ende(1..3)` 为0，应在C++解引用前以 `InvalidSegment` 显式失败；多于三次区间切换时不得偷偷改成自动绘制五段，须记录原始 `j/zone` 与第四段覆盖行为，安全拒绝还是兼容显示仍待P0.8确定。此处的数组/状态为**手工源控制流推演**，不是MATLAB实测、独立L3生成或CTest通过。平滑短段的空绘图区间不是可无证断言为原MATLAB异常。

沿用 `TC-F-AN-001`、`TC-I-ANALYSIS-001`、`APL-G03/04/05`，不新增 TC ID/测试 PASS；完整独立参考、Analyzer 和 CTest 尚未建立。旧扫描报告中关于“正确裁剪后的旋转内部值必然不同”的说法已在 [纠偏说明](reviews/AUDIT_ANALYSIS_SEGMENT_SMOOTHING_CORRECTION_2026-11-08.md) 限定，旧扫描仍保留历史原文。

### 2.12 分析错误枚举到 CLI 退出码的合同负例（AF-077，未测试）

根据 [contracts.md §3.1](design/contracts.md) 的**唯一候选**映射设计以下输入与 `stage/exit` 断言（非已经执行）：

| 原 TC | 固定触发条件 | 预期候选返回 | 当前状态 |
|---|---|---|---|
| TC-I-ANALYSIS-001 | 新 update-rates 结果不含历史 `rottransratio`；或指定目录缺实际 MAT | `AnalysisErrorCode::MissingVariable/MissingResult`，`read`/exit 3；记录原path/variable及完整`std::forward_list<ErrorFrame> causes`，若发生Reader→Analyzer等真实包装，依次保留各层domain/code/stage/message与已知上下文 | 仅计划 |
| TC-I-ANALYSIS-001 | MAT 解码后 shape 不能满足源曲线合同 | `AnalysisErrorCode::ShapeMismatch`，`decode` / exit 4 | 仅计划 |
| TC-F-AN-001、TC-I-ANALYSIS-001 | ratio 形成非法分段、分析值非有限或统计未定义 | `InvalidSegment/NonFiniteInput/UndefinedStatistic`，`analyze` / exit 5 | 仅计划 |
| TC-I-CLI-001、TC-I-ANALYSIS-001 | 非法 CLI `--experiment`；与合法命令内部传给 Analyzer 的 unsupported mode 分开 | CLI 无效枚举 `parse` / exit 2；运行期 `AnalysisErrorCode::UnsupportedExperiment` 为 `analyze` / exit 5 | 仅计划 |

本表不重新定义 enum、增删 66 个 TC 或签署 DEC-008；真实接口、JSONL 转义/底层原因传播与 CTest 待后续授权和证据。

### 2.12.1 跨模块 Result/Status 左右值生存期负例（2026-11-11，设计候选）

现行 [contracts.md §3](design/contracts.md) 已修正旧 `Result::value() && -> T&&` 的临时宿主悬垂风险，候选合同为左值只借用、右值取出自拥有值；`Result/Status::error()` 同样如此。沿用既有 `TC-DC-DOMAIN-001`、`TC-DC-IO-001` 和 `TC-NF-OOP-001`，建议未来在 `tests/contracts/test_result_lifetimes.cpp` 设计以下独立失败/成功断言（**文件未创建、未编译**）：

| 输入/操作 | 必须验证 |
|---|---|
| 局部左值 `Result<MatDenseArray,E>` 的 `value() const &` | 所借引用只在Result存活且没有被移动时有效，源MAT句柄释放后数组自己拥有元素 |
| 临时成功Result的 `value() &&` | 取出**值对象**而非 `T&&`；持有到原Result临时对象析构后仍有效；move-only类型的移动构造可用 |
| 临时失败Result、Status的 `error() &&` | `Failure<E>` 按值拥有 `Code + ErrorDetail`，源 RunId/阶段/路径不在临时析构后悬垂 |
| const临时对象访问及非法成功/失败反向访问 | 不得通过const右值借用获得短命引用，错误访问必须符合最终P0.8异常/断言决议；不能返回伪成功/默认值 |
| variant构造/赋值发生异常或进入 `valueless_by_exception` | `hasValue()==false` 不得冒充变体内存在有效 `Failure<E>`；需真实故障注入和不丢失错误来源的安全处理策略后才能判通过 |

**状态**：仅消除设计中的高风险引用返回形式，并补上覆盖责任；ABI/模板ODR、异常策略、真实代码/CTest与P0.8批准均未完成，不能关闭DAR-003或宣称TC-PASS。

### 2.12.1A 默认构造 Result/Status 会制造伪成功的负例（2026-11-12）

[公共合同 `3.B](design/contracts.md) 已修正 `Result<T,E>` 在T可默认构造时会默认构造variant的成功分支，以及 `Status<E>` 的默认`monostate`会无条件成功这一**真实C++17 API设计缺陷**。无论未来`hasValue()`如何实现，都不能把“没有真实调用求解器/Writer”当成成功。复用现有`TC-DC-DOMAIN-001`、`TC-NF-OOP-001`、`TC-F-ERROR-001`，作为**未来编译+运行**时的定点断言，当前未执行：

| 定点条件 | 待核对的编译/运行结果 |
|---|---|
| `std::is_default_constructible_v<Result<double,E>>`、`Result<FlowField,E>`、`Status<E>` | 三者必须为`false`；`Result<Scalar,E> r;`和`Status<E> st;`应产生确定编译错误，不是默认`hasValue()==true` |
| `Result<T,E>::success(valid T)`与`::failure(Failure<E>)` | 必须分别直接建立有效成功alternative与完整失败alternative，包含`code`、`ErrorDetail.stage`、`run_id`若存在；不得通过旧默认Success再赋值/改`variant`类型 |
| `Result<move-only T,E>`、右值`value()/error()`与const右值禁借用 | 右值返回自拥有值、调用后源对象禁止再次当数值有效的业务对象；无悬垂引用 |
| `std::variant`异常时构造失败、原结果仍存在/转移 | 不允许`valueless_by_exception`伪装成带`Failure<E>`的完整失败；是否能在所有异常路径排除无值态需要受限实现与故障注入，`DEC-008`未批准 |
| 默认`MotionEstimate`的Eigen未初始化值、业务`success(T)`误用 | 即使工厂创建成功，仍须由求解器先保证T的数学/有限性；禁默认Result不等于自动验证任意T |

**特别限制**：建议初版不可变结果封装、private带标签构造和禁止默认构造只是在设计上缩小伪成功和`valueless`路径；无真实头文件和编译前**不能声称`std::is_default_constructible_v`已测通过或绝无异常**。DEC-008详细候选见[TODO `2.2](TODO.md)，除已选择的variant/JSONL方向外其它选项未批准；H-10/H-11等六项仍无限期延期，不在本轮新增生产C++、CTest或MAT写入。

### 2.12.2 `Result/Status` 私有variant不等于稳定ABI（2026-11-11，设计更正）

[contracts.md §3.A](design/contracts.md) 已明确：即使 `payload_` 是 private，值类型 `Result<T,E>` / `Status<E>` 的 `std::variant` 仍决定公开类型的大小/对齐、移动/析构、标准库及二进制ABI。**只在源码调用者隐藏容器成员 ≠ 对编译器或动态库消费方隐藏对象布局**。沿用既有 `TC-DC-DOMAIN-001`、`TC-NF-OOP-001`，按批准的部署模式至少需检查：公共头独立编译与必要 include；两个翻译单元跨TU传递合法值/错误/移动；有实际共享库边界时生产者与消费者的编译器、标准库、编译选项、版本重编策略；异常/valueless状态与资源析构。在未定义稳定二进制升级契约前不得声称跨编译器版本ABI兼容，亦不得擅自把轻量`std::variant`技术选择换成另一个未经批准的设计。这是**代码实施前设计义务**，没有创建公共头/真实ABI测试，不关闭DAR-003。

**2026-11-14 低阶球谐叶子与公共ErrorDetail的真实适配边界**：源[Harmonics LLD `2.1](design/harmonics.md)及`include/egomotion/harmonics/low_order.hpp`表明已存在**局部**`low_order::ErrorCode/Result`，它在非法alternative访问时由标准库`std::get`抛`std::bad_variant_access`，且`Error.message`是`const char*`静态文本；这不等于公共`Result<T,NumericalErrorCode>`已实现，也不符合用户后来批准的公共`std::logic_error`签名。未来适配层必须`if (!leaf.hasValue())`后读取自拥有的`leaf.error()`，复制静态消息到`ErrorFrame.message`，原始枚举家族写`egomotion::harmonics::low_order::ErrorCode`，`stage=Solve`，外层若经Runner包装则顶层可为`ExperimentErrorCode::SolverFailed`且`causes=[low_order frame]`；不要误吞`bad_variant_access`当普通科学错误。

**八个真实局部ErrorCode必须全部检查**：`NonFiniteInput`、`NotUnitDirection`、`NotProperRotation`、`NonFiniteResult`、`ComplexResidue`、`InvalidMethod`、`InvalidSampleShape`、`ZeroProjectionNorm`；其中`NotProperRotation`、`ComplexResidue`、`InvalidMethod`在当前公共`NumericalErrorCode`**无无损一一对应项**，不能`static_cast`按底层整数或改称`InvalidShape/NonFiniteResult`伪成功。复用现有`TC-F-SH-001/002`、`TC-NF-OOP-001`、`TC-I-RESULT-001`的待授权隔离适配测试，逐个验证原域/码/消息拷贝/链序列化和无伪造额外帧；**当前未写/未运行桥接或CTest**，与固定`5e522198`下叶子3/3已通过是两种不同证据级别。

### 2.12.3 DEC-008所有子项已决定后的定点测试合同（2026-11-13，未执行）

**2026-11-13无包装的顶层错误家族遗漏整改**：当Reader/求解器Failure没有发生后续包装时，`ErrorDetail.causes=[]`仍是合法情况。此前JSONL只规定顶层`error_code`，直接MAT与直接Numerical的`InvalidShape`会同名歧义。现已要求**顶层`error_domain`必填**，并按现有枚举家族确定性输出；分别断言`MatErrorCode + InvalidShape + decode(4)`及`NumericalErrorCode + InvalidShape + solve(5)`，不凭`message`或退出码猜family。由[contracts §3.0.2](design/contracts.md)维护，仍属既有`TC-NF-LOG-001/TC-I-CLI-001/TC-F-ERROR-001`，未执行真实JSONL/CTest。

**2026-11-13追加的真实异常安全整改**：先搬走旧`causes`后再分配`push_front`节点可能在`std::bad_alloc`时失去可用的原始链。候选已在[contracts §3.0.2](design/contracts.md)改为**先完成可失败的新frame构造/`emplace_front`，然后以兼容allocator的`splice_after`接入旧链**；完整注入场景在下表原`TC-NF-LOG-001`行。**这是修正错误包装时序的具体接口设计缺陷**，不是新增正式TC或已运行C++故障注入。

所有者已决定：01不承诺跨版本稳定ABI；02选择**明确消息的`std::logic_error`方案B**、`tryValue/tryError`仅必要时增加；03采用不可默认构造、工厂创建、无公开assignment/emplace；04采用C++17的**`std::forward_list<ErrorFrame>`完整包装链**；05`inspect-mat`只打印MAT结构、不显示元素；06部分结果/Manifest由RunContext持有，失败Result只携完整Failure。以下**复用已有TC**，不增加正式测试编号：

| 既有TC | 未来需核验的确定条件 | 不可接受的错误表现 |
|---|---|---|
| `TC-DC-DOMAIN-001`、`TC-NF-OOP-001` | 失败Result上调用`value()`、成功Result/Status上调用`error()`：分别抛`std::logic_error`，消息必须包含`Result::value()/error()`或`Status::error()`、错误alternative状态；valueless提供**不同的明确消息**。如需要C探测再验证指针生命周期 | 抛仅`std::exception`不明确类型、直接露出`std::bad_variant_access`、静默返回0或伪造Failure；正常业务错误不可走异常 |
| `TC-DC-DOMAIN-001`、`TC-NF-OOP-001` | `Result()/Status()=delete`、private tagged success/failure、没有公开assign/emplace；`ThrowOnCtor/Copy/Move`在第N次操作抛`std::runtime_error`/`std::bad_alloc`，验证移动后对象、析构安全与无伪成功 | variant valueless状态被当成Failure、未创建完整Result就假报成功、把数学失败混成异常 |
| `TC-NF-LOG-001`、`TC-I-RESULT-001`、`TC-NF-RECOVERY-001` | 依次产生`MatErrorCode::InvalidShape@Decode`→`AnalysisErrorCode::ShapeMismatch@Analyze`→`ExperimentErrorCode::DataMismatch@Simulation`，顶层Experiment及`causes=[Analysis,Mat]`；**在原inner已有`causes=[Mat]`时注入新frame字符串构造异常和`emplace_front`分配异常，要求原inner链仍可读取`[Mat]`**；成功建新节点后`splice_after(new_head, inner.causes)`，核顺序、原链清空与分配器相容；再核JSONL及空链`[]`；即使空链也应显示**顶层error_domain**，分别给`MatErrorCode::InvalidShape`与`NumericalErrorCode::InvalidShape`以区分。 | 不得**先`move(inner.causes)`后`push_front`**导致异常时链丢失；不得漏帧/倒序/虚加CLI节点；标准`std::forward_list::splice_after`在同分配器前提下不抛异常，整体拼接复杂度对链长线性，不可假称O(1)；未执行CTest。 |
| `TC-F-CLI-001`、`TC-I-CLI-001`、`TC-F-MAT-002` | `inspect-mat --file F [--variable NAME]`仅读MAT结构元数据；多种MAT类型均可元数据枚举、变量筛选，无数值载荷访问/输出；未知`--mode/--format/--all`解析失败 | 自动进入业务readNamed/readFirstCompatible、读取/打印矩阵值、输出NumPy/pandas内容、改变原MAT SHA |
| `TC-I-RESULT-001`、`TC-NF-RECOVERY-001`、`TC-NF-SESSION-001` | Comparison八次聚合第5次失败、Writer回读失败、Manifest写失败、Adaptive本帧失败；Failure保留每层完整causes，RunContext独占有效/失败/无定义/部分结果计数；只有全部成功才Complete | 把前4次改当完整八次、只保留最终一条错误、Writer单文件成功即标Complete、Manifest未落盘却声称成功 |

**说明**：设计已获选择不等于真实API/CTest、MAT写回读或总体基线获得授权；H-10/H-11等六项继续无限期延期。仍需等获准后对真实C++17模板/Result移动/forward_list链异常/JSONL序列化实施测试。

### 2.13 轨迹与近度的符号级候选（AF-048/080，未运行）

参见 [trajectory.md §6.3](design/trajectory.md) 的 TR-C01～06：三点局部轨迹 `Rᵀ` 的正负号、严格反向输入 `(e_x,-e_x)` 的原函数**确定**输出零旋转向量（非有效折返），以及球内旋转视线的 `near=4/3` 与反向射线 `near=4/5`。新增的TR-C05必须以两个**夹角都为90°**的输入 `v1=e_x, v2=(0,5e-12,0)` 和 `v2=(0,2e-11,0)`，静态预测原旋转向量模长分别为 `(π/2)*5e-12` 与 `π/2`，验证原 `norm(cross(v1,v2))>1e-11` 属**绝对交叉幅度**而非旋转角阈值；恰等 `1e-11` 浮点条件单独记录舍入，不作稳健跨编译器黄金值。源保真与几何安全模式的失败决议必须分开。另用**TR-C06球外内向正根反例**确认：o=(5/4,0,0), n=-e_x 时，原式 Delta=1、rho=9/4、near=4/9 虽均有限、根为正，但相机**不在球内**，生产 SphereEnvironment 必须先按定义域拒绝；对球面边界 o=e_x，外向 rho=0、内向 rho=2，后者虽正也不是严格球内。不能将“Delta>=0 且 rho>0”当作球内资格。将六组固定输入关联现有 `TC-F-TRAJ-001` 与 `TC-F-FLOW-004`；要求实际 L3/CTest 分别报告源表达式数值、候选安全失败、端点有效性、坐标单位和真实误差。本文只给出静态可手算输入资格，**未生成独立 L3、未测试，也不批准 DEC-005/008**。

### 2.14 AN-G01～03 初始面表行序的静态资格（AF-047/055）

源码面表按 `level=0` 的三顶点单位中心公式，已经形成 [AN-G01/02/03 完整 8/6/20 行符号推导](reviews/AN_G_LEVEL0_FACE_ORDER_STATIC_2026-11-08.md)；关联原 `TC-F-GEOM-002/003` 的初始面顺序及零光流列。**本段为历史静态审查结论**：当时 level=1/2 尚无完整方向文件；当前已在 `verification/geometry/source_direction_levels_0_2.v1.json` 保存全714行有序候选并进行独立Wolfram加权行序校验，但仍缺原生采样器/CTest、正式数值容差及核准来源身份，**不能据此将 TC/AF-047/055 标为PASS**。无 MATLAB/L1 强制门槛。

### 2.15 球面递归行号/面 ID 的静态合同（AF-081）

参见 [sampling_noise.md §7.1.2](design/sampling_noise.md) 与 [四叉路径审计记录](reviews/AN_G_RECURSION_INDEX_STATIC_2026-11-08.md)。针对既有 `TC-F-GEOM-002/003`，除 `level=0` 面表外，还必须在 `level=1/2` 区分**源原面 ID**、**输出块顺序**、`A/B/C/D` 子面序列及其 MATLAB 1-based 行号。先核 `32/24/80`、`128/96/320` 行数，再按深度优先字典序 `A,B,C,D`（层内四叉数字）逐行与**独立**数值参考比较，不能先排序后只检查集合相等。

示例：缺损八面体输出首块来自原面 2，`L=2` 的 `AA`/ `DD` 分别位于行 `1/16`；第二块来自原面 3，分别在行 `17/32`；二十面体原 `F` 第 20 面的 `L=2` 叶子占行 `305…320`。这些是**索引恒等式**，没有实际计算任何一行坐标，不能标 L3/CTest PASS；失败/非有限边界及 DEC-005/008 仍待执行和批准。

### 2.16 Bias 聚合中的 xwerte 末运行覆写与四组 y 平均（AF-083，未测试）

原 `graphs_bias_test.m` 在 `k=1:number_of_cores` 读取各运行 `xwerte`，但**每轮直接覆写**它；只有 `ywerte3…6` 累加后除以运行数。故 `APL-G01` / `TC-F-BIAS-003` 的区分性夹具应分别检查：

| 输入（均为规划、非历史真实值） | 应比对的静态合同 |
|---|---|
| K=1；合法 x 和 4 个 `(iterationnumber+1)×3` 的 y | `x_plot=x^(1)`；各 `ybar_j=Y_j^(1)`；理论线 `100/sqrt(x^(1))`（按原绘图行 3…end） |
| K=2；两组 x 同 shape 但**故意不同**，两组 y 不同 | `x_plot=x^(2)`，**不能算 `(x^(1)+x^(2))/2`**；仅四组 y 算术平均，参考线取末轮 x。这是检测更新规则的人工负例，不是作者标准 Bias 源输出 |
| K=0、文件/变量缺失、y shape 不一致 | 不得将空除数/缺字段/shape 错误包装为成功；C++ 的具体显式错误按尚未批准的 `contracts.md`、分析 LLD 判断，**不宣称原 MATLAB 有相同安全错误** |

需保留实际 `k` 顺序、末轮 x 的 L2 MAT 路径/变量与四组 y 的每轮身份；参考线和输出横轴均不得替换成物理实际视线数量。此处不新增 TC、真实测试实体或通过记录；须在 AF-050/051 后续独立 L3 与 Analyzer/CTest 中真正验证。

**Bias 原理论线与图像数值的区分性回归（2026-11-12，仅源码静态输入）**：沿用 `APL-G01 / TC-F-BIAS-003 / TC-I-ANALYSIS-001 / TC-F-CLI-005`。原 `bias_test/data_analysis/graphs_bias_test.m:20–79` 用每次后覆盖的 **最后一个** `xwerte` 生成绘图横轴与 `100/sqrt(xwerte(j))`，不是根据完整球面真实点数 `N_actual`。取完整球面Bias细分 `i=2`（MATLAB绘图行 `j=3`），`xwerte=6*4^2=96`，`N_actual=8*4^2=128`；正确参考 `100/sqrt(96)≈10.206207`，错误的实际N替代为 `100/sqrt(128)≈8.838835`，比值固定 `sqrt(4/3)≈1.1547`。两个 Bias 实验都保留**原存**xwerte，真实方向数量只允许在元数据另列；K>1情况下必须先按源 `x^(K)` 做参考。

**可视化结构固定合同**：原 `graphs_bias_test.m` 使用 `loglog` 的**单个x对数轴＋单个y对数轴**（不是两条y轴）；`i=1:3` 三种噪声列各画四组 `ywerte3..6`，另外一条理论参考线，一共 **1+3×4=13条绘图曲线**，但源只提供 `legend('a','b','c','d','e')` 五个文本标签。不能从这五个字母推导出13条曲线的数值字段、误差方法与噪声列，源数值身份须靠 MAT 源变量及索引保存。源显示窗口 `xlim=[100,3000000]`、`ylim=[0.06,200]`；上面 `x=96` 虽已形成数值/曲线，却位于窗口之外，**禁止因画布不可见而删除数据、修改 `xwerte` 或将该样本判为输入错误**。当误差0或不满足对数可显示条件时，保留有定义的数值并将显示过滤与真正NaN/Inf、shape及无定义统计分开；原MATLAB具体图像/legend绑定没有本轮运行证据。

**验收范围**：用固定 `K=1/K=2` 的两组不同 x、四组3列 y，分别保存原值与可视范围，断言最后一次x覆盖、四组y均值、13条序列归属和理论线，且默认不会把作者五标签解释成完整13字段映射。修正的是源数值/显示合同，尚未创建 Analyzer/PlotExporter、独立L3 Golden或CTest；不新建TC或改DEC-005/008。

### 2.17 PlotErrorCode 四种失败的分层负例（AF-084，尚未执行）

`IPlotExporter::exportPlot` 的四值只在 [分析 LLD §2](design/analysis_plotting.md) 定义；**唯一候选 `exit/stage` 映射见 [contracts §3.1](design/contracts.md)**。复用现有 `TC-F-CLI-005`、`TC-NF-HEADLESS-001`、`TC-I-RESULT-001`、`TC-NF-SAFETY-001`、`TC-I-ANALYSIS-001`，不产生新 TC：

| 只在被授权测试时注入的情境 | 预期候选诊断与副作用边界 |
|---|---|
| `plot` 请求缺可选图形后端 | `BackendUnavailable`，`plot/7`；同一源数据的 `analyze` 不应需要后端、不产生伪造数值错误 |
| 已得到 AnalysisDataset，但某 CurveSeries 不符合绘图接口约定 | `InvalidSeries`，`plot/5`，不可误认为后端不可用；原本在 Analyzer 阶段失败的 MAT/shape/分段错误须保留该原错误而不是改成 PlotErrorCode |
| 请求持久输出时路径指向 `origin/**`、已存在 RunId/文件或越界目标 | `OutputForbidden`，`output/6`，不能修改原文件、不得覆盖已存在目标；底层已知代码、路径及 RunId 保留 |
| 合法隔离目标中实际写图失败（磁盘权限/写入中断等） | `WriteFailed`，`output/6`；保留失败可追踪性，不把不完整图像/Manifest 伪装成功，不改变既有分析结果 |

这里所有类型、阶段、退出码都是**待 DEC-008 批准的测试候选**；实际图形后端、JSONL、权限失败注入、真实 CTest 与输出文件系统安全尚未执行。图形能力缺失不能阻碍核心数值验收。

### 2.18 三份原始 illustration 的数值与显示忠实性（AF-090/091，候选未执行）

原 `illustrations/spherical_harmonic_illustration.m:10–30` 与核心求解用 `help_functions/kugelflfunc2.m` **不能共享“同一归一化球谐图”断言**。以下只在现有 `TC-F-AN-002/AC-F-AN-002` 下新增静态核对资格，不新增 TC、C++/绘图产物或运行证据。唯一显示算法候选合同在 [分析 LLD §3.1](design/analysis_plotting.md)。

| 源入口及独立输入候选 | 预期原脚本语义与错误边界 | 真正尚缺的证据 |
|---|---|---|
| `spherical_harmonic_illustration(0,0)` | 41×41 的 theta/phi 网格、`P_0^0=1`、`yy=1`、`rho=8`；**不等同**于 `Y_00=1/sqrt(4π)` 的核心实球谐数值表示 | 独立参考全网格几何值与真实绘图数据 |
| `spherical_harmonic_illustration(1,0)` 与 `(1,1)` | `(1,0)` 参考 `rho=5+3cos(theta)`；`(1,1)` 取含 Condon–Shortley 相位的 `P_1^1(cosθ)cosφ`，**没有负 m 的 sin 分支**，面坐标由径向 `rho` 换算。非法 order、零归一尺度应有明确 C++ 非成功状态；不宣称原 MATLAB 报相同错误码 | 独立 L3 几何节点/符号/边界及未来可选绘图适配测试 |
| 圆示意（600、半径 0.6、高 0.3）与正弦示意（600、幅 0.5、长 1、周期 2、高 0.3） | 两者均 `sphere(50)`，**只把 y<=0 的球面 z 置 NaN**；需核原轨迹点和可见半球遮罩。正弦脚本真实标题仍写 `circular trajectory within the sphere`，要以**源标题 vs 轨迹类别**两个字段记录，不擅自更正为“原版标题” | 可选图形后端输出、原数学几何数组及 source SHA、原字面文本与 UI 来源标签 |

**边界**：`TC-F-AN-002` 是可选图示数据的一致性审查，不意味着逐像素图形相同；核心 `TC-F-SH-001/002` 应继续以归一化九实基、R3/R5 全矩阵独立 L3 进行数值验收。未获 H-10/H-11 授权，不运行 MATLAB/Octave、CMake、CTest、绘图或 L3 生成；不改变 66 TC 总量、DEC-005/008 或软件状态。

### 2.19 六方向 KvD/Franz 逐轮参考：历史手算与现行精确Wolfram执行（AF-043）

本节是 [KvD LLD §9.2](design/kvd.md) 的**手工代数候选**到既有 `TC-F-FILTER-001`、`TC-F-KVD-001/002`、`TC-DC-KVD-001` 的映射；并非正式生成的 L3、真实软件运行或新 TC。输入是六行人为给定的 `[d_x,d_y,d_z,p_x,p_y,p_z]`，**不是原采样函数的等级 i=0 输入**：`(+e_x;(0,-1/2,0))`、`(-e_x;(0,1/2,0))`、`(+e_y;(-1/2,0,0))`、`(-e_y;(-3/2,0,0))`、`(+e_z;(-2,0,0))`、`(-e_z;(-2,0,0))`。

| 要核对的诊断节点 | 纸面候选结果与区分意义 |
|---|---|
| Franz 每轴响应与平方范数 | `trans numerator=(6,0,0)`、`rot numerator=(0,0,2)`；两类每轴 denominator 均为 4，得 `t=(1.5,0,0)`、`omega=(0,0,0.5)` |
| 第 1 轮近度与两种平移 | `h=1/(1+10^-13)`、`near=[0,0,h,h,2h,2h]`；Original `a=-(5h/3)e_x`、Modified `a=-e_x`，两者 `b=c=0`；归一前 t 不同、最终方向相同，不能只查最终方向 |
| 共享旋转/残差/停止 | `a_omega=(0,0,1/3)`、`b_omega=0`、`c_omega=(0,0,1/6)`、`omega_new=(0,0,1/2)`；候选 `MSE=(5/3)(1-h)^2`、`RMS=sqrt(5/3)(1-h)`、`grenze=RMS/(6000)`、最低 `x=6`，末轮 `t=h e_x`；真实浮点求解/次数仍须复核 |
| 无法由此例验证的内容 | 该例 `c_O=c_M=0`，**不能验证 near² 权重分支是否实现正确**；已有 [KvD §9.3](design/kvd.md) 的**第二组六方向斜向静态候选**（`eta=1/(1+2epsilon)`、`beta=1/(1+epsilon)`），其 `c_O=(4/3)eta² e_x` 与 `c_M=(2/3)eta e_x` 均非零，可作未来独立 L3 输入；仍缺 near 翻号、200 次上限、异常输入、一般 L3 与真实 CTest |

**§2.19初次登记时的历史状态**：上述表是纸面解析/静态源码追踪，当时没有独立保存完整数值参考。**最新数学状态已更新**：下文 §2.19.1 记录四组精确Wolfram实际运行的6轮trace及固定输出，不得将此句继续当成当前“数学计算未执行”。但生产C++/CTest仍未获本模块运行授权，最终 DEC-005/整体AC仍不批准；AF-043/051继续开放。

### 2.19.1 独立Wolfram四条六轮trace实算（2026-11-09，数学子证据已执行）

[源赋值先后Wolfram脚本](../verification/kvd/verify_six_direction_exact_trace.wl) 已使用精确有理 `epsilon=1/10^13`，对 §9.2 轴向六方向与 §9.3 斜向六方向**均从原Franz初态进入**，分别执行 Original/Modified KvD 六轮。四条路径累计 **24个精确数学迭代轮次**，已生成 [逐轮数值JSON](../verification/kvd/six_direction_exact_trace.v1.json)，包含模板响应/分母、各轮 near、符号翻转、平移 a/b/c 和未归一化候选、旋转 a/b/c、归一化姿态、MSE/RMS、`delta`、动态门限、各轮是否退出及末次尺度；有14/14 固定公式自检。

| 路径 | 第1轮raw t的x分量 | 首轮RMS | 最终 `trans_x` | 循环次数 |
|---|---:|---:|---:|---:|
| 轴向 Original | `1.6666666666665` | `1.2909944487356765e-13` | `0.9999999999999` | 6 |
| 轴向 Modified | `1` | 同上 | 同上 | 6 |
| 斜向 Original | `2.999999999999167` | `2.3804761428471544e-13` | `1.6666666666663668` | 6 |
| 斜向 Modified | `1.6666666666665333` | 同上 | 同上 | 6 |

两夹具的 Franz 平移皆为 `(1.5,0,0)`、每轴模板分母 `4`；第一夹具旋转为 `0.5e_z`、第二夹具旋转为0。首轮 `delta=RMS<1e-7`，第2至6轮 `delta=0`；因此`k<6` 分支强制前五轮继续，并在第六轮后停止。**这仅证明这两组夹具**的6轮结果，不代表所有运动6轮收敛，也不覆盖 near 翻号/200轮/零分母或一般噪声。现有 TC-F-FILTER-001、TC-F-KVD-001/002、TC-DC-KVD-001 的逐轮数学参考已实质增加，但生产KvD/Franz与CTest未授权实施，最终 DEC-005、全源数值及 AC 验收仍未满足。

### 2.19.2 八方向 near 首次负总和/翻号的完整理论 trace（AF-043/051，非真实软件）

§9.5 [KvD LLD](design/kvd.md) 构造 **\(N=8\)** 的有序切向光流：前四个近轴方向的**有符号**近度为 \(-10\)，后四个方向的近度为 \(+1\)。这是用于检验源码符号分支的**非物理数学输入**，不能假称所有点均有正深度。Franz 的初始平移、旋转分别为

\[
\boldsymbol t_{\mathrm{Franz}}=\frac{90}{101}\boldsymbol e_x,\qquad
\boldsymbol\omega_{\mathrm{Franz}}=\boldsymbol 0.
\]

第一次计算 \(\sum_i\mu_i^{\mathrm{raw}}<0\)，源码必须在平移与旋转更新前**同时翻转 near 和旧平移方向**。令 \(\epsilon=10^{-13}\)，\(\alpha=(1+100\epsilon)^{-1}\)，\(\beta=(1+\epsilon)^{-1}\)；首轮翻号后的近度均值及两种平移候选是

\[
\overline\mu=\frac{5\alpha-\beta}{2},\qquad
\widetilde{\boldsymbol t}_O=-\frac{\alpha+\beta+99\alpha^2}{2}\boldsymbol e_x,\qquad
\widetilde{\boldsymbol t}_M=-\frac{99\alpha-9}{20}\boldsymbol e_x.
\]

两种候选归一后均为 \(-\boldsymbol e_x\)，旋转仍为 \(\boldsymbol 0\)，最终平移为

\[
\boldsymbol t_{\mathrm{out}}=-\frac{5\alpha-\beta}{2}\boldsymbol e_x.
\]

**文本损坏整改**：此前这一段将 LaTeX 的反斜杠错误地写成了 ASCII 控制字符（如退格字符后接 oldsymbol、eta、armu），导致有关变量不能准确阅读、复制和自动核对。本次按照 KvD LLD §9.5 的**实际公式**恢复，未改变原始数值、计算轮次、参考 JSON、测试编号或运行状态；这不是新增数值测试。

独立 [Wolfram完整六轮脚本](../verification/kvd/verify_near_sign_flip_exact_trace.wl) 与 [全部12轮参考](../verification/kvd/near_sign_flip_exact_trace.v1.json) 确认 Original/Modified 均按 `[true,false,false,false,false,false]` 触发 near 翻号，第1轮 `RMS≈7.071421356346968e-12`，第二至第六轮 `delta=0`，第六轮后停止，11/11数学自检通过。**这测试的是混合正负构造近度下的控制流，并非正深度物理场景或生产软件通过**；还缺 200轮/异常/一般输入、C++/CTest和最终DEC-005。仍归原 `TC-F-FILTER-001`、`TC-F-KVD-001/002`、`TC-DC-KVD-001`，无新 TC。

**停止布尔上下界补充（KvD LLD §9.6）**：源码的 `((delta>grenze)&&(x<200)&&(delta>1e-7))||(x<6)` 在正常完成、x从0且每轮加1的前提下严格满足 **`6<=x_return<=200`**。人工 `RMS_k=1`（奇数k）/`RMS_k=2`（偶数k），`RMS_0=0`，可从逻辑上让前支在 `x<200` 时恒成立、于第200轮停止；**不代表这个RMS序列能由某个真实KvD输入产生**，也不是200轮软件/数值测试已经执行。现有六轮36次数学trace验证了最小迭代分支，200轮可达性/数值路径及 C++/CTest 仍是 AF-043/051 的真实卡点。

## 3. TC → 实体/CTest → AC 规划矩阵

下列表格共66项，与当前 SRS **v0.15 草案**的66条REQ/AC逐项按编号对应；具体AC→主要设计责任章节及冻结前Git blob身份见 [REQUIREMENTS.md §9.7](REQUIREMENTS.md)，其内容只是**主要责任导航与固定审查快照，不是已批准或执行通过的AC**；MAT I/O 的拟议数据结构、独立负例及测试文件映射详见 [mat_io.md §2、§5、§8–9](design/mat_io.md)。**独立 matio 探针已编写并执行**，但除球谐低阶叶子外，绝大多数拟议测试源文件、测试函数和 CTest 名称尚未创建；探针不是 CTest 注册项。人工审查项与自动测试区分。原始 45 个 MATLAB 源文件映射请参见 [REQUIREMENTS.md §9.2](REQUIREMENTS.md)。

**L2/L3/L4 来源及测试文件唯一索引（2026-11-11核对已修正过时章节）**：`contracts.md §3/§3.0`、`experiments.md §8.1`、`harmonics.md §6`、`sampling_noise.md §8`、`mat_io.md §5/§8` 仅补充对应模块的候选测试细节。本表仍是 66 项 TC 的单一归档，不产生新编号；**除已有球谐低阶叶子外**，其余多数 test 文件和 CTest 名称仍是候选；完整来源/批准尚未完成，H-10/H-11 实际执行权限保持暂缓。

| TC ID | REQ | 候选测试文件或审核文档 | CTest/审核 | AC | 状态 |
|---|---|---|---|---|---|
| TC-F-CLI-001 | REQ-F-CLI-001 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-F-CLI-001 | 未编写/未运行 |
| TC-F-MAT-001 | REQ-F-MAT-001 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-F-MAT-001 | 独立 matio 探针 582/582 已实测；业务 TC/CTest 及 P10.4 复核未完成 |
| TC-F-MAT-002 | REQ-F-MAT-002 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-F-MAT-002 | 独立 matio 探针 582/582 已实测；业务 TC/CTest 及 P10.4 复核未完成 |
| TC-F-MAT-003 | REQ-F-MAT-003 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-F-MAT-003 | 未编写/未运行 |
| TC-F-MAT-004 | REQ-F-MAT-004 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-F-MAT-004 | 未编写/未运行 |
| TC-F-GEOM-001 | REQ-F-GEOM-001 | tests/unit/test_sampling.cpp | egomotion.geometry.* | AC-F-GEOM-001 | 未编写/未运行 |
| TC-F-GEOM-002 | REQ-F-GEOM-002 | tests/unit/test_sampling.cpp | egomotion.geometry.* | AC-F-GEOM-002 | 未编写/未运行 |
| TC-F-GEOM-003 | REQ-F-GEOM-003 | tests/unit/test_sampling.cpp | egomotion.geometry.* | AC-F-GEOM-003 | 未编写/未运行 |
| TC-F-FLOW-001 | REQ-F-FLOW-001 | tests/unit/test_flow_simulation.cpp | egomotion.flow.* | AC-F-FLOW-001 | 未编写/未运行 |
| TC-F-FLOW-002 | REQ-F-FLOW-002 | tests/unit/test_flow_simulation.cpp | egomotion.flow.* | AC-F-FLOW-002 | 未编写/未运行 |
| TC-F-FLOW-003 | REQ-F-FLOW-003 | tests/unit/test_flow_simulation.cpp | egomotion.flow.* | AC-F-FLOW-003 | 未编写/未运行 |
| TC-F-FLOW-004 | REQ-F-FLOW-004 | tests/unit/test_flow_simulation.cpp | egomotion.flow.* | AC-F-FLOW-004 | 未编写/未运行 |
| TC-F-FILTER-001 | REQ-F-FILTER-001 | tests/unit/test_estimators.cpp | egomotion.estimators.* | AC-F-FILTER-001 | 未编写/未运行 |
| TC-F-KVD-001 | REQ-F-KVD-001 | tests/unit/test_estimators.cpp | egomotion.estimators.* | AC-F-KVD-001 | 未编写/未运行 |
| TC-F-KVD-002 | REQ-F-KVD-002 | tests/unit/test_estimators.cpp | egomotion.estimators.* | AC-F-KVD-002 | 未编写/未运行 |
| TC-F-ERROR-001 | REQ-F-ERROR-001 | tests/unit/test_estimators.cpp | egomotion.estimators.* | AC-F-ERROR-001 | 未编写/未运行 |
| TC-F-BIAS-001 | REQ-F-BIAS-001 | tests/integration/test_bias.cpp | egomotion.bias.* | AC-F-BIAS-001 | 未编写/未运行 |
| TC-F-BIAS-002 | REQ-F-BIAS-002 | tests/integration/test_bias.cpp | egomotion.bias.* | AC-F-BIAS-002 | 未编写/未运行 |
| TC-F-BIAS-003 | REQ-F-BIAS-003 | tests/integration/test_bias.cpp | egomotion.bias.* | AC-F-BIAS-003 | 未编写/未运行 |
| TC-F-TRAJ-001 | REQ-F-TRAJ-001 | tests/unit/test_trajectory.cpp | egomotion.trajectory.* | AC-F-TRAJ-001 | 未编写/未运行 |
| TC-F-SH-001 | REQ-F-SH-001 | tests/unit/test_low_order_harmonics.cpp；tests/unit/test_low_order_archived_golden.cpp；tests/unit/test_low_order_source_projection.cpp（SH-only） | low_order_harmonics（5e522198实测）；low_order_harmonics_archived_golden、low_order_harmonics_source_projection（5e522198实测） | AC-F-SH-001 | 5e522198的SH低阶3项CTest均通过：原回归53,532、AN-G06/07/08归档176、源二阶逐系数投影76次检查；全业务HarmonicFitter、DEC-005仍缺 |
| TC-F-SH-002 | REQ-F-SH-002 | tests/unit/test_low_order_harmonics.cpp；tests/unit/test_low_order_archived_golden.cpp（SH叶子） | low_order_harmonics、low_order_harmonics_archived_golden（5e522198均实测通过） | AC-F-SH-002 | 5e522198三种低阶旋转路线、归档AN-G08的R3全9/R5全25与原3×5系数已获真实CTest证据；公共rotator/ABI/全业务集成/正式DEC-005仍未验收 |
| TC-F-ADAPT-001 | REQ-F-ADAPT-001 | tests/unit/test_adaptive_mfa.cpp | egomotion.adaptive.* | AC-F-ADAPT-001 | 未编写/未运行 |
| TC-F-ADAPT-002 | REQ-F-ADAPT-002 | tests/unit/test_adaptive_mfa.cpp | egomotion.adaptive.* | AC-F-ADAPT-002 | 未编写/未运行 |
| TC-F-ADAPT-003 | REQ-F-ADAPT-003 | tests/unit/test_adaptive_mfa.cpp | egomotion.adaptive.* | AC-F-ADAPT-003 | 未编写/未运行 |
| TC-F-EXP-001 | REQ-F-EXP-001 | tests/integration/test_adaptive_experiments.cpp | egomotion.experiments.* | AC-F-EXP-001 | 未编写/未运行 |
| TC-F-EXP-002 | REQ-F-EXP-002 | tests/integration/test_adaptive_experiments.cpp | egomotion.experiments.* | AC-F-EXP-002 | 未编写/未运行 |
| TC-F-EXP-003 | REQ-F-EXP-003 | tests/integration/test_adaptive_experiments.cpp | egomotion.experiments.* | AC-F-EXP-003 | 未编写/未运行 |
| TC-F-AN-001 | REQ-F-AN-001 | tests/golden/test_analysis.cpp | egomotion.analysis.* | AC-F-AN-001 | 未编写/未运行 |
| TC-F-AN-002 | REQ-F-AN-002 | tests/golden/test_analysis.cpp | egomotion.analysis.* | AC-F-AN-002 | 未编写/未运行 |
| TC-D-SOURCE-001 | REQ-D-SOURCE-001 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-D-SOURCE-001 | 未编写/未运行 |
| TC-D-SOURCE-002 | REQ-D-SOURCE-002 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-D-SOURCE-002 | 未编写/未运行 |
| TC-D-MAT-001 | REQ-D-MAT-001 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-D-MAT-001 | 独立 matio 探针 582/582 已实测；业务 TC/CTest 及 P10.4 复核未完成 |
| TC-D-MAT-002 | REQ-D-MAT-002 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-D-MAT-002 | 未编写/未运行 |
| TC-D-MAT-003 | REQ-D-MAT-003 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-D-MAT-003 | 未编写/未运行 |
| TC-DC-DOMAIN-001 | REQ-DC-DOMAIN-001 | 现有：[OBJECT_MODEL.md](OBJECT_MODEL.md)、[REQUIREMENTS.md §9.2](REQUIREMENTS.md) 的 45 源文件追踪；现有：[SOURCE_INVENTORY.md](SOURCE_INVENTORY.md)（45 个源文件的静态 Git 身份；非测试通过） | 人工设计/来源评审；部分接口需合同测试 | AC-DC-DOMAIN-001 | 未编写/未运行 |
| TC-DC-IO-001 | REQ-DC-IO-001 | **候选** tests/contracts/test_mat_raii.cpp；现有：[OBJECT_MODEL.md](OBJECT_MODEL.md)、[MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) 的归档证据；现有：[SOURCE_INVENTORY.md](SOURCE_INVENTORY.md)（45 个源文件的静态 Git 身份；非测试通过） | **候选** egomotion.mat.raii.* ＋人工设计/来源评审 | AC-DC-IO-001 | 未编写/未运行；MatDenseArray 自拥有/移动、ErrorDetail 传播、句柄释放待验证 |
| TC-DC-RNG-001 | REQ-DC-RNG-001 | **候选** tests/unit/test_sampling.cpp ＋ tests/unit/test_flow_simulation.cpp；现有：[OBJECT_MODEL.md](OBJECT_MODEL.md)、[REQUIREMENTS.md §9.2](REQUIREMENTS.md)；现有：[SOURCE_INVENTORY.md](SOURCE_INVENTORY.md)（45 个源文件的静态 Git 身份；非测试通过） | **候选** egomotion.geometry.* / egomotion.flow.* ＋人工设计评审（依 [sampling_noise.md §8](design/sampling_noise.md)） | AC-DC-RNG-001 | 未编写/未运行；固定 RNG 值、调用数/耗尽和上下文待验证 |
| TC-DC-NOISE-001 | REQ-DC-NOISE-001 | **候选** tests/unit/test_flow_simulation.cpp；现有：[OBJECT_MODEL.md](OBJECT_MODEL.md)、[REQUIREMENTS.md §9.2](REQUIREMENTS.md)；现有：[SOURCE_INVENTORY.md](SOURCE_INVENTORY.md)（45 个源文件的静态 Git 身份；非测试通过） | **候选** egomotion.flow.* ＋人工设计评审（依 [sampling_noise.md §8](design/sampling_noise.md)） | AC-DC-NOISE-001 | 未编写/未运行；两噪声模型、错误上下文和样本数待验证 |
| TC-DC-KVD-001 | REQ-DC-KVD-001 | tests/unit/test_estimators.cpp | egomotion.estimators.* | AC-DC-KVD-001 | 未编写/未运行 |
| TC-DC-ADAPT-001 | REQ-DC-ADAPT-001 | tests/unit/test_adaptive_mfa.cpp | egomotion.adaptive.* | AC-DC-ADAPT-001 | 未编写/未运行 |
| TC-DC-EXPERIMENT-001 | REQ-DC-EXPERIMENT-001 | tests/integration/test_adaptive_experiments.cpp | egomotion.experiments.* | AC-DC-EXPERIMENT-001 | 未编写/未运行 |
| TC-DC-ANALYSIS-001 | REQ-DC-ANALYSIS-001 | tests/golden/test_analysis.cpp | egomotion.analysis.* | AC-DC-ANALYSIS-001 | 未编写/未运行 |
| TC-NF-PLATFORM-001 | REQ-NF-PLATFORM-001 | tests/integration/test_deployment.cpp | egomotion.deployment.* | AC-NF-PLATFORM-001 | 未编写/未运行 |
| TC-NF-DEPS-001 | REQ-NF-DEPS-001 | tests/integration/test_deployment.cpp | egomotion.deployment.* | AC-NF-DEPS-001 | 未编写/未运行 |
| TC-NF-SAFETY-001 | REQ-NF-SAFETY-001 | tests/contracts/test_output_safety.cpp | egomotion.safety.* | AC-NF-SAFETY-001 | 未编写/未运行 |
| TC-NF-NUM-001 | REQ-NF-NUM-001 | tests/golden/test_numerical_equivalence.cpp | egomotion.numerics.* | AC-NF-NUM-001 | 未编写/未运行 |
| TC-NF-NUM-002 | REQ-NF-NUM-002 | tests/golden/test_numerical_equivalence.cpp | egomotion.numerics.* | AC-NF-NUM-002 | 未编写/未运行 |
| TC-PROC-TRACE-001 | REQ-PROC-TRACE-001 | docs/REQUIREMENTS.md | 人工追踪审查 | AC-PROC-TRACE-001 | 未编写/未运行 |
| TC-NF-RNG-001 | REQ-NF-RNG-001 | tests/golden/test_numerical_equivalence.cpp | egomotion.numerics.* | AC-NF-RNG-001 | 未编写/未运行 |
| TC-NF-LOG-001 | REQ-NF-LOG-001 | tests/contracts/test_runlog.cpp | egomotion.logging.* | AC-NF-LOG-001 | 未编写/未运行 |
| TC-NF-OOP-001 | REQ-NF-OOP-001 | tests/contracts/test_state_isolation.cpp | egomotion.isolation.* | AC-NF-OOP-001 | 未编写/未运行 |
| TC-NF-HEADLESS-001 | REQ-NF-HEADLESS-001 | tests/integration/test_headless.cpp | egomotion.headless.* | AC-NF-HEADLESS-001 | 未编写/未运行 |
| TC-NF-RECOVERY-001 | REQ-NF-RECOVERY-001 | tests/contracts/test_output_safety.cpp | egomotion.safety.* | AC-NF-RECOVERY-001 | 未编写/未运行 |
| TC-NF-PORTABLE-001 | REQ-NF-PORTABLE-001 | tests/integration/test_deployment.cpp | egomotion.deployment.* | AC-NF-PORTABLE-001 | 未编写/未运行 |
| TC-NF-RUNTIME-001 | REQ-NF-RUNTIME-001 | tests/benchmarks/benchmark_original_runs.cpp | 非 CTest 基准 | AC-NF-RUNTIME-001 | 未编写/未运行 |
| TC-NF-SESSION-001 | REQ-NF-SESSION-001 | tests/contracts/test_state_isolation.cpp | egomotion.isolation.* | AC-NF-SESSION-001 | 未编写/未运行 |
| TC-NF-COPYRIGHT-001 | REQ-NF-COPYRIGHT-001 | 现有：[OBJECT_MODEL.md](OBJECT_MODEL.md)、[所有者技术决定](decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md)、[LICENSE](../LICENSE)；现有：[SOURCE_INVENTORY.md](SOURCE_INVENTORY.md)（45 个源文件的静态 Git 身份；非测试通过） | 人工设计/来源评审；部分接口需合同测试 | AC-NF-COPYRIGHT-001 | 未编写/未运行 |
| TC-F-CLI-002 | REQ-F-CLI-002 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-F-CLI-002 | 未编写/未运行 |
| TC-F-CLI-003 | REQ-F-CLI-003 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-F-CLI-003 | 未编写/未运行 |
| TC-F-CLI-004 | REQ-F-CLI-004 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-F-CLI-004 | 未编写/未运行 |
| TC-F-CLI-005 | REQ-F-CLI-005 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-F-CLI-005 | 未编写/未运行 |
| TC-I-CLI-001 | REQ-I-CLI-001 | tests/integration/test_cli.cpp | egomotion.cli.* | AC-I-CLI-001 | 未编写/未运行；必须验证已删除的输出根选项被拒绝（exit 2）、固定 `output/<RunId>/`、独立 OS RunId、JSONL、稳定 ErrorStage |
| TC-I-MAT-001 | REQ-I-MAT-001 | tests/golden/test_mat_archive.cpp | egomotion.mat.* | AC-I-MAT-001 | 未编写/未运行 |
| TC-I-RESULT-001 | REQ-I-RESULT-001 | tests/integration/test_adaptive_experiments.cpp | egomotion.experiments.* | AC-I-RESULT-001 | 未编写/未运行 |
| TC-I-ANALYSIS-001 | REQ-I-ANALYSIS-001 | tests/golden/test_analysis.cpp | egomotion.analysis.* | AC-I-ANALYSIS-001 | 未编写/未运行 |

**来源资料状态（AF-063）**：当前可使用 [SRS 的 45 个 .m 路径/REQ/TC 映射](REQUIREMENTS.md)、[对象模型](OBJECT_MODEL.md)、[MAT 数据字典](MAT_DATA_SCHEMA.md) 和已选技术决定；P0.6 的 [SOURCE_INVENTORY](SOURCE_INVENTORY.md) / [MIGRATION_MATRIX](MIGRATION_MATRIX.md) **已有文件级静态草案并与 45 个原源路径对应**；但这不构成逐函数审计、人工来源批准、软件测试或 AC 通过。上述五行保留原 TC/REQ/AC 编号，真实测试文件、来源验收与 CTest 仍未执行。

## 4. 非功能指标和验收记录格式

| 指标 | 必须记录的现状 | 目标冻结状态 |
|---|---|---|
| Debian/Ubuntu 构建 | 编译器版本、CMake、依赖、构建命令及结果 | 支持版本范围待实测 |
| 性能与内存 | 完整原始实验的 wall/CPU 时间、峰值 RSS、输入大小及构建耗时 | 阈值、加速比和覆盖率尚未批准，不得凭空填写 |
| 随机与会话隔离 | 运行种子、FixedSequenceRandomSource、交替双 Session 状态 | 单个 Session 的多线程安全不作承诺 |
| 安全与失败恢复 | origin 文件哈希、输出目录检查、RunId 冲突测试、日志/退出码 | 不擅自引入自动重试或新 SLA |

### 4.1 单次测试证据模板

| 字段 | 必需内容 |
|---|---|
| 标识 | REQ、TC、AC、对应 MATLAB 文件路径及源 SHA |
| 环境 | Debian/Ubuntu 版本、编译器、CMake、Eigen/matio/zlib 版本 |
| 执行 | 完整命令/参数、RunId、输入 SHA、随机源/种子、输出目录 |
| 数值 | 预期/实际、绝对/相对误差、容差来源、有效/无效/无定义样本数 |
| 状态 | 未编写、未运行、通过、失败、因依赖阻塞、条件不适用，及证据位置 |

### 4.2 测试优先级与最低资源配置（计划，未测）

| 级别 | 测试集合 | 触发时机与资源 | 失败处理 |
|---|---|---|---|
| T0（必须、快速） | FlowField 形状、叉积/球面采样 i=0/1、固定随机序列、最小 Franz/KvD/Adaptive 单步 | 每个相关模块提交后；Debian/Ubuntu 一台开发环境即可；具体 CPU/内存/耗时未知 | 阻止将相应模块标为验证通过，保存最小输入和日志 |
| T1（必须、数据合同） | matio/zlib 元数据、只读保护、结果新目录写回读、异常输入/路径冲突 | P1 MAT Reader/Writer 可用后；只读原始 MAT 与独立写目录 | 保留所有失败文件和原因，绝不修改 origin |
| T2（必须、全量） | **582 个 MAT、582 变量的全量读取/字典**及 45 个 .m 的功能归属 | 独立 matio 探针在 Ubuntu 已实际执行、全部 READ_OK；P1 业务 Reader 与 P10.4 复核尚待实施 | 582/582 无读取失败是**已获得的子项证据**，不可扩展宣称所有类型负例、原 MATLAB Golden 或项目整体验收通过 |
| T3（必须、L3 独立数值参考） | KvD 逐轮、SH 旋转块、Adaptive M1–M4 和 MSE | **独立 L3 固定输入逐节点参考**与批准的 DEC-005 容差；无需 MATLAB | 仅独立参考缺失或未经批准容差时阻塞；不能用被测 C++ 自算结果替代 |
| T4（必须、实验复现） | 两种 Bias、三种 Adaptive、四类分析及原论文曲线 | P5/P8/P9/P10 入口实际可执行后；性能资源按首轮实测安排 | 保留统计分母、失败/无定义样本、运行命令和真实耗时 |
| T5（应该、附加） | 可选绘图、额外编译器/CPU 组合、扩大性能测试 | 在必须需求有证据后，按项目所有者授权分期 | 明示未执行，不阻塞未要求的功能 |

**资源责任分工**：项目所有者批准需要运行的全量试验和数据使用范围；开发负责人提供测试入口与提交 SHA；数值验证负责人准备有来源的 Golden；维护/集成负责人核对目标 Debian/Ubuntu 工具链和 CTest 证据。当前没有经确认的设备型号、执行预算或时限，因此不编造 CPU 核心数、RAM 容量、分钟数、覆盖率及测试费用。

## 5. 原始 MAT、L2/L3/L4 参考数据管理与来源策略

### 5.1 测试数据来源清单

| 原数据分组 | 当前文件数量 | 角色 | 全量核验 | 开发期代表性样本 |
|---|---:|---|---|---|
| bias_test/produced_data/occluded_regions | 280（40×7） | 非球形 Bias 统计基准 | **matio 280/280 READ_OK**；xwerte **40 个 10×1 MAT 数组**，ywerte **240 个 10×3 MAT 数组**，class/type 均 double；独立复核待做 | 编号 1、中间编号、40 |
| bias_test/produced_data/proportional_error | 280（40×7） | 球形比例噪声 Bias 统计基准 | **matio 280/280 READ_OK**；xwerte **40 个 10×1 MAT 数组**，ywerte **240 个 10×3 MAT 数组**，double/double；独立复核待做 | 编号 1、中间编号、40 |
| adaptive_matched_filter_test/produced_data/initialization | 4 | 初始化已保存结果 | **matio 4/4 READ_OK**；全部 double[35×1] | 4 个文件 |
| adaptive_matched_filter_test/produced_data/without_errors | 5 | 无噪声对比 | **matio 5/5 READ_OK**；全部 double[600×1] | 5 个文件 |
| adaptive_matched_filter_test/produced_data/with_errors | 7 | 带噪声对比及历史额外字段 | **matio 7/7 READ_OK**；全部 double[600×1]，其中 `alleadaprotfehler` 与 `alleadaptransfehler` 为历史专属 **`HistoricalOnly`**（只读；新 Runner 不保存） | 7 个文件 |
| adaptive_matched_filter_test/produced_data/many_parameter | 6 | 多更新率与历史额外字段 | **matio 6/6 READ_OK**；四个 double[600×4]、两个 double[600×1] | 6 个文件 |
| **合计** | **582** | 原作者静态参考数据 | **matio 已读 582/582（582 变量、0 失败）；独立复核未完成** | 实际全量结果见 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) |

**数据保护**：origin/** 作为只读输入；测试只产生新目录或测试专属副本；不得向原目录写入、覆盖、删除或以 git add -f 强制提交忽略内容。样本选择和目录哈希应随源仓库提交 SHA 固定。

### 5.2 L2/L3/L4 参考数据的形成、保管与追溯

1. **现有 Golden**：作者归档中真正存在的 MAT 结果，可作为变量合同与统计结果参考，但没有每轮中间量时不能冒称提供了逐步算法真值。
2. **待生成的数值 Golden**：Franz 模板项、Original/Modified KvD 的 near/t/ω、Adaptive 的 M1…M4/b/z/error_mse、新系数、球谐块旋转。**必须以只读源公式、独立解析或独立非被测实现生成/复核 L3 参考**；不需要执行 MATLAB/Octave 或采集 L1；不能以被测 C++ 程序自己的结果作为真值。
3. **身份字段**：源 .m Git blob SHA、源树 SHA、工具及版本、原公式的固定输入和独立注入 RNG 序列、变量名/shape/dtype、参考数值文件 SHA-256、许可证/可存储位置。
4. **存放位置**：小型、可再分发的定值 Golden 在满足现有上游告知后可放 tests/golden/fixtures；受限或大型资料放在维护者批准的独立持久位置，仓库只保存路径映射和哈希，不擅自复制/再许可。
5. **版本更新**：变更输入、脚本 SHA、近度定义、索引或容差时，应生成新的 Golden 版本并保存历史，不静默覆盖旧结果。
6. **样本统计**：每次用例记录文件总数、成功解码、格式不支持、损坏、缺失、统计有效/失败/未定义，不能只报告成功部分。

### 5.3 数据合同变更与试验数据基准

MAT 全量字典和来源证据**已经生成**：见 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) 和 [design/mat_io.md](design/mat_io.md)。在 Ubuntu 24.04、matio 1.5.26 环境下，独立探针对原始归档 582/582 MAT、582/582 变量执行读取，均为 `READ_OK`，失败 0，这是**已取得的特定数据读取事实**。但它不代表生产 Reader/Writer、MAT 字节与 MATLAB 逐值 Golden、异常负例、数据来源独立核查或 P10.4 已通过；不得由此扩大推断到其他 MAT 格式和未来输入。

### 5.4 L2/L3/L4 独立证据及 TC 剩余缺项（无需 MATLAB）

以 [长期无需 MATLAB 运行政策](golden/NO_MATLAB_EXECUTION_POLICY.md) 为准。L1 原 MATLAB 运行证据**永久退出必需证据与阻塞条件**；L2 为原作者 582 MAT 最终输出，L3 为独立解析/独立重实现逐节点/逐轮参考，L4 为统计对照。AN-G01～12 是未实际测试的 L3 候选，**不等于现有全部参考就绪**。

| 重点 TC | 采用的可行参考 | 剩余缺项（不包括 L1） |
|---|---|---|
| TC-F-FILTER-001、TC-F-KVD-001/002 | 已执行Wolfram：轴向/斜向两组六方向Original/Modified共24轮数学trace；再增加混合有符号near的N=8两方法共12轮，首轮翻号、六轮停止、MSE/RMS/delta/grenze及原始 a/b/c/tRaw均已记录（共36轮） | 这些是独立数学参考，**不是**生产KvD/Franz真实C++/CTest；仍缺严格正深度下的一般退化、异常/200轮及DEC-005/正式全AC批准 |
| TC-F-ADAPT-001/002/003、TC-DC-ADAPT-001 | L3 独立 M1～M4/b/z/MSE/系数、连续帧状态；L2 最终误差 | 一般场景、退化输入、Session 状态和 C++/CTest |
| TC-F-GEOM-002/003、TC-F-FLOW-002/003、TC-DC-RNG-001、TC-NF-RNG-001 | i=0/1/2 全714行有序参考已保存，独立Wolfram按原20面/子面顺序对2142坐标完成逐元素验证，最大误差1.11e-16；uniform/normal、L4仍为候选 | 尚无生产采样/噪声C++或CTest；RNG耗尽/非有限、正式DEC-005/最终批准仍缺，不能标TC PASS |
| TC-F-SH-001/002、TC-F-TRAJ-001 | L3 R0/R3/R5 全元素、系数与几何旋转、轨迹解析值 | 多轴/复合/非平凡角独立参考、版权/适配、CTest |
| TC-F-EXP-001/002/003 | L3 固定两帧/8 次聚合/刷新规则；L2 历史 MAT、L4 统计 | 一般输入参考、Runner、CTest、L4 |
| TC-F-AN-001、TC-I-ANALYSIS-001 | 从 L2 只读 MAT 独立重算为 L3 begin/end/zone/平滑/曲线 | 原始值逐元素导出、独立分析结果、CTest |

无论哪项仍缺 C++ 实施、实际测试、参考值或 DEC-005 批准时均不可标记 PASS。宽松 `atol=0.01,rtol=0.1` 已确认用于**首轮探索筛查**（正式验收另定）；对形状、顺序、RNG 消耗、状态和几何不变量仍严格检查。**不再因无法运行 MATLAB 而阻塞任何 TC/AC。**

## 6. Debian/Ubuntu 环境与配置矩阵

| 环境组 | 发行版 / CPU 架构 | 编译器 | Eigen / matio / zlib | 当前状态 |
|---|---|---|---|---|
| LINUX-01 | **H-04 用户提供 Ubuntu 24.04.4 VirtualBox / x86_64 快照**（内核标识 7.0.0-38-generic） | **历史独立 matio 探针记录 GCC 13**；用户本次 JSON 没提供编译器版本新实测 | matio 探针历史记录 **1.5.26**；Sophus、Eigen、完整 CMake/CTest 未在该审批快照中测得 | **环境已选**；用户 `free` 总内存 8127380 KiB、available 4908696 KiB、Swap 4194300 KiB，根分区当时可用 65347856 KiB；**非项目通过预算**，业务构建/CTest 未执行 |
| LINUX-02 | Debian stable，具体版本待确认 / x86_64 | GCC 或 Clang，具体版本待记录 | 依赖版本待记录 | **未构建** |
| LINUX-03（可选） | Debian/Ubuntu 的另一受支持编译器或 CPU 架构 | 由维护者决定 | 兼容性待核 | 未安排 |

- 不将 Windows 或交叉编译视为本项目要求；不凭空规定编译器/Eigen/matio 的最低版本。
- 每组将来仍须记录 uname、/etc/os-release、编译器版本、CMake 版本、Eigen/Sophus/matio/zlib 的实际版本、CMake 选项、build SHA、CTest 名称与结果；**H-04 用户提供的某时刻容量并不是可自动分配的 CPU/RAM 预算或通过阈值**，未执行不能在平台矩阵打勾。
- 依赖安装、构建及数值测试必须遵守 [AGENTS.md](../AGENTS.md) 的远程检查点和 Git 安全规则。

## 7. 非功能测试方法与风险

| 测试领域 | 实施方法（计划） | 判定与不确定性 |
|---|---|---|
| 性能 | 选定原论文全量配置，固定 CPU/线程、seed、输入规模，记录墙钟（单调时钟）、CPU user+system、样本数与负载，必要时多次重复并报告分布 | 先形成基线，**没有经批准的时间/加速比阈值就不宣称性能通过** |
| 内存 | 使用 Debian/Ubuntu 可获得的峰值 RSS 记录方式，分别测 MAT 解码、最大采样、600 点实验与结果输出 | 先报告 peak RSS、输入数据量和分配行为，阈值 DEC-006 待定 |
| 构建时间 | 同一源 SHA、工具链/构建类型下记录源外配置、编译、链接时间；区分冷构建和增量构建 | 尚未设构建耗时门限 |
| 恢复/安全 | 对**测试副本**制造无效文件、权限不足、输出 RunId 冲突和中断情况，检查输出不覆盖、错误定位、历史日志保留 | 不允许破坏 origin 原始文件；正式自动恢复 SLA 未定义 |
| 会话隔离 | 交替运行两份不同 seed/刷新频率的 RunContext，并分别独立复算 | 结果与各独立运行一致；**不承诺单个 Session 多线程安全** |
| 可重复数值 | 独立固定输入与 Golden 的每步误差、统计有效率、无定义数目 | DEC-005 逐模块容差未冻结，不能判定 M 级通过 |
| 头less | 不安装/禁用可选绘图后端，分别运行 inspect、实验、analyze | 数值路径不应因绘图库缺失而不可用 |

### 7.1 测试策划风险

| 风险 | 缓解措施 / 状态 |
|---|---|
| 当前数据类型已明确；未来扩展可能带来其他 MAT class/shape | 当前归档 582/582 为 MAT v5 + zlib + double；仅需针对未来未知类型提供明确拒绝和负例，业务 Reader 未验收 |
| MATLAB 随机数与 C++ 难以逐样本一致 | 使用固定输入 Golden 验证算法，单独验证统计分布与运行种子 |
| 当前生成脚本和历史 MAT 字段不一致 | “当前源码/历史归档/分析依赖”三方差异表，不伪造字段 |
| 原 MATLAB L1 取证不可行 | 无需取证 MATLAB 运行；按只读静态源生成 L3 独立逐节点参考并用 L2/L4 做输出/统计核验，披露原 MATLAB 未可证的运行行为；不得由被测 C++ 自验替代 |
| 运行资源消耗或测试超时 | 保存命令及失败证据，先进行小规模单元回归；完整论文实验要单独安排，不因小规模通过而虚称全量 |
| 未经批准的绝对/相对误差门限 | 保留候选及测量结果，项目所有者确认后冻结；历史失败不删 |

## 8. 测试阶段进入/退出准则

| 阶段 | 进入准则 | 退出准则 |
|---|---|---|
| 测试策划基线 | SRS 编号与来源、风险和 P 任务映射已核对 | TC/AC、数据管理、环境矩阵、未决 DEC 和审查人明确；**不要求软件已运行** |
| 单元测试 | P0.8 所需接口/数值合同评审、相应代码已提交推送 | 对应 TC 实际执行并保存正常/异常证据；未通过保持开放 |
| MAT/集成测试 | P1 有最小 MAT 读写能力、原始输入受保护 | 相关源 MAT 全量读取/实际字段校验，输出安全/回读测试真实通过 |
| Golden 数值测试 | 固定输入的**独立 L3 参考**与批准容差可用（不要求 MATLAB/Octave） | 逐轮变量和公式通过判据，失败/退化样本单列 |
| 论文复现 | 各入口和输出合同真实可执行、统计流程可用 | 每类实验的有效/失败数量、误差、曲线数据、差异说明和完整证据 |
| 最终交付 | M 级要求及批准的 S 级范围有可追踪状态 | 项目所有者批准例外与未决项，PR/文档/测试/提交链路完整 |

**状态区分**：独立 matio 探针在 Ubuntu 上确已编译运行且 582/582 读取成功；但**没有**由本项目注册/运行 CTest，也未完成正式 Reader/Writer、负例、逐元素 Golden 或论文复现。其余阶段的进入/退出门槛不得因这一项成功而视为满足。

### 8.1 文档追踪静态一致性检查（不等同于执行测试）

**A. 规范要求：待人类授权的 Bash 现场执行（本项未执行）。** 需要完整的 Debian/Ubuntu 仓库检出环境、获准运行和可保存的真实日志。在 REQ、P、TC、AC 或 MATLAB 源文件映射变化后，应使用下列命令执行现场审查：

~~~bash
bash scripts/check_docs_traceability.sh
~~~

该脚本只依赖 Debian/Ubuntu 的 Bash、awk、find、sed、sort 和 git；核验下述文档级集合，不编译 C++、不注册或运行 CTest，也不判定算法数值等价。**上面的命令是待执行的操作规程，绝非本次已运行命令。** 现场运行须按 [AGENTS.md §10](../AGENTS.md) 与 [人类授权申请 H-11](approval/HUMAN_APPROVAL_REQUEST.html) 获得适用授权，并保留 stdout、stderr、实际退出码、HEAD SHA、系统/工具版本及执行人。

1. 解析 REQUIREMENTS §4 的全部 REQ ID，每个 ID 唯一且有对应的 `AC-`；解析 §9 的正向追踪，不允许不存在的需求 ID 或空 P/TC/AC。
2. 解析 TEST_PLAN §3 的 TC ID，检查 TC 集合严格等于 `{ "TC-" + REQ[4:] }`；对每个 TC 校验其引用的 REQ 和 AC 也完全匹配。人工核查类型不必注册 CTest，但必须有真实证据路径。
3. 解析 PLAN §5 的全部 P ID 与依赖，检查无重复、无未知前置、无有向环；每个 P 的 REQ、TC、AC 集合必须成对一一对应，任何孤立 REQ/P 都需要解释。
4. 解析 `origin/egomotion_estimation_dense_flow_fields_matlab_scripts/**/*.m` 与 SRS §9.2 的路径集合，确保两边完全一致；`OBJECT_MODEL.md` 的对象职责映射也应覆盖全部源文件，未移植标注不等于完成。
5. 数据文件数量、版本和哈希应来自真实 Git 树，不从对话旧表复制；只有实际完整解码后才检查 AC-D-MAT-001 的值域/类型/维度合同。
6. 将每轮静态审查报告记录为：源 Git SHA、REQ/P/TC/AC 和 .m 文件数量、唯一性、漏项、环数量、被审阅文件清单。只读检查成功不应标为 CTest、Golden 或需求已通过。

**B. 已执行与未执行证据（不得合并统计）。**

| 证据类别 | 当前状态 | 可核查的来源与未覆盖范围 |
|---|---|---|
| GitHub 连接器＋JavaScript 只读静态断言 | **已执行（此前的连接器静态检查）** | [2026-11-06 追踪审查记录](reviews/STATIC_TRACEABILITY_2026-11-06.md)：按固定 Git SHA 核对 66 REQ、66 TC、98 P、45 `.m`、26 条 HLD 边，未发现缺项/环；**没有执行 Bash** |
| 审查意见后的 GitHub 连接器只读复核 | **已执行（非 Bash、非 CTest）** | [跨文档一致性复核记录](reviews/DOC_CONSISTENCY_RECONCILIATION_2026-11-06.md)：固定提交 `5472c756b983fe002ef6e32143c96b6c6798cde9`，核对 66/66/98、45 `.m` 双向映射、26 条 HLD 静态边与源 `origin` 子树 SHA 未变；**未执行现场 Bash** |
| `bash scripts/check_docs_traceability.sh` 完整检出现场运行 | **待授权／未执行** | [H-11](approval/HUMAN_APPROVAL_REQUEST.html)；目前无现场 stdout、stderr、真实退出码和执行环境记录，不得写为 PASS |
| 正式 CMake 业务 targets 的构建、链接与 CTest | **未建立／未执行** | 静态 DAG 无环不能证明真实构建成功；审批与执行记录见 H-10 / [PROGRESS.md](PROGRESS.md) |
| 独立 matio 原始归档读取探针 | **此前已实测** | [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) 已记录 582/582 READ_OK；**不代表**本次重新运行探针、生产 Reader/Writer、异常负例或数值 Golden 通过 |

**实际现场执行的补录要求**：新建独立证据记录，附所执行命令、真实 HEAD SHA、工具链、stdin（若有）、stdout/stderr 路径、exit code、时间戳和执行人；经人类审阅后再同步 [PROGRESS.md](PROGRESS.md)，不能以本表的“应执行”替代“已执行”。

**当前文档级检查口径**：66 条 REQ、66 项 TC、98 个 P 工作项、45 个 .m 映射是版本化的预期数量；检查器同时验证 ID 唯一性、配对关系、真实路径集合、P 依赖环和 ARCHITECTURE §4.1 target DAG 的**静态环**，不只比对计数。变更基线数量时须同步评审脚本期望值；**静态无环不等于真实 CMake target 已创建或编译通过**。

**本轮实际执行状态**：用户审批 JSON 的 H-11=deferred，因此未运行上文的 Bash 现场脚本；H-10=deferred，因此未运行新的 CMake 配置/编译、CTest、MATLAB、MAT 写回读、负例或性能测量。原来的 582/582 matio 探针属于**早前独立实测**，不在本轮重跑。审批详情与阶段状态参阅 [审批落实记录](approval/DECISION_IMPLEMENTATION_NOTES.md) 和 [PROGRESS.md](PROGRESS.md)。

## 9. 变更与冻结条件

任何更改 MATLAB 对应公式、输出变量、MAT 读取边界、随机数处理、数值容差或实验输入都必须更新 REQ/TC/AC 对照并重新生成对照证据；不能通过跳过失败样本、修改阈值或者仅重新绘图来宣布结果通过。真实构建与 CTest 开始之前须遵守 AGENTS.md 的远程检查点规则。

**当前状态：66 个 TC 均未创建真实测试函数或 CTest，不能计为通过。**

**v0.8 文档修订（AF-063）**：五个 TC 行的尚未创建来源清单与现有可用文档分开列示，不修改 TC/AC/REQ、Golden 数值、授权或任何 PASS 状态。


**v0.11 历史文件身份索引补充（P0.6）**：已新增 45 个 `.m` 的 [SOURCE_INVENTORY](SOURCE_INVENTORY.md) 与 [MIGRATION_MATRIX](MIGRATION_MATRIX.md)；改变的只是五个 TC 行可引用的**已存在静态来源**，TC/REQ/AC 名称、66 个 TC 测试函数未执行事实及 H-10/H-11 仍不变。
