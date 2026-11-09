#pragma once

#include <Eigen/Core>

#include <utility>
#include <variant>

// 本模块单独获准实施；不创建或依赖尚未实施的 domain/contracts/Runner。
namespace egomotion::harmonics::low_order {

using Direction = Eigen::Vector3d;
using Matrix3 = Eigen::Matrix3d;
using Matrix5 = Eigen::Matrix<double, 5, 5>;
using Coefficients = Eigen::Matrix<double, 9, 1>;
using BasisValues = Eigen::Matrix<double, 9, 1>;
using MatlabCoefficients = Eigen::Matrix<double, 3, 5, Eigen::ColMajor>;

// 2015 论文附录式(59)--(67)：[g0,f1,f2,f3,h1,h2,h3,h4,h5]。
// f1/f2/f3 对应正 x/y/z；h2/h3/h4/h5 对应 xz/yz/(x²-y²)/xy。
inline constexpr int kCoefficientCount = 9;
inline constexpr int kMaximumDegree = 2;

enum class ErrorCode {
    NonFiniteInput,
    NotUnitDirection,
    NotProperRotation,
    NonFiniteResult,
    ComplexResidue,
    InvalidMethod,
    InvalidSampleShape,
    ZeroProjectionNorm
};

struct Error {
    ErrorCode code;
    const char* message; // 只指向静态字符串，结果不借用调用方内存。
};

// 仅球谐叶子 API 的局部结果类型，不冒充已冻结的跨模块 Result ABI。
// 错态访问明确抛 std::bad_variant_access，不返回默认数学值。
template <class T>
class [[nodiscard]] Result final {
public:
    static Result success(T value) { return Result(std::move(value)); }
    static Result failure(Error error) { return Result(error); }
    bool hasValue() const noexcept { return std::holds_alternative<T>(payload_); }
    const T& value() const & { return std::get<T>(payload_); }
    T value() && { return std::get<T>(std::move(payload_)); }
    Error error() const { return std::get<Error>(payload_); }

private:
    explicit Result(T value) : payload_(std::in_place_index<0>, std::move(value)) {}
    explicit Result(Error error) : payload_(std::in_place_index<1>, error) {}
    std::variant<T, Error> payload_;
};

struct RotationBlocks {
    Matrix3 dipole;
    Matrix5 quadrupole;
    // g0 的块恒为1，不保存可变的零阶矩阵。
};

enum class RotationMethod { TensorDirect, Ivanic1996, Choi1999 };

// 输入必须为有限单位方向；不静默归一化。计算范围固定 l=0,1,2。
Result<BasisValues> basisDirect(const Direction& direction);
Result<BasisValues> basisRecurrence(const Direction& direction);

// 唯一操作语义：mu_new(d) = mu_old(argument_rotation * d)。
// 列系数左乘返回块；因此一阶块为 argument_rotation.transpose()。
// 物体主动旋转/相机反旋应由调用方明确选择 A 或 A^T，模块不猜测。
Result<RotationBlocks> rotationDirect(const Matrix3& argument_rotation);
Result<RotationBlocks> rotationIvanic(const Matrix3& argument_rotation);
Result<RotationBlocks> rotationChoi(const Matrix3& argument_rotation);
Result<Coefficients> rotate(const Coefficients& coefficients,
                            const Matrix3& argument_rotation,
                            RotationMethod method);
Result<double> reconstruct(const Coefficients& coefficients,
                           const Direction& direction);

// 原 koeff_aus_near.m 固定 ordnung=2 的逐个实球谐系数投影，不是联合最小二乘。
// 输入 N 个有限近度与 N×K 样本（K>=3，只有前三列单位视线被使用）；N>0。
// 输出 MATLAB/CS 原有 3×5 布局，只有九项有效，其他位置严格填 0。
// 对 Y_lm 逐样本求 a=(sum near*Y/N)*4pi、b=(sum Y²/N)*4pi，再取 a/b。
// 无原 MATLAB warnings/NaN 传播保证：非有限、空/错尺寸、非单位方向或 b=0
// 会返回明确错误，不会在此引入 Adaptive/Session/联合拟合器。
Result<MatlabCoefficients> projectSourceCoefficientsOrderTwo(
    const Eigen::VectorXd& nearness, const Eigen::MatrixXd& setofpoints);

// 只转换同一个函数的表示，不复刻旧 SHRotate 未共轭相位的系数动作。
// MATLAB 九有效项顺序：(0,0), (1,-1..1), (2,-2..2)，含CS相位。
// 其他3×5位置不参与源重建；输入要求所有位置有限，输出填零无效位置。
Result<Coefficients> fromMatlabLayout(const MatlabCoefficients& matlab);
Result<MatlabCoefficients> toMatlabLayout(const Coefficients& paper);

} // namespace egomotion::harmonics::low_order
