# MAT 结果分析与可选绘图——详细设计草案

> **LLD v0.14草案（DEC-008-04完整ErrorFrame包装链已同步；未执行Analyzer/可视化程序）。** 来源：bias_test/data_analysis/graphs_bias_test.m、adaptive_matched_filter_test/data_analysis/ 下三份 graph_*.m、help_functions/kurveglaetten.m、illustrations/ 下三份 .m。对应 P5.5、P9、REQ-F-BIAS-003、REQ-F-AN-001/002。

## 1. 只读分析合同

| 原分析 | 输入 MAT 字段 | 数值操作与期望数据 |
|---|---|---|
| graphs_bias_test | 目录 `k=1…number_of_cores` 的 xwerte、ywerte3…6（ywerte1/2 在源脚本中注释） | **仅四个 ywerte 累加后除运行数**；xwerte 在每轮直接覆写，最后使用 **k=number_of_cores 的 xwerte**，并据其计算 `100/sqrt(xwerte)`；绘图行 3…(iterationnumber+1)，**一个横轴和一个纵轴都取对数的 `loglog` 单坐标轴**（不是 `plotyy` 双纵轴） |
| graph_agent_in_sphere_initialization_test | allemittransfehler、allemitrotfehler；也读取 ohne* | mittrans 第 1–13 项与 mitrot 全序列（semilogy）；其他读取字段不意味着都参与该图 |
| graph_agent_in_sphere_adaptive_matched_filter_test | allemittransfehler、allemitrotfehler、alleohnetransfehler、alleohnerotfehler、rottransratio | x=50:550；按 rottransratio <0.01 分段；旋转/平移角度误差与右侧比值双纵轴、对数刻度 |
| graph_agent_in_sphere_adaptive_matched_filter_many_parameter | allemittransfehler、allemitrotfehler、rottransratio | x=50:550；阈值 0.02 分段，四个更新率曲线；平滑窗口半宽 2/5/10 |

kurveglaetten.m 使用窗口内均值；原代码在头尾保留零值。该行为必须先核对，不得为了平滑曲线外观更好而改成延拓/镜像边界。

## 2. 结构化曲线数据（候选接口）

建议 Analyzer 输出包含：
- curve_id、**所有实际贡献的 MAT 输入列表**（逐项 path/variable/所属 k、贡献角色与可核验的原始MAT SHA-256）、**独立的原分析 .m Git blob SHA**、source_index_start/end、axis-x、axis-y、axis-unit、raw_values、smoothing_window；不能用单个source_mat_path代替跨RunId聚合输入；MAT SHA-256当前缺失时要明确未知，不得由FNV64或.m的Git blob冒充；
- 对分段值另带 segment_start/end、threshold、invalid_flag，输出明确有效样本数及数据缺失原因；
- 图像后端只消费这些不可变值，不负责重新读取 MAT、选择随机种子或触发求解。

~~~cpp
// 候选 LLD 约束，不是已实现类。
struct CurveSeries;
struct AnalysisDataset;
enum class AnalysisErrorCode;
enum class PlotErrorCode { BackendUnavailable, InvalidSeries, OutputForbidden, WriteFailed }; // 仅本模块定义；exit/stage 见 contracts §3.1
class BiasResultAnalyzer {
public:
    Result<AnalysisDataset, AnalysisErrorCode> aggregate(
        const std::filesystem::path& read_only_result_root) const;
};
class AdaptiveResultAnalyzer {
public:
    Result<AnalysisDataset, AnalysisErrorCode> analyze(
        ExperimentKind kind, const std::filesystem::path& read_only_result_root) const;
};
class IPlotExporter {
public:
    virtual ~IPlotExporter() = default;
    virtual Status<PlotErrorCode> exportPlot(
        const AnalysisDataset&, const std::filesystem::path& output_file) = 0;
};
~~~

实际签名需在模块间的值对象、异常枚举和 CLI 子命令冻结后确认；IPlotExporter 为可选组件，不应强迫 numerical-only 构建携带绘图库。

## 3. 展示脚本与选择性图形复现

- circle_curve_in_the_sphere：绘制单位球与圆轨迹；
- sinosodial_curve_in_the_sphere：绘制单位球与正弦轨迹；
- spherical_harmonic_illustration：绘制由原脚本定义的**未归一化关联勒让德项与 cos 方位项形成的径向示意面**；**不能**直接拿求解器的九项归一化实球谐基函数图替代。
- 图形几何和曲线数值/范围需与原 MATLAB 相符；无需声称像素级一致。禁止引入 Python/MATLAB Runtime。
- 若无法使用 gnuplot 或其他可选后端，analyze 命令必须仍能完成数值输出，且清楚报告图形不可用状态。

### 3.1 插图专用数学/显示合同与源异常（AF-090/091，未冻结）

本段只审查原作者 **`illustrations/`** 下三份可选展示脚本；它们与 `help_functions/kugelflfunc2.m` 的数值球谐计算、`near_aus_koeff.m` 的重建、轨迹数据生成以及 `AnalysisDataset` 的只读分析是**不同的职责**。不得以插图公式反向更改核心数学模块，亦不得因绘图依赖不可用阻塞不含绘图的 `analyze`。以下源行是**只读静态证据**，不是实际 MATLAB 运行或独立 L3。

