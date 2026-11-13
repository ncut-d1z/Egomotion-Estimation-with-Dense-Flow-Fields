# 人类输入、已决定事项与无限期延期清单（TODO）

> **文档类型：项目管理/人类输入清单，不是基线签署。** 用户此前仅开放低阶球谐相关 C++ 实现与 Wolfram 验证，其它模块仍禁止开启代码实现。用户已在固定 `8eab5dd`、`445ed151`、`601c74de` 的旧版本分别报告1/1球谐CTest通过，并于最新固定 `5e5221988cf093dbfddb88f2f65f06632460f3a1` 在Ubuntu 24.04/GCC16.2上完成三个SH独立CTest **3/3 Passed、0失败**，覆盖源二阶逐系数投影。**当前卡点不是低阶SH首次编译，而是其它模块权限、公共ABI、扩大数值覆盖及DEC-005/006/008与正式批准**。代理未在用户VM执行或获得全工程授权；不写MAT、不合并/发布。见[专项授权](decisions/SH_ONLY_IMPLEMENTATION_AUTHORIZATION_2026-11-09.md)及[PROGRESS E-DOC-062](PROGRESS.md)。

## 0. 2026-11-12 所有者最新延期决定（优先级覆盖旧“建议处理时间”）

**明确指令**：**H-10、H-11、DEC-005、DEC-006、BASELINE-SIGN、RELEASE-PR 一律无限期推迟**，在其它可完成的整改项目解决之前**禁止将这些事项重新提请批准、催办或作为停工借口**。这不是“批准通过”、不是“取消最终要求”，更不是以旧局部球谐授权变相开放全工程C++、CTest、MAT写入、合并/发布。此处只改变本轮任务排序/状态；历史JSON与已有用户执行证据原样保留。

- 延期状态统一记 **`DeferredIndefinitelyByOwner`**，无默认截止日期、无自动恢复条件；除非项目所有者**以后另行明确改变优先级**，其它文档/数学/源码对照与测试计划整改先行。
- **DEC-008 不在上述延期名单内**：所有者已经按01～06逐项作出决定：01不承诺跨版本稳定ABI；02采用明确消息的`std::logic_error`方案B（C仅必要时添加）；03采用工厂/不可变方案A；04采用完整`std::forward_list<ErrorFrame>`包装链；05检查MAT结构而**不打印数值内容**；06同步保留完整错误链的RunContext/Manifest方案。DEC-008全部六个子项**设计决策已记录（`DesignDecided / ImplementationNotVerified`）**，不据此开放六项延期权限。
- 本次仍严格遵守[全工程审计规范](AUDIT.md)只做**现有文档审查、真实文本与数学合同整改**；不因为延期后还剩5项综合DAR就虚称项目不能继续推进，也不因该延期把这些需要真实软件/签署证据的目标改成“已整改”。
- **本轮可独立继续的顺序**：源MATLAB事实复核 → 数学/数据/状态/错误合同修正 → 66 REQ/TC静态追踪及候选测试补足 → DEC-008已定设计的跨文件一致性核对；真实生产C++/构建/测试/数值/正式发布进入被延期的后续阶段，不在本轮执行。

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

## 2.1 第12阶段历史干预说明（2026-11-13最新决定由§0和§2.2覆盖）

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

## 2.2 DEC-008 详细决策说明（2026-11-13：六子项设计决定已记录；全部仍待实现验证）

**当前状态：`DesignDecided / ImplementationNotVerified`。01–06已按项目所有者本轮意见形成确定设计；H-10/H-11等六项无限期延期仍未解锁，也不推导出正式软件验收。**

**DEC-008 是什么**：C++公共Result/Status、错误包装、只读MAT结构检查与运行部分状态的设计规则。它不是DEC-005数值容差、DEC-006性能预算或H-10/H-11代码/运行许可。**01–06已经按所有者指令作出设计决定**；本文是可追溯规范说明，尚未建立公共头、运行CTest或正式软件验收，不因设计决定自动解锁延期事项。

### A. 已经选定，DEC-008 不必重新选择的部分

