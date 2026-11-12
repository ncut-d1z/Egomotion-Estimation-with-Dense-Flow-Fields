# 人类输入、已决定事项与无限期延期清单（TODO）

> **文档类型：项目管理/人类输入清单，不是基线签署。** 用户此前仅开放低阶球谐相关 C++ 实现与 Wolfram 验证，其它模块仍禁止开启代码实现。用户已在固定 `8eab5dd`、`445ed151`、`601c74de` 的旧版本分别报告1/1球谐CTest通过，并于最新固定 `5e5221988cf093dbfddb88f2f65f06632460f3a1` 在Ubuntu 24.04/GCC16.2上完成三个SH独立CTest **3/3 Passed、0失败**，覆盖源二阶逐系数投影。**当前卡点不是低阶SH首次编译，而是其它模块权限、公共ABI、扩大数值覆盖及DEC-005/006/008与正式批准**。代理未在用户VM执行或获得全工程授权；不写MAT、不合并/发布。见[专项授权](decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md)及[PROGRESS E-DOC-062](PROGRESS.md)。

## 0. 2026-11-12 所有者最新延期决定（优先级覆盖旧“建议处理时间”）

**明确指令**：**H-10、H-11、DEC-005、DEC-006、BASELINE-SIGN、RELEASE-PR 一律无限期推迟**，在其它可完成的整改项目解决之前**禁止将这些事项重新提请批准、催办或作为停工借口**。这不是“批准通过”、不是“取消最终要求”，更不是以旧局部球谐授权变相开放全工程C++、CTest、MAT写入、合并/发布。此处只改变本轮任务排序/状态；历史JSON与已有用户执行证据原样保留。

- 延期状态统一记 **`DeferredIndefinitelyByOwner`**，无默认截止日期、无自动恢复条件；除非项目所有者**以后另行明确改变优先级**，其它文档/数学/源码对照与测试计划整改先行。
- **DEC-008 不在上述延期名单内**：所有者已决定 **01不承诺跨版本稳定ABI**、**06采用RunContext独占计数/Manifest且失败Result仅返回Failure**；02确认**允许抛出标准库中派生自`std::exception`的异常**，具体异常/访问器仍待选择。03/04/05为待审问题。DEC-008整体状态为 **`PartiallyDecided / NotFullyApproved`**，不据此开放六项延期权限。
- 本次仍严格遵守[全工程审计规范](AUDIT.md)只做**现有文档审查、真实文本与数学合同整改**；不因为延期后还剩5项综合DAR就虚称项目不能继续推进，也不因该延期把这些需要真实软件/签署证据的目标改成“已整改”。
- **本轮可独立继续的顺序**：源MATLAB事实复核 → 数学/数据/状态/错误合同修正 → 66 REQ/TC静态追踪及候选测试补足 → DEC-008候选分析；真实生产C++/构建/测试/数值/正式发布进入被延期的后续阶段，不在本轮执行。

## 1. 人类介入事项与材料状态

