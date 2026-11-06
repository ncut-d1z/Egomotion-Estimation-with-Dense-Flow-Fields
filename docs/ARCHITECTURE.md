# 系统概要设计（HLD）— MATLAB 自运动估计 C++ 移植

> **状态：架构评审草案 v0.2，未冻结；无 C++ 实现与构建证据。** 来源是已核对的 MATLAB 源码和原 PLAN §3/4/6。需求依据见 [REQUIREMENTS.md](REQUIREMENTS.md)，类职责附件见 [OBJECT_MODEL.md](OBJECT_MODEL.md)，详细设计待定见 [design/README.md](design/README.md)，执行安排见 [PLAN.md](PLAN.md)。以 [AGENTS.md](../AGENTS.md) 和项目所有者批准为准。

## 1. 架构目标、质量属性和约束

- C++17、Eigen 3、matio/zlib、CMake；**仅 Debian/Ubuntu**；原始 MATLAB/MAT 只读；不依赖 MATLAB Runtime、Python/NumPy/NPZ、ROS、OpenCV 或 Windows。
- 忠实保留三轴 Franz、Original/Modified KvD、Adaptive 单步、球谐旋转和五个实验数据生成入口的数学语义；观测为 N×6 三维球面光流而非图像或像素流。
- 数据生成、原始 MAT 访问、新 MAT 结果写入、数值分析和可选绘图应能分别调用。图形输出失败不得撤销已成功生成的计算结果。
- 模块之间通过明确输入输出和 C++17 值对象连接；只在资源或状态确有所有权含义时使用类。纯数学运算可用自由函数。
- 依赖方向不得从数学/领域求解器反向引用 CLI、matio 或绘图库。无全局隐式 RNG；原始 MAT 路径不得用作任何写出根目录。

## 2. 逻辑组件图

```mermaid
flowchart TD
    CLI["CLI / Composition Root"] --> EX["实验 Runner / RunContext"]
    CLI --> AN["分析与结果展示"]
    EX --> SIM["采样 / 轨迹 / 噪声 / 光流合成"]
    EX --> SOL["Franz / KvD / Adaptive 单步与 Session"]
    SIM --> DOM["值对象 / 数学 / 球谐"]
    SOL --> DOM
    EX --> WR["MAT 结果 Writer"]
    AN --> RD["MAT 结果 Reader"]
    AN --> PLOT["可选绘图 Exporter"]
    RD --> MIO["matio RAII I/O"]
    WR --> MIO
    MIO --> MAT["原始只读 MAT 与独立结果目录"]
```

依赖的边界：
1. `origin/**` 是外部不可变输入；输出 `results/<RunId>/` 不允许覆盖已存在文件；执行前解析/检查路径是否与原始目录重叠。
2. 领域实体 `FlowField`、`MotionEstimate`、`HarmonicCoefficients` 自拥有值，不携带 matio 句柄或指向已释放缓冲区。
3. `AdaptiveMfaStepSolver` 是单步无状态计算，`AdaptiveMfaSession` 拥有跨帧 `akkoeff`；每种 baseline 的 `mitkoeff`/重置状态另行隔离。
4. `KvDIterationEngine` 可以共享计算流程，但不同平移更新规则必须可观察且不互换；不能硬造一个把 KvD 与 Adaptive 结果强行同构的“全能求解器”。
5. `IResultReader` 与 `IResultWriter` 是分别收窄的边界。C++ 运行器无权通过读接口修改原始数据。

### 2.1 部署视图（仅 Debian/Ubuntu，设计草案）

```mermaid
flowchart LR
    subgraph host["Debian / Ubuntu GNU/Linux"]
        cli["egomotion CLI"]
        libs["C++17 domain / simulation / estimator / analyzer"]
        io["matio 1.5.26 + zlib；已读 582 MAT v5"]
        plot["可选绘图导出"]
        cli --> libs
        cli --> io
        cli -.按需.-> plot
        libs --> io
    end
    raw[("origin/** 原始 MAT / MATLAB")]
    out[("results/<RunId> 新结果")]
    raw -.只读输入.-> io
    io -->|"仅新输出目录"| out
```

