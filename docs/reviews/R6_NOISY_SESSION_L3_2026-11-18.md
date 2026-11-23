# R6：固定噪声与真实多帧Session独立对照

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。受测源码 **`928a8071c7c4ff70b7e38a4ed91c13a31c770323`**，分支`dev-plan-P0-P1-mat-dictionary-probe`。在用户已有生产代码、构建/CTest及文档授权内，提交/正常推送并以GitHub branch API核实SHA后执行。**SVG ON72/72、OFF71/71 CTest通过**，配置/编译无诊断，零失败/跳过/禁用。本批没有改生产数值公式、阈值或RNG实现；参考仍为候选/未冻结，R6未结案。

## 目的与实际变化

原噪声测试和无噪声多帧测试分别验证局部模块，尚缺含噪观测进入真实Session后的独立演进参考。本批在既有测试入口加入`--noise`，新增CTest `r06_noisy_session_independent_multiframe`；真实`MeanMagnitudeGaussianNoise`调用之后，由真实Adaptive Step和Session处理252帧。独立参考从相同固定理想场/样本出发，以标准库long double计算噪声和投影，再由自身主系数继续演进；从不拿真实噪声输出或生产状态替换Expected。

修改文件为根[CMakeLists](../../CMakeLists.txt)、[集成测试](../../tests/integration/test_r06_session_l3.cpp)、[独立标量参考](../../tests/support/adaptive_scalar_reference.hpp)及[固定含噪输入说明](../../verification/adaptive/noisy_session_reference.md)。后者记录原`nonpropfehler.m` blob `c25d4743f5dc99b4825b738d822457d541813350`及固定样本公式。与SN-G06/AD-G05多帧子范围、P3.5/P3.7/P8.3、TC-F-FLOW-003/TC-F-ADAPT-002/TC-NF-RNG-001关联；正式TC/AC及接口不变。

## 实测结果与判据

Comparison模式，系数0、+0.1、-0.1各两条固定12视线流，MATLAB帧2–43，每条独占Session及固定样本源。参考先用两行模长5的精确有理投影结果校准。固定样本是测试输入，不能称为Gaussian分布验证或MATLAB同seed序列。

| 每种SVG配置的证据 | 实际结果 |
|---|---|
| 完整成功帧与检查 | 252帧，290923项检查，0失败 |
| normal消费 | 每流1512次、合计9072次，零系数仍全部消费；按每行x/y/z顺序，禁止uniform或reseed |
| 真实Step次数 | 记录器780次=756个成功帧节点+24个随后角度失败的比较求解；另默认Session首帧18次，共798次，不是798个独立科学样本 |
| 回滚 | 每流i=5/11/21/42复用已生成观测并令真旋转轴为零；真实UndefinedAngle后主和四基线逐位不变，同帧重试成功。不能由此推断Runner会回滚RNG |
| 噪声场最大绝对差 | `3.06178693509906452164e-16` |
| Step/状态最大绝对差 | `1.94913529760754045128e-14` |
| 最大缩放差 | `1.15713489189081181873e-14`，abs(delta)/(1+abs(reference)) |
| 比较角度最大绝对差 | `1.00516817092000110279e-13`度 |

数值来自C++ long double比较stdout。噪声场沿用已有sampling_flow绝对1e-12；Step沿用2e-8混合余量、角度沿用1e-9混合余量；没有新增/放宽工程或正式阈值。所有比较/主求解的near、均值、M1–M4/6×6、rhs/解、t/ω、MSE、新near/系数、输入系数、比较角度及提交状态逐节点核对。真实生产输出原样转交，没有数值替身。

原`r06_session_independent_multiframe`仍为299236项/252帧/24回滚，参考和实际JSONL分别与493ba86时的SHA完全相同。21组既有数学/合同摘要也与b2628a9相同，球谐494项最大差仍1.7763568394002505e-15；共享辅助代码没有改变原证据。

## 执行身份与留存

