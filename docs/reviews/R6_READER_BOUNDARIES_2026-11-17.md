# R6 Reader 合成边界验证

**[HUMAN-EXECUTE/APPROVE] 源码准备，尚未编译/运行。** 接续已实测24/24的源码 `68a5f684cc9fb45ee33a7b87b70525ac0510c8fa`；用户已授权生产代码、构建/CTest和文档。运行前仍提交、推送并核实远端检查点。

新增 `tests/integration/test_r06_reader_negative.cpp` 和 `r06_mat_reader_boundaries`，关联TC-F-MAT-001/002/003、TC-D-MAT-001/002及既有MAT错误合同。CTest登记25项，新增项当前没有通过证据。

在构建目录内中文/空格路径的独立RunId沙箱，使用现有matio生成有明确class/type/dims的合成文件。普通和zlib两种编码分别覆盖真实double含负零、single/int32/logical/complex/char/cell/struct/sparse类型拒绝、三维与零尺寸拒绝；inspect仍须保留合法不支持类型的结构。两种业务读取接口必须返回明确类型/形状错误及源路径/变量/阶段；成功double逐位比较。另覆盖MAT v4、缺文件/变量/NUL名称、两变量选择歧义，以及由本测试新文件派生的载荷截断、zlib校验和损坏、短头。前后SHA核对，不修改或复制原始MAT。

每例打印PASS/FAIL并汇总失败数，单例断言失败后继续收集其它场景；不改变数值门槛，不自动删除失败产物。生成器错误单独报告。与原始归档的独立L2 Reader对照是不同证据，本批不宣称覆盖全部任意损坏字节、压缩炸弹、并发替换或所有资源耗尽情况。

静态复查了matio1.5.26的[公开接口](https://github.com/tbeu/matio/blob/v1.5.26/src/mat.c#L2618)：`Mat_VarRead` 已检查底层ReadData返回值，不能未经证据把“改用Mat_VarReadDataAll”当成解决损坏输入的修复。实际失败须先保存、定位；原始存储tag拒绝仍是独立未完成项，不拿解码dtype作为物理标签证明。

## 执行结果

待本批检查点推送后执行；当前24/24记录仍是上一批事实。