| 源脚本与定位 | 原始数值/显示行为 | 必须保留的原版区别 |
|---|---|---|
| `spherical_harmonic_illustration.m:10–30` | `delta=π/40`；极角 `θ=0:delta:π`（41 值），方位角 `φ=0:2delta:2π`（41 值）；`meshgrid` 形成角网格。取 `P_degree^order(cosθ)` 即 MATLAB `legendre(degree,cos(theta(:,1)))` 的第 `order+1` 行，沿方位角重复并乘 `cos(order·φ)`，得到 `yy`；随后 `scale=max(max(abs(yy)))`、`rho=5+3·yy/scale`，再按 `x=rho·sinθ·cosφ`、`y=rho·sinθ·sinφ`、`z=rho·cosθ` 生成面。 | **没有** `kugelflfunc2` 的 `A_{l|m|}` 正交归一化，也没有其 `m<0` 对应的 `sin(|m|φ)` 分支；参数 `order` 在源脚本里被**覆写为幅值 scale**，不是新增球谐阶数。只承诺源直接索引的非负整数 `0≤order≤degree` 之候选输入域；`scale=0`/非法 degree/order 的显式失败属于 C++ 安全候选，原源可能报错/除零，不得伪称已实测。 |
| `circle_curve_in_the_sphere.m:6–44` | 选择 `kreis_trajektorie(stellen=600,parameter=0.6,hoehe=0.3)`。先 `sphere(quality=50)` 生成单位球表面，再对满足 `y<=0` 的球面**只令 z=NaN**（不改 x/y），绘制球面与圆轨迹，标题文本是 `circular trajectory within the sphere`。 | 球面是原图的**单侧遮罩示意**，不能当作完整无缺的可见球壳；轨迹仍用源 `kreis_trajektorie.m` 的 `2π(i-1)/stellen`，不改变数值轨迹。 |
| `sinosodial_curve_in_the_sphere.m:7–45` | 选择 `sinuskurve_trajektorie(stellen=600,amplithude=0.5,laenge=1,anzahlperioden=2,hoehe=0.3)`；相同 `sphere(50)` 和 `y<=0 → z=NaN` 球面遮罩，按正弦点列叠加轨迹。 | 源代码第 45 行**依然写** `title('circular trajectory within the sphere')`，而所谓正弦标题只在第 46 行被注释。这个**标题错误是原版事实**：忠实显示需保留或明确标注该差异；如另提供修正标题，须标记为非原版展示选项，不能称其为 MATLAB 原始标题。 |

**防止混淆的静态夹具资格（沿用 `TC-F-AN-002`）**：

1. 插图 `degree=0,order=0`：`P_0^0=1`、`yy=1`、`scale=1`，因而全部格点 `rho=8`（**不是**归一化的 `Y_{00}=1/√(4π)`）；`degree=1,order=0` 时 `yy=cosθ`、`rho=5+3cosθ`。两者为手工代数候选，不是已生成的 L3 文件。
2. 插图 `degree=1,order=1`：只使用 `P_1^1(cosθ)cosφ`，不得以 `Y_{1,-1}` 的 `sinφ` 分支代替；检查 41×41 角网格、`rho`、x/y/z 的单位和显示值；非法 `order`/零尺度必须单独有非成功状态。
3. 圆与正弦分别核对 600 点、固定参数、`sphere(50)`、`y<=0` 的 z-only NaN 遮罩、源标题与真实轨迹类别的差异；`kreis_trajektorie.m`、`sinuskurve_trajektorie.m` 内部均有 `line` 显示副作用，**数值轨迹生成模块不能复制该副作用**，图像组合交由可选绘图层。
4. 这些只是未来**数值几何/源标题**的 `TC-F-AN-002` 候选断言，不新增 TC、不执行绘图/数值程序。像素级图形等价、渲染器灯光等不作为当前数值 AC 的自动通过条件。核心球谐归一化与相位只由 [harmonics.md](harmonics.md) 负责，其 `TC-F-SH-001/002` 仍独立未验收。

## 4. 验收、Golden 和阻塞

- 每一种输出曲线必须从具体 MAT 变量、原始 MATLAB 索引（1-based）和读出 shape 追踪到 C++ 结构化曲线；逐元素误差与数值统计独立于颜色/图例验证。
- **DEC-004 已确定**：历史归档原样只读读取；新 C++ 输出只生成当前有效 save 字段。对于当前生成流程不写、但历史分析需要的 `rottransratio` 等字段，必须报告 `MissingVariable`（或由 Analyzer 按 contracts 明确映射的非成功）及 `HistoricalOnly` 来源说明，**绝不补零、跨 RunId 借用、推导虚假比值或伪造图像**。详见 [决策](../decisions/DEC-004_HISTORICAL_MAT_FIELDS.md)。
- TC-F-BIAS-003、TC-F-AN-001/002、TC-I-ANALYSIS-001、TC-F-CLI-004/005；当前无 Golden 数值、无签名冻结、无实际 CTest。


## 5. 四种分析入口的可编码索引/统计算法合同

