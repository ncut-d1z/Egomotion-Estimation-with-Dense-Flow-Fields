# 实施进度与证据登记（PROGRESS）

> **滚动证据登记 v0.3，非正式基线批准。** 当前区分材料、静态审阅、精确符号执行、C++实现、原生运行与完整验收。**本轮球谐C++叶子及专项测试源码已提交，Wolfram三份脚本已实际执行通过；原生编译/CTest没有执行，其他模块没有开启实现。**
>
> E-DATA-001、E-DOC-001～053全文及当时SHA/纠偏过程保留在[固定历史PROGRESS](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/98e95e2f867dc7142fa0d5ac0f1c79f5d6429ed1/docs/PROGRESS.md)；E-DOC-054保留于§8及前轮`4fe7be6`。旧“缺论文”“所有代码/数学执行均暂缓”不是当前球谐专项状态。

## 1. 当前范围与权限

- 仓库`ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields`，分支`dev-plan-P0-P1-mat-dictionary-probe`；不合并main、不发布。
- 原`origin/egomotion_estimation_dense_flow_fields_matlab_scripts/`的45个.m、582个.mat只读；原origin树为`70ee155131c24a50169346d0ffbaff0c01b1c547`。已有探针不修改。
- 原66REQ/66TC/98P、45文件/50函数映射和27边HLD设计范围不变；新球谐子用例不新增TC，不创建全工程target。
- 用户本轮明确授权**仅球谐相关C++及Wolfram辅助验证**，先记录于`ae8c0946558312f3e30accad46aa7dd18d5e3816`，见[范围](decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md)。其他模块、共享domain/contracts、MAT/CLI/KvD/Adaptive/Runner/采样/轨迹不得顺带实施。
- H-11原生编译、CMake/CTest、全工程运行、MAT写回读和性能测量没有新增授权；本轮native执行数为0。Wolfram精确计算已经实际执行，不再记成未执行候选。
- 四篇论文齐备；2015低阶基、IR96/IR98及Choi1999作为此次验证依据。原MATLAB/Octave/L1永久非必需。

## 2. 可核验子交付物

| 证据 | 实际支持的事实 | 不能推出 |
|---|---|---|
| E-DATA-001，历史matio探针 | 旧`src/mat_data_dict_probe.cpp`在Ubuntu24.04/GCC13/matio1.5.26曾解码582文件/582变量，READ_OK、失败0；参数source SHA为`48c590ba2564e4bdd293d2cbecd2eef83b291452`，FNV64有记录 | 正式Reader/Writer、独立SHA-256/逐值L2、TOCTOU或业务CTest通过 |
| E-DOC-001～053 | 历轮限定静态修订、源码追踪及纸面候选；完整历史如上 | 当前HEAD自动PASS；其他一般输入已验 |
| E-DOC-053的KvD两组六方向 | 原两种公式的不同a和非零c项候选，保留在kvd §9.2/9.3 | KvD真实逐轮L3或CTest已执行 |
| E-DOC-054（历史§8） | 三论文材料、勘误/相位静态问题、25显式公式和10候选 | 当时已执行一般证明或生产代码 |
| **E-DOC-055（本轮§9）** | 九基/递推/正交性、一般SO(3)三路线和系数转换已精确执行；SH C++与独立测试源码已提交 | native编译/浮点/异常/性能、共享ABI/业务集成或完整AF验收通过 |

### 材料身份（不上传原PDF到公共仓库）

| 文献 | 页数 / 字节 | SHA-256 |
|---|---|---|
| Adaptive MFA 2015 | 35 / 2577315 | `da0f9df06e42f0d3323c43bb425e9a5f384c73ede547586fdfd0c1426fa6f0ab` |
| Ivanic–Ruedenberg 1996 | 6 / 399242 | `4ab5accb70c64a296271c156dc429ba708662ca57a8497b1048a18649dedf176` |
| 1998勘误 | 2 / 44688 | `ad096931df0eedfb74e7d8461e964ec3c7532709f0269571a4ca27a0d7b0f266` |
| Choi等1999 | 7 / 78462 | `1c29c61bcd1eb1b3152ee252f75365309b84f29df388d55fa7b4376e9df6e2db` |

