# 实施进度与证据登记（PROGRESS）

> **滚动证据登记 v0.4，非正式基线批准。** 当前区分材料、静态审阅、精确符号执行、用户实际运行的 C++/CTest、代理尚未执行的下一提交及完整验收。球谐叶子及专项源码已提交，Wolfram 精确脚本已执行；**用户在 `8eab5dd`、`445ed151`、`601c74de` 三个固定提交上分别提供球谐 1/1 CTest Passed，后续 `build.sh` 详细输出改动尚未执行**。其他模块没有开启实现。
>
> E-DATA-001、E-DOC-001～053全文及当时SHA/纠偏过程保留在[固定历史PROGRESS](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/98e95e2f867dc7142fa0d5ac0f1c79f5d6429ed1/docs/PROGRESS.md)；E-DOC-054保留于§8及前轮`4fe7be6`。旧“缺论文”“所有代码/数学执行均暂缓”不是当前球谐专项状态。

## 1. 当前范围与权限

- 仓库`ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields`，分支`dev-plan-P0-P1-mat-dictionary-probe`；不合并main、不发布。
- 原`origin/egomotion_estimation_dense_flow_fields_matlab_scripts/`的45个.m、582个.mat只读；原origin树为`70ee155131c24a50169346d0ffbaff0c01b1c547`。已有探针不修改。
- 原66REQ/66TC/98P、45文件/50函数映射和27边HLD设计范围不变；新球谐子用例不新增TC，不创建全工程target。
- 用户本轮明确授权**仅球谐相关C++及Wolfram辅助验证**，先记录于`ae8c0946558312f3e30accad46aa7dd18d5e3816`，见[范围](decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md)。其他模块、共享domain/contracts、MAT/CLI/KvD/Adaptive/Runner/采样/轨迹不得顺带实施。
- **执行者与权限区别**：项目所有者已在自己的 Ubuntu 虚拟机上执行三次球谐专项目标 CMake/CTest，记录见 E-DOC-056/057/059；代理尚未获得自行运行全工程、其它模块、MAT写回读与性能测量的广泛授权。精确 Wolfram 运算已真实执行；不能再把所有 native 执行计数写成零。
- 四篇论文齐备；2015低阶基、IR96/IR98及Choi1999作为此次验证依据。原MATLAB/Octave/L1永久非必需。

## 2. 可核验子交付物

| 证据 | 实际支持的事实 | 不能推出 |
|---|---|---|
| E-DATA-001，历史matio探针 | 旧`src/mat_data_dict_probe.cpp`在Ubuntu24.04/GCC13/matio1.5.26曾解码582文件/582变量，READ_OK、失败0；参数source SHA为`48c590ba2564e4bdd293d2cbecd2eef83b291452`，FNV64有记录 | 正式Reader/Writer、独立SHA-256/逐值L2、TOCTOU或业务CTest通过 |
| E-DOC-001～053 | 历轮限定静态修订、源码追踪及纸面候选；完整历史如上 | 当前HEAD自动PASS；其他一般输入已验 |
| E-DOC-053的KvD两组六方向 | 原两种公式的不同a和非零c项候选，保留在kvd §9.2/9.3 | KvD真实逐轮L3或CTest已执行 |
| E-DOC-054（历史§8） | 三论文材料、勘误/相位静态问题、25显式公式和10候选 | 当时已执行一般证明或生产代码 |
| E-DOC-055（历史§9） | 九基/递推/正交性及一般 SO(3) 三路线已经精确验证；SH 源码当时已提交 | 不能凭 Wolfram 推断原生 double 正确或所有业务 AC 通过 |
| **E-DOC-056/057/059（用户实际运行）** | 依次于 `8eab5dd`、`445ed151`、`601c74de` 的球谐 CTest 各 1/1 Passed；后者624四元数、53532 checks及双类最大残差 | 后续新 HEAD 自动通过、全部 SO(3) 浮点行为或66TC/DEC-005完成 |

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
| P2～P9数值/实验/分析 | **SH叶子已实现、一般公式有精确验证，固定3个提交下的用户原生CTest通过**；其余模块仍是设计/候选 | 局部实现，非整体通过 | SH公共适配/额外异常/正式容差；其余模块独立授权、实现与实测 |
| P10集成 | 规划与证据模板 | 未开始验收 | 集成/MAT复核/性能/完整报告 |

M0～M5仍均未正式通过：M0缺公共接口及批准；M1缺业务MAT/构建闭环；M2缺Bias/KvD/噪声实现和实测；M3虽有SH局部实现仍缺Adaptive/Session与原生集成；M4缺完整分析数值/输出；M5缺完整平台、安全、数值、性能和发布批准。不得用数学局部完成率换算软件里程碑。

H-04历史Ubuntu24.04.4/x86_64 VirtualBox快照（8127380KiB物理内存、当时available4908696KiB、swap4194300KiB）不等于资源配额或通过标准，不据此承诺工期/性能。

## 4. 角色、批准与已选方案

MIT、IR96+IR98/Eigen-only/禁sdla、固定output、Result/ErrorStage/JSONL、独立Runner、DEC-004及0.01/0.1探索筛查沿用既定决定。当前只允许SH代码/Wolfram专门实现；**用户已亲自执行三次SH原生构建测试**，不默认开放代理全工程执行、其它模块、冻结、合并或发布。普通工程私有设计由开发者完成，不重复请求人类选型。实际引入第三方文件保留notice，不将项目MIT扩大到原PDF/MAT。

## 5. 当前卡点（不沿用已解除原因）

