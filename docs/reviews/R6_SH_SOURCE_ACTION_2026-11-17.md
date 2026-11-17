# R6 原球谐系数动作与 Adaptive 集成

## 最终复测：31/31 CTest通过

**[HUMAN-EXECUTE/APPROVE] 受测源码 `632139bb0d9dd0d3afa7a11afdb0578224a47f59` 已配置/编译成功，31/31 CTest通过；构建无诊断。** 本段为 `[AI-EDIT]` 事实登记，不代签AC/DEC或冻结。每次运行前均原子提交、正常推送并核实远端 `dev-plan-P0-P1-mat-dictionary-probe` 包含受测SHA。R6-SH-001由独立参考发现，保留首轮全部失败断言和日志后取得复测证据；未创建/关闭Issue或PR，未合并/发布。

RunId `r6-sh-source-fix-20261117-IPXj1uHL`，证据目录 `build/r6-sh-source-fix-20261117-IPXj1uHL/evidence/`；复用 `build/r6-20261117-KzjFHPDQ/cmake/` 并重新配置源码身份，属于增量构建。环境Ubuntu24.04.5、GCC16.2.0、CMake/CTest4.3.4、Eigen3.4.0、Sophus1.24.6、matio1.5.26、zlib1.3，C++17 Release，无依赖安装。新数学/Session夹具完全确定，无算法随机种子；其余测试种子由 `randomness-source.txt` 与源码记录，RunId由独立OS随机源产生。未执行完整科学实验、MATLAB/Octave或L4统计。

| 步骤 | 开始/结束（2026-11-17 UTC+08:00） | 退出码 | 实际结果 |
|---|---|---:|---|
| configure | 17:47:43 / 17:47:45 | 0 | 固定受测SHA正确 |
| build | 17:47:45 / 17:49:18 | 0 | 全目标通过，无编译诊断 |
| MAT CTest | 17:49:18 / 17:49:19 | 0 | 5/5，0.64秒 |
| 其余CTest | 17:49:19 / 17:49:20 | 0 | 26/26，0.70秒 |
| 输入完整性 | 17:49:20完成 | 0 | 原始资料、独立参考及测试文件SHA一致 |

配置参数 `-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`；构建 `cmake --build <build-dir> --parallel 4 -- -k`。CTest均为 `-C Release --verbose --no-tests=error --output-junit <evidence>.xml`；先 `--parallel 2 -R '^(r02_mat_reader|r06_historical_mat_readonly_schema|r06_mat_independent_storage_values|r06_mat_reader_boundaries|r06_mat_raw_payload_boundaries)$'`，再 `--parallel 4 -E` 同表达式，覆盖全部登记项。完整命令/参数/cwd/时区/环境/exit/stdout/stderr、JUnit、输入SHA/Git blob和Testing快照均保留，所有launcher stderr为空。

| 新增CTest | 内部检查 | 最大绝对误差 | 限定结论 |
|---|---:|---:|---|
| `r06_sh_source_phase_reference` | 4886/4886 | 4.4408920985006262e-16 | 10个固定旋转，九单位/混合系数，R0/R3/R5、相位、9方向场值、组合/逆/错误接口；既有物理合同仍通过，源动作与原静态数学参考一致 |
| `r06_sh_source_session_action` | 10006/10006 | 6.6613381477509392e-16 | 三模式各22帧、全部周期、主/比较/提交系数及旋转后失败回滚；修复前1667项差异已消失，替身不证明真实Step科学数值 |

沿用既有SH比较余量9.0949470177292824e-13，无门限调整。前批29项仍通过，包括原582 MAT/33,340值逐位/SHA、94载荷/29Reader边界、29发布故障，以及Adaptive固定8组600行聚合。默认Comparison/UpdateRates在当前替身下仍于第151帧出现UndefinedAngle（149有效、0完整）；不能把负例通过声称为完整实验成功。

MAT stdout SHA-256 `77e9956746cdc58daaa3f6a30d15d158d3f8d58007aa3e39def77de8ede2c999`；其余CTest stdout SHA-256 `31e18c5082eedbe264938711c07eea4a036a81035f3a6ccc6b38ffd64aa8afeb`。原 `origin` Git树仍为 `70ee155131c24a50169346d0ffbaff0c01b1c547`。

**实际源码改动**：`include/egomotion/harmonics/adapter.hpp`、`src/harmonics/adapter.cpp`增加源接口；`adaptive_session.hpp/.cpp`和`adaptive_runner.cpp`接通并明确输入身份；新 `tests/integration/test_r06_source_sh.cpp`及CMake登记两项；既有三份Session/Runner测试仅同步字段名。LLD/进度记录区分物理叶子与忠实原组合；没有修改底层递推、原文件、采样/求解公式/次数或误差阈值。

**剩余范围**：本批补足l≤2源动作差异限定及真实Session集成证据，不宣称所有浮点输入或原MATLAB警告/异常行为等价；R6的剩余独立L3、完整真实Runner/L4、广泛资源边界、DEC-005/006/008与正式验收仍未完成。下一批应继续核对这些业务覆盖缺口，不重新把已覆盖的源相位问题列为未实现。

