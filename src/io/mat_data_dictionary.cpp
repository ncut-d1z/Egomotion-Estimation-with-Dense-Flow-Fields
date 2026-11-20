#include "egomotion/io/mat_data_dictionary.hpp"
#include "egomotion/io/mat_result_reader.hpp"


#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


namespace egomotion::io {
namespace fs = std::filesystem;
namespace {
using Config = MatDictionaryOptions;

constexpr std::uint64_t kOffset = UINT64_C(14695981039346656037);
constexpr std::uint64_t kPrime = UINT64_C(1099511628211);
constexpr char kMarker[] = "<!-- generated-by: mat_data_dict_probe.cpp -->";
constexpr char kPreliminaryMarker[] =
    "<!-- preliminary-mat-data-schema: github-archive-inspection -->";

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
    out.exceptions(std::ios::badbit | std::ios::failbit);
    out << std::hex << std::setfill('0') << std::setw(16) << hash;
    return out.str();
}


void validate(Config& cfg) {
    if (cfg.root.empty() || cfg.output.empty() || cfg.expected == 0)
        throw std::invalid_argument("Missing MAT_ROOT, REPORT or expected count");
    cfg.root = fs::weakly_canonical(fs::absolute(cfg.root));
    cfg.output = fs::absolute(cfg.output).lexically_normal();
    if (!fs::is_directory(cfg.root))
        throw std::invalid_argument("MAT_ROOT is not a directory");
    if (within(fs::weakly_canonical(cfg.output.parent_path()), cfg.root))
        throw std::invalid_argument("Report must be outside original data");
    // Do not follow an existing report symlink even if its visible name matches.
    if (fs::is_symlink(fs::symlink_status(cfg.output)))
        throw std::invalid_argument("Refusing output report symlink");
    if (fs::exists(cfg.output)) {
        if (!cfg.replace)
            throw std::invalid_argument("Report exists; use --replace-generated");
        std::ifstream previous(cfg.output);
        std::string first;
        std::getline(previous, first);
        if (first != kMarker && first != kPreliminaryMarker)
            throw std::invalid_argument("Refusing non-generated report overwrite");
    }
}

File scanFile(const fs::path& path, const fs::path& root,
              const MatResultReader& reader) {
    File file;
    file.relative = fs::relative(path, root);
    try {
        file.bytes = fs::file_size(path);
        file.hash = checksum(path);
        // Reads every variable's entire data using ONE Mat_Open handle;
        // no MAT decoder or MAT pointers live in the dictionary layer.
        auto response = reader.scanFile(path, ScanMode::VerifyDecoded);
        if (!response.hasValue()) {
            const auto& failure = response.error();
            file.status = failure.detail.message == "EMPTY_OR_DIRECTORY_FAILED"
                ? "EMPTY_OR_DIRECTORY_FAILED" : "OPEN_FAILED";
            return file;
        }
        file.version = response.value().version;
        for (const MatScanVariable& record : response.value().variables) {
            const MatVariableInfo& info = record.info;
            Variable variable;
            variable.name = info.name;
            variable.cls = info.class_name;
            variable.dtype = info.data_type_name;
            variable.bytes = info.data_bytes;
            variable.compression = info.is_compressed ? "zlib" : "none/other";
            variable.flags = (info.is_complex ? "complex " : "") +
                std::string(info.is_logical ? "logical " : "") +
                (info.is_global ? "global" : "");
            if (variable.flags.empty()) variable.flags = "-";
            for (std::size_t i = 0; i < info.dimensions.size(); ++i) {
                if (i != 0) variable.shape += "x";
                variable.shape += std::to_string(info.dimensions[i]);
            }
            variable.status = record.status;
            if (variable.status != "READ_OK") file.status = "VARIABLE_FAILURE";
            file.variables.push_back(std::move(variable));
        }
    } catch (const std::exception& ex) {
        file.status = std::string("ERROR: ") + ex.what();
    }
    return file;
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
    const auto matio_version = MatResultReader::libraryVersion();
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out.exceptions(std::ios::badbit | std::ios::failbit);
    out << kMarker << "\n# MATLAB MAT 数据字典（matio 读取结果）\n\n"
        << "> 原始数据均以 Mat_Open(...,MAT_ACC_RDONLY) 打开；逐变量通过 "
           "Mat_VarRead 完整解码，而非仅解析文件头。READ_OK 表示变量解码，"
           "不表示与原 MATLAB 求解输出逐项数值等价。data_type/data_bytes 是 "
           "matio 完整解码后的字段，不是文件内原始存储标签及载荷长度。\n\n"
        << "- Source SHA（执行参数）: " << escaped(cfg.source_sha) << "\n"
        << "- 原始目录: " << escaped(cfg.root.generic_string()) << "\n"
        << "- matio: " << matio_version << "\n"
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
    // fclose也可能失败；析构器不能把关闭失败传播给调用者。
    file.close();
    if (!file) throw std::runtime_error("Report close failure");
}


}  // namespace

MatDictionarySummary generateMatDictionaryReport(MatDictionaryOptions options) {
    validate(options);
    const auto paths = filesIn(options.root);
    const MatResultReader reader;
    std::vector<File> data;
    data.reserve(paths.size());
    for (const auto& path : paths)
        data.push_back(scanFile(path, options.root, reader));
    emit(options, data);

    MatDictionarySummary summary;
    summary.scanned = data.size();
    for (const File& file : data) {
        if (file.status == "OK") ++summary.succeeded;
        else ++summary.failed;
        summary.variables += file.variables.size();
    }
    return summary;
}

}  // namespace egomotion::io
