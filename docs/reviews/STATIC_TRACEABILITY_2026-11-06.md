# 文档追踪与 MAT 字典静态审查记录（2026-11-06）

> **状态：已执行连接器数据上的只读静态核查；不是 Bash 检查器现场执行、CMake/CTest 或算法 Golden。** 本次在 GitHub 插件中按固定 commit SHA 读取真实文档内容和递归 Git 树，使用与 `scripts/check_docs_traceability.sh` 对应的 JavaScript 断言执行 ID/双向追踪/依赖环核查，并额外核对 MAT 数据字典逐行数据。未修改 origin。

## 1. 执行范围与环境

- 代码仓库：[Egomotion-Estimation-with-Dense-Flow-Fields](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields)
- 目标分支：`dev-plan-P0-P1-mat-dictionary-probe`
- **被审核提交（冻结读入）**：`1395b4f2709123ed0884a655968536fbe69d65e0`
- 数据来源：GitHub 连接器 `fetch_file(ref=SHA)` + `GET git/trees/SHA?recursive=1`，不是缓存旧摘要。
- 运行方式：当前对话中执行的连接器支持的只读 JavaScript 集合检查；**未运行** `bash scripts/check_docs_traceability.sh`。本机容器试图通过 `git ls-remote` 获取仓库时因 `Could not resolve host: github.com` 失败，故不将该脚本标为已实际执行。
- 工具环境：JavaScript 只读检查，不依赖项目 CMake/Eigen/matio 业务构建；脚本在 Debian/Ubuntu 完整工作树上的直接执行仍待记录。
- 结果：**PASS（此处明示的静态断言）**；错误与缺项 **0**。

## 2. 已执行断言及结果

| 检查 | 实际观察值 | 结果 |
|---|---:|---|
| 正式需求 REQ（唯一 ID/每项匹配 AC） | 66 | PASS |
| TEST_PLAN 的 TC（一项对一项匹配 REQ/AC） | 66 | PASS |
| PLAN 工作项 P（唯一 ID、REQ/TC/AC 配套、已知依赖） | 98 | PASS |
| P 工作项依赖环 | 0 | PASS |
| 未被 P 工作项引用的 REQ | 0 | PASS |
| Git 源树 .m 文件与 SRS/OBJECT_MODEL 双向路径集合 | 45/45/45 | PASS |
| HLD §4.1 target DAG 已解析边及环 | 26 条；环 0 | PASS |
| HLD 已声明的 11 组组件直接依赖与 DAG 对应 | 26/26 条预期边 | PASS |
| MAT_DATA_SCHEMA 实际逐文件行 | 582 | PASS |
| 原始 MAT 的字典状态 | `READ_OK` 582/582 | PASS |
| MAT 版本/变量类型/压缩/变量名与文件名一致性 | 582/582，MAT v5 / double / zlib | PASS |
| 原始 MAT 的五类 shape | 10×3: 480; 10×1: 80; 35×1: 4; 600×1: 14; 600×4: 4 | PASS |

**解释**：此记录重新核对既有 582 行字典的内部一致性，**并未在本轮重新解压 582 个原始 MAT 文件**；原始文件解压/读取证据仍为历史 matio 探针报告。源文件逐值 Golden、MAT 来源 SHA-256 独立复核、生产 Reader/Writer、CLI、真实 CTest 和版权审查一律未因此通过。

## 3. 已读取文档的 Git blob SHA

| 文件 | blob SHA |
|---|---|
| `docs/REQUIREMENTS.md` | `87d7c2be35362ec020306078649682e288cd5219` |
| `docs/TEST_PLAN.md` | `585521111f4876cb74ddf161586d12668d7f1e18` |
| `docs/PLAN.md` | `4b5f7ad404b3d5de0ad091d763fe4797ad8a0de2` |
| `docs/OBJECT_MODEL.md` | `91700b878c396039c1556892f45d7da200787b8a` |
| `docs/ARCHITECTURE.md` | `049061a620f8de9b6c8b029cb813dfefd7089f94` |
| `docs/MAT_DATA_SCHEMA.md` | `a472f76798d265dbeb8422de32fe78214d3f7ed8` |
| `scripts/check_docs_traceability.sh` | `3699abbbb151830f59904d8a7193478681f3e2d8` |

## 4. 复核命令及尚未完成的证据

在具有完整原始归档、能访问上述提交的 Debian/Ubuntu 工作树上使用：

```bash
git switch dev-plan-P0-P1-mat-dictionary-probe
git rev-parse HEAD
bash scripts/check_docs_traceability.sh
```

上述命令**尚未在本轮容器中执行成功**，不得把本报告写成 Bash/CTest 通过。正式复核时应保存 stdout/stderr、退出码、源 commit SHA、执行环境与日志路径，并在新记录中明确标记执行者和日期。真实 CMake target 编译验证仍待 P1.1；DEC-003/005/007/008 及真实 Golden/数值容差仍保持开放。

## 5. 文档交叉引用更新后的再次核查（2026-11-06）

- 固定目标提交：`5f6b831977157f5bb7ea431d361bf40e7b33e345`（含 SRS/HLD/LLD/TEST_PLAN 修正、PROGRESS 与静态核查链接）。
- 再次通过 GitHub 连接器读取该提交的 **实际文件内容和递归源树**，执行 JS 集合检查；**非 Bash/CTest 执行**，没有克隆或编译仓库。
- 结果：**PASS，错误 0**。REQ 66、TC 66、P 98；原始 MATLAB 45 个，SRS/OBJECT_MODEL 均 45/45；HLD DAG 26 条边无环；MAT 字典 582 行均 READ_OK（未重新解压原 MAT）。
- 额外核验：LLD 索引 10 份文档表述；TEST_PLAN 的 40 个 10×1 形状；PROGRESS 里程碑索引存在。
- 现场 Bash 检查器仍待网络可达或实际完整工作树环境执行；任何未执行的步骤均不计为通过。

## 6. 文档执行口径与引用修复后的再次静态核查

- **被审查 Git 提交**：`cb0a6e8cccec9d2326f35f1d60d9ed44432b0390`。
- **已执行动作**：通过 GitHub 连接器按该 SHA 读取 SRS、HLD、PLAN、TEST_PLAN、OBJECT_MODEL、MAT_DATA_SCHEMA、AGENTS、CLI/公共合同、审批申请表及递归 Git 树，以 JavaScript 断言检查集合和依赖关系。**未运行 Bash、CTest、完整 CMake 构建或数值 Golden**。
- **结果**：REQ=66、TC=66、P=98、源 `.m`=45、SRS/OBJECT_MODEL 两份映射均为 45、HLD target DAG=26 条边且无环；MAT_DATA_SCHEMA 中原有 582 条记录均 `READ_OK`；所有一致性检查错误 **0**。**未重新解压原始 MAT 文件**。
- **文档状态检查**：TEST_PLAN §8.1 已将 Bash 现场命令明确标为“待授权、未执行”，并分表列出已执行连接器静态审查及未执行 Bash/CTest；`cli.md` 与人类审批表的章节引用格式已修正。
- **审批状态**：审批申请现由 `docs/approval/HUMAN_APPROVAL_REQUEST.html` 提供；其 H-01～H-16 默认均为待决定（旧 Markdown 审批表的历史版本仍可从 Git 提交查询），`AGENTS.md` §10 的人类审批门禁仍有效。**此条记录不是 H-11 Bash 执行授权，不构成 DEC/CLI/Golden/CTest 的批准或验收**。
