#include "egomotion/analysis/result_analyzer.hpp"
#include "analysis_guard.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <map>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace egomotion::analysis {
using detail::DatasetResult;
using detail::MetricArray;
using detail::MetricArrayResult;
using detail::error;
using detail::load;
using detail::column;
using detail::provenance;
using detail::series;
namespace {

constexpr const char* kSmoothingSourceBlob="eced66659205a6386ec84723e37a46517d06c017";
constexpr const char* kMetricPolicy="egomotion_metric_policy";

using PolicyResult=Result<std::optional<DenseDoubleArray>,AnalysisErrorCode>;
PolicyResult metricPolicy(IResultReader& reader,const std::filesystem::path& root,
                          std::size_t planned) {
    auto marker=load(reader,root,kMetricPolicy,1,4);
    if(!marker.hasValue()) {
        // 只有确实缺文件才是历史数据。损坏/错误版本不能回退掩盖。
        if(marker.error().code==AnalysisErrorCode::MissingResult)
            return PolicyResult::success(std::nullopt);
        return PolicyResult::failure(std::move(marker).error());
    }
    if(marker.value().values_column_major!=
       std::vector<double>{1.0,1.0,1.0,static_cast<double>(planned)}) {
        ErrorDetail d;d.stage=ErrorStage::Analyze;d.variable=kMetricPolicy;
        d.source_path=(root/(std::string(kMetricPolicy)+".mat")).string();
        d.message="Unknown metric policy/schema or planned repetition count";
        return PolicyResult::failure({AnalysisErrorCode::UnsupportedExperiment,std::move(d)});
    }
    return PolicyResult::success(std::move(marker).value());
}
MetricArrayResult loadMetric(IResultReader& reader,const std::filesystem::path& root,
    const std::string& name,std::size_t rows,std::size_t columns,
    std::size_t repetitions,const std::optional<DenseDoubleArray>& marker) {
    if(marker) {
        auto loaded=detail::loadDeclaredMetric(reader,root,name,rows,columns,repetitions);
        if(!loaded.hasValue())return loaded;
        auto result=std::move(loaded).value();
        result.policy_marker=*marker;
        return MetricArrayResult::success(std::move(result));
    }
    auto loaded=load(reader,root,name,rows,columns);
    if(!loaded.hasValue())return MetricArrayResult::failure(std::move(loaded).error());
    MetricArray result;result.array=std::move(loaded).value();
    return MetricArrayResult::success(std::move(result));
}
std::vector<unsigned char> maskColumn(const MetricArray& array,std::size_t col) {
    if(array.defined.empty())return {};
    const auto rows=array.array.info.dimensions.at(0);
    if(col>=array.array.info.dimensions.at(1))
        throw std::out_of_range("MAT metric validity column out of bounds");
    const auto first=array.defined.begin()+static_cast<std::ptrdiff_t>(col*rows);
    return {first,first+static_cast<std::ptrdiff_t>(rows)};
}

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

struct Smoothed {
    std::vector<double> values;
    std::vector<unsigned char> defined;
};
Smoothed smoothSource(const std::vector<double>& input,
                       const std::vector<unsigned char>& defined,std::size_t half) {
    if(half==0)return {input,defined};
    Smoothed out{std::vector<double>(input.size(),0.0),{}};
    if(!defined.empty())out.defined.assign(input.size(),1);
    if(input.size()<=2*half)return out; // original zeros at edges
    for(std::size_t i=half;i+half<input.size();++i) {
        double sum=0.0;
        bool complete=true;
        for(std::size_t j=i-half;j<=i+half;++j) {
            if(!defined.empty()&&!defined[j])complete=false;
            else sum+=input[j];
        }
        // 固定窗口缺一项就缺测；不按剩余样本重加权或收缩窗口。
        out.values[i]=complete?sum/static_cast<double>(2*half+1):
            std::numeric_limits<double>::quiet_NaN();
        if(!out.defined.empty())out.defined[i]=static_cast<unsigned char>(complete);
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
template<class T>
std::vector<T> select(const std::vector<T>& v,std::size_t first,std::size_t last) {
    if(first==0||last>v.size()||last<first)
        throw std::out_of_range("Invalid MATLAB source plot range");
    return std::vector<T>(v.begin()+static_cast<std::ptrdiff_t>(first-1),
                               v.begin()+static_cast<std::ptrdiff_t>(last));
}
AnalysisSeries makeCurve(const std::filesystem::path& root,
                         const MetricArray& source,
                         const std::string& name,const std::string& panel,
                         std::size_t from,std::size_t to,
                         std::vector<double> y,
                         const std::string& unit="angle error in degree",
                         std::size_t source_column=0,
                         std::vector<unsigned char> defined={}) {
    auto curve=series(name,panel,matlabRange(from,to),std::move(y),
                      {provenance(root,source.array,1,"y_values")},from,to);
    curve.y_unit=unit;
    curve.y_scale="log"; // original comparison uses log-y for angle/ratio
    const bool ratio=source.array.info.name=="rottransratio";
    const bool rotation=source.array.info.name.find("rotfehler")!=std::string::npos;
    curve.metric_semantics=ratio?"source-rotational-translational-flow-ratio":
        (rotation?"historical-rotation-direction-acos-degrees":
                  "historical-translation-direction-acos-degrees");
    if(source.policy_marker) {
        curve.defined=defined.empty()?select(maskColumn(source,source_column),from,to):
            std::move(defined);
        curve.structural_endpoint.assign(curve.y.size(),0);
        const auto rows=source.array.info.dimensions[0];
        for(std::size_t i=0;i<curve.y.size();++i)
            curve.structural_endpoint[i]=static_cast<unsigned char>(from+i==1||from+i==rows);
        curve.inputs.push_back(provenance(root,*source.valid_counts,1,"valid_repetitions"));
        curve.inputs.push_back(provenance(root,*source.policy_marker,1,"metric_policy"));
        if(!ratio) {
            curve.metric_semantics=rotation?"rotation-so3-geodesic-degrees":
                "translation-direction-atan2-degrees";
            curve.y_unit=rotation?"rotation SO(3) geodesic error in degree":
                "translation direction error in degree";
        }
    }
    return curve;
}
void appendSegmentationProvenance(AnalysisSeries& curve,const std::filesystem::path& root,
                                  const MetricArray& ratio) {
    curve.inputs.push_back(provenance(root,ratio.array,1,"segment_threshold"));
    if(ratio.valid_counts)
        curve.inputs.push_back(provenance(root,*ratio.valid_counts,1,"segment_threshold_validity"));
}
DatasetResult loadRatioFailure(MetricArrayResult& ratio,bool rates) {
    auto inner=ratio.error();
    if(rates&&inner.code==AnalysisErrorCode::MissingResult&&
       inner.detail.variable=="rottransratio") {
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

DatasetResult AdaptiveResultAnalyzer::analyze(AnalysisKind kind,const std::filesystem::path& root,
                       IResultReader& reader) const {
    return detail::guardAnalysis(root,[&]{return analyzeSource(kind,root,reader);});
}

DatasetResult AdaptiveResultAnalyzer::analyzeSource(AnalysisKind kind,const std::filesystem::path& root,
                       IResultReader& reader) const {
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
    const std::size_t planned=comparison?8:1;
    auto policy=metricPolicy(reader,root,planned);
    if(!policy.hasValue())return DatasetResult::failure(std::move(policy).error());
    const auto& marker=policy.value();
    auto trans=loadMetric(reader,root,"allemittransfehler",rows,columns,planned,marker);
    if(!trans.hasValue())return DatasetResult::failure(std::move(trans).error());
    auto rot=loadMetric(reader,root,"allemitrotfehler",rows,columns,planned,marker);
    if(!rot.hasValue())return DatasetResult::failure(std::move(rot).error());

    AnalysisDataset out;
    out.metric_policy=marker?"adaptive-atan2-so3-geodesic-v1":
        "historical-adaptive-direction-acos";
    out.planned_repetitions=marker?planned:0;
    if(init) {
        // Other files are read as required by the original init graph, but
        // only the two 'mit' variables are actually plotted.
        auto without_t=loadMetric(reader,root,"alleohnetransfehler",35,1,1,marker);
        if(!without_t.hasValue())return DatasetResult::failure(std::move(without_t).error());
        auto without_r=loadMetric(reader,root,"alleohnerotfehler",35,1,1,marker);
        if(!without_r.hasValue())return DatasetResult::failure(std::move(without_r).error());
        out.source_analysis_blob="8816675ef5a4eb6767919df36ee11f95de36dac6";
        auto t=makeCurve(root,trans.value(),"mittrans_1_to_13",
                         "initialisation-semilogy",1,13,
                         select(column(trans.value().array,0),1,13));
        t.y_scale="log";
        auto r=makeCurve(root,rot.value(),"mitrot_all",
                         "initialisation-semilogy",1,35,
                         column(rot.value().array,0));
        out.curves.push_back(std::move(t));
        out.curves.push_back(std::move(r));
        return DatasetResult::success(std::move(out));
    }

    if(rates&&marker) {
        // akrot不是绘图曲线，但属于新输出合同，不能跳过其count/NaN核验。
        auto angles=loadMetric(reader,root,"akrot",600,1,1,marker);
        if(!angles.hasValue())return DatasetResult::failure(std::move(angles).error());
    }
    // Comparison的ratio只取最后一轮；它的有效计数始终是0/1。
    auto ratio=loadMetric(reader,root,"rottransratio",600,1,1,marker);
    if(!ratio.hasValue())return loadRatioFailure(ratio,rates);
    const auto ratios=column(ratio.value().array,0);
    if(marker) {
        const auto defined=maskColumn(ratio.value(),0);
        for(std::size_t matlab=50;matlab<=550;++matlab)
            if(!defined[matlab-1])
                return error(AnalysisErrorCode::InvalidSegment,
                    "Declared missing rottransratio prevents the original four-region segmentation",
                    root/"rottransratio.mat","rottransratio");
    }
    const auto threshold=comparison?0.01:0.02;
    auto segments=fourSourceSegments(ratios,threshold);
    if(!segments.hasValue())
        return DatasetResult::failure(std::move(segments).error());
    out.source_analysis_blob=comparison?
        "3924038f0ccd70de5ea7b60c589afe166da9c8fb":
        "67cb613328dec10105a6c383ce17025c4901e4c9";

    if(comparison) {
        auto without_t=loadMetric(reader,root,"alleohnetransfehler",600,1,8,marker);
        if(!without_t.hasValue())return DatasetResult::failure(std::move(without_t).error());
        auto without_r=loadMetric(reader,root,"alleohnerotfehler",600,1,8,marker);
        if(!without_r.hasValue())return DatasetResult::failure(std::move(without_r).error());
        const auto with_rotation=column(rot.value().array,0);
        const auto without_rotation=column(without_r.value().array,0);
        for(std::size_t i=0;i<segments.value().size();++i) {
            const auto& s=segments.value()[i];
            auto a=makeCurve(root,without_r.value(),
                             "without_rotation_segment_"+std::to_string(i+1),
                             "rotation",s.begin,s.end,
                             select(without_rotation,s.begin,s.end));
            appendSegmentationProvenance(a,root,ratio.value());
            auto b=makeCurve(root,rot.value(),
                             "with_rotation_segment_"+std::to_string(i+1),
                             "rotation",s.begin,s.end,
                             select(with_rotation,s.begin,s.end));
            appendSegmentationProvenance(b,root,ratio.value());
            out.curves.push_back(std::move(a));
            out.curves.push_back(std::move(b));
        }
        out.curves.push_back(makeCurve(root,without_t.value(),
            "without_translation","translation",50,550,
            select(column(without_t.value().array,0),50,550)));
        out.curves.push_back(makeCurve(root,trans.value(),
            "with_translation","translation",50,550,
            select(column(trans.value().array,0),50,550)));
        for(const std::string panel:{"rotation","translation"}) {
            auto ratio_curve=makeCurve(root,ratio.value(),
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
            const auto full_column=column(rot.value().array,col);
            const auto sub=select(full_column,s.begin,s.end);
            const auto full_mask=maskColumn(rot.value(),col);
            const auto sub_mask=full_mask.empty()?std::vector<unsigned char>{}:
                select(full_mask,s.begin,s.end);
            const std::size_t half=halfwindows[col];
            if(half>0 && sub.size()<=2*half)
                return error(AnalysisErrorCode::InvalidSegment,
                             "Rotation segment too short for original smoothing window");
            const auto smoothed=smoothSource(sub,sub_mask,half);
            const auto from=s.begin+half,to=s.end-half;
            const auto plotted=half?select(smoothed.values,half+1,smoothed.values.size()-half):
                                    smoothed.values;
            const auto plotted_mask=smoothed.defined.empty()?std::vector<unsigned char>{}:
                select(smoothed.defined,half+1,smoothed.defined.size()-half);
            auto curve=makeCurve(root,rot.value(),
                "rotation_period_"+std::to_string(col+1)+"_segment_"+
                    std::to_string(z+1),"rotation",from,to,plotted,
                    "angle error in degree",col,plotted_mask);
            appendSegmentationProvenance(curve,root,ratio.value());
            // many_parameter.m 使用 plot；只有 Comparison/Initialization 为 log-y。
            curve.y_scale="linear";
            curve.smoothing_half_window=half;
            curve.smoothing_full=smoothed.values;
            curve.smoothing_defined=smoothed.defined;
            if(half>0)curve.helper_source_blob_shas.push_back(kSmoothingSourceBlob);
            out.curves.push_back(std::move(curve));
        }
    }
    for(std::size_t col=0;col<4;++col){
        const auto all=column(trans.value().array,col);
        const std::size_t half=halfwindows[col];
        const auto smoothed=smoothSource(all,maskColumn(trans.value(),col),half);
        const auto plotted_mask=smoothed.defined.empty()?std::vector<unsigned char>{}:
            select(smoothed.defined,50,550);
        auto curve=makeCurve(root,trans.value(),
            "translation_period_"+std::to_string(col+1),
            "translation",50,550,select(smoothed.values,50,550),
            "angle error in degree",col,plotted_mask);
        curve.y_scale="linear";
        curve.smoothing_half_window=half;
        curve.smoothing_full=smoothed.values;
        curve.smoothing_defined=smoothed.defined;
        if(half>0)curve.helper_source_blob_shas.push_back(kSmoothingSourceBlob);
        out.curves.push_back(std::move(curve));
    }
    return DatasetResult::success(std::move(out));
}

} // namespace egomotion::analysis
