# 分模块详细设计（LLD）索引、完成度与冻结标准

> **设计状态：草案，未冻结、未编译、未验证。** 索引中的文件已实际创建，但其中的候选 C++17 接口、数学步骤、测试路径和错误枚举没有对应已实现代码。源 MATLAB 文件和已保存 MAT 仍由 origin/** 只读保存。

## 文档职责

- [REQUIREMENTS.md](../REQUIREMENTS.md)：SRS 中 REQ/AC/TC 与可验证行为的唯一来源。
- [ARCHITECTURE.md](../ARCHITECTURE.md)：HLD 组件边界、target DAG、部署和 ADR 的唯一来源。
- [OBJECT_MODEL.md](../OBJECT_MODEL.md)：类职责、所有权及 45 MATLAB 文件→拟议方法的唯一维护位置。
- [TEST_PLAN.md](../TEST_PLAN.md)：Golden、证据、测试环境与候选 CTest 的唯一来源。
- [PLAN.md](../PLAN.md)：98 个 P0–P10 最小工作项、任务依赖、风险与里程碑。
- [PROGRESS.md](../PROGRESS.md)：阶段及里程碑的实际证据、角色/批准状态、缺失退出条件；已核对的静态审查另见 [2026-11-06 记录](../reviews/STATIC_TRACEABILITY_2026-11-06.md)。
- [contracts.md](contracts.md)：C++17 领域值类型、Result<T,E>、错误分类、资源生命周期与跨模块接口的**统一候选合同**。
- 本目录每份文档是**对应模块的 LLD 草案**，具体签名、公式、错误处理、数据布局应在 P0.8 以后逐个确认。

## LLD 模块覆盖矩阵

| 模块 | 对应文档 | 源 MATLAB 范围 | 数学/数据约定 | 候选 API | Golden/实测 | 冻结状态 |
|---|---|---|---|---|---|---|
| 跨模块 C++17 合同 | [contracts.md](contracts.md) | SRS/OBJECT_MODEL/全部模块 | 值类型/错误/线程与所有权语义已列，需批准 | Result/Status 候选 | 未测 | 未冻结 |
| CLI 参数与错误协议 | [cli.md](cli.md) | 五个实验入口、inspect、analyze、可选 plot | 按入口列出参数/合法范围、退出码与诊断字段 | 候选命令/错误 schema | 未测 | 阻塞 P0.8/P1.2、DEC-008 |
| MAT I/O | [mat_io.md](mat_io.md) | getflowfield、各入口 load/save、582 MAT | **582/582 真实 v5+zlib/double 类型字典已获得**；生产 Reader/Writer、路径安全仍待验收 | 候选签名/错误码 | **独立探针已执行，非 CTest** | DEC-001/002 格式事实已证实，接口未冻结 |
| 基础采样与噪声 | [sampling_noise.md](sampling_noise.md) | findeDreiecke、各采样/流/噪声函数 | 已列数量、顺序、噪声方程 | 候选接口 | 未测 | 未冻结 |
| 几何与轨迹 | [trajectory.md](trajectory.md) | Kreis/Sinus/trajectory、球内近度与旋转 | 已列离散轨迹/求交公式 | 候选接口 | 未测 | 第三方 Rodrigues 来源待审 |
| Franz / KvD | [kvd.md](kvd.md) | zellendefinieren、franzeigen、两种 KvD、误差 | 已列各平移公式、RMS、终止规则 | 候选接口 | 未测 | 阻塞 DEC-005 |
| 实球谐 | [harmonics.md](harmonics.md) | kugelflfunc2、near/koeff、SHRotate | 已列 9 项索引/公式及旋转块合同 | 候选接口 | 未测 | 阻塞 DEC-003/005/007 |
| Adaptive 单步/Session | [adaptive_mfa.md](adaptive_mfa.md) | oneit、3 个 Adaptive 入口 | 已列 M1–M4、RHS、MSE 及会话差异 | 候选接口 | 未测 | 阻塞 DEC-005 |
| 实验 Runner | [experiments.md](experiments.md) | 2 Bias + 3 Adaptive 数据生成入口 | 已列参数与各 save 文件 | 候选接口 | 未测 | 数据字段需核验 |
| 分析与可选绘图 | [analysis_plotting.md](analysis_plotting.md) | 4 分析入口、kurveglaetten、3 插图 | 已列数据/索引/阈值/窗口 | 候选接口 | 未测 | Golden 及绘图后端未定 |

覆盖范围是**10 份文档：1 份跨模块合同（contracts.md）和 9 份模块 LLD（含 cli.md）**的存在性，不是代码/接口实现比例。无论数值公式写得多完整，缺少 Golden 和批准容差就不能把相应模块标为已完成。

## 模块 LLD 的标准模板与通过条件

每份 LLD 在进入正式编码前应具有：

1. **来源**：每个实际 MATLAB 函数的完整路径、原始 blob SHA、函数调用关系和可能的第三方来源。
2. **精确合同**：候选 C++17 类/方法最终签名、所有权/异常/复制移动/线程边界、输入输出单位与 shape。
3. **算法**：逐步公式或伪代码、固定顺序、数值阈值、源 MATLAB 与 C++ 索引差异；对原版明显异常单独记录。
4. **资源**：谁创建、谁销毁、谁可写；原始 MAT 不可写、输出新目录不可覆盖、RNG/Session 不跨运行共享。
5. **测试**：REQ→AC→TC→实际测试文件/函数/CTest、Golden 输入输出 SHA、批准容差、正常/异常路径和真实测试日志。
6. **构建**：target、公开/私有头文件、依赖库/编译器版本，避免逆向依赖。
7. **批准**：项目所有者/维护者批准、Git SHA、日期、开放风险及 DEC 处理决定。

### 进度状态字典

- **草案已创建**：已有文字设计，但不能据此创建“完成”百分比。
- **待核验**：来源、MAT 字典、算法细节、接口或 Golden 缺失。
- **可评审**：必填字段已准备且所有待决事项明确；尚不表示通过。
- **已冻结**：审阅人批准具体 SHA、合同、Golden、容差和开放例外。
- **已实现/已测试**：分别需要代码提交及实际执行证据，是与设计冻结独立的两个状态。

## 当前阻塞汇总

MAT 数据的 **582 文件全量 matio 字典已实际生成并提交**，包含 582 个 `READ_OK`、0 失败，以及 MATLAB class/data_type/dims/压缩和文件指纹。未完成的事项是生产 Reader/Writer/写回读、负例、P10.4 独立复核、两份第三方代码版权确认、CLI/error schema、数值 Golden 与正式容差，以及完整 Debian/Ubuntu 业务构建矩阵。因此，本目录除跨模块合同外的 **9 份模块 LLD** 可以用于进一步评审和实施准备，但均**不是正式冻结的 LLD 编码基线**。


## LLD 证据就绪度与源 MATLAB 核查重点

| 文档 | 已文档化的可执行设计检查点 | 尚缺的冻结证据 |
|---|---|---|
| contracts.md | FlowField N×6、3×5 近度系数、Result/MatError/NumericalError 候选及生命周期 | 代码内实际类型、错误码批准、线程合同测试 |
| mat_io.md | MAT v5+zlib/double 类型与形状已实测，选定探针环境 matio 1.5.26；路径保护和 Reader/Writer 候选 API | **仍缺**业务接口、写入/回读、安全负例、Golden 按值检查与 P10.4 复核 |
| cli.md | 命令、参数/类型、固定实验语义、失败类别及字段候选已表格化 | RunId 自动命名、JSONL/键值格式、部分参数合法边界待批准；真实 CLI/CTest 不存在 |
| kvd.md | 原始/修正平移与旋转公式、最低/最大迭代、逐轮 trace 字段 | MATLAB 逐轮 Golden、病态边界、正式容差 |
| harmonics.md | 9 项 l≤2 实球谐公式、3×5 索引、R3 元素、R5 递推结构 | R5 完整合法实现、外部版权、Golden 与精度 |
| adaptive_mfa.md | M1–M4、rhs、MSE 返回、Frame/Session 更新时序差异 | 逐节点 Golden、异常提交决定、容差 |
| sampling_noise.md | 三种采样顺序、噪声方程、rand/randn 调用顺序、source seed 重置 | 固定 RNG Golden 与样本全序核对 |
| trajectory.md | 圆/正弦公式、方向→旋转奇异性、Rᵀ 局部运动、球内求交 | 旋转矩阵 Golden、零模/反向边界、Rodrigues 权利 |
| experiments.md | 五个 Runner 原参数及输出、历史 MAT 的 double/double 和五种实测 shape、Bias 原 xwerte 与真实 N 不同 | **仍缺**当前/历史输出差异归因、RunContext、C++ 新结果测试 |
| analysis_plotting.md | 四分析入口与历史 MAT 的真实 shape、1-based 索引、分段阈值、半窗平滑 | **仍缺**数值曲线 Golden、绘图后端批准及 C++ 分析测试 |

### 评审与完成状态的强制区分

- “接口/算法已文档化”只表示具备静态评审材料；不等于签名可编译、Golden 已存在或数值结果正确。
- “MATLAB 原代码具有某种边界行为”应标明它来自**源文件的静态阅读**还是实际 MATLAB 数值执行，不能混淆。
- MATLAB 原始函数、原始 MAT 文件和版权 notice 一律不在文档维护任务中更改；尤其不能通过改动已有 MAT 获得“通过”。
- 模块签名只有在 SRS/HLD、[contracts.md](contracts.md)、源 MATLAB、CLI/error schema、真实 MAT 数据合同之间一致且项目所有者批准后才能冻结。批准之前不应称这些草案为“编码基线”。

## 本期未完成的先决工作

1. **已完成的独立子能力**：582/582 MAT 已通过 matio 解压/变量元数据检查；**尚待** P1 生产 Reader/Writer 按值加载、错误负例及 P10.4 独立结果审核。
2. P0.4/DEC-003/007：rodrigues.m、SHRotate.m 的第三方版权及可移植实现范围。
3. P0.8/DEC-008：统一 C++17 API、状态/错误、CLI schema、组件 target 和测试实体的批准。
4. DEC-005/006：固定源 MATLAB Golden、逐模块容差、目标 Debian/Ubuntu 环境、wall/CPU/RSS 预算。
5. P1–P10：真实 C++ 类、CTest、运行结果及按 TC/AC 记录的证据。

**当前模块 LLD 均未冻结**：独立 C++17 matio 扫描器确已编译运行，但无生产业务模块、CTest、逐元素 Golden 或算法数值实验。不能将数据字典扫描通过扩大为整项目或 LLD 验收通过。