## 首轮实测与修复候选（历史）

源码 `2d271ee154544644b4866940267332e9de758435` 已提交/推送/远端核实。RunId `r6-sh-source-baseline-20261117-BhrF3c1P`：configure/build exit0；新替身的未初始化 `trace.coefficients_next` 产生一条GCC警告，需显式初始化而不能关闭诊断。MAT先5/5、其余25/26，合计30/31 CTest；原29项保持通过。`r06_sh_source_phase_reference` 3302项检查全部通过，最大绝对误差4.4408920985006262e-16；`r06_sh_source_session_action` 10006项中1667失败，最大绝对差4.1944444444444438。独立反例raw=1/physical=3得到确认，原始输入哈希未变。日志目录为同名build目录的 `evidence/`，其余CTest stdout SHA-256为 `1bf4ae9c87fa48a230767aeaae2882acb7e5ac90c4797c867ccea5f0b4cb95f8`。

**本地缺陷R6-SH-001**：Session把物理适配器直接当作源 `transfor_koeff`，Comparison第3帧输入C(2,1)实际+0.25、源参考−0.25；UpdateRates的非恒定主/对照也受到影响。Initialization不旋转、原绕z特例通过不能否定这一通用输入缺陷。保存证据后再次检索全部Issue/PR为空，未创建远端Issue。不据此宣称论文或默认平面实验的全部数值错误。

修复只在集成边界增加 `rotateSourceCoefficients`：物理源CS动作 `D_CS=S R_source S`，所以 `S D_CS S=R_source`；S1=(-1,1,-1)、S2=(1,-1,1,-1,1)。复用现有纯Eigen IR生产叶子及原错误链，仅四处系数变号，没有复制上游代码、反转轨迹或修改物理API的定义。Session两处调用显式采用源接口，帧成员重命名为 `source_coefficient_rotation` 以免错误承诺物理参数动作；Runner仍直接传原上一帧Rodrigues矩阵。没有新增模式开关或改变原采样/求解/刷新次数。保持既定忠实移植义务，符合专题LLD §3，不改变需求阈值/范围或冻结状态。

新增直接源API对全部块/系数的断言、非交换组合、逆变换、无效填充和类型化失败；原1667项失败断言保持。替身明确初始化Trace。待新检查点后复测全部31项；当前修正尚未编译/执行。正式数值资格、完整Runner/L4与剩余L3仍未完成。

固定来源Git blob：`SHRotate.m`=`7b73000411a9294f6cbed2e111bf74ce7fb7d4df`，`transfor_koeff.m`=`7f26019cab53666539e1961dac67a2e07dae614c`，独立显式表脚本=`644eaa967316d52a3769ef1508f2db80d6607c89`。这些是来源身份，未将静态翻译声称为MATLAB运行轨迹。

## 首轮准备（历史）：未执行

**[HUMAN-EXECUTE/APPROVE]** 起点 `6fcc01034932e8cb15d28c2316a511bcbf94fbb2`，本地/远端一致且工作区干净；已重读关联任务最新继续R6要求，查询全部open/closed Issue/PR为空。用户已授权生产修改和构建/CTest。前批29/29仅对应 `bc51fdd`，本批新增两项后登记31项，尚未编译或执行。

本批覆盖REQ-F-SH-002、TC-F-SH-002、P7.5/P7.6及Adaptive调用链。现有物理叶子合同固定为 `mu_new(d)=mu_old(A*d)`，保持不变；[专题LLD §3](../design/SH_ROTATION_IVANIC_RUEDENBERG.md) 又明确要求业务保留原算法，不能自动改为物理修正版。静态源码显示Runner直接传上一帧Rodrigues矩阵，Session调用物理适配器，未显式处理源 `SHRotate + transfor_koeff` 的无CS块/CS系数组合。原默认平面轨迹绕z旋转，此特例不能判别一般输入的差异。

独立参考来自已审阅、一般SO(3)符号核验的 [IR96 Table3显式表](../../verification/harmonics/verify_low_order_layout.wl)，另以源CS九个Cartesian多项式核对场作用，不调用生产IR递推/张量生成Expected。固定10个旋转包含三轴90°及逆、非交换xy/yx、q=(1,2,2,0)有理矩阵及其逆；九单位系数和不对称混合系数，逐项核对R0/R3/R5、填充及9方向场值。沿用旧SH专项 `4096*epsilon(double)` 绝对比较余量，不批准或修改DEC-005。最小反例 `2+z`、Rx90、ey：原组合为1，物理合同为3。

新增 `tests/integration/test_r06_source_sh.cpp` 的第二模式通过注入仅记录/增加固定系数的StepSolver，检查真实Session三种模式的22帧、全部1/5/11/21刷新、主/对照顺序和两处失败后的非交换旋转回滚。Expected使用显式表；输入无真实光流，不声称科学求解或MATLAB运行等价。先取得实际集成结果，再决定必要修正。原始MATLAB/MAT只读，不新增依赖，不冻结或代签验收。
