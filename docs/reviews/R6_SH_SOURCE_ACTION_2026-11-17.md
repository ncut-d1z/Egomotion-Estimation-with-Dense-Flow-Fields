# R6 原球谐系数动作与 Adaptive 集成

## 首轮实测与修复候选

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
