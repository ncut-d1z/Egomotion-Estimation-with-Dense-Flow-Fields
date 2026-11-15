#include "egomotion/cli/command.hpp"
#include "egomotion/io/mat_result_reader.hpp"

#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
void require(bool value, const std::string& reason) {
    if (!value) throw std::runtime_error("R2 MAT regression: " + reason);
}
}  // namespace

int main(int argc, char** argv) {
    using namespace egomotion;
    if (argc != 3) throw std::invalid_argument("Expected two read-only MAT fixtures");
    const std::filesystem::path source(argv[1]);
    const std::filesystem::path matrix_source(argv[2]);
    const io::MatResultReader reader;

    auto inspection = reader.inspect(source);
    require(inspection.hasValue(), "metadata-only inspection failed");
    require(inspection.value().size() == 1, "single-variable archive changed");
    const auto& meta = inspection.value().front();
    require(meta.name == "xwerte", "MAT variable name changed");
    require(meta.original_class_type == 6 && meta.original_data_type == 9,
            "double class/data type changed");
    require(meta.dimensions == std::vector<std::size_t>({10, 1}),
            "first fixture dimensions changed");

    auto metadata_scan = reader.scanFile(source, io::ScanMode::MetadataOnly);
    require(metadata_scan.hasValue() && metadata_scan.value().variables.size() == 1,
            "metadata scan failed");
    require(metadata_scan.value().variables.front().status == "METADATA_OK",
            "metadata scan accidentally attempted numeric decode");

    auto data_scan = reader.scanFile(source, io::ScanMode::VerifyDecoded);
    require(data_scan.hasValue() && data_scan.value().variables.size() == 1,
            "full dictionary scan failed");
    require(data_scan.value().variables.front().status == "READ_OK",
            "legacy READ_OK full decode contract changed");

    auto named = reader.readNamed(source, "xwerte");
    require(named.hasValue(), "readNamed failed");
    const auto before = named.value().values_column_major;
    require(named.value().info.dimensions == meta.dimensions, "metadata/value mismatch");
    require(before.size() == 10 && named.value().info.data_bytes == 80,
            "MAT numeric length or bytes differ");

    auto no_extension = source;
    no_extension.replace_extension();
    auto compatible = reader.readFirstCompatible(no_extension);
    require(compatible.hasValue(), "compat extension handling failed");
    require(compatible.value().values_column_major == before,
            "compat data differs from strict named data");
    auto missing = reader.readNamed(source, "does_not_exist");
    require(!missing.hasValue() && missing.error().code == MatErrorCode::MissingVariable,
            "missing variable did not return an explicit error");

    auto matrix = reader.readNamed(matrix_source, "ywerte3");
    require(matrix.hasValue(), "read a 10x3 matrix failed");
    require(matrix.value().info.dimensions == std::vector<std::size_t>({10, 3}),
            "unexpected ywerte3 shape");
    require(matrix.value().values_column_major.size() == 30,
            "numeric payload was not fully copied");

    // Files can be closed before these owning vectors are accessed.
    require(named.value().values_column_major == before,
            "MAT payload must outlive matio's matvar_t");

    std::ostringstream out, error;
    cli::ApplicationCompositionRoot application(&reader);
    const int exit_code = cli::run({"inspect-mat", "--file", source.string()},
                                    out, error, application);
    require(exit_code == 0, "real CLI inspection failed");
    require(out.str().find("variable=xwerte") != std::string::npos &&
            out.str().find("dims=10x1") != std::string::npos,
            "metadata fields not surfaced");
    require(out.str().find("values=") == std::string::npos &&
            out.str().find("numeric_payload=") == std::string::npos,
            "inspect-mat must not print numeric payload");

    std::ostringstream filtered_out, filtered_error;
    require(cli::run({"inspect-mat", "--file", source.string(), "--variable", "missing"},
                     filtered_out, filtered_error, application) == 3,
            "CLI must report MissingVariable");
    require(filtered_out.str().empty(), "missing variable cannot output fake values");
    require(filtered_error.str().find("MissingVariable") != std::string::npos,
            "missing variable must retain the code identity");
}