| 来源分析函数 | MATLAB 索引与算法 | 结构化结果（建议） | 特别验证 |
|---|---|---|---|
| graphs_bias_test | 按 k=1..number_of_cores **每次覆盖 xwerte**、仅将 ywerte3…6 四组累加；循环后只对四组 y 除以运行数；取 anfang=3、ende=iterationnumber+1，参考线 y=100/sqrt(最后一个 k 的 xwerte) | 每个噪声系数、Original/Modified 平移/旋转的四组跨运行均值曲线；横轴与理论参考取末轮原 xwerte | **不能把 xwerte 求平均或替换成真实 N**；原四类 y 的分母固定 number_of_cores，最后 k 的 x 来源要可追踪；ywerte1/2 在原分析中被注释 |
| graph_agent_in_sphere_initialization_test | mittrans 的 MATLAB 1..13 行、mitrot 的全部行；semilogy | curve_id、x 的 MATLAB 1-based 下标、y 数组、log-y 语义 | 这两条曲线长度不同，不能强行补齐尾部 |
| graph_agent_in_sphere_adaptive_matched_filter_test | x=50:550（501 点）；rottransratio 阈值 0.01 **仅控制旋转误差图**的四段 begin/end，平移误差图直接全域绘制 x=50:550 | 旋转：按段 mit/ohne 两曲线；平移：连续 mit/ohne 两曲线；两图均有右轴 ratio | **AF-076**：平移图不使用旋转图的分段掩码；不把方向角度误差改成测地角 |
| graph_agent_in_sphere_adaptive_matched_filter_many_parameter | **旋转图**按 ratio 阈值 0.02 的四段单独切片：第 1 列原值，后 3 列对每段分别以 f=2/5/10 平滑并绘制段内 begin+f..end−f；**平移图**第 1 列原值，后 3 列对完整 600 行先全域平滑，再绘制 x=50:550（501 点） | 旋转需保留各段窗口输入/边缘零值及绘图区间；平移需保留全部 600 行平滑数据及连续 501 点绘图区间 | **AF-075/076**：平移不随 ratio 分段；历史 MAT 有 ratio、当前 update-rates 源不保存 ratio，缺失必须报错 |

### 5.1 分段选择逻辑（保留 MATLAB 规则）

原两个 Adaptive 分析脚本都**先为旋转误差图**计算 3 个分段区域的设置和 4 组 begin/end 数组，先设置 begin(1)=50，然后逐个 i=50..550 检查（这些数组**不用于**平移图绘制）：
- ratio(i) < threshold **且**先前 zone==0：将当前 i 写入 end(j)，j+1，zone←1；
- ratio(i) > threshold **且**先前 zone==1：将当前 i 写入 begin(j)，zone←0；
- 将第四段 end(4)=550；threshold=0.01（comparison）或 0.02（many_parameter）。
- **等于阈值不会触发任一分支**，不能自行改成 ≤/≥。

该脚本假设预期的阈值穿越次数和有效段数；当历史数据不满足条件时，部分 begin/end 可能为零，导致 MATLAB 索引无效。忠实模式需首先记录真实分段和失败行为，不能悄悄使用“自动发现所有片段”的新算法替代作者的四区域逻辑；改进策略属于单独需求。

#### 5.1.1 四段固定数组的精确分支与反例输入（2026-11-11 文档整改；非已执行L3）

对原只读 `graph_agent_in_sphere_adaptive_matched_filter_test.m:17–36` 和 `graph_agent_in_sphere_adaptive_matched_filter_many_parameter.m:15–34` 逐句核对：`anzahlzonen=3`，因此 MATLAB **先**创建长度4的 `begin/ende`，扫描区间严格为 `i=50:550`（**501个下标**），`j=1`、`zone=0`、`begin(1)=50`。`r_i<threshold && zone==0` 时写 `ende(j)=i`，`j=j+1`、`zone=1`；`r_i>threshold && zone==1` 时写 `begin(j)=i`、`zone=0`。两个条件为**两个独立的 if**，而不是一个可擅改阈值关系的统一分段器。扫描结束后**无条件**覆盖 `ende(4)=550`，绘图只枚举 `z=1:4`；多出的穿越不会让绘图自动扩展为第五段。MATLAB 数组在超出预分配长度后可能动态增长，所以“第五次穿越就必然发生数组越界异常”也**不是**源码能够支持的结论。

| 固定手工输入（均定义在600×1的 `rottransratio` 上） | 精确静态推导 | C++候选校验义务 |
|---|---|---|
| `r_i=t+δ`，仅 `120..179`、`260..319`、`400..459` 为 `t-δ`，其中 `t=0.01` 或 `0.02`、`δ>0` 且 `δ<t` | `begin=[50,180,320,460]`、`ende=[120,260,400,550]`；四段长度71/81/81/91，旋转 f=2/5/10 的窗口都有内部点；平移仍绘制完整501点 | 逐节点检查 `j/zone/begin/ende`、四段/三个窗口及右轴值 |
| 在上一组的 `i=121` 令 `r_i=t`，其他不变 | `i=121` 两个原 `if` 均不触发，`zone` 沿用1；四段边界**不变** | 不得改成 `<=`、`>=` 或四舍五入决定分支 |
| 对所有 `50..550` 均设 `r_i=t+δ` | `begin(1)=50`，`begin(2..4)=0`、`ende(1..3)=0`、`ende(4)=550`；原绘图最终将遇到含0索引的路径，不能将其描述成有效四段 | 在引用 `begin/ende` 对源数组取段**之前**明确失败 `AnalysisErrorCode::InvalidSegment`（候选安全行为，不是已测MATLAB错误码） |
| 增加第四个由 `t-δ` 构成的独立低值区间，随后再次高于阈值 | 源 `j` 可能超过4，MATLAB数组可扩展，但 `ende(4)` 仍被覆写为550且绘图仅选前四段；不能声称会自动绘制第五段或必然数组异常 | 记录源计算出的额外穿越；安全拒绝或保留原前四段须在P0.8审核时明确选择，不能默默发明自适应区域数 |

