# R6 有限 MAT 与非线性分析参考（2026-11-17）

状态：`[HUMAN-EXECUTE/APPROVE]`，用户已授权生产修复及构建/CTest。当前新增测试源码尚未运行；不得据此宣称已通过或正式冻结。关联 R6、P7 分析复现、DEC-004/005/008；不改变需求和容差。

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

## 源码静态差异待实测

原 Comparison 分析 blob `3924038f0ccd70de5ea7b60c589afe166da9c8fb` 明确将两侧纵轴设为log；UpdateRates blob `67cb613328dec10105a6c383ce17025c4901e4c9` 使用 `plot`，没有设置log。当前 `src/analysis/adaptive.cpp` 的 `makeCurve` 统一设为log，疑似错误影响UpdateRates的20条曲线。新测试依据原脚本固定UpdateRates为linear，先取得真实失败再修复；不把修正测试预期作为解决方式。

## 证据边界

这是独立解析L3固定输入的分析验证，不是原MATLAB运行记录，不是完整Runner/论文实验或L4统计复现。已有历史MAT的26个NaN继续按既有合同拒绝，原始数据只读；不借有限合成样本宣称这些历史目录分析成功。正式数值、性能与冻结签字仍由人类决定。
