# 七份“正在审计”文件的文档审查完成与 AF-026～027 整改复核（2026-11-08）

> **本次完成的是现存原 MATLAB `.m`、既有需求决定和七份 LLD/参考提案之间的**文档静态数学/索引/运行顺序审查**，不是 MATLAB 运行或 C++/CTest/独立 L3 数值测试。** 对仍缺原论文 PDF、完整独立 L3 或真实运行证据的事项保留明确限制；不能以“已审计”宣称算法/基线已通过。

- 初始审计快照：`e190ed74240bf7b3b32a65c8fb6099fbdedc46b9`，问题先登记为「未整改」的 [扫描报告](AUDIT_SEVEN_MATH_DOCS_SCAN_2026-11-08.md)。
- **整改后定向静态检查快照**：`22bc38635b2350bc9078c2952ba9ad520a55f7b1`。
- 审查对象：`docs/design/kvd.md`、`adaptive_mfa.md`、`harmonics.md`、`sampling_noise.md`、`trajectory.md`、`analysis_plotting.md`、`docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md`。
- 本轮仅修改 `analysis_plotting.md` 和 `GOLDEN_TOLERANCE_PROPOSAL.md` 两份规范/提案；`origin/**` 只读，既有原子整改项 AF-001～025 的历史记录不改写。固定快照及新问题见 [AUDIT_PROCESS.md](../AUDIT_PROCESS.md)。

## 一、逐文件审计结果（现有资料可验证的范围）