| 位置/AF | 已解除或已推进 | 真正剩余门槛 |
|---|---|---|
| SH / AF-045/046 | 四PDF齐备、一般数学符号证明已完成；SH叶子已在GCC16/Eigen3.4完成三次用户实测，最近一次包含624四元数及错态 | 更广泛double边界/异常、公共API/ABI适配、集成与正式批准，**不再缺首次原生编译** |
| TEST_PLAN / AF-051 | SH测试源码、独立Wolfram及第三次用户原生1/1 CTest均有真实证据；53,532 checks、624个非零四元数均已执行 | 其它业务TC/Golden/异常边界及新HEAD验证；66TC总范围不变 |
| Golden / AF-055 | 新独立有理旋转25个精确值已保存 | 原AN-G01～12整体义务不因新工件自动完成 |
| AF-043/044/047/048/050 | 保留原静态公式与候选 | 其他模块本轮无代码授权；完整逐节点L3和业务实测仍缺 |
| AF-040/041/042/049 | 保留已选设计，未新实施公共contracts/CLI/MAT/Runner | 自身授权、实现、ABI、安全/错误测试 |
| AF-054/056/060 | MAT旧字典仍存在，SH本轮符号证据单列 | 582MAT独立SHA/逐值L2、其他L3/L4及最终DEC-005 |
| AF-037/038/039/058 | 新叶子和公共设计边界明确 | 正式版本/批准、架构/公共类型工程证据 |
| AF-053/057/061 | SH材料、代码、Wolfram与用户原生执行证据均有 | 其它模块/代理运行权限、完整工程wall/CPU/RSS/DEC-006及批准 |

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

## 11. E-DOC-057：增强球谐回归测试的首次原生成功（2026-11-09）

- **实际执行者与证据等级**：项目所有者在 Ubuntu 24.04 VirtualBox 上执行 `git pl && ./build.sh`，将原 `8eab5dd` fast-forward 到完整 SHA `445ed151e5fc1faaad34b920f86363e29fb0a484`。Git HEAD 在脚本打印时与该 SHA 一致。Git 下载/更新提示共 9 个文件，新增 87 行测试。用户提供从 git 更新至构建、CTest、PASS 结束的**完整终端文本**；代理没有在该主机上运行命令或读取日志文件字节。
- **可复核环境/实际流程**：用户文本载明 `/usr/bin/g++` GCC `16.2.0`、CMake `4.3.4`、Eigen `3.4.0`、Release/C++17，4 jobs。Configure/Generate 成功；`egomotion_harmonics` 既有静态库被复用，`test_low_order_harmonics.cpp.o` 与测试可执行文件**重新编译/链接成功**。CTest：`low_order_harmonics` **1/1 Passed、0 failed**，单个测试 0.06s，CTest 总墙钟 `0.07s`，末尾 `PASS`。输出未显示编译警告，但没有另行检查所有历史警告文件/完整编译器命令或独立 shell `$?`。
- **本轮原生测试的新覆盖**：固定 HEAD `445ed151` 包含 `ba78d407` 添加的 `testAnalyticBasisAndDyadic` 与 `testMatlabSourcePhase` 调用。这次成功确实覆盖了上轮尚未执行的 `e_x/e_y/e_z/(1,1,1)/sqrt(3)` 独立解析九基锚点、2015 p.30 的 `h4` 符号/6 个二次张量恒等式、3×5 MATLAB 含 CS 相位和无效填充值对应的重建。两个不同静态里程碑必须分开：`8eab5dd` 的早期球谐测试 1/1、`445ed151` 的增补回归 1/1。
- **用户 VM 本地日志路径（仅来源路径，不是已取得文件）**：`/home/ros/Dev/Egomotion-Estimation-with-Dense-Flow-Fields/build/low-order-harmonics/logs/build-20261109T082009Z-457660.log`。源 `origin/**`、MAT 数据仍只读。CTest 的 1/1 指一个测试可执行文件，不代表 66 TC、全域 IEEE double 或最终 DEC-005/008 验收。Wolfram 精确一般 SO(3) 证明另有来源，不能代替现场数值误差统计。
- **后续状态规则**：此提交及新增测试的**编译/执行/通过**现均有用户实证；凡在 `445ed151` 后新修改的测试或球谐代码，都必须以其新 HEAD 再次实测才能标记通过。此日志不开放其它模块实现/代理自行运行全部工程测试。原 AF-045/046/051/055 仍为复合目标，不能因球谐单项 CTest 自动结案。

## 12. E-DOC-058：已证实增强回归与新数值压力测试的状态隔离（2026-11-09）