RunId为`r6-noisy-session-20261118-FnQ0oODZ`。实际环境Ubuntu24.04.5、Linux7.0.0-43-generic x86_64、GNU16.2.0/C++17 Release、CMake/CTest4.3.4、Eigen3.4.0、matio1.5.26、zlib1.3、Sophus1.24.6；工作目录为仓库根，无依赖安装。ON为2026-11-18 20:52:29–20:53:45，OFF为20:54:12–20:55:20，均+08:00。先运行MAT7项，再ON65项/OFF64项；configure/build/CTest/输入完整性全部退出0。

```bash
bash build/r6-noisy-session-20261118-FnQ0oODZ/run.sh 928a8071c7c4ff70b7e38a4ed91c13a31c770323 ON
bash build/r6-noisy-session-20261118-FnQ0oODZ/run.sh 928a8071c7c4ff70b7e38a4ed91c13a31c770323 OFF
```

复用ON `build/r6-20261117-KzjFHPDQ/cmake`、OFF `build/r6-headless-20261117-Joe73gEd/cmake-off`独立缓存；本RunId下`evidence-ON/`、`evidence-OFF/`保存完整展开命令/时间/退出码、环境、分离stdout/stderr、四份JUnit、Testing、依赖图/ELF、CMakeCache和compile_commands。737项origin/verification/tests文件的前后SHA全部一致，原628文件/582MAT的Git树仍为`70ee155131c24a50169346d0ffbaff0c01b1c547`。

新沙箱为ON缓存下`r06_noisy_session_l3/独立含噪-PAUH0N/`与OFF对应的`独立含噪-r3FWsH/`；完整JSONL另复制到本RunId的`saved-noisy-ON/`、`saved-noisy-OFF/`，每文件252行。保存理想输入、全部固定normal/序号、噪声系数、独立参考平均模长、各自生成的含噪场及全部成功帧Step/状态/角度。两模式对应文件相同：

| 文件 | 字节数 | SHA-256 |
|---|---:|---|
| reference.jsonl | 3011906 | 25254c52b196f9aa88e0ff68cc55e87fadc2c05190431ea1732f63267bf613c9 |
| actual.jsonl | 3078630 | 8692739a2baf0c22adf6a842f41d09540ae0cb86db938f92764cec3ecbab8ec8 |

只读Node审计匹配全部JUnit名称和status=run；ON/OFF唯一登记差异仍为`r06_svg_source_windows`，OFF无绘图源码/目标/符号。含噪JSONL另核对每模式152460个保存数值，独立以binary64重算平均模长/投影、固定样本索引及正负扰动中点；该复核不替代C++ long double误差或数学参考。正负观测最大差约0.611743，不能以零扰动冒充三条分支。保存`read-only-noisy-audit.json`、`read-only-multiframe-audit.json`、`read-only-original-reference-audit.json`及JUnit/旧摘要审计，旧证据未覆盖。

文档静态核对12份文件/472条相对链接，98个P编号、66个TC编号、45源路径/50声明保持；TEST_PLAN与Adaptive LLD除顶部事实接续外的合同正文未变。TC-F-FLOW-002的证据摘要纠正为原随机距离/近度义务，均模长与分量噪声均在原TC-F-FLOW-003下登记，没有移动或更改SRS验收义务。保存`read-only-document-audit.json`，不将Node静态核查当作Bash追踪脚本现场执行。

## 尚未完成

本批未运行原35/600帧完整含噪Runner/8轮统计，未生成完整科学MAT或L4结果；原真实Adaptive初始化帧14acos越界、另两入口帧151零旋转仍为Incomplete。不得把固定输入测试通过称为原实验成功、分布资格或正式Golden/DEC-005验收。病态前向误差、完整Bias/L4、跨环境/资源、冻结及发布仍未完成；没有MATLAB/Octave/L1阻塞。本轮没有新缺陷Issue，#1–#5保持开放，无PR，不合并/发布。
