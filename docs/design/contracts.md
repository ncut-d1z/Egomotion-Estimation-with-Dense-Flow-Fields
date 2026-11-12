# C++17 公共详细设计合同（跨模块 LLD）

> **跨模块合同草案 v0.8（未冻结、未建公共头文件、未编译）。** 此处集中定义候选值类型、状态、错误与 ABI 风格，以免 MAT I/O、KvD、Adaptive、实验和分析各自使用不同的 Result、形状和退出码。最终签名必须经 P0.7/P0.8 与项目所有者审批后才能成为编码基线。

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

本项目**不使用 C++23 std::expected**。项目所有者已选定：**自定义轻量 Result/Status 封装类，内部使用 C++17 `std::variant`，外部只访问 `hasValue()/value()/error()` 等明确接口**；不在调用方访问器接口直接暴露内部容器；**但私有 `std::variant` 成员仍参与 Result/Status 的对象布局、大小/对齐、移动/析构及二进制 ABI**。目前不能宣称“隐藏了 `variant` 就取得了稳定 ABI”。以下是候选精确签名示意，尚未创建真实公共头或编译。不得将原 MATLAB 非有限输出压成伪造的正常零值。

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
enum class ErrorStage {
    Parse, Read, Decode, Output, Solve, Simulation, Analyze, Plot
};
struct ErrorDetail {
    std::optional<RunId> run_id;
    ErrorStage stage;                       // 稳定枚举，JSONL 时转固定小写 token
    std::string message;                   // 供人阅读；不可用于业务条件判断
    std::string source_path;
    std::string variable;
    std::optional<std::size_t> observation_index;
    std::optional<std::string> source_error_code; // 跨层封装时保留底层错误来源
};
template<class Code>
struct Failure {
    Code code;
    ErrorDetail detail;
};
// 仅候选接口：实现包含 std::variant<T, Failure<Code>>，内部不公开。
template<class T, class Code>
class Result {
public:
    Result() = delete;                        // 禁止默认构造伪成功；详见§3.B
    Result(const Result&) = default;          // 若T不可复制，本重载自然被删除
    Result(Result&&) = default;               // 移动后源值不得再次作为有效数学量复用
    Result& operator=(const Result&) = delete; // 候选：不允许切换variant alternative
    Result& operator=(Result&&) = delete;
    static Result success(T value);
    static Result failure(Failure<Code> why);
    bool hasValue() const noexcept;
    const T& value() const &;                // 借用，仅限具有稳定生存期的左值
    T value() &&;                            // 右值按值移动取出，不返回内部引用
    const T& value() const && = delete;      // 禁止从 const 临时对象借用
    const Failure<Code>& error() const &;     // 借用，仅限左值
    Failure<Code> error() &&;                 // 右值按值移动取出
    const Failure<Code>& error() const && = delete;
private:
    explicit Result(std::in_place_index_t<0>, T valid_value);           // 直接建成功分支
    explicit Result(std::in_place_index_t<1>, Failure<Code> reason);    // 直接建失败分支
    std::variant<T, Failure<Code>> payload_;
};
// 仅候选接口：实现包含 std::variant<std::monostate, Failure<Code>>。
template<class Code>
class Status {
public:
    Status() = delete;                        // 禁止默认 monostate 伪成功
    Status(const Status&) = default;
    Status(Status&&) = default;
    Status& operator=(const Status&) = delete;
    Status& operator=(Status&&) = delete;
    static Status success();
    static Status failure(Failure<Code> why);
    bool hasValue() const noexcept;
    const Failure<Code>& error() const &;     // 左值借用
    Failure<Code> error() &&;                 // 右值拥有
    const Failure<Code>& error() const && = delete;
private:
    explicit Status(std::in_place_index_t<0>);                     // 只用于success()
    explicit Status(std::in_place_index_t<1>, Failure<Code> why);   // 只用于failure()
    std::variant<std::monostate, Failure<Code>> payload_;
};
}
~~~

### 3.B 默认构造会产生伪成功，必须采用工厂限定的有效状态（2026-11-12 真实API设计缺陷）

此前候选示意只写出 `static Result success/failure` 与 `std::variant<T,Failure<Code>> payload_`，**没有显式删除Result默认构造函数**；`Status` 的第一个variant alternative是 `std::monostate`，默认构造必定是成功分支。当 `T` 可默认构造（例如 `double`、默认构造的容器/矩阵等）时，`Result<T,E> r;` 会把没有经过任何检查或算法运算的 `T` 放进variant的**成功 alternative**；如果 `hasValue()` 按 `holds_alternative<T>` 实现，它会报告true。对Eigen默认矩阵尤其危险，因为其中的数值可能尚未初始化。`Status<E> st;` 更是会默认构造成功。**它们与“未运行/未赋有效值不得伪成功”的公共状态合同直接冲突。**