**范围/错误边界**：`rottransratio` 必须首先具有覆盖 550 的正确shape与**有限值**；数据形状错误归 `ShapeMismatch → decode(4)`，非有限归 `NonFiniteInput → analyze(5)`，结构不合法的段界（零下标、反序、超出数据长度）归 `InvalidSegment → analyze(5)`（候选单一映射见[contracts §3.1](contracts.md)）。短段的平滑 `f\ge L/2` 在 `kurveglaetten.m` 中没有可更新的内部值，原 `zeros` 边缘与随后可能为空的 `plot` 区间应作为**空曲线/候选失败策略**分别记录，不能写成“MATLAB必然抛异常”。这些输入是已按原源**静态推导的测试夹具**，不是已运行的 MATLAB、Analyzer、独立L3文件或CTest。沿用 `APL-G03/04/05` 和已有 `TC-F-AN-001/TC-I-ANALYSIS-001`；不新增TC或擅改DEC-008。

**AF-089 来源边界（分析模块只消费存档变量）**：`rottransratio` 原由 Comparison Runner 使用**注入噪声前**的理想平移/旋转分量平均模长求比（虽然源码在 `nonpropfehler` 之后赋值），保留该轮缓冲，**不是从含噪光流重新计算得到的比值**，参见 [实验 LLD §5.1](experiments.md)。本 Analyzer 对历史已有的 `rottransratio` 按 MAT 变量原值做阈值比较与绘图，不在本模块重新估计、不用含噪流代替、不为新 Update rates 生成缺少的历史专属字段。比值变量含 NaN/Inf 或缺失时遵守现有 `AnalysisErrorCode` / `contracts §3.1` 非成功候选合同；真实 MAT 逐值/独立 L3/CTest 仍为 AF-050/051 的开放项。

### 5.2 平滑窗口的完整边界行为

原 kurveglaetten(kurve,fenster)：

\[
y_i=\operatorname{mean}(x_{i-f:i+f}),\quad
i=f+1,\ldots,N-f;
\qquad
y_i=0\text{ （原始 zeros 预分配的其余边缘）}.
\]

这里的 i 是 MATLAB 1-based，f 是**半窗宽**而不是总窗长。分析脚本再对部分段选择 begin+f..end−f 进行绘图。C++ 数值输出要保留原 y 边缘零值和“被实际绘制的有效子段”，不能默默改为镜像延拓、全长卷积或在边缘计算更短窗口。

#### 5.2.1 旋转/平移平滑的不同数据域（AF-075/076）

**多更新率的旋转面板（四段局部平滑）**，按 MATLAB 1-based 逐段：

```text
for z = 1..4:
    b, e = begin[z], end[z]
    col1: plot x=b..e, y=rot[b..e,1]  # 不平滑
    for (col,f) in [(2,2),(3,5),(4,10)]:
        segment = rot[b..e,col]         # 局部长度 L=e-b+1
        smooth = kurveglaetten(segment,f) # 两端各 f 个 zeros
        plot x=b+f..e-f, y=smooth[f+1..L-f]
```

**多更新率的平移面板（全量平滑、不分段）**：

```text
for col in 1..4:
    full = trans[1..600,col]
    smooth = full if col==1 else kurveglaetten(full,[2,5,10][col-2])
    plot x=50..550, y=smooth[50..550]
```

`graph_agent_in_sphere_adaptive_matched_filter_test.m` 同样区分：**旋转图**按照比值阈值 0.01 画分段 mit/ohne，**平移图**直接对未平滑的 mit/ohne 绘制完整 `x=50..550`，两图的右轴 `rottransratio(x,1)` 均有完整范围。以上是两份 MATLAB 分析脚本的**实际执行域**，不新增输出指标。

**重要的可判别性限定**：若按 `begin+f..end-f` 对旋转段严格裁剪，则这些**实际绘制的内部均值**与先对整条 600 行做相同均值平滑、再裁剪同一个内部区间的数值相同——因为内部窗口不越过段边界。故**不能谎称只凭这些内部曲线值即可区分两种平滑流程**。需要在独立 L3 中比对各段平滑的*完整临时数组*（两端 f 个零）、各段长度/来源、段界有效性、实际绘制的 x/y 与平移图**不分段的全部 501 个 x 值**。对于短于窗口的段，忠实记录源的空绘制/索引行为及 C++ 明确错误策略，不能据假想跨段混值声称图像不同。

测试来源：原只读 `graph_agent_in_sphere_adaptive_matched_filter_many_parameter.m`、`graph_agent_in_sphere_adaptive_matched_filter_test.m`、`kurveglaetten.m`；见 [补充审计纠偏](../reviews/AUDIT_ANALYSIS_SEGMENT_SMOOTHING_CORRECTION_2026-11-08.md)。此处只建立静态 LLD，独立 L3、Analyzer/CTest 和最终阈值仍缺，原 AF-050/051 继续「未整改」。

### 5.3 Bias xwerte 的原版兼容约束

两个 Bias **生成脚本**都写 xwerte(i+1)=6·4^i，然而完整球面生成函数的实际视线数量为 8·4^i。分析脚本对原存 xwerte 求 100/sqrt(xwerte)。因此：
- **原始复现视图**必须使用历史文件的 xwerte 和理论曲线，不把 x 替换成真实 N；
- 如另行输出物理实际样本数，应使用 separate actual_flow_count 元信息及新的、明确标注的图表，不能把新图伪装成原论文结果；
- **原版 Bias xwerte 与理论参考线是当前复现的既定要求**，不再因“是否提供另一种对照图”请求当前人工选择。若另行提供物理实际方向数，必须作为明确标注的 `actual_flow_count` 元信息，与原存 `xwerte` 并列且不得覆写；**额外使用实际 N 的对照图属于当前必要交付/验收范围之外的新增功能**，只有收到明确新需求才另行规划、验收，不能阻塞既定分析功能。

