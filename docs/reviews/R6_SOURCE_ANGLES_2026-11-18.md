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
