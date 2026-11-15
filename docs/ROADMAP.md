# C++ 实施路线图

> **状态：R0/R1骨架及R2 MAT只读Reader、旧字典CLI重构已编写；尚未取得实际CMake/CTest证据。** H-10 已允许 C++17 业务源码、头文件、CMake 和测试源码编写；**H-11 运行权限仍单独延期**：本批未执行 CMake configure/build/CTest/CLI 程序、未安装依赖、未读写 MAT；正式容差、性能、基线签署和发布仍未获批准。最新权限见 [TODO](TODO.md)。

## 目标与边界

从用户操作自顶向下贯通：**CLI / Composition Root → 五类实验 Runner 或四类只读 Analyzer → 采样/轨迹/求解器 → 公共值类型与 MAT I/O → 结果及 Manifest**。优先交付可隔离测试的纵向业务链，而非机械按 MATLAB 文件逐个移植；目标命令见 [CLI LLD](design/cli.md)，11 个候选 target/27 条设计依赖见 [HLD §4](ARCHITECTURE.md)。

只支持 Debian/Ubuntu、C++17、Eigen、matio/zlib，SO(3) 按既定 Sophus 方案；现有低阶球谐叶子**复用而不重写**。原 `origin/**` 永远只读；无 MATLAB/Octave/L1、ROS、Python/NumPy/NPZ 或 sdla。新输出只能是 `output/<RunId>/`（须待运行/写入授权）。

## 自顶向下实施批次

| 批次 | 优先贯通的外部行为 | 编程交付与阶段出口（尚待实现/验证） |
|---|---|---|
| **R0 外层契约** | `inspect-mat`、两种 Bias、三种 Adaptive、`analyze`；`plot` 可选 | 确定参数/错误/RunId/RunOutcome 和各命令预期输出；用接口替身先描绘完整调用链，不为未实现命令伪造成功。参考 [CLI](design/cli.md)、[Runner](design/experiments.md)。 |
| **R1 工程骨架** | CLI → Composition Root → Reader/Runner/Analyzer 接口 | 建立 `domain/contracts`、Result/Status/完整错误链、值对象、窄读写与RNG接口和 CMake target 骨架；分离各层依赖，接入已有 SH 叶子。参考 [合同](design/contracts.md)、[对象模型](OBJECT_MODEL.md)。 |
| **R2 第一条只读纵切** | `inspect-mat --file` → Reader → 元数据输出 | 实现 matio RAII、`inspect`（**只显示结构，不读取数值载荷**）、严格/兼容业务Reader及错误传播；隔离测试源码覆盖shape、列主序与资源生命周期。参考 [MAT I/O](design/mat_io.md)。 |
| **R3 Bias 纵切** | `run-bias-*` → BiasRunner → Franz/KvD → 结果值对象 | 接入三种有序视线采样、显式RNG、光流/两噪声模型；实现Franz及Original/Modified KvD，先用**内存Writer替身**检查两种Bias的计算与输出字段，不启动真实MAT写入。参考 [采样](design/sampling_noise.md)、[KvD](design/kvd.md)。 |
| **R4 Adaptive 纵切** | `run-adaptive-*` → Session → 单步求解 | 实现圆/正弦轨迹、球内近度与局部旋转、现有SH叶子公共适配、Adaptive单步及三种实验的时序；比较输出`dumi`不写回，失败不提交后验。参考 [轨迹](design/trajectory.md)、[球谐](design/harmonics.md)、[Adaptive](design/adaptive_mfa.md)。 |
| **R5 结果与分析** | Writer/RunContext → 新MAT；`analyze` → 数值曲线；可选`plot` | 实现独占RunId、单文件非覆盖发布/回读与Manifest；四种Analyzer及HistoricalOnly缺字段失败；绘图不得成为数值计算依赖。**仅编写代码，运行/实际写MAT仍受H-11约束**。参考 [MAT I/O](design/mat_io.md)、[分析](design/analysis_plotting.md)。 |
| **R6 证据闭环** | 全部命令的错误路径、端到端输出与复现 | 先补齐隔离单测/集成/独立L3、原MAT只读L2和必要L4的测试源码及追踪；待H-11另行授权后才运行构建/CTest/安全回读并记录固定SHA。正式DEC-005/006与基线、发布仍独立审批。参考 [TEST_PLAN](TEST_PLAN.md)。 |

## R0/R1 当前交付

已建立 `include/egomotion/{domain,contracts,experiments,analysis,cli}/`、`src/cli/`，保留现存低阶球谐叶子，根 `CMakeLists.txt` 登记11个`egomotion_*`目标、CLI程序和5项CTest注册（既有SH 3项＋新增R0/R1 2项）。其中`sampling/simulation/estimators/matio/experiments/analysis/plotting`仍为仅声明依赖的**INTERFACE占位**，不冒充算法或MAT Reader已实现；业务命令尚无后端，必须显式失败。源码和CTest注册已推送，但**configure/编译/CTest均未执行（H-11）**。静态结构检查只能核文件存在、target无环/依赖关系和CMake括号配平；真实CMake正确性仍须待执行验证。

## R2 已实现的边界与构建命令

`MatResultReader` 位于 `src/io/mat_result_reader.cpp`：`inspect` 仅读结构元数据；`readNamed/readFirstCompatible` 完整解码并复制二维实数`double`列主序数据，RAII管理matio句柄。`src/mat_data_dict_probe.cpp`已改为薄CLI，字典扫描/FNV64/Markdown生成由`src/io/mat_data_dictionary.cpp`负责。旧`MAT_ROOT REPORT.md --expected-count N --source-sha SHA [--replace-generated]`调用格式保留；**报告已存在时必须显式`--replace-generated`并验证生成标记**。

后续获H-11授权时推荐：
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target egomotion mat_data_dict_probe
# 仅当允许写入 docs/MAT_DATA_SCHEMA.md 时才运行此命令：
./build/mat_data_dict_probe origin/egomotion_estimation_dense_flow_fields_matlab_scripts docs/MAT_DATA_SCHEMA.md --expected-count 582 --source-sha "$(git rev-parse HEAD)" --replace-generated
```
原先仅编译`src/mat_data_dict_probe.cpp`的单文件g++命令**不再有效**。如需继续使用手动g++而非CMake（**仅供H-11授权后执行**），须安装包含Eigen的开发依赖并一次链接三个源文件：
```sh
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -Iinclude \
  src/mat_data_dict_probe.cpp src/io/mat_data_dictionary.cpp \
  src/io/mat_result_reader.cpp -o build/mat_data_dict_probe \
  $(pkg-config --cflags --libs eigen3 matio) -lz
```
正式`MatResultReader`数值读取不允许变成`inspect-mat`的数值打印。CMake目标与注册测试是否能真实编译/运行仍需H-11另行授权核验。

## 执行规则

1. **从 R0/R1 开始**，每批先把调用方和失败边界写清，再向下实现所需最小模块；每个逻辑单元单独提交、推送。不得让下层依赖CLI/实验/具体matio。
2. 只对已实际实现的接口宣称“代码已写”；**已推送、已构建、已测试、已验收**分别记录，未经H-11不得标PASS。候选签名变化同步对应LLD/REQ/TC，不凭路线图宣称P0.8已冻结。
3. 细粒度任务、测试映射和实际证据分别以 [PLAN](PLAN.md)、[TEST_PLAN](TEST_PLAN.md)、[PROGRESS](PROGRESS.md)为准；本文件只维护阶段顺序，不重复98项WBS。
