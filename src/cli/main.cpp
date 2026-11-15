#include "egomotion/cli/command.hpp"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    std::vector<std::string> args;
    args.reserve(argc > 0 ? static_cast<std::size_t>(argc - 1) : 0);
    for (int i = 1; i < argc; ++i) args.emplace_back(argv[i]);
    // R0/R1: no real MAT, analysis or experiment service is constructed yet.
    // Invalid/unsupported requests are explicit failures, not success stubs.
    const egomotion::cli::ApplicationCompositionRoot app;
    return egomotion::cli::run(args, std::cout, std::cerr, app);
}
