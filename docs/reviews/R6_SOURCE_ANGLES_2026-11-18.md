# R6：源球面角度中间量

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。对应[PLAN P7.1](../PLAN.md)、REQ-F-SH-001、TC-F-SH-001与[98项复核D2](R6_PLAN_TASK_AUDIT_2026-11-18.md)。所有者已授权源码、构建/CTest及方案文档修改。起点本地与GitHub分支API均为`24aa140d5466f0a630a9a6a6601f967cb837495d`，工作区干净；all-state Issue #1/#2/#3仍开放，无PR。继续既有开发分支，不修改main或原始资料。

## 实现及数值边界

原`help_functions/winkelausvektor.m`仅静态阅读，blob `5bb7ba5832ae849c791df6c67bd49d53b60f250e`。源按`phi=atan2(y,x)`、`theta=acos(z)`返回极角和方位角，角度为弧度。新增低阶叶子`sourceAnglesFromDirection`返回自拥有`SphericalAngles{theta_radians,phi_radians}`，保留计算顺序、+z极轴、+x到+y方位角约定；不归一化、不夹紧z。

支持域沿用已有低阶基的有限单位方向校验及128epsilon范数舍入余量：非有限输入返回`NonFiniteInput`，非单位/范数溢出返回`NotUnitDirection`。即便范数检查允许，z超出[-1,1]仍使实数acos失败，返回既有`NonFiniteResult`，不扩大其定义域。原脚本未做这些显式验证，MATLAB复数/警告/异常传播未取证，不声称所有输入行为等价。两极方位角没有几何唯一值，当前接口保留C++ `atan2`的带符号零/±pi；该平台行为不充作MATLAB证据。

这是计划要求的独立中间角度接口；已有`basisDirect/basisRecurrence`、拟合、重建和旋转仍走原Cartesian路径，未使求解器改用三角运算。新接口为候选、未冻结；没有添加无用途的三类球谐包装、改动跨模块错误合同或默认数值阈值。P7.10职责审查和D1/D3/D4仍须继续，不能凭该接口宣布整体R6完成。

## 验证设计及首次状态

新增`r06_source_spherical_angles`：18个固定方向的独立解析角度（六轴、四象限、3-4-5三角形及(1,2,2)/3），角度重建坐标，并用l≤2含CS的关联Legendre解析三角式逐项核对现有直接/递推九基。参考角常数写在测试表中，不调用被测接口生成；三角重建属于一致性检查，不能代替独立Golden。覆盖8种两极带符号零、负x缝两侧、9种非有限分量、零/非单位/范数溢出、两个超过acos边界1ulp的方向；检查输入不变和返回值所有权。

沿用既有`test_low_order_harmonics`的4096epsilon工程回归余量，未修改DEC-005、验收规则或任何既有测试判据。新增检查不消除三个真实Adaptive入口的acos越界/零旋转失败，也不证明完整实验成功。

首次检查点仅有源码及静态审查；预期SVG ON58、OFF57项，尚未执行不能记为通过。先原子提交、正常推送并核对远端SHA，再分别配置/编译，先MAT7项、后其余全量CTest，保存独立RunId、命令、环境、输入身份、种子来源、stdout/stderr和退出码。未安装依赖、运行MATLAB/Octave、完整科学/L4或性能实验；未冻结、验收、合并或发布。

## 固定源码实测

受测源码 **`cce0bf2c66506e32dd5400e22a6b8d95b5ae6929`**，分支`dev-plan-P0-P1-mat-dictionary-probe`；运行前已正常推送并以GitHub branch API核实SHA、工作区干净。RunId **`r6-source-angles-20261118-ksdFXz5p`**，工作目录为仓库根。

| 配置 | 配置/构建 | MAT优先 | 其余CTest | 起止时间（2026-11-18，UTC+08:00） |
|---|---|---|---|---|
| SVG ON | exit0，无诊断 | 7/7 | 51/51；共58/58 | 02:30:03–02:31:37 |
| SVG OFF | exit0，无诊断 | 7/7 | 50/50；共57/57 | 02:31:46–02:33:25 |

新角度测试两模式均为**1137项检查**、18个固定解析方向，角度/重建/基值比较的最大绝对残差`1.3877787807814457e-16`。工程回归余量仍为既有`4096*epsilon=9.0949470177292824e-13`；残差不代表最终数值资格。原低阶SH、Session策略、真实Session交错、独立单步/KvD、MAT/Writer/CLI/分析等回归均实际执行。

独立读取四份JUnit逐项核对：零failure/error/skip/disabled，每项status=run，与各配置CTest登记逐名一致；两配置唯一差别为`r06_svg_source_windows`。OFF的编译源、链接列表、ELF及静态库检查继续确认没有SVG导出实现。两模式各718项输入SHA后验全部匹配，包含原628文件/582MAT；原树仍为`70ee155131c24a50169346d0ffbaff0c01b1c547`，未改`origin/**`。

环境：Ubuntu24.04.5、Linux7.0.0-43 x86_64、GNU16.2、C++17 Release、CMake/CTest4.3.4、Eigen3.4、matio1.5.26、zlib1.3、Sophus1.24.6；仅此环境有本轮证据。角度测试不使用RNG，其余固定seed来源另存；自动RunId仍使用独立OS随机源。启动命令为`bash build/r6-source-angles-20261118-ksdFXz5p/run.sh cce0bf2c66506e32dd5400e22a6b8d95b5ae6929 ON`及同参数`OFF`。包装器记录完整CMake参数、`--parallel 4`构建、MAT优先/其余CTest命令；复用两独立源外构建缓存，各模式证据存于本RunId下`evidence-ON/`和`evidence-OFF/`，没有覆盖旧日志。

证据包含`environment.txt`、各步骤`.meta/.stdout/.stderr`、四份JUnit、Testing快照、源Git blob、输入SHA前后、随机来源、编译/链接图及`summary.txt`；逐名复核结果在`read-only-junit-audit.json`。随后的追踪文档提交没有重新构建，受测源码仍为上述SHA。

文档静态复核：98个P编号及顺序与PLAN逐项一致；66个TC编号及顺序、45源路径/50声明与变更前相同；TEST_PLAN除顶部事实注记外正文完全相同。11份更新文档的470处相对文件引用均有目标，结果保存于`read-only-document-audit.json`。这不是Bash追踪脚本现场运行或新增CTest证据。

P7.1的独立角度接口及有限输入验证缺口已补；P7.10具体职责、领域不变量、其它装配/记录范围与完整科学实验继续保留。Issue #1/#2/#3未关闭，本轮无新缺陷/Issue或PR。未修改PLAN/TC/AC、阈值、冻结或验收状态；未合并或发布。
