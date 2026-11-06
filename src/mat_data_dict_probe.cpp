// 全量 MAT 字典探针。只读 origin，使用 matio + zlib，C++17 / Debian / Ubuntu。
#include <matio.h>
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;
namespace {
constexpr std::size_t kExpectedFiles = 582;
constexpr std::uint64_t kOffset = UINT64_C(14695981039346656037);
constexpr std::uint64_t kPrime = UINT64_C(1099511628211);
constexpr char kMarker[] = "<!-- generated-by: mat_data_dict_probe.cpp -->";
constexpr char kPreliminaryMarker[] = "<!-- preliminary-mat-data-schema: github-archive-inspection -->";
struct MatCloser { void operator()(mat_t* p) const { if (p) Mat_Close(p); } };
struct VarCloser { void operator()(matvar_t* p) const { if (p) Mat_VarFree(p); } };
using MatHandle = std::unique_ptr<mat_t, MatCloser>;
using VarHandle = std::unique_ptr<matvar_t, VarCloser>;

struct Config {
    fs::path root, output;
    std::string source_sha = "not-specified";
    std::size_t expected = kExpectedFiles;
    bool replace = false;
};
struct Variable {
    std::string name, cls, dtype, shape, compression, flags, status;
    std::size_t bytes = 0;
};
struct File {
    fs::path relative;
    std::uintmax_t bytes = 0;
    std::string hash, version, status = "OK";
    std::vector<Variable> variables;
};

std::string escaped(const std::string& in) {
    std::string out;
    for (char c : in) {
        if (c == '|') out += "\\|";
        else if (c == '\r' || c == '\n') out += ' ';
        else if (c == static_cast<char>(96)) out += '\'';
        else out += c;
    }
    return out;
}
bool within(const fs::path& child, const fs::path& parent) {
    auto i = child.begin();
    for (const auto& part : parent) {
        if (i == child.end() || *i != part) return false;
        ++i;
    }
    return true;
}
Config options(int argc, char** argv) {
    if (argc < 3) throw std::invalid_argument(
        "Usage: mat_data_dict_probe MAT_ROOT REPORT.md "
        "[--source-sha SHA] [--expected-count N] [--replace-generated]");
    Config c;
    c.root = fs::weakly_canonical(fs::absolute(argv[1]));
    c.output = fs::absolute(argv[2]).lexically_normal();
    for (int i = 3; i < argc; ++i) {
        const std::string flag = argv[i];
        if (flag == "--source-sha" && i + 1 < argc) c.source_sha = argv[++i];
        else if (flag == "--expected-count" && i + 1 < argc) {
            const std::string v = argv[++i];
            std::size_t pos = 0;
            const auto n = std::stoull(v, &pos);
            if (pos != v.size() || n == 0) throw std::invalid_argument("Bad expected count");
            c.expected = static_cast<std::size_t>(n);
        } else if (flag == "--replace-generated") c.replace = true;
        else throw std::invalid_argument("Unknown option: " + flag);
    }
    if (!fs::is_directory(c.root)) throw std::invalid_argument("MAT_ROOT is not a directory");
    if (within(fs::weakly_canonical(c.output.parent_path()), c.root))
        throw std::invalid_argument("Report must be outside original data");
    if (fs::exists(c.output)) {
        if (!c.replace) throw std::invalid_argument("Report exists; use --replace-generated");
        std::ifstream old(c.output);
        std::string first;
        std::getline(old, first);
        if (first != kMarker && first != kPreliminaryMarker)
            throw std::invalid_argument("Refusing non-generated report overwrite");
    }
    return c;
}
std::vector<fs::path> filesIn(const fs::path& root) {
    std::vector<fs::path> files;
    for (const fs::directory_entry& e : fs::recursive_directory_iterator(root)) {
        if (e.is_symlink()) throw std::runtime_error("Source symlink: " + e.path().string());
        if (e.is_regular_file() && e.path().extension() == ".mat") files.push_back(e.path());
    }
    std::sort(files.begin(), files.end());
    return files;
}
std::string checksum(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot open file for checksum");
    std::uint64_t hash = kOffset;
    char buffer[8192];
    while (in) {
        in.read(buffer, sizeof(buffer));
        for (std::streamsize i = 0; i < in.gcount(); ++i) {
            hash = (hash ^ static_cast<unsigned char>(buffer[i])) * kPrime;
        }
    }
    if (!in.eof()) throw std::runtime_error("Checksum read failed");
    std::ostringstream out;
    out << std::hex << std::setfill('0') << std::setw(16) << hash;
    return out.str();
}
std::string versionOf(enum mat_ft v) {
    switch (static_cast<int>(v)) {
        case 0x0010: return "MAT v4";
        case 0x0100: return "MAT v5";
        case 0x0200: return "MAT v7.3";
        default: return "unknown " + std::to_string(static_cast<int>(v));
    }
}
std::string typeOf(enum matio_types t) {
    const int n = static_cast<int>(t);
    const std::map<int, std::string> names = {
        {1,"int8"}, {2,"uint8"}, {3,"int16"}, {4,"uint16"},
        {5,"int32"}, {6,"uint32"}, {7,"single"}, {9,"double"},
        {12,"int64"}, {13,"uint64"}, {14,"matrix"}, {15,"compressed"},
        {16,"utf8"}, {17,"utf16"}, {18,"utf32"}
    };
    const auto it = names.find(n);
    return it == names.end() ? "unknown#" + std::to_string(n) : it->second;
}
Variable readVariable(mat_t* mat, const std::string& name) {
    Variable v;
    v.name = name;
    VarHandle handle(Mat_VarRead(mat, name.c_str())); // 加载完整数据，而不只读头。
    if (!handle) { v.status = "READ_FAILED"; return v; }
    const char* class_name = Mat_VarGetClassName(handle.get());
    v.cls = class_name ? class_name : "unknown";
    v.dtype = typeOf(handle->data_type);
    v.bytes = handle->nbytes;
    v.compression = handle->compression == MAT_COMPRESSION_ZLIB ? "zlib" : "none/other";
    v.flags = (handle->isComplex ? "complex " : "") +
              std::string(handle->isLogical ? "logical " : "") +
              (handle->isGlobal ? "global" : "");
    if (v.flags.empty()) v.flags = "-";
    if (handle->rank < 0 || (handle->rank > 0 && handle->dims == nullptr)) {
        v.status = "INVALID_DIMS";
        return v;
    }
    std::ostringstream dims;
    for (int i = 0; i < handle->rank; ++i) {
        if (i > 0) dims << "x";
        dims << handle->dims[i];
    }
    v.shape = dims.str();
    const int cls = static_cast<int>(handle->class_type);
    v.status = (cls >= 6 && cls <= 15 && v.bytes > 0 && handle->data == nullptr)
        ? "NO_NUMERIC_DATA" : "READ_OK";
    return v;
}
File scanFile(const fs::path& path, const fs::path& root) {
    File f;
    f.relative = fs::relative(path, root);
    try {
        f.bytes = fs::file_size(path);
        f.hash = checksum(path);
        MatHandle mat(Mat_Open(path.c_str(), MAT_ACC_RDONLY));
        if (!mat) { f.status = "OPEN_FAILED"; return f; }
        f.version = versionOf(Mat_GetVersion(mat.get()));
        std::size_t n = 0;
        char* const* dir = Mat_GetDir(mat.get(), &n);
        if (!dir || n == 0) { f.status = "EMPTY_OR_DIRECTORY_FAILED"; return f; }
        for (std::size_t i = 0; i < n; ++i) {
            if (!dir[i]) {
                Variable bad;
                bad.name = "(null)";
                bad.status = "INVALID_NAME";
                f.variables.push_back(bad);
            } else f.variables.push_back(readVariable(mat.get(), dir[i]));
        }
        for (const Variable& v : f.variables)
            if (v.status != "READ_OK") f.status = "VARIABLE_FAILURE";
    } catch (const std::exception& ex) {
        f.status = std::string("ERROR: ") + ex.what();
    }
    return f;
}
std::string groupOf(const fs::path& p) {
    const auto s = p.generic_string();
    const auto start = s.find("produced_data/");
    if (start == std::string::npos) return p.parent_path().generic_string();
    const auto end = s.find('/', start + std::string("produced_data/").size());
    return s.substr(0, end);
}
void emit(const Config& cfg, const std::vector<File>& data) {
    std::size_t ok = 0, failed = 0, nv = 0;
    std::map<std::string, std::size_t> groups, versions, classes;
    for (const File& f : data) {
        ++groups[groupOf(f.relative)];
        ++versions[f.version.empty() ? "(unreadable)" : f.version];
        if (f.status == "OK") ++ok; else ++failed;
        for (const Variable& v : f.variables) {
            ++nv;
            ++classes[v.cls.empty() ? "(unreadable)" : v.cls];
        }
    }
    int major = 0, minor = 0, patch = 0;
    Mat_GetLibraryVersion(&major, &minor, &patch);
    std::ostringstream out;
    out << kMarker << "\n# MATLAB MAT 数据字典（matio 读取结果）\n\n"
        << "> 原始数据均以 Mat_Open(...,MAT_ACC_RDONLY) 打开；逐变量通过 "
           "Mat_VarRead 完整解码，而非仅解析文件头。READ_OK 表示变量解码，"
           "不表示与原 MATLAB 求解输出逐项数值等价。\n\n"
        << "- Source SHA（执行参数）: " << escaped(cfg.source_sha) << "\n"
        << "- 原始目录: " << escaped(cfg.root.generic_string()) << "\n"
        << "- matio: " << major << "." << minor << "." << patch << "\n"
        << "- MAT 文件: " << data.size() << " / " << cfg.expected << "\n"
        << "- 成功文件: " << ok << "; 失败文件: " << failed
        << "; 已读取/尝试变量: " << nv << "\n"
        << "- 文件指纹: FNV-1a 64-bit（非密码学 SHA256）。\n\n"
        << "## 1. 分组与类型分布\n\n| 分组 | 文件数 |\n|---|---:|\n";
    for (const auto& kv : groups)
        out << "| " << escaped(kv.first) << " | " << kv.second << " |\n";
    out << "\n| MAT 版本 | 文件数 |\n|---|---:|\n";
    for (const auto& kv : versions)
        out << "| " << escaped(kv.first) << " | " << kv.second << " |\n";
    out << "\n| MATLAB class | 变量数 |\n|---|---:|\n";
    for (const auto& kv : classes)
        out << "| " << escaped(kv.first) << " | " << kv.second << " |\n";
    out << "\n## 2. 全量文件和变量字典\n\n"
        << "| MAT 相对路径 | 字节 | FNV64 | 版本 | 变量 | class | data_type | dims "
           "| data_bytes | 压缩 | flags | 状态 |\n"
        << "|---|---:|---|---|---|---|---|---|---:|---|---|---|\n";
    for (const File& f : data) {
        auto prefix = [&] {
            out << "| " << escaped(f.relative.generic_string()) << " | "
                << f.bytes << " | " << f.hash << " | " << escaped(f.version) << " | ";
        };
        if (f.variables.empty()) {
            prefix();
            out << "— | — | — | — | — | — | — | " << escaped(f.status) << " |\n";
        }
        for (const Variable& v : f.variables) {
            prefix();
            out << escaped(v.name) << " | " << escaped(v.cls) << " | "
                << escaped(v.dtype) << " | " << escaped(v.shape) << " | "
                << v.bytes << " | " << escaped(v.compression) << " | "
                << escaped(v.flags) << " | " << escaped(v.status) << " |\n";
        }
    }
    out << "\n## 3. 全量读取状态\n\n";
    if (data.size() == cfg.expected && failed == 0)
        out << "**全部 " << cfg.expected << " 个 MAT 文件读取成功。** "
               "数值等价、Golden、版权和正式容差仍须独立核验。\n";
    else out << "**未通过**：文件数量或变量读取存在差异；逐行检查失败状态，"
                "不得将元数据检查当成解码完成。\n";
    std::ofstream file(cfg.output, std::ios::out | std::ios::trunc);
    if (!file) throw std::runtime_error("Cannot write report");
    file << out.str();
    file.flush();
    if (!file) throw std::runtime_error("Report write failure");
    std::cerr << "scanned=" << data.size() << ", ok=" << ok
              << ", failed=" << failed << ", variables=" << nv << "\n";
}
} // namespace

int main(int argc, char** argv) {
    try {
        const Config cfg = options(argc, argv);
        const auto paths = filesIn(cfg.root);
        std::vector<File> results;
        results.reserve(paths.size());
        for (const auto& path : paths) results.push_back(scanFile(path, cfg.root));
        emit(cfg, results);
        const std::size_t failed = std::count_if(
            results.begin(), results.end(), [](const File& f) { return f.status != "OK"; });
        return results.size() == cfg.expected && failed == 0 ? 0 : 1;
    } catch (const std::exception& ex) {
        std::cerr << "mat_data_dict_probe: " << ex.what() << "\n";
        return 2;
    }
}
