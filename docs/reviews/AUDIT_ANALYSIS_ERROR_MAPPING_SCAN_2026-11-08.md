# AF-077：AnalysisErrorCode 与统一 CLI 错误映射的静态审计（2026-11-08）

审计固定提交：`4ffbbf93d0acf8177bbc4d9dfd28ed6aaaef197b`。原 `docs/design/analysis_plotting.md §6` 候选声明的七个错误为：`MissingResult`、`MissingVariable`、`ShapeMismatch`、`InvalidSegment`、`NonFiniteInput`、`UndefinedStatistic`、`UnsupportedExperiment`。但 `docs/design/contracts.md §3.1` 的退出码映射仅含 MatErrorCode、NumericalErrorCode、ExperimentErrorCode；`docs/design/cli.md §3` 只引用此公共表，没有 AnalysisErrorCode 的来源到用户可见退出码/阶段归属。

**风险**：从分析接口返回 `AnalysisErrorCode::MissingVariable` 时可能被当成“解码失败（4）”“分析失败（5）”或“输入缺失（3）”，与 MatErrorCode::MissingVariable 的已列 read（3）语义产生不一致；`UnsupportedExperiment` 的 CLI 参数错误与内部 Analyzer 不支持的运行状态也不能混为一谈。

**目标**：在 contracts 作为唯一候选来源补跨层分析状态映射；`analysis_plotting.md` 与 `cli.md` 仅引用它，`TEST_PLAN.md` 对缺变量/非法段/shape 等标明期待 stage/exit 及错误来源；保留 `ErrorDetail.source_error_code`。最终错误行为仍需 DEC-008/P0.8 评审批准和真实 C++/CTest，静态映射不等于已验收。

未更改旧审批、原 `origin/**`、算法公式、MAT 数据，也没有执行 Bash/CMake/CTest/MATLAB。