- Debian/Ubuntu、C++17、Eigen、matio；不使用C++23 `std::expected`。
- 自定义轻量 `Result<T,Code>` / `Status<Code>`，内部用 `std::variant`，失败携带 `Failure<Code>{code, ErrorDetail}`；`ErrorStage` 是强类型枚举。
- JSONL是机器诊断格式，字符串正确转义；`message`仅供人阅读，不能当作机器错误类别。
- 已选固定 `output/<RunId>/`，只允许新的独立RunId且不得覆盖`origin/**`。失效数值、失败的Session状态不得当作成功结果提交，八次聚合不改分母。
- `MatErrorCode`、`NumericalErrorCode`、`ExperimentErrorCode` 和分析/绘图错误**各归一个原LLD定义**，对外`exit/stage`映射仅由[contracts.md §3.1](design/contracts.md)维护；不复制第二份enum或每个Runner自行决定不一致的退出码。
- 前轮实际文档修订已定下**左值引用只借用存活宿主、右值按值转移、const右值禁止借用**；本轮又发现默认构造`Result/Status`可能伪造成功，已在[contracts §3.B](design/contracts.md)修正为工厂专用、默认构造`=delete`。这两处是从内部一致性得出的**候选最低安全要求**，不是声称公共头文件已存在或DEC-008签署。

### B. 用户已确定的DEC-008逐项设计决定（软件尚未实现验证）

| 决策子项 | 解决的具体问题 | **所有者已确定的方案** | 尚需真实软件验证的内容 |
|---|---|---|---|
| **DEC-008-01：部署与二进制ABI** | private variant、Eigen、std::string的对象布局不能跨工具链/版本自动保证兼容 | **所有者已决定：不承诺跨版本稳定二进制ABI（DecisionRecorded）**。只承诺同一构建配置和工具链下的接口，升级要求C++生产者/消费者一起重新构建，不增加长期跨版本插件ABI/稳定C桥 | 未编译/未链接/未运行同构建库测试，不能据决议宣称实际ABI验收通过 |
| **DEC-008-02：错误访问`value()/error()`** | 从失败态访问value或从成功态访问error是调用方违反前置条件；不能返回默认T或伪造Failure | **已决定B：显式检查分支，抛出带固定、明确错误消息的`std::logic_error`**（标准库`std::exception`派生类型）；对内部`valueless_by_exception`单独写明异常情况。**仅在确有非抛出探测需求时添加C：`tryValue/tryError`借用指针**；不将C视为无条件强制公共API扩展 | 普通求解/MAT失败依旧`Result::failure`；需未来检查所有const/右值重载、移动后访问及错误消息，真实编译/CTest仍被延期 |
| **DEC-008-03：构造、移动与`valueless_by_exception`** | 默认构造伪成功、状态切换时异常风险、搬空后读取风险 | **已决定A：禁止默认构造，私有in-place tag构造与成功/失败工厂，不公开赋值或emplace，尽量不可变；异常构造/移动单独处理并安排故障注入**。保留移动构造/右值按值取出，但搬空源不可再作为有意义数值使用 | 不把`valueless_by_exception`等同Failure；工厂构造抛异常时没有新结果对象可返回；尚无真实注入测试 |
| **DEC-008-04：错误跨层保真与CLI退出码** | 只存根因会丢失Reader→Analyzer→Runner→CLI等中间包装节点 | **已决定用C++17`std::forward_list<ErrorFrame>`单向链表保存完整包装链**（纠正用户笔误`std::forword_list`）；当前最外层Failure是链顶，`push_front`记录直接内层，链元素顺序从最近一层至最初根因，保留每层枚举家族/码/stage/消息/已知上下文；按顺序序列化为JSONL`causes`数组 | 不得只留`source_error_code`字符串替代完整链、不得覆盖原链/伪造RunId；实际C++/JSONL/异常内存分配测试尚未执行 |
| **DEC-008-05：`inspect-mat`只打印文件数据结构** | 用户原始需求仅是MAT数据结构（变量名、类型、rank、dims等），从未要求打印矩阵元素 | **已决定只检查并打印MAT元数据；撤回助手自行提出的NumPy/pandas式数值预览、全量转储、`--format/--all/--rows/--cols/--precision`等相关需求与CLI选项**。`--variable NAME`最多过滤结构信息；此命令不要求按名解码数值。完整读取仍只属于真实算法Reader/API，与inspect-mat区分 | 仍是规划中的只读命令，不允许改MAT/运行求解器；历史审计日志保留误提及撤销痕迹，现行规范禁止数值打印 |
| **DEC-008-06：Result/Status跨层失败后的运行状态** | 部分结果与失败Result由不同层持有，但既有设计还需反映已选完整包装链 | **已决定：维持RunContext独占计数/部分文件/Manifest，失败Result只有Failure；**依DEC-008-04，**每次Runner/Writer/Session包装失败都必须保留并向外传播完整`std::forward_list<ErrorFrame>`调用链**，Manifest/JSONL按已知数据记录同一顺序；完成全部校验及Manifest发布才可Complete | Manifest自身写失败时不得声称持久化成功；同一故障产生多个错误事件时分别记录，不伪造单链的分支；真实测试未授权 |

