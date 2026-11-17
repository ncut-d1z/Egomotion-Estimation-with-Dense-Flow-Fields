# R6 真实CLI与历史分析集成

## 最终复测：35/35 CTest通过

**[HUMAN-EXECUTE/APPROVE] 受测源码 `c42f12f0be89646fd39b93dbb61a79a0ee28e05f` 配置/编译成功，35/35 CTest通过，构建无诊断。** 本段为 `[AI-EDIT]` 事实登记，不代签AC/DEC或冻结。每轮运行前均原子提交、正常推送，并通过GitHub分支API核实 `dev-plan-P0-P1-mat-dictionary-probe` 包含该轮SHA。生产修复为 `src/cli/command.cpp` 的已知子命令帮助；原失败的帮助断言保持且已通过。未修改原始文件、算法、数值阈值或非有限拒绝合同。

RunId `r6-cli-final-20261117-sctymoQQ`，证据目录 `build/r6-cli-final-20261117-sctymoQQ/evidence/`；复用 `build/r6-20261117-KzjFHPDQ/cmake/` 并重新配置受测身份，属于增量构建。环境Ubuntu24.04.5、GCC16.2.0、CMake/CTest4.3.4、Eigen3.4.0、Sophus1.24.6、matio1.5.26、zlib1.3，C++17 Release。Bias固定seed7和level0..2；其余测试种子见 `randomness-source.txt`，RunId由独立OS随机源产生。没有依赖安装、MATLAB/Octave、完整科学实验或L4统计。

| 步骤 | 开始/结束（2026-11-17 UTC+08:00） | 退出码 | 实际结果 |
|---|---|---:|---|
| configure | 18:34:01 / 18:34:03 | 0 | 正确固定构建源码SHA |
| build | 18:34:03 / 18:34:31 | 0 | 全目标通过，无编译诊断 |
| MAT CTest | 18:34:31 / 18:34:32 | 0 | 5/5，0.49秒 |
| 其余CTest | 18:34:32 / 18:34:38 | 0 | 30/30，6.11秒 |
| 输入完整性 | 18:34:38完成 | 0 | 原始资料、独立参考及测试源码SHA一致 |

配置参数 `-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`；构建 `cmake --build <build-dir> --parallel 4 -- -k`。CTest使用 `-C Release --verbose --no-tests=error --output-junit <evidence>.xml`；先 `--parallel 2 -R '^(r02_mat_reader|r06_historical_mat_readonly_schema|r06_mat_independent_storage_values|r06_mat_reader_boundaries|r06_mat_raw_payload_boundaries)$'`，再 `--parallel 4 -E` 同表达式，全部35项无遗漏。完整cwd/命令/参数/时间/退出码/stdout/stderr/JUnit/Testing快照均保留，launcher stderr均为空；测试中的预期拒绝诊断保存在各子进程stderr中。

| 新增CTest | 实际进程数 | 结果与限定 |
|---|---:|---|
| `r06_cli_process_contract` | 42 | 全局/八子命令帮助、非法参数及JSON转义、真实Reader、两种有界Bias/七MAT/Manifest和文件SHA、重复RunId拒绝、生成Bias分析全部符合预期 |
| `r06_cli_process_archive` | 15 | 六组原始输入均实际执行。两种Bias各13曲线与Initialization两曲线成功，发布3份SVG；三个600帧Adaptive目录的analyze/plot均NonFiniteInput/exit5，无伪造图像。正例及负例合同通过不等于六组全部科学分析成功 |

子进程证据在构建目录 `r06_cli_process/CLI 中文 空格/contract_43f5efa1da48da3a83457f90/evidence/` 与 `archive_6e02e5c29e688be4d39f73d7/evidence/`。后者的 `coverage.txt` 明示每组成功/拒绝以及JSON输入摘要null数量；两组Bias各493、Initialization为2，合计988个贡献引用的摘要仍未知，不能声称JSON已携带原始MAT SHA。独立快照核验输入身份，Manifest中实际文件摘要则已全部核对。SVG只验证结构、曲线存在及来源，不宣称全部视觉坐标或独立分析数值复现。

