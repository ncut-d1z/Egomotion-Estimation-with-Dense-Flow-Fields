# R6：显式进程记录、只读命令证据与终止状态

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。接续[98项任务审计](R6_PLAN_TASK_AUDIT_2026-11-18.md)D4/P5.3/P8.9/P10.6、REQ/TC-NF-LOG-001。起点本地/远端bd2c6c0一致且干净，沿用独占分支dev-plan-P0-P1-mat-dictionary-probe；用户允许生产代码、构建/CTest和方案文档。本批最高标签覆盖实际执行，接口/schema仍为候选，未冻结或验收。

## 实现与边界

新增 [egomotion-record](../../src/cli/record_main.cpp) 显式入口，参数传给与原egomotion共用的 [应用入口](../../src/cli/application_main.cpp)。静态还原新增include和函数改名后，原main主体全文一致；数学模块、源资料、独立参考、参数解析和分派均无diff。直接egomotion输出行为保留，记录器stdout只交付日志位置收据，子进程通道原始字节分别写stdout.bin/stderr.bin，不实时转发。

[父进程](../../src/cli/process_record.cpp)在开始记录确认后fork；UTC、原argv、编译时SHA、实际cwd和环境白名单进入start.json。waitpid区分exit137与SIGKILL，finish.json记录原始wait状态、signal/core标志及INT/TERM/HUP/QUIT转发结果；子进程恢复先前信号状态。父进程只在自身失败时回收尚未wait的本次子进程，不留后台实验。正常返回实际child code，信号返回128+signal；自身记录/通道/收据失败返回6，结束记录的返回码明确以发布和收据成功为条件。通道fsync、实际字节数和同一open fd的SHA-256分别登记。

[存储](../../src/cli/process_record_storage.cpp)使用固定`build-cli-records/record_<128位独立OS随机值>/`，沿用既有build-*忽略规则；没有修改.gitignore或已有build目录权限。RecordId不是RunId，MAT/图形仍只在output/下。可信检出根须有真实origin目录；私有日志根拒绝非目录、符号链接、其他owner和非私有权限，目录fd锚定、独占创建和linkat禁止覆盖。开始/结束先写.pending、fsync文件、发布新链接并fsync目录，确认后才移除本次临时链接；失败证据保留。四次随机碰撞后明确失败，科学RNG未被调用。

外层明确application_context=not_observed_by_supervisor；尚不收集或推断应用实际RunId/seed/输入快照。Analyzer已有身份保留在原stdout，生成/导出原记录继续工作；inspect结构输出没有被外层绑定的读取快照摘要。后续需补跨层关联；不得拿请求RunId当成功预留身份、拿重开路径hash当实际读取对象。记录器只承诺自身存活且存储可用时观察子进程；父进程SIGKILL、掉电、无法写盘不能保证结束记录。捕获文件与直接终端/坏管道是不同环境，原入口流交付回归仍独立执行。本批没有单独经记录器执行科学生成/图形导出，不能把只读命令对照扩大为全部命令实测。

## 首轮失败与修复

