# R6 Adaptive 真实生成CLI验证

任务标签 `[HUMAN-EXECUTE/APPROVE]`，使用本轮生产代码、构建/CTest及文档授权。仅在隔离CTest沙箱进行三个实际入口各一次的装配/输出回归；比较使用noise_scale=0，原35/600点、level2/4、八次聚合分母与首个数值失败终止规则均保持。不是L4统计、论文图表复现、性能测试或数值资格认证；不运行MATLAB/Octave，不修改原始资料、公式/门限或正式审批状态。

## 当前证据缺口

既有 `r06_adaptive_runner_*` 包括StepSolver替身、真实轨迹和固定600行聚合；`r06_real_session_interleaving` 使用真实单步与固定有限输入。它们不能替代实际 `egomotion run-adaptive-*` 的进程装配、真实求解、失败审计及输出证据。此前真实CLI只覆盖这些入口的帮助/非法参数，并未执行生成逻辑。

本轮新增 `r06_adaptive_real_cli`，登记CTest由45增至46；**尚未执行，不宣称通过**。运行前仍先提交、正常推送并核实远端SHA。第一次预期基于静态源配置及既有轨迹边界：初始化应完成33个内部帧；比较/更新率的第151 MATLAB帧真实旋转为零，既有角度合同应显式UndefinedAngle并保持149个有效前缀。如果真实求解更早失败，将保存全部实际诊断并定位，不能用替身预期反向改公式或伪造成功。

## 测试范围

- `run-adaptive-init`：默认35点/level2、不注入替身。核对Complete、33有效帧、四个当前save变量及35×1 shape、Manifest字段/源blob/编译SHA/实际OS生成算法种子、文件SHA/大小。以真实inspect/analyze再次读取结果，拒绝重用RunId且保留全部证据。
- `run-adaptive-comparison --noise-scale 0`：默认600点/level4/计划8次；噪声为零仍抽取原法正态样本。检查实际非成功JSONL、完整因果链与Manifest一致、正确帧号/计数、分母保持8，不发布部分MAT或伪造Complete。
- `run-adaptive-update-rates`：默认600点/level2/周期1/5/11/21；检查同类失败证据、计划1次及无额外HistoricalOnly产物。
- 每入口各执行一次生成；重复RunId只检查输出拒绝，不能再运行数值过程。每个子进程有120秒保护上限，CTest420秒；这是回归防挂起上限，不是DEC-006性能预算。相关源参数不为测试缩减。

`r06_adaptive_cli_values` 只读新初始化产物和原初始化L2：复用测试专用独立MAT v5/zlib标签解析，比较真实Reader解码的全部140个double位模式，核对名称、shape、端点原零和有限角度范围。历史L2按原列主序输出全量CSV及每字段最大绝对差，仅作诊断，不据无批准的容差自动宣布科学等价。它不运行求解器、不以生产Reader生成参考、不写任何输入MAT；真实文件SHA前后比较。

全部输出保存在带中文/空格路径的独立 `build/.../r06_adaptive_cli/...` 沙箱。每次调用记录argv、cwd、实际build SHA、起止时间、stdout/stderr/退出码；实际随机seed来自各Manifest，不能用目录随机身份冒充算法seed。外部原始资料由全套先行五项MAT及运行前后摘要继续保护。无新的CLI参数、无新公共schema/算法依赖。

## 后续判定

仅实测可填写结果。三个入口通过合同测试不等于三种科学实验全部成功；原始无定义角度、有限精度拒绝及独立L2差异均须保留。R6剩余正式数值/L4、跨环境、资源与审批义务不因测试数量增长关闭。
