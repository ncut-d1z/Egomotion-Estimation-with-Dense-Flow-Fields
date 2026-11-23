# R5/R6 当前源码复核与接续方案

> 标签：`[AI-DRAFT + HUMAN-APPROVE]`。事实核查为 `[AI-EDIT]`；下面的修正及测试方案为候选、未冻结、未批准，不代替人类授权或验收。
>
> 固定审查对象：`eeb30135a48f27b3eb9c6c289709e9e792770ec8`，分支 `dev-plan-P0-P1-mat-dictionary-probe`。本次恢复本地落后的 389 个提交，采用 fast-forward，没有重做、丢弃或改写已有成果。

## 1. 结论与授权边界

R5 的 Writer、RunId、Manifest、Analyzer、SVG 及 CLI 装配有实质源码，不能再按早期空壳状态描述。但下文发现 Writer 的确定类型错误，应重开 R5 的这项源码修复；**“R5代码完成”不能解释为可以编译或已验收**。R6 不完整，路线图没有 R7，不新增替代阶段。

本轮用户直接提供的新 AGENTS.md §10.3 写明“本轮仅允许审阅与方案文档修改”，而固定树 AGENTS.md 的“2026-11-15 最新代码授权”章节记录过源码编写许可。本轮按用户直接提供的较新边界开展审阅与候选文档工作，不从仓库旧许可推断新的源码修改权限。H-11 的原生执行继续延期，不重新催办已经延期的数值容差、性能、冻结、发布事项；MATLAB/Octave/L1 和普通版权审批都不是研发前置条件。

## 2. 实际核查及证据限制

| 核查 | 结果 | 不能证明什么 |
|---|---|---|
| GitHub 分支与本地 HEAD | 已同步到上述固定 SHA；同步前工作区干净，无本地独有提交 | 不证明代码通过构建 |
| GitHub 全状态 Issue 集合 | 空集合，接口含 PR；未发现可复用 Issue/PR | 不代表没有源码缺陷 |
| 根 CMake 文本与 tracked `.cpp` 集合 | 59 个 src/tests 源文件都出现在 CMake 文本中，引用路径缺失 0；21 个 `add_test(NAME ...)` 名称无重复 | 只是静态文本与文件存在核对，不是 CMake 配置、链接或 CTest 列表执行 |
| 实际读取源码 | RunDirectory、MAT/Text Writer、Reader、RunContext、Manifest、ProductionRunner、CLI 装配以及相关合同/集成测试 | 是本报告限定范围审阅，不是所有算法的完整审核 |
| 本机 matio 头 | `/usr/include/matio_pubconf.h` 声明 1.5.26；`matio.h:305–307` 的数据参数为 `void*` | 不代表本机工程构建环境已经齐备 |
| 依赖包状态查询 | Ubuntu 24.04.5；`libmatio-dev` 1.5.26-1build3；Eigen 开发包未返回已安装版本，`libsophus-dev` 无包记录 | 包查询不能排除手动安装的 Eigen/Sophus；未运行 CMake 探测，不据此宣称依赖不可用 |

没有运行 CMake、编译器、CTest、业务 CLI、MAT 数值读取/写入、Bash 追踪脚本、Wolfram 或实验。没有将旧探针 582/582 或旧 SHA 的 SH 3/3 转记为当前工程通过。

## 3. 明确源码缺陷：const 数据不符合 matio 接口

- 位置：`src/io/run_mat_writer.cpp:143` 的 `const DenseDoubleArray& payload`，以及 `:185–187` 的 `Mat_VarCreate(..., payload.values_column_major.data(), 0)`。
- 该成员是 `std::vector<double>`，从 const payload 取得的是 `const double*`；当前 matio 1.5.26 头声明参数为 `void*`。C++17 不允许该隐式去 const 转换。
- 影响：`egomotion_matio` 源文件无法在该接口下编译，继而阻止其 CLI/测试依赖构建；这是静态类型结论，**尚无编译器实际退出码或诊断日志**。
- 已保存证据：上述固定源 SHA、源码行号、安装头的版本和签名；未执行原生复现。仓库 Issue 全状态搜索无已有项，本轮仅登记在此文档，未创建外部 Issue。

