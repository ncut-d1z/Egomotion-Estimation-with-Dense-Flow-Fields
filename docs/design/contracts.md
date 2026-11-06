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
| DirectionSet | N×3，按原 MATLAB 生成顺序；零方向禁止用于物理模型 | 值对象 |
| MotionEstimate | t、ω 均为三维并有明确数值有效性状态 | 求解器按值返回；不能将纯数值看作成功 |
| HarmonicCoefficients | 3×5，只有 1+3+5 个有效项；其他位置不参与运算 | Session 独占其可写主状态 |
| RunId | 只用于可审计输出身份；拒绝绝对路径、分隔符和回溯组件 | 一个运行上下文拥有，不可用于 origin 写入 |

具体类型是否保留公开数据成员应在 API 审查时明确；上述是**示意性的候选签名**而不是已实现编译目标。涉及 C++ 编码时，遵守 AGENTS.md 对语义化 using、100 行软上限和注释的要求。

## 3. 统一结果/错误语义

本项目**不使用 C++23 std::expected**。拟采用项目内定义的 Result<Value, Error>（值或错误二选一），无值的操作采用 Status<Error>；实际实现可用 C++17 std::variant / 显式 tagged union 等，并在 P0.8 冻结。不得将原 MATLAB 非有限输出压成伪造的正常零值。

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
    IllConditionedSystem, NonFiniteResult, UndefinedAngle
};
enum class ExperimentErrorCode {
    InvalidParameters, SourceUnavailable, DataMismatch,
    SolverFailed, OutputFailed, IncompleteRun
};
struct ErrorDetail {
    std::string stage;
    std::string message;
    std::string source_path;
    std::string variable;
    std::size_t observation_index = 0;
};
template<class Value, class Error> class Result;  // 拟由项目自行定义
template<class Error> class Status;                // void 成功/失败合同
}
~~~

- **Mat 错误**与**数值错误**分别归属 IO 与 solver，Runner 可以附带上下文，但不得吞掉原始错误。
- **源算法与安全行为区别**：记录 MATLAB 在奇异、除零、NaN 等情况下实际产生的结果；C++ 默认可返回明确非成功状态，但任何“遇错继续/停止/回滚”必须通过入口专属合同评审，不能误标为数学等价。
- **日志**：RunId、source SHA、输入文件/变量、字段维度、原参数、seed、失败阶段、退出码、stdout/stderr 位置。源文件原始 SHA 不因读操作改变。
- **CLI 退出码候选**：0=成功，2=参数错误，3=MAT 文件错误，4=类型/形状错误，5=数值失败，6=结果路径冲突，7=可选绘图不可用。最终数值与优先级以 SRS REQ-I-CLI-001 和 P0.8 审查为准。

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

## 5. 黄金数据与可验收合同模板

| 字段 | 要求 | 当前状态 |
|---|---|---|
| source_file / source_blob_sha | 指向唯一原始 .m 函数版本 | 文件存在，个别 SHA 已读取 |
| fixed_input_identity | 确定性 N×6、near/coeff、参数及 RNG 固定序列 | 尚未建立完整 Golden |
| expected_intermediates | MATLAB 逐轮 near/t/ω 或 M1–M4、b、z、error_mse | 尚未实际采集 |
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

完整的测试文件/CTest 候选位置由 [TEST_PLAN.md](../TEST_PLAN.md) 统一管理；不能因为本表列有 TC 就认定测试已经实现、执行或通过。
