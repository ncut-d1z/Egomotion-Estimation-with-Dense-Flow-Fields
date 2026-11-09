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
