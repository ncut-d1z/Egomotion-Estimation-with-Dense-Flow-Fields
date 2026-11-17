#include "egomotion/io/mat_result_reader.hpp"
#include "mat_container_validation.hpp"
#include "mat_read_snapshot.hpp"

#include <matio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <sstream>
#include <string>
#include <utility>

namespace egomotion::io {
namespace {
namespace fs = std::filesystem;

struct MatCloser {
    void operator()(mat_t* handle) const noexcept {
        if (handle) Mat_Close(handle);
    }
};
struct VariableCloser {
    void operator()(matvar_t* handle) const noexcept {
        if (handle) Mat_VarFree(handle);
    }
};
using MatHandle = std::unique_ptr<mat_t, MatCloser>;
using VariableHandle = std::unique_ptr<matvar_t, VariableCloser>;
struct MatInput {
    std::optional<detail::MatReadSnapshot> snapshot;
    MatHandle handle;
    mat_t* get() const noexcept{return handle.get();}
};
struct ReadDescriptor {
    int fd=-1;
    ~ReadDescriptor(){if(fd>=0)::close(fd);}
};

template<class Value>
Result<Value, MatErrorCode> fail(MatErrorCode code, ErrorStage stage,
                                 const fs::path& path, std::string message,
                                 std::string variable = {}) {
    ErrorDetail detail;
    detail.stage = stage;
    detail.message = std::move(message);
    detail.source_path = path.string();
    detail.variable = std::move(variable);
    return Result<Value, MatErrorCode>::failure({code, std::move(detail)});
}

std::string versionName(enum mat_ft version) {
    switch (static_cast<int>(version)) {
    case 0x0010: return "MAT v4";
    case 0x0100: return "MAT v5";
    case 0x0200: return "MAT v7.3";
    default: return "unknown " + std::to_string(static_cast<int>(version));
    }
}
std::string typeName(enum matio_types type) {
    switch (static_cast<int>(type)) {
    case 1: return "int8";
    case 2: return "uint8";
    case 3: return "int16";
    case 4: return "uint16";
    case 5: return "int32";
    case 6: return "uint32";
    case 7: return "single";
    case 9: return "double";
    case 12: return "int64";
    case 13: return "uint64";
    case 14: return "matrix";
    case 15: return "compressed";
    case 16: return "utf8";
    case 17: return "utf16";
    case 18: return "utf32";
    default: return "unknown#" + std::to_string(static_cast<int>(type));
    }
}
std::string className(enum matio_classes type) {
    switch (type) {
    case MAT_C_EMPTY: return "empty";
    case MAT_C_CELL: return "cell";
    case MAT_C_STRUCT: return "struct";
    case MAT_C_OBJECT: return "object";
    case MAT_C_CHAR: return "char";
    case MAT_C_SPARSE: return "sparse";
    case MAT_C_DOUBLE: return "double";
    case MAT_C_SINGLE: return "single";
    case MAT_C_INT8: return "int8";
    case MAT_C_UINT8: return "uint8";
    case MAT_C_INT16: return "int16";
    case MAT_C_UINT16: return "uint16";
    case MAT_C_INT32: return "int32";
    case MAT_C_UINT32: return "uint32";
    case MAT_C_INT64: return "int64";
    case MAT_C_UINT64: return "uint64";
    case MAT_C_FUNCTION: return "function";
    case MAT_C_OPAQUE: return "opaque";
    default: return "unknown#" + std::to_string(static_cast<int>(type));
    }
}
bool hasValidDims(const matvar_t& value) noexcept {
    return value.rank >= 0 && (value.rank == 0 || value.dims != nullptr);
}
MatVariableInfo describe(const matvar_t& value, const std::string& name) {
    MatVariableInfo info;
    info.name = name;
    info.original_class_type = static_cast<int>(value.class_type);
    info.original_data_type = static_cast<int>(value.data_type);
    info.class_name = className(value.class_type);
    info.data_type_name = typeName(value.data_type);
    info.data_bytes = value.nbytes;
    info.is_complex = value.isComplex != 0;
    info.is_logical = value.isLogical != 0;
    info.is_global = value.isGlobal != 0;
    info.is_compressed = value.compression == MAT_COMPRESSION_ZLIB;
    if (hasValidDims(value)) {
        for (int i = 0; i < value.rank; ++i)
            info.dimensions.push_back(value.dims[i]);
    }
    return info;
}

Result<MatInput, MatErrorCode> openReadOnly(const fs::path& path,bool verify_container=false,
                                           std::string_view variable={}) {
    if (path.empty() || path.native().find('\0') != std::string::npos)
        return fail<MatInput>(MatErrorCode::MissingFile, ErrorStage::Read,
                               path, "MAT path is empty or contains NUL");
    errno = 0;
    // 完整数值读取捕获并封存字节；元数据入口仍不读取数值载荷。
    ReadDescriptor descriptor;
    MatInput input;
    if(verify_container)descriptor.fd=::open(path.c_str(),O_RDONLY|O_CLOEXEC|O_NONBLOCK);
    if(descriptor.fd>=0){
        struct stat info{};
        if(::fstat(descriptor.fd,&info)!=0||!S_ISREG(info.st_mode))
            return fail<MatInput>(MatErrorCode::InvalidMatHeader,ErrorStage::Read,path,
                                   "Numeric MAT input must be a regular file",std::string(variable));
        auto snapshot=detail::snapshotMatInput(descriptor.fd,path,variable);
        if(!snapshot.hasValue())
            return Result<MatInput,MatErrorCode>::failure(std::move(snapshot).error());
        input.snapshot.emplace(std::move(snapshot).value());
    }
    const std::string read_path=verify_container?
        "/proc/self/fd/"+std::to_string(input.snapshot?input.snapshot->fd():-1):path.string();
    MatHandle handle(verify_container&&descriptor.fd<0?nullptr:
                     Mat_Open(read_path.c_str(), MAT_ACC_RDONLY));
    if (!handle) {
        const int saved_errno = errno;
        // matio may clear errno internally. The supplementary existence probe
        // is error classification ONLY, never a second MAT data read.
        std::error_code status_error;
        const bool still_exists = fs::exists(path, status_error);
        const MatErrorCode code = saved_errno == EACCES || saved_errno == EPERM
            ? MatErrorCode::PermissionDenied
            : (saved_errno == ENOENT || (!status_error && !still_exists)
                ? MatErrorCode::MissingFile : MatErrorCode::InvalidMatHeader);
        return fail<MatInput>(code, ErrorStage::Read, path,
                               "Unable to open MAT file for reading");
    }
    if(verify_container&&Mat_GetVersion(handle.get())==MAT_FT_MAT5){
        auto checked=detail::validateMat5Container(input.snapshot->fd(),path,variable);
        if(!checked.hasValue())
            return Result<MatInput,MatErrorCode>::failure(std::move(checked).error());
    }
    input.handle=std::move(handle);
    return Result<MatInput, MatErrorCode>::success(std::move(input));
}
std::size_t occurrences(char* const* directory, std::size_t count,
                        std::string_view target) {
    std::size_t matches = 0;
    if (!directory) return matches;
    for (std::size_t i = 0; i < count; ++i)
        if (directory[i] && target == directory[i]) ++matches;
    return matches;
}
// Business numeric reads are limited to the actual MAT v5/zlib archive;
// metadata enumeration can still identify otherwise unsupported versions.
bool isSupportedNumericFile(mat_t* handle) {
    return static_cast<int>(Mat_GetVersion(handle)) == 0x0100;
}

Result<DenseDoubleArray, MatErrorCode> owningDouble(const matvar_t& variable,
                                                    const fs::path& path,
                                                    std::string name) {
    if (variable.class_type != MAT_C_DOUBLE || variable.data_type != MAT_T_DOUBLE ||
        variable.isComplex || variable.isLogical)
        return fail<DenseDoubleArray>(MatErrorCode::UnsupportedClassOrType,
                                      ErrorStage::Decode, path,
                                      "Only real dense non-logical double arrays are supported", name);
    if (!hasValidDims(variable) || variable.rank != 2 ||
        variable.dims[0] == 0 || variable.dims[1] == 0)
        return fail<DenseDoubleArray>(MatErrorCode::InvalidShape, ErrorStage::Decode,
                                      path, "Expected a nonempty two-dimensional array", name);
    const std::size_t rows = variable.dims[0], columns = variable.dims[1];
    if (columns > std::numeric_limits<std::size_t>::max() / rows)
        return fail<DenseDoubleArray>(MatErrorCode::SizeOverflow, ErrorStage::Decode,
                                      path, "MAT dimensions overflow element count", name);
    const std::size_t count = rows * columns;
    if (count > std::numeric_limits<std::size_t>::max() / sizeof(double))
        return fail<DenseDoubleArray>(MatErrorCode::SizeOverflow, ErrorStage::Decode,
                                      path, "MAT element count exceeds byte address space", name);
    if (variable.nbytes != count * sizeof(double))
        return fail<DenseDoubleArray>(MatErrorCode::InvalidShape, ErrorStage::Decode,
                                      path, "MAT data length differs from rank/dimensions", name);
    if (!variable.data)
        return fail<DenseDoubleArray>(MatErrorCode::CorruptCompressedData,
                                      ErrorStage::Decode, path, "MAT numeric payload unavailable", name);
    try {
        DenseDoubleArray array;
        array.info = describe(variable, name);
        array.values_column_major.resize(count);
        std::memcpy(array.values_column_major.data(), variable.data,
                    count * sizeof(double));
        return Result<DenseDoubleArray, MatErrorCode>::success(std::move(array));
    } catch (const std::bad_alloc&) {
        return fail<DenseDoubleArray>(MatErrorCode::SizeOverflow, ErrorStage::Decode,
                                      path, "Cannot allocate owning MAT payload", name);
    }
}
Result<DenseDoubleArray, MatErrorCode> readWithHandle(const MatInput& mat,
                                                      const fs::path& path,
                                                      const std::string& name) {
    VariableHandle value(Mat_VarRead(mat.get(), name.c_str()));
    if (!value)
        return fail<DenseDoubleArray>(MatErrorCode::CorruptCompressedData,
                                      ErrorStage::Decode, path,
                                      "matio could not decode named MAT variable", name);
    auto decoded=owningDouble(*value,path,name);
    if(!decoded.hasValue())return decoded;
    auto array=std::move(decoded).value();
    if(mat.snapshot)array.source_mat_sha256=mat.snapshot->sha256();
    return Result<DenseDoubleArray,MatErrorCode>::success(std::move(array));
}
}  // namespace

Result<MatFileScan, MatErrorCode>
MatResultReader::scanFile(const std::filesystem::path& path, ScanMode mode) const {
    auto opened = openReadOnly(path,mode==ScanMode::VerifyDecoded);
    if (!opened.hasValue())
        return Result<MatFileScan, MatErrorCode>::failure(
            std::move(opened).error());
    MatInput mat = std::move(opened).value();
    std::size_t count = 0;
    char* const* directory = Mat_GetDir(mat.get(), &count);
    if (!directory || count == 0)
        return fail<MatFileScan>(MatErrorCode::InvalidMatHeader, ErrorStage::Read,
                                 path, "EMPTY_OR_DIRECTORY_FAILED");
    MatFileScan result;
    result.version = versionName(Mat_GetVersion(mat.get()));
    try {
        result.variables.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            if (!directory[i]) {
                MatScanVariable bad;
                bad.info.name = "(null)";
                bad.status = "INVALID_NAME";
                result.variables.push_back(std::move(bad));
                continue;
            }
            const std::string name(directory[i]);
            // Metadata-only inspection never calls Mat_VarRead.
            VariableHandle value(mode == ScanMode::MetadataOnly
                ? Mat_VarReadInfo(mat.get(), name.c_str())
                : Mat_VarRead(mat.get(), name.c_str()));
            if (!value) {
                MatScanVariable bad;
                bad.info.name = name;
                bad.status = mode == ScanMode::MetadataOnly ? "INFO_FAILED" : "READ_FAILED";
                result.variables.push_back(std::move(bad));
                continue;
            }
            MatScanVariable entry;
            entry.info = describe(*value, name);
            if (!hasValidDims(*value)) {
                entry.status = "INVALID_DIMS";
            } else if (mode == ScanMode::MetadataOnly) {
                entry.status = "METADATA_OK";
            } else {
                const int cls = static_cast<int>(value->class_type);
                entry.status = cls >= 6 && cls <= 15 && value->nbytes > 0 &&
                               value->data == nullptr ? "NO_NUMERIC_DATA" : "READ_OK";
            }
            result.variables.push_back(std::move(entry));
        }
    } catch (const std::bad_alloc&) {
        return fail<MatFileScan>(MatErrorCode::SizeOverflow, ErrorStage::Decode,
                                 path, "Cannot allocate MAT variable metadata");
    }
    return Result<MatFileScan, MatErrorCode>::success(std::move(result));
}

