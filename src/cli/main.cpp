#include "egomotion/cli/command.hpp"
#include "egomotion/io/mat_result_reader.hpp"
#include "egomotion/analysis/result_analyzer.hpp"
#include "egomotion/analysis/plot.hpp"
#include "egomotion/experiments/production_runner.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

int main(int argc,char** argv) {
    std::vector<std::string> args;
    args.reserve(argc>0?static_cast<std::size_t>(argc-1):0);
    for(int i=1;i<argc;++i)args.emplace_back(argv[i]);

    // R5: all services own their own internal state. Plot is optional;
    // changing the configured plot backend must not affect analyze.
    const egomotion::io::MatResultReader reader;
    egomotion::analysis::MatResultAnalyzer analyzer;
    egomotion::analysis::SvgPlotExporter plotter;
    egomotion::experiments::ProductionExperimentRunner runner;
    // The application must start from the trusted repository checkout,
    // identified by the presence of origin/; no output-root CLI argument.
    const egomotion::cli::ApplicationCompositionRoot app(
        const_cast<egomotion::io::MatResultReader*>(&reader),
        &runner,&analyzer,&plotter,std::filesystem::current_path());
    return egomotion::cli::run(args,std::cout,std::cerr,app);
}
