# 实球谐及旋转详细设计草案

> **状态：待 P0.4/P0.8、P7 冻结。** 第三方来源/版权未经核查前，不直接复制 rodrigues.m 或 SHRotate.m 的实现。来源函数 winkelausvektor.m、kugelflfunc2.m、koeff_aus_near.m、near_aus_koeff.m、SHRotate.m、transfor_koeff.m。

## 数学表示与原源码约定

- 单位方向向量转球坐标：theta = acos(z)、phi = atan2(y,x)。必须保持 MATLAB 方向和弧度约定。
- kugelflfunc2 使用 MATLAB legendre、相应归一化系数；m<0 用 sin(|m|phi)，m>0 用 cos(m phi)，m=0 不带方位角因子。关联勒让德函数的符号约定/是否有相位必须通过黄金值核对，不能套用其他库默认规范。
- HarmonicCoefficients 以 3×5 矩阵存放 l≤2 的系数：零阶 1 个，一阶 3 个，二阶 5 个（合计 9 个有效项）。原脚本初值是系数位置 (1,1)=1；其余初始化为 0。
- near_aus_koeff 在给定方向上逐项计算加权和。koeff_aus_near 以各自基函数的平方范数单独归一，不擅自替换成一次联合最小二乘。
- transfor_koeff 保持零阶项，以 SHRotate(r,2) 产生的一阶 3×3、二阶 5×5 旋转块分别变换对应系数。旋转方向、列/行系数约定应有单独对照。

## 分工与候选合同

| 单元 | 责任 | 约束 |
|---|---|---|
| RealSphericalHarmonics | 从方向计算 real Y_lm 与近度重建 | 无状态、确定性、固定顺序 |
| HarmonicFitter | 从离散近度求相应 3×5 系数 | 分母为零时明示异常 |
| HarmonicRotator | 依据 Rodrigues 旋转矩阵更新 l=1,2 系数 | 保留原 SHRotate 编码/顺序语义 |
| HarmonicCoefficients | 拥有 3×5 值矩阵与有效项合同 | 不持有 MAT 句柄和实验状态 |

## 数值验证与版权门禁

- TC-F-SH-001：固定三维方向，分别验证 l=0/1/2 和 m 正、负、零的原 MATLAB 基函数值；核对近度重建与拟合。
- TC-F-SH-002：对绕 x/y/z 的不同已知旋转及复合旋转，逐块核对 3×3、5×5 的系数变换；测试旋转/逆旋转恢复近度。
- TC-D-SOURCE-002、TC-NF-COPYRIGHT-001：审查 SHRotate.m 和 rodrigues.m 中外部代码来源、通知和可移植范围；无授权结论时保留阻塞，不新增 LICENSE。
- 正式容差、Golden 数据与 C++17 头文件签名待 P0/P7 审查，不得因写出本计划就宣称已通过。
