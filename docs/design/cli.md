# CLI 参数、退出码与诊断协议：详细设计草案

> **候选 v0.1，未冻结、未编码、未执行 CTest。** 对应 REQ-F-CLI-001～005、REQ-I-CLI-001、REQ-I-RESULT-001 和 P0.8/P1.2。来源为 [SRS](../REQUIREMENTS.md) `6.1、[实验 LLD](experiments.md) 与 [公共错误合同](contracts.md)。本文是设计评审材料，不代替所有者批准。

## 1. 语法与基本约束

- 进程入口：`egomotion COMMAND [OPTIONS]`。`egomotion --help`、`egomotion COMMAND --help` 和 `egomotion --version` 不读取 MAT、不执行算法、退出 0。
- 未知命令/选项、缺参数、单值选项重复、非法枚举/整数/浮点数都退出 **2**，不采取“最后值胜出”。
- 必填输出路径必须指向 origin/** 之外；所有 run 命令只创建新的 RunId 目录，已存在则退出 6，不得默认覆盖。原始归档只读。
- 解析时不改变 MATLAB 原值：35/600 点、二十面体细分级别、8 次比较、四个更新间隔 1/5/11/21、噪声乘数不擅自改变。
- 路径保留 Debian/Ubuntu 的 UTF-8/中文/空格语义；不把路径拼接结果当成可靠的防符号链接绕过实现。

## 2. 命令级输入合同

| 命令 | 必需选项 | 可选选项 | 已知输出、选择与来源边界 |
|---|---|---|---|
| `inspect-mat` | `--file PATH` | `--variable NAME`、`--mode strict/compat`（默认 strict） | 未指定变量时仅列举元数据；strict+NAME 指定变量；compat 且不带 NAME 时模拟原 getflowfield 读首变量。compat+NAME 视为参数错误；多变量首字段顺序待 Golden 解决 |
| `run-bias-occluded` | `--seed UINT32 --level NONNEG_INT --output-root DIR` | `--run-id ID` | 使用缺损八面体、平均光流模长噪声；level 为包含端点的最大递归级别；写 xwerte 和 ywerte1…6 |
| `run-bias-proportional` | 同上 | `--run-id ID` | 使用完整八面体和逐分量噪声；保留原作者 xwerte 与真实 N 不同的行为 |
| `run-adaptive-init` | `--output-root DIR` | `--run-id ID` | 35 点、level=2；四类初始化误差 MAT |
| `run-adaptive-comparison` | `--noise-scale FINITE_DOUBLE --output-root DIR` | `--run-id ID` | 600 点、level=4、8 次重复；含噪/无噪比较及 rottransratio，当前五类 MAT；噪声为 0 时仍消耗原有 randn |
| `run-adaptive-update-rates` | `--output-root DIR` | `--run-id ID` | 600 点、level=2，周期固定 1/5/11/21；当前脚本只写三个 MAT，不伪造历史额外数据 |
| `analyze` | `--experiment bias/adaptive-init/adaptive-comparison/adaptive-update-rates --input-root DIR` | `--output-root DIR` | 四种分析路径；只读取 MAT，不调用求解器；缺少字段显式失败 |
| `plot`（S） | 同 analyze | `--output-root DIR` | 绘图后端可缺席（exit=7），不得影响数值与分析命令 |

### 2.1 参数类型与不变量（待 P0.8 签署）

| 类型 | 拟议的精确校验 | 待决定 |
|---|---|---|
| UINT32 | 完整十进制输入，0 至 4294967295；禁止负数、溢出和尾随字符 | 固定 seed 的跨 MATLAB/C++ 序列不承诺相同 |
| NONNEG_INT | 完整十进制非负整数；先检查 4 的幂、样本数量及分配溢出 | 不凭空设定人为最大 level/实验规模 |
| FINITE_DOUBLE | C locale 完整解析，拒绝 NaN、Inf、尾随内容 | 负噪声系数的 MATLAB 语义及是否接收需批准 |
| ID | 候选 ASCII 字母/数字/下划线/连字符，1–64 字符；禁止点号、斜杠、反斜杠、空格 | 缺省 RunId 自动命名规则、并发创建策略待批准 |
| PATH / DIR | 单个非空实参，解析后检查实际路径、输入只读/输出边界 | 符号链接/TOCTOU 的安全实现必须通过真实负例验证 |

## 3. 退出码、错误阶段与错误优先级（候选）

| exit | 类别 | 示例 |
|---:|---|---|
| 0 | 成功 | help、version 或正常完成 |
| 2 | 参数错误 | 未知/重复/缺少参数、非法枚举、整数格式 |
| 3 | 输入或 MAT 读取错误 | 文件不存在/拒绝访问、坏 MAT 头、损坏压缩块、缺变量 |
| 4 | 变量类型/布局错误 | 不支持的 MAT class/data_type、rank/dims 不匹配、长度溢出 |
| 5 | 数值或实验失败 | SingularSystem、NonFiniteResult、SolverFailed、IncompleteRun |
| 6 | 输出路径/结果安全冲突 | origin 路径、已有 RunId、写失败、独立回读不一致 |
| 7 | 可选功能不可用 | 仅 plot 绘图后端不存在 |

阶段检查顺序为 **parse（2）→ input/read（3）→ decode/schema（4）→ output safety（6）→ solve/analyze（5）→ plot capability（7）**。如果一个请求包含多个问题，报告实际先触发的失败，保留原始错误码及上下文。具体 `MatErrorCode` 到退出码的映射应与 [contracts.md](contracts.md) 审批后保持单一来源。

失败诊断最少包含稳定的 `error_code`、`exit_code`、`stage`、`message`；涉及 MAT 需附 `path` 与已知 `variable`，涉及实验需附 RunId、算法与样本下标。正常输出使用 stdout、错误使用 stderr，文件里仍保存原 MATLAB 兼容 MAT。**诊断序列化格式（JSONL/键值行）、stderr 转义规则和 RunId 生成方式尚未批准**，不得将候选协议称为可机读的已实现 API。

## 4. 验收映射与冻结界线

| TC | 必要测试 | 证据状态 |
|---|---|---|
| TC-F-CLI-001 | 有效/无效 MAT、变量、strict/compat、源哈希不变 | 只有独立 matio 探针 582/582 读取的子项证据 |
| TC-F-CLI-002 | 两类 Bias、seed/level、输出安全与变量形状 | 未编写/未运行 |
| TC-F-CLI-003 | 三种 Adaptive、noise-scale、原更新间隔与当前写出的 MAT | 未编写/未运行 |
| TC-F-CLI-004 | analyze 四条路径独立运行、不触发估计器、缺字段失败 | 未编写/未运行 |
| TC-F-CLI-005 | 缺绘图后端 exit=7，核心命令不受影响 | 未编写/未运行 |
| TC-I-CLI-001 | 帮助、必填与默认值、错误优先级和诊断字段 | 未编写/未运行 |
| TC-I-RESULT-001 | RunId 不覆盖、变量名/类型/形状、新 MAT 回读 | 未编写/未运行 |

**冻结前必须完成**：项目所有者批准该候选 SHA 与待决定项，SRS/HLD/contracts 同步确认；P1.2 有实际 CLI 代码及 tests/integration/test_cli.cpp/CTest 证据；输出路径负例与原始 MAT 哈希核验。没有这些证据时本文继续是设计草案。
