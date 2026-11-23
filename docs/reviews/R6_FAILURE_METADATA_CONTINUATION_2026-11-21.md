# R6 失败记录退出码与元数据输入接续（2026-11-21）

`[HUMAN-EXECUTE/APPROVE]`。按所有者“继续代码实现，推迟构建、测试”，从 Draft [PR #6](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/pull/6) 的 `d0ce4c4ed48b01806c9058d6767051c5b1e32f6c` 继续。前批见 [完成状态与子进程回收记录](R6_MANIFEST_PROCESS_CONTINUATION_2026-11-21.md)。**本批仅生产源码、必要回归源码/CMake 登记和状态文档；未配置、未编译、未做语法编译、未执行测试、CLI、MAT I/O 或数值实验。**

## 1. 失败 Manifest 未确认时，最终退出码必须反映输出错误

[runExperiment](../../src/cli/application.cpp) 在实验计算失败后调用 `publishFailureRecord()`。若 Manifest 的发布或持久化确认也失败，旧实现会输出原计算诊断（例如 `SolverFailed / exit 5`）以及独立的 `OutputFailed / exit 6`，但仍将原计算码5传给 `deliverRecorded()`。只要 stderr 和 `execution.json` 保存成功，最终实际返回码及结束记录仍为5。

这与既有输出/审计错误返回6的合同不一致。旧流回归的 `secondDiagnostic` 同时使第二条诊断写入失败，其最终6来自 stderr 故障，不能证明 Manifest 自身故障已正确影响返回码。以上为静态路径分析，**本轮没有执行双故障反例**。

本批仅把最终交付码设为 `status.hasValue() ? code : 6`。原计算错误、原退出码及完整原因链不改写，Manifest 失败保留为另一条独立事件；`executionRecord()` 使用同一个 `delivery.exit_code`，使成功保存的结束记录与实际返回值一致。Manifest 确认成功的计算失败继续保留原计算码。

[test_r06_execution_records.cpp](../../tests/integration/test_r06_execution_records.cpp) 复用 `r06_execution_record_contract` 的 linkat/fsync 包装，新增 **2 类故障 × 5 个实验入口 = 10 种调用场景的源码**：

- 发布链接失败：最终 `manifest.json` 不存在；实际返回和 execution 记录均为6。
- 最终文件已发布、目录 fsync 失败：保留已发布但未确认的失败证据；实际返回和 execution 记录均为6。

两类均要求 stderr 交付成功，原计算诊断5在前、独立输出诊断6在后，保留真实 RunId、目标名称以及 `WriteFailed` / `ReadbackMismatch` 原因；这样不会再由另一个通道故障掩盖目标问题。**10 是准备的源码场景数，不是执行或通过数。**

## 2. 普通元数据读取也须检查并固定普通文件身份

[MatResultReader::openReadOnly](../../src/io/mat_result_reader.cpp) 原先仅在数值读取或注入观察器时，以非阻塞只读方式打开并确认普通文件；普通 `inspect()` / `scanFile(MetadataOnly)` 直接将原路径传给 matio。

静态依据：matio **v1.5.26** 的 [src/mat.c](https://github.com/tbeu/matio/blob/v1.5.26/src/mat.c)，blob `a3e6008ff74eb9a8388a6572db854a9a9a91bd7d`，`Mat_Open()` 直接 `fopen(matname,"rb")`；Linux [fifo(7)](https://man7.org/linux/man-pages/man7/fifo.7.html) 说明普通打开会等待另一端。因此，无写端命名管道可使元数据入口在后端打开时阻塞，尚未到达预期的普通文件拒绝。**本轮没有创建或运行该 FIFO 反例。**

现在所有入口先 `open(O_RDONLY | O_CLOEXEC | O_NONBLOCK)` 并 `fstat`，非普通文件在调用 matio 前返回现有 `InvalidMatHeader @ Read`；缺失/权限错误沿用原分类，诊断继续携带用户请求路径和已知变量。无需快照的元数据路径将源 FD 移入 `MatInput` 的私有 RAII 成员，后端经 `/proc/self/fd/N` 打开同一文件身份；成员销毁顺序确保先 Mat_Close，再释放源 FD。即使原路径被替换，也不会回退到新路径对象。

普通元数据路径不新增全文件封存、哈希或数值解码；它固定文件身份，**不声称文件内容不能被原地修改**。数值读取和带观察器的读取继续原 sealed 快照，原 v7.3 数值拒绝保持。Linux [proc_pid_fd(5)](https://man7.org/linux/man-pages/man5/proc_pid_fd.5.html) 给出 FD 路径语义。matio 对 v7.3 继续以同一名称调用 H5Fopen；HDF5 **hdf5-1_10_10** 的 [H5F__build_actual_name](https://github.com/HDFGroup/hdf5/blob/hdf5-1_10_10/src/H5Fint.c)，blob `d7382f260e0ecfacf6f574ec8c6c94480bb1f7d0`，对符号链接核对 stat/fstat 身份并解析实际名称。这里保留已命名磁盘文件 FD，区别于既有匿名快照的 v7.3 限制；后端能力与并发文件系统异常仍须后续运行验证，不承诺所有 HDF5 构建可用。

回归源码复用两个既有目标：

- [test_r06_mat_snapshot.cpp](../../tests/integration/test_r06_mat_snapshot.cpp) 的 `r06_mat_snapshot_identity` 新增6种元数据调用：FIFO/目录 × 两入口共4种提前拒绝，Mat_Open 前替换原路径 × 两入口共2种仍读原 inode。检查原路径诊断、FD 无泄漏，以及零 snapshot/零 Mat_VarRead；后端打开包装让旧实现确定性触发断言，不让 FIFO 回归本身等待写端。
- [test_r06_mat73.cpp](../../tests/integration/test_r06_mat73.cpp) 的 `r06_mat73_version_rejection` 新增2种元数据入口对照，使用已有真实 v7.3 合成夹具：直接后端能取得元数据时，Reader 也须取得；后端不可用时如实记录能力并返回错误，不把缺少后端写成成功读取。普通构建仍不要求安装或创建 HDF5 后端。

未新增测试目标；[CMakeLists.txt](../../CMakeLists.txt) 仅为既有 snapshot 目标追加 Mat_Open 链接包装。**6与2均为准备的源码场景数，这些回归未编译、未执行。**

## 3. 限定静态复核与延期

本轮另读 Session、系数策略、指标集成和聚合证据路径，未确认新的确定缺陷；候选提交/失败回滚、纯指标缺测继续、固定八轮分母及末轮 ratio 保持。没有将已存在且由旧回归固定的 Bias 范数错误分类作为本批新政策修改，也没有改动数值公式或通过阈值。

两项生产修复与对应回归源码经独立静态复核。实际检查限于源码交叉审阅、上游源码/系统语义核对、Git 状态/差异及 `git diff --check`。原始 `origin/**` 树保持 `70ee155131c24a50169346d0ffbaff0c01b1c547`；历史分支、工作树与失败证据保留，没有运行仓库验证脚本或安装依赖。

DEC-R6-01～04 与方案 B 保持，**没有新增需要所有者立即决定或干预的事项**。构建/测试、完整科学实验、更广独立 L3 与必要 L4、数值/性能及跨环境资格、正式冻结/验收和发布继续延期；旧固定 SHA 的通过记录不外推到本批。当前已列明功能已有源码实现，仍须修复确认的具体错误并在恢复验证授权后取得证据；PR #6 保持 Draft，R6 总体验收仍未完成。