- 运行进程仅在 Debian/Ubuntu 构建；MAT 文件由本地文件系统提供，不要求网络或 MATLAB/Python 服务。
- 原始输入目录的文件句柄只读、哈希不变；写入器不得通过软链接或路径穿越覆盖 origin。
- 只有 `inspect-mat`、实验生成、数据分析是数值主路径；图形服务可缺失，不影响无界面运行。
- 具体发行版最低版本、CMake/编译器版本及其性能边界待 DEC-006 实测与项目所有者批准，不在设计草案中虚构。

### 2.2 已实测的 MAT 数据合同

| 来源范围 | matio 实际读取结果 | 架构影响 |
|---|---|---|
| 全部 582 MAT | MAT v5/zlib、class=double/data_type=double、582 个变量 READ_OK | 当前真实归档只需 v5 + zlib 的 double 数值读取；其他 class 保留明确拒绝路径 |
| Bias 560 文件 | xwerte：80 个 10×1；ywerte1…6：480 个 10×3 | 使用真实 MAT rank/dims，不依据注释推测 |
| Adaptive 22 文件 | 35×1：4、600×1：14、600×4：4 | 当前历史结果结构已明确，逐步算法 Golden 仍缺失 |
| 文件身份 | 每份文件一个与 basename 同名的变量；FNV-1a64 指纹 | 原始输入只读；FNV 不是 SHA-256 |

独立数据字典探针已经实测，但生产 `egomotion_matio` target、Reader/Writer 注入、错误负例、索引转换与写回读尚未验证。

## 3. 架构组件接口与职责（单一来源）

本节只定义**组件级**边界。具体类、值对象、所有权、状态机和 MATLAB 函数映射由 [OBJECT_MODEL.md](OBJECT_MODEL.md) 统一维护；精确函数签名、算法公式和错误枚举由 [design/](design/README.md) 的模块详细设计承担。不得把 HLD 中的“接口名称”误认为已经编译的 C++17 头文件。

| 组件 | 对外提供的能力 | 所需依赖 | 主要数据合同 | 错误边界 |
|---|---|---|---|---|
| domain/math | N×6 FlowField 和 3D 数学、运动估计值 | Eigen | 三维向量、列主序、坐标与单位 | 无效维度、零范数与非有限输入 |
| sampling | 八面体、缺损八面体、二十面体方向采样 | domain/math | 顺序确定的 N×3 单位视线 | 非法细分级别、数目溢出 |
| simulation | 球内近度、光流合成、随机噪声、轨迹 | domain/math、采样、显式 RNG | 近度、t、ω、三维切向光流 | 无效几何、随机源异常 |
| harmonics | 实球谐基函数、近度拟合/重建、系数旋转 | domain/math | l≤2、3×5 以及 3×3/5×5 旋转块 | 系数形状、归一化与旋转失败 |
| estimators | Franz、Original/Modified KvD、Adaptive 单步估计 | domain/math、harmonics（Adaptive） | FlowField、近度/系数、运动估计与计算状态 | 退化、奇异系统、非有限值 |
| mat_io | 原始 MAT 只读元数据/变量读取与新 MAT 输出 | matio/zlib、domain 值类型 | rank、dims、class、dtype、列主序、变量名 | 文件/解压/类型错误、路径拒绝 |
| experiments | 五类 MATLAB 原实验的参数编排、独立 RunId 与状态 | simulation、estimators、写入窄接口 | 参数、seed、结果 MAT、有效/失败样本数 | 缺配置、数值失败、输出冲突 |
| analysis | 四类 MATLAB 结果分析的数据统计与曲线序列 | MAT 读取窄接口、domain | xwerte/ywerte、Adaptive 错误序列、曲线索引 | 缺变量、shape 冲突、无定义统计 |
| plotting（可选） | 统计结果的 2D/3D 可视化 | analysis | 绘图数组、单位、图例与区间 | 后端缺失；不得修改原始数值 |
| cli/composition | 命令、参数检查、状态/退出码和对象装配 | experiments、analysis、mat_io | 只读输入路径、独立输出根 | 参数、源/输出冲突、执行失败 |

