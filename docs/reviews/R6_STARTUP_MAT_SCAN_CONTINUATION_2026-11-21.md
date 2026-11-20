# R6 启动异常与 MAT 扫描歧义接续（2026-11-21）

`[HUMAN-EXECUTE/APPROVE]`。按所有者“继续代码实现，推迟构建、测试”，从 Draft [PR #6](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/pull/6) 的 `defc4d3664010561094115d241c1e2ffa36b1c5b` 继续。前批见 [MAT 名称选择与字典指纹记录](R6_MAT_SELECTION_LOCALE_CONTINUATION_2026-11-20.md)。**本批仅源码、必要回归源码/CMake 链接文本及状态文档，未配置、未编译、未做语法编译、未执行测试、CLI、MAT I/O 或数值实验。**

## 1. 实际应用启动阶段的标准异常回退

[application_main.cpp](../../src/cli/application_main.cpp) 原先在进入 [cli::run](../../src/cli/application.cpp) 的异常边界之前，构造参数字符串、装配服务并调用 `std::filesystem::current_path()`。普通 [main.cpp](../../src/cli/main.cpp) 没有外层捕获；[process_record.cpp](../../src/cli/process_record.cpp) 的 `invokeObserved()` 直接调用应用函数，外层子进程入口为 `noexcept`。

静态反例：进程进入一个目录后，该目录被移除，`current_path()` 抛出 `std::filesystem::filesystem_error`。原入口不能到达已有的 `std::exception` 回退，普通应用会异常逃逸，记录器应用子进程也会意外终止。**本轮没有创建这个运行场景或执行反例。**

本批在真实应用入口使用 function-try-block，覆盖参数构造和实际装配，仅捕获 `std::exception`，复用 [streams::outputFailure](../../src/cli/stream_output.hpp)。保留原 `what()`、`OutputFailed / Output / exit 6`；启动时尚未预留 RunId，不虚构身份或错误原因链。诊断再次失败时仍由原 noexcept 输出边界返回 6，不改变信号终止或非标准异常政策。

[test_r06_process_records.cpp](../../tests/integration/test_r06_process_records.cpp) 在既有 `r06_process_records_lifecycle` 中准备两种真实入口调用：

- 直接路径在隔离子进程内进入并移除自己新建的空 cwd，再调用 `egomotionApplication()`；正常返回 6 与异常逃逸的专用退出码区分。
- 记录器路径在自己的应用子进程内制造相同故障；要求 `termination=exited`、`child_exit_code=6`、`application_returned` 和 `context_return_confirmed=true`。

两路均检查错误 JSONL、无成功 stdout、无未分配的 RunId、无科学输出，并比较诊断字节。仅移除各子进程本次新建的空目录，父侧有效 cwd、已打开日志与其它证据保留。测试统计标签同步改为 `application/recorder calls`，准确包含新增直接调用。[CMakeLists.txt](../../CMakeLists.txt) 仅给该既有目标增加 `egomotion_application_main` 链接，使用其既有 SVG ON/OFF 依赖，没有新增测试目标。**上述回归未编译、未执行。**

## 2. MAT 扫描不得用首项结构冒充同名的其它项

[MatResultReader::scanFile](../../src/io/mat_result_reader.cpp) 原先通过 `Mat_GetDir` 枚举名字，再对每项调用 `Mat_VarReadInfo` 或 `Mat_VarRead`。同一文件含两个相同后端名称、但 shape 或 class 不同的矩阵时，按名查找会重复返回首项，导致后续结构被错误标为首项的 `METADATA_OK` 或 `READ_OK`。

本轮独立读取 matio **v1.5.26** 的 [src/mat.c](https://github.com/tbeu/matio/blob/v1.5.26/src/mat.c)，文件 blob 为 `a3e6008ff74eb9a8388a6572db854a9a9a91bd7d`：`Mat_GetDir` 逐项复制名称且不去重；`Mat_VarReadInfo` 从起点查找首个 `strcmp` 匹配项并恢复原位置；`Mat_VarRead` 复用该查找。这提供静态反例依据，**不表示本轮已生成或读回合成 MAT**。

现在扫描在任何逐名元数据/载荷读取之前，复用现有名称计数检查唯一性。重复时返回既有 `MatErrorCode::AmbiguousVariable`、`ErrorStage::Read`，保留原路径和后端冲突名称；`inspect()` 原样传播该错误。首 NUL 截断后发生的别名碰撞也按实际后端名称识别。没有新增命名字符限制、类型支持、公共 Reader 方法或错误枚举。

[test_r06_mat_payload.cpp](../../tests/integration/test_r06_mat_payload.cpp) 复用 `r06_mat_raw_payload_boundaries`，新增 6 个 fixture，既有大小端 × 普通/压缩循环对应 **24 种新增编码场景**：

| 场景 | 回归源码预期 |
|---|---|
| 同名但 shape 不同、同名但 class 不同 | 明确返回歧义失败，不用首项冒充第二项。 |
| 上述两类的首 NUL 别名碰撞 | 以实际后端名称识别重复，保留冲突名与来源。 |
| 唯一名字但 shape 不同、唯一名字但 class 不同 | 保留两个真实结构、扫描状态及支持目标的具名读取。 |

覆盖 `inspect`、两种 `scanFile`、`readNamed` 和 `readFirstCompatible`。歧义文件要求零次数值读取；唯一多变量正例要求元数据阶段不调用数值读取、完整扫描读取两项、具名读取指定 double 一项。正例中的 single class 仅沿用元数据/字典扫描的既有能力，业务 double 读取边界保持。**24 是源码设计数量，不是执行或通过数量；没有继承旧回归的运行计数。**

## 3. 同轮限定静态复核

数值侧对照原只读脚本核对 Franz/KvD、Adaptive Step、Reset/Inherit/Periodic 系数策略以及 SH 拟合/旋转/适配，未确认新的实现缺口；未改动更新顺序、CS 相位、已批准指标、阈值或支持范围。采样、噪声、领域值对象和 CLI 参数解析的本轮已读路径也未确认其它具体问题。对既有六个未消费系数位置、元数据空目录历史候选及发布持久性范围，没有擅自增加限制或改写合同。

这些是有限源码路径的审查结论，不是全域正确性证明。上一批“当前已列明代码范围具备源码级闭合条件”仍只表示功能已有实现；本批继续修复已实现路径的具体错误，不能把源码完成视为无需验证。

## 4. 交付与延期

实际检查包括源码交叉审阅、Git 状态/差异及 `git diff --check`。原始 `origin/**` 树保持 `70ee155131c24a50169346d0ffbaff0c01b1c547`；历史分支、工作树和失败证据保留。没有运行仓库验证脚本或安装依赖。

DEC-R6-01～04、方案 B、固定八轮分母和末轮 ratio 保持，**没有新增需要所有者立即决定或干预的事项**。构建/测试、完整科学实验、独立 L3 的更广覆盖与必要 L4、最终数值/性能及跨环境资格、正式冻结/验收和发布继续延期；旧固定 SHA 的通过记录不外推到本批。PR #6 保持 Draft，R6 总体验收仍未完成。
