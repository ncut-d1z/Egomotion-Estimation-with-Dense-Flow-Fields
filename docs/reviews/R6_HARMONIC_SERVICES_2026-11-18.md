# R6：P7.10球谐职责分离

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。起点本地与GitHub分支API均为`33071acc9c16e37b7bb5d26414ae3444fc675c6c`，工作区干净；关联“检查R5实现完成度”最近要求继续检查/实现R6，状态idle。all-state Issue #1/#2/#3开放、PR为空。用户已授权生产修改、构建/CTest和方案文档。本轮承接[98项复核D2](R6_PLAN_TASK_AUDIT_2026-11-18.md)的P7.10、REQ-F-SH-001/TC-F-SH-001；不以职责文件已有自由函数替代PLAN明确的三个固定约定组件。

## 实现与边界

公共`include/egomotion/harmonics/services.hpp`新增RealSphericalHarmonics、HarmonicFitter、HarmonicRotator三个无状态具体类，各有独立实现文件；它们拥有原适配器的布局、投影、旋转组合与错误处理责任。共享叶子错误桥接移至私有`service_error.hpp`，枚举映射、家族/Code/消息/阶段保持。原`adapter.cpp`只保留四个兼容转发函数，没有第二套数学实现。Adaptive单步、Session和Periodic策略直接调用对应组件。固定约定无需虚接口、运行时模式字符串或共享可变缓存。

新增`RealSphericalHarmonics::sourceBasis`将已验证的论文Cartesian基经现有正交布局/相位转换返回源CS的3×5基值（l=0..2、m=-l..l），六项无效填零；其余原函数体只迁移归属。HarmonicFitter仍按原4π逐通道投影，仅读取流场前三列，不能悄悄改成联合最小二乘或新增尾列有限性限制。HarmonicRotator仍区分物理`mu_new(d)=mu_old(A*d)`和原`transfor_koeff`动作，Session用后者。原叶子公式/近度正负规则/填充有限性/求解顺序/误差阈值不变，原始资料只读，候选公共API未冻结。

## 首次验证计划

新增`r06_harmonic_service_contract`直接使用三个const组件：十个解析方向的源九基/六项零填充和单通道重建、独立七方向精确投影、负近度与无关尾列、输入销毁后结果自拥有；五个旋转的物理场作用与逆作用、独立Rx四分之一转的源/物理正负判别；非法方向/非有限/错尺寸/空/零投影范数/反射拒绝及完整叶子错误来源。旧自由函数逐位对照只证明兼容转发，不充当独立Golden。比较沿用球谐既有4096×epsilon工程余量，不新增或批准数值阈值。已有更广独立SH、单步、策略和真实Session/CLI回归全部保留。

首次尚未执行：先原子提交、正常推送并核实远端SHA，再新RunId双模式配置/构建，先MAT7项后其余CTest，预期ON61/OFF60。逐步骤保留命令/环境/源SHA/输入摘要/固定seed来源/起止时间/退出码/stdout/stderr/JUnit。不安装依赖、不运行MATLAB/Octave/完整科学或性能实验，不签署/冻结/合并/发布。D1领域构造不变量、D2其余Runner/Analyzer职责、D4记录范围及完整实验/资格仍未完成，R6未结案。

## 固定源码双模式实测

受测源码 **`a19d2d82ab9b7b27f9b77061e9f4880c8d8944bf`**，分支`dev-plan-P0-P1-mat-dictionary-probe`，正常推送并经GitHub branch API核实同SHA、工作区干净后执行。RunId **`r6-harmonic-services-20261118-e9E0GmUL`**，工作目录为仓库根。

| 配置 | 配置/构建 | MAT优先 | 其余CTest | 起止时间（2026-11-18，UTC+08:00） |
|---|---|---|---|---|
| SVG ON | exit0，无诊断 | 7/7 | 54/54，共61/61 | 03:23:08–03:24:56 |
| SVG OFF | exit0，无诊断 | 7/7 | 53/53，共60/60 | 03:25:04–03:26:34 |

新增组件测试两模式各 **482项检查**，最大绝对差 **1.7763568394002505e-15**；十方向九基/零填充/单通道重建、七方向精确投影、五旋转及正负动作判别、形状/有限性/方向/零范数/反射负例和叶子来源均通过。原SH多路线、源相位、单步、Session策略、场景/八轮合成、真实Adaptive失败链、MAT/CLI/Writer/分析等全部回归执行。真实三Adaptive仍保留原acos越界/零旋转的Incomplete、无MAT，不以新组件测试宣称完整科学成功。

四份JUnit与每模式CTest登记逐名一致，全部status=run，零failure/error/skip/disabled；ON唯一多出`r06_svg_source_windows`。OFF编译源、链接列表、ELF符号及静态库检查无SVG实现。各721项输入SHA后验全匹配，原628文件/582MAT树仍为`70ee155131c24a50169346d0ffbaff0c01b1c547`。静态逐函数比较确认4个原适配函数体、相位函数及4个错误辅助函数只改变辅助命名空间限定，结果存于`read-only-body-audit.json`；低阶叶子及原始资料没有改动。

实测环境为Ubuntu24.04.5、Linux7.0.0-43 x86_64、GNU16.2、C++17 Release、CMake/CTest4.3.4、Eigen3.4、matio1.5.26、zlib1.3、Sophus1.24.6。新测试全部固定输入、无RNG；其它seed来源另存，RunId独立OS随机。启动命令为`bash build/r6-harmonic-services-20261118-e9E0GmUL/run.sh a19d2d82ab9b7b27f9b77061e9f4880c8d8944bf ON`及同参数`OFF`。本RunId的`evidence-ON/`、`evidence-OFF/`保留环境、所有步骤命令/参数、起止时间、退出码、stdout/stderr、JUnit、Testing快照、输入blob/SHA、随机来源、CMake缓存、编译/链接和ELF；后验逐名核对为`read-only-junit-audit.json`。复用原独立构建缓存，没有覆盖先前失败或其它运行证据。

P7.10源码职责缺口已补，正式对象/数值验收保持未签署。D1、D2其余Runner/Analyzer职责、D4记录边界、完整实验/资格仍未完成，R6未结案。本轮无新构建/测试缺陷，Issue #1/#2/#3开放，无PR；随后文档提交仅同步事实，不代表重新构建。

文档静态复核：98个P与PLAN逐项同序，66个TC及45源路径/50声明与变更前身份和顺序一致；TEST_PLAN除顶部事实注记外正文完全相同。11份更新文档的517处相对文件引用均有目标，结果为`read-only-document-audit.json`；这不是Bash追踪脚本现场执行或新增CTest证据。