| 文件 | 核对原源/数学校验范围 | 结论及保留的未知 |
|---|---|---|
| `docs/design/kvd.md` | `zellendefinieren/franzeigen` 三轴模板、Original/Modified KvD 的 near 和 near² 平移、旋转更新使用旧 t、近度正负翻转、1e−13、迭代下界 6/上界 200、动态 RMS/grenze、末次 near 均值调节 t | **文档现行核心公式/次序与只读源码相符**；没有重新运行独立 L3 全逐轮数据、退化条件或 CTest |
| `docs/design/adaptive_mfa.md` | `oneit_adaptiv_matched_filter` 原 s 的负号处理、M1–M4、6×6 rhs、实数求解、平移归一、返回的 MSE（sqrt 内部变量不返回）、重算 near 的 1e−11、系数拟合；三个 data_generation 入口的主/比较状态/周期刷新 | **文档现行数学/状态说明与可见原源码相符**；错误时 C++ 不提交是用户已选安全合同，并非原 MATLAB 异常行为实测 |
| `docs/design/harmonics.md` | `winkelausvektor`、`kugelflfunc2` 实基相位/角坐标，3×5 九项索引、`near_aus_koeff/koeff_aus_near` 逐项拟合、`SHRotate` R3 元素映射与 R5 当前递推结构、`transfor_koeff` | **已完成原 `.m` 静态合同审查**。Ivanic–Ruedenberg 1996 原文＋1998 勘误全文尚待用户上传；**不得**宣称 R5 原论文逐式核验、25 元素独立 L3 或生产 Eigen 实现已通过 |
| `docs/design/sampling_noise.md` | `getvektor` 两次 rand，八面体及缺损取面，二十面体表，`findeDreiecke` A/B/C/D 顺序及归一，Bias RNG 两次 seed、N 次 rand/3N 次 randn，`fixed_*` 实际 near_last 标量 vs C++ 逐点 nearness 诊断；零噪声调用 randn | **源文件接口、调用次序/采样规则的文档审查完成**，完整 i=1/2 的 L3 有序方向值与固定随机数流仍需独立测试 |
| `docs/design/trajectory.md` | 圆/正弦点列分母 N（不是 N−1）、首末点不填写有效运动、`R^T` 局部转换、叉积归一 1e−11、Rodrigues norm<eps、单位球射线正根倒数 | **文档公式/边界已完成源码静态核对**；Sophus 实现、反向/近 π/零向量与近球边界的真实 L3/CTest 未执行 |
| `docs/design/analysis_plotting.md` | 四类原分析入口、比值阈值 0.01/0.02、固定 begin/end 四组及 zone 条件、`kurveglaetten` 半窗与边缘零值、Bias xwerte/actual N 语义、DEC-004 历史字段 | **现行 MATLAB 分段/平滑合同审查完成**；本轮修复了独立分析 Golden 编号冲突 AF-026。L2 真实数据全曲线独立复算、生产分析器仍未执行 |
| `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | AN-G01～12 解析数学候选与原静态源公式对应、L2/L3/L4 定义、0.01/0.1 首轮筛查与更严格的几何/离散/状态不变量 | **候选参考来源/单位及编号合同审查完成**；本轮修复 AF-027 的错误 L1 来源暗示；尚未在真实数值证据下验证全部 12 个候选，不能批准 DEC-005 最终阈值 |

## 二、先登记后整改：两项 AF 结案

| 编号 | 修改文件 | 源冲突与修订 | 固定整改快照的独立复核 |
|---|---|---|---|
| **AF-026** | `docs/design/analysis_plotting.md` | 原分析测试表的 `AN-G01～AN-G06` 与全局 `analytic_golden_candidates.v1.json` 的 **AN-G01～AN-G12** 撞号且含义不同；现将六条分析场景唯一命名为 **`APL-G01～APL-G06`**，保留原 TC/AC、输入与公式，并在 LLD §7 和总容差提案明确命名空间 | **六个 APL-G ID 各出现一次，分析表已无原 AN-G 行；全局 AN-G01～12 的原 JSON ID 和提案 12 条均保留**；未生成 CTest |
| **AF-027** | `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` | 原先 `x_ref` 称为“原 MATLAB 数值”，易隐含必须运行原 MATLAB 收集 L1；现在明确为**可追溯的 L2 历史 MAT 数值或独立 L3 数值参考**，L4 统计单独处理，**不更改**候选 0.01/0.1 及逐元素不等式 | 旧 “原 MATLAB 数值” 首轮来源表述已移除；文档保留 L2/L3/L4 和来源 SHA、独立参考约束，未宣称 L3 运行/数值验证 |

**定向复核结果**：在 `22bc38635b2350bc9078c2952ba9ad520a55f7b1` 对 AF-026/027 及 7 份在审文件的约束关键字/相对文件链接/代码围栏作连接器只读检查：**7 文件、53 条相对链接缺失目标 0、围栏未配平 0、定向问题检查 0**。这属于**文档层面的静态核对**，不是对所有数值候选逐元素验算。

## 三、完成文件审计后，仍不得视为完成的独立证据

- **未取得**：用户尚未上传 Ivanic–Ruedenberg 1996 年原始论文与 1998 年勘误 PDF，故无法核对真正论文递推及勘误表内容；H-10/H-11 仍限制生产实现/真实运行。
- **未执行**：Matio 生产 Reader/Writer、CMake/CTest/Bash、MATLAB/Octave、实际 Euclidean/球面方向 i=1/2 全序列 L3、KvD/Adaptive 逐帧数值、R3/R5 独立全元素或完整历史 MAT 曲线独立重算。历史 `MAT_DATA_SCHEMA.md` 的 582/582 READ_OK 仍是**此前探针证据**，不是本轮重新读取。
- **未批准**：DEC-005 正式模块 PASS 容差、DEC-006 资源/性能、正式 SRS/HLD/LLD 编码/验收基线、PR 合并与发布。用户已经批准的探索 0.01/0.1 不能替代最终通过阈值。
- **状态语义**：根据 [AUDIT.md](../AUDIT.md) 中“已审计＝指定文档检查范围完成、非测试/批准”的定义，以上七份在审文件**可更新为“已审计（现有 .m 源码与文档静态核对）”**，但剩余数学数值与论文证据缺口继续在 `PROGRESS`/测试计划/用户材料 `TODO` 中留存，**不能把它们从实际验收清单删除**。

