# 人类审批 JSON 的方案落实记录（非测试验收）

> **后续细化**：可逐项审阅的 H-01～H-16 工作材料、MAT 历史差异证据、候选 SHA-256/Golden/异常清单、需求影响和里程碑退出条件，集中于 [HAR-2026-11-06-001 修订材料包](HAR-2026-11-06-001_REVIEW_PACKAGE.md)。它仅补充本文件，不能作为审批记录所缺 `applicationCommit`、`scopeIds` 或真实执行证据的替代。

> **状态：已根据用户在对话中提供的 `HUMAN_APPROVAL_REQUEST.json` 进行文档设计同步；不是已冻结/已交付的软件。** 该 JSON 的 `applicationCommit`、`scopeIds` 为空；不将它写成已签署某个 Git SHA 的完整软件基线。原始用户 JSON **不自动上传至 GitHub**。审批标签以该 JSON 的 `decision`、`referenceConfirmed`、`confirmedOpinion` 和 `technicalDetails.permissions` 为事实来源，预填但未确认的 `referenceText` 不冒充用户签署。

## 1. 各事项原始决定与落实边界

| H 项 | 人类 JSON 中的状态 | 对本次方案的作用 |
|---|---|---|
| H-01 | **draft；referenceConfirmed=false** | 可以起草 Golden、逐模块 atol/rtol 方案，但不能设置数值验收阈值或关闭 DEC-005 |
| H-02 | **approved；referenceConfirmed=true** | **指定** Sophus 取代 `rodrigues.m`；**指定** Don Williamson 原 C++ SHRotation（尤其 Ivanic–Ruedenberg 路线）取代 `SHRotate.m`；复用代码所在目录需 README 标明出处并保留相应第三方 MIT 许可/版权声明。仅批准明确源与方式，不代表其他原始 MATLAB 程序获得公开复制授权 |
| H-03 | **draft；referenceConfirmed=true** | 整理 `cli.md`、`contracts.md` 参数、错误码与 TC→测试/Golden 候选，仍不冻结 DEC-008 |
| H-04 | **approved；referenceConfirmed=true** | 采用已提供的 Ubuntu 24.04 VirtualBox/x86_64 平台快照作为开发目标环境资料。提供了内存、交换空间、磁盘快照，**没有批准具体墙钟/CPU/RSS、构建时间或性能通过阈值**；不能把内存现状视为测试限额 |
| H-05/H-06 | **draft；referenceConfirmed=false** | 准备需求变更影响、设计冻结核对表；SRS/HLD/LLD 不冻结 |
| H-07/H-08/H-09 | **draft；referenceConfirmed=false** | 起草历史 MAT 归因、SHA-256/Reader 拒绝规则、Golden 采集计划；不运行 MATLAB、不关闭 DEC-001/002/004 |
| H-10 | **deferred；referenceConfirmed=true** | **不授权**生产 C++ 实施、CMake/CTest、仿真或数值测试；所有代码和测试继续处于待实施状态 |
| H-11 | **deferred；referenceConfirmed=false** | **不执行** Bash `check_docs_traceability.sh` 现场运行；只可使用连接器进行只读静态分析并如实标注来源 |
| H-12 | **approved；referenceConfirmed=true** | 新产生的数据使用仓库根目录 `output/` 隔离存放，补 `.gitignore` 的 `/output/`；原 `origin/**` 永远只读。此项允许在合适条件下针对隔离副本审核 MAT 写入安全，**但本 JSON 中 `permissions.writeMat=false`，且 H-10 暂缓，故本轮不执行写回读/负例** |
| H-13 | **draft；referenceConfirmed=false** | 先审计 KvD/Adaptive 在 NaN/Inf、奇异/状态提交时的 MATLAB 行为，不批准回滚/继续/终止规则 |
| H-14 | **approved；referenceConfirmed=true** | **方案 A**：优先参考 Don Williamson 原 C++ Ivanic–Ruedenberg 实球谐旋转递推；保持 `l=0,1,2` 旋转块和 9 项系数顺序。版权声明及 Golden/DEC-005 验证仍须执行 |
| H-15/H-16 | **draft；referenceConfirmed=false** | 起草 M0–M5 里程碑、Draft PR/发布核对表；未承诺日期/预算，未授权 PR 合并、发布或创建项目级 LICENSE |

## 2. H-04：已提供的运行环境快照（不是实测性能预算）

来自用户 JSON 中粘贴的 shell 输出，**本轮并未在该虚拟机执行命令**：

