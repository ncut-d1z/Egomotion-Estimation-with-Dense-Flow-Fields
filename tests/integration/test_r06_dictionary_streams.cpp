#include "egomotion/cli/mat_dictionary_command.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <locale>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
namespace fs = std::filesystem;
std::size_t checks = 0, calls = 0;
void require(bool value, const char* reason) {
    ++checks;
    if (!value) throw std::runtime_error(reason);
}
std::string read(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    require(file.good(), "Missing input/report");
    return {std::istreambuf_iterator<char>(file), {}};
}
int invoke(const std::vector<std::string>& args, std::ostream& diagnostic) {
    std::vector<const char*> argv{"mat_data_dict_probe"};
    for (const auto& arg : args) argv.push_back(arg.c_str());
    ++calls;
    return egomotion::cli::runMatDictionaryCommand(
        static_cast<int>(argv.size()), argv.data(), diagnostic);
}
class GroupedNumbers final : public std::numpunct<char> {
protected:
    char do_thousands_sep() const override { return '_'; }
    std::string do_grouping() const override { return "\3"; }
};
class GlobalLocale final {
public:
    explicit GlobalLocale(const std::locale& locale)
        : previous_(std::locale::global(locale)) {}
    ~GlobalLocale() { std::locale::global(previous_); }
    GlobalLocale(const GlobalLocale&) = delete;
    GlobalLocale& operator=(const GlobalLocale&) = delete;
private:
    std::locale previous_;
};
void localeIndependentReport(const fs::path& source, const fs::path& sandbox) {
    const auto original = std::locale();
    const auto ordinary = sandbox / "locale-classic.md";
    const auto grouped = sandbox / "locale-grouped.md";
    {
        const GlobalLocale locale(std::locale::classic());
        std::ostringstream diagnostic;
        require(invoke({source.string(), ordinary.string(), "--expected-count", "4"}, diagnostic) == 0,
                "Classic-locale report must succeed");
        require(diagnostic.str() == "scanned=4, ok=4, failed=0, variables=4\n",
                "Classic-locale status must preserve its format");
    }
    {
        const GlobalLocale locale(std::locale(std::locale::classic(), new GroupedNumbers));
        std::ostringstream witness;
        witness << std::hex << 0x1234u;
        require(witness.str() == "1_234", "Custom locale must actually group hexadecimal numbers");
        std::ostringstream diagnostic;
        require(invoke({source.string(), grouped.string(), "--expected-count", "4"}, diagnostic) == 0,
                "Grouped-locale report must succeed");
        require(diagnostic.str() == "scanned=4, ok=4, failed=0, variables=4\n",
                "Caller locale must not change status formatting");
    }
    require(std::locale() == original, "Report locale test must restore the caller's global locale");
    // 报告不包含输出文件名；相同输入和参数应保留包括FNV64在内的相同字节。
    require(read(grouped) == read(ordinary), "Caller locale must not change FNV64 or report bytes");
}
enum class Fault { ShortWrite, PartialWrite, Flush, ThrowWrite, ThrowFlush };
class FaultBuffer final : public std::stringbuf {
public:
    explicit FaultBuffer(Fault fault) : fault_(fault) {}
protected:
    std::streamsize xsputn(const char* text, std::streamsize size) override {
        if (fault_ == Fault::ThrowWrite) throw std::runtime_error("write fault");
        if (fault_ == Fault::ShortWrite) return 0;
        if (fault_ == Fault::PartialWrite)
            return std::stringbuf::xsputn(text, size / 2);
        return std::stringbuf::xsputn(text, size);
    }
    int sync() override {
        if (fault_ == Fault::ThrowFlush) throw std::runtime_error("flush fault");
        return fault_ == Fault::Flush ? -1 : 0;
    }
private:
    Fault fault_;
};
void outputFaults(const fs::path& source, const fs::path& sandbox) {
    for (const auto fault : {Fault::ShortWrite, Fault::PartialWrite, Fault::Flush,
                             Fault::ThrowWrite, Fault::ThrowFlush}) {
        for (const bool throws : {false, true}) {
            for (const bool count_matches : {false, true}) {
                const auto report = sandbox / (std::to_string(calls) + ".md");
                FaultBuffer buffer(fault);
                std::ostream diagnostic(&buffer);
                if (throws) diagnostic.exceptions(std::ios::failbit | std::ios::badbit);
                require(invoke({source.string(), report.string(), "--expected-count",
                                count_matches ? "4" : "5"}, diagnostic) == 2,
                        "Failed status delivery must return legacy execution failure 2");
                require(!diagnostic.good(), "Failed stream state must remain observable");
                const auto bytes = read(report);
                require(bytes.find("成功文件: 4; 失败文件: 0") != std::string::npos,
                        "Status failure must preserve the completed report");
                require(bytes.find(count_matches ? "**全部 4 个 MAT 文件读取成功。**"
                                                : "**未通过**") != std::string::npos,
                        "Report must retain actual scan/count outcome");
                if (fault == Fault::PartialWrite)
                    require(!buffer.str().empty(), "Already delivered bytes must remain");
            }
            FaultBuffer buffer(fault);
            std::ostream diagnostic(&buffer);
            if (throws) diagnostic.exceptions(std::ios::failbit | std::ios::badbit);
            require(invoke({}, diagnostic) == 2, "Broken exception diagnostic must not escape");
            require(!diagnostic.good(), "Exception diagnostic failure state was erased");
        }
    }
}
void validAndInvalid(const fs::path& source, const fs::path& sandbox) {
    const std::string status = "scanned=4, ok=4, failed=0, variables=4\n";
    std::ostringstream success, incomplete;
    require(invoke({source.string(), (sandbox / "complete.md").string(),
                    "--expected-count", "4"}, success) == 0, "Normal successful exit");
    require(success.str() == status, "Historical status format changed");
    require(invoke({source.string(), (sandbox / "incomplete.md").string(),
                    "--expected-count", "5"}, incomplete) == 1, "Count mismatch exit");
    require(incomplete.str() == status, "Count mismatch must report actual counts");
    for (const auto& count : {"0", "-1", "+4", "4x", " 4", "", "184467440737095516160"}) {
        std::ostringstream diagnostic;
        const auto report = sandbox / ("invalid-" + std::to_string(calls) + ".md");
        require(invoke({source.string(), report.string(), "--expected-count", count},
                       diagnostic) == 2, "Malformed count must fail before report I/O");
        require(!fs::exists(report), "Invalid argv created report");
        require(diagnostic.str().find("mat_data_dict_probe: ") == 0,
                "Historical diagnostic prefix changed");
    }
    std::ostringstream usage;
    require(invoke({}, usage) == 2, "Usage exit");
    require(usage.str().find("Usage: mat_data_dict_probe") != std::string::npos,
            "Usage diagnostic missing");
}
}  // namespace

int main(int argc, char** argv) {
    if (argc != 3) throw std::invalid_argument("Expected four-MAT source and sandbox");
    const fs::path source(argv[1]), parent(argv[2]);
    fs::create_directories(parent);
    std::string pattern = (parent / "探针 流-XXXXXX").string();
    require(::mkdtemp(pattern.data()) != nullptr, "Cannot reserve new sandbox");
    const fs::path sandbox(pattern);
    std::map<fs::path, std::string> originals;
    for (const auto& file : fs::directory_iterator(source))
        if (file.path().extension() == ".mat") originals.emplace(file.path(), read(file.path()));
    require(originals.size() == 4, "Expected immutable four-file initialization archive");
    validAndInvalid(source, sandbox);
    localeIndependentReport(source, sandbox);
    outputFaults(source, sandbox);
    for (const auto& file : originals)
        require(read(file.first) == file.second, "Original MAT changed");
    std::cout << "PASS dictionary streams checks=" << checks << " calls=" << calls
              << " evidence=" << sandbox << '\n';
}