| 事项简称 | 具体内容 | 为什么需要人类干预 | 建议处理时间 | 当前状态/执行边界 |
|---|---|---|---|---|
| **PAPER-SH** | 用户已提供2015 Adaptive MFA、Ivanic–Ruedenberg 1996原文、1998勘误和Choi等1999论文完整PDF；身份及页图已核，见[三路验证报告](reviews/LOW_ORDER_SH_THREE_ROUTE_VERIFICATION_2026-11-09.md)。 | 材料提交已完成，不重复要求上传。公式、相位与独立证明由开发者完成。 | **材料前置条件已满足**。 | **EvidenceAccepted**。九基、两递推与直接张量的一般SO(3)精确符号验证已实际完成；材料和数学结果不替代原生C++验收。 |
| **AUTH-CODE / H-10** | 仅`include/egomotion/harmonics/`、`src/harmonics/`及其专项测试/验证工件、文档允许实施；l=0,1,2、C++17/Eigen/double、禁止sdla。 | 用户本轮已明确批准这一局部范围，其他模块不能由此自动获权。 | **无限期推迟；必须先完成其它整改，不设置处理时间，不得主动重新催办** | **DeferredIndefinitelyByOwner（2026-11-12）**；原历史事实/局部证据保留，未授予新执行或批准权限 |
| **AUTH-RUN / H-11** | 本轮Wolfram精确符号计算已明确允许；原生编译、CMake/CTest、全工程运行、MAT写回读和性能测量没有获得新的明确授权。 | 数学核验授权和原生工程执行是不同范围，不能把前者当作后者已通过。 | **无限期推迟；必须先完成其它整改，不设置处理时间，不得主动重新催办** | **DeferredIndefinitelyByOwner（2026-11-12）**；原历史事实/局部证据保留，未授予新执行或批准权限 |
| **DEC-005-FINAL** | 基于真实C++误差、独立L3、L2逐值、几何/状态不变量及必要L4确定逐模块最终PASS阈值。 | 需实际浮点误差与数值负责人/所有者确认，精确代数等式不直接给出所有double路径的最终误差限。 | **无限期推迟；必须先完成其它整改，不设置处理时间，不得主动重新催办** | **DeferredIndefinitelyByOwner（2026-11-12）**；原历史事实/局部证据保留，未授予新执行或批准权限 |
| **DEC-006-PERF** | 根据wall/CPU/RSS、构建时间与实际规模确认平台/资源目标或豁免。 | VM容量及文献高阶速度不是本项目实测。 | **无限期推迟；必须先完成其它整改，不设置处理时间，不得主动重新催办** | **DeferredIndefinitelyByOwner（2026-11-12）**；原历史事实/局部证据保留，未授予新执行或批准权限 |
| **BASELINE-SIGN** | 对SRS/HLD/OBJECT/contracts/LLD/TEST_PLAN的具体版本、SHA、范围、依赖、开放例外及证据作正式签署。 | 文件、代码或符号证明存在不替代正式批准。 | **无限期推迟；必须先完成其它整改，不设置处理时间，不得主动重新催办** | **DeferredIndefinitelyByOwner（2026-11-12）**；原历史事实/局部证据保留，未授予新执行或批准权限 |
| **RELEASE-PR** | 决定Draft转Ready、合并、release及版本说明。 | 正式状态变更须接受实测、来源与开放风险。 | **无限期推迟；必须先完成其它整改，不设置处理时间，不得主动重新催办** | **DeferredIndefinitelyByOwner（2026-11-12）**；原历史事实/局部证据保留，未授予新执行或批准权限 |
| **PM-SCHEDULE（可选）** | 对外承诺日期时明确人员、资源和里程碑。 | 开发者不能虚构人员/工时/交付日期。 | 需要外部承诺时，非专项推进前提。 | **可选，TBD不阻塞文档审阅**。 |

## 2. 不应重复要求人类决定的事项

已选定：DEC-004当前save/历史HistoricalOnly/缺字段明确失败；项目根MIT；IR96+IR98、Eigen-only且禁止sdla；固定output/<RunId>并删除自定义根；轻量Result/Status、ErrorStage、JSONL；独立Runner配置；Adaptive失败不提交、八次聚合不改分母、Manifest三态；探索筛查0.01/0.1；MATLAB/Octave/L1永久非必需。**论文材料、球谐专项实现权限及本次Wolfram验证权限均已提供，不再重复申请。**

本轮球谐叶子采用明确合同`mu_new(d)=mu_old(A*d)`与2015九项正相位/顺序，并给出保持同一场的MATLAB系数转换。三路线已在一般非零四元数参数域下精确核对，不再笼统把“尚无一般旋转数学核验”写成当前卡点。旧源码未做相位转换的组合行为与指定合同的差异、2015第30页符号问题已记录；**不因此擅自改动其他模块、原脚本或添加业务模式**。

普通私有命名、资源清理、球谐内部测试结构由开发者在专项范围内推进。当前局部Result不代表公共contracts模块已实施或ABI已冻结。

## 2.1 第12阶段历史干预说明（本次无限期延期由§0覆盖）

**继续审计与修正现有文档，不需要本轮新增人类批准**。本次已直接查到 `many_parameter.m` 的 `dumi` 弃置、主`akkoeff`每帧提交与周期比较旋转/刷新规则，并在 [Adaptive §5.0.1](design/adaptive_mfa.md)、[Runner §8.0.1](design/experiments.md)、[TEST_PLAN §2.7.1](TEST_PLAN.md)、SRS及对象模型中实际纠偏；该源代码事实**不属于待人类猜选的算法行为**。下面列出的仅是**仍缺真实人类授权或证据签署**的事项，任何未签字的候选状态都保持`NotApproved/Deferred`。

