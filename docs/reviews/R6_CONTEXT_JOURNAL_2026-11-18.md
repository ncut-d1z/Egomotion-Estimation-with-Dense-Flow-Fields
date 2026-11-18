# R6：进程记录关联实际应用上下文与 MAT 快照

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。用户已明确允许生产代码、构建/CTest及方案文档修改；候选观察接口/schema仍未冻结。本批落实D4、P1.8/P5.3/P8.9/P10.6及REQ/TC-NF-LOG-001的有限实现与验证，不修改数值公式、容差、AC或验收签字。

## 实现与语义边界

生产提交`798c84feeb5653699294b505d17dea4e599f613d`新增显式借用的`IInvocationObserver`及`IMatReadObserver`，普通egomotion默认不注入；egomotion-record的子进程将实际事件追加到私有`context.jsonl`。事件有独立RecordId、连续序号、UTC、事件名和结构化data；每条写完fsync，第一次失败缓存，后续观察不再写入。

- `command_accepted`只在解析成功后记录有效配置；解析失败仍保留进入与实际返回事件。
- `seed_selected`只表示选定seed，分别来自Bias Config或独立OS随机源，不宣称算法已经使用。`run_id_reserved`只在目录成功独占预留之后出现；碰撞不会伪造实际RunId。
- `run_context`和`artifact_context`复用同一份实际`invocation.json`序列化文本；保存源码身份、环境、参数和已绑定的数据来源，不事后重开路径补摘要。
- Reader观察发生在sealed memfd复制与SHA-256完成之后、matio打开之前。记录`mat_snapshot_prepared`及Metadata/Named/FirstCompatible/DictionaryDecode用途；事件本身不表示MAT合法或变量成功解码。记录模式的inspect额外复制原始字节，但只使用Mat_VarReadInfo，不调用数值载荷API、不输出元素。普通inspect保持原路径。
- 应用返回事件fsync成功后，子进程通过独立非阻塞管道发送确认。父进程waitpid后记录`context_return_confirmed`，同时对stdout.bin、stderr.bin、context.jsonl分别fsync并保存字节数/摘要；start/finish使用v2 schema。正常exit缺确认时返回6；信号终止保留事件前缀并返回128+signal，通道/最终记录失败仍为6。文件存在或某条返回事件存在不能替代确认。

观察器失败会在对应边界阻止后续matio消费、Runner或Exporter；已经预留的目录/已有失败证据保留。父进程SIGKILL、掉电、无法写盘不能保证结束记录；没有声称所有资源耗尽/硬件故障或MAT类型都已覆盖。记录成功与科学计算成功是独立事实。

## 首轮缺陷、修复与检查点

起点e225e5a工作区干净、远端SHA一致；Issue #1–#4开放，无PR。恢复本轮已有未提交实现后补测试、提交、正常推送并核实GitHub branch SHA。一次启动参数的预期SHA拼写错误被run.sh前置检查拒绝，exit2，未执行配置/构建/测试；保留`build/r6-context-journal-20261118-3iugs6sI/preflight.txt`，不将此调用错误登记为源码Issue。

首轮RunId `r6-context-journal-verified-20261118-bMgK4RMj`：798c84f的SVG ON配置/编译exit0，无诊断；MAT7/7，其余62/63，整体69/70，CTest exit8。新增上下文测试在导出日志故障后检查已预留RunId失败；完整日志、JUnit、输入校验与合成沙箱均保留。

