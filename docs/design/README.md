# 分模块详细设计（LLD）索引、完成度与冻结标准

> **设计状态：草案，未冻结、未编译、未验证。** 索引中的文件已实际创建，但其中的候选 C++17 接口、数学步骤、测试路径和错误枚举没有对应已实现代码。源 MATLAB 文件和已保存 MAT 仍由 origin/** 只读保存。

## 文档职责

- [REQUIREMENTS.md](../REQUIREMENTS.md)：SRS 中 REQ/AC/TC 与可验证行为的唯一来源。
- [ARCHITECTURE.md](../ARCHITECTURE.md)：HLD 组件边界、target DAG、部署和 ADR 的唯一来源。
- [OBJECT_MODEL.md](../OBJECT_MODEL.md)：类职责、所有权及 45 MATLAB 文件→拟议方法的唯一维护位置。
- [TEST_PLAN.md](../TEST_PLAN.md)：Golden、证据、测试环境与候选 CTest 的唯一来源。
- [PLAN.md](../../PLAN.md)：98 个 P0–P10 最小工作项、任务依赖、风险与里程碑。
- 本目录每份文档是**对应模块的 LLD 草案**，具体签名、公式、错误处理、数据布局应在 P0.8 以后逐个确认。

## LLD 模块覆盖矩阵

| 模块 | 对应文档 | 源 MATLAB 范围 | 数学/数据约定 | 候选 API | Golden/实测 | 冻结状态 |
|---|---|---|---|---|---|---|
| MAT I/O | [mat_io.md](mat_io.md) | getflowfield、各入口 load/save、582 MAT | 类型边界/路径安全已描述，全量字典缺失 | 有待核准的签名/错误码 | 未测 | 阻塞 DEC-001/002 |
| 基础采样与噪声 | [sampling_noise.md](sampling_noise.md) | findeDreiecke、各采样/流/噪声函数 | 已列数量、顺序、噪声方程 | 候选接口 | 未测 | 未冻结 |
| 几何与轨迹 | [trajectory.md](trajectory.md) | Kreis/Sinus/trajectory、球内近度与旋转 | 已列离散轨迹/求交公式 | 候选接口 | 未测 | 第三方 Rodrigues 来源待审 |
| Franz / KvD | [kvd.md](kvd.md) | zellendefinieren、franzeigen、两种 KvD、误差 | 已列各平移公式、RMS、终止规则 | 候选接口 | 未测 | 阻塞 DEC-005 |
| 实球谐 | [harmonics.md](harmonics.md) | kugelflfunc2、near/koeff、SHRotate | 已列 9 项索引/公式及旋转块合同 | 候选接口 | 未测 | 阻塞 DEC-003/005/007 |
| Adaptive 单步/Session | [adaptive_mfa.md](adaptive_mfa.md) | oneit、3 个 Adaptive 入口 | 已列 M1–M4、RHS、MSE 及会话差异 | 候选接口 | 未测 | 阻塞 DEC-005 |
| 实验 Runner | [experiments.md](experiments.md) | 2 Bias + 3 Adaptive 数据生成入口 | 已列参数与各 save 文件 | 候选接口 | 未测 | 数据字段需核验 |
| 分析与可选绘图 | [analysis_plotting.md](analysis_plotting.md) | 4 分析入口、kurveglaetten、3 插图 | 已列数据/索引/阈值/窗口 | 候选接口 | 未测 | Golden 及绘图后端未定 |

覆盖范围是**模块文档存在性**，不是代码/接口实现比例。无论数值公式写得多完整，缺少 Golden 和批准容差就不能把相应模块标为已完成。

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

MAT 数据的 **582 文件全量变量字典**、两份外部来源代码的权利核查、CLI/error schema、正式数值容差、真实 Golden 与 CTest 名称、Debian/Ubuntu 实测平台矩阵均未闭环。因此，本目录八份模块设计可以用于进一步评审和实施准备，但均**不是正式冻结的 LLD 编码基线**。
