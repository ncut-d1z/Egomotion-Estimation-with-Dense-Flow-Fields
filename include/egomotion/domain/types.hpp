#pragma once

#include <Eigen/Core>
#include <cstddef>
#include <string>
#include <vector>

namespace egomotion {

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
struct MotionEstimate { Vector3 translation; Vector3 rotation; };
struct HarmonicCoefficients { HarmonicMatrix values; };
struct RunId { std::string text; };

// matio-independent metadata. Numeric class/type values are opaque until R2.
struct MatVariableInfo {
    std::string name;
    std::vector<std::size_t> dimensions;
    int original_class_type = 0;
    int original_data_type = 0;
    bool is_complex = false;
    bool is_logical = false;
    // Optional diagnostic metadata populated by the R2 matio reader.
    bool is_global = false;
    bool is_compressed = false;
    std::size_t data_bytes = 0;
    std::string class_name;
    std::string data_type_name;
};

// The reader/writer boundary returns an owning, column-major double payload.
// Unlike an I/O adapter, this value does not own a mat_t or matvar_t.
struct DenseDoubleArray {
    MatVariableInfo info;
    std::vector<double> values_column_major;
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