**边界不变量**：求解器不 include matio 或绘图库；I/O 不持有求解器；实验 Runner 不重复数学公式；分析/绘图不修改求解结果。实验参数按 MATLAB 的五个入口分别定义，禁止以一个未经核准的全局默认值覆盖其差异。

### 3.1 接口合同与设计冻结状态

- 外部可观察合同以 [REQUIREMENTS.md](REQUIREMENTS.md) 中 REQ-I-* 与 [TEST_PLAN.md](TEST_PLAN.md) 的 TC 为准；内部设计原则单独归入 REQ-DC-*，不将 RAII/继承选择误写成 CLI 用户需求。
- 尚未确定的 C++17 方法签名、返回错误类型、固定矩阵布局、线程安全边界和 ABI 由 P0.7/P0.8 评审后在 LLD 逐模块冻结。
- `IResultReader` / `IResultWriter` 在 `egomotion_contracts` 中分别声明，`egomotion_matio` 的具体实现由 Composition Root 注入实验/分析。原始文件只读保障还必须体现在路径校验与不可覆盖测试中。跨层用自拥有值对象，MAT 缓冲区失效后不得持有 Eigen Map。
- 对 KvD 两条平移更新保留独立入口；Adaptive 的单步求解与多帧 Session 独立。**这些是已选设计方向，并非已验证实现**。

## 4. CMake target、公共头文件与文件结构（设计候选）

本 HLD 负责给出**无环依赖的库边界**；工程目录概览见 [PLAN.md](PLAN.md) §4，具体对象与 MATLAB 函数的映射由 [OBJECT_MODEL.md](OBJECT_MODEL.md) 统一维护，模块内部合同见 [design/](design/README.md)。正式 include/src 路径应在 P0.8 冻结；以下 target 名称不表示仓库已经创建对应库。

| 拟议 target | 公开边界 | 依赖 | 私有实现/禁止逆向依赖 |
|---|---|---|---|
| egomotion_domain | domain 值类型、向量/旋转及基本数学函数头 | Eigen 3 | 不引入 matio、experiment、CLI |
| egomotion_contracts（INTERFACE 候选） | IResultReader/IResultWriter、IRandomSource 等只声明领域依赖的窄接口 | domain | 不含 matio.h、不链接 I/O 实现、不提供 C 函数入口 |
| egomotion_sampling | 方向采样接口及实现 | domain、contracts（仅在实际 API 使用接口时） | 内部递归实现可私有，不依赖 experiment |
| egomotion_harmonics | l≤2 基函数、拟合/旋转接口 | domain | 第三方 SH 递归细节仅私有、须版权审查 |
| egomotion_estimators | Franz、KvD、Adaptive 计算合同 | domain、harmonics | 不依赖 matio、CLI、plotting |
| egomotion_simulation | 轨迹、噪声、场景/光流生成 | domain、sampling | RNG 内部策略只通过窄接口暴露 |
| egomotion_matio | MAT Reader/Writer 与元数据，**实现** Reader/Writer 合同 | contracts、domain、matio、zlib | 仅私有源码包含 matio.h；不依赖 estimators/experiments |
| egomotion_experiments | 五类实验 Runner、RunContext | domain、contracts、sampling、simulation、estimators；**不链接 egomotion_matio** | 只通过 IResultWriter 使用写出能力；无原始 MAT 写权限、不依赖 plotting |
| egomotion_analysis | 历史 MAT 数据分析与曲线序列 | domain、contracts；**不链接 egomotion_matio** | 仅 IResultReader；不调用 estimators，不依赖 plotting |
| egomotion_plotting（可选） | 从分析结果绘图 | analysis | 可缺席、不得影响核心 targets |
| egomotion_cli | 命令解析与 Composition Root，唯一具体 I/O 装配处 | experiments、analysis、matio、contracts；可选 plotting | 构造 MatResultReader/RunMatWriter 再以接口传入，CLI 不实现数学公式 |
| tests/benchmarks（后期） | 各 target 的回归、基准 | 按被测组件链接 | 禁止生产组件依赖测试 |

