# R6 MAT封存读取与分析输入摘要

任务标签 `[HUMAN-EXECUTE/APPROVE]`；落实 `TC-F-MAT-001/003`、`TC-I-MAT-001`、`TC-I-ANALYSIS-001`、`TC-NF-SAFETY-001`、`TC-DC-IO-001` 的既有来源/生命周期义务。源码、构建/CTest和文档使用本轮明确授权；原始MAT只读，合成改写只发生在独立CTest沙箱，不运行MATLAB或统计实验。

## 现状与实现边界

检查对象 `5863433ebe11a99e9e8456875b34f4f702c88557`，旧受测代码 `9eb831531ce04f2bfab8bf813011c36a451ee409`。旧Reader固定源fd后分别做容器校验与matio解码；可防路径替换，但不能阻止同inode原地改写。`DenseDoubleArray`没有MAT字节身份，Analyzer只能输出 `mat_sha256=null`。这是 [MAT LLD §7](../design/mat_io.md) 和 [分析LLD §6](../design/analysis_plotting.md) 已明确保留的实现缺口，不能把测试旁路摘要当成生产读取来源。

本轮完整数值读取（named、compatible、VerifyDecoded）先打开只读普通文件，检查fstat，流式复制到Linux匿名memfd；复制前后核dev/ino/size/mtime/ctime，不一致则明确失败。随后加 `F_SEAL_WRITE/GROW/SHRINK/SEAL` 并核实，再计算SHA-256、校验原MAT容器、让matio从同一封存副本解码。副本及matio句柄均RAII关闭，不写原文件或持久临时目录。元数据inspect保持原方式，不复制/摘要/解码数值。

**保证范围**：摘要对应实际校验和解码的那份不可改字节。复制期间可观测到的改动明确拒绝；完成捕获后原路径替换或原inode改写不会改变解码值/摘要。不是整个目录的原子事务快照，不保证每个独立输入在同一时间点采集，也不承诺对能够伪造文件元数据的对抗者证明原文件历史上的单一时刻；原始归档仍通过独立固定SHA清单额外核对。每次读取增加一份原MAT大小的匿名字节存储和摘要遍历，未据此宣布资源/性能资格。

`DenseDoubleArray::source_mat_sha256` 是新增的可选值字段（候选C++接口、未冻结）；不暴露fd/matio，也不新增Reader方法。纯合成数组或未提供身份的自定义Reader保持空。Analyzer从实际数组复制身份到既有 `AnalysisInput::mat_sha256`；Bias跨run贡献、最后x、Adaptive误差/ratio分段来源均按实际载荷绑定。没有新增JSON键、改变数值算法/比较阈值或替换.m源码Git blob。

## 验证准备（执行前记录）

- 新 `r06_mat_snapshot_identity`：确定性链接层注入，named/compatible各15种场景，共30次完整读取，另1次元数据检查。覆盖路径替换、同inode改写、复制期间变化拒绝、memfd失败、读/写/封存/摘要失败、EINTR/短读写重试；成功副本实际pwrite/ftruncate必须EPERM；所有返回后fd数量不变。所有改动对象为新建合成MAT，参考A/B另存并保留。
- 扩展582份独立L2逐值测试：读取返回的SHA必须与既有独立固定清单一致；原33340值逐位断言保留。
- 历史Bias全部曲线输入摘要、真实历史分析CLI摘要均要求实际值，不再允许生产Reader返回null通过。
- 有限Adaptive分析仍用独立多项式数值oracle，SHA由CMake独立 `file(SHA256)`填充期望对象，再完整比较32条曲线JSON；纯内存替身未知摘要仍有原空值断言。

CTest登记48项不等于通过；先原子提交、正常推送、核实远端，再先6项MAT（含新快照）后其余42项执行。保存精确命令、工具链、RunId/SHA/输入身份、起止时间、退出码/stdout/stderr/JUnit；不覆盖旧失败日志。R6完整科学成功、45源现行映射、必要L4、资源与正式数值/接口验收仍未完成。

## 实测结果

代码检查点 `130a82ba0090e186fcf43bc28bb98c282e68c96c` 已提交、正常推送并经GitHub分支API核实。RunId `r6-mat-snapshot-20261117-wfxpgjMh`；日志 `build/r6-mat-snapshot-20261117-wfxpgjMh/evidence/`。工作目录仓库根、源外构建目录 `build/r6-20261117-KzjFHPDQ/cmake/`；实际Ubuntu24.04.5/Linux7.0.0-43、GCC16.2.0/C++17、CMake/CTest4.3.4、Eigen3.4、matio1.5.26、zlib1.3、Sophus1.24.6。Release并行4构建；先6项MAT并行2，其余42项并行4。完整命令/参数、环境、源Git blob/输入SHA、随机种子来源、stdout/stderr/退出码和JUnit保留。

| 操作 | 2026-11-17 UTC+08:00 | 实际结果 |
|---|---|---|
| CMake配置 | 22:03:22–24 | exit0 |
| 增量构建（公共值布局依赖重编译） | 22:03:25–22:06:59 | exit0，无诊断 |
| 六项MAT CTest | 22:06:59–22:07:00 | exit0，6/6，real1.22秒 |
| 其余CTest | 22:07:01–06 | exit0，42/42，real5.39秒 |
| 输入摘要复核 | 结束22:07:06 | exit0，原始资料未改变 |

**48/48 CTest通过。** 快照用例256项检查、30次named/compatible读取、1次元数据检查均通过；包括14次预期成功（普通/EINTR/短读写/路径替换/原inode改写）和16次预期拒绝。每次成功都验证封存副本拒绝pwrite/ftruncate，所有案例返回后fd数量不变。证据目录 `build/r6-20261117-KzjFHPDQ/cmake/r06_mat_snapshot/snapshot-UBq5Xo/`，参考A/B和各修改后的合成源均保留。这是确定性边界注入，不声称穷尽任意调度。

582份归档的读取摘要与独立固定清单全部一致，33340个double仍逐位一致；历史Bias 400份实际贡献输入、986条曲线来源摘要检查通过，原3323项曲线检查/208对x-y最大差仍约2.64e-14。有限Comparison/UpdateRates实际CLI的32曲线、5578对x-y及3460个平滑全量值与独立多项式参考/独立CMake文件摘要的完整JSON相等。其证据目录 `build/r6-20261117-KzjFHPDQ/cmake/r06_analysis_finite/有限输入 中文/9d4dd1191c383d38e0628590/evidence/`，保留原数值oracle、两份 `*.bound.expected.json`、实际stdout、前后文件SHA和coverage；未知来源替身的空摘要断言仍通过。没有用生产摘要生成独立期望。

CTest stdout SHA-256：MAT `c498efd3426431a2a9463707873d83cf92844b6e2f7d6d5ed2e0ee3e9e0be793`，其余 `c6e5e7852822530bc6763dad221eae97b06c8343ba112e0d4b475a8e3a991dbe`。launcher stderr为空；所有旧失败记录保留。三个Adaptive生成进程仍因既有acos/零旋转边界Incomplete、无完整MAT，本轮没有改变该数值政策或宣布科学成功。没有新增Issue/PR、合并、发布或正式验收。下一项继续45源现行实现/分支/测试追踪，完整科学/L4、资源和正式资格仍待完成。
