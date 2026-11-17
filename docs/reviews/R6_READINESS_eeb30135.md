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