材料哈希不替代MAT验证或代码测试。论文页图、语义及实际计算输入/输出见[本轮报告](reviews/LOW_ORDER_SH_THREE_ROUTE_VERIFICATION_2026-11-09.md)。

## 3. 阶段与里程碑

| 阶段 | 已有局部成果 | 当前状态 | 剩余证据 |
|---|---|---|---|
| P0资料/设计 | 清单/字典/论文、候选合同；SH一般数学证明新增 | 进行中、未正式验收 | 公共接口/例外/最终批准 |
| P1环境/CLI/MAT | 历史探针/环境事实 | 业务未验收，未开放本轮实现 | Reader/Writer/安全/原生构建测试 |
| P2～P9数值/实验/分析 | **SH叶子已实现且公式有精确验证**；其余模块仍是设计/候选 | 局部实现，非整体通过 | SH native/共享适配；其余模块独立授权、实现与实测 |
| P10集成 | 规划与证据模板 | 未开始验收 | 集成/MAT复核/性能/完整报告 |

M0～M5仍均未正式通过：M0缺公共接口及批准；M1缺业务MAT/构建闭环；M2缺Bias/KvD/噪声实现和实测；M3虽有SH局部实现仍缺Adaptive/Session与原生集成；M4缺完整分析数值/输出；M5缺完整平台、安全、数值、性能和发布批准。不得用数学局部完成率换算软件里程碑。

H-04历史Ubuntu24.04.4/x86_64 VirtualBox快照（8127380KiB物理内存、当时available4908696KiB、swap4194300KiB）不等于资源配额或通过标准，不据此承诺工期/性能。

## 4. 角色、批准与已选方案

MIT、IR96+IR98/Eigen-only/禁sdla、固定output、Result/ErrorStage/JSONL、独立Runner、DEC-004和0.01/0.1探索筛查仍按既定决定。当前授权只新增SH代码与Wolfram，不默认开放其他模块、native、冻结、合并或发布。普通工程私有设计由开发者完成，不重复请求人类选型。实际引入第三方文件保留notice，不将项目MIT扩大到原PDF/MAT。

## 5. 当前卡点（不沿用已解除原因）

| 位置/AF | 已解除或已推进 | 真正剩余门槛 |
|---|---|---|
| SH / AF-045/046 | 四PDF齐备；九基与两递推对定义的一般精确证明完成，SH叶子C++已提交 | 原生编译、double误差/异常、API公共适配和集成/批准；不是缺论文或完全未实现 |
| TEST_PLAN / AF-051 | SH测试源码与实际Wolfram证据已形成 | native用例未跑，66TC其余范围不变 |
| Golden / AF-055 | 新独立有理旋转25个精确值已保存 | 原AN-G01～12整体义务不因新工件自动完成 |
| AF-043/044/047/048/050 | 保留原静态公式与候选 | 其他模块本轮无代码授权；完整逐节点L3和业务实测仍缺 |
| AF-040/041/042/049 | 保留已选设计，未新实施公共contracts/CLI/MAT/Runner | 自身授权、实现、ABI、安全/错误测试 |
| AF-054/056/060 | MAT旧字典仍存在，SH本轮符号证据单列 | 582MAT独立SHA/逐值L2、其他L3/L4及最终DEC-005 |
| AF-037/038/039/058 | 新叶子和公共设计边界明确 | 正式版本/批准、架构/公共类型工程证据 |
| AF-053/057/061 | SH材料、代码与Wolfram权限不再缺 | 原生执行/其他阶段授权、完整运行资源/DEC-006及批准 |

0.01/0.1不是最终阈值；本模块128eps输入检查和专项测试4096eps比较也不是DEC-005科学验收签署。Wolfram精确运算证明指定表达式，不代替原生双精度实现误差分析。

## 6. 后续真实执行证据模板

```text
evidence_id:
date_time_with_timezone:
owner_or_executor:
repository / branch / commit_sha:
REQ / AC / TC / P:
source_matlab_blob_sha / input_identity / reference_sha256:
command_and_arguments:
os / compiler / dependencies / cmake_config:
expected_vs_observed:
pass / fail / skip / not_run counts:
stdout_stderr_log_path / output_path:
actual_test_exit_code:
open_issue / DEC / caveats:
reviewed_by / review_date:
```

