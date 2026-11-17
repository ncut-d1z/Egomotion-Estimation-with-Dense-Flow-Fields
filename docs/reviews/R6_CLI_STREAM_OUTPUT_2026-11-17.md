# R6：CLI 输出流故障与失败审计保存

`[HUMAN-EXECUTE/APPROVE]`，用户已授权生产代码、构建/CTest和文档修改。关联既有`TC-NF-LOG-001`、`TC-NF-RECOVERY-001`、`TC-F-CLI-001`及输出错误合同；不改变算法、数值阈值、Manifest计算完成定义或正式冻结状态。

## 缺陷与修复准备

`R6-STREAM-001`：固定受测源码`f3398b099c70233e55e150ecf41c2dd90f282c00`的成功输出未检查ostream状态或主动flush。2026-11-17 22:49:58–22:49:59（UTC+08:00），在远端已包含该源码的`4f2fb4637f49367d0af55be5d3455eac45fcdd3d`干净工作区，实际二进制执行`help > /dev/full`、`illustrate --kind circle > /dev/full`均错误返回0；`unknown-command 2> /dev/full`返回2，未识别诊断交付失败。没有生成实验文件。证据在`build/r6-stream-output-20261117-j7wXeMCQ/baseline.*`，含实际HEAD、受测源码、二进制SHA、命令、开始结束时间与返回码。

静态核查还发现：若调用方对stderr设置异常掩码，原Runner错误输出可先抛异常，导致独立的失败Manifest保存未执行；最外层catch再次写同一错误流可能继续抛出。原有测试未注入stdout/stderr短写/flush/异常，不能用其通过证明此边界。全部状态Issue/PR检索为空；此缺口按本记录跟踪。

首个检查点时准备的修复/回归，**当时尚未编译执行**（最终结果见下文）：

- 统一检查实际write与flush；覆盖短写、部分写、延迟flush失败及ostream异常。写失败返回既有`OutputFailed@Output`退出6；stderr本身不可用时仅保证非零退出，不伪造诊断已写出。
- 所有成功stdout入口使用受检交付；已分配RunId的实验/绘图输出失败保留真实身份。失败stderr不抛出到调用者，也不阻止独立Manifest发布。原计算失败和Manifest失败仍是各自完整的独立事件。
- 已经完整发布的MAT/Manifest/SVG不因stdout通知失败而重写、删除或降级计算结果。进程退出6表示通知交付失败；原成功产物继续保留供复核，不能据此自动覆盖或重跑相同RunId。
- 新增5种流故障×2种异常掩码组合，覆盖数值/元数据/帮助输出、真实有限Bias发布、保留产物及自动RunId、失败Manifest完整链、两通道同时失败和第二条独立诊断失败。另以真实CLI和Linux`/dev/full`覆盖输出失败。
- 默认SVG ON预计50项CTest、OFF预计49项；两个新测试均在两模式运行。正式执行前须先提交、正常推送并核实远端SHA。此前48/47的结果不覆盖新源码。

仅在隔离沙箱运行seed=7、level=2有限Bias及固定几何/合同测试，不进行完整科学实验；原始`origin/**`只读。运行环境、实际命令/退出码/日志及结果在执行后追加。

## 首轮执行与夹具诊断

`7c444ecce56ebcdf4ec92fa2ea81ed93c4665afe` 提交、正常推送并核实远端后，SVG ON配置/编译退出0且无编译诊断；MAT6/6、其余43/44，合计**49/50**。真实`/dev/full`的11次调用全部通过。唯一失败在`r06_cli_output_streams`最后的第二诊断场景：夹具把`manifest.json`目录放到`approvedOutputRoot()`下，而该接口表示`output/`，应再追加真实RunId。实际`output/two_events/manifest.json`成功保存，因而没有第二次失败诊断，测试期望6而实际返回原计算失败5。故障位置和文件身份已由源码/实际目录复核，未将该夹具错误归因于生产算法。

现只修正夹具的RunId路径，**保留全部原断言**；生产修复不变，待新远端检查点复测。首轮原日志、JUnit、沙箱和输入SHA核验均保留于RunId `r6-stream-output-20261117-j7wXeMCQ` 的`evidence-ON/`及其中引用的测试目录；原始输入完整性核验退出0。首轮尚未运行OFF配置，不将其计为通过。

## 第二轮与真实管道边界

`a24ac9176ab8cb9e609dd1b966ef3f19b56facb6` 在远端确认后，两模式配置/增量构建无诊断，ON 50/50、OFF 49/49 CTest通过；各模式流测试841项/122次装配调用、真实`/dev/full`11次调用通过。RunId `r6-stream-output-final-20261117-LgpGa2o3`，两份输入SHA前后核验均退出0，实际记录保留。

后续静态复核发现普通ostream错误之外的边界：默认SIGPIPE可在stderr写入时直接终止进程，因此catch不能替代先保存独立审计。将失败Manifest保存移到stderr通知之前，仍按原计算失败→独立Manifest失败的顺序发送两条诊断；不改变用户进程的信号策略。新增子进程关闭stderr管道唯一读端、显式使用默认SIGPIPE的测试，要求信号终止后原Failed Manifest及完整链已经保存。**该新增边界在第二轮结束时尚未复测，最终结果见下一节**；SIGPIPE仍由信号终止，不能把它宣称为常规I/O错误退出6。