| 编号及当前事实 | 何时必须由人类动作或明确批准 | 无需重复讨论或当前可先做的工作 |
|---|---|---|
| **H-10 / AUTH-CODE**：仅球谐低阶专项曾获代码授权，其它业务模块仍无生产实现许可 | **在首次对contracts/CLI/MAT/KvD/Adaptive/Runner等模块编写生产C++之前**，指定可实施的目标目录/模块及范围；若仍保持纯文档审计则**此项可延后** | 独立核对MATLAB源、修数学/设计文档、固定输入和待测合同无需升级授权；既有局部SH许可不外推 |
| **H-11 / AUTH-RUN**：用户自己的SH3/3有固定SHA证据，代理未获全工程原生运行许可 | **在代理首次构建、CMake/CTest或运行真实软件之前**，单独明确工具链/测试范围、运行平台和允许写入的隔离路径；**MAT写回读需要显式写入授权**，必须保证原`origin/**`只读 | 独立数学/静态逻辑演练与GitHub文档修改不等于运行授权；MATLAB/Octave/L1永久不设门禁 |
| **DEC-008 / 公共ABI与错误策略**：Result/Status、CLI、Reader、Session仍为候选，已有std::variant技术方向 | **公共接口冻结/跨库兼容、非法访问策略和错误码最终确定之前**，由所有者审核选定的二进制部署/ABI、异常与错误转换策略；不应借一次文档修订伪称批准 | 已选的轻量variant、ErrorStage、JSONL和`dumi`原源弃置无需再次决定；开发者可以先拟订精确选项和定点测试 |
| **DEC-005 / 最终数值PASS阈值** | 在取得各模块真正C++误差、独立L3/L2逐值及必要L4之后，所有者/数值负责人签署适用输入域、容差、失败率/样本分母；现阶段**不具备**有意义的最终阈值批准材料 | `atol=0.01,rtol=0.1`已仅作探索筛查；结构/状态不变量继续严格，不用L1 |
| **DEC-006 / 性能与平台预算** | 第一次端到端软件运行取得wall/CPU/RSS、构建用时、输入规模和环境后，批准实际验收阈值/豁免 | 现有Ubuntu VM容量与历史MAT探针不是性能预算；无须当前虚构数字 |
| **BASELINE-SIGN**：SRS/HLD/LLD/TEST_PLAN/版本矩阵尚未获正式签署 | 确定冻结范围、完整文件路径/版本/blob/commit、全部依赖/DEC例外/测试来源时，由所有者或授权评审人给出姓名、日期/时区及批准范围；DAR-001/005届时才能关 | 36文件固定审阅快照和66 TC编号核对有参考价值，但不能代替签字 |
| **RELEASE-PR**：未获Ready/merge/release授权 | 整套代码/安全/科学证据与批准完成后，由所有者决定Draft→Ready、合并或发布 | 维持Draft/Open，不靠本轮审计更改状态 |

**本轮需要立刻回复的人类决策：无。** 在继续当前**只读源检查＋真实文档整改**范围内，不要求用户重复上传论文、不要求重新决定已选的球谐Eigen路线、也不要求立刻签署数值容差/性能/基线；只有要扩大为生产编码、原生运行/文件写入或正式验收时才需要相应明确动作。所有既有H-10/H-11原始历史批准状态仍保留、不做追溯改写。

## 2.2 DEC-008 详细决策说明（2026-11-12：01/06已决定；02允许标准异常，其余细节待审）

**当前状态：`PartiallyDecided / NotFullyApproved`。01/06已记录，02标准异常可用但具体策略未选，03/04/05未批准。**

**DEC-008 是什么**：它是C++公共接口及失败/诊断协议的**最后可执行语义选择**，不是DEC-005的数值容差、DEC-006的性能阈值，也不是H-10/H-11的代码/运行许可。它规定模块之间交换`Result/Status`、`ErrorDetail`、MAT数据、CLI错误及会话状态时，**什么叫成功、什么叫失败、什么时候允许返回、二进制调用是否要求跨版本兼容**。代码尚未授权完成，故这里是**待所有者审阅的建议决策书**，不代表已通过，也不阻止其它文档/数学整改。

### A. 已经选定，DEC-008 不必重新选择的部分