**已修订的候选可编码约束**：

- 两种类型的`默认构造函数都显式`=delete`；仅通过已明确建立成功值的 `success(T)` / `success()` 或包含 `Code+ErrorDetail` 的 `failure(Failure<E>)` 静态工厂创建；不得隐式从 `T`、`bool` 或默认值转换。
- 真实工厂必须经**private tagged in-place构造**直接建立 `variant` 的选定alternative：成功构造第0个；失败构造第1个。`Result::success(T)` 的调用方仍负责验证T的数学不变量；**工厂限制不能使任意默认T自动数值有效**。
- **禁止改变已存在对象的variant alternative的`emplace/assignment`作为普通“由失败改成功”操作**；优先采用构造新的不可变结果对象并返回。候选类型的公共copy/move assignment可删除以约束这一点，但是否保留copy construction、move-only T、异常或内存分配失败行为仍须DEC-008评审。这样可减少`valueless_by_exception`的触发路径，但不能无源码/编译/故障注入证据宣布已从类型系统完全消除。
- `Result::value() &&` / `error() &&` 必须依前述生存期修订返回**自拥有的值**；调用前必须验证 alternative。移动后源对象不得继续作为包含有效数学数据的成功结果复用；不能凭源 `hasValue()` 仍为true使用已搬离的Eigen缓冲区。
- **正确失败语义**：已构造的成功/失败对象有明确类别；创建某个variant alternative时构造本身发生异常，通常**没有新的Result对象可返回**，不能伪造 `Failure<Code>`。真正的`valueless_by_exception`若可发生，`hasValue()==false`也不代表已持有Failure。如何覆盖抛异常/无异常编译、强异常保证与意外失效，列入[TODO DEC-008 `2.2](../TODO.md)供最终接口审批。

**判别性静态负例**：`Result<double,E> r;`、`Result<FlowField,E> r;`、`Status<E> st;` 都必须在C++接口层**不可编译**；`success(validValue)` 才能得到成功，`failure({code,detail})` 才能得到失败。将来仍须编译核验模板自包含、const右值、move-only值与异常、跨TU，并使用真实数值不变量核查业务成功分支；文档修改不构成原生测试。

### 3.A C++17 私有实现不是稳定二进制 ABI（2026-11-11 静态合同更正）

候选声明中的 `std::variant<T, Failure<Code>> payload_` 与 `std::variant<std::monostate, Failure<Code>> payload_` 虽为 **private**，但只要 `Result<T,E>` 或 `Status<E>` 作为完整值类型出现在公开头/接口中，其私有成员仍影响 `sizeof`、`alignof`、编译器生成的移动/析构、模板实例和调用约定。访问器仅隐藏了**源代码层的容器操作**，**没有隐藏二进制对象布局**；跨共享库或混合编译器/标准库/编译选项时不能凭“private”保证稳定 ABI。这与本仓库目前“C++17/Eigen-only 当前无完整公共ABI”的事实一致。

**不改变所有者已选定的轻量 `std::variant` 方案**，但为 P0.8/DEC-008 列出必须显式选择的部署边界：若公共 Result/Status 按值跨 target/共享库使用，应记录所有消费 target 的 C++ 标准库/编译器/构建选项兼容边界，以及同一发行版编译/安装/升级时是否要求消费者一同重新构建；如用户未来要求独立、长期稳定的插件/动态库 ABI，应另行评审不暴露 C++ 实体布局的边界（例如不透明句柄/稳定 C 桥接），**不能擅自将已选设计改成 pImpl 并称已获批准**。模板定义若放公共头须自包含、ODR 唯一、所需 include 明确；头文件自包含编译**不等于**跨版本 ABI 合格。

**未来验证用例（复用 TC-DC-DOMAIN-001/TC-NF-OOP-001，不新增正式 TC）**：在两个独立翻译单元分别构造/取出 `Result<move-only T,E>`、`Status<E>`，验证成功/失败/异常生存期及 ODR；若实际设计有跨动态库边界，再用同一批准工具链/stdlib/配置构建生产者与消费者，核对接口调用、对象生存期与版本策略。不能使用历史SH叶子3/3 CTest证明通用合同已 ABI 验收。当前此项仅为**合同设计错误纠正**，实际公共头/链接/ABI/批准仍全部未完成，DAR-003保持未整改。

