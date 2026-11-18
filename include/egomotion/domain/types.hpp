#pragma once

#include <Eigen/Core>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace egomotion {

template<class T,class Code> class Result;
enum class NumericalErrorCode;

using Scalar = double;
using Vector3 = Eigen::Matrix<Scalar, 3, 1>;
using Matrix3 = Eigen::Matrix<Scalar, 3, 3>;
using Matrix6 = Eigen::Matrix<Scalar, 6, 6>;
using Vector6 = Eigen::Matrix<Scalar, 6, 1>;
using HarmonicMatrix = Eigen::Matrix<Scalar, 3, 5, Eigen::ColMajor>;
using FlowMatrix = Eigen::Matrix<Scalar, Eigen::Dynamic, 6, Eigen::ColMajor>;
using DirectionMatrix = Eigen::Matrix<Scalar, Eigen::Dynamic, 3, Eigen::ColMajor>;

// N×6有限观测存储；空/单位/切向条件仍由实际算法按源语义判断。
class FlowField final {
public:
    FlowField() noexcept = default;
    FlowField(const FlowField&) = default;
    FlowField(FlowField&&) noexcept = default;
    FlowField& operator=(const FlowField& other);
    FlowField& operator=(FlowField&&) noexcept = default;
    static Result<FlowField,NumericalErrorCode> create(
        const Eigen::Ref<const Eigen::MatrixXd>& observations);
    // 先校验再移交，失败保留输入；成功后调用者须放弃旧可写别名。
    static Result<FlowField,NumericalErrorCode> fromOwned(FlowMatrix&& observations);
    // 阻止Eigen在进入工厂前隐式转换形状或分配另一份矩阵。
    template<class T> static Result<FlowField,NumericalErrorCode> fromOwned(T&&) = delete;

    const FlowMatrix& observations() const & noexcept { return observations_; }
    FlowMatrix observations() && noexcept { return std::move(observations_); }
    FlowMatrix observations() const && { return observations_; }
private:
    explicit FlowField(FlowMatrix&& observations) noexcept : observations_(std::move(observations)) {}
    FlowMatrix observations_;
};
// N×3有限方向存储；允许空集合，单位/非零要求由实际物理模型检查。
class DirectionSet final {
public:
    DirectionSet() noexcept = default;
    DirectionSet(const DirectionSet&) = default;
    DirectionSet(DirectionSet&&) noexcept = default;
    DirectionSet& operator=(const DirectionSet& other);
    DirectionSet& operator=(DirectionSet&&) noexcept = default;
    static Result<DirectionSet,NumericalErrorCode> create(
        const Eigen::Ref<const Eigen::MatrixXd>& directions);
    // 先校验再移交，失败保留输入；成功后调用者须放弃旧可写别名。
    static Result<DirectionSet,NumericalErrorCode> fromOwned(DirectionMatrix&& directions);
    // 阻止Eigen在进入工厂前隐式转换形状或分配另一份矩阵。
    template<class T> static Result<DirectionSet,NumericalErrorCode> fromOwned(T&&) = delete;

    const DirectionMatrix& directions() const & noexcept { return directions_; }
    DirectionMatrix directions() && noexcept { return std::move(directions_); }
    DirectionMatrix directions() const && { return directions_; }
private:
    explicit DirectionSet(DirectionMatrix&& directions) noexcept : directions_(std::move(directions)) {}
    DirectionMatrix directions_;
};
// 有限运动值；零运动合法，归一化/可观测性由具体算法判断。
// 默认值明确为静止，不表示Result成功，也不允许未初始化分量。
class MotionEstimate final {
public:
    MotionEstimate() noexcept = default;
    static Result<MotionEstimate,NumericalErrorCode> create(
        const Eigen::Ref<const Eigen::MatrixXd>& translation,
        const Eigen::Ref<const Eigen::MatrixXd>& rotation);

    const Vector3& translation() const & noexcept { return translation_; }
    const Vector3& rotation() const & noexcept { return rotation_; }
    Vector3 translation() && noexcept { return translation_; }
    Vector3 rotation() && noexcept { return rotation_; }
    Vector3 translation() const && noexcept { return translation_; }
    Vector3 rotation() const && noexcept { return rotation_; }
private:
    MotionEstimate(const Vector3& translation,const Vector3& rotation) noexcept
        : translation_(translation),rotation_(rotation) {}
    Vector3 translation_=Vector3::Zero();
    Vector3 rotation_=Vector3::Zero();
};
// MATLAB 3×5列主序；1+3+5项有效，六个未用位置也须有限但不强制为零。
class HarmonicCoefficients final {
public:
    HarmonicCoefficients() noexcept = default;
    static Result<HarmonicCoefficients,NumericalErrorCode> create(
        const Eigen::Ref<const Eigen::MatrixXd>& values);
    // 原脚本akkoeff(1,1)=1，其余为零；不是把重建近度归一成1。
    static HarmonicCoefficients zeroOrderUnit() noexcept;

    const HarmonicMatrix& values() const & noexcept { return values_; }
    HarmonicMatrix values() && noexcept { return values_; }
    HarmonicMatrix values() const && noexcept { return values_; }
private:
    explicit HarmonicCoefficients(const HarmonicMatrix& values) noexcept : values_(values) {}
    HarmonicMatrix values_=HarmonicMatrix::Zero();
};
struct RunId { std::string text; };

// matio-independent metadata. Numeric class/type values are opaque until R2.
struct MatVariableInfo {
    std::string name;
    std::vector<std::size_t> dimensions;
    int original_class_type = 0;
    // matio 的字段快照：MAT v5 inspect 可为 0（MAT_T_UNKNOWN），
    // 完整解码的内存类型另行严格校验；不凭 class 推造物理数据标签。
    int original_data_type = 0;
    bool is_complex = false;
    bool is_logical = false;
    // Optional diagnostic metadata populated by the R2 matio reader.
    bool is_global = false;
    bool is_compressed = false;
    std::size_t data_bytes = 0; // 仅元数据读取时 0 也可能表示尚未提供。
    std::string class_name;
    std::string data_type_name;
};

// The reader/writer boundary returns an owning, column-major double payload.
// Unlike an I/O adapter, this value does not own a mat_t or matvar_t.
struct DenseDoubleArray {
    MatVariableInfo info;
    std::vector<double> values_column_major;
    // 仅Reader完整解码的封存MAT字节身份；合成数组/未知来源保持空。
    std::optional<std::string> source_mat_sha256;
};

struct FileEvidence {
    std::string variable;
    std::string filename;
    std::string sha256;
    std::size_t size_bytes=0;
    bool durably_confirmed=false;
};
enum class RunStatus { Complete, Incomplete, Failed };

struct RunOutcome {
    RunStatus status = RunStatus::Failed;
    RunId run_id;
    std::size_t attempted = 0;
    std::size_t completed = 0;
    std::size_t valid = 0;
    std::size_t failed = 0;
    std::size_t undefined = 0;
};

}  // namespace egomotion
