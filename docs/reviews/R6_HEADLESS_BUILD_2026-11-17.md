# R6：可选 SVG 的独立构建与无显示环境验证

`[HUMAN-EXECUTE/APPROVE]`。用户已授权生产代码、构建/CTest与方案文档修改；正式数值/资源资格及合同冻结仍未完成。关联 `REQ/TC-NF-DEPS-001`、`REQ/TC-NF-HEADLESS-001`、`TC-F-AN-002`，不改变原义务、阈值或实验范围。

## 缺口与准备

静态对象 `f1634419067db11fbffe02043f003da5aa91606f`：CMake 无条件创建两个 SVG 库，CLI 库公开链接 plotting，main 无条件实例化两个导出器。已有空后端注入例不能证明实际可执行文件在禁用绘图构建时可用。SVG 本身没有外部 GUI 依赖，但“没有 GUI 依赖”与“不构建 SVG”是两项不同证据。2026-11-17 检索仓库全部状态 Issue/PR 返回空列表；此缺口以本记录 `R6-HEADLESS-001` 跟踪，不另建重复 Issue。

- 新增 CMake `EGOMOTION_ENABLE_SVG`，默认 `ON` 保留现有行为；`OFF` 不创建或链接 `egomotion_plotting` / `egomotion_illustration_svg`。
- CLI 库仅依赖绘图接口，main 按构建选项注入具体后端或空后端。未改 CLI 参数、错误 schema、算法公式或数值阈值。
- 所有 CTest 子进程移除 `DISPLAY` / `WAYLAND_DISPLAY`；三个实际进程脚本断言该环境并保存配置记录。
- 预期 ON 登记48项，OFF登记47项：仅 `r06_svg_source_windows` 属于未构建后端。插图混合测试保留两轨迹600点、全部45组0–8阶独立Rodrigues参考及数值负例；仅导出器专属断言随选项关闭。
- 真实CLI保留全部MAT检查、限定Bias生成、历史/有限分析、三个Adaptive失败合同和三类插图JSON。OFF额外检查合法绘图请求的退出码7、JSONL和不预留输出；非有限原始输入仍按原数值错误拒绝，不改变优先顺序。

此段为**未运行的检查点准备**。新源码需先提交、正常推送、远端SHA核实，再分别执行 ON / OFF 配置、编译、MAT优先及其余CTest。运行日志、工具链、输入SHA、链接产物审计与实际计数另行追加；此前130a82的48/48不得代替本次结果。原始 `origin/**` 保持只读，测试写入独立沙箱。
