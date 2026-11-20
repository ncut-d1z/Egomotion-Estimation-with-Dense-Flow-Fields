#include "application_main.hpp"
#include "stream_output.hpp"
#include "egomotion/cli/command.hpp"
#include "egomotion/io/mat_result_reader.hpp"
#include "egomotion/analysis/result_analyzer.hpp"
#include "egomotion/analysis/plot.hpp"
#include "egomotion/experiments/production_runner.hpp"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

int egomotionApplication(int argc,char** argv,egomotion::cli::IInvocationObserver* observer) try {
    std::vector<std::string> args;
    args.reserve(argc>0?static_cast<std::size_t>(argc-1):0);
    for(int i=1;i<argc;++i)args.emplace_back(argv[i]);

    // R5: all services own their own internal state. Plot is optional;
    // changing the configured plot backend must not affect analyze.
    egomotion::io::MatResultReader reader(observer);
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
        &runner,&analyzer,plotter,std::filesystem::current_path(),illustrator,observer);
    return egomotion::cli::run(args,std::cout,std::cerr,app);
} catch (const std::exception& error) {
    // 参数构造和实际装配也可能在cli::run的异常边界之前失败。
    return egomotion::cli::streams::outputFailure(std::cerr,error.what());
}