每次只改变有证据的状态，实际精确符号执行与尚未执行的native各自登记。

## 7. 历史审计证据索引

[E-DATA-001与E-DOC-001～053固定全文](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/98e95e2f867dc7142fa0d5ac0f1c79f5d6429ed1/docs/PROGRESS.md)保留历次输入、修改/复核提交、失败检查及纠偏、原统计和权限限制。各历史reviews原文件不修改，旧审批JSON不回写。旧数字仅属当时快照，现行AF与文件状态以两台账唯一行计算。

## 8. E-DOC-054：三论文审阅与球谐合同整改（历史，2026-11-09）

### 8.1 先登记、实际修改、固定复读

输入`98e95e2f867dc7142fa0d5ac0f1c79f5d6429ed1`。在`d0618b8f02ba35ff47ae8355eca8f319cde8b7ed`先登记AF-092～095为未整改（96＝69＋27）。以下提交是实际远端变更：

| 提交 | 文档/实际成果 |
|---|---|
| `c30078bdcbb3d7b46e4b42bfee592a6a2095fe3f` | 新论文审计报告：三材料身份、原式/勘误/源码对应、印刷冲突与旋转约定反例 |
| `e37f3922e1594011162039866c699c084f80e828` | 新SH-PAPER：IR96 Table3全25项非递归公式、10组区分性候选、1999独立路线 |
| `138f45bbb5fa95ff1829e6cd346b187737fcb5e3` | 专题v0.3：材料已齐、完整P/U/V/W候选、边界短路、唯一声明保留 |
| `04a57d4e62fcb51c117ffef62a90ee0d95bb9f43` | TODO的PAPER-SH改EvidenceAccepted；当时其他授权/批准不变 |
| `ad34ed08d96f45612295aaaee392eef7a7bffb7b` | harmonics v0.8：原实基不改，论文转置和J相位区分，旧z90结论限界 |
| `f2c3aca4205035d670df625d22d4b48935cb4a2f` | LLD索引v0.10：材料/真正缺口同步，专题声明指向§5；目标文档固定复读点 |
| `b2414382a326fa89aff9f00bebe97a211c942210` | 复读后关闭AF-092～095限定文档目标，原23阶段项目不变 |
| `0eb8debf87748f0545357ae6337f2b42a41773be` | AUDIT保留原60对象并新增2篇静态工件 |

### 8.2 当时发现和限定结论

IR98 Table2负m因子减号与源码/IR96显式表存在冲突；x45纸面例给sqrt(3)/4与sqrt(3)/2。论文轴矩阵与源输入转置、CS基J相位使x90源系数重建与指定场关系不同。z90成功不足以证明一般方向。IR96全25公式与10个SH-P候选、Choi F/G与W、l2的w恒零和零权重短路已经文档化，**在该轮尚未执行**。

前轮固定复读`f2c3aca`后：96AF＝73已整改＋23未整改；62文件＝39已审计＋23正在审计。材料子卡点解除，完整目标未关。前轮无生产C++、独立数学程序或native执行。这些旧事实保留，不改写成当前新增结果。

## 9. E-DOC-055：低阶球谐三路精确验证与专项C++（本轮，2026-11-09）

### 9.1 固定输入与实际提交

输入`4fe7be6cc50a2c2b7dec1555edf9859c0497870b`：96AF（73/23），62文件（39/23）。先提交专项权限`ae8c094`，再仅在允许路径实施：

