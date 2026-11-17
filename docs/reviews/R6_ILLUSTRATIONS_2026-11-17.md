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