MAT stdout SHA-256 `d7f397f8abb76c7c9dd84f5760ca95968f9d97cebe1894224639c0f43ec7220f`；其余CTest stdout SHA-256 `40e4e7f1815b522c226e18d46e9238c8d85527abdff4ad16ed291451b3202345`。原 `origin` Git树仍为 `70ee155131c24a50169346d0ffbaff0c01b1c547`。四个代码/测试检查点依次为 `3843a814b1b48bea6ba78765e25583d131098f01`、`3986ed5f7958df9a8859b0ddc8109eb8dcd04f77`、`8fad2921ea4346a16492018f0320cb5170ba78b3`、`c42f12f0be89646fd39b93dbb61a79a0ee28e05f`；完整失败/诊断/纠正链保留。未创建/关闭Issue或PR，未合并/发布。

**下一具体缺项**：以明确的有限合成MAT验证Comparison/UpdateRates真实CLI成功路径，并用非线性固定数据独立核对逐值分段/平滑；目前旧平滑测试主要使用线性序列，不能充分区分窗口内部均值错误。本批不把已知原始NaN改写或屏蔽成成功。其余独立L3/L4、完整科学实验、资源资格和正式验收保持未完成，R6未结案。

## 首轮实测与修复候选（历史）

**独立审计轮 `8fad2921ea4346a16492018f0320cb5170ba78b3`**：RunId `r6-cli-archive-audit-20261117-AGxqN2uB`，构建无诊断，34/35；5项MAT测试先通过，独立于matio的原始标签/列主序解码与生产结果仍逐位一致。全部六组输入的15次CLI进程均已执行；两种Bias与Initialization分析/绘图exit0，三个600帧Adaptive目录的analyze/plot均NonFiniteInput/exit5。原始哈希不变，原失败预期及日志均保留。

独立原始字节事实：末轮 `occluded_regions/40/xwerte` 第3/10行为96/1572864，而 `proportional_error/40/xwerte` 为128/2097152。后者与当前生成源码的6·4^i不同，来源成因未确认；Analyzer忠实使用文件值，不能为迎合当前生成公式改写归档。归档共有7个旋转误差MAT含26个NaN、0个Inf：many_parameter的alleadaprotfehler/allemitrotfehler各8个（线性索引151,451,751,1051,1351,1651,1951,2251，即每列第151/451帧），with_errors的alleadaprotfehler/allemitrotfehler/alleohnerotfehler和without_errors的allemitrotfehler/alleohnerotfehler各2个（151,451）。这些是原始L2输入事实，不是本次C++生成的NaN，也不证明其历史计算原因。

据此纠正**测试输入依据**，不改变生产数值或错误合同：两种历史Bias分别验证各自文件值；三个含NaN的历史目录继续实际执行analyze/plot，断言稳定NonFiniteInput、精确文件/变量和无输出目录增加。新生成Bias的96断言保留。原数据、非有限拒绝规则、样本数和任何阈值均不修改；负例通过不能声称这三个历史分析/图表成功。有限输入下Comparison/UpdateRates的真实进程成功路径仍须另用明确合成夹具验证；独立逐值分析L3亦未由本测试替代。

独立MAT stdout SHA-256 `4842b9550317c77a684c1a8ed6ec1a85acc14fdae1d2323e30c4b9064df83bc2`，其余CTest stdout `259c75c027abf95341ba3f6f331f434cda45e08f53ae7268c909f1a407b6c544`；所有证据在该RunId的evidence目录。实际历史分析输出在构建目录 `r06_cli_process/CLI 中文 空格/archive_05b044381007e0ef5cbffa24/`；首轮修复复测的stdout SHA-256为 `33b9adf4ef66f399921827b525f57e2f528cb0369cbff02564e0783e1b175f38`。下一轮以纠正后的真实输入分类复测35项，尚未运行。

**第二轮 `3986ed5f7958df9a8859b0ddc8109eb8dcd04f77`**：RunId `r6-cli-fix-20261117-2fVbZtmW`，构建无诊断，34/35通过。真实CLI合同42次调用全通过，帮助问题已修复；archive模式实际执行后，proportional历史横轴返回128/2097152，而测试误将当前生成脚本的96/1572864当历史文件预期；without_errors的allemitrotfehler被生产Reader/Analyzer报告非有限并退出5，后续JSON解析使其它归档未执行。先增加既有独立MAT解析器的事实输出，记录实际NaN/Inf位置及末轮xwerte，不以生产输出单独决定新预期，不修改原数据、Analyzer非有限拒绝合同或科学算法。测试仍保留当前失败预期，同时让失败输入返回后继续执行其它归档，以获取完整证据。

