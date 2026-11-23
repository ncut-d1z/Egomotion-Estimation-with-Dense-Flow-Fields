# AF-001～AF-012：跨文档审计发现的修订与复核记录（2026-11-07）

> **本文件是纯文档一致性修订后的核对证据，不是 SRS/HLD/LLD 正式基线、可执行 C++ 代码、CMake/CTest 或数值 Golden 的通过证明。** 不改写旧 [FULL_DOCUMENT_AUDIT_2026-11-07.md](FULL_DOCUMENT_AUDIT_2026-11-07.md) 的固定提交与“当时待修订”事实；本文件单独记载其后的修订和复查。

- 审计原始快照：`4627cc1e48be9c43ad10b28fda646d65ee8ef01c`（12 项 AF 在此快照中均为未修订）。
- **全部 AF 文本修订和版本同步后的只读复核快照**：`893acb7b07862c0701e62b786462ef8395a116da`。
- 仓库及分支：`ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields` / `dev-plan-P0-P1-mat-dictionary-probe`。
- 方法：GitHub 连接器按上方固定 SHA 读取源文档及 Git tree，以 JavaScript 逐个断言历史冲突短语已经替换、现行约束存在，并核对追踪 ID、依赖图、相对链接与 Markdown 围栏。**没有**运行 `scripts/check_docs_traceability.sh`、Bash、CMake、CTest、MATLAB/Octave、算法程序或对原 MAT 的新读取/写入。

## 一、逐项结案（结案范围：现行文档语义；不是算法验证）

| 审计 ID | 优先级 | 修订的现行文档 | 修订内容 | 固定快照的复核结果 |
|---|---|---|---|---|
| **AF-001** | 高 | `docs/design/experiments.md` §6.1 | Composition Root 仅确定/核验仓库根固定 `output/` 和可选 `--run-id`，默认 ID 来自独立 OS 源；Writer 二次校验，CLI 不接受自定义输出根 | **文本一致性通过**，无旧“用户输出根”/CLI 输出根校验语义 |
| **AF-002** | 中 | `docs/design/harmonics.md` §3.1.1 | 将“转换上游输出”改为依据 Ivanic–Ruedenberg 1996 原文和 1998 勘误、由项目**纯 Eigen** 生成 R3/R5 并映射原 m 顺序；Don Williamson 仅参考、禁 sdla | **文本一致性通过**；未取得用户上传的论文 PDF、未验数学元素 |
| **AF-003** | 中 | `docs/design/harmonics.md` §4.1 | `egomotion_harmonics→egomotion_contracts` 已在 HLD §4/27 边 DAG 明确，不再重新表述为“若需新依赖”；只在将来边界实际改变时更新 ADR/DAG | **文本一致性通过**；未创建/编译 CMake targets |
| **AF-004** | 中 | `docs/design/contracts.md` §3.2 | 按已选的独立样本可继续、Adaptive 失败不提交、不能安全恢复即停止、八次聚合不得改分母、Manifest 三态/部分证据保留；指向实验/Adaptive 单一合同 | **文本一致性通过**；没有真实异常/回归测试 |
| **AF-005** | 中 | `docs/TEST_PLAN.md` §2.8 | 明确历史 H-01 `draft/referenceConfirmed=false` 没有批准数值阈值；所有者**后来另外**确定 0.01/0.1 首轮筛查，DEC-005 最终 PASS 未冻结 | **历史状态和新决定已分离**；原审批包/JSON 未修改 |
| **AF-006** | 中 | `docs/golden/NO_MATLAB_EXECUTION_POLICY.md` §1 | 已选 `atol=0.01,rtol=0.1` **仅用于首轮宽松探索性筛查**，而非“未选定的候选”；最终验收须独立 L3/真实误差及 DEC-005 | **文本一致性通过**；仍不得宣称正式数值通过 |
| **AF-007** | 中 | `docs/golden/NO_MATLAB_EXECUTION_POLICY.md` §1 | 保留实际上游文件必要的来源/许可告知，但不再把一般版权复核列为原创 C++ 开发阻塞；不擅自对独立作品重新许可 | **文本一致性通过**，MIT 来源边界保留 |
| **AF-008** | 中 | `docs/design/mat_io.md` §2.2 | 统一使用 `contracts.md` 已选的 `Failure<MatErrorCode>` 与轻量 `Result/Status`（内部 variant）；只是 ABI、头文件及异常/资源测试尚未完成 | **文本一致性通过**；生产 Reader/Writer 未实现 |
| **AF-009** | 中 | `docs/OBJECT_MODEL.md` §7 | `near_aus_koeff.m` 的候选方法名统一为 `RealSphericalHarmonics::reconstruct`，与专题 LLD 唯一声明对应 | **文本一致性通过**；无实际头文件/链接 |
| **AF-010** | 低 | `docs/REQUIREMENTS.md` §6 | MAT 输出只允许固定 `output/<RunId>/`，不能把任意不与 `origin/**` 重叠的用户路径当成支持的写出根 | **文本一致性通过**；没有安全负例测试 |
| **AF-011** | 低 | `docs/ARCHITECTURE.md` §3 | 对实际调用向量→旋转矩阵的实现路径，Sophus `SO3d::exp` 是**已选依赖**；其它不调用此能力的模块可不链接，不改变 27 边内部 target DAG | **文本一致性通过**；没有编译/版本验证 |
| **AF-012** | 低 | `docs/design/harmonics.md` §3.1.1 | 旧“上游 C++ 复用路线与法律/构建边界”标题改为“原论文、Don Williamson 参考与 Eigen-only 构建边界”，仅标来源/实施约束 | **文本一致性通过**；不属于上游源码复制或版权执行审批 |