## 最终固定源码实测

最终受测提交 **`5a2cedc47b59787f441aef377feea2f6c11c2f8c`**，分支`dev-plan-P0-P1-mat-dictionary-probe`。执行前已原子提交、正常推送、通过GitHub branch API核实相同SHA且工作区干净；执行者为本轮获授权的Codex。生产修复起于`7c444ec`，夹具修正为`a24ac91`，最终提交补审计先于stderr的顺序及真实SIGPIPE测试。没有合并/发布或新建/关闭Issue、PR。

| 配置 | 构建 | MAT优先 | 其余CTest | 实际时间（2026-11-17 UTC+08:00） |
|---|---|---|---|---|
| SVG ON | 重新配置、增量构建退出0，无编译诊断 | 6/6 | 44/44，合计50/50 | 23:09:34–23:11:19 |
| SVG OFF | 重新配置、增量构建退出0，无编译诊断 | 6/6 | 43/43，合计49/49 | 23:11:33–23:13:10 |

JUnit逐项核对没有失败/跳过/禁用项；两集合唯一差别仍是SVG专用`r06_svg_source_windows`。所有此前数值判据与样本保留。每种配置的新证据：

- `r06_cli_output_streams`：**848项检查、123次CLI调用**，覆盖5种流故障×2种异常掩码、成功数值/元数据输出、真实有限Bias及自动RunId、两种导出器发布替身、stderr失效后的Failed Manifest、两个通道同时故障及第二条独立诊断失败。已有计算产物保留、输入fixture逐字节不变。
- 同一测试的真实子进程在唯一stderr管道读端关闭后以默认SIGPIPE终止；父进程核对实际信号及已保存的Failed Manifest完整原始错误链。没有把信号终止标成返回6，也没有更改主进程的SIGPIPE策略。
- `r06_cli_output_real_process`：**11次真实可执行文件调用**。help/version/插图JSON、inspect/analyze、限定Bias及ON绘图响应写满设备均返回6；解析/读取/数值失败的stderr写满也返回6。OFF用两次缺后端诊断交付失败检查替代ON的两种真实SVG响应检查。需要身份时诊断含实际RunId，已发布7份MAT+Manifest及ON图形文件保持存在；对可读通道保存真实stdout/stderr，对`/dev/full`只记录准确重定向目标，不伪造捕获内容。
- 原始582份MAT独立SHA/33340逐位值、快照、历史/有限分析、几何、真实Adaptive失败合同等均仍通过。两批输入前后SHA核验退出0；`origin`树仍为`70ee155131c24a50169346d0ffbaff0c01b1c547`，原始资料只读。

实际环境：Ubuntu24.04.5 LTS、Linux7.0.0-43-generic x86_64、GCC16.2.0/C++17 Release、CMake/CTest4.3.4、Eigen3.4.0、matio1.5.26、zlib1.3、Sophus1.24.6；未安装依赖。源码工作目录为仓库根，RunId **`r6-stream-sigpipe-20261117-dPSEjNjc`**。日志均保留在忽略目录，不提交生成MAT或敏感日志：

- `build/r6-stream-sigpipe-20261117-dPSEjNjc/run.sh`及`evidence-ON/`、`evidence-OFF/`，每一步`.meta/.stdout/.stderr`、工具链、Git blob、输入SHA/前后核验、JUnit、各批`Testing`副本齐全；子进程沙箱完整路径见CTest verbose输出。
- ON构建目录为`build/r6-20261117-KzjFHPDQ/cmake/`，OFF为`build/r6-headless-20261117-Joe73gEd/cmake-off/`。本轮两者均为增量构建，不能冒称全新构建。
- 配置使用`-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DEGOMOTION_ENABLE_SVG=ON/OFF`；构建`--parallel 4 -- -k`；MAT六项`ctest --verbose --no-tests=error --parallel 2 -R ...`先行，其余使用同正则`-E`及`--parallel 4`。各步完整argv/工作目录/起止时间/退出码见`.meta`；测试种子7，有限Bias最高level2，非统计实验。
- 另保留编译清单、CMake cache/CTest登记、实际链接及ELF符号/依赖。OFF仍无两个SVG实现。首轮失败和第二轮成功记录均保留，不由最终结果覆盖。

本次证明的是`egomotion`输出交付及失败审计边界，**不是全部进程异常/资源耗尽保证或正式CLI冻结**。计算Complete与CLI通知交付是不同事实：通知写失败可退出6而已有Complete Manifest保持真实；原计算失败的Manifest事件仍记录原错误类别，不改成成功。后续继续独立探针的状态输出、运行参数/环境身份记录及P/TC覆盖核对。三Adaptive完整成功、L4/完整科学实验、资源与正式数值/基线资格仍未完成，R6未结案。
