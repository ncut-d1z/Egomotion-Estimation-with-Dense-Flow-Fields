# AF-013～AF-020：按整改进度台账完成纯文档修订（2026-11-07）

> **本记录是指定 Git 版本上的文档语义复核，不代表源码实现、编译、CTest、MATLAB/Octave 运行、MAT 读写或独立 L3 数值验收。** 原 [AUDIT_PROCESS.md](../AUDIT_PROCESS.md) 的历史扫描快照与旧版 [FULL_DOCUMENT_AUDIT_2026-11-07.md](FULL_DOCUMENT_AUDIT_2026-11-07.md) 的事实继续保留，不倒改历史报告。

## 1. 固定身份与修订范围

- 仓库：`ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields`；分支：`dev-plan-P0-P1-mat-dictionary-probe`。
- 用户指定待整改问题的审计输入提交：`3beb8c06a9d40380431f99e31ed0f70b346863a2`。
- **本轮六份目标文档及 LLD 索引修订后的只读复核提交：`a782b5c8d86e50fe4d2ae4e2b198b3cd6345de52`**。
- 工程边界：只修改 **Markdown 文档**；`origin/**` 保持原样和只读；没有实施 C++ 类/头文件、CMake/CTest、Bash、任何 MAT 写入或数值/性能运行。原论文 PDF 尚待用户上传。
- 复核方式：通过 GitHub 连接器读取固定提交内容及 Git tree，使用 JavaScript 逐项断言**新决定的正确文本存在且指定过时短语消失**；核查 REQ/AC/TC/P 与 HLD 静态依赖关系、相对 Markdown 链接。没有现场执行任何 Bash 静态脚本。

## 2. AF-013～020 逐条整改与限定验证

| 整改 ID | 影响的当前文档 | 改动内容（仅设计合同） | 固定快照结果与仍缺证据 |
|---|---|---|---|
| AF-013 | `docs/design/kvd.md` §5、§6 | 废止历史 `rtol=1e-10` 作为现行候选；`atol=0.01/rtol=0.1` 已选为**首轮宽松探索筛查**，最终 DEC-005/严格几何、状态、迭代检查另行验收 | 文本断言**通过**；无真实 KvD 逐轮 L3/CTest |
| AF-014 | `docs/design/kvd.md` §10 | 将 H-13 原审批的“仅起草”写明为**当时的历史授权状态**，并引用后来所有者已定的**单次失败显式传播、独立样本可由 Runner 记录后继续、Adaptive 无效状态不提交/无法恢复停止、保留部分结果及八次聚合分母不变**；不冒称已验证 MATLAB 异常 | 文本断言**通过**；尚无异常回归/原运行数据 |
| AF-015 | `docs/design/sampling_noise.md` §3.1 | 采样或 RNG 出错时单次调用必须传播错误，实验整体处理按已确定的 Runner/Session 合同；删除“仍待重新选型”的口径 | 文本断言**通过**；未运行固定 RNG 和独立 L3 |
| AF-016 | `docs/OBJECT_MODEL.md` §4 | 状态图规定单步成功且有限才提交；失败无效状态不提交新主系数；不能安全恢复就终止轨迹，独立样本可记录失败后继续；区分已选策略与未来恢复路径资格 | 文本断言**通过**；未构建 Session/执行 CTest |
| AF-017 | `docs/ARCHITECTURE.md` §7 QA-01 | 删除“CLI 用户设置输出根”的不可能场景；改为非法/重复 `--run-id`、固定 `output/` 被软链或 TOCTOU 替换、目标已有文件时拒写并保持 `origin/**` 不变 | 文本断言**通过**；TOCTOU/写入拒绝负例未执行 |
| AF-018 | `docs/design/mat_io.md` §4.2 | 外层分析的 0.01/0.1 是**已选首轮筛查、非正式 PASS 阈值**；MAT 解码/数据转移仍严格核对 class/dtype/rank/dims/列主序与原始 double，不允许丢值/转置 | 文本断言**通过**；真实 Reader/Writer、原值逐项、SHA-256 与 CTest 未运行 |
| AF-019 | `docs/ARCHITECTURE.md` §8 ADR-002 | Sophus `SO3d::exp` 已选用于旋转向量到矩阵的实际调用路径，**不是“拟采用”**；版本锁定、编译/链接和 L3 仍未完成 | 文本断言**通过**；未编译 Sophus 或任何 target |
| AF-020 | `docs/design/trajectory.md` §5.1/§6.1 | 保留原 `rodrigues.m` 在旋转向量模长小于 double `eps` 时返回单位矩阵；与 `drehvektor_aus_zwei_vektoren.m` 的 **1e−11 叉积门槛**区分，只有新增阈值扩大/非法输入处理才需后续验证 | 文本断言**通过**；近零、反向、球外独立 L3/CTest 未执行 |

## 3. 静态复核范围与不可扩大解释

在固定提交 `a782b5c8d86e50fe4d2ae4e2b198b3cd6345de52`：

- **AF-013～020：8/8 定向文本一致性断言通过**。LLD 索引同时更新为 v0.5，`mat_io.md` 为 v0.8；其它修订后草案版本：KvD v0.4、采样/噪声 v0.2、对象模型 v0.5、HLD v0.7、轨迹 v0.2。
- **追踪与设计结构**：66 REQ、66 TC、98 P、附录 8 REQ-DC；HLD 静态依赖 DAG **27 条边、无环**，P 引用及前置集合限定检查无不一致。
- **引用与数据身份**：在本轮复核的十份当前文档中核查 **242 条 Markdown 相对文件引用**，未发现缺失目标。Git 树仍有原 `origin/**` 中 **45 个 .m/582 个 .mat**；未重新读取 MAT、重新生成字典或开展数值比较。
- **所有者决定未变**：Eigen-only/禁 sdla，1996 原文与 1998 勘误待用户上传，固定 `output/<RunId>/` 无自定义输出根，轻量 Result/ErrorStage/JSONL，DEC-004 HistoricalOnly，首轮宽松筛查非最终验收，MATLAB/Octave/L1 永不强制。
- **没有执行 MATLAB/Octave、Bash、CMake、CTest、生产代码或 MAT 写入；没有新的人类执行授权或正式基线签署**。

## 4. 下一步整改状态与剩余审核

经此提交的**文档合同修订与固定 SHA 复核**，可把 [AUDIT_PROCESS.md](../AUDIT_PROCESS.md) 中 AF-013～020 的整改状态由「未整改」更新为「已整改」，需引用本报告而不能凭台账生成即视为修复。**仅针对已登记八个整改目标结案，不等于完整数学逐式、性能或软件验收完成**。

[ AUDIT.md ](../AUDIT.md) 的文件审计三态与本问题级整改状态保持独立：KvD、采样/噪声、轨迹及部分球谐/Adaptive/分析/容差参考仍需独立 L3 和后续真实 C++/CTest；仅文档合同层面的对象模型/HLD/MAT I/O 可依据复读核对归为“已审计”，仍标注未冻结、未执行。
