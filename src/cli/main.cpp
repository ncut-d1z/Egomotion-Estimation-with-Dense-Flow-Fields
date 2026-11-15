#include "egomotion/cli/command.hpp"
#include "egomotion/io/mat_result_reader.hpp"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    std::vector<std::string> args;
    args.reserve(argc > 0 ? static_cast<std::size_t>(argc - 1) : 0);
    for (int i = 1; i < argc; ++i) args.emplace_back(argv[i]);
    // R2 composes the real MAT reader; the CLI itself does no MAT decoding.
    // Experiment and analysis backends remain explicit R3-R5 placeholders.
    const egomotion::io::MatResultReader reader;
    const egomotion::cli::ApplicationCompositionRoot app(&reader);
    return egomotion::cli::run(args, std::cout, std::cerr, app);
}
