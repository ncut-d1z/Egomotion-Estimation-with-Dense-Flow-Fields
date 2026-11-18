# R6 阶段 Draft PR 交付记录

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18，关联P10.8、REQ/TC/AC-PROC-TRACE-001。已创建并回读核实 [Draft PR #6](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/pull/6)，并附加到当前任务。**Open、Draft、未合并**；不代表人类审查、Ready、正式验收、冻结或发布，R6仍未结案。

## 权限与创建时身份

用户要求按PLAN移植并继续R6，已允许生产代码、构建/CTest及方案文档。PLAN P10.8要求创建或更新Draft供审查，AGENTS.md §7允许阶段成果以Draft交付；TODO中的RELEASE-PR延期针对Draft转Ready、合并、release及正式版本决定。本次只建立Draft审查入口，不恢复上述延期事项，不请求审查人、不关闭Issue。

创建前工作区干净，本地与GitHub branch API一致；全状态PR查询为空，无重复任务PR。关联任务“检查R5实现完成度”当前idle、最后可见用户请求继续检查/实现R6；未向其发送消息。

| 身份 | 创建时核实值 |
|---|---|
| base/main | `eb08c6ba006727c3361af9ea8cb424ece99ba4e0` |
| head/dev-plan-P0-P1-mat-dictionary-probe | `9c0022d2d7c3d19890cc46988f63b0544823d599` |
| 历史关系 | main为祖先，left/right为0/1081；未重写、压缩或合并历史 |
| 创建时间 | 2026-11-18T13:26:39Z（北京时间21:26:39） |
| 创建时diff | 353文件，48782新增行/748删除行；6根文件、113文档、40公共头、1脚本、82源码、80测试、31参考文件 |
| 原资料树 | base/head均`70ee155131c24a50169346d0ffbaff0c01b1c547`，628文件/582MAT未变 |

diff中的历史`PLAN.md → docs/PLAN.md`迁移保留，不是本轮移动/删除。未把原始资料、新生成output/build结果或审批JSON加入PR。本轮后续只登记文档事实，PR随分支接收这些文档提交，创建时快照不追溯改写。

## 审查材料与实测证据

PR正文按CLI/领域值、MAT安全、数值模块、Runner/分析/交付、构建测试及来源分组，列出真实文件入口、五个Issue的诊断→修复→复测链，以及原轨迹失败位置和未完成范围。正文已通过GitHub API回读，与准备的完整文本一致；使用关联Issue编号，未使用自动关闭指令。

- [45源实现映射](../MIGRATION_MATRIX.md)、[98项P复核](R6_PLAN_TASK_AUDIT_2026-11-18.md)、[66项TC证据](R6_TC_EVIDENCE_MAP_2026-11-17.md)保留原任务与验收义务，不以有索引代替逐项验收。
- **实际受测源码仍为`3985b056aa19935c8348f0fefa5f5292f9fbd3b4`**；创建时head只有后续文档差别，CMake、build.sh、src/include/tests/verification/origin逐路径比较完全一致。本轮未重新编译或运行CTest。
- 重新读取既有四份JUnit及登记表，逐名确认ON73/73、OFF72/72、全部status=run、无失败/跳过/禁用；配置/编译/两批CTest原始meta均exit0。739项输入重新计算SHA全部匹配。保存JUnit文件自身SHA，避免仅依赖历史文字摘要。
- 最新[完整合成数值交付证据](R6_REAL_STEP_DELIVERY_L3_2026-11-18.md)记录Ubuntu24.04.5/GNU16.2/C++17及工具链、全部命令/时间/状态、MAT优先顺序、17,441真实Step/12MAT/8,540值与独立参考。合成成功不证明原科学实验成功。
- Issue [#1](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/issues/1)、[#2](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/issues/2)、[#3](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/issues/3)的修复复测已在正文记录；[#4复测评论](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/issues/4#issuecomment-6977604417)、[#5复测评论](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/issues/5#issuecomment-6978398158)另经API核实，未重复发布或改变Open状态。

本轮只读交付核查保存在`build/r6-draft-handoff-20261118-5nZ8yTP0/`：原始Git状态/差异/树身份、`read-only-handoff-audit.json`、739项当前输入校验、准备的PR正文和`github-draft-readback.json`。本轮使用静态Node读取已保存证据，不执行生产程序、Bash追踪脚本、依赖安装或新实验。

同步README、PROGRESS、ROADMAP、TODO、TEST_PLAN顶部事实、NUMERICAL_EQUIVALENCE及P/TC索引，登记实际Draft状态。README两条MAT优先筛选命令补入既有`r06_mat73_version_rejection`；按已保存CTest登记核对为7项MAT与其余66项ON测试的互斥完整分组，仅改文档，没有重跑命令。9份文档451条相对链接有效，98个P与66个规范TC编号保持，TEST_PLAN顶部事实以外的规范正文未变，见`read-only-documents.json`。

## 未完成事项

P10.8的**Draft创建动作已完成**，人类审查与整个REQ/AC不因此通过。原轨迹初始化帧14acos越界、另两Adaptive入口帧151零旋转仍Incomplete；完整Bias/L4、一般病态前向误差、跨环境/资源、最终数值和设计资格均未完成。后续保持原源、数值公式及失败策略，不以合成输入或绿色CTest替代完整科学验证，不催办已无限期延期的审批。PR维持Draft；未合并、关闭Issue或发布。
