# R6 原生构建与 CTest 记录（2026-11-17）

**[HUMAN-EXECUTE/APPROVE]** 用户在当前任务明确授权生产代码修改、构建/CTest 及方案文档修改。本记录描述实际执行，不表示 DEC-005/006/008 批准、正式验收、冻结或论文实验复现。分支为 `dev-plan-P0-P1-mat-dictionary-probe`，每次运行前先提交、正常推送并通过 GitHub branch API 核对远端 SHA。

## 环境和数据边界

- Ubuntu 24.04.5 LTS，Linux 7.0.0-43-generic x86_64；GNU C++ 16.2.0（`/usr/bin/g++`），C++17/Release；CMake/CTest 4.3.4。
- Eigen 3.4.0、Sophus 1.24.6、matio 1.5.26、zlib 1.3；使用已安装依赖，没有安装或修改开发环境。
- 构建、合成 MAT 沙箱及日志位于独立 `build/<RunId>/`，原始 `origin/**` 只读；不运行 MATLAB/Octave、业务完整实验或性能基准。
- 每次日志目录 `build/<RunId>/evidence/` 保存 `environment.txt`、`input-git-blobs.txt`、`inputs.sha256`、`randomness-source.txt`、逐步骤 `.meta/.stdout/.stderr`。测试使用固有固定输入/源码内种子；文件 RunId 独立使用 OS 随机源。CTest 子进程输出由 CTest 收集进 verbose stdout，启动器 stderr 另外保存。
- 本地完整路径、原始日志与数据保持在被忽略的构建目录，不提交生成结果；下列仓库相对路径供复查。本次 SHA-256 文件清单用于执行前后完整性核查，不冒充预先独立批准的 L2 Golden。

## 运行 1：首次全工程编译发现测试源码错误

RunId：`r6-20261117-LNjYbhF5`；源码 SHA：`52ed76831fe5cdf867b2374fee96a39a8750ff7d`（已核实远端）；工作目录为仓库根。

```bash
cmake -S "$PWD" -B "$PWD/build/r6-20261117-LNjYbhF5/cmake" \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ \
  -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$PWD/build/r6-20261117-LNjYbhF5/cmake" --parallel 4
```

| 步骤 | 开始/结束（UTC+08:00） | 退出码 | 结果 |
|---|---|---:|---|
| configure | 15:07:11 / 15:07:19 | 0 | 正确识别依赖和干净源码 SHA |
| build | 15:07:19 / 15:08:11 | 2 | `test_r01_contracts.cpp:21` 使用未声明的 `static_require`；尚未开始 CTest |

MAT I/O、球谐、分析库已编译，但不能据此认定全工程通过。`evidence/build.stderr` SHA-256：`6abd09b37b13434d09b8a30ed4c11c57fdc52623cc8fef9626556e2e61a41c05`。保存全部失败日志；发现后检索远端全部 open/closed Issue/PR，结果为空，仓库未找到对应已知问题记录；未创建或关闭远端 Issue/PR。

修复：把四处类型合同检查恢复为 C++17 `static_assert`，保留全部原有断言条件；同时显式初始化分析输入错误中的空 observation_index/causes，处理本次编译器诊断，不关闭警告、不改变错误码/阶段或数值判据。修复后须在新检查点重试，不能沿用本次 configure 作为新源码的验证证据。

## 运行 2：Bias Runner 缺失函数声明头

RunId：`r6-20261117-AAjc8Xpe`；源码 SHA：`cfcc9d8ac7cc3f3174700dedb49e4c16bcc1c6aa`（已核实远端）。使用与运行 1 相同命令/参数，构建目录替换为该 RunId 下的 `cmake/`；工作目录仍为仓库根。

| 步骤 | 开始/结束（UTC+08:00） | 退出码 | 结果 |
|---|---|---:|---|
| configure | 15:10:06 / 15:10:11 | 0 | 依赖与源码 SHA 正确 |
| build | 15:10:11 / 15:13:20 | 2 | `bias_runner.cpp:89/92` 未见 `randomUnitDirection` 声明；尚未开始 CTest |

运行 1 的 `static_require` 错误及四条初始化警告均已消失。此轮 `build.stderr` SHA-256：`efcc8d2f19411e2788c3c6b35a57301c40e1a92ea334bcaf08ac1841c07b9930`。重新查询远端全部 Issue/PR 仍为空。

诊断：函数已在 `sampling/directions.hpp` 声明、`directions.cpp` 定义，Bias Runner 仅包含 `sampling/random.hpp`。补充正确头文件，保持调用、随机数顺序及计算公式不变。下一次可复用此构建缓存并重新配置新 SHA，但采用独立 RunId/证据目录，保留本次失败日志；缓存复用不算一次全新干净构建。

## 运行 3：Reader 测试的注入对象 const 不匹配

