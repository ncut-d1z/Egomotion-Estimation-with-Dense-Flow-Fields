# R6 插图数值与记录目录身份接续（2026-11-20）

`[HUMAN-EXECUTE/APPROVE]`。按所有者“继续代码实现，推迟构建、测试”，从 Draft [PR #6](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/pull/6) 的 `c0e6135884f49fc0eafb0089909681179010810c` 继续实施。前批停止阈值、格式化完整性和诊断修复见 [上一批记录](R6_FORMATTING_FAILURE_CONTINUATION_2026-11-20.md)。**本批只修改源码和状态文档，未执行配置、编译、语法编译、CTest、测试程序、CLI、MAT I/O 或数值实验。**

## 1. 插图有限输入的法向溢出

公共插图 `Data` 的坐标有限、网格有效，并不保证三角形光照计算有限。取首个三角形的三个顶点 `a=(R,0,0)`、`b=(-R,0,0)`、`c=(0,R,0)`，则原叉积为 `(0,0,-2R²)`。以下是源码与 double 表达式的静态反例，没有运行复现：

| 有限输入 | 原代码的路径 | 本批拒绝位置 |
|---|---|---|
| `R=1e100` | 顶点模长和叉积有限，但法向平方模溢出，开方得到 Inf；有限点积除以 Inf 得到零，原代码把失败掩盖为零光照。 | `normal.norm()` 后返回明确的法向模长溢出错误。 |
| `R=1e154` | 各顶点平方模约为 `1e308`，相机半径检查可通过；叉积约为 `-2e308`，溢出后光照可成为 NaN，继续转换为整数颜色不安全。 | 叉积之后、求模之前拒绝非有限法向。 |

[src/illustrations/svg.cpp](../../src/illustrations/svg.cpp) 按原顺序检查法向、法向模长和光照有限性，并在 RGB 浮点值转换为 `int` 前检查该整数类型的可表示范围。没有改变叉积、光照、RGB 公式或 double 精度，没有重新缩放法向、钳制亮度或引入科学阈值。原零面积面仍采用 `length==0 → light=0`，其颜色保持 `rgb(70,130,160)`。

失败复用原 `InvalidSeries@Plot` 包装 `NonFiniteResult@Analyze`，并保留具体消息。Exporter 在调用 Publisher 发布 `illustration.json` 或 `illustration.svg` 前返回错误；这一保证针对插图载荷，不否认上层 CLI 可以已保留开始/失败审计记录。正常数据、投影、深度排序与发布顺序保持。两类反例针对公共 Data 的有限输入边界，不声称默认生成器或原科学实验曾产生这些顶点。

对应 [test_r06_illustrations.cpp](../../tests/integration/test_r06_illustrations.cpp) 在既有 `r06_illustrations_independent_geometry` 中增加上述两种反例和零面积正例。源码断言先确认数值 JSON 合法，再核对准确错误码/阶段/原因、发布调用为零及输入 JSON 不变；退化面仍成功发布两份载荷。新增 SVG 检查受原 `EGOMOTION_ENABLE_SVG` 条件控制，未改 CMake。

## 2. 记录目录已创建、构造尚未完成时的身份

原进程记录器在 `mkdirat` 成功后，还要打开新目录并 `fsync` 日志根；若这两步失败，`Directory` 构造未完成，内部 `path_` 与外层 `location` 均未赋值。目录创建已经发生，原诊断却只有空的 `retained record:` 尾缀，无法定位留下的记录目录。

[process_record_storage.cpp](../../src/cli/process_record_storage.cpp) 的私有检查只在本次 `mkdirat` 成功之后使用：失败时先保存 `errno`，再将实际 `root/build-cli-records/record_<id>` 路径与原操作消息一起传播。[process_record.cpp](../../src/cli/process_record.cpp) 仅在外层 `location` 非空时追加路径，避免重复空尾缀，也避免把未创建目录的失败或碰撞候选称为本次记录。

退出码仍为 6，独立 RecordId、四次碰撞规则、目录权限、创建/确认顺序及保留失败证据的行为保持。路径表示本次成功创建后已知的位置，不表示持久化已经确认；它与科学实验 RunId 及 `output/<RunId>` 不混用。持续内存耗尽仍可能阻止构造错误对象，本批没有新增无分配错误通道。

对应 [test_r06_process_records.cpp](../../tests/integration/test_r06_process_records.cpp) 在既有 `r06_process_records_lifecycle` 中复用 `fsync` 替身，以设备号和 inode 锁定日志根目录 fd，准备目录预留确认失败的回归。断言故障命中一次、exit 6、实际完整目录路径、无空尾缀、无收据/子进程/开始或结束记录、目录保留；碰撞负例增加不冒认本次记录的断言。`openat` 失败分支仅静态审阅，没有为其增加故障注入或 CMake 链接包装。

## 3. 检查范围与交付状态

本批修改三个生产文件和两个既有测试源码文件，分为两个逻辑代码提交。源码交叉审阅确认反例索引、错误包装、整数转换范围、保存 errno 的时机与故障替身命中位置；Git 文本差异检查未发现问题。**这些均不是编译或测试执行证据。** 原始 `origin/**` 树保持 `70ee155131c24a50169346d0ffbaff0c01b1c547`，旧工作树、分支和证据保留。

定向审查还覆盖轨迹/采样尺寸算术、相关 Eigen 分配保护、记录器短写、粘滞失败和返回确认路径，未确认其它需要修改的具体问题，因此没有添加猜测性门禁或任意性能上限。插图投影和深度路径也没有本批可确认的反例，未作额外修改。

DEC-R6-01～04、方案 B、固定八轮分母、末轮 ratio 与全部延期决定保持。**本批两项确定源码缺口已修复并提交；新代码未构建、未测试，R6 总体验收仍未完成。没有新增需要所有者立即介入的决定。** 恢复验证后需在新固定 SHA 上执行对应回归；完整科学实验、最终数值/性能资格、冻结和发布继续延期。
