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

## 本次代码与构建验证请求的交付范围复核

`[AI-EDIT]`，2026-11-18，复核对象为已推送的`e2275d06c20514851ad229971a667b087ac1024d`。关联任务最新可见请求是“检查 R6 代码实现部分是否完成。如果尚未完成，则继续实现 R6”；当前任务另明确允许生产修改、构建/CTest及方案文档修改。**本次请求中的已识别代码缺口补齐、编译和CTest验证已交付；不据此宣布整个R6或PLAN验收完成。** 后续审查发现新缺陷时仍须修复并复测。

| 本次可交付事项 | 当前证据及结论 |
|---|---|
| R6统一构建及可选SVG边界，P9.6/P10.3 | 当前`src/`共62个C++翻译单元全部出现在CMake中，保存的ON编译命令覆盖62个；OFF覆盖60个，仅排除两个SVG实现。没有遗漏业务源文件；两配置实测配置/编译均退出0、无诊断。 |
| 已识别的D1–D5代码缺口，P2.1/P4.9/P5.7/P7.10/P8.8/P8.9/P9.8/P10.9 | 四类自拥有值、KvD/Session策略、球谐/分析职责、具名Runner及依赖组合、真实运行身份记录、MAT v7.3拒绝顺序均有实现与有限回归，逐项修复链仍见[98项复核](R6_PLAN_TASK_AUDIT_2026-11-18.md)。不是总体对象/API正式签署。 |
| 编译、CTest与原数据保护 | 直接重读原始四份JUnit及meta，确认ON73/73、OFF72/72逐名对应实际登记，无失败/跳过/禁用；重新计算739项输入SHA全部一致。受测代码仍是3985b05；当前CMake/build.sh/src/include/tests/verification/origin与其无差异。 |
| 数值和交付回归的实际边界 | 独立L2读取、固定输入L3、真实多帧Step/Session、合成完整日程与MAT交付均有证据。完整合成场景的17,441次真实Step和12个MAT不替代原物理场景；三个原Adaptive入口仍按现有合同失败。 |
| 源码及审查交付 | 45源映射保留明确实现/融合/未使用分支说明；98项P和66项规范TC集合再次匹配。GitHub分支和PR #6的head均核实为e2275d0，Issue #1–#5仍Open；Draft未转Ready、未合并。 |

静态搜索未发现业务TODO/未实现占位；`BackendUnavailable`用于可选绘图缺席或应用装配缺少Reader/Analyzer/Runner时明确报错，不能仅见该错误名就断言生产入口未实现。该搜索及全量编译清单只能证明已列实现和接入情况，不能证明所有输入域无缺陷，也不替代逐项数值验收。

本次追加的只读证据保存在`build/r6-scope-audit-20261118-RdRFcN3p/`，包括检查前Git状态、受测源码差异、`audit.js`与`read-only-scope-audit.json`。四份JUnit SHA保存在JSON中；本次没有新增生产修改、重新构建或运行CTest，不将只读核查计作新测试。

后续独立工作仍包括：原场景的完整科学结果与L4、一般病态输入域资格、其它工具链/平台与性能，以及已延期的最终容差/基线/发布审查。初始化acos越界和零旋转角度无定义是当前明确失败策略的触发；改变公式、钳制、跳帧、样本分母或有限值合同需要具体设计决定，不能以“继续移植”为由默改。保持这些事项开放，不把它们反复改写为“业务代码尚未接通”，也不为重复证明既有通过结果而增加测试。