RunId：`r6-20261117-Xc5pz93H`；源码 SHA：`5362234637ee0594b83b926ca0c4a759f46e3ddf`（已核实远端）。复用运行 2 的构建目录，重新配置；证据另存于本 RunId。构建参数为 `cmake --build build/r6-20261117-AAjc8Xpe/cmake --parallel 4 -- -k`，GNU Make 的 `-k` 收集独立目标错误，任何失败仍返回非零且阻止启动 CTest。

| 步骤 | 开始/结束（UTC+08:00） | 退出码 | 结果 |
|---|---|---:|---|
| configure | 15:14:49 / 15:14:53 | 0 | 构建源 SHA 已更新 |
| build | 15:14:53 / 15:16:17 | 2 | 生产可执行程序已链接；`test_r02_reader.cpp:75` 注入 const 指针不符合 Composition Root 参数类型；CTest 未开始 |

`build.stderr` SHA-256：`0e6d8412556a514385dd12b74187e10eb9668548ed12acb7b5cb4b7a1e6cfb5a`。再次查询全部 Issue/PR 为空。修复测试 Reader 实例声明以匹配现有 `IResultReader*` 注入合同；Reader 的读取接口仍为 const，文件打开模式不变。另将 CLI 维度输出循环中的两个语句分行，消除 `-Wmisleading-indentation`，保留原输出和全部诊断选项。

## 运行 4：编译通过，MAT 元数据测试的错误假设

RunId：`r6-20261117-mdWkQyL8`；源码 SHA：`bd06d4085499b9669fdffe5c25e161fa0867cf30`（已核实远端）。复用运行 2 的构建目录，配置/编译命令同运行 3。

| 步骤 | 开始/结束（UTC+08:00） | 退出码 | 结果 |
|---|---|---:|---|
| configure | 15:17:36 / 15:17:39 | 0 | 已记录新 SHA |
| build | 15:17:39 / 15:18:49 | 0 | 全工程构建成功，本轮 stderr 为空 |
| MAT CTest | 15:18:49 / 15:18:51 | 8 | 2/2 失败于 metadata data_type==9；剩余 19 项未执行 |

CTest 命令为 `ctest --test-dir build/r6-20261117-AAjc8Xpe/cmake -C Release --verbose --no-tests=error --parallel 2 -R '^(r02_mat_reader|r06_historical_mat_readonly_schema)$' --output-junit <本RunId>/evidence/ctest-mat.xml`。原始 stdout/stderr/JUnit 和 Testing 快照已保留；stdout SHA-256 为 `0e2250f446c6b26647811439959a9e2df11869279ee23a1d19518d4c4ddf061e`。再次查询全部 Issue/PR 为空。

在同一已推送 SHA 上进行一次只读诊断：15:20:37–15:20:38，`egomotion inspect-mat --file origin/egomotion_estimation_dense_flow_fields_matlab_scripts/bias_test/produced_data/occluded_regions/1/xwerte.mat`，exit 0。`inspect-reader.meta/.stdout/.stderr` 记录完整路径/命令及结果：`class=6 type=0 rank=2 dims=10x1 data_bytes=0 compressed=1`，无数据值输出。

