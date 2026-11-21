# R6：Adaptive Session 多帧独立数值参考实测

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。用户已明确授权生产代码、构建/CTest及方案文档；本批在远程检查点后执行。受测源码 **`493ba86917a8147a0ddf63fad214751f1b33dd9c`**，分支 `dev-plan-P0-P1-mat-dictionary-probe`。**SVG ON 71/71、OFF 70/70 CTest通过**，两次配置/编译无诊断，零失败/跳过/禁用。没有变更生产数值实现或数值门槛，没有冻结Golden/合同或代签验收；R6仍未结案。

## 补足的证据缺口

原Session实测分别使用同一生产实现独立/交替重放，或固定Step替身检查时序；这些能证明隔离与组合，不能独立验证演进中的数学状态。本批新增只使用标准库`long double`的参考：独立部分选主元Gauss-Jordan求解、源CS多项式基及三维张量参数代换、逐通道投影。它不调用生产Eigen求解、球谐递推/拟合/Session，且每帧从自身参考状态继续，绝不由生产结果重置期望状态。

参考先核对已有Wolfram正/负均值单步值和`2+z`源/物理旋转判别，再按三模式、两条固定12视线流执行MATLAB帧2–43。真实生产Step由透明记录器调用，原结果原样返回；另核对六条序列首帧默认未注入Session。固定输入、源blob SHA、公式、布局及独立性完整说明见 [参考定义](../../verification/adaptive/session_multiframe_reference.md)。对应AD-G04/05/06的多帧/周期子范围、P7.7/P7.8/P7.9/P8.3/P8.5/P8.8及TC-F-ADAPT-001/002/003、TC-NF-SESSION-001；**未覆盖AD-G05的完整addederror实验/八轮rottransratio交付，也不替代P8.1/P8.2原35/600帧实验**。

## 实际检查

每种SVG配置的结果相同：

| 证据 | 实际结果 |
|---|---|
| 完整成功帧 | 三模式×两流×42帧，共252帧 |
| 真实Step调用 | 记录器948次：924次成功帧节点、24次随后角度失败的比较求解；另22次默认Session首帧调用，共970次。不是970个独立科学样本 |
| 失败原子性 | 在每条流i=5/11/21/42令真旋转轴为零，真实比较求解后返回UndefinedAngle；24次失败前后主与四基线逐位不变，仍与独立上帧一致，同帧重试成功 |
| 刷新边界 | 每条UpdateRates流的1/5/11/21周期刷新精确为42/8/3/2；每流26个0-based偏移判别点；四基线均能区分误提交dumi |
| 源旋转判别 | 每条序列27个源/物理动作可区分点，含非交换旋转 |
| 数值及结构检查 | 299236项，0失败；每个比较/主求解的输入、raw/sign/normalized near、原均值、M1–M4/6×6、rhs/raw解、t/ω、MSE、新near/九系数、比较角度和提交状态均核对；六个未用布局位置精确为0 |
| 最大数值绝对差 | `1.38144704009413032963e-14` |
| 最大缩放差 | `1.01199598751027928934e-14`，定义为abs(actual-reference)/(1+abs(reference)) |
| 最大角度绝对差 | `5.82190545772576228956e-14`度 |

上述最大误差来自C++ long double比较的原始stdout；不以独立JSON读取时的binary64舍入值替代。工程余量沿用既有单步`2e-8*(1+abs(reference))`及角度`1e-9*(1+abs(reference))`，不是新批准的DEC-005阈值。没有夹断acos、跳过失败输入、缩减样本或改8次聚合除数。

## 运行身份与保存证据

RunId：`r6-session-l3-20261118-UF1B2GwG`。工作目录为仓库根；原子提交和正常推送后，以GitHub branch API确认完整SHA，再分别启动两配置。环境为Ubuntu24.04.5、Linux7.0.0-43-generic x86_64、GNU16.2.0/C++17 Release、CMake/CTest4.3.4、Eigen3.4.0、matio1.5.26、zlib1.3、Sophus1.24.6；未安装依赖。固定输入不使用RNG，整个套件的既有seed/OS RunId来源另记日志。