Result<std::vector<MatVariableInfo>, MatErrorCode>
MatResultReader::inspect(const std::filesystem::path& path) const {
    auto scan = scanFile(path, ScanMode::MetadataOnly);
    if (!scan.hasValue())
        return Result<std::vector<MatVariableInfo>, MatErrorCode>::failure(
            std::move(scan).error());
    std::vector<MatVariableInfo> variables;
    for (const MatScanVariable& record : scan.value().variables) {
        if (record.status != "METADATA_OK")
            return fail<std::vector<MatVariableInfo>>(
                MatErrorCode::InvalidMatHeader, ErrorStage::Read,
                path, "Invalid MAT variable metadata", record.info.name);
        variables.push_back(record.info);
    }
    return Result<std::vector<MatVariableInfo>, MatErrorCode>::success(
        std::move(variables));
}

Result<DenseDoubleArray, MatErrorCode>
MatResultReader::readNamed(const std::filesystem::path& path,
                           std::string_view variable) const {
    if (variable.empty() || variable.find('\0') != std::string_view::npos)
        return fail<DenseDoubleArray>(MatErrorCode::MissingVariable, ErrorStage::Read,
                                      path, "MAT variable name is empty or invalid");
    auto opened = openReadOnly(path,true,variable);
    if (!opened.hasValue())
        return Result<DenseDoubleArray, MatErrorCode>::failure(
            std::move(opened).error());
    MatInput mat = std::move(opened).value();
    std::size_t count = 0;
    char* const* directory = Mat_GetDir(mat.get(), &count);
    if (!directory || count == 0)
        return fail<DenseDoubleArray>(MatErrorCode::InvalidMatHeader,
                                      ErrorStage::Read, path, "MAT directory is missing");
    const auto matches = occurrences(directory, count, variable);
    if (matches == 0)
        return fail<DenseDoubleArray>(MatErrorCode::MissingVariable, ErrorStage::Read,
                                      path, "Named MAT variable does not exist",
                                      std::string(variable));
    if (matches != 1)
        return fail<DenseDoubleArray>(MatErrorCode::AmbiguousVariable, ErrorStage::Read,
                                      path, "Duplicate variable name in MAT directory",
                                      std::string(variable));
    if (!isSupportedNumericFile(mat.get()))
        return fail<DenseDoubleArray>(MatErrorCode::UnsupportedClassOrType,
                                      ErrorStage::Decode, path,
                                      "Numeric MAT reader supports only MAT v5", std::string(variable));
    return readWithHandle(mat, path, std::string(variable));
}

