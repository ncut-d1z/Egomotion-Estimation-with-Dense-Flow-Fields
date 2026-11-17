# R6：P7.10球谐职责分离

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。起点本地与GitHub分支API均为`33071acc9c16e37b7bb5d26414ae3444fc675c6c`，工作区干净；关联“检查R5实现完成度”最近要求继续检查/实现R6，状态idle。all-state Issue #1/#2/#3开放、PR为空。用户已授权生产修改、构建/CTest和方案文档。本轮承接[98项复核D2](R6_PLAN_TASK_AUDIT_2026-11-18.md)的P7.10、REQ-F-SH-001/TC-F-SH-001；不以职责文件已有自由函数替代PLAN明确的三个固定约定组件。

## 实现与边界

公共`include/egomotion/harmonics/services.hpp`新增RealSphericalHarmonics、HarmonicFitter、HarmonicRotator三个无状态具体类，各有独立实现文件；它们拥有原适配器的布局、投影、旋转组合与错误处理责任。共享叶子错误桥接移至私有`service_error.hpp`，枚举映射、家族/Code/消息/阶段保持。原`adapter.cpp`只保留四个兼容转发函数，没有第二套数学实现。Adaptive单步、Session和Periodic策略直接调用对应组件。固定约定无需虚接口、运行时模式字符串或共享可变缓存。

新增`RealSphericalHarmonics::sourceBasis`将已验证的论文Cartesian基经现有正交布局/相位转换返回源CS的3×5基值（l=0..2、m=-l..l），六项无效填零；其余原函数体只迁移归属。HarmonicFitter仍按原4π逐通道投影，仅读取流场前三列，不能悄悄改成联合最小二乘或新增尾列有限性限制。HarmonicRotator仍区分物理`mu_new(d)=mu_old(A*d)`和原`transfor_koeff`动作，Session用后者。原叶子公式/近度正负规则/填充有限性/求解顺序/误差阈值不变，原始资料只读，候选公共API未冻结。

## 首次验证计划

新增`r06_harmonic_service_contract`直接使用三个const组件：十个解析方向的源九基/六项零填充和单通道重建、独立七方向精确投影、负近度与无关尾列、输入销毁后结果自拥有；五个旋转的物理场作用与逆作用、独立Rx四分之一转的源/物理正负判别；非法方向/非有限/错尺寸/空/零投影范数/反射拒绝及完整叶子错误来源。旧自由函数逐位对照只证明兼容转发，不充当独立Golden。比较沿用球谐既有4096×epsilon工程余量，不新增或批准数值阈值。已有更广独立SH、单步、策略和真实Session/CLI回归全部保留。

首次尚未执行：先原子提交、正常推送并核实远端SHA，再新RunId双模式配置/构建，先MAT7项后其余CTest，预期ON61/OFF60。逐步骤保留命令/环境/源SHA/输入摘要/固定seed来源/起止时间/退出码/stdout/stderr/JUnit。不安装依赖、不运行MATLAB/Octave/完整科学或性能实验，不签署/冻结/合并/发布。D1领域构造不变量、D2其余Runner/Analyzer职责、D4记录范围及完整实验/资格仍未完成，R6未结案。
