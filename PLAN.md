# Egomotion Estimation with Dense Flow Fields：MATLAB → C++ 移植计划

## 一、总体目标与技术决策

采用忠实移植（Faithful Port）策略，以 Strübbe、Stürzl 和 Egelhaaf（2015）的论文和 MATLAB 脚本为依据，将原始数值实验移植为独立 C++ 程序。

| 项目 | 决策 |
|---|---|
| 算法语言 | C++17 |
| 构建系统 | CMake |
| 线性代数 | Eigen 3 |
| MATLAB 数据读取 | matio |
| 原始数据 | 保留 .mat 文件及变量结构 |
| NumPy .npz | 不支持、不转换、不引入 cnpy |
| MATLAB Runtime / Python | 不作为运行依赖 |
| OpenCV | 核心移植不需要 |
| 数值验收 | 对照 MATLAB 和论文实验 |
| 许可证 | 暂不选择项目级 LICENSE |

必须实现 Original KvD、Modified KvD、Original MFA 和 Adaptive MFA 四种算法。输入为已知视线方向和光流向量，不加入图像读取、特征跟踪、VO 后端和传感器接口。

## 二、移植前源码审计

首先取得原始 MATLAB 归档，检查所有 .m、.mat 文件、调用关系、数据格式和版权声明，不猜测未取得的源码内容。

产出：
- docs/SOURCE_INVENTORY.md：文件树、哈希、入口、依赖。
- docs/MAT_DATA_SCHEMA.md：MAT 版本、变量名、类型、维度、结构字段。
- docs/MIGRATION_MATRIX.md：MATLAB 到 C++ 函数逐项对应。

检查 load、save、run、addpath、rng、randn、矩阵求解、停止条件和工具箱依赖。MEX、.p 或无源码组件登记为障碍。

## 三、C++ 工程架构

原始 MATLAB .mat（只读）→ matio 读取与类型校验 → Eigen 数据结构 → 球面光流几何和数值模块 → Original/Modified KvD、Original/Adaptive MFA → 论文实验复现。

建议结构：

~~~text
Egomotion-Estimation-with-Dense-Flow-Fields/
├── CMakeLists.txt
├── CMakePresets.json
├── README.md
├── .gitignore
├── include/egomotion/
│   ├── mat_reader.hpp
│   ├── types.hpp
│   ├── flow_geometry.hpp
│   ├── spherical_harmonics.hpp
│   ├── kvd.hpp
│   ├── matched_filter.hpp
│   └── adaptive_mfa.hpp
├── src/
│   ├── io/mat_reader.cpp
│   ├── geometry/flow_geometry.cpp
│   ├── geometry/spherical_harmonics.cpp
│   ├── algorithms/kvd.cpp
│   ├── algorithms/modified_kvd.cpp
│   ├── algorithms/matched_filter.cpp
│   ├── algorithms/adaptive_mfa.cpp
│   └── main.cpp
├── experiments/
│   ├── kvd_bias.cpp
│   └── adaptive_mfa_simulation.cpp
├── tests/
│   ├── test_mat_reader.cpp
│   ├── test_flow_geometry.cpp
│   ├── test_kvd.cpp
│   ├── test_mfa.cpp
│   └── test_reproduction.cpp
├── data/original/
└── docs/
    ├── SOURCE_INVENTORY.md
    ├── MAT_DATA_SCHEMA.md
    ├── MIGRATION_MATRIX.md
    └── NUMERICAL_EQUIVALENCE.md
~~~

目录中的 C++ 文件名是拟创建的文件，不是原始 MATLAB 文件名。程序应接受任意 MAT 路径，不依赖固定目录。公开原始数据前核实版权与文件大小。

## 四、MAT 文件直接读取

matio 是唯一 MAT 文件解析库。先实现拟议命令：

~~~bash
egomotion inspect data/original/example.mat
~~~

列出文件版本、变量、类型、维度、压缩、复数、struct/cell、错误及不支持的类型。

读取要求：只读、不修改源文件；检查 rank、dims、class_type、data_type、复数标志、数据长度；RAII 管理资源；保留 MATLAB 列主序、变量名和维度；不静默转置或改变 single/double；显式处理 struct、cell、逻辑、复数和稀疏矩阵。

zlib 支持压缩 MAT v5，HDF5 支持 v7.3；默认启用 zlib，HDF5 需求待原始数据审计决定。

Eigen 矩阵建议明确列主序：

~~~cpp
using MatrixXd = Eigen::Matrix<
    double, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor>;
~~~

注意 1-based/0-based 索引、reshape、线性索引、隐式扩展和三维以上数组。

## 五、算法移植顺序

**阶段 1：球面光流几何基础。** 实现单位视线、球面切向投影、平移与旋转光流模板、叉乘矩阵、噪声生成，统一坐标系、符号和单位。

**阶段 2：Original KvD。** 对照论文式（8）—（12）及 MATLAB 源码，实现逆深度、平移、旋转交替迭代、尺度约束、初始化和停止条件。

**阶段 3：Modified KvD。** 按论文第 2.5.2 节实现独立路径，保留与原始 KvD 不同的平移更新公式，尤其是移除额外逆深度因子的修正。

