# Franz / KvD 求解器详细设计草案

> **状态：待 P0.8 和 P4 冻结。** 原始函数包括 zellendefinieren.m、franzeigen.m、norm_koenderink_iteration.m、mue_norm_koenderink_iteration.m、skalarprodukt.m、kreuzprodukt.m、transfehler_div.m、normrelfehler.m。

## 输入、输出与状态

- 输入 setofpoints：N×6，每行 d(3) + p(3)，后者为三维切向光流。
- FranzMatchedFilter 接收该流场，计算三轴模板的平移/旋转响应及归一化，输出 t、ω，不持有跨次调用的近度状态。
- OriginalKvDEstimator / ModifiedKvDEstimator 接收流场，各自输出 t、ω、near、迭代次数和残差/状态。两者共享真正相同的迭代步骤，但平移更新方程不同。
- 初始 t、ω 由 franzeigen 给出；t 初始归一化。near 根据当前方向/流和 t、ω 重算，并按原作者的符号处理规则调整。

## 迭代时序（按原 MATLAB，不是新算法）

1. 基于现有 t、ω 重算各观测的 near，并处理平均符号为负时的翻转。
2. 根据**所选具体 KvD 更新规则**计算 t_new。Original 含对应近度权重，Modified 则按 mue_norm_koenderink_iteration 的另一公式。
3. 计算旋转更新 ω。源码对旧 t 的使用时机有明确顺序，必须保存，不得提前用 t_new 替换。
4. 提交 t_new，使用 near、t、ω 预测流场并计算 RMS 残差。
5. 检查原 MATLAB 的动态误差门限与绝对阈值，保留至少六次、常规上限 200 次的循环语义及最终按 near 均值缩放 t 的输出行为。
6. 记录每轮 t、ω、near、残差与停止原因，以便固定输入对照。

本文件暂不写出未经再次逐式审查的完整数学展开，具体公式应引用原 MATLAB 源码及论文式号后进入可复核 LLD。任何奇异/零范数保护与原 MATLAB 行为的差异须分开标注。

## 设计对象与拟议职责

- FranzMatchedFilter：三轴模板响应与归一化。
- KvDIterationEngine：公共近度、旋转、残差、停止条件；不得隐藏与公式有关的执行顺序。
- IKvDTranslationRule：**有实际替换需求才采用的内部策略**；Original、Modified 对外仍应是两个独立求解器入口。
- 纯数学辅助操作保留自由函数。单个数据/时间段内求解尽量不持久化可变状态。

## 验收矩阵

- TC-F-FILTER-001：固定 N×6 输入逐项比较 Franz 模板与结果。
- TC-F-KVD-001 / 002：两个算法的每轮中间值与停止时刻，不可用最后一轮结果代替。
- TC-F-ERROR-001：尺度对齐向量误差不等于角度误差；零模向量时不伪造零误差。
- TC-DC-KVD-001：算法差异明确且无跨算法状态串扰。
- TC-NF-NUM-001：逐轮差异容差需 DEC-005 批准，当前 1e-10 仅候选。

## 待冻结

精确 C++17 方法签名、角度/方向类型、异常枚举、分解方法、算法状态转移、每轮 Golden 样本与实际 CTest 文件名。**不进行编译或测试即不能验收本文件。**