- **本轮开始固定事实**：分支 HEAD `445ed151e5fc1faaad34b920f86363e29fb0a484`；从用户粘贴的 `git pl && ./build.sh` 日志可以确认现有 `testAnalyticBasisAndDyadic`、`testMatlabSourcePhase` 与先前三路线相关用例所在的单个 `low_order_harmonics` CTest 已编译并通过 **1/1**。该实际结果单列 E-DOC-057，属于用户VM运行，不等于代理本地执行。
- **发现并修复 TC 路径漂移**：`docs/TEST_PLAN.md` 中已登记 `TC-F-SH-001/002` 的候选文件仍是不存在的 `tests/unit/test_harmonics.cpp`。实际已实现的路径与CTest是 `tests/unit/test_low_order_harmonics.cpp` → `low_order_harmonics`。在提交 `ebb352a2e707909fdb6b3ee88fbec30ea0dd97f2` 修订原两条映射和局部结果，保留其完整公共接口/正式容差/全部 TC 未验收状态，不新增 TC。
- **扩大球谐单测覆盖，但没有冒充执行**：`tests/unit/test_low_order_harmonics.cpp` 提交 `974889b0bd4f6c2d45e3369228817c7fcbdef380` 新增确定性整数四元数格点 `5^4-1=624`（全体非零），比较 TensorDirect/IR96/Choi99 的R3/R5、正交性和单个一般方向非平凡九系数场；另增加 `NotUnitDirection`、`NonFiniteInput`、`NotProperRotation`、`InvalidMethod`、`NonFiniteResult` 等错态及 MATLAB 无效填充拒绝。提交 `9d4c71abd74d233e4eb9626292e67a210ee8c030` 将矩阵/向量和标量最大绝对残差分开统计（原统一输出只能追踪矩阵比较，不能误当所有比较的误差）；提交 `8d40f1a3ce680475507ab86f14201f14155cc647` 简化 Eigen 固定输入初始化。
- **测试结果边界**：上述 624 组、错误码、有限大数溢出及新误差诊断只是**已提交的源码**，**不是已经执行的测试或数值数据**。唯一已实测过的新基线仍为 `445ed151`；下一次运行应保留新 Git SHA/工具链/CTest stdout/stderr/退出码与是否警告。如需检查最大残差，用户可在 `./build.sh` 后执行 `ctest --test-dir build/low-order-harmonics -V`（现有脚本默认只显示通过摘要），不要在尚未运行前填报数字。
- **模块/操作权限**：本批没有修改三个球谐生产 `.cpp` 或公共 `.hpp`，也没有为 MAT/CLI/KvD/Adaptive/Runner 等编写代码、触碰 `origin/**` 或改写审批历史；未执行项目 Bash、CMake/CTest、MATLAB、MAT 读写、性能试验。所有实际外部执行均明确标为用户证据。
- **阶段状态仍真实**：AF-045/046/051/055/057 相关专项推进但完整目标未满足；唯一 `97 AF＝74 已整改＋23 未整改`，文件审计 `76＝47 已审计＋29 在审`。第一轮仍有阶段门槛，没有进入前置条件未满足的第二轮全面审计。

## 13. E-DOC-059：624 四元数 / 53,532 断言真实双精度执行与构建日志改进（2026-11-09）

### 13.1 项目所有者真实运行（可核对固定版本）

- **受测版本**：`601c74de315fdeb7b913cecd2eceeadee56fb6b2`；Ubuntu 24.04 / GCC 16.2.0 / CMake 4.3.4 / Eigen 3.4.0 / Release / C++17 / 并行4；命令 `./build.sh`，并另执行 `ctest --test-dir build/low-order-harmonics -V`。用户提供这两个命令的完整可见终端输出；代理**没有执行上述构建或访问用户机器日志文件字节**。
- **真实构建**：CMake配置、生成成功，球谐静态库沿用上次产物，`tests/unit/test_low_order_harmonics.cpp` 重新编译并链接成功；可见构建输出没有编译警告。CTest常规执行 `1/1 Passed`、0失败，测试用时0.07秒、总0.08秒，脚本打印PASS。
- **真实详细输出**：第二次显式 `ctest -V` 亦 `1/1 Passed`，测试用时0.08秒、总0.11秒；测试程序打印 **`checks=53532`、`quaternion_grid=624`、`max_matrix_vector_abs_residual=8.8817841970012523e-16`、`max_scalar_abs_residual=1.0658141036401503e-14`、`regression_tolerance=9.0949470177292824e-13`**。两类观察残差均小于测试源自定容差。624代表**非零整数四元数输入数**，不是624个不重复的SO(3)旋转（例如 q 与 -q 表示相同旋转）。
- **日志路径（仅用户声明，未检验字节/SHA-256）**：`/home/ros/Dev/Egomotion-Estimation-with-Dense-Flow-Fields/build/low-order-harmonics/logs/build-20261109T084133Z-459644.log`。日志原始字节未上传本会话，shell独立 `$?` 未单列；但该次可见 CTest 输出0失败并显示脚本PASS。
- **验证限界**：53,532是当前程序调用 `require` 的次数，不是独立样本个数或正式证明覆盖率；Wolfram符号域与C++固定样本浮点域分开；4096eps是专项回归阈值，未经 DEC-005 批准；没有因此关闭AF-045/046/051、全部66TC或M0～M5验收。

### 13.2 继续审计：为何需要再次运行 `ctest -V`，本轮已如何修正

- 固定提交 `601c74de` 的 `build.sh` 已在 CTest 阶段调用 `--output-on-failure`，成功时不显示程序 stdout，因此虽有日志，该日志**没有**包含PASS检查数/误差统计。用户为得到所需详细误差另执行 `ctest --test-dir build/low-order-harmonics -V`，这确实重复运行了已通过的同一测试。用户指出的脚本便利性问题成立。
- 后续提交 `2e059fb3717619a70e1433fe65e21f153a1567fc` **仅修改根 `build.sh`**：脚本 CTest 改为一次调用 `--verbose --no-tests=error --parallel`，将成功时的 `checks/quaternion_grid/残差/tolerance` 与失败详情一并经 `tee` 写入新 Run 日志，同时保留 `set -Eeuo pipefail` 的失败传播；**不额外启动第二次 CTest**，不改编译器、C++17或球谐生产代码。
- **本轮新脚本的实际运行状态**：`601c74de` 的两次 CTest 是上个脚本版本的用户实测，新增 `--verbose` 的脚本提交后**未在用户 VM 实际执行**；不能把上述成功结果视为新 `build.sh` 已通过。
- **接续测试和审计状态**：针对现有 AF-045/046/051/053/057 推进本机编译/异常与证据归档子义务，仍有其它模块广泛测试、正式Golden/公共接口、最终容差和批准。整改台账中 97 个唯一 AF（74已整改、23未整改）及76个文件审计对象（47已审计、29在审）不因本轮用户局部成功而虚关。不会扩展 C++ 到非球谐模块。

## 14. E-DOC-060：AN-G06/07/08 归档固定候选的两路核查与独立CTest登记（2026-11-09）