候选最小修正：保留公开的 const 输入和 matio 的复制所有权模式，在调用前创建可变局部值副本。不得对调用方 payload 使用 `const_cast`，不得开启 `MAT_F_DONT_COPY_DATA` 引入借用生命周期变化。

```cpp
auto writable_values = payload.values_column_major;
VarHandle variable(Mat_VarCreate(name.c_str(), MAT_C_DOUBLE, MAT_T_DOUBLE,
                                2, dims, writable_values.data(), 0));
```

副本位于已有 `try` 中，分配异常沿现有 `WriteFailed` 返回；代价是创建 MAT 变量阶段多一份 double 缓冲，需要保留在内存评估中，不隐瞒此成本。单独提交修正；不改字段、列主序、数值、错误枚举或原始 MAT。关联 P1.7/P1.9、REQ-F-MAT-004、REQ-DC-IO-001、REQ-NF-SAFETY-001 及对应 TC/AC，现有测试实体为 `r05_anchored_io`，不新造编号。

对应测试源码候选：现有 2×3 fixture 改为真正的 const 输入，保留调用前完整 payload；Writer 返回后及重复写拒绝后，检查 payload 的值和元数据保持不变，并继续检查回读字节/维度/列主序及 receipt。实际运行仍是单独的 H-11 范围，写入仅允许独立合成沙箱，不触碰 `origin/**`。

## 4. R6 尚未覆盖的关键路径

| 义务 | 当前真实代码证据 | 下一步候选 |
|---|---|---|
| 暂存文件错误路径清理 | `test_r05_io.cpp` 覆盖成功、重复名、非法名；重复/非法名在创建暂存文件前返回 | 增加创建暂存文件后的写入/回读/文件 fsync/linkat/目录 fsync/unlinkat 故障注入；分别核最终文件有无、receipt 的 durably_confirmed、原始错误链。当前测试不能证明所有错误路径无暂存遗留 |
| Manifest 发布后失败 | `RunTextPublisher` 在 linkat 后仍可能返回 ReadbackMismatch；合同测试替身只模拟发布前失败 | 先补候选场景：最终名字已存在且内容为 Complete，但后续持久化确认失败时，API 返回失败且不得覆盖原名字；明确磁盘可见内容与已确认运行状态的区别，再审核恢复/审计合同。不能用“替身失败后可重试”证明真实 no-overwrite 发布能这样恢复 |
| L2 文件身份与逐值 | `test_r06_archive_readonly.cpp:68–70` 只检查本次计算出的 SHA 长度为64，并核对 shape 频数 | 独立预期 SHA-256 与逐值参考需明确来源、存储和批准；被测 SHA 函数本次算出的摘要不是独立预期值，不将 shape/计数测试命名成全部 L2 验收 |
| 全链执行记录 | `build.sh` 有 H-11 门禁、一次 verbose CTest 和日志路径 | 获准执行时另保存 RunId、固定 SHA、分支、实际命令、开始/结束时间、各步骤退出码、工具链/依赖、stdout/stderr、输入身份及输出目录；现脚本的合并输出本身不是完整验收包 |
| 科学与业务闭环 | 现有独立 L3、Session 控制流和局部 geometry/SH 测试源码 | 按原 ROADMAP 继续完整 Runner、多帧/8-run 聚合、SHRotate 参数/相位作用限定、必要 L4；不以21项注册或某一合同测试缩小这些义务 |

这些测试和恢复方案均未执行、未批准，不改变 SRS/LLD/AC、既有阈值或 Complete 的验收义务。特别是 Manifest 的发布后失败，需要先明确候选恢复合同，不能擅自覆盖已经发布的文件。

## 5. 接续顺序

