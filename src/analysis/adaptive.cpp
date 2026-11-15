#include "reader_helpers.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace egomotion::analysis::detail {
namespace {

struct Segment {
    std::size_t begin=0; // source MATLAB 1-based, inclusive
    std::size_t end=0;
};
using Segments=Result<std::array<Segment,4>,AnalysisErrorCode>;

Segments fourSourceSegments(const std::vector<double>& ratio,double threshold) {
    if(ratio.size()!=600) {
        ErrorDetail d;d.stage=ErrorStage::Decode;
        d.message="Source ratio must be 600-by-1";
        return Segments::failure({AnalysisErrorCode::ShapeMismatch,std::move(d)});
    }
    std::array<Segment,4> result{};
    result[0].begin=50;
    std::size_t j=0;
    bool zone=false;
    // Two independent if-branches, strict comparisons only, just as .m.
    for(std::size_t matlab=50;matlab<=550;++matlab){
        const double x=ratio[matlab-1];
        if(x<threshold&&!zone) {
            if(j>=result.size()) {
                ErrorDetail d;d.stage=ErrorStage::Analyze;
                d.message="Too many threshold crossings for four source regions";
                return Segments::failure({AnalysisErrorCode::InvalidSegment,std::move(d)});
            }
            result[j].end=matlab;
            ++j;zone=true;
        }
        if(x>threshold&&zone) {
            if(j>=result.size()) {
                ErrorDetail d;d.stage=ErrorStage::Analyze;
                d.message="Threshold start would exceed four source regions";
                return Segments::failure({AnalysisErrorCode::InvalidSegment,std::move(d)});
            }
            result[j].begin=matlab;
            zone=false;
        }
    }
    result[3].end=550; // original unconditional overwrite
    for(const auto& s:result)
        if(s.begin<50||s.end>550||s.begin>s.end||s.end==0){
            ErrorDetail d;d.stage=ErrorStage::Analyze;
            d.message="Original four-region boundaries are incomplete or invalid";
            return Segments::failure({AnalysisErrorCode::InvalidSegment,std::move(d)});
        }
    return Segments::success(std::move(result));
}

std::vector<double> smoothSource(const std::vector<double>& input,
                                 std::size_t half) {
    std::vector<double> out(input.size(),0.0);
    if(half==0)return input;
    if(input.size()<=2*half)return out; // original zeros at edges
    for(std::size_t i=half;i+half<input.size();++i) {
        double sum=0.0;
        for(std::size_t j=i-half;j<=i+half;++j)sum+=input[j];
        out[i]=sum/static_cast<double>(2*half+1);
    }
    return out;
}

std::vector<double> matlabRange(std::size_t first,std::size_t last) {
    std::vector<double> out;
    if(last<first)return out;
    out.reserve(last-first+1);
    for(std::size_t i=first;i<=last;++i)out.push_back(static_cast<double>(i));
    return out;
}
std::vector<double> select(const std::vector<double>& v,
                           std::size_t first,std::size_t last) {
    if(first==0||last>v.size()||last<first)
        throw std::out_of_range("Invalid MATLAB source plot range");
    return std::vector<double>(v.begin()+static_cast<std::ptrdiff_t>(first-1),
                               v.begin()+static_cast<std::ptrdiff_t>(last));
}
AnalysisSeries makeCurve(const std::filesystem::path& root,
                         const std::string& variable,
                         const std::string& name,const std::string& panel,
                         std::size_t from,std::size_t to,
                         std::vector<double> y,
                         const std::string& unit="angle error in degree") {
    auto curve=series(name,panel,matlabRange(from,to),std::move(y),
                      {provenance(root,variable,1,"y_values")},from,to);
    curve.y_unit=unit;
    curve.y_scale="log"; // original comparison uses log-y for angle/ratio
    return curve;
}
DatasetResult loadRatioFailure(ArrayResult& ratio) {
    auto inner=ratio.error();
    if(inner.code==AnalysisErrorCode::MissingResult) {
        ErrorDetail detail;
        detail.stage=ErrorStage::Analyze;
        detail.variable="rottransratio";
        detail.message="HistoricalOnly rottransratio is required by the source analysis but not saved by the current UpdateRates runner";
        return DatasetResult::failure(
            wrapFailure(AnalysisErrorCode::MissingVariable,std::move(detail),inner));
    }
    return DatasetResult::failure(std::move(ratio).error());
}
} // namespace