300aae56195ae682a39e4edd7fb019850814bfe3的SVG ON配置/编译exit0，MAT7/7、其余61/62，整体68/69；生命周期测试在碰撞重试断言失败。RunId为r6-process-records-20261118-m0d1HWOs，日志、JUnit、Testing、失败沙箱均保留；732项输入后验匹配。预建collision/build实际0775先触发生产拒绝，碰撞替身未到达预期分支。全状态检索无同问题且无Issue Template，登记 [Issue #4](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/issues/4)。

f8d556a显式设置夹具安全祖先权限并保存每次返回/诊断/hits，未降低断言。静态审查又发现父进程异常展开早于诊断会恢复SIGPIPE；e16b27e将诊断纳入保护域，添加开始/结束发布故障时stderr闭管道、收据stdout闭管道的真实进程检查。该版ON69/69、391项/23次通过，RunId r6-process-records-final-20261118-VqqWkrhN，碰撞实测exit6/hits4。最终617c7f8将日志根独立为build-cli-records，兼容已有0775 build而无需chmod；同样拒绝共享/公开日志根，增加现有build权限/内容保持检查。全部旧记录保留，Issue #4不自动关闭。

## 最终构建与测试

受测源码 **`617c7f8a21966ee8f8f9974bfd8ab0f123c50eda`**，每次配置前均已提交、正常推送、GitHub branch API核实，工作区干净。RunId **`r6-process-records-private-20261118-d91C3PbC`**；命令为`bash build/r6-process-records-private-20261118-d91C3PbC/run.sh 617c7f8a21966ee8f8f9974bfd8ab0f123c50eda ON`及同参数OFF。两个模式复用不同源外缓存，日志/evidence-ON与evidence-OFF独立保存；未覆盖前两轮证据。

| 配置 | 配置/编译 | MAT优先 | 其余CTest | 起止时间（含时区） |
|---|---|---|---|---|
| SVG ON | exit0，无诊断 | 7/7 | 62/62，共69/69 | 2026-11-18T19:37:48+08:00 — 2026-11-18T19:39:50+08:00 |
| SVG OFF | exit0，无诊断 | 7/7 | 61/61，共68/68 | 2026-11-18T19:40:05+08:00 — 2026-11-18T19:42:05+08:00 |

[生命周期测试](../../tests/integration/test_r06_process_records.cpp)各399项、23次受监控调用：exit0/6/137、SIGKILL/SIGPIPE及父TERM转发；NUL和中文/完整诊断链原始字节保留；开始/结束linkat与发布后fsync、通道fsync、fork、OS随机故障；四次碰撞、日志根符号链接/普通文件/共享/公开权限拒绝；收据/诊断ostream坏态及三个真实闭管道。仅测试目标包装linkat/fsync/fork/getrandom；无生产故障开关。每次故障实际触发次数和应用是否进入均断言，开始未确认不执行，末尾失败不丢子进程通道。

[实际CLI测试](../../tests/integration/test_r06_process_records_cli.cmake)每模式12组、各两次真实调用：help/version、非法参数/自定义根拒绝、原始MAT结构读取、缺文件、历史初始化分析/缺分析输入、circle/harmonic JSON、非法阶数和RunId预留碰撞。CMake独立解析两份JSON、原argv/源码/环境/信号null及返回状态，逐通道独立SHA-256/字节数比对原命令输出。只读及解析失败不创建output目录，预留失败不向已有RunId添加记录；原归档全部文件摘要不变。

每模式732项origin/verification/tests输入SHA后验通过；原始树70ee155131c24a50169346d0ffbaff0c01b1c547、628文件/582MAT未变。四份JUnit逐名匹配注册清单，无失败/错误/跳过/禁用，ON唯一多出r06_svg_source_windows。21组既有数值/合同摘要与b2628a9逐字相同（除CTest编号），包含Flow1245、方向901、运动839、系数178、球谐服务494、Adaptive入口172962/场景88121/注入36091和原SH/KvD/策略/Session/分析。两入口的ELF/链接与全编译源均审计：OFF无SVG实现，ON保留实际导出符号。

环境：Ubuntu24.04.5、Linux7.0.0-43 x86_64、GNU16.2、C++17 Release、CMake/CTest4.3.4、Eigen3.4、matio1.5.26、zlib1.3、Sophus1.24.6。受测目录及所有命令/参数、分支/SHA、带时区起止/退出码、stdout/stderr、JUnit/Testing、输入blob/SHA与随机源说明在evidence-*保存。新测试无统计随机抽样，OS记录身份独立；既有固定seed见保存的randomness-source.txt。read-only-junit-audit.json、read-only-regression-summary-audit.json及read-only-source-audit.json为已保存证据的只读复核，不是额外程序执行。

## 修改文件与未完成项

本批四次源码提交共触及14个文件；本报告与进度同步另作文档提交：

```text
CMakeLists.txt
docs/design/cli.md
src/cli/application_main.cpp
src/cli/application_main.hpp
src/cli/execution_record.cpp
src/cli/execution_record.hpp
src/cli/main.cpp
src/cli/process_record.cpp
src/cli/process_record.hpp
src/cli/process_record_storage.cpp
src/cli/process_record_storage.hpp
src/cli/record_main.cpp
tests/integration/test_r06_process_records.cpp
tests/integration/test_r06_process_records_cli.cmake
```

文档同步保持98项P、66项TC、45个.m路径/50个声明与TEST_PLAN验收正文，9份文档的440条相对链接静态检查保存在read-only-document-audit.json；未运行Bash追踪脚本或代签冻结。外层尚未关联应用RunId/seed/实际MAT快照身份，不能将进程记录等同整个REQ-NF-LOG验收；完整科学实验和正式资格仍未完成，真实Adaptive仍Incomplete、无MAT。未安装依赖、运行MATLAB/Octave/完整科学/L4/性能实验、修改原资料/数值阈值、合并或发布。Issue #1–#4开放，无PR；继续D4跨层身份和其余P/TC覆盖复核，R6未结案。
