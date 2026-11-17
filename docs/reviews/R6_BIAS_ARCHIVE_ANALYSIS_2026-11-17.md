# R6 历史Bias全曲线独立复核与平滑来源

任务标签 `[HUMAN-EXECUTE/APPROVE]`。范围为已授权C++/CTest和文档；只读已有归档及固定输入，不重跑40次Bias科学实验、不执行MATLAB、不修改原始MAT或正式容差/冻结状态。

## 首轮准备（尚未执行）

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

待检查点后的实际配置、编译、43项CTest和输入摘要复核；R6仍未结案。
