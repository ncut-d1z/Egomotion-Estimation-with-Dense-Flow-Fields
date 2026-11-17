# R6 原球谐系数动作与 Adaptive 集成

## 首轮准备：未执行

**[HUMAN-EXECUTE/APPROVE]** 起点 `6fcc01034932e8cb15d28c2316a511bcbf94fbb2`，本地/远端一致且工作区干净；已重读关联任务最新继续R6要求，查询全部open/closed Issue/PR为空。用户已授权生产修改和构建/CTest。前批29/29仅对应 `bc51fdd`，本批新增两项后登记31项，尚未编译或执行。

本批覆盖REQ-F-SH-002、TC-F-SH-002、P7.5/P7.6及Adaptive调用链。现有物理叶子合同固定为 `mu_new(d)=mu_old(A*d)`，保持不变；[专题LLD §3](../design/SH_ROTATION_IVANIC_RUEDENBERG.md) 又明确要求业务保留原算法，不能自动改为物理修正版。静态源码显示Runner直接传上一帧Rodrigues矩阵，Session调用物理适配器，未显式处理源 `SHRotate + transfor_koeff` 的无CS块/CS系数组合。原默认平面轨迹绕z旋转，此特例不能判别一般输入的差异。

独立参考来自已审阅、一般SO(3)符号核验的 [IR96 Table3显式表](../../verification/harmonics/verify_low_order_layout.wl)，另以源CS九个Cartesian多项式核对场作用，不调用生产IR递推/张量生成Expected。固定10个旋转包含三轴90°及逆、非交换xy/yx、q=(1,2,2,0)有理矩阵及其逆；九单位系数和不对称混合系数，逐项核对R0/R3/R5、填充及9方向场值。沿用旧SH专项 `4096*epsilon(double)` 绝对比较余量，不批准或修改DEC-005。最小反例 `2+z`、Rx90、ey：原组合为1，物理合同为3。

新增 `tests/integration/test_r06_source_sh.cpp` 的第二模式通过注入仅记录/增加固定系数的StepSolver，检查真实Session三种模式的22帧、全部1/5/11/21刷新、主/对照顺序和两处失败后的非交换旋转回滚。Expected使用显式表；输入无真实光流，不声称科学求解或MATLAB运行等价。先取得实际集成结果，再决定必要修正。原始MATLAB/MAT只读，不新增依赖，不冻结或代签验收。