- **起点事实**：远端 `544fdc68b2354bcda9286b08261b24b22dc01438`，97个唯一AF（74已整改、23未整改），76个审计路径（47已审计、29正在审计、0空白）。之前用户实测的 `601c74de` 单个SH CTest 1/1 Passed、53,532 checks、624非零四元数和双类最大绝对残差保持历史事实；当时还没有本批新CTest。
- **为什么推进现有AF-045/051/055而非建立新小目标**：`docs/golden/analytic_golden_candidates.v1.json` 已有 `AN-G06` 的原 MATLAB CS九基字面值、`AN-G07` 常数近度及20行方向（引用AN-G03）、`AN-G08` 源旋转向量对应的完整R3/R5与3×5系数；此前只有静态候选，当前球谐 C++ 的实际 CTest 尚未逐值读取这些独立的固定候选。该 JSON 继续保持 `DRAFT_UNAPPROVED_UNEXECUTED`，没有回填被测程序的输出、改写来源或降低DEC-005。
- **实施（球谐授权范围内）**：`tests/unit/test_low_order_archived_golden.cpp` 新建提交 `f6dce5362f477b79ffc9589db3b9c31acd13cc93`，后补标准头 `a0be0ead2a65363641e53a238e142cdedbbd0e33`，再固定 AN-G07 归档数字常量/来源blob与 `initializer_list` `6117fe3ec9ca30d08c278d5264a21a051765a8ce`。其三组独立断言：AN-G06 的九个原 m 排序基值，AN-G07 的20行方向近度恒为1，AN-G08 三算法在源CS布局下的输出15项、R3九项、R5全25项。纯球谐代码依赖既有 C++17/Eigen/局部 Result，**没有创建采样器、MAT Reader/Writer 或其它非球谐业务代码**。
- **CMake/脚本**：根 `CMakeLists.txt` `49a18511c5ba1e4b8790184a71798752d12116b6` 增加 `test_low_order_archived_golden` 和 `low_order_harmonics_archived_golden`；旧 `test_low_order_harmonics`、`low_order_harmonics` 的源码/名称不变。根 `build.sh` `3c8983e9355313bdecdfeedfc719e37132902a52` 只更新 Scope 文本，继续**一次** `ctest --verbose` 调用；现有两个CTest都应在用户下一次运行时执行，**本轮新程序未编译/未运行，不能标2/2 Passed**。
- **独立静态来源核对**：固定 JSON Git blob `1e6499eff3ce5b55503f8cd913b9f492629da499`：从 GitHub 文本直接逐值读取，对新测试的20行方向（60个数）、九基（9）、系数（15）、R3（9）、R5（25）进行字面比对，**各组最大差均为0**；20行的最大单位范数平方偏差约 `3.3306690738754696e-16`，低于球谐叶子输入验算的128eps。原样保留 20 行顺序，而非要求本阶段实现 i=0 原采样器。
- **独立符号证据**：在 Wolfram Language 15.0.1 中确实执行了原CS实球谐多项式、z90场作用的核验：AN-G06九基解析差9个精确0，AN-G07常数解析为1，AN-G08原m排序R3三个基函数列、R5五个基函数列的作用多项式差均精确0、原3×5系数整体场多项式差0，结果 `allExact=True`。可重放输入见 `verification/harmonics/verify_archived_sh_candidates.wl` 提交 `c476924070e492b98384931f13e52db86c264396`。**这验证符号公式与候选一致，不是新C++/MATLAB执行结果**。
- **归口与状态**：`TEST_PLAN` v0.24 将现有 TC-F-SH-001/002 关联第二个CTest并单列 `NOT_RUN`（提交 `7980007a36069b0d49c895a52b081bebfef0ba9d`）；`AUDIT` 新增两个文件路径（C++ 正在审计；实际执行的 Wolfram 脚本已审计）。本批未关闭 AF-045/046/051/055 或其他旧阶段目标；没有代理原生构建/CTest、没有修改 `origin/**`、MAT、生产球谐三路计算公式和其它业务源。下次需要用户在最新 HEAD 的GCC16环境执行 `./build.sh` 并提供两个CTest的真实完整结果及新脚本的verbose日志。

## 15. E-DOC-061：源低阶系数逐项投影的受限球谐实现与独立解析参考（2026-11-09）