源码 `3843a814b1b48bea6ba78765e25583d131098f01` 已提交/推送/远端核实；RunId `r6-cli-baseline-20261117-LPFe6rGv`，配置/编译exit0且无诊断，MAT先5/5、其余28/30，合计33/35。真实CLI合同执行41个进程，除八项子命令帮助错误退出2（每项另有stderr断言失败）外，解析/诊断、两种Bias生成/七MAT/Manifest来源/不覆盖、生成结果分析均通过。原始输入哈希未变。

**本地缺陷R6-CLI-001**：生产解析器未处理已知子命令单独的 `--help`。修复在确认命令合法后、解析业务参数前返回Help；未知命令仍失败，不改变算法或运行参数。**测试自身缺陷R6-CLI-TEST-001**：CMake条件的未加引号 `archive` 被同名路径变量展开，导致archive模式在调用CLI前即拒绝；修正为带引号的显式字符串比较。不是历史MAT/Analyzer失败，不能声称六组分析已执行。

保存证据后检索全部open/closed Issue/PR仍为空，未创建远端Issue。首轮证据在 `build/r6-cli-baseline-20261117-LPFe6rGv/evidence/`；其余CTest stdout SHA-256 `32bf963537354e5e9f7bf367dcf044fd89800576127d3c42650fa4083a7b67b7`。实际子进程证据与输出在构建目录 `r06_cli_process/CLI 中文 空格/contract_f24e95ff336e223c6806a5ce/`，均保留。原失败断言不变，待新检查点后复测全部35项。

## 首轮准备（历史）：尚未执行

**[HUMAN-EXECUTE/APPROVE]** 起点 `4b75c046980cde8b382b3725e879d3d8d95fff51` 本地/远端一致且工作区干净，关联任务仍要求继续R6，全部open/closed Issue/PR查询为空。前批33/33只属于 `c218753`；本批新增两项后登记35项，尚未构建或执行。执行前继续提交、推送并核实远端SHA，不执行MATLAB/Octave、完整科学实验或依赖安装。

现有 `r01_cli_contract` 只调用库入口并注入替身；四种分析也主要由合成Reader验证。本批通过 `tests/integration/test_r06_cli_process.cmake` 启动真实 `egomotion` 可执行文件，复用CMake现有JSON解析能力，无新运行依赖。每次CTest创建独立中文/空格沙箱，所有CLI子进程保存参数、cwd、固定构建SHA、UTC起止时间、退出码、stdout/stderr；原始归档逐文件SHA在前后核验，旧输出不删除。

- `r06_cli_process_contract`：全局及八个子命令帮助、版本、未知/重复参数、数值范围、自定义输出根与RunId拒绝；真实MAT元数据与缺变量、带引号/反斜线/中文路径的JSON诊断；两种Bias seed7/level0..2的真实CLI→Runner→七MAT→Manifest，手工/自动RunId、不覆盖、MAT结构/文件SHA/构建及源blob身份，生成结果可进入Bias分析。
- `r06_cli_process_archive`：原始两组40目录Bias、初始化、含/不含噪声Comparison、更新率六组输入；验证四种Analyzer的JSON结构、曲线数、MATLAB区间、来源变量/路径、末轮Bias横轴及可选SVG文件的发布。只读历史分析不等于重新运行生成科学实验。SVG仅检查结构/曲线存在及来源，不声称像素或全部坐标视觉复现；本批也不提供新的独立逐值分析L3。

**源码疑点**：CLI LLD §1要求 `egomotion COMMAND --help` 退出0且无副作用，但解析器仅识别全局帮助，子命令单个 `--help` 会进入奇偶数量拒绝。先保存实际失败再修复；不改变未知命令或其它参数的错误规则。

**来源边界**：当前Analyzer的输入摘要可明确为null，生产 `provenance` 未取得SHA；测试独立保存真实输入文件SHA，仅在JSON确实提供摘要时比较，不把null声称为已核验的输出来源身份。Manifest的MAT SHA则必须与文件完全一致。原分析blob分别为Bias `112aa9eca1fa9728f493d31fc8e59620a8445230`、Initialization `8816675ef5a4eb6767919df36ee11f95de36dac6`、Comparison `3924038f0ccd70de5ea7b60c589afe166da9c8fb`、UpdateRates `67cb613328dec10105a6c383ce17025c4901e4c9`。

关联TC-F-CLI-001/002/004/005、TC-I-CLI-001、TC-I-RESULT-001及TC-I-ANALYSIS-001；Adaptive生成命令仅验证帮助/非法参数，不据此宣称三个完整实验通过。原默认第151帧限制、剩余独立L3/L4、资源资格、DEC-005/006/008和正式冻结/验收保持原状态。
