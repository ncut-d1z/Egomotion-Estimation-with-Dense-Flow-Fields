# 详细设计（LLD）文档索引

> **状态：预设计草案，所有模块均未冻结、未实现。** 本目录仅记录从真实 MATLAB 源码已经核实的计算步骤、数据约定与待落实的详细设计合同。不能把这些材料当成已有 C++ 类、CMake 目标、测试或性能结果。

## 文档分工

- [../REQUIREMENTS.md](../REQUIREMENTS.md)：系统应满足什么行为；REQ、AC、TC 与状态（SRS v0.2）。
- [../ARCHITECTURE.md](../ARCHITECTURE.md)：HLD，模块、依赖、对象和运行时数据流。
- [../OBJECT_MODEL.md](../OBJECT_MODEL.md)：值对象、不变量、所有权和类的候选关系。
- [../../PLAN.md](../../PLAN.md)：P0–P10 的 98 项实施任务、依赖与审计证据。
- [../TEST_PLAN.md](../TEST_PLAN.md)：测试实体、Golden 参考、测试层级与候选容差。

## 分模块详细设计草案

| 文件 | 主要 MATLAB 来源 | 当前状态 | 相关 REQ |
|---|---|---|---|
| [mat_io.md](mat_io.md) | getflowfield、load/save、582 个 MAT | 设计待冻结 | F-MAT、D-MAT、I-MAT、DC-IO |
| [kvd.md](kvd.md) | franz、norm_koenderink、mue_norm | 设计待冻结 | F-FILTER、F-KVD、F-ERROR、DC-KVD |
| [harmonics.md](harmonics.md) | kugelflfunc2、SHRotate、near、koeff | 设计待冻结；SHRotate 版权待核 | F-SH、D-SOURCE |
| [adaptive_mfa.md](adaptive_mfa.md) | oneit、三个 sphere experiment | 设计待冻结 | F-ADAPT、F-EXP、DC-ADAPT |

其他低阶设计（采样/噪声、轨迹、实验 runner、结果分析、绘图）将在对应 P2/P3/P6/P8/P9 开始前按同一模板补齐。本轮没有为它们编造未经核验的方法签名。

## 正式 LLD 冻结的最低格式

每个模块最终均必须填写：
1. 对应源 MATLAB 完整相对路径、源 Git SHA、公式或行号、已核对的输入/输出数据形状与单位。
2. 经批准的 C++17 类型、精确头文件/API 签名、构造依赖、所有权、复制/移动和线程安全约束。
3. 前置条件、后置条件、不变量、错误状态枚举、异常/失败处理及资源释放。
4. 可审查的算法伪代码或关键公式、索引约定、符号与数值顺序、与原版差异（如有）。
5. 关联 REQ/AC/TC、拟建测试文件、固定 Golden 数值、批准容差、已测结果。
6. 构建 target、依赖版本、运行示例、跨模块序列和 ADR 归属。
7. **冻结审批**：项目所有者批准、commit SHA、被阻塞/未验证条目。审批前绝不标“完成”。

本目录不强制所有数学运算都实现为类；对纯函数保留命名空间自由函数的做法参照 HLD。
