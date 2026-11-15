#include "egomotion/cli/command.hpp"

#include <cassert>
#include <sstream>
#include <string>
#include <vector>

namespace {
struct FakeReader : egomotion::IResultReader {
    mutable int inspect_calls = 0;
    mutable int read_calls = 0;

    egomotion::Result<std::vector<egomotion::MatVariableInfo>, egomotion::MatErrorCode>
    inspect(const std::filesystem::path&) const override {
        ++inspect_calls;
        egomotion::MatVariableInfo info;
        info.name = "xwerte";
        info.dimensions = {10, 1};
        info.original_class_type = 6;
        info.original_data_type = 9;
        return egomotion::Result<std::vector<egomotion::MatVariableInfo>,
                                 egomotion::MatErrorCode>::success({info});
    }
    egomotion::Result<egomotion::DenseDoubleArray, egomotion::MatErrorCode>
    readNamed(const std::filesystem::path&, std::string_view) const override {
        ++read_calls;
        return egomotion::Result<egomotion::DenseDoubleArray,
                                 egomotion::MatErrorCode>::failure(
            {egomotion::MatErrorCode::MissingVariable, egomotion::ErrorDetail{}});
    }
    egomotion::Result<egomotion::DenseDoubleArray, egomotion::MatErrorCode>
    readFirstCompatible(const std::filesystem::path&) const override {
        ++read_calls;
        return egomotion::Result<egomotion::DenseDoubleArray,
                                 egomotion::MatErrorCode>::failure(
            {egomotion::MatErrorCode::MissingVariable, egomotion::ErrorDetail{}});
    }
};
}  // namespace

int main() {
    using namespace egomotion::cli;
    FakeReader reader;
    ApplicationCompositionRoot app(&reader);
    auto check = [&](std::vector<std::string> args, int expected,
                     const std::string& needle) {
        std::ostringstream out, err;
        const int exit_code = run(args, out, err, app);
        assert(exit_code == expected);
        if (expected == 0)
            assert(out.str().find(needle) != std::string::npos);
        else
            assert(err.str().find(needle) != std::string::npos);
    };
    check({}, 0, "Usage:");
    check({"--version"}, 0, "0.1.0");
    check({"unknown"}, 2, "\"error_domain\":\"ExperimentErrorCode\"");
    check({"inspect-mat"}, 2, "requires --file");
    check({"inspect-mat", "--file", "a.mat", "--mode", "compat"},
          2, "requires --file");
    check({"run-bias-occluded", "--seed", "-1", "--level", "0"}, 2, "decimal range");
    check({"run-bias-proportional", "--seed", "4294967296", "--level", "0"},
          2, "decimal range");
    check({"run-bias-proportional", "--seed", "4", "--level", "-1"}, 2, "decimal range");
    check({"run-adaptive-comparison", "--noise-scale", "NaN"}, 2, "finite");
    check({"run-adaptive-comparison", "--noise-scale", "-0.5"}, 5, "not implemented");
    check({"run-adaptive-update-rates", "--run-id", "../origin"}, 2, "Invalid RunId");
    check({"analyze", "--experiment", "unknown", "--input-root", "a"},
          2, "Invalid --experiment");
    check({"analyze", "--experiment", "bias", "--input-root", "a"}, 5, "not implemented");
    check({"plot", "--experiment", "bias", "--input-root", "a"}, 7, "not implemented");
    check({"inspect-mat", "--file", "input file.mat"}, 0, "rank=2");
    check({"inspect-mat", "--file", "a.mat", "--variable", "xwerte"},
          0, "variable=xwerte");
    check({"inspect-mat", "--file", "a.mat", "--variable", "missing"},
          3, "MissingVariable");
    assert(reader.inspect_calls == 3);
    assert(reader.read_calls == 0);
}
