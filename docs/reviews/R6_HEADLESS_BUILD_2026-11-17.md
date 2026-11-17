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

## 固定源码的实际结果

受测提交 **`f3398b099c70233e55e150ecf41c2dd90f282c00`**，分支 `dev-plan-P0-P1-mat-dictionary-probe`；执行前正常推送并通过GitHub branch API核实相同SHA，工作区干净。RunId **`r6-headless-20261117-Joe73gEd`**，执行者为获本轮授权的Codex。未新增Issue/PR、未合并/发布；`R6-HEADLESS-001` 的上述构建缺口已取得限定运行证据，R6整体仍未完成。

| 配置 | 配置/构建 | MAT优先 | 其余CTest | 时间（UTC+08:00） |
|---|---|---|---|---|
| `ON` | 既有独立目录重新配置、增量构建，退出0，无编译诊断 | 6/6 | 42/42，合计48/48 | 2026-11-17 22:35:19–22:36:55 |
| `OFF` | 全新目录配置、完整构建，退出0，无编译诊断 | 6/6 | 41/41，合计47/47 | 2026-11-17 22:37:03–22:41:40 |

两个JUnit集合逐项核对：仅ON有 `r06_svg_source_windows`；没有跳过/禁用/失败项。插图检查ON为536225项、OFF为492879项，差额为SVG导出/几何显示及发布故障断言，全部原数值采样仍执行；两种数值最大绝对/缩放误差相同（约5.73e-9/2.18e-10），沿用原工程回归阈值，不构成DEC-005签署。

- 两种模式的真实CLI合同各42次、历史分析各15次、插图各28次调用通过。OFF下3次历史plot、2次有限输入plot、3类显式RunId插图SVG及1次自动RunId插图SVG共9次合法绘图请求，均返回退出码7/`BackendUnavailable`且不创建输出。非有限历史数据仍按原错误优先顺序返回5。
- 两模式均完成582份MAT固定SHA/33340逐位值、封存快照256项、两Bias的有限生成、历史Bias独立参考、32条有限曲线全JSON、独立几何和三个Adaptive失败合同。Adaptive仍Incomplete、无完整MAT；该错误合同通过不等于科学实验成功。
- 对已有CTest产物另作只读跨模式比较：seed=7、level=2的两Bias共14份MAT在128字节文件头之后的全部数据载荷逐字节一致（头含创建时间，不声称整文件SHA相同）；三类插图默认JSON全文逐字节一致。比较命令退出0，时间22:41:47；没有新增实验或改样本/阈值。
- 两批输入文件完整SHA前后核对退出0。原始628文件/582 MAT未变，`origin`树仍为 `70ee155131c24a50169346d0ffbaff0c01b1c547`。

## 依赖、环境与证据位置

实际环境为Ubuntu 24.04.5 LTS、Linux 7.0.0-43-generic x86_64、GCC 16.2.0、C++17 Release、CMake/CTest 4.3.4、Eigen 3.4.0、matio 1.5.26、zlib 1.3、Sophus 1.24.6。使用既有依赖，未安装软件；未验证其它发行版/编译器组合或性能预算。

OFF的编译清单、链接命令、库文件与ELF已定义符号均无两个SVG实现；ON的两个真实导出符号存在。CMake DOT实图（包含测试目标）ON为61节点/101边、OFF为58节点/94边，DFS检查均无环；生产项目内直接边为35/30条。这是实际构建图，不改动HLD拟议图的正式冻结状态。

ELF直接动态依赖为matio、zlib、libstdc++、libm、libgcc_s、libc；Eigen/Sophus是编译期依赖。当前系统matio间接带入HDF5及其加密/网络等系统库，完整清单保留在`elf-transitive.stdout`；**不声称运行只需两个共享库，也不因此扩大Reader的MAT格式支持范围**。两模式实际链接/传递依赖均无Python/NumPy/MATLAB/Octave/ROS/OpenCV/sdla/cnpy及Qt/GTK/X11/Wayland图形库，未发现缺失库。源码外部进程调用名检查无匹配；全部CTest移除显示服务变量，三组进程测试显式断言环境。

本地证据均为忽略目录内保留的实际文件，不提交原始/生成MAT或敏感日志：

- 工作目录：仓库根；运行脚本及模式比较：`build/r6-headless-20261117-Joe73gEd/{run.sh,compare-modes.sh}`。
- 分模式证据：同目录`evidence-ON/`、`evidence-OFF/`，含每步`.meta/.stdout/.stderr`、环境、Git blob/输入SHA、前后完整性核验、JUnit及各批CTest的`Testing`副本。
- ON构建：`build/r6-20261117-KzjFHPDQ/cmake/`；OFF全新构建：`build/r6-headless-20261117-Joe73gEd/cmake-off/`。各CLI沙箱、参数、输出及逐进程日志的完整位置在CTest verbose记录中。
- 配置命令使用`-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DEGOMOTION_ENABLE_SVG=ON/OFF`及各模式独立DOT输出；构建`--parallel 4 -- -k`。MAT六项`ctest --verbose --no-tests=error --parallel 2 -R ...`先行，其余用同一正则`-E`及`--parallel 4`，均保存JUnit；完整命令逐字记录于`.meta`。
- 同目录`dependency-audit.json`、`compare-modes.{stdout,stderr,exit}`保存图审计与跨模式一致性；每种配置保存`CMakeCache.txt`、`compile_commands.json`、`CTestTestfile.cmake`、`egomotion-link.txt`、ELF直接/传递依赖及符号。配置/编译/CTest启动器stderr为空；预期失败子进程的JSONL在对应逐进程stderr保留。

接续工作：独立复核日志/输出合同和完整P/TC证据覆盖。三Adaptive完整成功、L4完整科学实验、病态数值资格、资源与正式DEC/冻结签署仍未完成；不修改数值策略来取得虚假成功。
