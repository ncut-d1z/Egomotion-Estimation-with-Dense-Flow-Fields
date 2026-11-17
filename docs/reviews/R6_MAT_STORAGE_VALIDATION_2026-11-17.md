# R6 原始 MAT 存储身份与独立 Reader 对照

**[HUMAN-EXECUTE/APPROVE]** 用户已授权源码、构建/CTest与方案文档。本批代码/候选参考尚未编译测试；只读诊断已实际执行。沿用 `dev-plan-P0-P1-mat-dictionary-probe`，不执行MATLAB，不修改原始文件，不宣布Golden/DEC冻结。

## 发现与证据

上一批23/23 CTest通过后，依据 `Mat_VarRead` 会填充解码类型的实际边界，对原始存储标签做独立复核。只读诊断脚本在源码 `1e15b9c32342c96e2e553325a8870c0a276b31a4` 的干净已推送检查点执行，Node18.19.1；RunId `r6-io-faults-20261117-sm6Ovszn` 的 `evidence/storage-tags.meta/.json/.stderr` 保存命令、时间和退出码。归档输入身份来自同RunId的 `inputs.sha256`。

实际结果：582个MAT v5均为zlib单变量、little-endian、MATLAB class=6（double），但502个实部存储tag=9（miDOUBLE），80个tag=5（miINT32）。后者全是两组Bias各40目录中的 `xwerte.mat`。因此旧“582个物理data_type都为double”的说法不成立；旧探针报告的是matio解码后的内存类型。既有元数据inspect仍如实返回未知类型，不为修正文档而偷读数值。

`origin` 固定Git树为 `70ee155131c24a50169346d0ffbaff0c01b1c547`。从独立诊断输出生成 [候选存储/SHA清单](../../verification/mat/archive_storage_identity.v1.tsv)，Node crypto的全部582个文件哈希与已有GNU sha256sum结果完全一致。脚本保存为 [独立只读检查器](../../verification/mat/inspect_storage_tags.cjs)，不进入生产或CTest运行依赖。

## 实现及范围

新增 `r06_mat_independent_storage_values`，关联P1.4/P1.8、TC-F-MAT-001、TC-D-MAT-001/002、TC-I-MAT-001：独立C++/zlib解析原始标签及实部值，再核对生产Reader的名字、形状、class/内存类型、完整列主序binary64值和独立固定文件SHA。固定档案的数值是读取/解包问题，使用逐位比较，不新增科学算法容差。测试总登记由23变24。

参考读取路径不调用matio、不复用生产Reader或字典方法；支持范围严格限于这些原始档案的实际结构。miINT32中的每个整数均可精确恢复为逻辑double。与源MATLAB实验计算的比较仍是另一项任务，不能把“同一保存文件的独立解码一致”称作Runner复现实验。

同步纠正README、MAT LLD、SRS中**来源事实**及字典生成器说明，保留历史已生成 `MAT_DATA_SCHEMA.md` 原样。没有新增/删除REQ/TC/AC，不改变double计算精度，不批准未来MAT类型扩展；正式物理存储允许集合与拒绝策略若涉及合同变更，仍按现有流程评审。

## 尚未完成

生产Reader尚未独立检验原始packed tag，不能仅凭Mat_VarRead的内存dtype断言严格拒绝所有异型载荷。后续需要单独处理真实原档的整数打包、single/complex/logical/struct/cell/sparse/高维负例和截断/损坏；不能为收紧检查而拒绝这80个原始文件。独立L3/L4、完整Runner/八轮结果及正式验收仍未完成。

## 执行结果

待本批提交/推送/远端核实后执行构建、MAT测试与其余CTest，记录真实结果；当前只有只读标签和SHA交叉核查，不能将新增CTest标为通过。