- **范围/授权**：只在此前获准实施的 `egomotion::harmonics::low_order` 中扩展 `koeff_aus_near.m` 固定 `ordnung=2` 的独立投影。新增 `src/harmonics/low_order_source_projection.cpp`（提交 `842c4a82f1be3494299cb123a74bb6323fe893c9`）、对外声明/局部错误枚举 `low_order.hpp`（`3f3bed8189f74161a97922eb5b11c1ed9330b772`）；**没有新增其它模块、共享contracts、通用HarmonicFitter、MAT/Reader、Adaptive/Session、采样/Runner代码或改写 `origin/**`**。
- **公式与排序**：原 `koeff_aus_near.m` 在 `l=0,1,2` 的有效 `m=-l..l` 位置，对每个系数分别累计 `sum(near*Y)` 与 `sum(Y²)`，按源码先分别 `/N*4π` 后相除，**不是**解联立法方程或其他全局最小二乘；`low_order_source_projection.cpp` 的 nine-channel 索引/Condon–Shortley符号映射参照已证明的 canonical 九基，返回3×5源CS系数。输入要求有限N近度、N×K点且K≥3，读取前三列有限单位方向；附加流列可忽略；N=0/维度错、零基范数、非有限/溢出返回明确叶子ErrorCode，而不伪称原MATLAB在病态输入有相同warn/NaN表现。
- **可复核独立 L3 候选**：`verification/harmonics/verify_source_projection_order2.wl`（`db8a930785b6689c36dd5754112097dff10587bd`）在 Wolfram 15.0.1 上**确实执行**：7个非对称单位方向 `[(1,0,0),(0,1,0),(0,0,1),(3/5,4/5,0),(0,3/5,4/5),(4/5,0,3/5),(1/3,2/3,2/3)]`，近度 `[2,-1,4,1/2,-3,5/4,11/2]`；九系数的精确比对残差均为0、每个分母正、离散 Gram 的非对角项 `-0.2206948544207615` 非零，`allExact=True`。全部数学参考/来源状态存入 `verification/harmonics/source_projection_reference.v1.json`（`3ec35cf6a348e583dcb72d25cfe8bc992935ee87`），**不从被测C++求值填Expected、不标正式Golden批准**。
- **补充无C++运行的算术交叉核验**：从 GitHub 最新 JSON 和 C++文本读取输入/九个源CS索引，独立JS double模型按源码累积顺序求值，与 Wolfram 17位数值参考的最大绝对误差为 **2.6645352591003757e-15**；10/10 现场静态对照检查通过（源索引、累积次序、前三列、错误类型、候选七方向、CTest注册、仅SH源文件范围）。此模型不是 C++ 原生执行，也不是新批准的DEC-005容差。
- **测试**：`tests/unit/test_low_order_source_projection.cpp`（`b866f178c43233fba5de082d31bea1dbfbf1b068`，对 Eigen 子表达式 .eval 显式化 `9fdaf487691c7b95f5c287ec92bf262e1fa91d1d`），用独立精确九系数核旧3×5全部位置以及N=0、近度数不符、少于3列、方向非单位/非有限、近度Inf、单极点零平方范数、有限大数累加溢出、尾随列不应影响结果。根 CMake 注册新的独立 `low_order_harmonics_source_projection` CTest（`8e55e4615c49afb9ded839f72b43413087c6aa18`），不覆盖原已有两项；`build.sh` 范围显示更正（`fa3fc224a9c52749281f34407bb1598f137f8bce`），仍仅一轮verbose CTest。
- **必须保留的运行状态**：新增的球谐生产投影CPP和测试**尚未由用户Ubuntu编译/CTest**，此前 `601c74de` 的一个 CTest 通过不适用于新增SH程序与当前三个CTest；本轮代理亦没有运行原生GCC/CMake/CTest或实际MAT读写。没有完成动态阶数、其它模块/通用公共HarmonicFitter/正式ABI、广泛数值及最终DEC-005/008，因此原AF-045/051/055仍未整改，不依据局部数学证明关闭这些复合目标。

## 16. E-DOC-062：用户首次三个低阶球谐 CTest 全部通过（2026-11-09）

- **固定证据与执行者**：项目所有者提供其 Ubuntu 24.04 VirtualBox 运行 `./build.sh` 的完整可见终端输出，脚本内 Git HEAD=`5e5221988cf093dbfddb88f2f65f06632460f3a1`。工具链 GCC 16.2.0 (`/usr/bin/g++`)、CMake 4.3.4、Eigen 3.4.0、Release/C++17、4 jobs。项目所有者执行的命令不是代理执行；没有取得 VM 原始日志文件字节、独立 SHA-256 或单独打印的 shell `$?` 数值。
- **编译实测**：CMake configure/generate 成功；`src/harmonics/low_order_basis.cpp`、`low_order_rotation.cpp`、`low_order_source_projection.cpp`、`low_order_choi.cpp` 全部参与构建，`libegomotion_harmonics.a` 成功链接，三项测试可执行文件全部编译、链接成功；可见输出中无 compiler warning 行。之前未运行的源二阶逐系数投影 CPP 与新归档 CTest 现有真实原生构建证据。
- **真实 CTest（`--verbose` 单次执行）**：`low_order_harmonics`：53,532 checks、624 非零四元数，最大矩阵/向量残差 `8.8817841970012523e-16`、最大标量残差 `1.0658141036401503e-14`；`low_order_harmonics_archived_golden`：176 checks，最大绝对残差 `8.8817841970012523e-16`；`low_order_harmonics_source_projection`：76 checks，最大绝对残差 `1.7763568394002505e-15`。三项比较使用现有回归阈值 `9.0949470177292824e-13`，全部满足该阈值。CTest **3/3 Passed、0 failed、总墙钟0.10s**，脚本末尾PASS；标签耗时之和0.14s不是总墙钟耗时。
- **记录边界**：共 **53,784 次各测试程序内部检查**（53,532+176+76），不是53,784个独立CTest/输入样本；624个非零四元数含 q 与 -q 等重复SO(3)旋转。用户 VM 日志原始路径 `/home/ros/Dev/Egomotion-Estimation-with-Dense-Flow-Fields/build/low-order-harmonics/logs/build-20261109T104655Z-470926.log`；尚未上传该日志文件的独立SHA-256。该实测更新前轮 E-DOC-060/061 的“归档、投影 CTest NOT_RUN”，但**历史章节当时的状态不回写**，以本节作为当前事实。
- **适用结论**：AN-G06/07/08 固定数学候选、受限 `koeff_aus_near` 二阶逐系数投影均已有固定提交的 C++ 真实浮点运行佐证；数学 Wolfram 独立来源与原 MATLAB 只读源码并未因此改变。仍未完成动态阶数、业务公共HarmonicFitter/ABI、其它模块及全部66TC、DEC-005/008正式容差和批准、MAT独立L2/广泛L3/L4。**AF-045/046/051/055/060仍是复合目标，不因本模块3/3自动关闭**。

## 17. E-DOC-063：AN-G01/02/03 全714行有序几何候选与独立Wolfram签名（2026-11-09）

