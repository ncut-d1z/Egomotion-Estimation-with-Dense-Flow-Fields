# R6：独立 MAT 探针的输出交付与报告关闭

`[HUMAN-EXECUTE/APPROVE]`：用户已授权生产代码、构建/CTest、方案文档修改。关联R2历史探针保留义务、`TC-NF-LOG-001`、`TC-NF-RECOVERY-001`及MAT只读检查；不改变业务算法、MAT类型范围、数值阈值或正式验收状态。

## 当前证据与缺口

`R6-DICTIONARY-001`：2026-11-17 23:23:30（UTC+08:00），干净工作区/远端检查点`44d1db5cb191adf1ca06a3d56b0009c2d9393f3c`，使用`5a2cedc47b59787f441aef377feea2f6c11c2f8c`构建的`mat_data_dict_probe`读取原归档initialization四份MAT、`--expected-count 4`、新隔离报告、stderr重定向`/dev/full`，实际**退出0**。完整报告已保存，但状态通知没有送达；原四份MAT前后SHA-256一致。二进制SHA-256为`bedd4ba46f23180a294f5bac9404c2e5b7aa68da67badf15dd5660210c619f92`。

证据保留在`build/r6-dictionary-output-20261117-JDef6yWA/baseline.sh`、`baseline.meta`、`baseline.stdout`、`baseline-report.md`、`baseline-inputs.sha256`及后验文件。stderr准确记为`/dev/full`，不伪造捕获日志。实际环境Ubuntu24.04.5、GCC16.2.0、CMake4.3.4、matio1.5.26、zlib1.3，执行者为获授权的Codex；无算法RNG。当前全部状态Issue/PR检索为空，按本记录跟踪。

另经静态核对，报告`ofstream`只显式检查write/flush，关闭交由析构器，无法将关闭失败反馈到退出码。未将该静态发现称为已经实测的关闭故障。

## 首次检查点准备（尚未编译/执行）

- 将探针argv/退出码处理提取为独立`egomotion_dictionary_cli`，主程序继续只处理标准流。复用已有受检write/flush，捕获启用流异常掩码后的异常；不清除错误位、不递归输出、不向stdout补写失败诊断。
- 保留历史参数、报告格式和0/1/2体系：0要求数量/读取/报告/状态交付成功，1表示实际扫描数量或读取失败，2表示参数/执行/报告/状态交付失败。没有套用`egomotion`的退出6。默认SIGPIPE策略不变。
- 报告显式`close()`并检查结果，格式化使用classic locale及异常掩码。状态/关闭失败不删除已写报告；写失败可能留下部分报告，应以实际退出码及日志判断，不能据文件存在声称报告完整或持久化。
- 新增`r06_dictionary_output_streams`：真实四份MAT读取，五种短写/flush/异常故障×两种流异常设置，成功/数量不符/异常诊断路径；保留报告和原始字节、检查历史参数与格式。
- 新增`r06_dictionary_real_process`：默认582份归档、四份子集、空/损坏输入、原始目录保护、已有/显式替换/非生成/符号链接报告、真实`/dev/full`、报告write/close故障。ELF插桩只加载进指定测试子进程，精确匹配新沙箱报告路径；每次必须看到实际注入标记。关闭故障在真实关闭一次后注入EIO，不泄漏或重复关闭。

预计SVG ON登记52项、OFF登记51项，两项探针测试均运行；仍先运行MAT六项，再运行其余全部。运行前先提交、正常推送并确认远端SHA；源码准备不等于通过。所有原始MAT只读，新报告与日志仅在build隔离目录，不重写已提交数据字典，不进行完整科学/L4/性能实验。

## 后续范围

本项覆盖独立探针的通知/报告故障，不声称探针报告具有业务`RunDirectory`的全部并发路径安全或fsync持久性保证。正式运行的命令、参数、环境与日志身份记录仍须继续补足；R6整体、三Adaptive完整成功、L4和正式数值/基线资格均未结案。