### 4.1 目标依赖有向无环图

~~~mermaid
flowchart BT
  D[egomotion_domain] --> E[Eigen 3]
  K[egomotion_contracts / INTERFACE] --> D
  S[egomotion_sampling] --> D
  H[egomotion_harmonics] --> D
  V[egomotion_simulation] --> D
  V --> S
  A[egomotion_estimators] --> D
  A --> H
  I[egomotion_matio] --> D
  I --> K
  I --> M[matio / zlib]
  X[egomotion_experiments] --> V
  X --> A
  X --> K
  N[egomotion_analysis] --> K
  P[egomotion_plotting] --> N
  C[egomotion_cli] --> X
  C --> N
  C --> I
  C --> P
~~~

箭头方向为**依赖者指向依赖项**；plotting 只有启用时链接。**egomotion_contracts 是拟议的纯接口 target**：`egomotion_experiments` 和 `egomotion_analysis` 只依赖接口声明与 domain 值类型；`egomotion_matio` 实现这些接口，只有 `egomotion_cli` 的 Composition Root 依赖具体 MAT target 并负责注入。这样消除“文字说抽象、target DAG 却链接具体 MAT”的不一致。需要更细分库或因实际编译发现循环依赖，应出具 ADR 并更新此图，不得用巨大单一库掩盖耦合。

### 4.2 公有/私有边界检查

- 对外 `include/egomotion/` 只暴露领域值对象、必须的领域接口及可公开算法合同；matio C API、递归实现、实验配置解析、具体绘图库适配放在 `src/` 私有实现。
- `MatFileHandle` 作为内部资源管理器不需要由算法层持有；如公开类声明涉及 `mat_t`，须解释必要性或通过私有实现消除泄漏。
- CMake 公开依赖/传递包含路径只用于真实 API 必需内容；源外构建、Debian/Ubuntu 平台与依赖版本可追踪。**接口头文件不可传递 `matio.h`、`mat_t*` 或 `matvar_t*` 给实验/求解模块**。实际 target 的静态/共享链接类型、安装导出及依赖版本在 P0.8 决定。
- **MAT 格式与类型已有实测证据**：[MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) 记录 582/582 MAT v5/zlib、double/double、五种二维 shape，全部 READ_OK；现存归档无需 MAT v7.3/HDF5 读取能力。新输入范围改变时再按 ADR 评审。

## 5. 关键数据流与运行时序

### 5.1 MAT 文件读取与单轮实验

```mermaid
sequenceDiagram
  participant User as 用户/CLI
  participant Root as CompositionRoot
  participant Run as ExperimentRunner
  participant Solver as Estimator
  participant Writer as RunMatWriter
  User->>Root: experiment + 参数 + output-root
  Root->>Run: 注入模型、RNG、写入器和 RunContext
  Run->>Run: 生成球面方向与光流
  Run->>Solver: FlowField / 旧系数（如适用）
  Solver-->>Run: Result(估计值、状态、指标)
  Run->>Writer: 原变量名、shape、结果与唯一 RunId
  Writer-->>Run: 写入/验证状态（禁止写 origin）
  Run-->>User: 状态、输出位置和实际验证记录
```