| 来源命令 | 记录到的事实 |
|---|---|
| `uname -a` | VirtualBox，Ubuntu 24.04.4（内核标识 `7.0.0-38-generic`），x86_64 |
| `free` | 物理内存总量 `8127380` KiB、当时 available `4908696` KiB；Swap 总量 `4194300` KiB |
| `df` 根挂载 | 根分区总 `205835960` KiB、当时可用 `65347856` KiB |
| 编译器、工具链 | 此段 JSON **没有** `g++ --version`、Eigen、Sophus 或 CMake 的实测版本；此前独立 matio 探针记录 GCC 13/matio 1.5.26，仅代表历史探针环境 |

以上数据只是用户提供的某一时刻容量/系统信息；**不据此设置性能阈值、最大样本量、最大线程或测试超时时间**。Debian 支持与其他发行版尚待验证。

## 3. Sophus 与 Don Williamson 上游材料核查

| 选定技术 | 公开上游、核查范围 | 实施前需保持的条件 |
|---|---|---|
| Sophus SO(3) | [strasdat/Sophus](https://github.com/strasdat/Sophus)，`sophus/so3.hpp`，`Sophus::SO3d::exp(omega).matrix()`；上游 `LICENSE.txt` 的许可文本为 MIT 形式 | 依旧使用 C++17/Eigen，明确可重复安装/版本和 `SO(3)` 朝向；`rodrigues.m` 的逆变换/Jacobian 未有调用需求时不无谓重写，零角/小角必须独立 Golden |
| Don Williamson SHRotation | [dwilliamson/SHRotation](https://github.com/dwilliamson/SHRotation)，核心递推位于 [`Source/IvanicSHRotate.cpp`](https://github.com/dwilliamson/SHRotation/blob/master/Source/IvanicSHRotate.cpp) / [`.h`](https://github.com/dwilliamson/SHRotation/blob/master/Source/IvanicSHRotate.h)，配套 `Source/SHRotateMatrix.h`；根 `LICENSE` 包含 Don Williamson 版权及 MIT 条件 | `Source/SHRotation.h` 是 SDLA 示例应用类，**不是**需要移植的核心递推单元；上游核心代码存在 `sdla/` 依赖，不能把它直接作为 Debian/Ubuntu 上可编译的孤立头文件。采用后应仅纳入经过审核的所需源码与依赖，在**复用代码目录**放 `README.md` 注明上游仓库、原文件路径/版本 SHA 和适用许可，并保留必要版权与 `LICENSE`（作为第三方许可，不是仓库根许可证） |

**版权边界**：用户确认了特定替代来源和采用方案；该决定只覆盖该明确源与方法，**不**表示未知第三方软件、原 MATLAB 的所有函数或归档数据的许可已独立验证。未来实际复制第三方源文件需要遵守相应上游许可文本、保留声明并检查衍生依赖。

## 4. H-10/H-11 与权限位交叉核对

上传 JSON 的 `permissions` 中 `runCMake`、`runCTest`、`runMatlab`、`writeMat`、`runBash`、`measurePerformance`、`mergePR`、`publish`、`createLicense`、`freezeBaseline` **全为 false**。因此**本轮未执行任何上述动作**，只修改审查/实施方案以及 `.gitignore` 的新结果目录排除规则。

**规则**：H-02/H-12/H-14 的具体技术方案选择不推翻 H-10 暂缓开发、H-11 暂缓现场运行，且不等于 H-16 的发布/许可证/基线冻结授权。此登记是文件内容审阅记录，**不是新的人类审批签名**。

## 5. 实际进行的文档静态核对（非 Bash / 非数值测试）

- **只读核对对象**：GitHub 连接器从固定提交 `b2a162fe762c60cb2a6b6e5e82df1da04aeb2f97` 读取真实 SRS、TEST_PLAN、PLAN、OBJECT_MODEL、ARCHITECTURE、.gitignore 及 Git 源树，并运行 JavaScript 集合/依赖断言。
- **核对结果**：REQ **66**、TC **66**、P 工作项 **98**、原始 MATLAB `.m` **45**；SRS/对象模型均映射 45；内部 HLD target DAG **26** 条边且无环；关联 ID、映射及依赖检查错误 **0**。
- **变更安全检查**：此轮源码/原始 `origin/**` 无修改；没有新增 `src/`、`include/` C++ 实现，也没有真实执行 CMake、CTest、Bash、MATLAB 或 MAT 写回读。根目录 `.gitignore` 有 `/output/`，没有通过 `*.mat` 全局忽略原始资料。
- **证据限制**：以上属于 GitHub 内容读取后的**静态一致性检查**，**不**是 `scripts/check_docs_traceability.sh` 现场运行、真实 CMake target 链接、独立 MATLAB Golden、性能或逐元素数值等价的验收记录。
