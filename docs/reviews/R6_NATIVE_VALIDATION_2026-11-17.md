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