### C. 本轮六项决策及实施前验证责任

DEC-008-01不承诺跨版本稳定ABI；02采用明确消息的std::logic_error且只有必要时才扩展try*；03采用工厂创建/不公开赋值；04采用std::forward_list记录完整调用链；05的inspect-mat严格仅打印MAT结构元数据，删除助手先前凭空增加的数值打印需求；06将完整错误链纳入RunContext与Manifest。这些**设计决定已经确认**，不再向所有者重复询问。原H-10/H-11、DEC-005/006、BASELINE-SIGN、RELEASE-PR继续无限期延期，因此本轮只修现有文档/测试合同。

未来在获准的真实代码/CTest中验证：标准异常确切消息、不能默认构造、私有工厂与移动后数据、前插链顺序、序列化不会丢环节、已知/缺失RunId、元数据检查绝不解码/打印元素、RunContext部分失败及Manifest写失败。现行66个TC编号维持不变。

### D. DEC-008-02 已决定方案B，标准库异常消息应明确

**决定**：错误调用value/error时先检查当前alternative，抛出`std::logic_error`，其消息必须指出具体方法与状态；正常业务失败一律返回`Result::failure`。`std::logic_error`派生自`std::exception`，可用`catch(const std::exception&)`接住。不得抛出只有笼统信息的`std::exception{}`或返回默认值。

~~~cpp
const T& value() const & {
    if (payload_.valueless_by_exception())
        throw std::logic_error("Result::value(): internal variant has no active alternative");
    if (!std::holds_alternative<T>(payload_))
        throw std::logic_error("Result::value(): called on a failure result");
    return std::get<0>(payload_);
}
const Failure<Code>& error() const & {
    if (payload_.valueless_by_exception())
        throw std::logic_error("Result::error(): internal variant has no active alternative");
    if (!std::holds_alternative<Failure<Code>>(payload_))
        throw std::logic_error("Result::error(): called on a success result");
    return std::get<1>(payload_);
}
~~~

`Result::value()&&`和`error()&&`也必须先检查后按值移动；`Status::error()`采用相同语义，只是没有`value()`。诊断文本为固定人类消息，不应当作JSONL业务错误码。**C仅在调用方确有无异常安全探测需求时增加**`tryValue()/tryError()`，用`std::get_if`返回借用指针，指针寿命受Result宿主约束；默认不扩大公共接口。以上是真实设计决定，尚未编译/运行测试。

### E. DEC-008-03 已决定工厂与尽量不可变方案

1. **禁止默认构造**：`Result()=delete`、`Status()=delete`，使未执行操作不能默认进入variant第0成功alternative；`T`本身的shape/有限值仍由业务代码核。
2. **工厂创建**：`success(validT)`、`failure(Failure<Code>)`调用private tagged构造，分别用`std::in_place_index<0/1>`直接建立alternative，而非“默认成功后再赋失败”。
3. **尽量不可变**：不提供公开`emplace`或copy/move assignment；可以根据T条件复制/移动构造**新对象**；`value()const&`借用只读值，`value()&&`按值取出，但已搬空源不能再当有效计算结果。禁止默认Result不代表允许成功工厂接受未初始化Eigen矩阵。
4. **内部`valueless_by_exception`**：variant切换alternative过程中异常可能使其无值；`hasValue()==false`不代表有`Failure`。构造工厂本身抛异常时尚未创建有效Result，不可伪造Failure；非预期异常应在已确定的程序边界记录而不是作为算法成功。
5. **故障注入（未来真实测试）**：用`ThrowOnCtor/ThrowOnCopy/ThrowOnMove`在指定第N次操作抛`std::runtime_error`或`std::bad_alloc`，逐项检查工厂失败不会产生伪成功、已存在对象析构安全、无双重释放、不访问不存在Failure、MoveOnly T能正确转移且跨TU自包含。测试此处未运行；用户对设计方案已决定不等于有CTest证据。

### F. DEC-008-04 已决定完整单向包装链

用户所写`std::forword_list`的正确C++17标准容器拼写是 **`std::forward_list`**（头`<forward_list>`）。**采用此容器**记录每一次错误包装，而不是只留`source_error_code`这一单个字符串。