- **本轮基准**：远端 `2210926aaee00aa00095fe49aaaf18ee362d4bd8`，唯一97个AF（74已整改、23未整改）及82个审计文件（50已审计、32在审、0空白）。球谐 `5e522198` 的用户原生3/3与53,784次内部检查已登记在 E-DOC-062；本批**不修改SH生产C++/CTest，也未运行几何生产C++**。
- **原始数据身份和递归**：只读 `erzeugerichtungsvektoren.m` SHA `6ad7da8e3865cd2fd63ec641548ea38e0723b700`、`erzeugerichtungsvektoren_loch.m` SHA `13992600417e2ebd87193c06c1a7ec5740655735`、`richtungsvektoren_icosahedron.m` SHA `c785db126cef2149a6ffc177ecadb21e457699ca`、`findeDreiecke.m` SHA `8f6abf2cfb5daeca197314a19e382401d11df75f`。固定面顺序：完整八面体1～8、缺损原面2,3,5,6,7,8、二十面体F表1～20；递归子序A/B/C/D，每个终止面返回单位归一的三个顶点重心。输出 N×6 的后三列为零占位，不生成实际流场。
- **完整数值候选（非C++输出）**：`verification/geometry/source_direction_levels_0_2.v1.json` 固定 Git blob `85998162fb7e32ffed76cfe037e692ae101d9169` 含 i=0/1/2 的所有有序 XYZ：完整八面体8/32/128，缺损6/24/96，二十面体20/80/320，合计**714行 / 2,142个方向坐标**；记录每层shape [N,6]、原MATLAB末3列零占位约定、单位范数平方误差、固定源码blob及独立签名。脚本实现采用只读源关系的 JavaScript 重建，**不是生产采样C++、源MATLAB运行或历史MAT输出**。
- **两个独立核验**：首先与原 `docs/golden/analytic_golden_candidates.v1.json`（blob `1e6499eff3ce5b55503f8cd913b9f492629da499`）的 AN-G01/02/03 级别0 **全部34个有序方向逐坐标核对**，最大差 `1.1102230246251565e-16`；再实际运行独立 Wolfram 15.0.1 脚本 `verification/geometry/verify_source_direction_levels.wl`（blob `d4a448790bd1518c825b221faed039e53c26c5e3`），每层计算 `N,Σ_i(i*d_i),Σ_i(i²*d_i)`（i 是MATLAB 1-based 行号），共9组签名；与JS候选的**全局最大加权和绝对差 `9.313225746154785e-10`、最大相对差 `1.495521319253601e-15`**，后者因最大加权和数值量级达到百万而并非坐标误差。各级均为有限单位方向，最大归一范数平方误差不超过 `4.440892098500626e-16`。Wolfram签名可识别许多乱序，但**不等于Wolfram逐元素双实现核对全部714行**；当前不能宣称严格逐位相同。
- **审批与状态界限**：现有 AN-G01～12 `DRAFT_UNAPPROVED_UNEXECUTED` 状态保持不变；本批独立参考属于数学候选与跨语言加权签名核验，不是正式Golden、采样模块实际实现/CTest、DEC-005批准或L2历史MAT逐值。采样模块原样未实现、未获新H-10/H-11运行授权，RNG耗尽、固定uniform/normal、样本异常、i>2/资源溢出仍缺实际证据；AF-047/055/060依然「未整改」。
- **审计计数**：这次登记2份新验证工件，Wolfram可重放脚本（限定符号范围已审计）和714行候选JSON（正在审计，待实际生产逐元素对照），使文件三态变为 **84个路径＝51已审计＋33正在审计＋0未审计**；整改目标不新增虚假小目标，仍 **97个AF＝74已整改＋23未整改**。第一轮没有清零，因此不进入以其清零为前提的第二轮全面审计/整改。

## 18. E-DOC-064：完整714行独立Wolfram逐元素核验与SHA-256来源闭环（2026-11-09）

