# R6 Adaptive 真实生成CLI验证

任务标签 `[HUMAN-EXECUTE/APPROVE]`，使用本轮生产代码、构建/CTest及文档授权。仅在隔离CTest沙箱进行三个实际入口各一次的装配/输出回归；比较使用noise_scale=0，原35/600点、level2/4、八次聚合分母与首个数值失败终止规则均保持。不是L4统计、论文图表复现、性能测试或数值资格认证；不运行MATLAB/Octave，不修改原始资料、公式/门限或正式审批状态。

## 首轮准备时的证据缺口（历史）

以下准备记录对应首轮执行前的假设；实际结果及当前限制以本文“最终复测与原因确认”为准。

既有 `r06_adaptive_runner_*` 包括StepSolver替身、真实轨迹和固定600行聚合；`r06_real_session_interleaving` 使用真实单步与固定有限输入。它们不能替代实际 `egomotion run-adaptive-*` 的进程装配、真实求解、失败审计及输出证据。此前真实CLI只覆盖这些入口的帮助/非法参数，并未执行生成逻辑。

本轮新增 `r06_adaptive_real_cli`，登记CTest由45增至46；**尚未执行，不宣称通过**。运行前仍先提交、正常推送并核实远端SHA。第一次预期基于静态源配置及既有轨迹边界：初始化应完成33个内部帧；比较/更新率的第151 MATLAB帧真实旋转为零，既有角度合同应显式UndefinedAngle并保持149个有效前缀。如果真实求解更早失败，将保存全部实际诊断并定位，不能用替身预期反向改公式或伪造成功。

## 首轮拟定测试范围（历史）

- `run-adaptive-init`：默认35点/level2、不注入替身。核对Complete、33有效帧、四个当前save变量及35×1 shape、Manifest字段/源blob/编译SHA/实际OS生成算法种子、文件SHA/大小。以真实inspect/analyze再次读取结果，拒绝重用RunId且保留全部证据。
- `run-adaptive-comparison --noise-scale 0`：默认600点/level4/计划8次；噪声为零仍抽取原法正态样本。检查实际非成功JSONL、完整因果链与Manifest一致、正确帧号/计数、分母保持8，不发布部分MAT或伪造Complete。
- `run-adaptive-update-rates`：默认600点/level2/周期1/5/11/21；检查同类失败证据、计划1次及无额外HistoricalOnly产物。
- 每入口各执行一次生成；重复RunId只检查输出拒绝，不能再运行数值过程。每个子进程有120秒保护上限，CTest420秒；这是回归防挂起上限，不是DEC-006性能预算。相关源参数不为测试缩减。

`r06_adaptive_cli_values` 只读新初始化产物和原初始化L2：复用测试专用独立MAT v5/zlib标签解析，比较真实Reader解码的全部140个double位模式，核对名称、shape、端点原零和有限角度范围。历史L2按原列主序输出全量CSV及每字段最大绝对差，仅作诊断，不据无批准的容差自动宣布科学等价。它不运行求解器、不以生产Reader生成参考、不写任何输入MAT；真实文件SHA前后比较。

全部输出保存在带中文/空格路径的独立 `build/.../r06_adaptive_cli/...` 沙箱。每次调用记录argv、cwd、实际build SHA、起止时间、stdout/stderr/退出码；实际随机seed来自各Manifest，不能用目录随机身份冒充算法seed。外部原始资料由全套先行五项MAT及运行前后摘要继续保护。无新的CLI参数、无新公共schema/算法依赖。

## 首轮判定约束

仅实测可填写结果。三个入口通过合同测试不等于三种科学实验全部成功；原始无定义角度、有限精度拒绝及独立L2差异均须保留。R6剩余正式数值/L4、跨环境、资源与审批义务不因测试数量增长关闭。

## 首轮实测：真实初始化提前拒绝

检查点 `3e39a1421b11ae6144e9d95ecd8eb072581499c6` 已提交、正常推送、经GitHub分支API确认后运行。RunId `r6-adaptive-cli-20261117-MJnRT5Fg`，完整证据 `build/r6-adaptive-cli-20261117-MJnRT5Fg/evidence/`，工作目录仓库根，复用 `build/r6-20261117-KzjFHPDQ/cmake/`。Ubuntu24.04.5/Linux7.0.0-43、GCC16.2、CMake/CTest4.3.4、Eigen3.4、matio1.5.26、zlib1.3、Sophus1.24.6；Release配置、并行4构建，先MAT五项再其余41项。精确命令/参数、起止时间、退出码、stdout/stderr、JUnit、源Git blob及输入SHA全部保存。

配置和编译退出0、无诊断；MAT5/5通过，其余40/41通过，总计 **45/46**，CTest退出8，输入摘要未改变。失败是新真实CLI测试的初始化成功预期；原45项仍通过。实际三个生成进程都已执行，输出位于构建目录 `r06_adaptive_cli/Adaptive 中文 空格/745d0faa6776ae3aae8b5918/`：

| 入口 | 实际退出/Manifest | 有效前缀及原因 |
|---|---|---|
| initialization | 5 / Incomplete | 第14 MATLAB帧，已完成12内部帧；comparison translation角度报 `normrelfehler acos argument outside [-1,1]`；attempted1/completed0/failed1/undefined1，无MAT |
| comparison，noise0 | 5 / Incomplete | 第151帧零旋转，149有效帧；expected_repetitions仍8，completed0，无部分MAT |
| update-rates | 5 / Incomplete | 第151帧零旋转，149有效帧；expected_repetitions1，completed0，无部分MAT |

初始化实际算法seed为2439300603（该模式未消费算法RNG）；其余入口实际seed和完整错误链在各Manifest，未以固定目录名代替。两个失败RunId的再次调用退出6且Manifest摘要不变。初始化四MAT/独立140值与L2差异工具**未执行**，不能因工具编译成功声称生成成功。

