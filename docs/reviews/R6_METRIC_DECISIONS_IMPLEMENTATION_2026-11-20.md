# R6 指标决定与部分有效结果实现（2026-11-20）

`[HUMAN-EXECUTE/APPROVE]`。依据项目所有者在当前会话明确确认的 DEC-R6-01～04 和“继续代码实现”，在 Draft [PR #6](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/pull/6) 的现有开发分支接续。起点为 `8f6d46c0fb98ca4b45304bc86876ec35376778bb`；决定记录见 [2026-11-20 所有者决定](../decisions/PROJECT_OWNER_DECISIONS_2026-11-20.md)。本记录不代替 R6 总体验收或正式冻结。

## 已实现的合同

| 决定 | 生产实现 | 数值与兼容性边界 |
|---|---|---|
| DEC-R6-01 | `sourceDirectionErrorDegrees` 对有限非零向量先按最大分量缩放，再用 `atan2(norm(cross(u,v)),dot(u,v))` | 保留零向量无定义；不再因 `acos` 端点舍入越界中断。Bias `normalizedVectorError` 未改。 |
| DEC-R6-02 | 将每帧弧度旋转向量变换为矩阵，计算 `Exp(truth)^T Exp(estimate)` 的 SO(3) 主测地角，输出度 | 零向量为单位旋转；相同旋转轴而幅值不同不再被误判零误差。**这是经批准的指标变更，不是历史旋转轴方向角等价实现。** |
| DEC-R6-03 | `AdaptiveFrameResult` 的平移/旋转指标分别返回 `Result`；纯指标无定义仍完成主求解并提交有效状态 | 策略失败、比较/主求解失败、非法输入/状态、资源失败仍停止并回滚该帧，不跳过失败帧。 |
| DEC-R6-04(B) | 保存部分有效字段、逐轮原值、缺测坐标/原因和有效重复次数；完整求解但统计缺测的交付状态为 `Incomplete` | 未采用方案 A。八轮误差均值只在八次均有效时产生，任一次缺测即保留 NaN；绝不改除数。 |

### 数据发布

原当前 save 变量名、形状、列主序、35/600 帧、8 次重复和首尾结构零保持；没有新增 HistoricalOnly 字段。每次新 Adaptive 输出还包含：

- `egomotion_metric_policy.mat`：变量同名，`1×4`，值为 `[1,1,1,planned_repetitions]`。它先于其它字段发布，防止中途失败的新目录被误认作历史结果。
- 每个原字段的 `valid_<source>.mat`：形状与原字段相同，值为有效重复次数。首尾结构零计数为 0；它们不是有效观测，也不是缺测。Comparison 的四个误差字段内部次数为 0～8，`rottransratio` 保持末轮值及 0/1 次数；UpdateRates 的 `akrot` 为 0/1。
- `egomotion-run-manifest-v2`：含指标公式/单位、固定分母、计算/统计完整性、每个聚合字段的次数与值、每轮原值，以及按 MATLAB 1-based 轮/帧/列定位的完整缺测原因链。JSON 只用 `null` 表示缺测，禁止 bare NaN/Inf。

原未掩码 Writer 仍严格拒绝非有限值；新掩码入口只接受 `defined=1` 的有限值与 `defined=0` 的 NaN，任何 Inf、形状/长度错误或矛盾掩码均拒绝。新入口复用既有安全发布、回读、摘要与持久化确认流程。Adaptive 的旧 Context 发布接口显式拒绝，不能绕过政策和证据。原始 MAT 均只读。

只有全部求解轮次完成、逐轮证据与聚合值/次数一致、没有求解失败、全部预期 MAT 回执及 Manifest 发布成功，才返回交付成功。统计完整返回 `Complete`；部分指标无定义返回 `Incomplete`。**CLI 退出 0 表示交付成功，不能单独当作统计完整；应读取状态和 Manifest。** 求解失败/输出失败仍分别返回 5/6，求解失败不会通过方案 B 伪装为完整求解轨迹。

### 分析与图形

Analyzer 先读取政策 marker，仅确实缺文件才按历史严格有限值路径解释。未知版本、错误重复次数、损坏 marker、缺计数、计数与 NaN 不一致均明确拒绝。新分析输出为 `egomotion-analysis-v2`，保留指标语义、有效标志、结构端点和输入来源；缺测为 `null`。SVG 对声明缺测断线，无声明 NaN/Inf 仍失败。平滑窗口含任一缺测时整窗缺测，不重加权、不缩小窗口。

UpdateRates 原分析需要但当前 save 未包含的 `rottransratio` 仍是已知 HistoricalOnly 限制：新结果直接执行该原分析会明确报告缺变量，不补造科学字段；Comparison 已声明缺测的 ratio 若阻止原阈值分段，也明确失败。

## 验证状态

**受测源码：`594a1b11f91028cb7549fef0ab1cd512ee4f7edb`。有限验证 12/12 通过，0 失败、0 阻塞；29 个生产翻译单元及 11 个测试二进制编译成功。** 本地普通 Git 推送因无写入凭据失败后，使用已连接 GitHub 账号按独立逻辑提交创建同内容树，并以非强制快进更新既有 PR 分支；每个源码树 SHA 均与本地提交核对一致。随后以 `git ls-remote` 核实远端受测 SHA，再从该 SHA 的隔离 worktree 编译执行。未改写旧远端历史、未合并 PR。

环境为 **Debian 13.7 / Linux x86_64、GCC 14.2.0、Eigen 3.4.0、C++17 `-O2 -Wall -Wextra -Wpedantic`**。已有 Eigen 来自当前环境；未发现可用 CMake、matio 或 Sophus 开发依赖。没有安装或下载依赖，也没有造替身依赖。本次直接使用已有编译器运行显式子集，**没有执行 CMake/CTest，不构成全工程或原科学实验复现**。此前固定 `3985b056...` 的 ON 73/73、OFF 72/72 不作为本轮通过证据。

RunId：`r6-metric-subset_20261120T043907Z_594a1b11f910_2`；执行时间为 **2026-11-20 12:39:07—12:40:33（Asia/Shanghai）**。命令：

```bash
bash scripts/verify_r6_metric_subset.sh \
  /opt/codex/runtimes/codex-primary-runtime/dependencies/python/lib/python3.12/site-packages/casadi/include/eigen3 \
  /workspace/scratch/0cabe2e08188/r6-validation-594a1b11
```

工作目录为 `/workspace/scratch/0cabe2e08188/r6-verify-594a1b11`；日志目录内 `commands.tsv`、`tests.tsv`、各命令 `.command.sh/.meta/.stdout/.stderr/.exit`、`source-inputs.sha256`、`remote-checkpoint.json` 保存实际命令、远端身份、时间、输出和退出码。编译/测试退出码均为 0；编译与补充语法检查日志无 warning/error。执行前后源码字节和 Eigen 版本头 SHA-256 均一致，受测 worktree 保持干净。

| 实际执行用例 | 结果与范围 |
|---|---|
| `r06_source_motion_errors` | 通过；973 项，覆盖旧 `1±ulp` 域外样本、极端尺度、零方向、SO(3) 零/幅值/非共轴/近 π。 |
| `r06_adaptive_periods_atomic_commit` | 通过；周期与状态提交合同。 |
| `r06_coefficient_policy_composition` | 通过；48,523 项策略组合检查。 |
| `r06_real_session_interleaving` | 通过；11,049 项、252 成功帧与 12 拒绝帧。 |
| `r06_session_independent_multiframe` | 通过；299,768 项、252 帧、924 次真实 Step、24 回滚；最大指标绝对差 `3.47066125838679795379e-14` 度。 |
| `r06_noisy_session_independent_multiframe` | 通过；290,951 项、252 帧、756 次真实 Step、24 回滚、9,072 次固定 normal；最大指标绝对差 `5.72181191316190052021e-14` 度。 |
| `r06_partial_adaptive_output` | 通过；18,166 项，纯内存八轮聚合与 v2 发布门禁，包含一次缺测保留 count 7 / 均值 NaN、逐轮原值、拒绝缩分母及错误证据。 |
| `r05_manifest_gate` | 通过；完整性、回执和错误发布合同，使用内存替身。 |
| `r06_partial_analysis` | 通过；政策/次数核对、JSON null、缺测传播及拒绝非法输入。 |
| `r06_analyzer_service_contract` | 通过；727 项、4 模式、40 个 Reader 故障。 |
| `r06_analysis_segments_smoothing` | 通过；原分段与平滑合同。 |
| `r06_svg_source_windows` | 通过；128 项，包含显式缺口、源视窗及新指标可见标签。 |

另在同一受测 SHA 对 **13 个文件执行 `c++ -fsyntax-only`，全部退出 0、无诊断**：4 个生产文件（Adaptive Runner、entries、production runner、CLI application）和 9 个相关测试/辅助程序（Session contract、Runner、scene、真实 Step delivery、I/O faults、masked Writer、CLI values、frame14 trace、RunContext interleaving）。命令和结果保存在 `syntax/`；这只证明语法与接口可编译，不证明链接、MAT I/O 或真实 CLI 运行成功。

### Session 指标有效性与回滚补充验证

为让 DEC-R6-03 的关键状态合同在现有环境中实际执行，新增独立 `r06_session_metric_validity`，保留原完整 Session 测试。新增测试与 CMake/子集脚本注册已提交并在运行前核实远端 **`2ed5c2d95474eb34977d6998fa70330226437614`**。相对上述 `594a1b11...`，`include/`、`src/` 与 `origin/` 无差异；据此复用该已受测生产静态库，只编译、链接新增测试，没有重复构建未变的生产源码。

**补充用例 1/1 通过：1,706 项检查、48 个成功帧、6 个求解失败回滚案例、3 个策略失败回滚案例，0 失败。** 覆盖三种 Session 模式下的零真平移、零估计平移、双零、后续有效指标恢复、零旋转/非零旋转测地角；纯指标缺测后仍提交主状态并推进帧号；缺测后的比较或主求解失败，以及指标计算前的策略失败，均逐字节保持全部五组系数，并通过跳帧拒绝与同帧重试验证帧时钟不推进。测试使用真实 Session 和系数策略，Step 为可控替身；它验证状态合同，不冒充真实 Step 数值或原轨迹实验。

编译参数、编译器及 Eigen 与主子集相同，编译与执行均退出 0，stderr 为空。执行时间为 **2026-11-20 12:50:13—12:50:16（Asia/Shanghai）**；工作目录 `/workspace/scratch/0cabe2e08188/r6-verify-session`，日志目录 `/workspace/scratch/0cabe2e08188/r6-session-validity-20261120`。该目录保存 `run.txt`、编译/执行命令与 stdout/stderr/exit、`inputs.sha256`、空的 `production-equivalence.diff` 以及执行前后干净工作树记录；生产静态库 SHA-256 为 `ac78cc22c6697790261aefe10b733ae817e3db0f485a09d878d34906d3b7fa95`，新增测试源码 SHA-256 为 `1b173e5aafed93eff5af3e72fbff8bb03ef1f759d85ed027648a2c141abac931`。

**本轮实际执行共 13 个不同用例，证据由 `594a1b11...` 的 12 项和 `2ed5c2d...` 的补充 1 项组成。** 更新后的子集脚本已注册 13 项/12 个测试程序，但本轮没有在 `2ed5c2d...` 重新执行整份脚本，也没有把旧 12 项重复计为新测试。

### 未执行范围

真实 `RunMatWriter` 的新掩码读写、完整 CMake/CTest（SVG ON/OFF）、真实原轨迹 CLI 和完整交付尚未在新版本运行。完整 `r06_adaptive_session_transaction` 含 Sophus 场景依赖，整体未执行，没有裁剪测试或伪造通过。原始资料树仍为 `70ee155131c24a50169346d0ffbaff0c01b1c547`，无 `origin/**` 修改。

## 延期项与后续边界

所有其它可推迟决定按所有者要求继续延期；当前四项无需重复批准。完整科学实验/L4、病态与跨环境资格、最终容差/性能决定、基线签字及 Draft 转 Ready/合并/发布均未执行或宣称完成。完整构建验证需要已有合适依赖的环境，或另行决定是否安装依赖；在此之前先保留源码、有限验证及具体限制，不以未知结果填“通过”。