DatasetResult adaptive(AnalysisKind kind,const std::filesystem::path& root,
                       IResultReader& reader) {
    if(kind==AnalysisKind::Bias)
        return error(AnalysisErrorCode::UnsupportedExperiment,
                     "Adaptive analyzer received Bias kind");
    const bool init=kind==AnalysisKind::AdaptiveInitialization;
    const bool comparison=kind==AnalysisKind::AdaptiveComparison;
    const bool rates=kind==AnalysisKind::AdaptiveUpdateRates;
    if(!init&&!comparison&&!rates)
        return error(AnalysisErrorCode::UnsupportedExperiment,
                     "Unknown Adaptive analysis entry");
    const std::size_t rows=init?35:600;
    const std::size_t columns=rates?4:1;
    auto trans=load(reader,root,"allemittransfehler",rows,columns);
    if(!trans.hasValue())return DatasetResult::failure(std::move(trans).error());
    auto rot=load(reader,root,"allemitrotfehler",rows,columns);
    if(!rot.hasValue())return DatasetResult::failure(std::move(rot).error());

    AnalysisDataset out;
    if(init) {
        // Other files are read as required by the original init graph, but
        // only the two 'mit' variables are actually plotted.
        auto without_t=load(reader,root,"alleohnetransfehler",35,1);
        if(!without_t.hasValue())return DatasetResult::failure(std::move(without_t).error());
        auto without_r=load(reader,root,"alleohnerotfehler",35,1);
        if(!without_r.hasValue())return DatasetResult::failure(std::move(without_r).error());
        out.source_analysis_blob="8816675ef5a4eb6767919df36ee11f95de36dac6";
        auto t=makeCurve(root,"allemittransfehler","mittrans_1_to_13",
                         "initialisation-semilogy",1,13,
                         select(column(trans.value(),0),1,13));
        t.y_scale="log";
        auto r=makeCurve(root,"allemitrotfehler","mitrot_all",
                         "initialisation-semilogy",1,35,
                         column(rot.value(),0));
        out.curves.push_back(std::move(t));
        out.curves.push_back(std::move(r));
        return DatasetResult::success(std::move(out));
    }

    auto ratio=load(reader,root,"rottransratio",600,1);
    if(!ratio.hasValue())return loadRatioFailure(ratio);
    const auto ratios=column(ratio.value(),0);
    const auto threshold=comparison?0.01:0.02;
    auto segments=fourSourceSegments(ratios,threshold);
    if(!segments.hasValue())
        return DatasetResult::failure(std::move(segments).error());
    out.source_analysis_blob=comparison?
        "3924038f0ccd70de5ea7b60c589afe166da9c8fb":
        "67cb613328dec10105a6c383ce17025c4901e4c9";

    if(comparison) {
        auto without_t=load(reader,root,"alleohnetransfehler",600,1);
        if(!without_t.hasValue())return DatasetResult::failure(std::move(without_t).error());
        auto without_r=load(reader,root,"alleohnerotfehler",600,1);
        if(!without_r.hasValue())return DatasetResult::failure(std::move(without_r).error());
        const auto with_rotation=column(rot.value(),0);
        const auto without_rotation=column(without_r.value(),0);
        for(std::size_t i=0;i<segments.value().size();++i) {
            const auto& s=segments.value()[i];
            auto a=makeCurve(root,"alleohnerotfehler",
                             "without_rotation_segment_"+std::to_string(i+1),
                             "rotation",s.begin,s.end,
                             select(without_rotation,s.begin,s.end));
            a.inputs.push_back(provenance(root,"rottransratio",1,"segment_threshold"));
            auto b=makeCurve(root,"allemitrotfehler",
                             "with_rotation_segment_"+std::to_string(i+1),
                             "rotation",s.begin,s.end,
                             select(with_rotation,s.begin,s.end));
            b.inputs.push_back(provenance(root,"rottransratio",1,"segment_threshold"));
            out.curves.push_back(std::move(a));
            out.curves.push_back(std::move(b));
        }
        out.curves.push_back(makeCurve(root,"alleohnetransfehler",
            "without_translation","translation",50,550,
            select(column(without_t.value(),0),50,550)));
        out.curves.push_back(makeCurve(root,"allemittransfehler",
            "with_translation","translation",50,550,
            select(column(trans.value(),0),50,550)));
        for(const std::string panel:{"rotation","translation"}) {
            auto ratio_curve=makeCurve(root,"rottransratio",
                "ratio_"+panel,panel,50,550,select(ratios,50,550),"rot/trans flow ratio");
            out.curves.push_back(std::move(ratio_curve));
        }
        return DatasetResult::success(std::move(out));
    }

    // UpdateRates:
    // ROTATION uses separate segment-local windows, while TRANSLATION
    // smooths all 600 rows before selecting 50:550 (no segmentation).
    constexpr std::array<std::size_t,4> halfwindows{{0,2,5,10}};
    for(std::size_t z=0;z<segments.value().size();++z){
        const auto& s=segments.value()[z];
        for(std::size_t col=0;col<4;++col){
            const auto full_column=column(rot.value(),col);
            const auto sub=select(full_column,s.begin,s.end);
            const std::size_t half=halfwindows[col];
            if(half>0 && sub.size()<=2*half)
                return error(AnalysisErrorCode::InvalidSegment,
                             "Rotation segment too short for original smoothing window");
            const auto smoothed=half?smoothSource(sub,half):sub;
            const auto from=s.begin+half,to=s.end-half;
            const auto plotted=half?select(smoothed,half+1,smoothed.size()-half):
                                    smoothed;
            auto curve=makeCurve(root,"allemitrotfehler",
                "rotation_period_"+std::to_string(col+1)+"_segment_"+
                    std::to_string(z+1),"rotation",from,to,plotted);
            curve.inputs.push_back(provenance(root,"rottransratio",1,"segment_threshold"));
            curve.smoothing_half_window=half;
            curve.smoothing_full=smoothed;
            out.curves.push_back(std::move(curve));
        }
    }
    for(std::size_t col=0;col<4;++col){
        const auto all=column(trans.value(),col);
        const std::size_t half=halfwindows[col];
        const auto smoothed=half?smoothSource(all,half):all;
        auto curve=makeCurve(root,"allemittransfehler",
            "translation_period_"+std::to_string(col+1),
            "translation",50,550,select(smoothed,50,550));
        curve.smoothing_half_window=half;
        curve.smoothing_full=smoothed;
        out.curves.push_back(std::move(curve));
    }
    return DatasetResult::success(std::move(out));
}

} // namespace egomotion::analysis::detail