**阶段 4：Original MFA。** 实现六个运动模板、模板响应、6×6 耦合矩阵和参数解耦；检验给定逆深度时与 KvD 相应迭代步的等价性。

**阶段 5：Adaptive MFA。** 实现自适应逆深度、实球谐系数、耦合矩阵更新和更新频率，不提前引入新优化器或鲁棒损失。

逆深度球谐展开：

[
\mu(\vartheta,\varphi)\approx
\sum_{l=0}^{2}\sum_{m=-l}^{l}
a_{lm}Y_{lm}^{\mathrm{real}}(\vartheta,\varphi)
]

零阶 1 个、一阶 3 个、二阶 5 个，共 9 个球谐系数；需区分完整数学表示与论文提到的 8 个可独立更新环境参数。严格遵循论文实球谐归一化、基函数顺序和坐标定义。

## 六、原论文实验复现

| 实验 | 论文 | 内容 |
|---|---|---|
| KvD 偏差实验 | Fig. 1 | 两种视场、噪声、视线数量、Original/Modified KvD 误差 |
| Adaptive MFA 初始化 | Fig. 2 | 固定深度结构条件下多次更新的运动估计误差 |
| Adaptive MFA 轨迹实验 | Fig. 5 | MFA 对比 Adaptive MFA、噪声、运动轴误差、更新频率 |

KvD 实验使用八面体三角面递归细分，完整球面视线数 N=8·4^n；非完整视场移除两个初始三角面，每个视线数量 40 次估计。保持原始距离分布、噪声、初始化和误差统计。

Adaptive MFA 轨迹实验使用球形环境中的正弦轨迹、600 个运动步骤，比较无噪声和加噪情况，以及每 1、5、10、20 帧更新深度模型。正确更新旋转后的参考坐标系；零运动附近无定义角度误差不得写成零。

实验参数首先取自原始 MATLAB 脚本，论文用于交叉核对。

## 七、数值等价性与验收

目标为算法等价、数据一致、实验可复现，而非逐位浮点相同。

| 层级 | 检验 | 通过标准 |
|---|---|---|
| MAT 数据层 | 变量、类型、维度、数值 | 与源文件一致 |
| 算法层 | 逆深度、平移、旋转、模板、耦合矩阵和中间迭代 | 在非退化条件下满足预设容差 |
| 实验层 | Fig. 1、2、5 统计和趋势 | 同配置下复现或符合统计容差 |

优先对照固定输入及中间结果，避免 MATLAB 和 C++ 随机数生成器差异。非退化双精度基准可先以 1e-10 为候选相对容差，再根据条件数、矩阵分解和计算顺序调整。优先使用 QR、LDLT 等适当分解，不把直接求逆作为通用方案。

## 八、实施任务与依赖关系

1. **P0 — 原始 MATLAB 资料审计**：归档、文件、MAT 变量、版权和依赖。
2. **P1 — C++ 工程初始化**：CMake、Eigen、matio、命令行和跨平台依赖。
3. **P2 — 原始 MAT 数据读取闭环**：inspect、按名称读取、类型校验、Eigen 适配。
4. **P3 — 公共数学模块**：球面视线、光流、矩阵、随机生成、误差评估。
5. **P4 — Original KvD 与 Modified KvD**：求解器、偏差实验和数值对照。
6. **P5 — Original MFA**：匹配滤波和 KvD 迭代等价性。
7. **P6 — Adaptive MFA**：球谐模型、参考系旋转、自适应更新。
8. **P7 — 原论文实验复现**：Fig. 1、2、5 的输出、统计与对照。
9. **P8 — 项目文档整理**：README、安装、数据路径、算法对应、复现说明。

P0 是后续工作依据；P1—P3 后 KvD 与 MFA 可相对独立推进；P6 依赖 Modified KvD 和 MFA 公共模块。

## 九、仓库文档及版权处理

README 仍包含旧的 .npz 转换目标。实施时统一改为：

> C++ reimplementation of the original MATLAB scripts, using the original MATLAB MAT data files directly through matio.

不提供 .npz 转换器，不依赖 Python 或 MATLAB Runtime。保留论文和脚本 DOI 署名；ODC-By 1.0 不能自动证明 .m 程序的再分发授权。遵照此前决定，不擅自创建 LICENSE。

## 十、最终完成标准

1. 两组主要实验涉及的可移植 MATLAB 计算流程均有 C++ 实现。
2. C++ 直接读取原始 .mat 文件且不修改。
3. Original KvD、Modified KvD、Original MFA、Adaptive MFA 可独立运行。
4. 关键阶段具有 MATLAB/C++ 函数与公式对应关系。
5. 两组主要实验及 Adaptive MFA 初始化试验可复现，有数值对照。
6. 不依赖 MATLAB、Python、ROS、OpenCV 或 .npz 转换流程。

**计划最终确定：采用 C++17 + Eigen + matio + CMake，忠实移植四种算法、保留原始 MAT 数据、复现论文数值实验。**

目前最关键的前置工作是取得 MATLAB 归档并完成 P0 源码审计，核对实际函数、数据结构和参数，再落实准确的一对一移植清单。
