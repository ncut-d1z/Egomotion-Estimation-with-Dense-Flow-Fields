# R6 Reader 合成边界验证

**[HUMAN-EXECUTE/APPROVE] 已修复并通过25/25 CTest。** 首次负例运行发现3个真实失败，修正源码 `dffddf3b6a62b906a5b20eba365a2087e9c86662` 已重新编译并执行全部CTest。接续已实测24/24的源码 `68a5f684cc9fb45ee33a7b87b70525ac0510c8fa`；用户已授权生产代码、构建/CTest和文档。两次运行前均提交、正常推送并核实远端检查点。

新增 `tests/integration/test_r06_reader_negative.cpp` 和 `r06_mat_reader_boundaries`，关联TC-F-MAT-001/002/003、TC-D-MAT-001/002及既有MAT错误合同。CTest登记25项；首次29个边界场景中26通过、3失败。

在构建目录内中文/空格路径的独立RunId沙箱，使用现有matio生成有明确class/type/dims的合成文件。普通和zlib两种编码分别覆盖真实double含负零、single/int32/logical/complex/char/cell/struct/sparse类型拒绝、三维与零尺寸拒绝；inspect仍须保留合法不支持类型的结构。两种业务读取接口必须返回明确类型/形状错误及源路径/变量/阶段；成功double逐位比较。另覆盖MAT v4、缺文件/变量/NUL名称、两变量选择歧义，以及由本测试新文件派生的载荷截断、zlib校验和损坏、短头。前后SHA核对，不修改或复制原始MAT。

每例打印PASS/FAIL并汇总失败数，单例断言失败后继续收集其它场景；不改变数值门槛，不自动删除失败产物。生成器错误单独报告。与原始归档的独立L2 Reader对照是不同证据，本批不宣称覆盖全部任意损坏字节、压缩炸弹、并发替换或所有资源耗尽情况。

静态复查了matio1.5.26的[公开接口](https://github.com/tbeu/matio/blob/v1.5.26/src/mat.c#L2618)：`Mat_VarRead` 已检查底层ReadData返回值，不能未经证据把“改用Mat_VarReadDataAll”当成解决损坏输入的修复。实际失败须先保存、定位；原始存储tag拒绝仍是独立未完成项，不拿解码dtype作为物理标签证明。

## 执行结果

首次源码 `05a46c6c172fa59249b1ccdfe518e704094bd578` 已提交推送并核实，RunId `r6-reader-boundaries-20261117-EqWCdlwB`。配置/构建exit0且无编译诊断；2026-11-17 16:04:54（UTC+08:00）MAT4项中3通过、Reader边界失败，CTest exit8。按先MAT后其它测试的顺序停止，未执行其余21项。环境及全部日志位于 `build/r6-reader-boundaries-20261117-EqWCdlwB/evidence/`；日志SHA-256 `cc71528065dc529c1c0616d70d09f4338b2c329ff61453b1a083fb4a461ff695`。原始输入及测试/参考前后SHA核对exit0。

### 缺陷 R6-MAT-001：损坏载荷被当作读取成功

受影响模块 `MatResultReader`。真实结果：`plain-truncated`、`zlib-truncated`、`zlib-checksum` 均被named Reader接受；预期必须非成功且保留来源。其它26场景通过。失败合成文件位于构建目录的 `r06_reader_boundaries/中文 空格/output/run_d424b16513767c6a0f4e2d4526b36878/`，不删除。复现命令为 `ctest --test-dir <build-dir> -C Release --verbose -R '^r06_mat_reader_boundaries$'`，相同源码与matio1.5.26。

已保存失败后检索GitHub全部open/closed Issue（返回空）及仓库已知记录；旧文档仅注明待测，未有同缺陷条目。本条为本地缺陷索引，未创建/关闭远端Issue。静态依据：[matio numeric读取](https://github.com/tbeu/matio/blob/v1.5.26/src/mat5.c#L2746)在固定元素读取后返回错误状态，不能据其指针/nbytes保证顶层长度或完整压缩尾部已验证；本项目缺少这层检查。没有改写第三方代码。

修正：新增私有 `mat_container_validation.{hpp,cpp}`，在named、compatible及字典完整解码前核对MAT v5顶层边界、实际文件长度和完整zlib流终止/Adler-32/声明长度。流式解压使用固定大小缓冲区，不把全部解压内容另行驻留；业务值仍由matio读取。固定源fd后通过 `/proc/self/fd` 让matio打开同一inode，避免前置校验与实际读取因路径替换读到不同文件；不承诺外部进程原地写同一inode时的快照一致性。metadata-only inspect不执行新增载荷校验，仍不读取数值。

保留3个失败断言并增加完整字典扫描拒绝检查；不改数值阈值。修正仅针对已定位容器完整性，尚未证明所有嵌套标签/packed-type/任意损坏输入的严格拒绝。完整读取会多做一次流式校验，性能预算未验收。

### 修复后全量复测

RunId `r6-reader-fix-20261117-USlbrReS`，证据目录 `build/r6-reader-fix-20261117-USlbrReS/evidence/`。工作目录为仓库根，复用 `build/r6-20261117-KzjFHPDQ/cmake/` 并重新配置源码SHA，属于增量构建。Ubuntu24.04.5/GCC16.2/CMake4.3.4/Eigen3.4/Sophus1.24.6/matio1.5.26/zlib1.3，C++17 Release，无安装；与前批相同环境。合成数据固定，无算法随机抽样；新沙箱RunId由独立OS随机源创建。

配置参数 `-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`，构建 `cmake --build <build-dir> --parallel 4 -- -k`。CTest使用 `-C Release --verbose --no-tests=error --output-junit <evidence>.xml`，先 `--parallel 2 -R '^(r02_mat_reader|r06_historical_mat_readonly_schema|r06_mat_independent_storage_values|r06_mat_reader_boundaries)$'`，再 `--parallel 4 -E` 同一表达式；无跳过登记项。

| 步骤 | 开始/结束（2026-11-17 UTC+08:00） | 退出码 | 结果 |
|---|---|---:|---|
| configure | 16:11:17 / 16:11:20 | 0 | 源码SHA正确 |
| build | 16:11:20 / 16:12:52 | 0 | 全目标成功，无编译诊断 |
| MAT CTest | 16:12:52 / 16:12:53 | 0 | 4/4 Passed，0.97秒 |
| 其余 CTest | 16:12:53 / 16:12:54 | 0 | 21/21 Passed，0.79秒 |
| 输入身份 | 16:12:54 完成 | 0 | 原始数据及受控测试/参考SHA全部不变 |

`Reader boundary scenarios=29 failed=0`；3个原失败场景均PASS，named/compatible及完整字典扫描拒绝损坏文件。原始582个MAT/33,340值独立对照仍精确一致，80个miINT32打包double原文件正常读取，发布故障/Writer回读等既有测试全部通过。全部步骤stderr为空。MAT日志SHA-256 `1b95bd2acf7ffcec6e20945536a67f23f0a159afba3169d0ac59c01be5ba945a`，其余CTest日志SHA-256 `c3a0e481aa798fdbdaeb570abbb663a393dd804ea6c083cf514f3aca90e8357e`。

本地缺陷R6-MAT-001已取得修复复测证据；没有自动关闭远端Issue、合并PR、冻结或验收签字。R6后续仍包括其它raw tag/恶意尺寸负例、完整Runner/八次聚合、剩余独立L3/L4以及正式资格复核。
