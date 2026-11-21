# 合成 MAT v7.3 拒绝夹具

`[HUMAN-EXECUTE/APPROVE]` 测试资料，非算法Golden、非原始论文资料。`synthetic_v73_double.mat`由本仓库原创 [生成器](../../tests/support/generate_mat73_fixture.cpp) 在源码检查点`f1fb7d377a2967304a5f718d3623bfab3ceb2604`使用预装matio1.5.26/HDF5生成；项目根MIT适用于该原创合成测试资料，不重新许可上游库。

- 3408字节；SHA-256：`f6b703ff389c19c8a871009f299ffeb5fcb004273dc7f2b290479cf35e29b2a5`。
- `MAT_FT_MAT73`，一个`value`变量，2×3实数非logical double，列主序binary64值`[1,-0.0,-3.5,4,0.25,6]`。MAT版本/端序头在字节124…127为`00 02 49 4d`，HDF5签名在偏移512。
- 生成后用独立于生产Reader的直接matio重新打开，版本、目录、rank/dims/class/type/flags/nbytes和六值逐位比较全部通过；未运行MATLAB/Octave。
- 用途：首版v5-only数值Reader的版本拒绝与后端调用边界。不要以这个文件声称v7.3数值支持或MATLAB算法等价。测试仅读取本文件，所有头变体/输出仅保存到独立build沙箱。

普通构建/CTest读取已固定的夹具，不构建生成器、不要求HDF5创建能力。若要重建，先遵守AGENTS远端检查点，在新沙箱显式构建`r06_mat73_fixture_generator`并传入不存在的输出文件路径；需要预装且支持HDF5的matio。HDF5对象元数据可能随库版本改变，重新生成的字节须重新核验身份，不能静默覆盖本夹具或更新期望摘要。

生成RunId为`r6-mat73-fixture-20261118-9lQv0oaJ`，完整命令、工具链、stdout/stderr/退出码、原始数据不变检查保存在同名build目录的`evidence/`；汇总见 [R6专项记录](../../docs/reviews/R6_MAT73_REJECTION_2026-11-18.md)。