**证据支持的测试修正**：matio 1.5.26 [Mat_VarReadNextInfo5](https://github.com/tbeu/matio/blob/v1.5.26/src/mat5.c#L4923) 在这些 MAT v5 普通数值数组的结构读取阶段不填 data_type/nbytes；完整解码才设置内存类型与字节数。原测试将 `Mat_VarRead` 的预期用于 `Mat_VarReadInfo`，与已选 DEC-008-05“只读元数据、不解码数值”的职责冲突。现对元数据明确断言未知类型/未知字节数；把 double 类型/实数/non-logical 检查移至完整解码，并增加全量扫描/每文件解码字节数检查。没有删除 double 检查、减少 582 样本或改动数值容差，也没有让 inspect 为通过测试偷偷读取数值。

**边界保留**：这些字段是 matio 返回的快照；解码后的 `data_type` 不是独立解析文件原始 packed data tag 的证据。所有 MAT 变种的原始物理类型、截断/损坏等严格拒绝能力仍需专门负例，元数据接口是否补充该信息仍是设计/实现缺口，不能由本轮 CTest 自动宣称完整覆盖。

## 运行 5：干净全工程构建及 21/21 CTest 通过

RunId：`r6-20261117-KzjFHPDQ`；源码 SHA：`84c86624954406b31e8a82d5e145c7f6ae30ee93`（运行前已核实远端）。新建空目录 `build/r6-20261117-KzjFHPDQ/cmake/`，没有复用旧编译缓存。完整执行包装保存为本 RunId 的 `run.sh`。

```bash
run_dir="$PWD/build/r6-20261117-KzjFHPDQ"
cmake -S "$PWD" -B "$run_dir/cmake" -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DBUILD_TESTING=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$run_dir/cmake" --parallel 4 -- -k
ctest --test-dir "$run_dir/cmake" -C Release --verbose --no-tests=error \
  --parallel 2 -R '^(r02_mat_reader|r06_historical_mat_readonly_schema)$' \
  --output-junit "$run_dir/evidence/ctest-mat.xml"
ctest --test-dir "$run_dir/cmake" -C Release --verbose --no-tests=error \
  --parallel 4 -E '^(r02_mat_reader|r06_historical_mat_readonly_schema)$' \
  --output-junit "$run_dir/evidence/ctest-remaining.xml"
```

| 步骤 | 开始/结束（UTC+08:00） | 退出码 | 实际结果 |
|---|---|---:|---|
| configure | 15:23:20 / 15:23:24 | 0 | 全工程目标生成成功 |
| build | 15:23:24 / 15:26:13 | 0 | 全部生产库、两个工具及测试程序成功编译链接；无编译警告/错误 |
| MAT CTest | 15:26:13 / 15:26:14 | 0 | 2/2 Passed，0 failed，0.52 秒 |
| 其余 CTest | 15:26:14 / 15:26:14 | 0 | 19/19 Passed，0 failed，0.44 秒 |
| 输入完整性复核 | 15:26:15 完成 | 0 | 全部本轮输入 SHA-256 一致；另核对本轮最初保存的全部 628 个 origin 文件也一致 |

两组 CTest 名称互斥，合计覆盖当前全部 **21 项**，没有跳过或降低阈值。配置、构建和两组 CTest 的独立 stderr 均为 0 字节；详细断言输出及 JUnit/Testing 快照保留在 evidence。

| 覆盖组 | 本次通过的 CTest |
|---|---|
| 球谐 | `low_order_harmonics`、`low_order_harmonics_archived_golden`、`low_order_harmonics_source_projection` |
| 基础/入口/读取 | `r01_contracts`、`r01_cli_contract`、`r02_mat_reader`、`r06_historical_mat_readonly_schema` |
| 发布/安全/生命周期 | `r05_manifest_gate`、`r05_anchored_io`、`r06_io_symlink_and_runid_safety`、`r06_result_lifetimes_error_frames` |
| 采样/数值/轨迹 | `r06_sampling_flow_contract`、`r06_kvd_independent_l3`、`r06_adaptive_step_independent_l3`、`r06_sh_argument_action`、`r06_directions_all_ordered_l3`、`r06_trajectory_sphere_geometry` |
| 状态/分析 | `r06_adaptive_session_transaction`、`r06_adaptive_periods_atomic_commit`、`r05_analysis_source_contract`、`r06_analysis_segments_smoothing` |

归档测试读取 **582/582 MAT**，核对五类 shape、完整解码类型/字节数及逐文件 SHA 计算；并非独立预期值逐元素比对。球谐三个测试分别完成 53,532/176/76 次内部检查，最大残差分别为矩阵/向量 `8.8817841970012523e-16`、标量 `1.0658141036401503e-14`；归档候选 `8.8817841970012523e-16`；源投影 `1.7763568394002505e-15`。沿用专项回归阈值 `9.0949470177292824e-13`，未批准新的 DEC-005 门限。

关键本地证据 SHA-256（`build/r6-20261117-KzjFHPDQ/evidence/`）：

| 文件 | SHA-256 |
|---|---|
| `build.stdout` | `09a8441fa3cd62d985a283bf817407a0d29fdd4302d2a9f3977fded94a8b09d2` |
| `ctest-mat.stdout` | `3da74d08b1696eecf477c39ce446de6e28a28329dae9a923a72692d5679196d8` |
| `ctest-remaining.stdout` | `c29d77d268f81390f99f2bf329bfc660ba4f560c54071cda04ba9d6f813a04a7` |
| `inputs.sha256` | `4b4f00d6e8fe3f4d68420d73475ffc5b238a29be2ea05700eaae33fa640dfd11` |

## 交接与剩余范围

- 本轮提交链：`12c4482` Writer const 适配/输入不变；`52ed768` Manifest 发布后失败合同；`cfcc9d8` 静态断言与初始化；`5362234` Bias include；`bd06d40` Reader 注入与 CLI 警告；`84c8662` 元数据/解码测试修正。均正常推送同一开发分支。
- 实际源码改动：`src/io/run_mat_writer.cpp`、`src/analysis/reader_helpers.cpp`、`src/experiments/bias_runner.cpp`、`src/cli/application.cpp` 及领域类型注释。回归改动：R01 合同、R02 Reader、R05 Manifest/Writer、R06 归档测试；相关授权/进度/测试/构建文档同步。
- 本次未创建/修改/关闭 Issue 或 PR，未合并、发布或冻结。最后的文档提交只登记已验证 SHA，不冒称该后续文档 SHA 重新构建。
- **R6 尚不能结案**：仍需独立 L2 文件身份/逐值参考、剩余 L3 多轮/多帧和完整 Runner/八轮行为、必要 L4 统计、真实发布后 fsync/unlink 故障注入、MAT 原始 packed tag/损坏等负例及 SHRotate 差异限定；最终 DEC-005/006/008 与人类验收独立办理。没有 MATLAB/Octave/L1 阻塞，也没有为通过测试改变数值门限或删除断言。