**已修正的候选API生命周期缺陷（2026-11-11，仍待 P0.8 ABI 批准）**：旧版本把 `Result::value() &&` 定义成 `T&&`，调用方若保留返回的引用而销毁临时 `Result`，即出现悬垂引用；对临时成功/失败 `Result` 或 `Status` 取 `error() const &` 亦有同类风险。现统一要求**左值返回有明确宿主生命周期的 const 引用，右值返回自拥有的移动值**（`T value() &&` / `Failure<Code> error() &&`），并禁用从 const 右值借用。此处是**更安全的设计候选修订**，没有创建或编译公共头，不能声称已通过 C++17 重载解析、move-only T、异常安全、ABI 或 ODR 测试。今后写入真实接口时，需使用固定实例检查 `auto value = std::move(result).value()` 与 `auto failure = std::move(result).error()` 的生存期，并禁止把**左值 `value() const &` 返回的借用引用**保存到宿主 Result 已销毁/移动之后。旧 `T&& value() &&` 下的 `auto&& dangling = makeResult().value()` 是悬垂风险；**新按值 `T value() &&` 会生成独立值对象，直接绑定的局部右值引用可按C++生存期扩展规则安全存活**，不应把这个修正后的场景继续误报为悬垂。跨函数返回引用、引用成员或延迟使用仍须做单独生命周期分析；移动失败不得被当作成功。调用 `value`/ `error` 仍须先检查状态，非法访问的最终异常/断言合同由 P0.8 确定。

- **Mat 错误**与**数值错误**分别归属 IO 与 solver，Runner 可以附带上下文，但不得吞掉原始错误。
- **源算法与安全行为区别**：只记录原 .m 只读源码能证明的奇异、除零、NaN 分支；原 MATLAB 实际警告、非有限传播并无 L1，**无需运行 MATLAB**。C++ 可以候选地返回明确非成功状态，但“遇错继续/停止/回滚”须按入口专属合同评审，不能误标为已验证的原版运行行为。
- **错误上下文与 RunId**：`ErrorDetail::run_id` 使用 C++17 `std::optional<RunId>`（实现须包含 `<optional>`）；仅当 RunContext 已分配独立运行身份时填充，`inspect-mat` 和参数解析可为空。RunContext 拥有唯一 RunId；各层传播错误时不得丢失或重新生成它。`observation_index` 也可为空，不与真实 0-based 第 0 个样本混淆。
- **已选定错误结构方向**：自定义轻量 Result/Status，内部 `std::variant`，统一 `Failure<Code>{code,detail}`；ABI/头文件/Exception 边界仍待实施验证。不得依赖丢失 RunId 的全局 `last_error`。
- **日志**：区分 RunId、探针 `source_sha` 参数、**原只读 .m Git blob SHA**、L2 MAT SHA-256、**独立 L3 参考生成器/数据 SHA-256**及 L4 统计来源；另记输入变量、维度、参数、seed、失败阶段、退出码、stdout/stderr。无需 MATLAB 运行或脚本捕获，原源文件读取不应改变其哈希。
- **CLI 退出码候选**：错误枚举到 CLI 退出码的**唯一候选映射**见 §3.1；命令参数、阶段与诊断输出由 [cli.md §2–3](cli.md) 维护，SRS §6.1 仅作外部接口概览。项目所有者已选定 Result/ErrorStage/JSONL 的方案，真实 API/CTest 与其它错误级别仍需 P0.8/P1.2 验收。

### 3.0 失败分支、稳定阶段与 JSONL（项目所有者已选设计）

**统一容器**：上方 §3 的 `Failure<Code>`、`Result<T,Code>`、`Status<Code>` 使用**单一轻量封装**，内部 `std::variant` 不向调用方泄漏。成功只返回 T（或无值 Status），失败必须保留 `Code` 和 `ErrorDetail`；失败绝不能返回默认值、零向量、NaN/Inf 并假装成功。实现模板需保证单一定义与正确 include；对 `valueless_by_exception` 应有明确非成功处理。**特别注意**：在此状态下 `hasValue()==false` 并不意味着变体里必有 `Failure<Code>`；不得直接调用 `std::get<Failure<Code>>` 或凭空编造来源错误。正式实现必须证明构造/赋值不会进入无值态，或有能真实保存源错误的外部异常边界，且写出单独故障注入测试；不能在当前候选示意中假称已解决。最终 ABI/头文件仍需真实编译与 P0.8 审查。

