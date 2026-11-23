# 原始 MAT 的独立存储核查

本目录是 **候选参考，未经人类冻结/验收**。原始文件仍在 `origin/`，不复制或改写它们，不执行 MATLAB/Octave。

- `archive_storage_identity.v1.tsv`：582个文件的SHA-256、原始实部存储标签、维度、变量名和归档相对路径。来源树 `70ee155131c24a50169346d0ffbaff0c01b1c547`；SHA由Node内置crypto计算，并与先前运行的GNU `sha256sum` 582/582独立核对一致。实际标签为502个miDOUBLE和80个miINT32，MATLAB class均为double。
- `inspect_storage_tags.cjs`：仅审计时使用的独立只读解析器，依赖Node自带fs/zlib/crypto；本次实际Node18.19.1。它只向stdout输出结构与哈希，不写MAT。**生产程序、CMake和CTest均不依赖Node**，无需为运行C++项目安装它。
- `tests/integration/test_r06_mat_independent.cpp`：另用C++/zlib直接解包原始文件并解释标签/实数值，不经matio生成预期值；再与生产Reader逐元素逐位比较，并检查固定SHA清单。该验证覆盖Reader对这些归档的解码，不等于求解器或Runner重现历史实验。

独立解析参考 [MathWorks MAT-File Format](https://www.mathworks.com/help/pdf_doc/matlab/matfile_format.pdf) 的Level 5标签、small data element、压缩和class/real-part布局。仅实现本仓库实际用到的little-endian单变量zlib二维mxDOUBLE，以及其miDOUBLE/miINT32实部；不宣称通用MAT支持。miINT32到逻辑double的整数恢复是精确的，不是single精度升级。

可复查命令（stdout另存新的本地证据目录，不覆盖历史结果）：

```bash
node verification/mat/inspect_storage_tags.cjs \
  origin/egomotion_estimation_dense_flow_fields_matlab_scripts
```

原始诊断、提交、测试状态和限制见 [R6存储验证记录](../../docs/reviews/R6_MAT_STORAGE_VALIDATION_2026-11-17.md)。严格生产Reader对物理标签/损坏输入的拒绝仍需独立负例与实现，不能拿这份正常归档对照替代。
