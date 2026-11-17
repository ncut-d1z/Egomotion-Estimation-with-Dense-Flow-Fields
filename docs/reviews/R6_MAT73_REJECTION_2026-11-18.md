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