### 5.3.0 Bias 原理论线和实际点数的精确分离（2026-11-12 源合同整改）

原 `bias_test/data_analysis/graphs_bias_test.m:20–51` 在每个 k 读取并**覆盖** `xwerte`，最后在 MATLAB 1-based 行 `j=3..iterationnumber+1` 生成参考 `100/sqrt(xwerte(j))`。它不是以真实 N、不是以四组 y 的统计量、更不是对跨 k 的 x 均值生成参考。对于细分层级 i，两个原生成脚本都**写** `xwerte(i+1)=6·4^i`；但是完整球面 Bias 的真实样本数 `N_actual(i)=8·4^i`。因此两个候选参考量的比为

\[
\frac{100/\sqrt{6\cdot4^i}}{100/\sqrt{8\cdot4^i}}
=\sqrt{\frac43}\approx1.154700538.
\]

固定判别输入 `i=2`（源绘图行 j=3）：**保存横轴** `xwerte=96`，**完整球面真实数量** `N_actual=128`；原源码理论值 `100/√96≈10.206207`，错用实际 N 得 `100/√128≈8.838835`，约差15.47%（以错用N时参考为基准）。此外脚本的固定 `xlim([100,3000000])` 会把该行 **x=96 的可视位置裁剪掉**，但 j=3 的数值数据与线对象仍被构造，**不得把“不可见”当作“该帧不属于分析数据”**。任意其它 i 的比值相同；K>1 的普通聚合还应引用**最后一运行**的 xwerte，见下节。该反例可以在没有作者实际MAT数值时按原静态生成公式手算；不是正式独立L3或C++/CTest PASS。
#### 5.3.1 Bias 跨 RunId 聚合的非对称覆盖顺序（AF-083）

原只读 `bias_test/data_analysis/graphs_bias_test.m` 的 MATLAB 结构是：先将四组 `allywerte3…6` 初始化为 `zeros(iterationnumber+1,3)`，然后执行 `k=1:number_of_cores`。**循环每次直接赋值** `xwerte=getflowfield([str 'xwerte'])`，但四组 y 值分别执行累加；循环结束之后才令 `anzahl=(number_of_cores-1)+1`，并且**只对四个 y 数组**做除法。结果是：

\[
 x_{\mathrm{plot}}=x^{(K)},\qquad \overline{Y}_j=\frac{1}{K}\sum_{k=1}^{K}Y_j^{(k)},\quad j\in\{3,4,5,6\},\ K=\mathrm{number\_of\_cores}\ge1.
\]

参考线使用 `100/sqrt(x_plot(i))`，绘图下标 `i=3:(iterationnumber+1)`；原分析函数并**不**检查不同 k 的 xwerte 是否一致。即使两次运行的 x 向量不相同，忠实复现原脚本时也不能悄然改成 `mean_k(x^k)`，或将其改用真实球面方向数。额外数据一致性告警/安全拒绝只可单独标记为候选增强，不能冒充原脚本具有此行为。

**辨别用的独立输入资格（未执行）**：构造 K=2 的两个相同 shape 的人工 MAT 组，故意让 `x^(1)≠x^(2)`（只作区分性负例，**不是作者实际 Bias 生成器的正常输出**），同时令两组 `Y_j` 不相同。期待源算法的 `x_plot=x^(2)`、`Ybar_j=(Y_j^(1)+Y_j^(2))/2`、参考线取 `x^(2)`；以 K=1 确认单轮自反；K=0 以及读取缺失/shape 不匹配的情况下，C++ 候选应显式非成功，不能宣称 MATLAB 本身提供对应安全分类。关联 `APL-G01`、`TC-F-BIAS-003` 及 `TC-I-ANALYSIS-001`。完整 L2/独立 L3/Analyzer/CTest 仍未执行，AF-050/051 仍开放。

#### 5.3.2 Bias `loglog` 单坐标轴、线数和图例的实际源行为（2026-11-12 显示合同纠偏）

只读 `graphs_bias_test.m:44–79` **仅调用 `loglog`**，没有 `plotyy`、`yyaxis` 或第二纵轴。因此 Bias 图是同一张图的 **x轴与y轴分别对数化**，与 Adaptive Comparison 图实际使用 `plotyy` 的双纵轴严格不同。源先画 `100/sqrt(xwerte)` **1条黑色参考线**，再以三组噪声列 `i=1:3` 为每组画 `ywerte3、4、5、6` **4条线**，合计 **13个绘图调用/线对象**；但末尾的 `legend('a','b','c','d','e')` 只给出 **5个文本标签**。**不得把五个图例文字误作为全部13条线的唯一数值/列身份**；哪五个对象如何由具体后端默认绑定不在未运行条件下臆断，必须从源字段编号、噪声列、线型顺序恢复可靠series identity。

原脚本设置 `xlim=[100,3000000]` 与 `ylim=[0.06,200]`，横纵轴均为对数。**合法数值与可视范围要分开**：可见性受上述窗口和log的正值条件限制；原始参考值/误差为0、低于0.06或高于200时，不能把原数据静默改为0.06、删去输入样本、甚至报告整个Analyzer数值必然失败。正确的设计是让 `AnalysisDataset` 保存未经显示裁剪的原值和源索引，再由可选Exporter按正值/窗口和原坐标轴条件选择显示；真正 NaN/Inf、错误MAT形状及源无定义指标则遵循原 `AnalysisErrorCode` 合同。**显示修正图例、范围或零值替代属于标明的增强模式，不冒充源原图。**