- Debian/Ubuntu、C++17、Eigen、matio；不使用C++23 `std::expected`。
- 自定义轻量 `Result<T,Code>` / `Status<Code>`，内部用 `std::variant`，失败携带 `Failure<Code>{code, ErrorDetail}`；`ErrorStage` 是强类型枚举。
- JSONL是机器诊断格式，字符串正确转义；`message`仅供人阅读，不能当作机器错误类别。
- 已选固定 `output/<RunId>/`，只允许新的独立RunId且不得覆盖`origin/**`。失效数值、失败的Session状态不得当作成功结果提交，八次聚合不改分母。
- `MatErrorCode`、`NumericalErrorCode`、`ExperimentErrorCode` 和分析/绘图错误**各归一个原LLD定义**，对外`exit/stage`映射仅由[contracts.md `3.1](design/contracts.md)维护；不复制第二份enum或每个Runner自行决定不一致的退出码。
- 前轮实际文档修订已定下**左值引用只借用存活宿主、右值按值转移、const右值禁止借用**；本轮又发现默认构造`Result/Status`可能伪造成功，已在[contracts `3.B](design/contracts.md)修正为工厂专用、默认构造`=delete`。这两处是从内部一致性得出的**候选最低安全要求**，不是声称公共头文件已存在或DEC-008签署。

### B. 仍需所有者最终决策的关键问题与建议默认方案

| 决策子项 | 为什么必须明确 | 候选选择；**推荐仅为建议，非批准** | 不批准时保留的边界/测试 |
|---|---|---|---|
| **DEC-008-01：部署与二进制ABI** | private variant、Eigen、std::string的对象布局不能跨工具链/版本自动保证兼容 | **所有者已决定：不承诺跨版本稳定二进制ABI（DecisionRecorded）**。只承诺同一构建配置和工具链下的接口，升级要求C++生产者/消费者一起重新构建，不增加长期跨版本插件ABI/稳定C桥 | 未编译/未链接/未运行同构建库测试，不能据决议宣称实际ABI验收通过 |
| **DEC-008-02：错误访问`value()/error()`** | 取错variant alternative时无法合法返回所请求类型，不能返回默认成功值 | **已决定允许标准库派生自`std::exception`的异常**；具体A `std::get`抛`std::bad_variant_access`、B手动前置检查抛`std::logic_error`、C增`tryValue/tryError`指针探测（可叠加A/B）仍待选择；见§D | 普通输入/数值/IO错误仍返回`Result::failure`，异常只处理API错误访问/构造故障；测试尚未授权 |
| **DEC-008-03：构造、移动与`valueless_by_exception`** | 默认构造假成功、替换alternative时异常无值、移动后继续使用会读到已搬空的数据 | **仍待选择**：A工厂+禁止默认构造及公开assignment/emplace+值可读不可换态；B保留受控赋值则另定异常安全/失效态；详见§E。现行contracts是候选而非最终批准 | 真实MoveOnly/ThrowOnMove/ThrowOnCtor跨TU和fault injection未运行，不可宣称绝不valueless |
| **DEC-008-04：错误跨层保真与CLI退出码** | 错误enum、发生阶段、进程退出码、原始root cause不是同一概念；Mat/Numerical均有同名InvalidShape | ErrorStage/JSONL方向已确定；建议保留实际根因的**枚举家族+代码+原始stage**，Runner可加外层ExperimentErrorCode但不可擦掉源信息；详见§F | 根因串/causes数组/错误协议版本仍待确认，现有单一映射只能作为候选、未实测 |
| **DEC-008-05：JSONL输出及`inspect-mat`成功结果** | inspect-mat是新项目只读MAT检查命令，不是MATLAB原脚本；数据是否已解码与终端如何打印是两个独立问题 | 建议参考NumPy array2string预览及pandas DataFrame.to_string表格；不引入Python依赖，默认有界显示；`--format numpy|table`、`--all`、行列与精度参数均为未批准候选；见§G | 具体默认精度/省略/JSONL异常字段/日志失败行为仍需选择，绝不能截断原MAT数据本身 |
| **DEC-008-06：Result/Status跨层失败后的运行状态** | 单次失败的Failure与整个运行已有部分产物/计数是两种责任，不应在失败Result中返回伪成功RunOutcome | **所有者已决定：采用原建议（DecisionRecorded）**。失败Result只携`Failure<ExperimentErrorCode>`并保留原因；RunContext独占`attempted/valid/failed/undefined`、验证过的文件/Manifest；所有必要输出与回读/Manifest成功才Complete；详见§H | 仍未授权真实Runner/Writer/CTest；Manifest写失败不得声称已经持久化或Complete |

### D. DEC-008-02 标准异常许可下的三种具体访问器方案（候选，待用户选择）

