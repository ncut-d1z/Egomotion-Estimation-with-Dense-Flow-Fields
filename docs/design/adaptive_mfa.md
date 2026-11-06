# Adaptive MFA 详细设计草案

> **状态：待 P0.8/P7/P8 冻结，未有 C++ 实现。** 实际 MATLAB 入口 oneit_adaptiv_matched_filter.m、agent_in_sphere_initialization_test.m、agent_in_sphere_adaptive_matched_filter_test.m、agent_in_sphere_adaptive_matched_filter_many_parameter.m。

## 单步求解器的数据合同（不持有跨帧状态）

输入：旧 3×5 球谐系数和 N×6 流场。输出：新系数、三维平移、三维旋转、残差与数值状态。单步调用应允许不创建轨迹 Session。

按 oneit_adaptiv_matched_filter.m 的实际计算次序：
1. 由 near_aus_koeff 计算 near，执行平均符号与尺度归一（原代码的符号/除法顺序需逐式保留）。
2. 构造由流场得到的六维右端向量与四个 3×3 块，形成 6×6 系统。
3. 按原语义求出 t、ω 并归一 t；MATLAB 使用 real(inv(M))，任何 Eigen 解算优化必须先与原函数作数值对照。
4. 使用 near、t、ω 计算预测光流与 RMS 残差；重算 near，再按二阶实球谐得到新 3×5 系数。
5. 记录零近度均值、零 t、病态系统、非有限输出的异常状态，不以“得到数值”冒充有效解。

## 跨帧 Session 与更新策略

- AdaptiveMfaSession 独占主 akkoeff 与前一帧旋转上下文；每次运行都应新建，不与其他运行共享内存或随机状态。
- 每帧遵守原先后顺序：旋转 akkoeff 和用于比较的系数；根据策略保留/重置/刷新 baseline；计算对比输出；最后执行主 oneit 更新并提交 akkoeff。
- InitializationRunner：35 个圆轨迹点、继承对照与每帧重置对照。
- AdaptiveComparisonRunner：600 个正弦轨迹点、二十面体级别 4、八次重复，可含噪/无噪；输出 allemittransfehler、allemitrotfehler、alleohnetransfehler、alleohnerotfehler、rottransratio 五个 MAT 文件。
- UpdateRateRunner：600 个轨迹点、级别 2、刷新间隔 **1、5、11、21**。历史 many_parameter/ MAT 与当前脚本 save 列表存在差异，必须登记，而非补造。
- 当前 MATLAB 600 点比较中的前四个结果是八次重复平均；rottransratio 仅保留最后一次被覆盖的结果，忠实模式不能未经授权改变统计行为。

## 候选状态流

准备输入 → 上帧旋转 → 对比系数策略 → 计算对比值 → 单步计算主状态 → 提交 akkoeff → 下一帧。异常时是否提交、停止或跳过尚未核验和批准，不能假定采用某种滤波器回滚模型。

## 验收与待冻结

- TC-F-ADAPT-001：单步矩阵块、右端、解、残差、近度与系数的逐项 Golden。
- TC-F-ADAPT-002/003：连续会话旋转/刷新时序、独立状态、1/5/11/21 的准确触发时刻。
- TC-F-EXP-001/002/003：三个实验入口按原参数和各 MAT 写出合同验收；不忽略失败/无定义样本。
- TC-DC-ADAPT-001、TC-NF-SESSION-001：无 Session 的单步测试与两个交替 Session 隔离。
- P0.8 后补齐精确 C++17 签名、前置/后置条件、失败枚举、主状态提交规则、Golden 来源和数值容差。本文件不构成已经完成的 LLD 基线。