关联原 `TC-F-BIAS-003`、`TC-I-ANALYSIS-001`、`TC-F-CLI-005`；必须分别核对数值13条序列的来源、单轴loglog显示、数据和视窗裁剪的区分。本段是源静态复核与未来测试合同，**未生成正式Plot/Analyzer、未运行MATLAB或CTest，也不批准新ABI字段**。
## 6. 候选 CurveSeries 输出 schema 与 C++17 错误合同

~~~cpp
// 设计合同，不是已实现类；公共 Result/Error 在 contracts.md。
enum class AxisScale { Linear, Log10 };
// 当前仍为可审查候选值类型，不表示公共ABI已获批准。
struct MatInputProvenance {
    std::string original_mat_relative_path; // one actually contributing MAT
    std::string original_mat_variable;      // exact internal variable name
    std::size_t source_run_ordinal_matlab;  // original 1-based k; single-run = 1
    std::string role;                       // e.g. "y_member", "x_last", "ratio"
    std::optional<std::string> mat_content_sha256;
    // nullopt means NOT verified; never substitute FNV64 or a .m Git blob.
};
struct SeriesProvenance {
    std::vector<MatInputProvenance> inputs; // all actual x/y/ratio contributors
    std::string source_blob_sha;            // original analysis .m Git blob ONLY
    std::vector<std::string> helper_source_blob_shas; // e.g. smoothing .m
};
struct CurveSeries {
    SeriesProvenance provenance;
    std::string curve_id;
    std::string x_unit, y_unit;
    AxisScale x_scale, y_scale;
    Eigen::VectorXd x_values, y_values;
    std::size_t source_index_first_matlab;
    std::size_t source_index_last_matlab;
    std::size_t smoothing_half_window;
    std::vector<std::pair<std::size_t, std::size_t>> plotted_segments;
    std::vector<std::size_t> invalid_indices;
};
struct AnalysisDataset { std::vector<CurveSeries> curves; };
enum class AnalysisErrorCode {
    MissingResult, MissingVariable, ShapeMismatch, InvalidSegment,
    NonFiniteInput, UndefinedStatistic, UnsupportedExperiment
};
~~~

**2026-11-14 DAR-039：多输入曲线的可验证身份和两个不同哈希域。** 原§6只含一个`original_mat_relative_path`、一个`original_mat_variable`及含糊的`source_blob_sha`；但源`graphs_bias_test.m`的每条`ywerte3..6`曲线**依赖K份运行目录中的同名y变量和最后一次的xwerte**，而Comparison等也同时依赖误差MAT与`rottransratio`。单一MAT路径无法完整说明数据如何聚合，且`source_blob_sha`若被当成MAT内容SHA-256会将**原.m源码Git blob（40位SHA-1身份）**与**原MAT字节SHA-256（64位内容摘要）**混淆，违反[contracts §4](contracts.md)的来源分层。

现按已存在的`CurveSeries/SeriesProvenance`候选性质细化为`inputs[]`：每个参与本条曲线数值的MAT路径/内部变量名/原1-based k/角色均必须列出；`source_blob_sha`只表示原**分析 .m** Git blob，若使用`kurveglaetten.m`等附属公式则另保存其Git blob身份；`mat_content_sha256`单独属于该实际MAT文件，**只有从被分析的稳定文件身份获得并验证后才填写**，否则为`nullopt`并明确`L2 SHA not verified`，不得把历史探针FNV64或另一份文件的哈希顶替。算法能从Reader的值对象做分析，不代表获得MAT原始文件的字节摘要；实际读取/绑定哈希的可靠性属于未来I/O和集成测试，不能凭新增字段伪称582份独立SHA已经完成。此处不增加对外新Reader方法，也不改变历史MAT。

**K=2固定来源测试**：使用仅用于鉴别的两个隔离运行（不冒充作者实际40份）——第一轮`xwerte=[6,24,96]`、`ywerte3(:,1)=[2,4,6]`；第二轮`xwerte=[7,28,112]`、`ywerte3(:,1)=[4,8,10]`。应有`x_plot=[7,28,112]`、`mean_y=[3,6,8]`，MATLAB绘图行3得到`(x,y)=(112,8)`、理论参考`100/sqrt(112)`。该曲线的`inputs[]`必须至少追溯到`k=1,ywerte3`、`k=2,ywerte3`及`k=2,xwerte`**三个实际来源**；不得记录`k=1,xwerte`作为最终x的数值贡献。三份MAT的SHA应分别关联同一份真实已读文件，不存在则标未知而非填写原.m Git blob。这个独立静态输入与[TEST_PLAN §2.6](../TEST_PLAN.md)原TC绑定，**不是已运行Analyzer/CTest，也不将额外分析数据字段写入原MAT**。

**单一分析错误归属（AF-077）**：`AnalysisErrorCode` 七个状态只在本分析 LLD 定义；其对外 `exit_code/ErrorStage` **唯一候选映射**由 [contracts.md §3.1](contracts.md) 维护：缺结果/变量归 `read(3)`，shape 归 `decode(4)`，无效分段/非有限/无定义统计/Analyzer 内部不支持模式归 `analyze(5)`；不再在本模块定义第二份退出码表。非法 `--experiment` 值应在 CLI parse(2) 阶段被拒绝而非进入 Analyzer；底层缺MAT/变量的原`MatErrorCode`与阶段/路径/变量须按DEC-008-04保存于**`ErrorDetail.causes`的`std::forward_list<ErrorFrame>`完整包装链**：例如Reader `MissingVariable@Read`被Analyzer `AnalysisErrorCode::MissingVariable@Analyze`包装，则外层Analysis为当前Failure，链首是内层Mat错误；如果继续由Runner包装，再`push_front`保留Analyzer和更深的Mat帧，不能只存最初的根因字符串。例：新多更新率结果没有历史 `rottransratio` 时应报告 `AnalysisErrorCode::MissingVariable` 和缺失变量身份，**不得**编造比值。此映射为待 DEC-008 签署的**候选**，并无真实分析测试或 JSONL PASS。