- **固定起点**：远端 `2dbc4650a7919a2eb63a73bed40f97525fdfe04e`，97个唯一整改目标（74已整改/23未整改），84个审计文件（51已审计/33正在审计/0空白）。沿用只读 `origin/**` 的八面体、缺损八面体、二十面体和 `findeDreiecke.m`，不新建采样生产C++/CMake/CTest，也不扩大先前仅SH代码的专项授权。
- **首次独立检查发现的临时错误**：最初临时 Wolfram evaluator 输入漏抄二十面体原 `F` 面表中的 `{11,3,7}`，导致 19 个面和 `19/76/304` 行；这是本轮**临时核验代码的抄录错误**，不是存档JSON或原源的错误。对原 `richtungsvektoren_icosahedron.m` 的完整20面 F 表逐行确认后修正，并增加 `Length[faces]==20` 和每一级shape检查，未将失败输出计为通过。
- **真实独立数值核验**：Wolfram Language 15.0.1 按原完整8/6/20初始面序和 A/B/C/D 递归独立生成 i=0/1/2 所有方向，与先前独立 JavaScript 生成的 `verification/geometry/source_direction_levels_0_2.v1.json` **逐行逐坐标直接相减**。输出九组行数 `8/32/128`、`6/24/96`、`20/80/320`，合计 **714 行、2142坐标**；全部 `coordinatesAbove1eMinus12=0`，`globalMaxAbsCoordinateDifference=1.1102230246251565e-16`，Wolfram自身输出方向的最大范数平方差 `2.220446049250313e-16`，`allComparedWithin1eMinus12=True`。这里使用1e-12作为本次比较的诊断阈值，**不是DEC-005正式批准的容差**。
- **完整可重放工件**：新增 `verification/geometry/verify_full_source_direction_rows.wl` Git blob `f63f6f874bc93a2c94cffa87df038638c1933956`；该脚本在实际仓库检出环境从同目录 `source_direction_levels_0_2.v1.json` 读取方向。由于云端 Wolfram 工具看不到GitHub仓库的物理文件，本轮在 evaluator 中对**同一脚本**把 `Import[referencePath,...]` 的读入动作替换为 `ImportString[同一Git文件的714行数据,...]` 后运行，已固定校验整个比较程序而不是只看加权签名。输出经固定 JSON 工件 `verification/geometry/full_ordered_row_verification.v1.json` 存档，保留各级行数/最大差和修复过程。
- **来源身份**：用 Wolfram `Hash[ByteArray[ToCharacterCode[rawFileContent,"UTF8"]],"SHA256","HexString"]` 计算真实GitHub文件UTF-8字节：参考JSON Git blob `85998162fb7e32ffed76cfe037e692ae101d9169`、SHA-256 `67fde0f3af0eaf62fde607edfa483a85d202818583a0ff40125504cce5e8a434`；全量复核WL Git blob `f63f6f874bc93a2c94cffa87df038638c1933956`、SHA-256 `6bf9a98efd6bf3d40a97a942485849fe75fcf3a3dbcc4603c92e2a76a2d11f41`；旧签名WL blob `d4a448790bd1518c825b221faed039e53c26c5e3`、SHA-256 `c025aef1fd596d839b5d39618634e9db227dc6dae8624beb92c536af3dc7d809`。不把Git blob SHA-1与SHA-256混写；原AN-G01/02/03与MAT没有被修改。
- **阶段目标与限制**：此前AF-047/055中“尚无 i=1/2 全量方向数值”与“只有顺序加权签名、无逐行独立比较”的**数学参考子缺口**现在已得到更完整证据，但 AF-047仍缺获授权生产采样器/CTest、全异常/RNG与噪声流程；AF-055仍包括其余AN-G及正式参考批准；AF-060仍需其它AC的L2/L3/L4、原MAT SHA-256/逐值和DEC-005。**不以此局部数字完成直接关闭复合AF**，也不能把两份独立源公式实现在double下的一致性宣称为全部输入/机器上的严格正确性。
- **本批计数/用途**：新增1份已独立执行的Wolfram逐行脚本、1份已核验的数值结果JSON；此前在审的714行候选JSON已完成限定数学文件审核，故审计文件三态由 `84=51已审计+33正在审计+0空白` 变为 `86=54已审计+32正在审计+0空白`。整改目标仍 `97=74已整改+23未整改`，其它业务代码执行权限不扩展，第二轮全面审计尚不满足前置条件。

## 19. E-DOC-065：AN-G12 对称 Adaptive 单步已执行的独立数学参考（2026-11-09）

- **问题定位**：现有 `docs/golden/analytic_golden_candidates.v1.json` 中 AN-G12 是 `AN-G03` 的20方向、旧系数常数近度1、理想 `t=e_x`、`omega=0` 的单步数学候选。此前仅依据对称性静态认为 `M1=M4=2I/3`、`M2=M3=0`、`rhs=(2/3,0,0,0,0,0)`，而 `near_after_update_approx=1` 与 `coefficient_after_update_approx` 没有正式独立数字。原 `oneit_adaptiv_matched_filter.m:119–129` 的 `nulldiv=1e-11` 会造成非零数值差。
- **独立 Wolfram 15.0.1 真实执行**：使用固定的归档AN-G03的20个有序方向和 `p=-e_x+(d_x)d`，直接按只读源码分别计算旧近度、M1～M4、6×6右端和解、归一化 `t=e_x`、`omega=0`，`error=MSE=0`，再以源 `1e-11` 分母重估 near、按源CS九基逐项 `koeff_aus_near` 投影。实测 `M1/M4` 与 `2I/3` 最大差 `1.1102230246251565e-16`，`M2/M3` 严格零，`rhs` 最大差同为 `1.1102230246251565e-16`；新 near 最小 `0.999999999921459`、最大 `0.99999999999`；新原CS九系数首项 `3.544907701718864`，与旧常数近度九系数的最大绝对差 `9.216760687991155e-11`。**这些数值证明更新近度并非严格等于1、C_new并非严格等于C_old**，AN-G12原注释“approx”正确。
- **实际可重放工件与身份**：`verification/adaptive/verify_AN_G12_symmetric_one_step.wl`，Git blob `04f547eb6120b6aa72a8496d9fb5efc774b64b29`；具体输入/期望/逐节点检查/来源状态在 `verification/adaptive/AN_G12_symmetric_one_step_evidence.v1.json`，用户原文件仍处于候选 `DRAFT_UNAPPROVED_UNEXECUTED`。Wolfram工具并不挂载GitHub目录，本次对**同一脚本**仅将源JSON文件读取替换为同一Git AN-G03方向的 `ImportString` 执行，保证可追溯而非伪称本地原Matlab运行。
- **证据边界**：这是一个理想对称输入下数学公式的独立浮点演算，**没有生产 Adaptive C++、没有 CMake/CTest、没有 Session/多轮/原子提交、更没有在 MATLAB/Octave 执行**；现有固定 `5e522198` 的 3/3 CTest仅涵盖SH叶子，不能给AN-G12或Adaptive业务记PASS。负/零 near 均值、一般状态、奇异M、非有限/失败提交、L2 MAT与最终DEC-005仍是AF-044/055的真实卡点。
- **统计**：在前一子批基础上新登记2份限定数学审阅工件（脚本/执行证据JSON），使本轮文件数预计变为 `88个＝56已审计＋32正在审计＋0空白`；AF唯一目标维持 `97个＝74已整改＋23未整改`。详细三态以最终 `AUDIT.md` 唯一路径表核验。

## 20. E-DOC-066：Franz/Original/Modified KvD 36次数学逐轮trace与翻号反例（2026-11-09）