**R6-ADAPTIVE-CLI-001（待独立复核）**：既有 [角度边界证据](R6_INTERLEAVING_2026-11-17.md#同批补充acos舍入端点) 已确认不钳制acos的合同会拒绝单位化后点积越界，原 `normrelfehler.m` blob `c9fecc37180ab13d854a144d333a2599e601a9a8` 直接使用acos。GitHub all-state Issue/PR检索为空；本次暂不能仅凭日志断言具体向量/超出量。新增记录装饰器，逐次委托真实生产Step并保存最后一轮输入系数、320×6观测、系统矩阵/RHS、解和真值；用独立标量double及long double重新计算cosine，并要求最后真实Step可按位重放。

测试改为严格核对实测初始化的第14帧/12有效前缀/完整UndefinedAngle链/仅失败审计，并由新增真实Step诊断工具交叉验证边界；比较/更新率第151帧断言不变。原成功MAT核验分支保留，但当前未覆盖；若将来执行结果变为成功，本轮固定边界预期仍会报变化，成功产物也会完整核查。**这修正的是未经实测的初始化成功假设，不是修改生产公式/放宽门限或宣布算法成功**。待新检查点复测后才能确认原因；正式科学成功与鲁棒角度策略仍未闭合。

## 最终复测与原因确认

检查点 `2438a8039a905c7f3d8ab74aba1faeaa9a2387cf` 已正常提交、推送并经远端分支API核实。RunId `r6-adaptive-cli-trace-20261117-Ks76fbQ0`，日志 `build/r6-adaptive-cli-trace-20261117-Ks76fbQ0/evidence/`，环境/工作目录/配置与上一轮相同，所有命令/参数、输出、退出码、JUnit、输入Git blob/SHA和实际种子说明均保存。

| 操作 | 2026-11-17 UTC+08:00 | 结果 |
|---|---|---|
| CMake配置 | 21:28:46–47 | exit0 |
| Release增量构建 | 21:28:47–21:29:12 | exit0，无诊断 |
| 五项MAT CTest | 21:29:12–13 | exit0，5/5，real1.16秒 |
| 其余CTest | 21:29:13–21 | exit0，41/41，real7.73秒 |
| 输入SHA复核 | 结束21:29:21 | exit0，未改变 |

**46/46 CTest通过；三个真实Adaptive生成入口均为预期非成功，不代表科学实验完成。** 新用例实测6次CLI（3次生成、3次重复RunId拒绝）和1次独立诊断进程。运行子目录为 `r06_adaptive_cli/Adaptive 中文 空格/3b43d0c5c3fcba652b846f85/`。三个生成入口各写一份Incomplete Manifest，无MAT；初始化valid12/frame14、比较与更新率valid149/frame151，completed均0，undefined/failed均1。计划次数1/8/1保持，完整stderr错误对象与Manifest保存对象一致；重复RunId退出6且原Manifest摘要不变。实际算法种子分别为2333051600、518536331、3446348628；noise0比较仍按原序抽样，初始化/更新率不消费算法RNG。

真实Step记录器完成38次生产求解，失败发生在第14帧第1号比较槽（0-based，即更新后的Adaptive系数），前12帧全部完成。所有Step本身成功；角度失败的输入为：

- 真平移：`(0.105838521997944870479, 0.0192068599796313688177, 0)`。
- 估计平移：`(0.983929588586116565807, 0.178556894867590298936, 1.77050023892425639131e-10)`。
- 独立标量binary64分别单位化再点积：`1.00000000000000022204`，即1+2^-52；原公式直接acos的实数定义域已越界。
- long double独立norm/dot显示为1；这只说明此宽精度计算舍入到1，**不能据此断言几何角严格为零**。实际向量有限且非零，故不是零真值异常。
- 使用保存的同一输入系数及320×6观测重新调用真实Step，平移结果按位一致；真实角度函数再次返回相同UndefinedAngle。未钳制、改阈值、替换算法、丢弃失败样本或制造完整MAT。

记录 `initialization-last-solve.csv` 保存全部最后一步输入/矩阵/RHS/解及真值，其SHA-256 `ea3f662a7e36037d44a532208e1cd1edf5d3b3c41fa827af81af41d9ddd4d3aa`。R6-ADAPTIVE-CLI-001根因得到有限环境复核，是已有acos边界合同在真实初始化路径上的触发；初始化完整科学结果仍未成功，不能把诊断闭合写成算法问题已消失。比较/更新率则保持原零旋转无定义状态。跨工具链的首个浮点失败帧不能由本次数据保证。

CTest stdout SHA-256：MAT `d0c359290669a0ad6166c6b104efa698141cf0329a77edd7d6b0ed4abb3b7549`，其余 `33826fbdfc9005af5e57f1f9f6efb7bcd4ad007e58777c4dd951cf2e0fb4e830`。launcher stderr为空，CLI预期失败stderr保留在各调用日志；首轮45/46失败日志与原始输入均保留。

实际修改为CMake、`tests/integration/test_r06_adaptive_cli.cmake`、`r06_adaptive_cli_values.cpp`、`r06_adaptive_init_trace.cpp`及进度/证据文档；**生产公式和源文件未改动**。独立140值/L2差异工具虽已编译，但由于没有生成完整初始化MAT，其成功分支仍未执行。没有新GitHub Issue/PR、合并或发布。后续先继续当前实现/追踪及可独立完成的证据复核；三个Adaptive完整成功、非零噪声统计、完整科学/L4、资源/跨环境、正式数值验收仍未满足。改变不钳制角度或零旋转处理政策需要明确设计决定，不能以绿色CTest代替该决定。
