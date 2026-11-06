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