| ErrorDetail 字段 | 当前设计决定 | 责任 |
|---|---|---|
| stage | **ErrorStage 强类型枚举**：Parse、Read、Decode、Output、Solve、Simulation、Analyze、Plot；JSONL 稳定对应 `parse/read/decode/output/solve/simulation/analyze/plot` | 原始失败层设置，Runner 传播时不随意重写 |
| message | 自由文本，仅供人阅读 | 序列化成合法转义的 JSON 字符串，不用于自动判断成功 |
| run_id | `std::optional<RunId>`，未分配的 parse/inspect 可为空，产生后绝不丢失 | RunContext 提供，不产生新的 ID |
| source_path / variable | 已知时记录源路径/变量名，未知可为空 | 原 Reader/Writer 设置 |
| observation_index | `std::optional<size_t>`，未知与真实的第 0 个样本明确区分 | 逐样本失败层填写 |
| source_error_code | 跨 Mat/Numerical/Experiment 层封装时保留原始错误码的稳定名称 | Runner 不吞掉底层失败原因 |

**诊断输出采用 JSONL**：每行一个 UTF-8 JSON 对象，至少包含 `stage`、`error_code`、`exit_code`、`message`；可选 `run_id/path/variable/observation_index/source_error_code`，必须有正确的引号/反斜杠/控制字符转义，非有限浮点不得输出非法 JSON token；日志失败也不能被当作正常完成。可以使用审计过的小型序列化逻辑，**不为 JSONL 强行引入大型依赖**。JSONL 文件的创建与 CLI stderr 消息分离，均不得覆盖原始 MAT。以上仅为已选设计，未实际写/测。

### 3.0.1 第一轮错误合同未决边界（AF-040，部分文档工作）

| 分支 | 当前合同检查点 | 真正未定/待验证 |
|---|---|---|
| `Result/Status` 正常与失败态 | **默认构造已禁止，须由带标签的成功/失败工厂直接构造选定variant分支**；只有成功值或 `Failure<Code>{code,detail}`；`hasValue()` 不可将失败伪装为成功；左右值按引用/自拥有值区分；private variant仍参与 ABI 布局 | 真实 `<variant>/<optional>` include、右值重载、move-only T、模板 ODR、自包含头、跨target/跨共享库ABI策略及异常构建兼容性待编译 |
| `std::variant::valueless_by_exception()` | **不能**将无值内部状态当成功或返回默认 `T`；实现前必须给出能保留失败原因的稳定处理路径 | 使用哪一个公共错误码/异常边界仍待 P0.8 批准与故障注入，不凭文档编造映射 |
| 对错误调用 `value()` / 成功调用 `error()` | 均属违反访问前置条件；不得泄漏无效引用或转换成默认值 | 选择抛异常/断言/显式返回等精确 ABI 行为待决定与测试 |
| `ErrorDetail` 与 CLI JSONL | `run_id`、`observation_index` 可空；保持底层 code/stage；JSONL 对字符串/非有限值合法编码 | 实际跨 Mat→Experiment→CLI 嵌套错误、JSONL 转义/失败负例及多线程所有权未测 |

**状态**：明确待批准项而非重新设计 Result/Status 方案；`MatDenseArray` 的定义仍仅见 [mat_io.md](mat_io.md)，本文件只引用，不创建第二套值类型。**AF-040 未整改**。

### 3.1 错误枚举与 CLI 退出码：唯一候选映射

本表是当前 LLD 的**候选单一来源**，[cli.md](cli.md) 不另行定义冲突枚举。按错误**来源层级**区分：MAT 解码/形状错误返回 4，求解器内部的 NumericalErrorCode::InvalidShape 返回 5。

| 错误来源/枚举 | 候选 exit | 诊断阶段 |
|---|---:|---|
| 正常完成、help、version | 0 | success |
| CLI 未知/缺少/重复选项、非法实参；ExperimentErrorCode::InvalidParameters | 2 | parse |
| MatErrorCode::MissingFile、PermissionDenied、InvalidMatHeader、CorruptCompressedData、MissingVariable、AmbiguousVariable；ExperimentErrorCode::SourceUnavailable（读取输入）；**AnalysisErrorCode::MissingResult、MissingVariable**（缺分析输入文件/必需变量） | 3 | read |
| MatErrorCode::UnsupportedClassOrType、InvalidShape、SizeOverflow；**AnalysisErrorCode::ShapeMismatch**（已读取变量的维度/长度无法满足分析合同） | 4 | decode/schema |
| 全部 NumericalErrorCode（含 InvalidShape、RandomSourceExhausted、InvalidRandomSample、SampleCountOverflow、ResourceLimitExceeded）；ExperimentErrorCode::SolverFailed、DataMismatch、IncompleteRun；**AnalysisErrorCode::InvalidSegment、NonFiniteInput、UndefinedStatistic、UnsupportedExperiment**（Analyzer 运行期） | 5 | solve/simulation/analyze |
| MatErrorCode::OutputPathForbidden、OutputExists、WriteFailed、ReadbackMismatch；ExperimentErrorCode::OutputFailed | 6 | output |
| **PlotErrorCode::OutputForbidden、PlotErrorCode::WriteFailed**（可选图形发布或写出受阻） | 6 | output |
| **PlotErrorCode::InvalidSeries**（输入已分析但无法作为合法绘图序列） | 5 | plot |
| **PlotErrorCode::BackendUnavailable**（可选 plot 后端缺席） | 7 | plot |

