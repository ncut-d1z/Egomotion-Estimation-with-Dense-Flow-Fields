# R6 MAT内部载荷与维度边界验证

**[HUMAN-EXECUTE/APPROVE] 测试源码已编写，尚未执行。** 起点本地/远端 `88e68407a4516dd823e1ee82913895a5f0bfd5e8` 一致且工作区干净，关联任务最新要求继续R6代码；用户已授权生产修改及构建/CTest。前批受测 `a5336b3` 为28/28通过，不能继承为本批结果。已核查全部open/closed Issue/PR（空）及Reader已有缺陷记录。

本批继续 [MAT LLD §8–9](../design/mat_io.md) 的TC-F-MAT-001/002、TC-D-MAT-002、TC-DC-IO-001：顶层/zlib完整不等于内部miMATRIX长度、dims乘积与实部载荷一致；matio完整解码后的dtype/nbytes不能独立证明物理tag。新 `tests/integration/test_r06_mat_payload.cpp` 不通过生产解析器或matio生成输入，而按文件格式构造字节，压缩仅使用zlib。新增CTest `r06_mat_raw_payload_boundaries`，合计登记29项。

预定70场景，覆盖little/big-endian、plain/zlib：16个正例包含double/负零/列主序、small name、逻辑double的miINT32打包及small real；54个负例包括过短/过长/不完整数值载荷、double class下的miSINGLE、负维度、double字节数溢出、内部tag越界、错误flags/dims标签或长度、未知数值tag、miMATRIX声明长度不一致、压缩流中多余元素。拒绝精确错误码并核对来源；原始582 MAT和合法miINT32打包继续参加全量回归。此处拒绝single升级沿用AGENTS，未决定所有未来物理存储类型的正式允许集合，也不改变MATLAB class支持范围。

每个小夹具在fork子进程中测试，临时RLIMIT_AS=256 MiB、RLIMIT_CPU=5秒仅约束该测试进程，避免旧Reader按恶意维度分配或无限循环；信号/超时/异常退出一律算失败，**这不是生产资源预算或性能阈值**。父进程保留每例SHA和结果，所有输入写build独立RunId沙箱并验证读取前后hash。既不改原始资料，也不采用NPZ/Python或运行MATLAB。首轮仅取得真实失败证据后再定修复，不先猜测全部负例行为。

格式依据为MathWorks MAT v5说明及 [matio 1.5.26解析/写出源码](https://github.com/tbeu/matio/blob/v1.5.26/src/mat5.c)的静态对照；MathWorks当前PDF入口重定向登录，未据无法读取的PDF编造结论。没有复制上游代码。独立夹具与生产验证器各自实现，正例及原档逐位结果用于交叉排除夹具编码错误。

待本轮提交/推送/远端核实后，配置编译，先运行MAT5项，再运行其余24项；若MAT先失败则保留失败证据后修复再执行全量。R6的完整科学实验/剩余L3/L4/正式验收仍未完成。
