# R6 历史Bias全曲线独立复核与平滑来源

任务标签 `[HUMAN-EXECUTE/APPROVE]`。范围为已授权C++/CTest和文档；只读已有归档及固定输入，不重跑40次Bias科学实验、不执行MATLAB、不修改原始MAT或正式容差/冻结状态。

## 首轮准备（历史未执行状态）

新增 `r06_bias_archive_independent_curves`，CTest登记由42增至43；登记不表示编译或通过。运行前仍须原子提交、正常推送和远端SHA核实，先运行五项MAT测试，再运行其余测试。日志与失败结果使用新的隔离目录保存。

### 历史曲线的独立参考

`tests/integration/test_r06_bias_archive.cpp` 对 `occluded_regions`、`proportional_error` 两组各40个目录完整验证原 `graphs_bias_test.m`（Git blob `112aa9eca1fa9728f493d31fc8e59620a8445230`）的13条曲线。原脚本只累加并平均 `ywerte3…6`，每次覆盖x、最后使用第40份原x；绘图行3…10。参考不调用生产Reader、Analyzer、分段或求解器。

已有582文件L2测试的有界MAT v5/zlib标签解析器提取为测试专用 `tests/support/archive_mat_reference.hpp`，解码逻辑保持不变。新测试直接解码两组的400份实际分析输入（x及四个y），应为10400个数值；每份SHA-256与既有独立清单对照。其余160份 `ywerte1/2` 在源图中被注释，不伪称为曲线贡献者；它们仍由原582文件测试逐值核验。

对每个y字段/噪声列/绘图行，以独立 `long double` 标量累加40份原始值再除40，并用宽精度计算 `100/sqrt(x_last)`。比较真实MatResultReader与MatResultAnalyzer的全部26条曲线、208对x/y、986个来源条目；输出每点真实值、独立参考及绝对差，沿用既有R6 Bias回归余量 `1e-9`，不新设DEC-005门限。横轴精确相等，保留比例噪声归档行3的128，不能代换为现生成公式的96。

另核对真实Reader的完整命名读取顺序（每组200次）、曲线单位/log-log轴/原索引/未平滑状态、所有40份y与末轮x的来源角色，以及分析前后的400份摘要。生产 `mat_sha256` 仍须为空：本测试独立取得的摘要未绑定生产Reader稳定快照，不能将测试摘要写成生产对象的已验证来源。可复核数值参考不等于原MATLAB运行结果或完整统计复现。

### 已有LLD中的平滑来源字段落实

[analysis LLD §6](../design/analysis_plotting.md) 已规定每条曲线记录辅助 `.m` Git blob。现将 `helper_source_blob_shas` 落实到 `AnalysisSeries` 与分析JSON，每条真正使用窗口2/5/10的UpdateRates曲线记录 `kurveglaetten.m` blob `eced66659205a6386ec84723e37a46517d06c017`；未平滑的第一列、Bias、Initialization、Comparison均为空。SVG按曲线保留同一辅助来源metadata，不改变曲线数值或几何。

有限输入的独立三次多项式参考同步覆盖整个JSON对象，检查20条更新率曲线中15条有平滑来源、5条无来源，以及Comparison12条均为空；SVG分别应含12条旋转/3条平移辅助来源。历史成功CLI分析另检查Bias和Initialization均为空。仍使用现有 `egomotion-analysis-v1` 的新增字段，既有字段意义不变；这是既有候选字段的实现与测试准备，不代表公共ABI/schema已冻结或DEC-008正式验收。

## 实测结果

源码检查点 `d68422cddc5de1b40baed5c1b5c57de5e4402074` 已原子提交、正常推送并由GitHub分支API确认远端SHA后执行。分支 `dev-plan-P0-P1-mat-dictionary-probe`，RunId `r6-bias-archive-20261117-0q1aV120`；工作目录为仓库根，日志 `build/r6-bias-archive-20261117-0q1aV120/evidence/`，复用构建缓存 `build/r6-20261117-KzjFHPDQ/cmake/`。

| 操作 | 时间（2026-11-17，UTC+08:00） | 退出码/结果 |
|---|---|---|
| CMake配置 | 20:33:53–20:33:55 | 0 |
| Release增量构建 | 20:33:55–20:35:33 | 0，无诊断 |
| 五项MAT CTest | 20:35:34–20:35:34 | 0，5/5，real0.76秒 |
| 其余CTest | 20:35:34–20:35:38 | 0，38/38，real3.70秒 |
| 全输入SHA-256复核 | 结束20:35:38 | 0，未改变 |

环境为Ubuntu24.04.5、Linux7.0.0-43、GCC16.2.0/C++17、CMake/CTest4.3.4、Eigen3.4.0、matio1.5.26、zlib1.3及已有Sophus1.24.6。配置 `-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`，编译 `--parallel 4 -- -k`；CTest先对原五项MAT正则 `-R` 并行2，再 `-E` 排除它们并行4，均有 `--verbose --no-tests=error --output-junit`。完整命令/工作目录/开始结束时间/退出码在各`.meta`；stdout、stderr、JUnit及CTest原始日志分别保存，所有stderr为空。

**43/43 CTest通过。** 新历史Bias测试实测**3323项检查、400文件、10400输入值、26曲线/208对x-y、986个来源条目、0失败**。最大绝对差为 `2.63782051757033286776e-14`，出现在比例噪声 `ywerte3.column2` 的MATLAB行3（x=128）：C++ `61.4212078761995812215`，独立宽精度参考 `61.4212078761996075997`。全部208点均保留真实值/参考/误差，不筛掉失败或更换分母。原L2测试也通过：582文件/33340值，包含80个mxDOUBLE/miINT32载荷与502个miDOUBLE载荷，解码值按位一致。

有限输入真实CLI的两个Analyzer仍与独立三次多项式JSON逐曲线对象一致：Comparison12曲线/2650点，UpdateRates20曲线/2928点/3460个完整平滑值。UpdateRates JSON含15个辅助源blob、Comparison为0；SVG旋转/平移分别12/3个。该轮真实CLI stdout/命令/8份合成MAT摘要/4份SVG均保留在缓存内 `r06_analysis_finite/有限输入 中文/7f6a0372d09141bdadd27ee9/`。历史成功CLI的Bias/Initialization空辅助来源检查亦通过；历史NaN归档仍按已知合同拒绝。

新历史参考与多项式均无数值RNG；其它回归的seed及OS随机RunId与输入身份另在 `randomness-source.txt`、`input-git-blobs.txt`、`inputs.sha256` 登记。`inputs-after.txt` 与 `summary.txt` 证明本轮输入未改变。CTest stdout SHA-256：MAT `2b600aadb7685cb21c3202702963e535511dc3893c079517290088e1c183f4e3`；其余 `e558ffa2d3551ae6f6fa16451809c8b7c9d352528028578b17b7c7e2314d2a8b`。原始628文件/582MAT的Git树仍为 `70ee155131c24a50169346d0ffbaff0c01b1c547`。

本轮未运行MATLAB、完整科学/L4或性能实验，也未安装依赖、创建/关闭Issue或PR、合并/发布。已完成的是历史曲线独立复核及既有辅助来源字段的实现/回归；MAT快照摘要绑定、插图展示、三Adaptive真实生成CLI的剩余覆盖、完整数值资格及正式验收不由此次通过推定完成。R6继续按 [66 TC映射](R6_TC_EVIDENCE_MAP_2026-11-17.md) 推进。