~~~cpp
struct ErrorFrame {
    std::string domain;    // 例如MatErrorCode / NumericalErrorCode / ExperimentErrorCode
    std::string code;      // 例如InvalidShape / SolverFailed
    ErrorStage stage;      // 该层自己发生/包装时的阶段
    std::string message;
    std::string source_path;
    std::string variable;
    std::optional<std::size_t> observation_index;
    std::optional<RunId> run_id;
};
struct ErrorDetail {
    // stage/message/path/variable/observation_index/run_id 表示当前最外层
    std::forward_list<ErrorFrame> causes; // 最近一层包装 → ... → 最深根因
};
~~~

**单链规则**：最外层`Failure<Code>{code,detail}`代表当前错误；其`ErrorDetail.causes`保留所有已包装的内层帧，**首节点是直接内层、尾节点是根因**。从内层错误`inner`新建外层错误`outer`时，**先完成可能失败的快照、外层字段及新节点创建**：`ErrorFrame frame = make_frame(inner.code,inner.detail)`；`outer.causes.emplace_front(std::move(frame))`。这一步任何分配/拷贝抛异常时，**原`inner.detail.causes`不应提前移动**。新节点已构造成功后才执行`outer.causes.splice_after(outer.causes.begin(),inner.detail.causes)`，将原链节点**插在新节点之后**形成**外层失败 + 最近内层 → 更深层 → 根因**。`splice_after`须两个`forward_list`分配器相等，C++17标准规定此操作不抛异常、整链拼接复杂度对旧链长度为线性；不允许使用“先move旧链再push_front”的旧错误顺序；每一层保留原始family/code/stage/message与已知path/variable/index/RunId。没有新增包装时直接传递，不凭空增加层；绝不可覆盖、截断或倒转既有链。

**例子**：Reader解码`MatErrorCode::InvalidShape (Decode)` → Analyzer包装`AnalysisErrorCode::ShapeMismatch (Analyze)` → Runner包装`ExperimentErrorCode::DataMismatch (Simulation)`。最终Failure顶层为Experiment，链顺序为 **Analysis → Mat**；不能只剩一个`"MatErrorCode::InvalidShape"`字符串。`exit_code`只是对外大类，例如Decode=4、Solve/Analyze=5；不能代替任何一层Code。JSONL**顶层必须同时包含`error_domain`及`error_code`**（即使`causes=[]`），内层包装再序列化`causes`数组，保持各帧domain/code/stage/message/已知上下文及顺序。直接Mat/Numerical各自的同名`InvalidShape`因顶层`error_domain`不同仍可区分，不能靠message反推Code。

`std::forward_list`是单向链；**新节点在原链转移前创建**，可在`emplace_front`分配失败时保留内层原链，而`splice_after`不复制/分配已有节点，须满足分配器相容性。**后续Failure移动、RunContext保存、JSONL/Manifest写出仍可能因`std::bad_alloc`等失败，不能承诺错误链自动成功持久化**，必须避免伪造已记录事件。完整包装链并不天然是“函数调用堆栈”：没有实际包装的函数不得编造帧；一个异常并列触发多个独立错误时应保存多个事件，不把分支关系伪造成一条单链。生产实现、链复制/移动、JSONL转义/顺序、RunId合法性都待授权真实测试。

### G. DEC-008-05 已决定`inspect-mat`仅检查并打印MAT数据结构

**明确纠错**：此前助手把你“查看MAT数据结构”的要求擅自扩展为“打印矩阵元素、NumPy/pandas预览乃至全量转储”，这是**没有用户授权的虚构扩展**。现已撤回这些需求和选项。NumPy/pandas式数值显示、`--format`、`--all`、`--rows`、`--cols`、`--precision`及打印元素的测试均从**现行CLI合同和测试要求删除**，历史整改记录保留撤回原因而不作为新需求。

`inspect-mat`仅计划输出MAT文件结构元信息：真实变量名、MAT class、dtype、rank/dims、元素数量、是否complex/logical及压缩标记（仅在源元数据可确认时显示）。`--file PATH`检查全部变量；`--variable NAME`可选，仅过滤/定位该变量的**元数据**。不存在变量返回MissingVariable；重复/未知选项解析失败。不提供`--mode strict/compat`作为inspect-mat的参数，不读取或打印数组元素，不要求DataFrame/NumPy格式，也不执行估计器、修改`origin/**`或生成RunId。