前置条件：源目录可只读加载、每条曲线要求的变量存在且形状可核实；任何 1-based→0-based 的转换必须在 Analyzer 内显式处理。后置条件：曲线数据自拥有且可在 MAT 文件关闭后独立消费；错误不得诱发求解器运行或写回源 MAT。

PlotExporter 仅消费 AnalysisDataset，输出图形位置必须在独立 RunId 中且不得覆盖原数据；禁用绘图或没有图形终端时 CLI analyze 仍可返回全部数值与分析状态。

**绘图错误的唯一外部映射（AF-084）**：`PlotErrorCode` 四值在 §2 定义，跨模块 `exit_code/ErrorStage` 的**唯一候选**为 [contracts.md §3.1](contracts.md)；`BackendUnavailable` 只指缺后端，`InvalidSeries` 指已到绘图层的数据不合法，`OutputForbidden/WriteFailed` 指持久图像写入的文件系统失败。不能把四类统一当作 exit 7，也不能在 IPlotExporter/CLI 复制第二份映射。分析器内部先发生的 `MatErrorCode/AnalysisErrorCode` 不应被改写成 `PlotErrorCode`；底层来源身份、输入/输出路径和已分配 RunId 应按 ErrorDetail 保留。真实后端、异常测试和 DEC-008 仍未验证。

## 7. 对应 Golden、边界数据与测试映射

**参考标识全局唯一**：本模块六个候选分析/绘图参考使用 `APL-G01～APL-G06`。原有 `docs/golden/GOLDEN_TOLERANCE_PROPOSAL.md` 与 `analytic_golden_candidates.v1.json` 的 `AN-G01～AN-G12` 是**不同的全局解析数学候选**（例如其 AN-G01 是八面体方向、非 Bias 曲线），**不得与此表共享同一 ID**。此次只重命名本模块的候选标识，不改变各组输入、公式、TC/AC 或任何历史 MAT 数据；正式 L3 与 CTest 仍未生成。


| 计划 Golden | 输入来源与核验内容 | 对应 TC |
|---|---|---|
| APL-G01 | 两类 Bias（历史各40-run）：仅 ywerte3…6 跨 k 平均，xwerte 取最后一次 k 的原值；参考线 100/sqrt(x_last)，绘图从 MATLAB 行 3 开始；K=2 不同 x 的区分性负例 | TC-F-BIAS-003、TC-I-ANALYSIS-001 |
| APL-G02 | init 的 1..13 与 1..35 原样索引，两个不等长度序列 | TC-F-AN-001 |
| APL-G03 | comparison x=50..550、阈值 0.01、旋转/平移曲线、右轴 ratio 和各分段 | TC-F-AN-001 |
| APL-G04 | many_parameter：**旋转**四段各自局部平滑/裁剪＋**平移**完整 600 行全局平滑后绘制 50:550；保留局部临时零边界和连续平移 x | TC-F-AN-001 |
| APL-G05 | rottransratio 缺失、NaN/Inf、阈值恰好相等、四段缺项/多次额外穿越、**旋转局部段短于窗口及可能空绘图区间**、平移全域窗长不足、MAT 形状错误；固定手工输入与预期见§5.1.1 | TC-I-ANALYSIS-001、TC-NF-HEADLESS-001 |
| APL-G06 | 单位球圆/正弦轨迹、l≤2 实球谐三种插图数据 | TC-F-AN-002 |

所有参考数据须来自原只读 .m 源公式/历史 MAT L2 及独立 L3 曲线实现，带原源码 SHA、输入 MAT SHA、参考生成过程与明确 Golden 版本；**不要求运行原 MATLAB 分析脚本**。图像像素一致性不要求，但**数据值、MATLAB 索引、绘图分段、单位和有效数量必须可审查**。

**当前状态**：分析的索引、分段、平滑及每曲线多MAT来源合同已列出；**历史 582 个 MAT 已由 matio 实测 READ_OK，真实形状与类型见下表**。实际曲线数值 Golden、批准容差、绘图后端、C++ 分析程序与 CTest 仍未完成。

## 8. matio 已确认的历史分析输入合同

[MAT_DATA_SCHEMA.md](../MAT_DATA_SCHEMA.md) 是当前作者归档中 MAT 变量名、class/type、真实 dims 和读取状态的唯一事实来源。所有历史文件为 MAT v5/zlib、class=double/data_type=double，**582/582 文件/变量读取成功**；不能因此直接判定曲线计算正确。