- **来源与严格权限边界**：本轮从远端固定 `036a84e71db8ff5992b8eaeec12e35928439e336` 的 `AUDIT_PROCESS.md`（97AF＝74已整改+23未整改）和 `AUDIT.md`（88文件＝56已审计+32在审）继续第1轮，仅允许 **Wolfram符号/精确数学推导、验证文件和文档**。没有修改/创建 C++、CMake、CTest、Bash、原 `origin/**`、原MAT，也没有实际运行MATLAB/原生C++。四项只读原源Git blob：`franzeigen.m` `6fe556fd038b983f8d80aac5c76ee2ea11c08f38`、`zellendefinieren.m` `7132613d27135cfa00664429251da6b95553bb7c`、`norm_koenderink_iteration.m` `d48048f37b859bfb19402ca85b0f4c20e76afbfd`、`mue_norm_koenderink_iteration.m` `9ffdac1baaecac06db20bcc6849bf4cdd165a484`。
- **文档错误预警经复核撤销**：开始审阅§9.3时一度怀疑第二组六方向的 Franz 平移 `(3/2)e_x` 应为 `e_x`，独立 Wolfram 将每个模板的**三维光流内积全部计入**后确认平移响应 `(6,0,0)`、分母 `(4,4,4)`，故**原§9.3的`(3/2)e_x`是正确的**。这是本轮审阅临时心算漏计 `p_y`，没有擅自修改正确公式。第二组旋转响应零，第一组旋转 `(0,0,1/2)`。
- **旧候选上升为实际执行的完整数学证据**：新 `verification/kvd/verify_six_direction_exact_trace.wl` 从 Franz 开始，严格按原循环次序执行原/改两种KvD规则与§9.2轴向、§9.3斜向两组六方向固定输入，使用 **精确有理 `epsilon=10^-13` 与精确平方根**；逐次保存模板响应/范数、near/翻号、平移a/b/c、未归一化/归一化t、旋转aω/bω/cω、旧t赋值顺序、MSE/RMS、delta、动态grenze、下一轮布尔及最终缩放t。四条轨迹各 **6轮、共24轮**；实际Wolfram输出 `exactSixIterations=true`、near状态逐轮稳定、14/14公式自检，全部逐轮JSON固定于 `verification/kvd/six_direction_exact_trace.v1.json`。
- **新增判别 near 翻号的数学输入**：新 `verification/kvd/verify_near_sign_flip_exact_trace.wl` 使用8个切向视线与人工混合正负的有符号近度，Franz `t0=(90/101)e_x,ω0=0`；**首次求 near 原总和约 -35.9999999996004**，源按次序先同时翻 near 和t、再求a/b/c，Original/Modified的 raw t_x 分别约 `-50.49999999900495`、`-4.4999999999505`，各轮归一方向均为 `-e_x`。两轨迹各6轮，翻号向量均为 `[true,false,false,false,false,false]`；RMS约 `7.071421356346968e-12`，第2～6轮delta=0、第六轮后退出，最终t约 `-4.49999999995005 e_x`。**11/11理论自检通过**，所有12轮数据存 `verification/kvd/near_sign_flip_exact_trace.v1.json`。此场景包含非物理的负有符号近度，只用来确认算法分支，不作为严格正距离场景的物理恢复证明。
- **合计与未办**：现有**三组固定输入×Original/Modified，各6轮，共36次数学迭代**，包含正均值与**第一轮负总和翻号**、近度加权与near²非零c项、RMS/delta/grenze及停止轨迹。它们是可重放的**源公式数学模型**，不是原MATLAB、C++/CTest生产输出，也不涵盖200轮/Franz零分母/噪声/一般退化/正距离下翻号。AF-043/051/054/060及全量66TC/DEC-005仍不结案，且未扩大非球谐代码权限。
- **四份数学工件Git blob/UTF-8字节SHA-256（独立Wolfram Hash ByteArray实际计算）**：六方向WL blob `5e288035ede1775a8a16dd5abf177a378cb37a1f`、SHA256 `a949a504cbf1fc6d4ff1c647051a926e6ea34c3a4265871642f9380970a631bf`；其JSON blob `627f0a6133fcd2157fe8a0e4c5ae31e6b4e47d8b`、SHA256 `0d495edf672190841878f14487ef24d9c1128d8d443cd1cdf48caae65587f378`；八方向WL blob `fcc5ddbb39a91f7a85798826c23a0968930e1470`、SHA256 `e69c98e4d63f0631096c002451bbdb54f503678e7d2493cd63aa76ae31013537`；其JSON blob `2b2a8f5a7357f3ad971fc1c148cd3e5bb057807b`、SHA256 `22b2d91b1d8d98e5733cd7e4e27aaeba90abe6304935a30016aaf3df1a0c60ba`。Git blob SHA-1与文件字节SHA-256分列、不伪造源文件修改。
- **纯理论循环界限补充**：进一步从原 `((delta>grenze)&&(x<200)&&(delta>1e-7))||(x<6)` 的布尔条件和 `x` 自0逐轮+1严格推出：**正常返回时6≤x≤200**。人工 `RMS_k` 奇数轮1/偶数轮2（`RMS_0=0`）证明布尔控制逻辑可在第200轮停，但不证明存在真实固定光流导致该残差序列，也不等于运行了200轮C++或MATLAB。详见 [KvD LLD §9.6](design/kvd.md)；这一证明不能单独满足 AF-043 的真实200轮/异常分支验收。
- **阶段台账**：本轮仅新登记4份限定数学验证工件（2实际运行Wolfram脚本＋2审查通过的数学数值JSON），审计文件计划由88→92、已审计56→60、正在审计32→32；原97项复合AF仍74已整改/23未整改。最终统计以 `AUDIT.md` 和唯一AF表实测复读取值。
