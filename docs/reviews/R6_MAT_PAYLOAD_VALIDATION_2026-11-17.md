# R6 MAT内部载荷与维度边界验证

**[HUMAN-EXECUTE/APPROVE] 测试源码已编写，尚未执行。** 起点本地/远端 `88e68407a4516dd823e1ee82913895a5f0bfd5e8` 一致且工作区干净，关联任务最新要求继续R6代码；用户已授权生产修改及构建/CTest。前批受测 `a5336b3` 为28/28通过，不能继承为本批结果。已核查全部open/closed Issue/PR（空）及Reader已有缺陷记录。

本批继续 [MAT LLD §8–9](../design/mat_io.md) 的TC-F-MAT-001/002、TC-D-MAT-002、TC-DC-IO-001：顶层/zlib完整不等于内部miMATRIX长度、dims乘积与实部载荷一致；matio完整解码后的dtype/nbytes不能独立证明物理tag。新 `tests/integration/test_r06_mat_payload.cpp` 不通过生产解析器或matio生成输入，而按文件格式构造字节，压缩仅使用zlib。新增CTest `r06_mat_raw_payload_boundaries`，合计登记29项。

预定70场景，覆盖little/big-endian、plain/zlib：16个正例包含double/负零/列主序、small name、逻辑double的miINT32打包及small real；54个负例包括过短/过长/不完整数值载荷、double class下的miSINGLE、负维度、double字节数溢出、内部tag越界、错误flags/dims标签或长度、未知数值tag、miMATRIX声明长度不一致、压缩流中多余元素。拒绝精确错误码并核对来源；原始582 MAT和合法miINT32打包继续参加全量回归。此处拒绝single升级沿用AGENTS，未决定所有未来物理存储类型的正式允许集合，也不改变MATLAB class支持范围。

每个小夹具在fork子进程中测试，临时RLIMIT_AS=256 MiB、RLIMIT_CPU=5秒仅约束该测试进程，避免旧Reader按恶意维度分配或无限循环；信号/超时/异常退出一律算失败，**这不是生产资源预算或性能阈值**。父进程保留每例SHA和结果，所有输入写build独立RunId沙箱并验证读取前后hash。既不改原始资料，也不采用NPZ/Python或运行MATLAB。首轮仅取得真实失败证据后再定修复，不先猜测全部负例行为。

格式依据为MathWorks MAT v5说明及 [matio 1.5.26解析/写出源码](https://github.com/tbeu/matio/blob/v1.5.26/src/mat5.c)的静态对照；MathWorks当前PDF入口重定向登录，未据无法读取的PDF编造结论。没有复制上游代码。独立夹具与生产验证器各自实现，正例及原档逐位结果用于交叉排除夹具编码错误。

待本轮提交/推送/远端核实后，配置编译，先运行MAT5项，再运行其余24项；若MAT先失败则保留失败证据后修复再执行全量。R6的完整科学实验/剩余L3/L4/正式验收仍未完成。

## 首轮失败与修正候选

源码 `1b1349df1b32bd2e880e00bf4f8c70103c736fe6` 已提交/推送/核实。RunId `r6-mat-payload-baseline-20261117-OfDyO8XW`：配置/构建exit0，无编译诊断；MAT5项中4通过、raw载荷失败（exit8），按先MAT顺序未执行其余24项。70场景中22通过、48失败：**34个非法载荷被静默接受**，14个已拒绝但分类不符；16个正例全部通过，无信号退出或内存上限触发。原始582文件测试仍通过。原始输入及参考/测试SHA核对exit0。完整日志在同名build目录 `evidence/`，MAT stdout SHA-256 `79249885793694084a852bb320ab74cbb5a72a0750633221b0ff2ef408721544`。所有合成失败文件保留在构建树 `r06_mat_raw_payload/原始标签 合成/output/run_55d8b137f7feaf234e6e3d268b29a1c1/`。

**本地缺陷R6-MAT-002**：`MatResultReader` 的完整读取只校验外层文件边界/zlib，随后matio按维度解码并给出内存nbytes，不能检出实部tag长度矛盾、未知tag或single升级。普通/压缩的短、长、零碎数值、single-under-double、内部tag越界、flags长度与未知storage均被接受；压缩内root短/长/多余元素也被忽略。负dims、字节溢出、错误dims type及普通root-short返回通用错误。全状态Issue复查为空，未创建远端Issue，不对matio作者作未经定位的缺陷归因。

候选修正为私有 `mat_payload_validation.{hpp,cpp}`：在任何`Mat_VarRead`前，以固定缓冲流式检查每个miMATRIX的flags、signed dims及乘法/解码字节溢出、普通/small字段边界、实际数值元素宽度×数量、根长度、完整压缩结束和剩余字节；仍由matio拥有实际数值解码。保留同一fd身份，支持原582个double及miINT32 packed原文件，不自行转换新数值。double class下single载荷明确拒绝；具名读取仍可选择另一个合法变量，不会仅因无关变量使用single存储就拒绝所选值。struct/cell/sparse仅检查当前层边界，**不宣称递归类型支持/全部恶意结构安全，也未设生产资源预算**。inspect路径保持metadata-only。

修正回归保留原70场景，增加4个混合文件具名选择、16个流式跨块正例，合计90场景；新增仅测试链接的 `--wrap=Mat_VarRead` 计数，要求每个非法载荷在调用matio数值读取前拒绝，并验证完整字典扫描同样拒绝。更广raw存储集合的正式冻结、嵌套恶意对象与性能/资源资格仍待后续，不以本批代替完整R6。

## 第二轮：90个raw场景通过，定位旧char夹具错误

`926f07871dbc524f2630879a5a59d994fa80d44a`，RunId `r6-mat-payload-fix-20261117-oRRxVQOE`：配置/编译exit0且无诊断，新增90/90场景全部通过，582 MAT/33,340值仍精确一致；MAT5项中4通过，旧29场景的 `zlib-char` 错误分类断言失败。输入SHA核对exit0，日志SHA-256另由该批 `evidence/` 保留；未执行其余24项。

已实际只读检查失败文件字节（审计RunId `r6-char-layout-20261117-QUuz0FOI`，Node18.19.1内置fs/zlib，无安装、无生产Node依赖）：plain-char解压长度72、root声明64，结束位置72正确；zlib-char解压长度同为72，却root声明72、结束位置80。两者均class4、shape2×3、UTF8 type16、6字符abcdef。后者是**matio 1.5.26生成器对UINT8 char压缩根长度高估8字节的夹具问题**，新校验拒绝该不一致符合合同。静态源码的 `GetTypeBufSize` 与 `WriteCompressedCharData` 路径支持此诊断；不修改第三方代码或放宽生产根长度检查。

把旧“合法但不支持的char”夹具输入编码明确为 `MAT_T_UTF8`，保留相同字符/shape、UnsupportedClassOrType及metadata断言；另由独立字节构造器增加4个同样错误的char-root-long负例，要求在matio读取前拒绝，**不删除失败语义或忽略原损坏文件**。raw场景因此90→94，旧边界仍29。诊断和历史生成文件均保留，接续待新检查点后全量复测。
