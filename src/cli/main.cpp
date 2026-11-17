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
    egomotion::io::MatResultReader reader;
    egomotion::analysis::MatResultAnalyzer analyzer;
    egomotion::IPlotExporter* plotter=nullptr;
    egomotion::illustrations::IExporter* illustrator=nullptr;
#if EGOMOTION_ENABLE_SVG
    egomotion::analysis::SvgPlotExporter svg_plotter;
    egomotion::illustrations::SvgExporter svg_illustrator;
    plotter=&svg_plotter;
    illustrator=&svg_illustrator;
#endif
    egomotion::experiments::ProductionExperimentRunner runner;
    // The application must start from the trusted repository checkout,
    // identified by the presence of origin/; no output-root CLI argument.
    const egomotion::cli::ApplicationCompositionRoot app(
        &reader,
        &runner,&analyzer,plotter,std::filesystem::current_path(),illustrator);
    return egomotion::cli::run(args,std::cout,std::cerr,app);
}
