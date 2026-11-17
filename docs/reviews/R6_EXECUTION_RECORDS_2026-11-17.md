# R6：五实验入口的运行身份与输出交付记录

`[HUMAN-EXECUTE/APPROVE]`：用户已授权生产代码、构建/CTest及文档修改。对应`REQ/TC-NF-LOG-001`、`TC-NF-RECOVERY-001`、P5.3/P8.9/P10.6。记录schema为候选实现，未冻结、未代替人类验收；算法、MAT字段、数值判据不变。

## 缺口证据

检查点`e1ebdf3e07ecb7872fb3c278a4ec81f6fc746458`工作区干净且远端相同。最新已受测源码`56317e2`的`SourceProvenance`只有repo/source blob SHA及seed，`runExperiment`未保存实际Config/argv/环境。实际上轮初始化失败目录`build/r6-20261117-KzjFHPDQ/cmake/r06_adaptive_cli/Adaptive 中文 空格/f75ecda42cc8f70c887ecbcb/output/initialization`只有`manifest.json`，其键无参数或环境。外部验证脚本日志完整，但不构成产品自动留证。全部状态Issue/PR检索为空；按本记录追踪`R6-EXECUTION-001`。

## 已编写、尚未构建验证

- 五个`run-*`入口在数值Runner前通过原`RunTextPublisher`独占发布`invocation.json`：实际RunId、构建源码/原.m blob SHA、命令及原始参数序列（不含argv[0]）、有效Config、真实seed及来源、当前工作目录/可执行文件路径、固定输出目录、UTC开始时间、运行内核/发行版与构建工具链/依赖。直接库dispatch没有argv时保存null；空Config以源码SHA追踪其固定常量。五实验输入由源码与RNG生成，不伪称读取了历史MAT。
- 记录环境只读取`/etc/os-release`的ID/VERSION_ID/PRETTY_NAME和uname系统/内核/架构；不执行配置脚本、不收集完整环境变量或主机名。不可得信息标null，构建依赖版本与运行内核明确分层；不声称运行时动态库一定等同配置时版本。
- 正常返回路径在输出后发布`execution.json`，保存UTC结束时间、开始记录确认状态、每次stdout/stderr尝试全文及write/flush是否完整成功。它保存输出尝试，不能把失败尝试的全文声称为已送达字节；原始JSONL及全部causes保留在字符串内，可独立解析。
- 返回状态区分`exit_code_if_record_published`和`exit_code_on_record_failure=6`：结束记录发布成功时返回前者；发布/持久化失败时返回6并另报真实错误，已写字节可能存在但未确认。此条件字段避免把已写入的0/5误称为随后记录失败时的真实退出码。进程信号终止不伪造结束记录，已有开始记录/计算Manifest保留。
- 开始记录未确认时不调用数值Runner，保存原始输出失败链；结束记录失败不改写已有Complete/Failed/Incomplete计算Manifest，也不删除MAT。文件继续固定在`output/<RunId>/`，不增加输出根CLI参数或覆盖既有记录。
- `version`显示实际构建源码SHA，移除已经过时的“native checks pending”固定文本。

新增`r06_execution_record_contract`检查五入口/直接dispatch、准确参数、错误转义、启动前记录、发布及发布后fsync故障、结束记录拒覆盖和真实有限Bias Complete产物保留。现有真实CLI测试用CMake独立JSON解析器核对五入口的argv/config/seed/路径/时间/环境/实际返回状态；扩展真实`/dev/full`和SIGPIPE记录边界。原文件数量断言仅增加这两份明确命名的记录，MAT集合及原数值断言保留。

预计SVG ON 53项、OFF 52项CTest；远端检查点确认后重新配置/增量构建，MAT六项优先，其余全部执行。所有新记录、生成MAT与日志只在隔离build沙箱，原`origin/**`只读。当前尚未执行新源码，不以测试登记或schema草案充当通过。

## 尚未覆盖的边界

此批针对五个生成实验入口；只读inspect/analyze、plot/illustrate、探针及解析失败/RunId分配前的外部调用记录仍需按各自不写输入/不擅自预留输出的合同继续复核。信号/资源耗尽导致的非正常进程结束，仍以外部执行者的真实退出/信号记录为准；缺结束文件不等于成功。记录保存不代表三Adaptive完整成功或L4复现，正式容差、资源与基线验收仍独立办理。
