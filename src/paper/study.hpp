#pragma once

#include "egomotion/analysis/paper_figures.hpp"
#include "egomotion/experiments/adaptive_runner.hpp"
#include "egomotion/illustrations/illustration.hpp"

#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <utility>

namespace egomotion::paper {

struct BiasTrial {
    bool occluded;
    std::uint32_t seed;
    std::vector<experiments::BiasPaperSample> samples;
};
struct AdaptiveCondition {
    std::string name;
    std::size_t repetitions;
    std::vector<experiments::AdaptivePaperSample> samples;
    std::size_t attempted=0,completed=0;
    bool succeeded=false;

    // 八轮在一个 RNG 流中顺序执行，后续轮次是否开始只能由 Runner 的事件确定。
    template<class Run>
    auto collect(Run&& run) {
        auto result=run([this](experiments::AdaptiveProgressEvent event) {
            if(event==experiments::AdaptiveProgressEvent::RunStarted)++attempted;
            if(event==experiments::AdaptiveProgressEvent::RunCompleted)++completed;
        },[this](const experiments::AdaptivePaperSample& sample){samples.push_back(sample);});
        succeeded=result.hasValue();
        return result;
    }
};
struct Study {
    std::vector<BiasTrial> bias;
    std::vector<AdaptiveCondition> adaptive;
    std::vector<illustrations::Data> geometry;
};

inline std::ostringstream table() {
    std::ostringstream stream;
    stream.exceptions(std::ios::failbit|std::ios::badbit);
    stream.imbue(std::locale::classic());
    stream<<std::setprecision(std::numeric_limits<double>::max_digits10);
    return stream;
}

// 内部异常只用于把嵌套发布/分析错误送回公共 Result，保留原始原因链。
struct PaperFailure { Failure<ExperimentErrorCode> reason; };
template<class Code>
[[noreturn]] void propagate(ExperimentErrorCode code,ErrorStage stage,
                            std::string message,const Failure<Code>& child) {
    ErrorDetail detail;detail.stage=stage;detail.message=std::move(message);
    throw PaperFailure{wrapFailure(code,std::move(detail),child)};
}
[[noreturn]] inline void invalid(std::string message) {
    ErrorDetail detail;detail.stage=ErrorStage::Analyze;detail.message=std::move(message);
    throw PaperFailure{{ExperimentErrorCode::DataMismatch,std::move(detail)}};
}
struct Files {
    ITextPublisher& publisher;
    std::vector<std::string> names;
    void save(std::string name,const std::string& contents) {
        auto written=publisher.publishNew(name,contents);
        if(!written.hasValue())propagate(ExperimentErrorCode::OutputFailed,ErrorStage::Output,
                                         "Cannot publish paper evidence "+name,written.error());
        names.push_back(std::move(name));
    }
};

void publishFigures(const FigureConfig&,const Study&,Files&);
std::string caseCountsJson(std::size_t attempted,std::size_t failed,const Study&);

} // namespace egomotion::paper