### 5.2 Adaptive Session 跨帧行为

```mermaid
sequenceDiagram
  participant Runner as AdaptiveRunner
  participant Session as AdaptiveMfaSession
  participant Policy as 更新策略
  participant Step as AdaptiveMfaStepSolver
  Runner->>Session: beginFrame(previousRelativeRotation)
  Session->>Session: 旋转当前 akkoeff 和对照系数
  Session->>Policy: 刷新本帧对照基线？
  Policy-->>Session: 保留/重置/周期刷新
  Session->>Step: 输入主系数 + 本帧 FlowField
  Step-->>Session: 新系数/运动/残差/状态
  Session->>Session: 按 MATLAB 次序提交主状态
  Session-->>Runner: 本帧结果和可审计状态
```

说明：实际 MATLAB 脚本在进行主状态 `akkoeff` 更新之前会分别对照 baseline 求解；设计时必须保留该先后关系。只有单步成功与失败行为经过源码/数值核对后，才能确定 Session 的异常状态转移与提交规则。

### 5.3 错误/状态流

结果至少区分输入验证、MAT 解码/形状、输出覆盖保护、数值退化（零范数、奇异系统、非有限值）、可选绘图缺失及实验部分完成。错误应携带调用阶段、源/目标变量、RunId 和上下文，不把 NaN/Inf 当作通过。具体枚举与 CLI 退出码须在 LLD + SRS 验收条款冻结。

## 6. 实施依赖与并行约束（引用单一来源）

**P0–P10 的阶段/子任务依赖图、并行工作区、里程碑及进入退出准则以 [PLAN.md](PLAN.md) 为单一维护位置。** HLD 只约束 target DAG 和接口依赖方向，不在此复制计划的 P 任务图，避免不同文档产生相互矛盾的依赖和时间表。

P0.7/P0.8 需校验本 HLD、[OBJECT_MODEL.md](OBJECT_MODEL.md)、MAT schema、LLD 契约之间不存在反向依赖或冲突；任何架构改变先调整 ADR 和 REQ-DC，再同步修订 PLAN 关联任务。

## 7. 架构质量属性场景（冻结前评审）

每个场景采用“**刺激/环境 → 必须响应 → 度量/证据**”结构，具体实现和测试见 SRS/TEST_PLAN。没有测过的性能或内存数字不得写为已满足阈值。

| 场景 ID / 质量属性 | 刺激与环境 | 架构响应 | 度量/证据及状态 |
|---|---|---|---|
| QA-01 数据完整性 | 用户把输出根目录设为 origin 子目录、软链接别名或旧 RunId | 拒绝写入；原数据不变 | 原始 582 MAT 与源 .m 哈希不变；TC-NF-SAFETY-001，设计已定、未测 |
| QA-02 可复现性 | 相同固定 N×6 流场进入 Franz、KvD、Adaptive | 输出独立步骤/中间量与状态；RNG 不影响固定样本 | 逐轮向量/矩阵误差表；DEC-005 容差待核 |
| QA-03 数据格式可恢复性 | MAT 变量缺失、压缩块损坏、shape 错误 | Reader 返回带文件/变量上下文的错误，不覆盖既有结果 | 正常归档路径 582/582 已由探针验证；损坏输入负例与正式 Reader 的 TC-F-MAT-* 仍未测 |
| QA-04 会话隔离 | 两个 Adaptive RunContext 交替执行，刷新频率不同 | akkoeff、mitkoeff、RNG、输出目录互不污染 | TC-NF-SESSION-001；同 Session 多线程安全**不在承诺范围** |
| QA-05 无图形执行 | 只安装数值依赖且无 DISPLAY/绘图库 | inspect、run、analyze 仍独立运行 | TC-NF-HEADLESS-001，未测 |
| QA-06 可维护性 | 增加采样器或噪声模型实现 | 不修改 KvD 和 Adaptive 数学公式，不引入依赖环 | 接口替身与 target DAG 审查，未测 |
| QA-07 Debian/Ubuntu 适配 | 在所声明支持的 GNU/Linux 版本部署 | 依赖安装、CMake 源外构建与 CLI 可复现 | 发行版/编译器矩阵见 TEST_PLAN；DEC-006 版本下限待实测 |
| QA-08 性能与资源 | 使用原作者最大配置进行 600 点/大量视线仿真 | 完整记录 wall/CPU/RSS、输入规模与失败数量 | 只定义测量方案，数值上限与加速比 TBD、不可通过性能验收 |

