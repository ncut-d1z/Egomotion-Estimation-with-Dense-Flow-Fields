#pragma once

#include <Eigen/Core>
#include <cstddef>
#include <optional>
#include <string>
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

struct FlowField { FlowMatrix observations; };
struct DirectionSet { DirectionMatrix directions; };
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
struct HarmonicCoefficients { HarmonicMatrix values; };
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