1. 获得本轮 Writer 源码修正的明确范围后，落实第3节最小修改和 const 输入测试源码，原子提交、正常推送。
2. 在文档/测试准备允许范围内，逐项补第4节的故障场景与独立参考映射；待批准方案保持候选。
3. H-11 由所有者主动恢复后，再核查干净工作区和远端 SHA，选新的隔离构建/运行目录，执行真实配置、编译、21项及届时新增测试，失败证据原样保留并修复，不降低阈值。
4. 依现有 ROADMAP 完成 L2/L3/L4 与最终验收条件。源码修正、测试通过、基线批准、合并发布分别登记，目标保持 R6 全部原义务，不以本报告完成代替目标达成。

## 6. Manifest 发布后故障的确定路径与候选测试

本节接续核查对象为 `2615b828f6c8aa8aebb8849286d4150b586157e0`；以下生产/测试源与 `eeb30135` 相同。**只静态逐分支推演，没有执行故障注入、系统调用或 CTest**。不是实测故障报告，也不把预计行为记成PASS。

| 源文件 | Git blob SHA |
|---|---|
| `src/io/run_text_publisher.cpp` | `b4e4354539282f09f25d53d3aec6e426d71c6b2b` |
| `src/io/run_mat_writer.cpp` | `976f1743eb4905871bf1c79e0797e79b06894ca6` |
| `src/experiments/run_context.cpp` | `cd72d8c77d8fd9c91c5549af783f870fdb10d56c` |
| `tests/contracts/test_r05_manifest.cpp` | `04483fd17a9cb042da85cfc017dabca78e522f9b` |

### 6.1 三种状态不能混为一谈

1. `makeCompleteManifestJson` 先产生内存字符串，其中有 `completeness=Complete`；此时没有文件，也没有成功 RunOutcome。
2. `RunTextPublisher::publishNew` 的 `linkat` 成功后，最终文件名已经可见；随后两次目录 `fsync` 和中间 `unlinkat` 仍可失败。文件内容可见不等于持久化确认成功。
3. `RunContext::publishCompleteOutcome:140–149` 只有收到 Publisher 成功才设置内存 `manifest_published_=true`、`draft_.status=Complete` 并返回成功；失败分支保持非成功。这部分不能误报成“API在fsync失败时仍返回Complete”。

**确定可达的条件路径**（条件是注入“linkat成功、第一次目录fsync失败”，而非声称真实磁盘已发生该错误）：

- `RunTextPublisher:115–121` 已链接 `manifest.json`，返回 `ReadbackMismatch`，最终名及其Complete文本保持不变；守卫仅尝试删除暂存名。
- `RunContext:140–145` 返回 `ExperimentErrorCode::OutputFailed`，原因链包含该 Mat 错误；未将内存状态设为Complete。
- `cli/application.cpp:171–178` 输出首次JSONL错误，随后调用 `publishFailureRecord`；`RunContext:182` 再用同名 `manifest.json` 尝试发布Incomplete记录。
- `RunTextPublisher:81–84` 检出已有最终名，返回 `OutputExists`；CLI输出第二个独立JSONL错误，退出码为6，不输出成功摘要。**第一次持久化未确认与第二次目标已存在是两个错误事件，不能折叠成同一个根因。**

结果是：API/CLI非成功、内存记录Incomplete，而磁盘可见的Manifest文本仍可能是Complete且其发布持久性未确认。当前没有可把该文件自动认作正式成功证据的恢复记录。既有“不覆盖/不删除已发布文件”约束阻止用第二次发布覆盖它；不得为了使测试通过而更改该安全规则。

`test_r05_manifest.cpp` 的 `FakePublisher` 在fail分支先返回，**既不保存contents，也不占用文件名**；恢复后的第二次调用自然可写，因此只验证发布前失败。它不建模上面的已发布状态，不能证明所有Manifest失败都能持久化Incomplete。

### 6.2 最小候选合同测试（不新增正式TC或已注册CTest）

在既有 `r05_manifest_gate` 中增加一个测试局部Publisher替身，持有 `Absent / Published` 两种名字状态。仅模拟控制流，不在该合同测试中读写MAT；它与真正文件系统故障注入的证据分开。