## 8. 架构决策记录（ADR 草案）

| ADR | 决策（与原 MATLAB 和 SRS 对应） | 状态 | 拟审查责任人与批准记录 |
|---|---|---|---|
| ADR-001 | 原始 MAT 只读、matio/zlib 直接读取、不开发 NPZ | **实测**：独立 matio 探针读取 582/582 MAT v5/double/zlib，失败 0；当前归档无需 HDF5。正式 Reader/Writer 合同未冻结 | [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md) 作为数据依据，项目所有者待批准支持范围 |
| ADR-002 | Debian/Ubuntu、C++17/Eigen/CMake | 用户已定方向；具体发行版/编译器版本待 DEC-006 | 维护者提交实际 build 矩阵，所有者批准 |
| ADR-003 | 值语义/RAII/窄接口/组合优先；纯数学自由函数 | HLD 提议，未冻结 | P0.7 对象设计评审者 + 所有者 |
| ADR-004 | Adaptive StepSolver 与跨帧 Session 分离；KvD 双入口保真 | HLD 提议，未冻结 | 数值/接口评审者 + 所有者 |
| ADR-005 | 分析与可选绘图隔离，核心无图形运行 | 方向已定、后端待选 | 分析模块维护者 + 所有者 |
| ADR-006 | SHRotate/Rodrigues 第三方来源单独审查 | 待 DEC-003/007 | 所有者与版权审查相关方 |
| ADR-007 | 固定输入原 MATLAB 行为优先于“改善”数值稳定性 | 方向已定；测试/容差未冻结 | 数值验证负责人 + 所有者 |
| ADR-008 | target DAG 分层及公共/私有边界以 §4 为候选；明确 contracts 声明 target 与 matio 实现 target 分离 | 未验证实际编译、链接或跨 target 替身，无完成记录 | P0.8 架构审查者 + 所有者 |

**批准登记格式**：ADR ID、关联 REQ-DC/NF、批准人/日期、批准范围、证据文件与源 Git SHA、仍未解决的风险。表中角色只是拟议责任人，并非已有签名或授权记录。只有项目所有者明确确认后才可标为“架构已冻结”。

## 9. HLD 评审冻结清单

**检查项由 P0.8 及项目所有者审查；下列项均未验收。**

- [ ] 原始 45 个 .m → 模块/对象/无状态函数映射可追踪；**582 MAT 的 matio 全量字典已生成**，但来源身份审查、负例、生产 Reader 兼容性与 P10.4 复核仍需完成。
- [ ] 每个公共领域值对象、误差类型、抽象接口有明确约束、方向、所有权和必要的不变量。
- [ ] CLI 参数 schema、错误码、RunId 与输出不可覆盖合同被批准。
- [ ] CMake target 依赖图没有环，模块公共/私有头文件边界被批准。
- [ ] Adaptive Session 的时序和重置策略以原函数调用顺序交叉核对。
- [ ] 球谐基函数/旋转约定、数值容差与黄金测试数据来源有证据。
- [ ] 第三方版权、性能指标的未决事项明确列出阻塞状态和批准人。
- [ ] 确认源外构建、平台要求与 [AGENTS.md](../AGENTS.md) 无冲突。

**当前 HLD 仍为草案；不因创建本文件而通过任一检查项。**