**标准库异常的概念**：`std::exception` 是标准异常的基类，建议抛它的**具体派生类型**，例如 `std::bad_variant_access`（`<variant>`）和 `std::logic_error`（`<stdexcept>`）；两者都能由 `catch(const std::exception& e)` 接住。不要把`std::exception`误当成有上下文的通用错误报告。正常数学失败如ZeroNorm、MAT缺变量仍以`Result::failure(Failure<Code>{...})`返回，不把普通输入错误变成抛异常的热路径。

**方案A：直接使用C++17 `std::get` 的标准行为（推荐简单默认，未批准）**。
~~~cpp
const T& value() const & { return std::get<0>(payload_); }
T value() && { return std::get<0>(std::move(payload_)); }
const Failure<Code>& error() const & { return std::get<1>(payload_); }
Failure<Code> error() && { return std::get<1>(std::move(payload_)); }
// 错误alternative或valueless：std::get抛std::bad_variant_access。
~~~
优势：只依赖标准 `std::variant`，没有重复的手工条件分支。不足：`std::bad_variant_access::what()` 的文本由标准库实现决定，不宜用作机器可解析错误码；错用accessor和内部valueless可能都表现为相同异常类型。**不允许为了“捕获方便”把它吞掉后返回0或默认T。**

**方案B：先判断，再抛有明确含义的 `std::logic_error`（未批准）**。
~~~cpp
const T& value() const & {
    if (!std::holds_alternative<T>(payload_))
        throw std::logic_error("Result::value called without success");
    return std::get<0>(payload_);
}
const Failure<Code>& error() const & {
    if (!std::holds_alternative<Failure<Code>>(payload_))
        throw std::logic_error("Result::error called without failure");
    return std::get<1>(payload_);
}
~~~
对应右值访问器也应先检查，再按值移动。优势：错误文本指明具体违规API、可以明确区分valueless内部缺陷。不足：多一次分支，异常文本仍非协议；需要保证所有重载执行相同检查。在C++17标准库中`std::logic_error`同样派生自`std::exception`，不需要自定义异常类。

**方案C：添加无异常的探测访问器（可与A或B并行，不单独替代）**。
~~~cpp
const T* tryValue() const noexcept {
    return std::get_if<0>(&payload_);
}
const Failure<Code>* tryError() const noexcept {
    return std::get_if<1>(&payload_);
}
~~~
返回的指针为**借用**：Result销毁、移动、内部值不再稳定后不能持有使用；调用方仍需检查nullptr。它能让程序在有意分支检查时避免抛异常，但对`value()/error()`调用错误仍需选择A或B；如果批准C，需正式更新公共头并补跨TU与指针寿命测试。

**重要设计边界**：三方案都没有许可从失败Result返回T、从成功Result制造Failure。`std::bad_variant_access`和`std::logic_error`是**程序员错用接口或内部状态违约**的异常，不应取代已确定的`Failure<Code>`业务路径。用户本轮仅许可标准库异常，**尚未选择A、B或C组合**。

### E. DEC-008-03 构造工厂、尽量不可变、故障注入的实现机制

**(1) 禁止默认构造**：`Result() = delete; Status() = delete;`，因此 `Result<double,E> r;`、`Result<FlowField,E> r;`、`Status<E> st;` 应在未来编译期被拒绝。缘由是std::variant默认激活第0 alternative，原布局第0个T或monostate恰是成功态；默认成功会让未计算的矩阵/值误报有效。**注意：禁止默认构造不自动保证`success(T)`传入的T有限、符合shape，数学有效性仍归业务模块校验。**

**(2) 私有带标签工厂直接构造有效alternative**：
~~~cpp
// 仅解释机制；T/Code/Failure及头文件定义尚待冻结
Result(std::in_place_index_t<0>, T v)
    : payload_(std::in_place_index<0>, std::move(v)) {}
Result(std::in_place_index_t<1>, Failure<Code> why)
    : payload_(std::in_place_index<1>, std::move(why)) {}
static Result success(T v) {
    return Result(std::in_place_index<0>, std::move(v));
}
static Result failure(Failure<Code> why) {
    return Result(std::in_place_index<1>, std::move(why));
}
~~~
即成功/失败都在新对象诞生时建立，不走“先默认成功再赋值成失败”。`Status<Code>::success()`应显式创建第0个monostate；`failure`显式创建第1个Failure，不能靠默认`Status`表示“任务已经做成”。