| 模式 | 开始/结束（+08:00） | CTest顺序 | 退出状态 |
|---|---|---|---|
| ON | 2026-11-18 20:38:33–20:39:50 | 先MAT7项，再64项 | configure/build/CTest/输入核验均0 |
| OFF | 2026-11-18 20:40:11–20:41:29 | 先MAT7项，再63项 | configure/build/CTest/输入核验均0 |

执行命令入口（完整展开命令及参数在各`.meta`中）：

```bash
bash build/r6-session-l3-20261118-UF1B2GwG/run.sh 493ba86917a8147a0ddf63fad214751f1b33dd9c ON
bash build/r6-session-l3-20261118-UF1B2GwG/run.sh 493ba86917a8147a0ddf63fad214751f1b33dd9c OFF
```

执行器复用ON `build/r6-20261117-KzjFHPDQ/cmake`和OFF `build/r6-headless-20261117-Joe73gEd/cmake-off`独立缓存；本轮日志及新测试输出均独占，旧失败证据未覆盖。`build/r6-session-l3-20261118-UF1B2GwG/evidence-ON/`、`evidence-OFF/`保存environment、命令/时间/退出码、分离stdout/stderr、四份JUnit、Testing、CMakeCache/compile_commands/链接命令、ELF与依赖图。每模式736项origin/verification/tests输入SHA-256前后不变；原628文件/582MAT的Git树仍为`70ee155131c24a50169346d0ffbaff0c01b1c547`。

新增输出位于各缓存的`r06_session_l3/独立多帧-aSSLOh/`（ON）和`r06_session_l3/独立多帧-iWc4h0/`（OFF），并完整复制到本RunId的`saved-multiframe-ON/`、`saved-multiframe-OFF/`。每份JSONL有252行，包含全部输入、924个成功帧Step节点、主/基线状态和比较角度。两配置对应文件SHA完全相同：

| 文件 | 字节数 | SHA-256 |
|---|---:|---|
| reference.jsonl | 3008566 | 4b141e0ed851d7983298ae417a5c8392e2ec47a28e5449f18c087185e0932205 |
| actual.jsonl | 3069457 | 2b13b5c5601c00a189ad36bd8ee3e9a13e86f5a1b3568e549d75b2eaabfdab59 |

只读Node复核逐项匹配JUnit登记名称与status=run；ON/OFF唯一测试差异为`r06_svg_source_windows`。JSONL逐行核对6条连续序列、形状/有限性、每模式145824个数值及输入完全相同；它是保存证据的复核，不是另一套数学Golden。21组既有数值/合同摘要与b2628a9逐字一致，球谐服务494项最大差仍`1.7763568394002505e-15`。审计保存`read-only-junit-audit.json`、`read-only-multiframe-audit.json`、`read-only-regression-summary-audit.json`；OFF仍无绘图源码/目标/符号。

## 边界与交付

本批实际修改根[CMake](../../CMakeLists.txt)、[集成测试](../../tests/integration/test_r06_session_l3.cpp)、[独立参考头](../../tests/support/adaptive_scalar_reference.hpp)及参考说明，另同步进度、P/TC、数值证据、Adaptive LLD和Golden证据矩阵。参考与接口仍是候选/未冻结；本轮没有新源码缺陷或Issue，Issue #1–#5保持开放，无PR，不合并/发布。

只读文档复核核对12份文件的469条相对链接，98个P编号、66个TC编号、45个源路径/50声明不变；TEST_PLAN及Adaptive LLD除顶部事实接续外的合同正文不变。保存`read-only-document-audit.json`，不将Node只读核查称为Bash追踪脚本现场执行。

原真实Adaptive初始化仍在第14帧acos越界，Comparison/UpdateRates仍在第151帧零旋转时拒绝，均Incomplete、无完整MAT。本测试的有限固定比较轴明确不是原轨迹真值；通过不能消除上述科学成功路径缺口。完整Bias/L4、一般及病态输入资格、正式Golden/容差、资源/平台与冻结验收仍未完成。没有运行MATLAB/Octave，也没有以缺少L1作为阻塞。
