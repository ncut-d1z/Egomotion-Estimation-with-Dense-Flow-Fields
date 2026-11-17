# R6 MAT v7.3 数值拒绝边界

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18；对应P1.5、REQ/TC-F-MAT-002及任务复核D5。用户已授权生产修改、构建/CTest和方案文档；合成版本负例只用于格式拒绝，不扩大首版MAT v5二维密集实数非logical double范围，不新增HDF5生产依赖或批准数值Golden。

## 起点与分步检查点

本地/远端均为`e32cbc1b93a4c1fea0a0cc1465447e18760f9178`，工作区干净，分支`dev-plan-P0-P1-mat-dictionary-probe`。全部状态Issue/PR列表为空；关联“检查R5实现完成度”任务idle，最新用户要求继续检查/实施R6。上一轮Bias注入完成了真实进展，R6总体仍未完成。

当前Reader在Mat_Open成功、目录/变量选择后才检查版本。所用matio1.5.26的[Mat_Open实现](https://github.com/tbeu/matio/blob/v1.5.26/src/mat.c)先读取128字节头的endian/version字段，版本0x0200进入可选HDF5路径；未构建MAT73时无法打开。这意味着当前错误分类依赖后端能力，尚缺不进入后端的版本拒绝证据。只静态审查上游实现，不复制上游代码。

首个检查点增加`r06_mat73_fixture_generator`（EXCLUDE_FROM_ALL）：明确调用matio创建新的、单变量2×3实数double v7.3文件，再由直接matio重新打开验证版本、目录、形状、类型及全部六个binary64值。预装matio的pkg-config记录已包含HDF5，但生成能力仍待实际确认。生成器只在重建夹具时需要HDF5；普通生产构建/后续拒绝测试将读取固定合成夹具，不强制该能力。

待提交推送并核实SHA后，记录配置/显式生成器构建/夹具生成验证命令、环境、日志与退出码；生成物先留在独立build沙箱，经核验后登记固定SHA-256并作为测试夹具单独提交。随后补数值入口版本预检和named/compatible/full-scan负例，再按新远端检查点执行两模式构建与全量CTest。当前尚未生成夹具或运行新测试，不宣称D5已经完成。

## 执行证据

待执行。
