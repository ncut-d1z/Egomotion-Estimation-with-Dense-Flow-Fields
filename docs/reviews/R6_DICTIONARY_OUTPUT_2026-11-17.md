# R6：独立 MAT 探针的输出交付与报告关闭

`[HUMAN-EXECUTE/APPROVE]`：用户已授权生产代码、构建/CTest、方案文档修改。关联R2历史探针保留义务、`TC-NF-LOG-001`、`TC-NF-RECOVERY-001`及MAT只读检查；不改变业务算法、MAT类型范围、数值阈值或正式验收状态。

## 当前证据与缺口

`R6-DICTIONARY-001`：2026-11-17 23:23:30（UTC+08:00），干净工作区/远端检查点`44d1db5cb191adf1ca06a3d56b0009c2d9393f3c`，使用`5a2cedc47b59787f441aef377feea2f6c11c2f8c`构建的`mat_data_dict_probe`读取原归档initialization四份MAT、`--expected-count 4`、新隔离报告、stderr重定向`/dev/full`，实际**退出0**。完整报告已保存，但状态通知没有送达；原四份MAT前后SHA-256一致。二进制SHA-256为`bedd4ba46f23180a294f5bac9404c2e5b7aa68da67badf15dd5660210c619f92`。

证据保留在`build/r6-dictionary-output-20261117-JDef6yWA/baseline.sh`、`baseline.meta`、`baseline.stdout`、`baseline-report.md`、`baseline-inputs.sha256`及后验文件。stderr准确记为`/dev/full`，不伪造捕获日志。实际环境Ubuntu24.04.5、GCC16.2.0、CMake4.3.4、matio1.5.26、zlib1.3，执行者为获授权的Codex；无算法RNG。当前全部状态Issue/PR检索为空，按本记录跟踪。

另经静态核对，报告`ofstream`只显式检查write/flush，关闭交由析构器，无法将关闭失败反馈到退出码。未将该静态发现称为已经实测的关闭故障。

## 首次检查点准备（当时尚未编译/执行）

- 将探针argv/退出码处理提取为独立`egomotion_dictionary_cli`，主程序继续只处理标准流。复用已有受检write/flush，捕获启用流异常掩码后的异常；不清除错误位、不递归输出、不向stdout补写失败诊断。
- 保留历史参数、报告格式和0/1/2体系：0要求数量/读取/报告/状态交付成功，1表示实际扫描数量或读取失败，2表示参数/执行/报告/状态交付失败。没有套用`egomotion`的退出6。默认SIGPIPE策略不变。
- 报告显式`close()`并检查结果，格式化使用classic locale及异常掩码。状态/关闭失败不删除已写报告；写失败可能留下部分报告，应以实际退出码及日志判断，不能据文件存在声称报告完整或持久化。
- 新增`r06_dictionary_output_streams`：真实四份MAT读取，五种短写/flush/异常故障×两种流异常设置，成功/数量不符/异常诊断路径；保留报告和原始字节、检查历史参数与格式。
- 新增`r06_dictionary_real_process`：默认582份归档、四份子集、空/损坏输入、原始目录保护、已有/显式替换/非生成/符号链接报告、真实`/dev/full`、报告write/close故障。ELF插桩只加载进指定测试子进程，精确匹配新沙箱报告路径；每次必须看到实际注入标记。关闭故障在真实关闭一次后注入EIO，不泄漏或重复关闭。

预计SVG ON登记52项、OFF登记51项，两项探针测试均运行；仍先运行MAT六项，再运行其余全部。运行前先提交、正常推送并确认远端SHA；源码准备不等于通过。所有原始MAT只读，新报告与日志仅在build隔离目录，不重写已提交数据字典，不进行完整科学/L4/性能实验。

## 固定源码实测

受测源码 **`56317e2215fca608b6afa2ffd1a144dd3444b4a6`**，分支`dev-plan-P0-P1-mat-dictionary-probe`。运行前已提交、正常推送，经GitHub branch API核对相同SHA及干净工作区；执行者为本轮获授权的Codex。没有创建/关闭Issue、PR或合并/发布。

