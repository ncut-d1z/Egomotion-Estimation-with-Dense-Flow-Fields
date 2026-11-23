# R6 Bias 求解器与随机源注入

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。按用户明确授权修改生产代码、构建/CTest和方案文档；关联 P5.7/P10.9、REQ-DC-EXPERIMENT-001、TC-F-BIAS-001/002/003、TC-NF-SESSION-001。实现接口仍是候选、未冻结，不改变数值公式、阈值或正式验收状态。

## 检查点准备

从本地/远端一致的 `4039bfd18cd1f554c4c6e0932cec67f826e59c2a` 接续，分支 `dev-plan-P0-P1-mat-dictionary-probe`。GitHub all-state Issue/PR 列表为空。原始 `origin/**` 保持只读；MATLAB 两入口的第16/22行两次 seeding、第51/52行噪声列/细分层循环仅静态核对，没有运行 MATLAB/Octave。

- `IFranzMatchedFilter` 和 `IKvDEstimator` 分别保留 MotionEstimate/KvDResult；Franz、Original、Modified 三个槽通过借用的 `BiasEstimators` 注入。生产默认服务由 ProductionExperimentRunner 拥有，计算函数不再创建具体求解器。
- Bias compute 接收 `IRandomSource&`，生产调用传入 RunContext 独占的 RNG。`reseed(seed)` 返回 Status，明确不支持重置的流失败，不替换成隐藏 RNG；Mt19937 重置引擎和分布缓存。初始化前、真值后各一次重置，Adaptive 的连续流调用不新增重置。
- 保持唯一采样/噪声配对、原 KvD 参数、六指标/列主序、`xwerte=6*4^level`、callback 与失败传播位置。原 Bias 数值/真实 MAT 发布回归只调整装配调用，不改期望值和阈值。
- 新 `r06_bias_dependency_injection` 使用固定求解替身及可观察 RNG：两模式完整事件序列，三求解槽/六指标映射，seed7/11交错运行，首/次重置、真值/近度 uniform、normal 及各求解器失败的精确前缀，非重置流拒绝，奇数 normal 后缓存重置。Production/RunContext 另用内存 Writer/Publisher 验证七字段、RunId、计数和部分交付；这些替身不构成真实 MAT 持久化证据。

检查点时新源码尚未构建/执行。完成原子提交、正常推送与远端 SHA 核实后，已在两个隔离构建目录执行 SVG ON/OFF 配置、编译和 MAT 优先全量 CTest，结果如下。

## 实测证据

受测源码 **`909fd57dbafa4ebf680e5edd5fa718adb30b4c51`**；分支如上，执行前 GitHub branch API 返回同一 SHA，工作区干净。执行者为获授权的 Codex；没有创建/关闭 Issue 或 PR，没有合并、发布或安装依赖。

| 配置 | 配置/编译 | MAT优先 | 其余CTest | 合计 | 本地起止时间（UTC+08:00） |
|---|---|---|---|---|---|
| SVG ON | exit 0，无诊断 | 6/6 | 48/48 | **54/54** | 2026-11-18 00:36:03–00:40:38 |
| SVG OFF | exit 0，无诊断 | 6/6 | 47/47 | **53/53** | 2026-11-18 00:41:15–00:45:08 |

两个模式均零失败/跳过/禁用，配置、编译、CTest启动器stderr为空；JUnit与CTest登记逐名核对。OFF仅缺SVG专用`r06_svg_source_windows`，实际链接和符号检查未发现SVG实现。平台 Ubuntu24.04.5 LTS/x86_64，GNU16.2.0、CMake/CTest4.3.4、C++17 Release、Eigen3.4.0、matio1.5.26、zlib1.3、Sophus1.24.6；没有扩大为其它发行版/编译器已验证。

- 新测试每模式 **668项检查**，涵盖完整/交错运行、2种模式×9种故障共18个场景、默认不可重置流、normal缓存重置及6次Production/内存交付场景。固定seed7/11；事件/身份/位级重放精确比较，六指标独立弦长公式沿用原Bias的1e-9回归余量。不是新批准的数值容差或科学样本数。
- 原真实Bias合同 **17695项**、真实MAT发布 **5560项**，均零失败、最大绝对残差 **6.6613381477509392e-16**；原求解参数/期望值/阈值未修改。其它原有数学、582MAT/33340逐位值、CLI、Session、错误/持久化负例全部在这次两模式重跑。
- `origin/` Git树仍为`70ee155131c24a50169346d0ffbaff0c01b1c547`；每模式`inputs.sha256`覆盖原始资料、verification和测试输入，运行后逐文件检查exit0。原始582MAT未写入或变更。

RunId **`r6-bias-injection-20261118-ptpZAp0s`**。本地完整证据保存在`build/r6-bias-injection-20261118-ptpZAp0s/evidence-ON/`和`evidence-OFF/`（忽略、不上传生成结果）。每目录包括`environment.txt`、Git输入身份/文件SHA、随机源定位、命令/起止/退出码`*.meta`、独立`*.stdout`/`*.stderr`、两份JUnit、CTest原始Testing副本、CMakeCache/编译命令/依赖图/链接证据及`summary.txt`。执行包装脚本为同RunId目录下`run.sh`。

实际配置/构建命令模板（B分别为`build/r6-20261117-KzjFHPDQ/cmake`和`build/r6-headless-20261117-Joe73gEd/cmake-off`，MODE为ON/OFF，E为对应证据目录）：

```bash
cmake -S "$PWD" -B "$B" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DEGOMOTION_ENABLE_SVG="$MODE" --graphviz="$E/dependencies.dot"
cmake --build "$B" --parallel 4 -- -k
ctest --test-dir "$B" -C Release --verbose --no-tests=error --parallel 2 -R '^(r02_mat_reader|r06_historical_mat_readonly_schema|r06_mat_independent_storage_values|r06_mat_reader_boundaries|r06_mat_raw_payload_boundaries|r06_mat_snapshot_identity)$' --output-junit "$E/ctest-mat.xml"
ctest --test-dir "$B" -C Release --verbose --no-tests=error --parallel 4 -E '^(r02_mat_reader|r06_historical_mat_readonly_schema|r06_mat_independent_storage_values|r06_mat_reader_boundaries|r06_mat_raw_payload_boundaries|r06_mat_snapshot_identity)$' --output-junit "$E/ctest-remaining.xml"
```

重跑须遵守AGENTS远端检查点及独立新证据目录，不覆盖本次记录。两模式所有CTest子进程均取消DISPLAY/WAYLAND_DISPLAY。仅文档证据登记发生在受测源码之后，不声称文档提交SHA被重新构建。

证据整理后静态核对：98项P和66项TC编号/顺序与受测提交完全一致，10个新增专项报告引用可解析，`git diff --check`通过；没有据此更改AC义务或宣布正式验收。

## 未覆盖边界

本次补齐任务复核D3的 Bias 求解/RNG 注入缺口；Adaptive其它候选装配边界、D1/D2公共结构、D4非生成日志和D5真实MAT v7.3负例继续保留。完整Bias/L4、三个Adaptive真实完整成功、正式数值/资源资格及冻结未完成。保留既有失败样本，不变更容差，不宣称原MATLAB运行轨迹等价。
