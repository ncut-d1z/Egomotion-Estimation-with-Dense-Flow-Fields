# R6 已分配 RunId 的失败诊断补齐

任务标签 `[HUMAN-EXECUTE/APPROVE]`；使用当前生产代码、构建/CTest和文档授权。关联 `REQ/AC/TC-NF-LOG-001`、`TC-NF-RECOVERY-001`、`TC-I-CLI-001`、`TC-I-RESULT-001`。本轮只落实既有可得RunId记录义务，不改数值公式、阈值、错误码、退出码、字段定义或冻结状态。

## 缺陷证据：R6-RUNID-001

检查对象 `a457f0a83c2656f16244b6e19bf26b7d1c29333f`（代码仍为受测 `2438a8039a905c7f3d8ab74aba1faeaa9a2387cf`）。上一轮真实初始化命令 `egomotion run-adaptive-init --run-id initialization` 退出5；`output/initialization/manifest.json` 顶层已有真实 `run_id`，但stderr和Manifest的 `errors[0]` 均缺少该字段。其原始日志保存在：

`build/r6-20261117-KzjFHPDQ/cmake/r06_adaptive_cli/Adaptive 中文 空格/3b43d0c5c3fcba652b846f85/evidence/1-initialization.stderr`

该原始stderr的SHA-256为 `bca844a09d85903a40a62c82f256883007e16c4c86402aac309c0b15adc5f383`，本轮不覆盖。

根因是Production Runner包装数值错误时未带入已有RunContext身份；CLI直接序列化结果，`RunContext::recordFailure`也原样保存。仅在Manifest最外层保存RunId，不能满足单条错误事件独立定位的既有合同。`docs/design/cli.md` §5及 `REQ-NF-LOG-001` 要求保留已分配RunId；`plot`/`illustrate`完成预留后的导出错误、输出schema/Manifest序列化拒绝存在同类遗漏。已检索GitHub all-state Issue/PR，结果为空；本地缺陷编号与前后日志保留，不新增重复问题。

## 修复范围与验证设计（准备状态，尚未运行）

- `production_runner.cpp`：在掌握RunContext的包装层写入真实RunId，纯内存数值算法仍不虚构运行身份。
- `run_context.cpp`、`manifest_json.cpp`：补齐缺省顶层身份、当前save校验及Manifest序列化失败的运行身份。保持内层链的原始字段及已有身份，不将上下文传播伪装为新的错误因果层。
- `application.cpp`：对外部Runner替身及可选绘图/插图后端返回的缺省顶层身份做边界补全；stderr与Manifest使用同一个错误对象。尚未分配RunId的错误不因用户传了选项而伪造已分配身份。
- 新内部 `src/run_diagnostics.hpp` 仅补缺省顶层字段；没有新增公共头、公开方法或JSON字段。
- 新 `r06_runid_failure_context`：五实验命令、plot、illustrate及分配前无后端共8次真实Composition Root调用，故障后端为明确的合同替身；实际预留目录及发布失败Manifest，核完整JSONL/内层身份/路径/索引/转义、手工及自动RunId。不是8次科学实验或8个独立CLI进程。
- 扩展现有Manifest测试，检查两独立上下文和已知身份保留、源错误对象不变；扩展Bias/Adaptive真实生产Runner失败断言及三Adaptive实际CLI进程的run_id断言。原失败帧/有效样本/数值边界断言均保留。

当前CTest登记47项，尚未编译或执行本次修改；此前46/46属于旧代码SHA。先提交、正常推送、核实远端检查点，再执行配置/构建、先五项MAT后其余42项CTest。原始资料只读；仅隔离沙箱写入，保存命令/环境/时间/退出码/stdout/stderr/SHA与真实RunId。完整Adaptive成功、MAT稳定读取摘要、45源现行映射、完整科学/L4、正式数值资格与审批仍有剩余工作。