**必须区分不同职责**：`MatResultReader::readNamed/readFirstCompatible`仍服务于真实算法、历史`getflowfield.m`兼容解析和独立MAT数值验证，可以根据业务需求**完整读取数值**；撤销的只是`inspect-mat`命令输出/解码数值这一未经要求的行为，不能删除Reader数值能力。只有`MatResultReader::inspect`用于本命令，只枚举合法元数据。JSONL仍用于错误诊断，结构检查成功显示只列元信息。未来测试必须确认被检查文件的原SHA不变、未调用数值解码、不同MAT类型也可报告结构、单/多变量信息正确。

### H. DEC-008-06 已获所有者同意的失败与部分运行状态

**决议`DecisionRecorded`**：单次调用失败由`Result::failure(Failure<Code>)`传播，**其`ErrorDetail.causes`必须保留DEC-008-04要求的完整`std::forward_list<ErrorFrame>`包装链**；一次实验的部分结果、计数、Manifest由`RunContext`独立拥有。Runner不能在失败`Result<RunOutcome,E>`里放一个`hasValue()==true`的“不完整成功”；只能在达到完成门槛后返回成功的`RunOutcome`。此决议已获所有者明确采纳，**不是H-10/H-11编码/测试授权**。

1. **`RunContext`记录**：`expected_repetitions`、`attempted/completed`、`valid/failed/undefined`，已核输出的MAT变量及SHA、失败发生阶段/底层cause、运行身份。独立的部分产物允许**保留供审计**，但不可当完整算法结果发布。
2. **`Complete`条件**：全部必需计算、输出变量及shape/value核验完成，MAT非覆盖发布及发布后校验完成，Manifest写入成功，才可声明`Complete`。**Manifest写失败即使MAT存在也不得宣布Complete**。
3. **`Incomplete`**：已有能独立证明身份的部分产物，但整体未完成全部预期计算/八次聚合。`Failed`：前置失败、完全没有可靠产物、或产物本身无法确认正确身份。二者都不能产生“完整实验成功值”。
4. **例子**：Comparison需要8次完整聚合，若第5次失败，`RunContext`记录expected=8/completed=4及底层故障；不按4个结果除以4生成“原八次”平均。若可持久化Manifest则标`Incomplete`，对外Runner返回带原因的`Failure<ExperimentErrorCode>`。
5. **Session原子性**：Adaptive失败帧不得部分提交已旋转主系数、周期对照、下一帧状态；终止依赖轨迹时保留上一有效帧及失败信息。Writer只负责**单个文件**，不拥有整个RunOutcome的Complete决定。
6. **写入故障和异常**：如果Manifest自身写入失败，Runner返回`OutputFailed`/底层错误，尽量经独立stderr通道发出诊断，**但不能伪称错误日志一定已持久化**、不能擅自清除已发表产物。`RunContext`与Manifest的可见状态必须与真实落盘证据对应。

**同步规则**：RunContext中每个失败事件持有当时完整的顶层Failure及其causes链，Manifest/JSONL如果实际写出必须按最外→最内顺序记录；不能将`causes`降级为单一`source_error_code`。新错误包装时只追加一层（使用push_front），不清除旧链；若Manifest写失败，其持久化状态必须如实报告，不得宣称已记录。链构造时内存分配异常也不能成为伪成功。

此内容后续将在[Runner LLD](design/experiments.md)、[contracts LLD](design/contracts.md)、[TEST_PLAN](TEST_PLAN.md)逐文档同步；只执行**已批准的文档修订**，未创建生产代码、MAT文件或运行CTest。

## 3. 回填与关闭

材料/决定记录包括来源、范围、日期、固定文件身份、适用AF/REQ/TC及是否允许执行。实际Wolfram结果与用户已执行的球谐C++/CTest按固定SHA分开登记，尚未运行的新脚本不得继承旧通过结果。AF-045/046/051/055/057包含的更广义原生数值、公共接口、业务集成、Golden或批准目标仍保持开放，不能用局部证明替代整个项目验收。

历史审批JSON与reviews保持当时事实；当前人工授权/决策待办以本表为准，**本轮全工程文档规范审计及其DAR进度以[AUDIT_PROGRESS](AUDIT_PROGRESS.md)为准**，而[AUDIT_PROCESS](AUDIT_PROCESS.md)仅是低阶球谐SH-01～06的历史理论专项整改台账；真实数学/原生测试证据仍见[PROGRESS](PROGRESS.md)。不再复制“缺论文”“全部C++未授权”“全部独立数学程序未执行”为当前原因，也不把球谐专项授权扩展到其他模块。