实际`artifact_context/output/actual/execution.json`含RunId actual，而其stderr事件WriteFailed@Output缺run_id。旧Publisher错误携带身份，新Observer不知道RunId；exportRecorded直接序列化started.error()遗漏补全。检索全部open/closed Issue无重复、无Issue Template后登记[Issue #5](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/issues/5)。修复提交 **`ff8aaa723d979642cca30b6fc79953fc8f2963ed`**复用withRunIdIfMissing，保留已有身份和错误链；增加失败定位文本，不删除断言、不改阈值。修复后重新提交/推送/核实SHA，再执行下面双配置复测。

## 最终构建与 CTest

受测源码 **`ff8aaa723d979642cca30b6fc79953fc8f2963ed`**；分支`dev-plan-P0-P1-mat-dictionary-probe`。RunId **`r6-context-journal-final-20261118-TbQO9zck`**，每次配置前工作区干净并确认远端包含该SHA。

| 配置 | 配置/编译 | MAT优先 | 其余CTest | 起止时间（含时区） |
|---|---|---|---|---|
| SVG ON | exit0，无诊断 | 7/7 | 63/63，共70/70 | 2026-11-18T20:12:43+08:00 — 2026-11-18T20:14:20+08:00 |
| SVG OFF | exit0，无诊断 | 7/7 | 62/62，共69/69 | 2026-11-18T20:14:40+08:00 — 2026-11-18T20:17:15+08:00 |

实际命令为`bash build/r6-context-journal-final-20261118-TbQO9zck/run.sh ff8aaa723d979642cca30b6fc79953fc8f2963ed ON`及同参数OFF。脚本记录完整cmake -S/-B/Release/C++17/编译器/SVG参数、build --parallel 4、MAT优先CTest正则及其余排除正则。复用两套独立源外缓存，证据目录evidence-ON/OFF分别保存，未覆盖旧失败日志。

环境：Ubuntu24.04.5、Linux7.0.0-43 x86_64、GNU16.2、CMake/CTest4.3.4、Eigen3.4、matio1.5.26、zlib1.3、Sophus1.24.6。所有命令、cwd、SHA、分支、起止时间、退出码、stdout/stderr、JUnit/Testing、输入Git blob/SHA、随机源说明均已保存。OS RunId独立随机；受限Bias仍seed7/level0–2；Adaptive选定seed在实际事件及invocation中留存。不执行完整科学统计实验。

## 覆盖与独立复核

新增`r06_context_journal_identity`两模式各 **224项检查**：日志短写/EINTR/零写/写错误/fsync失败、粘滞失败禁止继续写；8组元数据/Named/兼容/字典读取的路径替换与同inode改写；A/B变量名与数值不同，观察后必须仍消费A；元数据数值读取次数严格为0。日志拒绝在Mat_Open前停止，缺文件无虚构摘要。另测配置/seed/预留/RunContext失败阻止Runner，插图来源/预留/导出上下文失败阻止Exporter且保留实际RunId。

`r06_process_records_lifecycle`两模式各 **464项检查、26次受监控调用**：保留原进程/信号/发布/权限/管道故障覆盖，新增直接_exit(0)、有返回事件但无管道确认、返回事件fsync失败；即使父进程随后确认文件，context_return_confirmed仍为false，外层exit6。没有把正常137与SIGKILL混淆。

`r06_process_records_real_cli`两模式各 **12组逐字节对照（24次调用）+5次记录调用，共29次真实CLI调用**。检查完整JSONL事件序号/UTC/RecordId、配置和来源、三个通道独立SHA/字节数、实际RunContext与invocation相等、RunId碰撞不伪造预留。额外执行两种受限Bias（指定/自动RunId）、真实Adaptive初始化、历史初始化plot和circle SVG。ON图形成功，OFF明确exit7且不预留RunId；Bias Complete，Adaptive保持既有exit5/Incomplete且无MAT，后者是失败合同通过，不是科学实验成功。

独立Node只读解析保存的证据：每模式17份记录；ON76条事件、OFF72条事件，各9份MAT快照摘要匹配原文件；Analyzer输入身份逐一关联已记录快照。四份JUnit逐名等于注册清单，零失败/跳过/禁用；ON唯一多出的测试仍为r06_svg_source_windows。每模式 **733项输入SHA后验匹配**；原始树`70ee155131c24a50169346d0ffbaff0c01b1c547`、628文件/582MAT不变。21组既有数值/合同摘要与b2628a9一致，包括球谐494项的max_abs=1.7763568394002505e-15。两个入口ELF/链接复核确认OFF无SVG实现、ON含真实导出符号。上述只读审计分别保存为read-only-junit-audit.json、read-only-regression-summary-audit.json、read-only-context-audit.json和recorder-inspection.meta。

## 仍未完成

D4列明的进程/应用身份关联缺口已补有限实测；REQ-NF-LOG整体及R6不能因此签署完成。完整Bias统计/L4、真实Adaptive完整成功产物、最终数值/性能资格、平台范围和正式基线验收仍未完成；此前Adaptive数值失败不通过夹断、删样本或更改分母掩盖。Issue #1–#5仍开放，无PR；本批未合并、发布、安装依赖或执行MATLAB/Octave，原资料只读。

源码改动清单见下节；进度与映射同步仅登记事实，不改98项P或66项TC的验收义务。

## 修改文件

两次生产提交合计23个文件：

```text
CMakeLists.txt
docs/design/cli.md
docs/design/mat_io.md
include/egomotion/cli/command.hpp
include/egomotion/cli/invocation_observer.hpp
include/egomotion/io/mat_read_observer.hpp
include/egomotion/io/mat_result_reader.hpp
src/cli/application.cpp
src/cli/application_main.cpp
src/cli/application_main.hpp
src/cli/context_journal.cpp
src/cli/context_journal.hpp
src/cli/execution_record.cpp
src/cli/execution_record.hpp
src/cli/invocation_observer.cpp
src/cli/process_record.cpp
src/cli/process_record.hpp
src/cli/process_record_storage.cpp
src/cli/process_record_storage.hpp
src/io/mat_result_reader.cpp
tests/integration/test_r06_context_journal.cpp
tests/integration/test_r06_process_records.cpp
tests/integration/test_r06_process_records_cli.cmake
```

结果登记另更新README、PROGRESS、ROADMAP、TODO、TEST_PLAN、CLI/MAT LLD、P/TC证据索引及本报告。

本轮静态文档复核：10份文档的463条相对链接有效；98个P编号、66个TC编号、45个源路径/50个声明保持，TEST_PLAN顶部事实登记之外的合同正文未变。保存read-only-document-audit.json；未将此Node只读核查称为Bash追踪脚本现场执行。Issue #5的[修复复测评论](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/issues/5#issuecomment-6978398158)已通过GitHub API回读核实。
