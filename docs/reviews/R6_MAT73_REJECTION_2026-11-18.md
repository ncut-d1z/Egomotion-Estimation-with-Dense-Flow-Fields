# R6 MAT v7.3 数值拒绝边界

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18；对应P1.5、REQ/TC-F-MAT-002及任务复核D5。用户已授权生产修改、构建/CTest和方案文档；合成版本负例只用于格式拒绝，不扩大首版MAT v5二维密集实数非logical double范围，不新增HDF5生产依赖或批准数值Golden。

## 起点与分步检查点

本地/远端均为`e32cbc1b93a4c1fea0a0cc1465447e18760f9178`，工作区干净，分支`dev-plan-P0-P1-mat-dictionary-probe`。全部状态Issue/PR列表为空；关联“检查R5实现完成度”任务idle，最新用户要求继续检查/实施R6。上一轮Bias注入完成了真实进展，R6总体仍未完成。

当前Reader在Mat_Open成功、目录/变量选择后才检查版本。所用matio1.5.26的[Mat_Open实现](https://github.com/tbeu/matio/blob/v1.5.26/src/mat.c)先读取128字节头的endian/version字段，版本0x0200进入可选HDF5路径；未构建MAT73时无法打开。这意味着当前错误分类依赖后端能力，尚缺不进入后端的版本拒绝证据。只静态审查上游实现，不复制上游代码。

首个检查点增加`r06_mat73_fixture_generator`（EXCLUDE_FROM_ALL）：明确调用matio创建新的、单变量2×3实数double v7.3文件，再由直接matio重新打开验证版本、目录、形状、类型及全部六个binary64值。预装matio的pkg-config记录已包含HDF5，但生成能力仍待实际确认。生成器只在重建夹具时需要HDF5；普通生产构建/后续拒绝测试将读取固定合成夹具，不强制该能力。

待提交推送并核实SHA后，记录配置/显式生成器构建/夹具生成验证命令、环境、日志与退出码；生成物先留在独立build沙箱，经核验后登记固定SHA-256并作为测试夹具单独提交。随后补数值入口版本预检和named/compatible/full-scan负例，再按新远端检查点执行两模式构建与全量CTest。当前尚未生成夹具或运行新测试，不宣称D5已经完成。

## 执行证据

夹具生成检查点`f1fb7d377a2967304a5f718d3623bfab3ceb2604`已原子提交、推送并核对远端SHA；配置、仅构建生成器、生成及直接matio六值逐位回读均exit0，无编译诊断，原输入摘要检查exit0。RunId `r6-mat73-fixture-20261118-9lQv0oaJ`；日志在`build/r6-mat73-fixture-20261118-9lQv0oaJ/evidence/`，完成于2026-11-18 00:57:42 UTC+08:00。合成文件3408字节、SHA-256 `f6b703ff389c19c8a871009f299ffeb5fcb004273dc7f2b290479cf35e29b2a5`，来源和重建边界见 [夹具说明](../../verification/mat/SYNTHETIC_MAT73.md)。

下一检查点固定夹具并增加`r06_mat73_version_rejection`：3个数值入口、正常/禁用Mat_Open、header短读/EINTR/EIO、大端头变体、兼容后缀、版本优先及v5正常控制。首先对尚未修改的生产Reader执行这个回归，保存真实不符合P1.5的证据；预期会因进入后端/错误分类而失败，不把新登记当通过。之后修复Reader并另行提交推送、复测；完整两模式结果待登记。禁用Mat_Open是确定性替身，不冒充另装的无HDF5库实测。

## 旧Reader实际失败与修复准备

检查点`0d73ed114191debb52bcf514d7a2d3103ca730b5`已推送/远端核实后执行；配置/构建exit0，无诊断，专项CTest **0/1，exit8**：32场景中29失败，3个v5正常控制通过。没有注入后端故障时三个入口也返回`InvalidMatHeader`，实际HDF5诊断是`can't retrieve real path for file`；Reader在封存memfd路径上先调用Mat_Open，根本未到达版本门禁。源资料及固定夹具SHA不变。日志RunId `r6-mat73-baseline-20261118-G1E1eWoz`，全部命令/日志/JUnit/Testing副本在同名build目录的`evidence/`，2026-11-18 01:01:51–01:02:26 UTC+08:00。

保存证据后再次查询all-state Issue/PR为空，仓库无Issue模板，已登记 [Issue #1](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/issues/1)。当前不自动关闭Issue。

修复保持封存快照：完整数值路径在封存输入上读取128字节头，独立识别有效IM/MI标志和0x0200版本，**在Mat_Open之前**返回`UnsupportedClassOrType`/Decode；named保留请求变量，compatible/full-scan尚未枚举变量时保留空变量，不伪造来源。头读取支持EINTR和短读，真实EIO保持`InvalidMatHeader`/Read；短于128字节或非v7.3的文件继续原matio/v5校验，不推断为v7.3。版本拒绝先于不支持格式内的变量选择；空/非法请求名仍按原参数门禁处理。

`scanFile(VerifyDecoded)`也拒绝不支持的数值版本；MetadataOnly不读取完整载荷，仍由实际matio能力决定可否列出元数据，不新增v7.3数值支持。新增测试断言与固定夹具不变，准备对修复检查点执行两模式全量构建/CTest，预计ON55/OFF54项；尚未声明修复已通过。

## 固定修复源码复测

受测源码 **`f1a7ab2aed7795877e070260ebfb1b30a89d6f33`**；执行前已提交、正常推送、核对GitHub branch API同SHA及干净工作区。与基线相比，专项测试源码和固定夹具未更改；修复只前移版本拒绝并保留头读取失败。

| 配置 | 配置/编译 | MAT优先 | 其余CTest | 合计 | 本地起止时间（UTC+08:00） |
|---|---|---|---|---|---|
| SVG ON | exit0，无诊断 | 7/7 | 48/48 | **55/55** | 2026-11-18 01:06:43–01:08:52 |
| SVG OFF | exit0，无诊断 | 7/7 | 47/47 | **54/54** | 2026-11-18 01:09:39–01:12:02 |

每模式均零失败/跳过/禁用，JUnit与CTest逐名匹配；唯一配置差异为SVG专用`r06_svg_source_windows`。四类启动器stderr均为空，未隐藏HDF5/编译诊断。专项 **32场景、308项检查**通过：正常和禁用后端两种条件下，v7.3数值拒绝均为`UnsupportedClassOrType`，`Mat_Open=0`、`Mat_VarRead=0`；EIO保留读取错误，短读/EINTR真实命中且完成判断，文件描述符无泄漏，v5正常控制仍调用后端一次。大端头变体仅验证识别，不宣称另一个v7.3工具链已支持；未装无HDF5 matio的实际构建**未验证**，禁用后端替身不得冒充该平台证据。

原有582MAT/33340值独立逐位核对、256项快照/30数值场景和全部数学/CLI/输出负例在新源码上两模式重跑通过。每模式输入SHA验证exit0；原`origin/`树仍为`70ee155131c24a50169346d0ffbaff0c01b1c547`，固定合成夹具SHA仍为`f6b703ff389c19c8a871009f299ffeb5fcb004273dc7f2b290479cf35e29b2a5`。没有写入原始数据、改变容差、运行MATLAB/Octave、完整科学实验或性能测试。

RunId **`r6-mat73-rejection-20261118-3L89aVqh`**。完整本地证据在`build/r6-mat73-rejection-20261118-3L89aVqh/evidence-ON/`和`evidence-OFF/`：环境、输入Git身份/SHA、随机源定位、命令/起止/退出码`*.meta`、分离stdout/stderr、两份JUnit、原始Testing副本、CMakeCache/编译命令/依赖图/链接证据和`summary.txt`。旧失败及生成证据目录全部保留，不覆盖。平台仍是Ubuntu24.04.5 LTS/x86_64、GNU16.2.0、CMake/CTest4.3.4、C++17 Release、Eigen3.4.0、matio1.5.26、zlib1.3、Sophus1.24.6；生成和旧故障现场HDF5为1.10.10。没有额外安装依赖。

实际命令模板（B分别为`build/r6-20261117-KzjFHPDQ/cmake`、`build/r6-headless-20261117-Joe73gEd/cmake-off`，MODE为ON/OFF，E为对应证据目录）：

```bash
cmake -S "$PWD" -B "$B" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DEGOMOTION_ENABLE_SVG="$MODE" --graphviz="$E/dependencies.dot"
cmake --build "$B" --parallel 4 -- -k
ctest --test-dir "$B" -C Release --verbose --no-tests=error --parallel 2 -R '^(r02_mat_reader|r06_historical_mat_readonly_schema|r06_mat_independent_storage_values|r06_mat_reader_boundaries|r06_mat_raw_payload_boundaries|r06_mat_snapshot_identity|r06_mat73_version_rejection)$' --output-junit "$E/ctest-mat.xml"
ctest --test-dir "$B" -C Release --verbose --no-tests=error --parallel 4 -E '^(r02_mat_reader|r06_historical_mat_readonly_schema|r06_mat_independent_storage_values|r06_mat_reader_boundaries|r06_mat_raw_payload_boundaries|r06_mat_snapshot_identity|r06_mat73_version_rejection)$' --output-junit "$E/ctest-remaining.xml"
```

重跑前需新的远端检查点和独立日志目录，CTest统一取消DISPLAY/WAYLAND_DISPLAY。未创建PR、合并或发布，Issue #1保持开放供人类复核。D5的真实版本负例缺口已补；D4日志、D1/D2结构和Adaptive其它装配边界、完整Adaptive/Bias/L4、正式资格与冻结仍未完成，R6未结案。

文档收尾只同步实际状态：98项P和66项TC编号/顺序与受测提交一致，14个新增证据/夹具链接可解析，`git diff --check`通过；基线与修复之间测试源码/固定夹具diff为空。文档证据提交不是新的受测源码或验收签字。
