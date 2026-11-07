# C++17 公共详细设计合同（跨模块 LLD）

> **预编码设计候选，未冻结、未创建头文件、未构建。** 此处集中定义候选值类型、状态、错误与 ABI 风格，以免 MAT I/O、KvD、Adaptive、实验和分析各自使用不同的 Result、形状和退出码。最终签名必须经 P0.7/P0.8 与项目所有者审批后才能成为编码基线。

## 1. 适用范围与版本边界

仅 Debian/Ubuntu、C++17、Eigen 3、matio/zlib。原始 origin/** 只读，外部结果写入单独 RunId；禁止引入 MATLAB Runtime、NumPy/.npz、Python、ROS 和 Windows 特有 API。

唯一规范引用：[SRS](../REQUIREMENTS.md) 的 REQ/AC、[HLD](../ARCHITECTURE.md) 的组件/target DAG、[对象模型](../OBJECT_MODEL.md) 的拥有关系、[测试计划](../TEST_PLAN.md) 的 Golden/TC。**其他 LLD 应引用此处的通用合同，不要分别复制并擅自冻结互相冲突的枚举。**

## 2. 核心值类型（候选 C++17 精确接口）

~~~cpp
namespace egomotion {
using Scalar = double;
using Vector3 = Eigen::Matrix<Scalar, 3, 1>;
using Matrix3 = Eigen::Matrix<Scalar, 3, 3>;
using Matrix6 = Eigen::Matrix<Scalar, 6, 6>;
using Vector6 = Eigen::Matrix<Scalar, 6, 1>;
using HarmonicMatrix = Eigen::Matrix<Scalar, 3, 5, Eigen::ColMajor>;
using FlowMatrix = Eigen::Matrix<Scalar, Eigen::Dynamic, 6, Eigen::ColMajor>;
using DirectionMatrix =
    Eigen::Matrix<Scalar, Eigen::Dynamic, 3, Eigen::ColMajor>;

struct FlowField { FlowMatrix observations; };
struct DirectionSet { DirectionMatrix directions; };
struct MotionEstimate { Vector3 translation; Vector3 rotation; };
struct HarmonicCoefficients { HarmonicMatrix values; };
struct RunId { std::string text; };
}
~~~

| 类型 | 不变量 | 资源/生命周期 |
|---|---|---|
| FlowField | N×6，左三列单位视线，右三列三维切向光流；N 必须大于 0（适用数值求解时） | 自拥有的 Eigen 内存；不借用 MAT 缓冲区 |
| MatDenseArray（MAT I/O 自拥有值对象） | 详见 [mat_io.md §2–2.1](mat_io.md)：当前候选只支持二维、密集、非 complex/logical double；含列主序数据和 rank/dims；**本合同不重复定义该类** | 属于 `egomotion_matio` 的 io 值对象，供具体 Reader/Writer 适配层使用；**不得让 experiments/analysis 的 IResultReader/IResultWriter 接口反向依赖 io 具体类型**；接口跨层以 domain/contract 值对象传递 |
| DirectionSet | N×3，按原 MATLAB 生成顺序；零方向禁止用于物理模型 | 值对象 |
| MotionEstimate | t、ω 均为三维并有明确数值有效性状态 | 求解器按值返回；不能将纯数值看作成功 |
| HarmonicCoefficients | 3×5，只有 1+3+5 个有效项；其他位置不参与运算 | Session 独占其可写主状态 |
| RunId | 只用于可审计输出身份；拒绝绝对路径、分隔符和回溯组件 | 一个运行上下文拥有，不可用于 origin 写入 |

具体类型是否保留公开数据成员应在 API 审查时明确；上述是**示意性的候选签名**而不是已实现编译目标。涉及 C++ 编码时，遵守 AGENTS.md 对语义化 using、100 行软上限和注释的要求。

## 3. 统一结果/错误语义

本项目**不使用 C++23 std::expected**。下方代码展示一种一致的 C++17 `std::variant` 候选：Result<Value, ErrorCode> 的值/Failure 二选一，无值的操作采用 Status<ErrorCode>；它尚未成为已批准 API，P0.8 可审定为封装类或其他带标签联合体。不得将原 MATLAB 非有限输出压成伪造的正常零值。

~~~cpp
namespace egomotion {
enum class MatErrorCode {
    MissingFile, PermissionDenied, InvalidMatHeader, CorruptCompressedData,
    MissingVariable, AmbiguousVariable, UnsupportedClassOrType,
    InvalidShape, SizeOverflow, OutputPathForbidden, OutputExists,
    WriteFailed, ReadbackMismatch
};
enum class NumericalErrorCode {
    EmptyInput, InvalidShape, NonFiniteInput, ZeroNorm,
    InvalidDirection, InvalidNearness, SingularSystem,
    IllConditionedSystem, NonFiniteResult, UndefinedAngle,
    InvalidSubdivisionLevel, SampleCountOverflow, ResourceLimitExceeded,
    RandomSourceExhausted, InvalidRandomSample
};
enum class ExperimentErrorCode {
    InvalidParameters, SourceUnavailable, DataMismatch,
    SolverFailed, OutputFailed, IncompleteRun
};
struct ErrorDetail {
    std::optional<RunId> run_id; // 可无 RunId 的 inspect / parse 仍可报告错误
    std::string stage;
    std::string message;
    std::string source_path;
    std::string variable;
    std::optional<std::size_t> observation_index; // 无编号与 0-based 首个样本可区分
};
// C++17 可评审实现候选；实际容器/ABI 待 P0.8 冻结。
template<class ErrorCode>
struct Failure {
    ErrorCode code;
    ErrorDetail detail;
};
template<class Value, class ErrorCode>
using Result = std::variant<Value, Failure<ErrorCode>>;
template<class ErrorCode>
using Status = std::variant<std::monostate, Failure<ErrorCode>>;
}
~~~

- **Mat 错误**与**数值错误**分别归属 IO 与 solver，Runner 可以附带上下文，但不得吞掉原始错误。
- **源算法与安全行为区别**：只记录原 .m 只读源码能证明的奇异、除零、NaN 分支；原 MATLAB 实际警告、非有限传播并无 L1，**无需运行 MATLAB**。C++ 可以候选地返回明确非成功状态，但“遇错继续/停止/回滚”须按入口专属合同评审，不能误标为已验证的原版运行行为。
- **错误上下文与 RunId**：`ErrorDetail::run_id` 候选为 C++17 `std::optional<RunId>`（实现须包含 `<optional>`）；仅当 RunContext 已分配独立运行身份时填充，`inspect-mat` 和参数解析可为空。RunContext 拥有唯一 RunId；各层传播错误时不得丢失或重新生成它。`observation_index` 也可为空，不与真实 0-based 第 0 个样本混淆。
- **错误结构待冻结**：当前 `Result<Value, ErrorCode>` 为候选类型；P0.8 须明确统一承载枚举及 ErrorDetail 的方式（如 Result 的失败分支包装 `Failure<Code>{code, detail}`），不得依赖可丢失 RunId 的全局 `last_error`。
- **日志**：区分 RunId、探针 `source_sha` 参数、**原只读 .m Git blob SHA**、L2 MAT SHA-256、**独立 L3 参考生成器/数据 SHA-256**及 L4 统计来源；另记输入变量、维度、参数、seed、失败阶段、退出码、stdout/stderr。无需 MATLAB 运行或脚本捕获，原源文件读取不应改变其哈希。
- **CLI 退出码候选**：错误枚举到 CLI 退出码的**唯一候选映射**见 §3.1；命令参数、阶段与诊断输出由 [cli.md §2–3](cli.md) 维护，SRS §6.1 仅作外部接口概览。签名/错误级别仍须 P0.8/P1.2 审批。

### 3.0 失败分支与稳定诊断协议（C++17 候选，待 P0.8 冻结）

建议以单一带类型的失败记录实现所有 Result/Status 的错误传播，而不是仅返回枚举或使用全局 last_error：

上面 §3 的同一份候选代码包含 Failure<ErrorCode>、Result<Value,ErrorCode>、Status<ErrorCode>；**不在不同 LLD 或示例中第二次定义同名模板/别名**。实现时须具备 C++17 的 <variant>、<optional> 等头文件以及完整的 RunId/ErrorDetail 类型，且通过真正的编译与单元测试验证后才能称为可编码 ABI。

**这是一种候选实现方式，不是已创建的头文件或正式 ABI**。Success 仅对应 Result 的 Value 或 Status 的 std::monostate；Failure 必有非成功 ErrorCode 和 ErrorDetail。不得用默认 Value、空对象、NaN/Inf 或布尔 false 代替 Failure；variant 出现 valueless_by_exception 时也**不是成功**，应按内部异常路径报告。是否采用上述别名、封装类或显式 tagged union 仍需 P0.8 决定，但以下不丢失错误上下文的语义是设计要求。

| ErrorDetail 字段 | 稳定合同候选 | 设置/传播责任 |
|---|---|---|
| stage | 失败时必须为可识别阶段：parse、read、decode/schema、output、solve、simulation、analyze、plot；不依赖中文/英文自由文本识别 | 错误发生层填写；contracts.md §3.1 的组合类别 `solve/simulation/analyze` 在实际发生处选择一个具体阶段 |
| message | 供人阅读的诊断文本，可自由描述，但**不可被自动化当成稳定枚举** | 原层保留；Runner 可以添加上下文而不能覆盖 code |
| source_path / variable | 已知时记录源文件路径及原始变量名；未分配时可为空 | Reader/Writer 写入，调用者不得随意伪造值 |
| run_id | std::optional<RunId>：未创建 RunContext 的 parse/inspect 可为空；分配后传播不丢失 | RunContext 在跨模块边界补足缺失身份，**不得生成第二个 RunId** |
| observation_index | std::optional<size_t>：空与真正 0-based 第 0 个样本不同 | 产生逐样本错误的模块填入，不允许以 0 冒充未知 |

**唯一入口映射**仍为下面 §3.1；本节不另定义 CLI 退出码。通用阶段值采用现有 string 容器，是否提升为强类型 enum、如何序列化/转义、已有错误的嵌套 cause/运行时异常处理均待 DEC-008/P0.8 确认。MAT I/O 的 MatErrorCode、采样的 NumericalErrorCode 和 Runner 的 ExperimentErrorCode 不互相改写；调用方封装时应保留原错误代码和 ErrorDetail 于内部诊断证据。

### 3.1 错误枚举与 CLI 退出码：唯一候选映射

本表是当前 LLD 的**候选单一来源**，[cli.md](cli.md) 不另行定义冲突枚举。按错误**来源层级**区分：MAT 解码/形状错误返回 4，求解器内部的 NumericalErrorCode::InvalidShape 返回 5。

| 错误来源/枚举 | 候选 exit | 诊断阶段 |
|---|---:|---|
| 正常完成、help、version | 0 | success |
| CLI 未知/缺少/重复选项、非法实参；ExperimentErrorCode::InvalidParameters | 2 | parse |
| MatErrorCode::MissingFile、PermissionDenied、InvalidMatHeader、CorruptCompressedData、MissingVariable、AmbiguousVariable；ExperimentErrorCode::SourceUnavailable（读取输入） | 3 | read |
| MatErrorCode::UnsupportedClassOrType、InvalidShape、SizeOverflow | 4 | decode/schema |
| 全部 NumericalErrorCode（含 InvalidShape、RandomSourceExhausted、InvalidRandomSample、SampleCountOverflow、ResourceLimitExceeded）；ExperimentErrorCode::SolverFailed、DataMismatch、IncompleteRun | 5 | solve/simulation/analyze |
| MatErrorCode::OutputPathForbidden、OutputExists、WriteFailed、ReadbackMismatch；ExperimentErrorCode::OutputFailed | 6 | output |
| 可选 plot 后端不存在（CLI capability，非 MatErrorCode） | 7 | plot |

**优先级候选**：解析参数 → 输入读取 → 数据解码 → 输出路径安全 → 数值计算/分析 → 仅 plot 命令的后端检查。以实际首先失败的阶段报告，不跨层改写错误为成功；保留底层 code、stage、path 和可用 RunId。上表仍未获项目所有者批准，未生成真实 CLI/CTest 证据。

### 3.2 随机源接口与失败语义候选

`IRandomSource::uniform()`、`IRandomSource::normal()` 统一返回 `Result<Scalar, NumericalErrorCode>`（见 [sampling_noise.md §3](sampling_noise.md)），不可用无状态的 `double` 返回掩盖错误：

- uniform 成功值为有限 [0,1)，normal 为有限 double；NaN/Inf 或超界 uniform 返回 InvalidRandomSample，保留调用位置及已知样本索引。
- FixedSequenceRandomSource 输入耗尽返回 RandomSourceExhausted，不重复末尾样本、不补随机数、不自动重试。
- 一旦失败，本次数值路径立即传播错误；“继续/回滚/保留部分结果”是实验专属的开放决策，不能假装复现 MATLAB 的失败行为。
- 原 Bias 的两次 seeding 由 RunContext 在相应阶段创建/重置随机源；须保持 motion 前的两组 uniform、reset 后的 N 次 uniform 与 3N 次 normal 的顺序。`nonpropfehler(...,0)` 仍须消费 3N 次 normal。
- 每 RunId 独占 RNG，FixedSequence 测试覆盖数值、调用计数、提前耗尽、越界/非有限值；结果性能和 ABI 仍待批准。

### 3.3 `ExperimentErrorCode::DataMismatch` 的候选触发边界

仅用于 **Runner/集成层确认数据合同不一致**：例如本次预定输出与五个入口的变量名/shape/有效样本数/作者预存统计字段之间存在可证明的错配，或者独立 L2/L3 检查的来源/单位/样本集合不相容且无法进行有效比较。应记录 expected/observed、来源 SHA、RunId 和发现阶段（simulation 或 analyze），映射为 §3.1 的 exit 5。**不用于** MAT 原始解码的 `MatErrorCode::InvalidShape/UnsupportedClassOrType`、数值求解失败 `SolverFailed`、输出文件失败 `OutputFailed`，也**不将不同 RNG 导致的逐样本差异直接认定 DataMismatch**。参考 [实验 Runner LLD](experiments.md) §4/§7。

## 4. 所有权、const、线程和错误传播

| 对象 | 所有者 | 复制/移动 | 并发保证 |
|---|---|---|---|
| MatFileHandle、MatVariableHandle | MAT I/O 内部 | 禁止复制，可移动，析构释放 | 单句柄共享使用不予保证 |
| FlowField、DirectionSet、系数值 | Solver/Runner 自有值 | 按值复制/移动，必要时显式避免大拷贝 | 只读可并行；写需独占 |
| RNG 具体实现 | 每次 RunContext | 不共享可变 RNG | 单个实例并发调用不予保证 |
| AdaptiveMfaStepSolver | 无跨帧状态 | 可共享只读参数 | 同步并发须后续实测 |
| AdaptiveMfaSession | 每条轨迹单一实例 | 默认禁止复制，移动/重置策略待冻结 | 单 Session 并发不承诺 |
| RunMatWriter | 每个 RunId 独占 | 不允许同一目标并发写 | 不覆盖已有结果 |
| Analyzer | 分析命令 | 只消费自拥有数值 | 不得触发求解或修改源 MAT |

算法调用按值返回的结果不能包含已释放句柄或局部 Eigen::Map 引用。异常/失败传播应包含对应 REQ/TC 和运行证据，不依赖全局 last_error。

## 5. L2/L3/L4 参考数据与可验收合同模板

本合同遵循 [不执行 MATLAB 的长期政策](../golden/NO_MATLAB_EXECUTION_POLICY.md)：以 L2 MAT、L3 独立逐节点参考、L4 统计进行限定数值验收；不再需要原 MATLAB 运行的 L1 或采集脚本 blob SHA。完整 C++ 构建/真实 TC、独立参考数据来源与 DEC-005 容差仍待批准。

| 字段 | 要求 | 当前状态 |
|---|---|---|
| source_file / source_blob_sha | 原始 .m 函数的 Git blob SHA，不能用探针 source_sha 参数替代 | 文件存在，个别 SHA 已读取 |
| probe_source_sha / verified_archive_git_sha | 探针运行参数与经独立核实的归档 Git tree/commit 分开保存，未验证二者等同则不能合并 | 已记录前者、未独立核实两者关系 |
| reference_generator_source_sha / reference_sha256 | 独立 L3 参考生成算法/推导文档的源码身份与参考文件 SHA-256 分别保管；同时保存原只读 .m 的 blob SHA。**无需 MATLAB 运行/采集脚本** | 只有解析候选，正式独立参考未全覆盖 |
| fixed_input_identity | 确定性 N×6、near/coeff、参数及 RNG 固定序列，并明确 L2/L3/L4 来源等级 | 尚未建立全部独立 L3 参考 |
| expected_intermediates | 依**原只读源码静态公式、赋值顺序**，由独立 L3 解析或非被测实现生成的逐轮 `near/t/ω`、`M1–M4`、`b/z/error_mse` 等参考；含输入、节点索引及来源 SHA，**不是 MATLAB 运行输出** | 一般/退化输入的独立 L3 参考尚未完整生成或实测 |
| target_method / unit | 确切数学合同、尺寸、rad/deg | 目前在模块 LLD 拟定 |
| atol/rtol + scope | 每个节点阈值、有效数值条件、失败合同 | **DEC-005 未批准** |
| testcase / ctest / runlog | TC ID、真实文件、CTest、SHA、执行记录 | 目前 TC 仅标识符 |

**冻结门槛**：全部使用本合同的模块必须对齐类型/错误/所有权；如确有需要增加 enum 值或改变数据布局，须更新相关 REQ-DC、TC、HLD 和模块 LLD。当前文档的具体声明不构成已经运行的接口或正式编译通过证据。


## 6. 公共合同到 TC/AC 的验收对应

| 候选公共合同 | 需求/测试编号 | 首个证明场景 | 当前状态 |
|---|---|---|---|
| FlowField/DirectionSet N×6/N×3、HarmonicCoefficients 3×5 | REQ-DC-DOMAIN-001 / TC-DC-DOMAIN-001 | 合法/非法形状、元素顺序、值拥有 | 未编码、未运行 |
| MAT RAII、读写接口和自拥有缓冲区 | REQ-DC-IO-001 / TC-DC-IO-001 | move-only 释放恰好一次、MatFileHandle 销毁后数据安全 | 未编码、未运行 |
| 显式 RNG 与独立运行上下文 | REQ-DC-RNG-001 / TC-DC-RNG-001 | 两组固定 RNG 序列按调用顺序重放 | 未编码、未运行 |
| KvD 原始/修正独立求解入口 | REQ-DC-KVD-001 / TC-DC-KVD-001 | 同一流场下两规则不串扰 | 未编码、未运行 |
| Adaptive StepSolver / Session 状态隔离 | REQ-DC-ADAPT-001 / TC-DC-ADAPT-001 | 单步脱离 Session、两个 Session 交替执行 | 未编码、未运行 |
| 对象所有权与层间依赖 | REQ-NF-OOP-001 / TC-NF-OOP-001 | 不向求解器暴露 matio、组件 target 无反向依赖 | 未编码、未运行 |
| 结果目录与历史文件保护 | REQ-NF-SAFETY-001 / TC-NF-SAFETY-001 | 重复 RunId/路径逃逸/失败写入时源 hash 不变 | 未编码、未运行 |
| Session 非并发使用边界 | REQ-NF-SESSION-001 / TC-NF-SESSION-001 | 两个 RunContext 互相隔离；同一 Session 的线程安全未承诺 | 未编码、未运行 |

完整的测试文件/CTest 候选位置由 [TEST_PLAN.md §3](../TEST_PLAN.md) 统一管理；不能因为本表列有 TC 就认定测试已经实现、执行或通过。
