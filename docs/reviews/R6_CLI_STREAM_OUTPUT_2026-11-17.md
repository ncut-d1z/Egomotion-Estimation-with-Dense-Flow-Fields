# R6：CLI 输出流故障与失败审计保存

`[HUMAN-EXECUTE/APPROVE]`，用户已授权生产代码、构建/CTest和文档修改。关联既有`TC-NF-LOG-001`、`TC-NF-RECOVERY-001`、`TC-F-CLI-001`及输出错误合同；不改变算法、数值阈值、Manifest计算完成定义或正式冻结状态。

## 缺陷与修复准备

`R6-STREAM-001`：固定受测源码`f3398b099c70233e55e150ecf41c2dd90f282c00`的成功输出未检查ostream状态或主动flush。2026-11-17 22:49:58–22:49:59（UTC+08:00），在远端已包含该源码的`4f2fb4637f49367d0af55be5d3455eac45fcdd3d`干净工作区，实际二进制执行`help > /dev/full`、`illustrate --kind circle > /dev/full`均错误返回0；`unknown-command 2> /dev/full`返回2，未识别诊断交付失败。没有生成实验文件。证据在`build/r6-stream-output-20261117-j7wXeMCQ/baseline.*`，含实际HEAD、受测源码、二进制SHA、命令、开始结束时间与返回码。

静态核查还发现：若调用方对stderr设置异常掩码，原Runner错误输出可先抛异常，导致独立的失败Manifest保存未执行；最外层catch再次写同一错误流可能继续抛出。原有测试未注入stdout/stderr短写/flush/异常，不能用其通过证明此边界。全部状态Issue/PR检索为空；此缺口按本记录跟踪。

已准备的修复/回归，**尚未编译执行**：

- 统一检查实际write与flush；覆盖短写、部分写、延迟flush失败及ostream异常。写失败返回既有`OutputFailed@Output`退出6；stderr本身不可用时仅保证非零退出，不伪造诊断已写出。
- 所有成功stdout入口使用受检交付；已分配RunId的实验/绘图输出失败保留真实身份。失败stderr不抛出到调用者，也不阻止独立Manifest发布。原计算失败和Manifest失败仍是各自完整的独立事件。
- 已经完整发布的MAT/Manifest/SVG不因stdout通知失败而重写、删除或降级计算结果。进程退出6表示通知交付失败；原成功产物继续保留供复核，不能据此自动覆盖或重跑相同RunId。
- 新增5种流故障×2种异常掩码组合，覆盖数值/元数据/帮助输出、真实有限Bias发布、保留产物及自动RunId、失败Manifest完整链、两通道同时失败和第二条独立诊断失败。另以真实CLI和Linux`/dev/full`覆盖输出失败。
- 默认SVG ON预计50项CTest、OFF预计49项；两个新测试均在两模式运行。正式执行前须先提交、正常推送并核实远端SHA。此前48/47的结果不覆盖新源码。

仅在隔离沙箱运行seed=7、level=2有限Bias及固定几何/合同测试，不进行完整科学实验；原始`origin/**`只读。运行环境、实际命令/退出码/日志及结果在执行后追加。