| 提交 | 实际成果 |
|---|---|
| `6d2e7ccacfb823559c3efdcbe22588b5d4b7c50b` | `include/egomotion/harmonics/low_order.hpp`：独立叶子API、明确场作用与局部Result |
| `18af25fde647e2bdda40f52541c503006fc47368` | 私有有限输入/SO(3)/排列检查 |
| `0d4982861e0f1b339ad4c9d8cd5f106b8d73cc31` | 九基直接/递推值、重建、旋转列系数与CS布局转换 |
| `1136b9423ca6441843e4c38b6012abad30679546` | 直接张量与IR96/98实递推C++ |
| `c8473d91d71a21ed06e46412b4d75f1e36d75240` | Choi1999独立复递推与W共轭转换C++ |
| `7dd6fbc1586f6a28034bcc9e45b462c0387dff92` | 独立球谐C++测试源码（未运行） |
| `42f8120b3dda435ae0b3e0fd07a848c859184b56` | 九基定义/递推/Gram/调和及dyadic验证脚本 |
| `385636d7c840c60e03b0705f463c04650b8d1b37` | 一般四元数、全SO(3)三路精确证明脚本 |
| `7eb04ede3d6f381caad5be2f80bba88b16d21b3c` | 任意系数布局同场性及旧相位反例脚本；代码/三脚本固定检查点 |
| `0985914356aebe982741c9d7b8009a52b775cc86` | 实际Wolfram结果JSON、内核版本、独立全25值参考、执行限制与诊断 |
| `9a508c63272d3369f7e69562375b468641dcd090` | 完整三路数学与代码对应报告 |
| `e34eee9`、`2be34a1`、`d6b3382`、`35d138e` | TODO权限、harmonics当前实现、AF数学子项和74文件审计范围同步 |

### 9.2 确实执行的数学验证

工具`Wolfram.WolframLanguageEvaluator`，最终内核`15.0.1 for Linux x86 (64-bit) (July 2, 2026)`。最终Module局部脚本主体按提交内容复算：

- `verify_low_order_basis.wl`：九个定义差=0、九个递推差=0、81项Gram=I9、9个Laplacian=0、9项正确dyadic展开差=0，返回allExact=true。
- `verify_low_order_rotations.wl`：任意非零实四元数给A=N/(q·q)，精确验证N正交/行列式；IR和Choi各9项D1及25项D2相对定义差=0；5个场作用多项式差=0、25项D2正交差=0；返回allExact=true。不是随机点或少量角度试验。
- `verify_low_order_layout.wl`：九项往返差=0、任意系数的场表示差=0；mu_old=2+z、Rx90的旧组合ey值1和指定正确值3得到精确确认；返回allExact=true。

IR98印刷减号变体被拒绝：Rx45目标元素正确sqrt(3)/2，减号sqrt(3)/4。2015第30页印刷xx实际等于y²、yy等于x²，正确h4符号已记录；九个式59–67本身正确。完整输入、返回和独立有理旋转全25精确值见[结果JSON](../verification/harmonics/verification_results_2026-11-09.json)，细节见[报告](reviews/LOW_ORDER_SH_THREE_ROUTE_VERIFICATION_2026-11-09.md)。

早期探索调用曾有大写全局符号提示，已换Module局部定义并复算，最终输出无该提示。公共PDF截图服务失败后使用成功的上传页图；容器raw GitHub读取DNS失败后使用GitHub连接器。失败没有冒充成功，诊断保留于JSON。

### 9.3 实现与运行边界

只新增5个球谐生产头/源文件及1个专项测试源码，未实施其他模块、共享domain/contracts、拟合器或业务适配。固定double/C++17/Eigen，无sdla；独立Result只是叶子API。原origin、MAT、探针、历史审批/reviews、AN-G JSON未修改。

测试源码规划30旋转、10方向、9基、3方法、全25独立值、非交换组合和非法输入；**没有执行这些C++用例**，native编译、CMake/CTest、性能、MAT读写和全工程运行均未进行。没有虚报退出码、warning数量或C++通过数。符号证明也不是浮点全路径或编译器形式化证明。

### 9.4 审计变化和剩余卡点

问题两态仍**96＝73已整改＋23未整改**，净变化0；本轮直接推进原AF-045/046/051/055/057，不通过新增小文档目标美化关闭率。文件三态**74＝45已审计＋29正在审计**，较前轮+12/+6/+6；6源码/测试仍待native，6授权/报告/精确验证工件已限定核对。正式状态以远端两台账唯一行复读为准。