| 候选子场景 | 替身规则 | 可判别观察 |
|---|---|---|
| 发布前失败 | 首次返回WriteFailed，不保存文件名或文本；恢复后首次成功保存 | 原有测试保持；第二次发布能够保存Incomplete |
| 发布后未确认 | 首次先保存名字和Complete文本，再返回ReadbackMismatch；以后同名一律OutputExists | 首次结果失败；内存无Complete；第二次失败，原文本字节不变，不能断言Incomplete已落盘 |
| 正常完成 | 首次保存并返回成功；以后同名OutputExists | 只有第一次可产生Complete Outcome；再发布被拒绝 |

每例使用相同合格内存输出和Writer receipts，以隔离Publisher行为；同时检查原因链、两个错误事件的次序、原文保存和内存状态。CLI退出码及stdout/stderr断言在独立集成层进行，不把未装配CLI的合同测试写成CLI验证。关联既有 TC-F-MAT-004、TC-NF-SAFETY-001、TC-I-RESULT-001、TC-DC-EXPERIMENT-001；以现行 TEST_PLAN 映射为准，候选子场景不增加66项TC总数。

### 6.3 真正I/O故障场景（待源码方案批准和独立执行授权）

下表是未来独立合成沙箱的预期观察，不是已发生的系统调用结果。每次独立RunId只注入一个明确操作的失败；必须按“目录fd/文件fd、调用阶段、调用次数”选择，不能笼统使所有fsync失败而掩盖实际分支。标注“清理可用”时，后续清理调用须正常执行；清理也失败则按最后一行单独记录。

| 注入位置 | 最终名/旧产物 | 返回与审计要求 |
|---|---|---|
| 独占暂存openat失败 | 无本次最终名，已有文件不变 | 非成功；不可把非本次创建名字交给清理守卫 |
| write失败或返回0 | 无最终名 | WriteFailed；清理可用时本次暂存名移除 |
| MAT关闭/回读/逐值比较失败 | 无最终名 | WriteFailed或ReadbackMismatch，按实际阶段；没有确认receipt |
| 文件fsync失败 | 无最终名 | WriteFailed；清理可用时暂存名移除 |
| linkat竞争EEXIST | 竞争者最终文件的inode和内容不变 | OutputExists；本次MAT待发布receipt撤回，临时名只清理自己的 |
| 发布后身份核对或首次目录fsync失败 | 保留已链接最终名，不覆盖/删除 | ReadbackMismatch；MAT保留durably_confirmed=false；文本返回失败，无成功发布声明 |
| 显式unlinkat失败、守卫重试成功 | 保留最终名；守卫可移除临时名 | 仍为ReadbackMismatch，不能因析构清理成功反改为成功；没有确认后续目录持久性 |
| 暂存名移除后第二次目录fsync失败 | 最终名保留、暂存名在当前进程视图已消失 | ReadbackMismatch；不把文件名可见当作已完成持久化 |
| 清理unlinkat持续失败 | 保留最终名（若已发布）以及真实暂存遗留 | 非成功；记录遗留事实，**不能要求无条件“零暂存”从而隐藏清理失败**，不自动删除其他证据 |

先采用测试局部可控Publisher完成合同场景，再评价真正Linux调用故障注入方式；不向公共业务接口预置可由CLI开启的故障选项。不安装注入工具、不实施源码改造、不执行任何条目；先保留原始生产默认行为与已有21项测试。

### 6.4 需要后续审核的合同范围

现行设计已有“Manifest不能持久化时明确报告、不得虚称Incomplete已落盘”的义务（`docs/design/experiments.md` 的Result/Manifest边界）。上述补测遵守该义务。对**仅持有磁盘文件的离线消费者如何识别发布未确认**，当前单个manifest内容没有完整回答，仍应作为候选设计问题：必须保留非覆盖约束，不能默许把未确认Complete作为成功，也不能承诺发生磁盘故障时另一个审计文件必然能写出。

本节没有选择新增schema、完成标记、恢复命令或旁路日志，没有更改ITextPublisher签名、Complete定义、最终验收规则或现有TC阈值。若未来增加这些协议，须先按DEC-008及相关LLD/AC审查。当前继续等待此前已提出的限定源码授权，不重复催办H-11或最终冻结；本节补足可实施场景后，不用继续堆叠文档替代源码修复与真实验证。
