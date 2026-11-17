# R6 真实CLI与历史分析集成

## 首轮准备：尚未执行

**[HUMAN-EXECUTE/APPROVE]** 起点 `4b75c046980cde8b382b3725e879d3d8d95fff51` 本地/远端一致且工作区干净，关联任务仍要求继续R6，全部open/closed Issue/PR查询为空。前批33/33只属于 `c218753`；本批新增两项后登记35项，尚未构建或执行。执行前继续提交、推送并核实远端SHA，不执行MATLAB/Octave、完整科学实验或依赖安装。

现有 `r01_cli_contract` 只调用库入口并注入替身；四种分析也主要由合成Reader验证。本批通过 `tests/integration/test_r06_cli_process.cmake` 启动真实 `egomotion` 可执行文件，复用CMake现有JSON解析能力，无新运行依赖。每次CTest创建独立中文/空格沙箱，所有CLI子进程保存参数、cwd、固定构建SHA、UTC起止时间、退出码、stdout/stderr；原始归档逐文件SHA在前后核验，旧输出不删除。

- `r06_cli_process_contract`：全局及八个子命令帮助、版本、未知/重复参数、数值范围、自定义输出根与RunId拒绝；真实MAT元数据与缺变量、带引号/反斜线/中文路径的JSON诊断；两种Bias seed7/level0..2的真实CLI→Runner→七MAT→Manifest，手工/自动RunId、不覆盖、MAT结构/文件SHA/构建及源blob身份，生成结果可进入Bias分析。
- `r06_cli_process_archive`：原始两组40目录Bias、初始化、含/不含噪声Comparison、更新率六组输入；验证四种Analyzer的JSON结构、曲线数、MATLAB区间、来源变量/路径、末轮Bias横轴及可选SVG文件的发布。只读历史分析不等于重新运行生成科学实验。SVG仅检查结构/曲线存在及来源，不声称像素或全部坐标视觉复现；本批也不提供新的独立逐值分析L3。

**源码疑点**：CLI LLD §1要求 `egomotion COMMAND --help` 退出0且无副作用，但解析器仅识别全局帮助，子命令单个 `--help` 会进入奇偶数量拒绝。先保存实际失败再修复；不改变未知命令或其它参数的错误规则。

**来源边界**：当前Analyzer的输入摘要可明确为null，生产 `provenance` 未取得SHA；测试独立保存真实输入文件SHA，仅在JSON确实提供摘要时比较，不把null声称为已核验的输出来源身份。Manifest的MAT SHA则必须与文件完全一致。原分析blob分别为Bias `112aa9eca1fa9728f493d31fc8e59620a8445230`、Initialization `8816675ef5a4eb6767919df36ee11f95de36dac6`、Comparison `3924038f0ccd70de5ea7b60c589afe166da9c8fb`、UpdateRates `67cb613328dec10105a6c383ce17025c4901e4c9`。

关联TC-F-CLI-001/002/004/005、TC-I-CLI-001、TC-I-RESULT-001及TC-I-ANALYSIS-001；Adaptive生成命令仅验证帮助/非法参数，不据此宣称三个完整实验通过。原默认第151帧限制、剩余独立L3/L4、资源资格、DEC-005/006/008和正式冻结/验收保持原状态。
