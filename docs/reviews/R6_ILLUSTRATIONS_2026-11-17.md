# R6 三份插图移植与独立几何验证

任务标签 `[HUMAN-EXECUTE/APPROVE]`。生产代码、构建/CTest、方案文档已获本轮明确授权；新CLI/schema部分为 `[AI-DRAFT + HUMAN-APPROVE]` 候选实现，未冻结。关联 P9.7、REQ-F-AN-002、REQ-DC-ANALYSIS-001、TC-F-CLI-005、DEC-008。不改变核心求解器、原资料、数值验收阈值或批准状态。

## 实现及参考范围

三份原脚本仅静态阅读，未运行MATLAB/Octave。原始资料来源为 Strübbe、Stürzl、Egelhaaf 的 [2015论文](https://doi.org/10.1371/journal.pone.0128413) 与 [MATLAB归档](https://doi.org/10.4119/unibi/2736885)，保留原来源身份；新C++独立编写，未复制第三方实现。

| 原文件 | Git blob | C++行为 |
|---|---|---|
| circle_curve_in_the_sphere.m | `c6bdbff07cdfff796c105f299fb728b15783ac7c` | 600点，半径0.6，高度0.3；sphere(50)单位球及y≤0的z-only遮罩 |
| sinosodial_curve_in_the_sphere.m | `06022612e9249c532f4dabae5658efa9ece4c10d` | 600点，振幅0.5、长度1、周期2、高度0.3；相同球面遮罩；保留原错误圆轨迹标题，用kind区分 |
| spherical_harmonic_illustration.m | `d7f97e0fe15c761f33f6ebd152610ec4144561e9` | θ/φ各41点，未归一化P_l^m(cosθ)cos(mφ)，scale=max(abs(yy))，rho=5+3*yy/scale |

轨迹复用纯数值TrajectoryGenerator，辅助来源分别为 `6d4bd8a7d0b7cb516b1ab6e80061617b386977bd`、`e2857131361619593f13f1bcd2cf2207f1a90493`。分母保持600，不重复终点；不复制原辅助函数的line副作用。

几何在 `egomotion_illustrations`，SVG在独立 `egomotion_illustration_svg`。无Python/NumPy/MATLAB或外部绘图库依赖；纯数值入口无需Reader、Runner或图像后端。生产递推使用关联勒让德的P_m^m初值及升degree关系，保留Condon–Shortley相位，依据 [MathWorks legendre](https://www.mathworks.com/help/matlab/ref/legendre.html) 的默认unnorm定义和 [DLMF 14.10.3](https://dlmf.nist.gov/14.10)。不依赖求解器的九项归一化基函数，不人为限制为degree≤2。非法阶数/零尺度/非有限结果明确失败。

[sphere文档](https://www.mathworks.com/help/matlab/ref/sphere.html)规定单位球及(n+1)×(n+1)形状。实现使用纬度[-π/2,π/2]、经度[-π,π]，精确子午线/极点取零，因此2601节点中1425节点只将z设NaN。**未取得MATLAB内部sphere端点舍入、接缝排序的运行轨迹，不能把该离散化选择声称为逐位一致**；JSON保留全部x/y、显式z_masked和仅有意遮罩的null，其余NaN/Inf拒绝。后续来源审查可复核这一已显式记录的端点选择。

SVG采用正交等比例投影、面片深度排序和面法线平面着色；轨迹视角(-37.5,30)、径向面(40,30)/zoom1.5，视角定义参照 [MathWorks view](https://www.mathworks.com/help/matlab/ref/view.html)。渲染器不是MATLAB Phong光照，图像外观和逐像素等价未验证。所有数值位于独立JSON，不以图形外观替代几何验证。

## CLI/schema候选实现（未冻结）

`egomotion illustrate --kind circle|sinusoidal|harmonic [--format json|svg]`。harmonic必需 `--degree N --order M`，完整非负十进制且0≤M≤N≤INT_MAX；轨迹拒绝这两个参数。默认json只写stdout；svg可带 `--run-id ID`，固定独占 `output/<RunId>/`，发布 `illustration.json` 与 `illustration.svg`。无自定义输出根；自动RunId复用独立OS随机源，不消费算法RNG。

`egomotion-illustration-v1`包含kind/source_script_blob/source_title/helper_source_blobs、列主序surface(rows/columns/角名/弧度角/vertices_xyz/z_masked)、trajectory_xyz、harmonic(degree/order/amplitude_scale/unnormalized_yy/rho或null)、view。角与三维坐标保留double的max_digits10；不设科学实验Manifest，不把一幅插图冒充Complete实验。JSON与SVG均准备成功后才发布；第二文件失败保留第一文件及错误，不声称成功，也不覆盖已有结果。

退出码复用既有合同：解析2、数值5、路径/写入6、可选后端不可用7，错误为既有JSONL结构。CLI输出属于候选增量，不改变已有命令、66项TC或DEC-008正式冻结状态。

## 首轮准备（尚未执行）

新增两项CTest，登记由43增至45，尚不表示通过。`r06_illustrations_independent_geometry`以long double标量公式复核两种600点轨迹、51×51球面和遮罩；用独立Rodrigues有限多项式逐项求导核对degree0…8全部45组、每组41×41的YY/scale/rho/x/y/z。参考不调用生产递推、Eigen特殊函数或MATLAB。沿用工程回归量级1e-9，比较 `abs(actual-reference)≤1e-9*(1+abs(reference))` 并记录最大绝对/缩放误差；这不是新增DEC-005正式数值通过阈值。

直接Exporter测试核对2208/3200个面三角形、599条轨迹线、来源、数值JSON前后不变、仅有意遮罩可null、非法数据零发布、第二文件写失败/路径冲突保留第一文件；无后端时数值命令仍成功、SVG明确不可用。degree200/order200验证未归一化溢出拒绝；无效degree/order、网格、mask、NaN/Inf、零scale/zoom均显式失败。

`r06_illustrations_real_cli`在中文/空格隔离目录调用真实CLI，检查帮助、解析负例、数值溢出前不创建output、三种JSON/SVG发布字节一致、重复RunId不覆盖、自动RunId及原目录不变。每次进程均留argv、SHA、cwd、时间、stdout/stderr/exit。构建与全套CTest之前仍须提交、推送并核实远端SHA；先执行原五项MAT测试，再其余40项。原始资料及全部已有测试保持。

## 剩余范围

本批证据未执行前不能宣称插图通过。R6还需Adaptive真实生成CLI的剩余覆盖；完整科学/L4、病态数值资格、资源与正式验收不因插图实现自动完成，继续按既有边界处理。

## 首轮实测与画幅修正

检查点 `80a1ce2417946f9ced72318f3585f9c1d6ccfbd9` 已提交、正常推送、由GitHub分支API确认SHA后执行。RunId `r6-illustrations-20261117-MepeASNK`，日志 `build/r6-illustrations-20261117-MepeASNK/evidence/`，构建缓存 `build/r6-20261117-KzjFHPDQ/cmake/`，工作目录仓库根，执行者为本轮获授权Codex。Ubuntu24.04.5/Linux7.0.0-43、GCC16.2/C++17、CMake/CTest4.3.4、Eigen3.4、matio1.5.26、zlib1.3、Sophus1.24.6。

2026-11-17 UTC+08:00：配置20:58:06–08、Release增量构建20:58:08–20:59:22，退出均0、无诊断；MAT20:59:22–23（5/5、0.90秒），其余20:59:23–31（40/40、7.57秒），全部45/45通过，原始及其他输入SHA复核退出0。配置使用Release、/usr/bin/g++、BUILD_TESTING、导出编译命令；编译并行4/`-- -k`，先五项MAT正则-R并行2，再-E并行4，均verbose/JUnit/no-tests=error。精确命令/参数、开始结束、stdout/stderr/退出码、JUnit和原输入Git blob/文件SHA均在上述evidence，各stderr为空。固定几何无算法随机种子；测试目录/自动RunId随机身份独立记录。

新独立几何测试492946项、0失败；45组全网格YY/坐标的最大绝对差 `5.73129455005982890725e-09`、最大 `abs(diff)/(1+abs(reference))` 为 `2.18014586343682713417e-10`。绝对差不能单独当作小量附近误差；工程回归使用前述固定缩放规则，不是正式DEC-005资格。真实CLI28次进程全部符合预期，输出在构建目录 `r06_illustrations_cli/插图 中文 空格/c8ae493a7f47e14b278e0149/`；JSON/SVG一致及防覆盖均通过。CTest日志SHA-256：MAT `4b9b9831c458ddc4ae9731a4e8ee58acf8fb0001f118cb3fbf611f530861faca`，其余 `b4c191031fda1711a4b8942069edcdde62ee23ae793a87552fe316fc51c31556`。

**R6-ILLUSTRATION-001：目视发现径向面被SVG画幅裁剪。** 初次测试仅验证面片数量和数值未变，没有断言全部投影点位于画幅。使用预装Node18.19.1/Sharp0.35.5/librsvg2.63.2离线栅格化实际三份SVG（仅QA工具，不是工程依赖，未安装软件），圆/正弦轨迹可见，degree3/order2的径向面上下边缘被裁掉。原SVG/PNG、render.cjs、输入SHA、stdout/stderr/时间/退出码均保留在首轮目录，不覆盖。GitHub all-state Issue/PR检索为空，仓库无已有对应插图问题；不把辅助浏览器超时或REPL模块加载限制当作源码问题。

现仅调整径向面SVG基础像素半径，从290改210，保留源camera_zoom=1.5及全部数值几何。补独立的SVG每三角形三顶点画幅边界检查，另覆盖degree0/order0的完整半径8球面；固定轨迹画幅不变。新源码待下一远程检查点后构建/CTest和目视复核，不因首轮45/45而宣称此修正已验证。

## 最终复测

修正检查点 `8c808be58e25de16da7b3d0c66ce3c0764e74e76` 已提交、正常推送并由远端分支API确认。RunId `r6-illustrations-fit-20261117-mxoyaHiL`，完整日志及栅格化预览在 `build/r6-illustrations-fit-20261117-mxoyaHiL/evidence/`，构建缓存、环境与上轮相同。所有命令/参数、输入身份、种子说明、起止、退出码、stdout/stderr和JUnit均独立保存。

| 操作 | 2026-11-17 UTC+08:00 | 结果 |
|---|---|---|
| CMake配置 | 21:04:01–02 | exit0 |
| Release增量构建 | 21:04:02–21:05:28 | exit0，无诊断 |
| 五项MAT CTest | 21:05:28 | exit0，5/5，real0.78秒 |
| 其余CTest | 21:05:29–34 | exit0，40/40，real5.79秒 |
| 全输入SHA复核 | 结束21:05:34 | exit0，未改变 |

**45/45 CTest通过。** 插图独立几何及SVG边界共 **536225项检查、0失败**，最大绝对/缩放误差与首轮相同；真实CLI仍28次调用通过，产物在构建目录 `r06_illustrations_cli/插图 中文 空格/72e7dff2f4e847628f37feda/`。第二次栅格化exit0，径向面与标题完整可见、不再上下裁剪；圆/正弦SVG哈希与首轮完全相同。实际径向SVG SHA-256为 `bdb720e71678966d3ee08d07e505db520738ff5c258b601cb5d0b0c1c09f9ebc`。R6-ILLUSTRATION-001修复及有限回归闭合，不声称原MATLAB灯光/像素相同，首轮裁剪图保留。

CTest stdout SHA-256：MAT `5c59cab351347214a4ccc4d58d52b3158da1bae9c7fc1e03fa699c06b6b52bd6`，其余 `74f96d787ecda2fb6371319d221b889a12b6b7a49931523ce0a555ada5f5db6a`。各launcher stderr为空，子进程预期失败诊断另在CLI evidence内；不能把这类负例的通过当作科学算法成功。

生产文件为 `include/egomotion/illustrations/illustration.hpp`、`src/illustrations/{detail.hpp,geometry.cpp,data_json.cpp,svg.cpp}`、CLI公共头/解析/装配/main及CMake；测试为 `tests/integration/test_r06_illustrations.cpp`、`test_r06_illustrations_cli.cmake`。来源归档树与所有原始文件未改动；没有新GitHub Issue/PR、没有合并/发布。三份展示入口已实现并取得限定数值/CLI/SVG证据；Adaptive真实生成CLI剩余覆盖、完整科学/L4、跨平台/资源/正式数值验收仍未完成，R6不结案。
