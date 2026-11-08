# 定向审计：函数声明与台账简化（2026-11-08）

固定输入：`fd05af47fdddb8d2f03ac404d509c2ff82bdbad8`。静态只读浏览了原 `origin/**` 下 45 个 .m 的函数声明；其中 44 文件各 1 个，`SHRotate.m` 共有 6 个，合计 **50 个函数定义**。SHRotate 主函数位于第 17 行，内部 M/U/P/V/W 位于第 58/87/91/108/127 行；这些属于递推内部函数而非独立 public API。

`rodrigues.m` 内在 `return` 后还有 Jacobian、随机旋转示例/调试文本，不应自动视为生产主路径。现行 [迁移矩阵](../MIGRATION_MATRIX.md) 只列 45 文件的主职责，缺内部子函数的单独身份（AF-072）。

[审计计划](../AUDIT.md) 与 [整改台账](../AUDIT_PROCESS.md) 各约 15241 / 17034 字符，重复存储历史轮次、动态状态与 SHA；简化目标分别记为 AF-073/074。原始历史证据由 `docs/reviews/` 和 `docs/PROGRESS.md` 保留。

**限制**：只核函数声明与部分源段落，不构成全部算法分支审计、独立 L3、C++/CTest 或正式冻结；不执行 MATLAB/Octave、不修改源 .m/MAT。
