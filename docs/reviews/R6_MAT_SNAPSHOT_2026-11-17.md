# R6 MAT封存读取与分析输入摘要

任务标签 `[HUMAN-EXECUTE/APPROVE]`；落实 `TC-F-MAT-001/003`、`TC-I-MAT-001`、`TC-I-ANALYSIS-001`、`TC-NF-SAFETY-001`、`TC-DC-IO-001` 的既有来源/生命周期义务。源码、构建/CTest和文档使用本轮明确授权；原始MAT只读，合成改写只发生在独立CTest沙箱，不运行MATLAB或统计实验。

## 现状与实现边界

检查对象 `5863433ebe11a99e9e8456875b34f4f702c88557`，旧受测代码 `9eb831531ce04f2bfab8bf813011c36a451ee409`。旧Reader固定源fd后分别做容器校验与matio解码；可防路径替换，但不能阻止同inode原地改写。`DenseDoubleArray`没有MAT字节身份，Analyzer只能输出 `mat_sha256=null`。这是 [MAT LLD §7](../design/mat_io.md) 和 [分析LLD §6](../design/analysis_plotting.md) 已明确保留的实现缺口，不能把测试旁路摘要当成生产读取来源。

本轮完整数值读取（named、compatible、VerifyDecoded）先打开只读普通文件，检查fstat，流式复制到Linux匿名memfd；复制前后核dev/ino/size/mtime/ctime，不一致则明确失败。随后加 `F_SEAL_WRITE/GROW/SHRINK/SEAL` 并核实，再计算SHA-256、校验原MAT容器、让matio从同一封存副本解码。副本及matio句柄均RAII关闭，不写原文件或持久临时目录。元数据inspect保持原方式，不复制/摘要/解码数值。

**保证范围**：摘要对应实际校验和解码的那份不可改字节。复制期间可观测到的改动明确拒绝；完成捕获后原路径替换或原inode改写不会改变解码值/摘要。不是整个目录的原子事务快照，不保证每个独立输入在同一时间点采集，也不承诺对能够伪造文件元数据的对抗者证明原文件历史上的单一时刻；原始归档仍通过独立固定SHA清单额外核对。每次读取增加一份原MAT大小的匿名字节存储和摘要遍历，未据此宣布资源/性能资格。

`DenseDoubleArray::source_mat_sha256` 是新增的可选值字段（候选C++接口、未冻结）；不暴露fd/matio，也不新增Reader方法。纯合成数组或未提供身份的自定义Reader保持空。Analyzer从实际数组复制身份到既有 `AnalysisInput::mat_sha256`；Bias跨run贡献、最后x、Adaptive误差/ratio分段来源均按实际载荷绑定。没有新增JSON键、改变数值算法/比较阈值或替换.m源码Git blob。

## 验证准备（尚未执行）

- 新 `r06_mat_snapshot_identity`：确定性链接层注入，named/compatible各15种场景，共30次完整读取，另1次元数据检查。覆盖路径替换、同inode改写、复制期间变化拒绝、memfd失败、读/写/封存/摘要失败、EINTR/短读写重试；成功副本实际pwrite/ftruncate必须EPERM；所有返回后fd数量不变。所有改动对象为新建合成MAT，参考A/B另存并保留。
- 扩展582份独立L2逐值测试：读取返回的SHA必须与既有独立固定清单一致；原33340值逐位断言保留。
- 历史Bias全部曲线输入摘要、真实历史分析CLI摘要均要求实际值，不再允许生产Reader返回null通过。
- 有限Adaptive分析仍用独立多项式数值oracle，SHA由CMake独立 `file(SHA256)`填充期望对象，再完整比较32条曲线JSON；纯内存替身未知摘要仍有原空值断言。

CTest登记48项不等于通过；先原子提交、正常推送、核实远端，再先6项MAT（含新快照）后其余42项执行。保存精确命令、工具链、RunId/SHA/输入身份、起止时间、退出码/stdout/stderr/JUnit；不覆盖旧失败日志。R6完整科学成功、45源现行映射、必要L4、资源与正式数值/接口验收仍未完成。
