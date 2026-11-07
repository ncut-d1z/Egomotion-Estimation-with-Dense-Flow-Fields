# Ivanic–Ruedenberg 球谐旋转：原论文与纯 Eigen 实现约束（LLD 附件）

> **设计方向已由项目所有者确定；不是生产算法实现，也没有对尚未上传的论文全文做逐式核验。** 1996 原始论文及 1998 勘误在开始生产递推实现前**必须阅读上传的 PDF**。本附录只依据已核实的论文书目信息、ACS 勘误公开说明和既有只读源码 LLD，提出实现/核对清单；不运行 MATLAB/Octave、不编写递推 C++。

## 1. 原始资料

- **Ivanic, J.; Ruedenberg, K. (1996).** *Rotation Matrices for Real Spherical Harmonics. Direct Determination by Recursion.* `J. Phys. Chem. 100(15), 6342–6347`. [DOI 10.1021/jp953350u](https://doi.org/10.1021/jp953350u)；[ACS PDF 入口](https://pubs.acs.org/doi/pdf/10.1021/jp953350u)。ACS 当前标示付费访问，可能需要用户机构权限。
- **Ivanic, J.; Ruedenberg, K. (1998).** 同题 *Addition/Correction*. `J. Phys. Chem. A 102(45), 9099–9100`. [DOI 10.1021/jp9833350](https://doi.org/10.1021/jp9833350)；[ACS PDF 入口](https://pubs.acs.org/doi/pdf/10.1021/jp9833350)。ACS 标示 Free to Read，直接 PDF 自动访问可能返回 403。**不可遗漏勘误**。
- **Don Williamson 源代码参考**：[dwilliamson/SHRotation](https://github.com/dwilliamson/SHRotation) 的 `Source/IvanicSHRotate.cpp/.h`、相邻 `SHRotateMatrix.h`；仅用于识别现有递推设计与比对，不将 `sdla` 列为目标依赖。

**已由 ACS 页面明确可见的勘误**：1998 年文献指出原 1996 年 eqs. 6.7、7.9b、7.9c 的某个负号应改为正号，eq. 7.9a 的一个下标应从 1 改为 -1，表 1 和表 2 也有更正。**由于尚未收到用户上传的原始 PDF，本项目不能声称已逐项确定或复核这些公式的所有实现细节。**

## 2. 禁止 sdla：Eigen 独立改写策略

| 计算/数据边界 | 确定方案 | 不得出现 |
|---|---|---|
| 输入物理旋转 | `Eigen::Matrix3d` 或 domain 的 `Matrix3`；保持从物理矩阵到原 MATLAB 实球谐基底的轴/m 顺序映射 | `sdla::Matrix`、`sdla/Matrix.h` |
| 一阶块 | `Eigen::Matrix3d` 对应 (R_3)，与源 MATLAB 的 `(-1,0,+1)` 排列一致 | 将物理 R 原封不动当成 R3 |
| 二阶块 | `Eigen::Matrix<double,5,5>` 对应 (R_5)，下标映射 `row=m+2, col=n+2`；递推系数按**1998 勘误修正后的数学规则**实现 | 仅拷贝 Williamson 原 `sdla` API；未校对勘误直接翻译 |
| 零阶块 | R0=1；统一依照 domain `HarmonicCoefficients` 的 3×5 九有效项布局 | 擅自扩大到 l>2 或修改系数顺序 |
| 异常/返回 | 公共 `Result<...,NumericalErrorCode>` 的轻量封装、`ErrorDetail`；检查非有限、非正交/未定义输入 | `sdla/Exception.h` 或 sdla 的异常类型 |
| target/CMake | `egomotion_harmonics` 明确依赖 `egomotion_domain`、`egomotion_contracts` 和 Eigen；私有适配代码自包含 | `sdla` 的 include path、linked target、vendor 子模块或动态依赖 |

**Eigen 是真正替代，不是上游 sdla→Eigen→sdla 的兼容包装**。如果引用 Don Williamson C++ 算法片段，保留相应上游告知并将具体差异记录为正常工程溯源；**一般版权审核不再是开发启动门禁**。项目原创代码采用仓库根 [MIT License](../../LICENSE)，但不声称此许可重新授予他人的独立作品。

## 3. 论文上传后、实现前的技术核对任务

1. 阅读 **1996 原文全文和 1998 勘误全文**，为公式 6.7、7.9a/b/c 及表 1/2 建立“原文→勘误→Eigen 代码”的对应表；不得仅根据第三方实现猜测更正。
2. 阅读本仓库只读 `SHRotate.m`、`transfor_koeff.m` 与 `kugelflfunc2.m` 中定义的实基顺序、Condon–Shortley 相位、主动/被动旋转、(R)/(R^\top) 的方向性；明确与论文及 Williamson 的差异。
3. 为 `R0=1`、`R3` 全 9、`R5` 全 25 元素构建 **独立 L3** 数值样本，覆盖单位阵、绕 x/y/z 的 90° 和一般角、复合旋转及其逆；比较九个系数 before/after、正交性与行列式，不把单纯往返成功当充分条件。
4. 实现时严格只用 Eigen，自拥有固定尺寸矩阵；确认有限输入和正交性策略、错误码与 `HarmonicRotationBlocks` 的值生命周期，避免隐式的全局临时/堆内存。
5. 在后续**明确授权**的 C++/CTest 阶段检查真正的 target 依赖闭包不含 sdla、MATLAB、HDF5 等无关新增依赖，并在 Git 提交证据中记下变更文件、固定输入 SHA、失败数与结果。
6. 保留用户已选定的第一轮宽松筛查 `atol=0.01`、`rtol=0.1`，但 R3/R5 元素、相位、正交性、球谐旋转方向等另列严格判据；最终 DEC-005 阈值以实际数据确定。

**当前入口状态**：用户尚未上传论文全文，因此只准备资料和候选合同，不启动 R5/Eigen 递推生产编写；也不执行 MATLAB 或强制采集 L1。