| MATLAB 分析脚本 | 历史 MAT 输入变量及真实 shape | 约束 |
|---|---|---|
| graphs_bias_test | 两类归档各 40 运行；每运行 xwerte `10×1`，ywerte1…6 各 `10×3` | 原图分析主要使用 ywerte3…6，且 xwerte 按作者存储值而非实际完整球面 N |
| graph_agent_in_sphere_initialization_test | 四个 allemit/alleohne trans/rot 变量，均 `35×1` | 1..13 与全行索引按源脚本控制，不因 dtype/shape 合法而省略索引测试 |
| graph_agent_in_sphere_adaptive_matched_filter_test | without_errors 的四误差和 rottransratio 均 `600×1`；with_errors 另有 alleadaprot/alleadaptrans，亦 `600×1` | 当前源码不写 `alleadap*`，历史额外字段归类为 `HistoricalOnly`，**无需先归因**；保留历史数据的只读分析能力 |
| graph_agent_in_sphere_adaptive_matched_filter_many_parameter | 现存 allemittrans/allemitrot/alleadaptrans/alleadaprot 均 `600×4`；akrot/rottransratio 均 `600×1` | 当前多频率生成脚本只保存两个 allemit 和 akrot；历史 rottransratio 属 HistoricalOnly。新输出进行该图分析时缺少比值即明确失败，不虚构数据；保留旧历史完整分析 |

**DEC-004 的最新所有者决定优先于历史 H-07 的仅起草状态**：当前生成字段正常输出，历史额外字段标 `HistoricalOnly` 并只读保留；未知来源无需先归因。分析需要却缺失的字段明确非成功，不补造/不调整统计口径。三态来源分类可保留 `CurrentGenerated/HistoricalOnly/Unknown`，具体报告类型待 LLD/CTest 验证。

后续验收要分别进行：(1) `Mat_VarRead` 正常读取、rank/dims 解析（**已有独立探针证据**）；(2) 业务 Reader 将 MAT 列主序数据正确映射到分析数组；(3) 分段、窗口、指标及数值曲线与从 L2 MAT 依据原静态索引/公式**独立重算的 L3 数值参考**逐项相符；(4) 可选绘图输出。只有 (1) 有实际完成证据，其余仍未实施。

### 8.1 第一轮分析数值与缺字段验证表（AF-050，设计补齐）

| 场景/输入来源 | 独立比较的数组或错误语义 | 需保留的证据 |
|---|---|---|
| Bias 两组各 40 次的历史 L2 xwerte/ywerte | `xwerte` 逐 RunId **覆盖并只取最后 k**，而 `ywerte3…6` 各自累加后除以 K；理论线从 `xwerte_last` 得出，绘图 MATLAB 行 3…end（不能用实际 N） | 每组实际 K、最后 k 的源路径和变量身份、四组均值及源 shape；用 K=2 且两组 x 不同的人工区分输入验证非对称更新；独立 L2/L3 数值仍缺 |
| Initialization 的 35×1 输入 | `mittrans(1:13)` 为 13 点、`mitrot(:)` 为 35 点；不得补齐/截断成相同长度 | 原 MATLAB 1-based 下标、各自输出 shape、对数坐标元数据 |
| Comparison/Update-rate 的历史 `rottransratio` | `x=50:550` 共 **501** 点；阈值分别 **0.01/0.02**，**等于阈值不转移**，`begin/end/zone` 顺序与最多四段按源定义 | 可逐点复查的 ratio、阈值状态、begin/end、真实绘制区间与 NaN/Inf 分类 |
| 选定平滑半窗 `f=2/5/10`，包括短数组/越界分段 | **旋转**：各 begin/end 段分别构建长度 L 的平滑数组，边缘 f 个零，绘图 `begin+f..end-f`；**平移**：完整 600 行平滑，连续绘制 `x=50..550`，不使用 begin/end | 段级完整临时数组/零边缘、全局平移 600 行、各图绘制 x/y 和无效段错误；不能错误地断言正确裁剪的旋转内部值必与全局平滑不同 |
| 当前 Update rates 三份 save 缺 `rottransratio` 或历史字段不全 | 明确 `AnalysisErrorCode::MissingVariable`（或合同指定的非成功），**不得**推导比值/回退另一字段；旧历史结果只读分析能力保留 | MissingVariable 诊断含 source/path/variable，来源 `CurrentGenerated/HistoricalOnly` |
| 绘图后端未安装或无显示环境 | 只读数值 Analyzer 仍应返回完整 `AnalysisDataset`；可选 plot 独立返回能力错误，不改变数值/文件 | 有图/无图两路输出的曲线值与有效样本身份一致 |

上表绑定 `APL-G01～APL-G06`、`TC-F-AN-001/002`、`TC-I-ANALYSIS-001`、`TC-NF-HEADLESS-001` 的现有候选；不重命名全局 `AN-G01～12`。尚缺 L2 逐值/SHA-256、独立 L3 曲线、业务 Analyzer、错误负例及 CTest，**AF-050 仍「未整改」**，绘图后端不是核心数值验证门禁。

## 附录 A：本模块无需运行 MATLAB 的验收参考

按 [长期无需 MATLAB 执行政策](../golden/NO_MATLAB_EXECUTION_POLICY.md) 与 [L2/L3/L4 来源矩阵](../golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md)：原 MATLAB .m 文件仅作为**只读公式、数据布局和顺序规范**；**不要求执行 MATLAB/Octave、采集 L1 或以缺少 L1 阻塞冻结/编码/验收**。

采用原始 MAT 的 L2 列主序数值作为只读输入，独立生成 L3 begin/end/zone、平滑数组、曲线/分段和 L4 统计；不能从原绘图脚本取得的 L1 中间数组**不是必需取证项**。不要求执行 graph_*.m；实际独立分析器、参考 SHA、批准容差和真实 CTest 尚未完成。

本附录不取消真实 C++/CTest、Golden 独立复核、DEC-005/008 以及源数据保护门禁；当前无新的软件测试通过证据。
