# R6 有限 MAT 与非线性分析参考（2026-11-17）

状态：`[HUMAN-EXECUTE/APPROVE]`，用户已授权生产修复及构建/CTest。首轮已编译、35/36 CTest，失败定位于纵轴设置；修复待新检查点复测，不宣称正式冻结。关联 R6、P9.1/9.2/9.3 分析复现、TC-F-AN-001/002、TC-I-ANALYSIS-001、DEC-004/005/008；不改变需求和容差。

## 固定输入与独立参考

新增 `r06_analysis_finite_cli` 启动独立夹具程序，然后通过真实 CLI 对 Comparison/UpdateRates 分别执行 analyze/plot。夹具只链接 MAT I/O，不链接 Analyzer、生产平滑、分段或 JSON 序列化。8个新 MAT 只写独立 build 沙箱的 `output/<RunId>/`，空 `origin/` 与真实原始资料完全隔离。每次保存命令、SHA、起止时间、退出码、stdout/stderr、输入 SHA-256、实际/参考 JSON，保留失败记录。

第 v 个变量（0-based）、第 c 列（0-based）在 MATLAB 帧 k 的输入为
`p(k)=1000a + a*k^3 + 3b*k^2 + c1*k`，其中 `a=4(v+1)+c, b=a+1, c1=a+2`。四变量顺序为 `allemittransfehler, allemitrotfehler, alleohnetransfehler, alleohnerotfehler`；每列600帧。根据 `sum(j)=sum(j^3)=0`、`sum(j^2)=(2f+1)f(f+1)/3`，对称窗口的独立解析均值为 `p(k)+(a*k+b)f(f+1)`。f为0/2/5/10。所有多项式值、窗口整数和与均值都在double精确整数范围内；本例要求精确相等，不新增浮点容差或替代DEC-005审批。参考不累加相邻样本，不复用生产计算结果。

ratio默认0.03；110…189、250…329、390…469为0.015；120…179、260…319、400…459为0.005；另设110帧为0.02、180帧为0.01，区别严格比较与等号。手工边界：

| 入口 | 阈值 | 四段含端点范围 |
|---|---|---|
| Comparison | 0.01 | 50…120；181…260；320…400；460…550 |
| UpdateRates | 0.02 | 50…111；190…250；330…390；470…550 |

Comparison预期12曲线、2650对x/y；UpdateRates预期20曲线、2928对x/y、3460个完整平滑值（含保留的零边界）。旋转分段内平滑并裁去f个端点；平移先对600帧平滑再选50…550。比较所有JSON曲线字段、贡献变量、路径和角色；目前JSON允许未知输入摘要null，真实MAT摘要单独记录，不能声称JSON已携带摘要。SVG仅验证成功发布、两面板、结构/来源标识，不能由此声称视觉等价。

## R6-ANALYSIS-001：UpdateRates 纵轴实测缺陷

原 Comparison 分析 blob `3924038f0ccd70de5ea7b60c589afe166da9c8fb` 明确将两侧纵轴设为log；UpdateRates blob `67cb613328dec10105a6c383ce17025c4901e4c9` 使用 `plot`，没有设置log。首轮 `src/analysis/adaptive.cpp` 的 `makeCurve` 统一设为log，20条UpdateRates曲线全部在 `y_scale` 与原脚本不符，其余字段及所有数值均一致。修复为UpdateRates旋转、平移显式linear，保持Comparison/Initialization的log；增加真实SVG轴标记验证。未修改原值、容差、参考预期或原始资料。

首次失败对象 `29e965c4e234dabe7cc40e55c51734ae559b2032`，分支 `dev-plan-P0-P1-mat-dictionary-probe`，运行前提交/推送及GitHub远端SHA核实完成。2026-11-17 GitHub all-state Issue/PR只读检索为空，无已有关联项；以本文本地缺陷ID保留诊断→修复→复测链路，未创建/关闭Issue或PR。

| 首轮步骤（Asia/Shanghai） | 起止 | 退出码/结果 |
|---|---|---|
| configure | 18:49:24…18:49:26 | 0 |
| build | 18:49:26…18:49:55 | 0，无编译诊断 |
| MAT CTest | 18:49:55…18:49:56 | 0，5/5 |
| 其余 CTest | 18:49:56…18:50:00 | 8，30/31；仅新分析测试失败 |

RunId `r6-analysis-baseline-20261117-MKiE6lji`，完整记录 `build/<RunId>/evidence/`；共享源外增量构建目录 `build/r6-20261117-KzjFHPDQ/cmake`，环境Ubuntu24.04.5、GCC16.2.0、CMake/CTest4.3.4、Eigen3.4.0、matio1.5.26、zlib1.3。MAT stdout SHA-256 `38edee5a540ffb8cd54b675a9699c2cf2c1ccc70aff03ab4846de9837f595cc6`，其余 stdout `3898eae9b3bd7b4828d640d117ecf75e1215fb64e51f9b7cb15e988fb0fbdbaa`。原始输入及测试输入完整性检查exit0。

新测试的子进程记录/预期JSON/实际JSON位于 `build/r6-20261117-KzjFHPDQ/cmake/r06_analysis_finite/有限输入 中文/41f37ad7060eb079bcb936a8/evidence/`。夹具与4次CLI进程均exit0；失败来自严格独立比较，不是进程启动错误。两入口32曲线共5578对x/y及3460个完整平滑值一致；UpdateRates20处y_scale差异导致CTest正确失败。8个合成MAT前后摘要相同，4份SVG确实产生，但首轮UpdateRates轴错误，不能称正确图像。

## 证据边界

这是独立解析L3固定输入的分析验证，不是原MATLAB运行记录，不是完整Runner/论文实验或L4统计复现。已有历史MAT的26个NaN继续按既有合同拒绝，原始数据只读；不借有限合成样本宣称这些历史目录分析成功。正式数值、性能与冻结签字仍由人类决定。