当前球谐剩余门槛：原生编译、double数值与异常实测、未来公共类/ABI及集成、最终批准。**论文缺失、球谐代码未授权、一般旋转数学完全未验证均已不是当前原因**。其他模块代码权限仍关闭，原23项复合验收不自动完成。第一轮未清零，不进入条件性的第二轮全面审计。

## 10. E-DOC-056：Ubuntu GCC16 球谐首次原生成功与持续审查（2026-11-09）

### 10.1 实际外部运行事实（与模型在容器内执行严格区分）

项目所有者在本轮对话粘贴 Ubuntu 终端 `./build.sh` 的**完整显示输出**，记录的源代码提交为 `8eab5dd7971f9ba9263528d62e9dccdeb926d638`，分支为 `dev-plan-P0-P1-mat-dictionary-probe`。用户机器记录：Ubuntu 24.04、VirtualBox、GCC 16.2.0（`/usr/bin/g++`）、CMake 4.3.4、Eigen 3.4.0、Release/C++17、4 并行任务；执行顺序为 CMake configure/generate → 静态库 `egomotion_harmonics`、`test_low_order_harmonics` 编译链接 → CTest。输出显示所有目标 100% built，无警告行；`low_order_harmonics` **1/1 Passed，0 failed**（单测试 0.07s，CTest 总墙钟 0.11s），脚本打印 `PASS`。故在固定 SHA 上，“球谐从未原生编译/CTest”现在已不是事实。

- **日志路径（用户 VM 上，未复制到代理）**：`/home/ros/Dev/Egomotion-Estimation-with-Dense-Flow-Fields/build/low-order-harmonics/logs/build-20261109T071118Z-451487.log`，路径信息只来自用户贴出的文本；并未对原始文件做 SHA-256 或直接读取；单独的 shell `$?` 数值没有出现在用户消息中，脚本 `PASS` 表明流程完成。
- **权限边界**：用户亲自执行的一次专项构建，不意味着 H-11 全工程、其他模块代码或代理自行运行 Bash/CMake/CTest 的普遍授权；本轮代理没有运行编译器、CTest、MAT 读写、MATLAB/Octave 或其他工程测试。球谐代码实施权沿用专项授权。
- **验收边界**：1/1 CTest 是**一个测试可执行文件**，它包含多组断言，不等于 66 个 TC 或全工程已通过；Wolfram 全 SO(3) 精确符号参考和此次 C++ double 实际执行分别保留。没有批准 DEC-005 正式容差、公开/私有 ABI、错误传播与全模块集成。

### 10.2 本轮新增静态审计和球谐测试源码（后续版本必须重测）

- **AF-096**：最新 `docs/TEST_PLAN.md` 仍把已经构建的 SH 误称“未实现/未编译/CTest未运行”，故先登记；随即改为当前固定 SHA 真实结果，历史 2026-11-07 审批时点作为历史事实保留。新增根 `CMakeLists.txt`、`build.sh` 的可追溯静态审阅范围登记在 AUDIT。其它业务模块仍未获授权，未修改 `origin/**`。
- **AF-051 的现有目标继续推进**：`tests/unit/test_low_order_harmonics.cpp` 在 `ba78d407e9868aabb3cfbf16f0e379bbf8bac77d` 增加独立的 `e_x/e_y/e_z/(1,1,1)/sqrt3` 九基解析锚点，避免两个计算分支共同错误而互比通过；增加 2015论文p.30 (h_4) 正负号、全部6个对称二次项的 dyadic 恒等式，以及非零且有无效填充项的 3×5 MATLAB CS 相位源公式重建。均归既有 `TC-F-SH-001/002`，无新 TC/模块/依赖。**这些新增断言不属于用户已执行的旧提交；不能标记测试 PASS，必须在新的 HEAD 上重跑。**
- 本次没有修订低阶球谐生产公式或引入其他模块；构建文件仍只注册一项 SH CTest，没有把 matio 探针或尚不存在模块纳入业务 target。严格区分现有 (8eab5dd) 实测和其后的新增回归。
- 本轮静态核查依据：GitHub 固定远端 blob 比对、CMake/脚本目标、源/测试对应及文档状态；没有原生二次运行。AF-045/046/051/055 等整体依然因其它接口/数学/完整测试/批准义务未清零。