Result<DenseDoubleArray, MatErrorCode>
MatResultReader::readFirstCompatible(const std::filesystem::path& path) const {
    std::filesystem::path actual = path;
    if (!actual.has_extension() && !actual.empty() && actual.native().find('\0') == std::string::npos) {
        std::error_code ec;
        if (!std::filesystem::exists(actual, ec))
            actual += ".mat";
    }
    auto opened = openReadOnly(actual,true);
    if (!opened.hasValue())
        return Result<DenseDoubleArray, MatErrorCode>::failure(
            std::move(opened).error());
    MatInput mat = std::move(opened).value();
    std::size_t count = 0;
    char* const* directory = Mat_GetDir(mat.get(), &count);
    if (!directory || count == 0)
        return fail<DenseDoubleArray>(MatErrorCode::MissingVariable, ErrorStage::Read,
                                      actual, "MAT file contains no variables");
    if (count != 1 || !directory[0])
        return fail<DenseDoubleArray>(MatErrorCode::AmbiguousVariable, ErrorStage::Read,
                                      actual, "MAT variable order is ambiguous");
    if (!isSupportedNumericFile(mat.get()))
        return fail<DenseDoubleArray>(MatErrorCode::UnsupportedClassOrType,
                                      ErrorStage::Decode, actual,
                                      "Numeric MAT reader supports only MAT v5",
                                      directory[0]);
    return readWithHandle(mat, actual, directory[0]);
}

std::string MatResultReader::libraryVersion() {
    int major = 0, minor = 0, patch = 0;
    Mat_GetLibraryVersion(&major, &minor, &patch);
    return std::to_string(major) + "." + std::to_string(minor) + "." +
           std::to_string(patch);
}

}  // namespace egomotion::io