| 配置 | 配置/增量构建 | MAT优先 | 其余CTest | 起止时间（2026-11-17 UTC+08:00） |
|---|---|---|---|---|
| SVG ON | 退出0，无编译诊断 | 6/6 | 46/46，合计**52/52** | 23:30:44–23:32:20 |
| SVG OFF | 退出0，无编译诊断 | 6/6 | 45/45，合计**51/51** | 23:32:42–23:34:23 |

逐项核对四份JUnit：0失败、0跳过、0禁用，所有case状态为run；两模式唯一不同仍为`r06_svg_source_windows`。每种模式新增证据：

- **165项检查 / 40次入口调用**：五种流故障×两种异常掩码的成功/数量不符及异常诊断路径，原始字节与已完成报告保留；非法expected-count在报告写入前失败，历史格式与0/1/2规则通过。
- **17次真实进程调用**：默认582/582读取、子集/空/损坏输入、报告边界及明确替换规则、真实stderr=`/dev/full`均符合预期。两份报告的SHA逐字一致，分别验证成功和数量不符报告不因通知失败而改变。
- write与close插桩均实际命中指定报告。stderr分别含`INJECT dictionary report write EIO`/`Report write failure`及`INJECT dictionary report close EIO`/`Report close failure`，实际退出2；关闭失败后的报告字节与正常成功报告一致。插桩只属于测试环境，没有作为生产依赖链接。
- 两批582份原始MAT前后SHA-256核验均通过；外层对`origin/verification/tests`的前后摘要核验退出0。628原文件Git树保持`70ee155131c24a50169346d0ffbaff0c01b1c547`。已有数学、真实Adaptive失败、原始MAT独立逐值等回归全部保留，没有修改阈值或有效样本数。

RunId **`r6-dictionary-output-20261117-JDef6yWA`**；完整日志在同名`build/`目录的`evidence-ON/`、`evidence-OFF/`，包括`.meta/.stdout/.stderr`、JUnit、工具链/输入Git blob与SHA、`Testing`副本和实际链接/ELF记录。每个真实探针调用另记录argv、cwd、起止时间、实际退出码和注入环境；具体新沙箱路径见CTest verbose输出。所有报告、失败记录、测试产物保留且忽略上传，已提交数据字典没有重生成。

实际Ubuntu24.04.5 LTS / Linux7.0.0-43-generic x86_64，GCC16.2.0/C++17 Release，CMake/CTest4.3.4、Eigen3.4.0、matio1.5.26、zlib1.3、Sophus1.24.6。未安装依赖。两个构建目录沿用`build/r6-20261117-KzjFHPDQ/cmake`和`build/r6-headless-20261117-Joe73gEd/cmake-off`，本轮均为增量构建。实际命令：CMake Release、BUILD_TESTING/EXPORT_COMPILE_COMMANDS=ON、`EGOMOTION_ENABLE_SVG=ON/OFF`，`cmake --build ... --parallel 4 -- -k`；MAT六项先`ctest --verbose --no-tests=error --parallel 2 -R ...`，其余`--parallel 4 -E ...`，完整参数见`run.sh`和各`.meta`。探针无算法RNG；其余回归固定seed按原测试保留，不是统计实验。

## 后续范围

本项覆盖独立探针的通知/报告故障，不声称探针报告具有业务`RunDirectory`的全部并发路径安全或fsync持久性保证。正式运行的命令、参数、环境与日志身份记录仍须继续补足；R6整体、三Adaptive完整成功、L4和正式数值/基线资格均未结案。

后续运行身份缺口已静态定位：`SourceProvenance`当前只有repo SHA、源.m blob SHA和seed；`runExperiment`实际Config没有随Manifest序列化，`makeCompleteManifestJson/makeFailureManifestJson`没有命令参数和运行环境字段。验证脚本记录了真实命令/环境/退出码，但这些外层验证日志不能冒充产品每次正式运行均自动留证。应沿既有`REQ-NF-LOG-001`补齐实际argv/有效参数、构建与运行环境及交付状态，保持计算Complete与最后通知退出状态的区分，不改既有MAT字段、算法或RunId目录规则；候选记录格式仍不等于正式冻结。
