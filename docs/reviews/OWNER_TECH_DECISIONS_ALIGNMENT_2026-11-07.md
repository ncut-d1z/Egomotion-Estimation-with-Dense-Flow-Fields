# 项目所有者技术决定落实与静态核对（2026-11-07）

> **检查范围限定**：本轮根据项目所有者明确确认的 Eigen-only Ivanic–Ruedenberg、根 MIT、删除 CLI 自定义输出根、Result/JSONL、独立实验配置/失败保存与宽松数值筛查等决定，更新文档与根 LICENSE。**没有开始 R5 生产实现**；用户稍后上传 1996 原论文及 1998 勘误后才进行相应代码工作。没有运行 Bash、CMake、CTest、MATLAB/Octave、MAT 写入/回读、科学仿真或数值验收，未合并 PR/发布。

## 1. GitHub 修改及可核对来源

- 仓库：`ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields`，分支：`dev-plan-P0-P1-mat-dictionary-probe`。
- 本轮开始的 Git HEAD：`672bfd8d8a79485e66275bd49a7cda7f52f509a0`。
- 在进行本记录前的**只读静态核对提交**：`168bb5d16cc3519023a3985ee14e80ceee60c791`。后续登记报告和 PROGRESS 的提交不属于该固定 SHA 的断言对象。
- 最新明确决策见 [OWNER DECISIONS](../decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md)，源论文/勘误和严格 Eigen-only 约束见 [SH_ROTATION_IVANIC_RUEDENBERG.md](../design/SH_ROTATION_IVANIC_RUEDENBERG.md)；新仓库 MIT 文本见 [LICENSE](../../LICENSE)。

## 2. 已落实的决定与实际状态

| 事项 | 落实位置 | 可证明结果 / 仍未完成 |
|---|---|---|
| 项目 MIT | 根 `LICENSE`、`README.md`、`AGENTS.md`、SRS/PLAN/decision | 根许可文本已创建，项目原创软件采用 MIT；原始第三方资料不自动被重新许可，正常开发不再因版权复核单独阻塞。未涉及第三方文件复制/发布 |
| 原论文/勘误 | `docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md`、`harmonics.md`、HLD/SRS | 1996 DOI 10.1021/jp953350u 和 **1998 correction DOI 10.1021/jp9833350** 已记录；ACS 1996 可能需机构权限，1998 标为 Free to Read。**用户 PDF 尚未上传，不能声明逐式核对完成** |
| 纯 Eigen，完全禁用 sdla | `harmonics.md`、HLD §4/§4.1、`PLAN.md` | target/LLD 已确定 Eigen-only，Don Williamson 仅为算法参考，禁止所有 sdla include/link/vendor；**未写生产递推，无编译/依赖扫描证据** |
| CLI 删除可配置输出根 | `cli.md v0.2`、SRS v0.5、TEST_PLAN/PLAN、`mat_io.md v0.6` | 运行与持久 plot 输出固定 `output/<RunId>/`；RunId 可选，自动 ID 与实验 RNG 分离；Reader 纯读取不会创建目录；**没有已实现 CLI/Writer** |
| 错误协议 | `contracts.md`、`cli.md` | 轻量 Result/Status 封装内含 C++17 variant，ErrorStage 枚举，结构化 JSONL；**无头文件/ABI/CTest 证明** |
| 失败/Manifest | `experiments.md` | 独立类型 Config、依赖状态失败不提交、不能安全延续则终止、8 次聚合缺次不得改除数、Manifest Complete/Incomplete/Failed，保留部分证据不覆盖；**无真实实验执行** |
| 数值筛查 | SRS/PLAN/TEST_PLAN、`GOLDEN_TOLERANCE_PROPOSAL.md` | `atol=0.01`、`rtol=0.1` 已选为**首轮探索性筛查**；MAT 原值/形状/状态和几何不变量有严格独立检查。**DEC-005 最终分模块验收阈值与数据实测仍未确定** |
| 开发授权 | `AGENTS.md`、最新 decision | 阶段一改代码 → 阶段二构建/CTest/隔离负例 → 阶段三验收/发布。**本轮仅文档和 LICENSE**，不解除历史 H-10/H-11 具体运行暂缓；不会擅自做 R5 代码 |

## 3. 本轮执行的只读 GitHub + JavaScript 核对

- 固定提交 `168bb5...`：**66 个 REQ、66 个 TC、98 个 P**，核对对应 ID、P 依赖的存在性/无环。
- HLD 的静态 target DAG：**27 条边**，新增 `egomotion_harmonics → egomotion_contracts` 后无环。过去日期的 26 条历史检查记录未修改；**仍未创建/测试实际 CMake target**。
- 在当前活动合同文件 `AGENTS.md`、SRS、PLAN、TEST_PLAN、CLI、MAT I/O、README 中不再出现旧的输出根 CLI 参数名。最新**决策记录**允许引用已删除的选项名作为历史变更说明。
- `contracts.md` 的轻量封装、`ErrorStage`、JSONL、`experiments.md` 的 `RunCompleteness`、`harmonics.md` 的 Eigen-only/禁用 sdla 及根 MIT 文本均在固定提交静态查验，检查范围内 **0 项不一致**。
- 原始 `origin/**`、生产源码 `src/**`、实际代码构建与全部测试没有在本轮修改/执行；Git 目录树哈希差异应以最终读取为准。

## 4. 不再需要重新作的决定，以及当前真正的未完成项

**无须再反复审批的设计选择**：根 MIT、Eigen 替代 sdla、原论文和 1998 勘误为必读、删除 CLI 自定义输出根、统一 Result/ErrorStage/JSONL、独立 Runner 配置、严格状态/统计不变量及首轮宽松筛查。原 MATLAB/Octave 运行和 L1 Golden **永久非强制**。

**真实未完成项**：用户原始论文 PDF 上传、R5 完整递推的逐式核验与 C++ 实现、Reader/Writer/RunContext/Result/JSONL 的真实代码及 ABI、独立 L2 SHA-256/逐值与 L3 固定输入、CTest/数值/安全/统计证据、最终分模块验收阈值、阶段性具体执行权限及人类基线验收签署。项目原创 MIT 不构成这些数学/工程义务的替代。