**分析错误映射候选（AF-077）**：`AnalysisErrorCode` 的七个枚举值**仅由 [analysis_plotting.md §6](analysis_plotting.md) 定义，不在此复制第二套 enum**；本表作为唯一对外 `exit_code + ErrorStage` 候选来源。分析对缺少真实 MAT 结果或必需字段返回 `MissingResult/MissingVariable → read(3)`，已解码变量的 shape 错误返回 `ShapeMismatch → decode(4)`，分段/统计/非有限与 Analyzer 不支持模式返回 `InvalidSegment/NonFiniteInput/UndefinedStatistic/UnsupportedExperiment → analyze(5)`。**非法的 CLI `--experiment` 枚举在 Parse 阶段即返回 exit 2，不得让它穿透至 Analyzer 后误映射成 5**。跨层若 Mat 错误已发生，保留原 `MatErrorCode` 和 `ErrorDetail::source_error_code`，不可丢失 path/variable/RunId。所有上述退出码仍须 DEC-008/P0.8 签署及真实 CTest。

**优先级候选**：解析参数 → 输入读取 → 数据解码 → 输出路径安全 → 数值计算/分析 → 仅 plot 命令的后端检查。以实际首先失败的阶段报告，不跨层改写错误为成功；保留底层 code、stage、path 和可用 RunId。上表仍未获项目所有者批准，未生成真实 CLI/CTest 证据。

**可选绘图错误映射候选（AF-084）**：四个 `PlotErrorCode` 值只由 [analysis_plotting.md §2](analysis_plotting.md) 的 `IPlotExporter` 候选接口定义，**本表只负责对外 `exit_code/ErrorStage`**，禁止在 CLI 或绘图模块复制另一份退出码映射。

- `BackendUnavailable → plot(7)`：没有可选绘图后端，不是分析算法错误；`analyze` 仍可返回数值。
- `InvalidSeries → plot(5)`：导出器收到无法生成有效绘图的 `AnalysisDataset/CurveSeries`，不将其误报为“缺图形后端”；如果错误实际上起源于分析器的 MAT 解码/分段，仍应保留并传播原 `MatErrorCode/AnalysisErrorCode` 及其 `ErrorStage`，不重写为 PlotErrorCode。
- `OutputForbidden/WriteFailed → output(6)`：仅写出持久图表时发生，沿用固定 `output/<RunId>/` 的安全创建、独占、不覆盖原则；未获授权不得写 MAT 或新增输出。任何已知底层错误须附 `ErrorDetail::source_error_code`、输出路径及可用 RunId。

这个候选映射不等于后端已实现、四类错误已触发或获 DEC-008/P0.8/CTest 批准；诊断中的 `stage` 必须按实际发生层设置。对应 [测试计划 §2.17](../TEST_PLAN.md)。

### 3.2 随机源接口与失败语义候选

`IRandomSource::uniform()`、`IRandomSource::normal()` 统一返回 `Result<Scalar, NumericalErrorCode>`（见 [sampling_noise.md §3](sampling_noise.md)），不可用无状态的 `double` 返回掩盖错误：

- uniform 成功值为有限 [0,1)，normal 为有限 double；NaN/Inf 或超界 uniform 返回 InvalidRandomSample，保留调用位置及已知样本索引。
- FixedSequenceRandomSource 输入耗尽返回 RandomSourceExhausted，不重复末尾样本、不补随机数、不自动重试。
- 一旦失败，本次数值路径必须显式传播错误。**所有者已选定实验失败策略**：独立样本可记录失败并继续其他独立单位；Adaptive 失败步不提交无效主系数，无法安全恢复时终止当前连续序列；8 次聚合不以 7 次替代除数；Manifest 区分 Complete/Incomplete/Failed，并保留可追踪部分结果、不覆盖旧输出。详见 [experiments.md §6–8](experiments.md) 和 [adaptive_mfa.md §5.1](adaptive_mfa.md)；**策略已确定，具体实现及异常/状态回归测试尚未进行**，不声称原 MATLAB 异常行为已实测。
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