**统计**：AF-001～AF-012 **12/12 均已完成文档措辞和交叉合同修订，并通过各自的限定静态断言**。这不等于每个文件已完成数学逐式审计、没有其它潜在问题、已创建接口、已运行测试或获基线批准。

## 二、同步的草案版本（均未冻结）

| 文档 | 本轮修订后的独立版本 |
|---|---|
| `docs/REQUIREMENTS.md` / `docs/ARCHITECTURE.md` / `docs/OBJECT_MODEL.md` | SRS v0.11 / HLD v0.6 / 对象模型 v0.4 |
| `docs/design/contracts.md` / `docs/design/mat_io.md` | v0.2 / v0.7 |
| `docs/design/experiments.md` / `docs/design/harmonics.md` | v0.3 / v0.7 |
| `docs/TEST_PLAN.md` / `docs/PLAN.md` / `docs/design/README.md` | v0.5 / v0.5 / v0.4 |
| `docs/golden/NO_MATLAB_EXECUTION_POLICY.md` | 政策文字澄清，未赋予正式基线版本 |

版本/SHA 在 [DOCUMENT_BASELINE_REGISTER.md](../DOCUMENT_BASELINE_REGISTER.md) 另按固定快照登记；**不同层次可有不同版本号**，历史 `PROGRESS.md` 中的 v0.4/v0.5 等内容属于当时审查事实，不倒改旧证据。

## 三、复核覆盖与限度

在 `893acb7b07862c0701e62b786462ef8395a116da`：

- 对上述 12 条 AF 的新旧文本做 **12 项定向断言，失败数 0**。
- SRS **66 REQ**、TEST_PLAN **66 TC**、PLAN **98 P**；附录 **8 REQ-DC**、SRS/OBJECT_MODEL 各 **45 .m** 映射；HLD §4.1 **27 条边**静态无环，关联 REQ/AC/TC/P 核对错误 **0**。
- 对本轮修订的 **11 个现行文档**核对 **250 条相对 Markdown 文件链接**，缺失文件目标 0，未配平围栏 0。
- Git tree 原始资料仍为 45 个 `.m` 和 582 个 `.mat`；以上只是 Git 树/文档只读核对，没有本轮新获得原 MAT 数据、L2 SHA-256 或完整独立 L3。
- **AH-001/002 不是待修复的现行缺陷**：早期 HAR/审查报告保留其旧 L1、26 条 DAG、审批标记和旧版本的历史时间点；既有 H-10/H-11 执行限制未被本次改动解除。

## 四、后续真正尚未完成的工作

- 按 [AUDIT.md](../AUDIT.md) 三态继续区分已审计的*文档合同*和仍处于数学深审的 KvD、sampling/noise、trajectory、Adaptive、analysis、harmonics、Golden 独立参考。
- 根据 [TODO.md](../TODO.md) 在相应阶段获取 Ivanic–Ruedenberg 原文＋勘误、生产 C++/CTest/Bash/MAT 安全负例的具体授权，真实数值/性能测量以及正式 DEC-005/006/基线签署。
- 未经明确授权不做编译、测试、MATLAB/Octave、MAT 写入，不复制上游代码；用户所定 Eigen-only/禁 sdla、MIT、固定输出、DEC-004 保持有效。

**结论**：本报告可将 **12 个 AF 在“文档一致性修订”维度标记为已修订、已静态复核**；**不能**将 TC/AC、正式 SRS/HLD/LLD 基线或数值结果标为 PASS。