**(3) 尽量不可变**：公开读取`value() const & / error() const &`只能借用对应只读结果，`Result& operator=(...) = delete`及不提供公开`emplace`避免已存在Result从成功改失败或反向切换。仍可按T能力复制/移动**构造另一个Result**；右值`value() &&`按值转移，源Result虽然可能仍在成功alternative，但其T已被移动，**禁止再次把它当数值有效数据**。若后续容器必须要求Result可赋值，应由所有者审查兼容方案，不暗改。

**(4) `valueless_by_exception`机制**：`std::variant`如果在替换alternative的过程中构造抛异常，有可能无有效alternative。此时`hasValue()==false`但`error()`也未必存在！默认禁用assignment/emplace能减少风险，但不能仅凭文档证明所有内部构造/移动/复制异常已被封闭。**正在构造的Result抛异常时根本不存在一个已构造好的“失败Result”可以返回**，禁止伪造Failure或将异常当作科学数值错误。

**(5) 故障注入的真实验证方法（未来测试，当前不执行）**：定义具有计数器的`ThrowOnCtor`、`ThrowOnCopy`、`ThrowOnMove`值对象，分别在第1/N次构造/拷贝/移动时抛`std::runtime_error`；另准备`std::bad_alloc`路径。检查测试运行后（a）没有未经验证的成功Result，（b）已存在的Result仍可安全析构且不丢已存原因，（c）不能在valueless状态访问`Failure`，（d）move-only T可通过右值按值转移，（e）实际内存/析构次数正确，（f）模板头自包含及跨翻译单元不触发ODR冲突。**不允许把模拟成功的`is_default_constructible_v`静态断言当成已实际执行的CTest。**

**待用户挑选**：A=采用以上不可默认构造/带tag工厂、默认禁止公开赋值和emplace（建议）；B=公开受控赋值/状态切换，但必须额外设计强异常保证、独立失效态与测试。用户本轮只要求解释实现机制，没有批准A/B。H-10/H-11仍无限期延期。

### C. 可复用的最小审阅材料和决策顺序

建议先确认 **01 ABI范围**、**02 非法访问/异常政策**、**03 工厂/valueless与移动约束**，才能让公共`Result/Status`合同语义自洽；再核 **04 ErrorStage/exit 映射**、**05 JSONL/inspect-mat显示**、**06 运行级失败状态**。这并不是要求立刻批准代码和测试。

**测试证据要怎样形成**（在H-10/H-11解除无限期延期**之后**）：复用`TC-DC-DOMAIN-001`、`TC-NF-OOP-001`、`TC-F-ERROR-001`、`TC-F-CLI-001`、`TC-I-RESULT-001`、`TC-NF-LOG-001`、`TC-NF-SESSION-001`。分别对不可默认构造、成功与失败工厂、move-only T、const右值、`valueless`异常注入、跨两个翻译单元/可选共享库、所有exit/stage、JSONL控制字符、无RunId、文件读写冲突及Session失败不提交进行真实编译/CTest核对。**现阶段只允许把这些写成待测条件，不执行原生工程命令。**

**当前需要人类立即选择吗？** 你要求的是“DEC-008详细说明”，不是授权实施或批准某个子项。本记录只给出A/B候选和推荐默认，**不替你做选择，DEC-008整体保留`DecisionPending / NotApproved`**。你可在其它文档整改更充分时再一次性审阅；六项无限期延期独立保留，不被DEC-008讨论解除。若以后需要你逐项决定，真正要选择的是**01跨版本二进制兼容与否、02错误访问异常政策、03是否采用不可变工厂Result、05成功输出/日志细节**；04/06的大方向已有合同，主要审核边界即可。

## 3. 回填与关闭

材料/决定记录包括来源、范围、日期、固定文件身份、适用AF/REQ/TC及是否允许执行。实际Wolfram结果与用户已执行的球谐C++/CTest按固定SHA分开登记，尚未运行的新脚本不得继承旧通过结果。AF-045/046/051/055/057包含的更广义原生数值、公共接口、业务集成、Golden或批准目标仍保持开放，不能用局部证明替代整个项目验收。

历史审批JSON与reviews保持当时事实；当前人工授权/决策待办以本表为准，**本轮全工程文档规范审计及其DAR进度以[AUDIT_PROGRESS](AUDIT_PROGRESS.md)为准**，而[AUDIT_PROCESS](AUDIT_PROCESS.md)仅是低阶球谐SH-01～06的历史理论专项整改台账；真实数学/原生测试证据仍见[PROGRESS](PROGRESS.md)。不再复制“缺论文”“全部C++未授权”“全部独立数学程序未执行”为当前原因，也不把球谐专项授权扩展到其他模块。
