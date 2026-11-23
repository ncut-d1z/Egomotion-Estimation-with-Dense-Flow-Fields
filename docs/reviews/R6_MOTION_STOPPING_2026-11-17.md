# R6 误差边界与 KvD 停止分支验证

任务标签：`[HUMAN-EXECUTE/APPROVE]`。用户已明确授权生产代码、构建、CTest 及文档修改；本批仅有界固定输入测试。以下是实现与验证记录，不是 DEC-005/008 批准、MATLAB 运行等价声明或 R6 结案。

## 首轮检查点准备（历史未执行状态）

当时新增 `r06_source_motion_errors`、`r06_kvd_stopping_reference`，总登记数从 37 增至 39；登记不代表通过。运行前提交、正常推送、核实远端 SHA，仍先执行五项 MAT 测试，再执行其余测试。所有日志与失败记录存入独立忽略目录，原始文件只读。最终结果见下文。

## 误差函数的实现边界

从 Bias Runner 与 Adaptive Session 提取到内部 `src/numerics/source_motion_errors.hpp`，表达式、运算顺序、错误码及错误消息保持不变；没有新增公共 ABI、CLI 或 JSON schema。

- `transfehler_div.m`（blob `89708055b98e0b6651990c08dc3850393ca8acad`）：距离 `||t - e ||t||/||e||||`，不是角度。沿用 C++ 对零真值的显式 ZeroNorm 拒绝，不把原公式得到的零作为有效统计。
- `normrelfehler.m`（blob `c9fecc37180ab13d854a144d333a2599e601a9a8`）：分别单位化后 `abs(acos(dot)*180/pi)`，不钳制 acos 参数；零范数为 UndefinedAngle。
- 三轴、符号、尺度及 3-4-5 三角形提供独立解析参考；NaN/Inf、范数上溢/下溢、有限范数但距离上溢直接检查失败分类和输入字节不变。
- 沿用既有 R6 回归 `1e-9` allowance；这不是新设正式数值验收门限。本批最初未覆盖acos舍入越界，后续`b05a28b`已在当前Ubuntu/GCC/Eigen环境实测固定`(1,1,1)`正反向两端拒绝，误差检查增至703项，见 [补充证据](R6_INTERLEAVING_2026-11-17.md#最终复测)；不推断其它浮点环境已测。

## 独立 KvD 参考的推导与范围

来源为 `norm_koenderink_iteration.m`（blob `d48048f37b859bfb19402ca85b0f4c20e76afbfd`）与 `mue_norm_koenderink_iteration.m`（blob `9ffdac1baaecac06db20bcc6849bf4cdd165a484`），仅静态审阅。

固定视线为 `+x,-x,+y,-y,+z,-z`，每对视线有相同正 nearness `m_i`，真值平移 `T=s(1,2,3)`、旋转 `s(1/4,-1/2,3/4)`。反向成对求和使平移/旋转交叉项相消，旋转保持 Franz 初值。Franz 平移分量为 `T_i(m_j+m_k)/2`，随后单位化。独立参考直接用标量 `long double`，不调用 Eigen 数值运算、Franz、流生成器或生产停止函数：

1. `n_i=m_i sum(j!=i,t_j T_j)/(1-t_i²+epsilon)`。
2. Original 候选 `q_i=[T_i sum(j!=i,n_j m_j)+t_i n_i²]/3`；Modified 候选 `q_i=[T_i sum(j!=i,m_j)+t_i n_i]/3`，然后 `t=q/||q||`。
3. `RMS²=sum(axis i, component j!=i,(n_i t_j-m_i T_j)²)/3`，比较相邻 RMS。
4. 有界 `for 1..200`；第 6 轮起，delta 不大于动态门限或绝对门限时提前退出。参考提升原 binary64 epsilon/门限，避免无意改变参数。

生产求解器保留默认最少 6、最多 200、绝对门限 `1e-7`、精度因子 1000；只启用 trace。固定 `m_i∈{1,4,16,64,256}`、`s∈{1,1e-12}`、两算法，共 500 次求解，全部输出保留，无 RNG、筛选失败或调低门限。逐例比较独立参考的轮数与继续标志，并要求两个算法各自出现真实上限、单独动态门限退出、单独绝对门限退出及最少轮数覆盖。数值较大但有限的计算仍按真实结果记录，不能把理想参考假定成已经通过。

该解析族只核查此对称场景下的控制流，不替代既有非对称逐节点 L3，也不证明一般场景所有退出路径或完整论文实验。

## 最终复测

受测源码 **`bcee2ca02ef100b569da3ee2a54137b6875bb0af`** 已先推送并经 GitHub branch API 核实，分支 `dev-plan-P0-P1-mat-dictionary-probe`。RunId **`r6-motion-stopping-fix-20261117-sQGrT7nI`**。CMake、增量构建、CTest 均成功，构建 stdout 无编译器诊断且 stderr 为空；先 MAT **5/5**，再其余 **34/34**，合计 **39/39**。首轮失败及其后全部输入场景保留。

- 误差函数 **685 项检查、0 失败**：91 组合法解析输入及全部异常边界，并核对弦长—角度恒等式与输入字节不变。
- KvD **75080 项检查、0 失败**：506 次真实求解全部与独立参考的停止轮数和继续标志一致。Original：仅上限触发 80 例、仅动态门限 6 例、仅绝对门限 164 例；Modified：分别 43、3、206 例。另各有 650 次在低于门限时仍继续到最少轮数的 trace 检查；该数是轮数，不能当独立场景数。
- 新扰动 `b=1,4,16`：Original 在第 133/51/9 轮退出，Modified 在第 61/31/8 轮退出，均由动态门限单独触发；参考与生产一致。真实上限例 `m=(1,1,16),s=1,b=0` 的两算法都达到第 200 轮且两个误差门限仍未满足，没有修改最少/最多轮数制造覆盖。
- 原始及被记录测试/参考输入的 SHA-256 检查退出 0，原始 Git tree 保持 `70ee155131c24a50169346d0ffbaff0c01b1c547`。未安装依赖、未运行 MATLAB/Octave、未执行完整科学实验；原数据未修改。

### 环境、命令与证据

实际环境为 Ubuntu 24.04.5 LTS / Linux 7.0.0-43 x86_64、GCC 16.2.0、C++17 Release、CMake/CTest 4.3.4、Eigen 3.4.0、Sophus 1.24.6、matio 1.5.26、zlib 1.3。工作目录为仓库根；构建目录 `build/r6-20261117-KzjFHPDQ/cmake`。本次两个新测试无 RNG；其余固定 seed 与 OS RunId 随机源记录在证据目录 `randomness-source.txt`。

| 步骤 | 本地时间（2026-11-17，UTC+08:00） | 退出码 / 结果 |
|---|---|---|
| configure | 19:36:59–19:37:01 | 0 |
| build | 19:37:01–19:37:27 | 0，无诊断 |
| CTest MAT | 19:37:27–19:37:29 | 0，5/5，1.34 s |
| CTest 其余 | 19:37:29–19:37:33 | 0，34/34，4.02 s |

入口为 `bash build/r6-motion-stopping-fix-20261117-sQGrT7nI/run.sh bcee2ca02ef100b569da3ee2a54137b6875bb0af`。实际配置 `cmake -S <repo> -B <build> -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`；构建 `cmake --build <build> --parallel 4 -- -k`；CTest 使用 `-C Release --verbose --no-tests=error --output-junit <log>`，MAT 以五个既有测试名的精确正则 `-R`、并行 2，其余同正则 `-E`、并行 4。完整展开参数、cwd/开始结束/退出码在各 `.meta` 中。

日志根 `build/r6-motion-stopping-fix-20261117-sQGrT7nI/evidence/` 包含独立 stdout/stderr、JUnit、两次 `Testing` 快照、环境、输入 blob/逐文件 SHA-256 和运行后完整性结果。所有 launcher stderr 为 0 字节，子程序输出保存在 CTest verbose/JUnit 中，不宣称所有子进程均无 stderr。关键 stdout 的 SHA-256：

- `ctest-mat.stdout`：`7daf193f58ccbadb142c545cc3390856ecaa78a26f1862612c7250c7ad1f33fc`
- `ctest-remaining.stdout`：`e70a78116207c8d5e6d4607bbddd7b459991bde75d80bb853f3a5c3b492bdb93`

本批覆盖事项 `R6-STOP-001` 已以新增固定输入与复测闭合。Issue/PR 全状态检索为空，没有创建、关闭或合并。后续为 Adaptive conditioning 独立族、两个 Session/RunContext 交替隔离、其余分析参考与插图入口；正式数值资格及 R6 整体仍未完成。

## 首轮失败与修订（历史记录）

首轮源码 `54e3ac140403025aa57246431a50fd925f678ca6`，RunId `r6-motion-stopping-20261117-SZFD6nbw`，构建无诊断，MAT 5/5、其余 33/34，合计 **38/39**。误差函数 503 项通过；KvD 74470 项检查中仅一项覆盖断言失败，500 次求解的独立轮数/继续标志全部一致。Original 实际上限 80 例、单动态 3 例、单绝对 164 例；Modified 上限 43 例、单动态 0 例、单绝对 206 例；各有 650 次最少轮数覆盖。原始输入摘要保持不变。失败记录完整保留在 `build/r6-motion-stopping-20261117-SZFD6nbw/evidence/`。

该失败是固定输入族未覆盖 Modified 的动态门限，未发现生产求解器与参考不一致。已检索全部状态 Issue/PR，结果为空；记录为本地覆盖事项 `R6-STOP-001`，不伪造源码缺陷。

补充六次固定求解：`m=(1,4,16),s=1`，两算法分别在 `±x` 视线的 y 光流分量加入 `-b`，`b∈{1,4,16}`，形成非零残差地板。原 500 例及全部覆盖断言保留；仍不修改生产门限。记速度表 `V_ij=m_i T_j`（i!=j），另 `V_01+=b`，解析参考推广为 `n_i=sum(j!=i,t_j V_ij)/(1-t_i²+epsilon)`，Franz 初值 `t_i=sum(j!=i,V_ji)/2`，Original 候选 `[sum(j!=i,n_j V_ji)+t_i n_i²]/3`，Modified 为 `[sum(j!=i,V_ji)+t_i n_i]/3`，残差使用 `n_i t_j-V_ij`。成对相消与旋转保持仍成立。另补误差函数的弦长—角度恒等式及合法输入字节不变检查。等待新检查点复测，不能把修订当通过。
