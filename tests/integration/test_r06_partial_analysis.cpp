// DEC-R6-04(B)的只读消费：声明、计数、结构端点、缺口和平滑传播。
#include "egomotion/analysis/result_analyzer.hpp"
#include "egomotion/analysis/dataset_json.hpp"

#include <array>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
using namespace egomotion;
using Data=Result<AnalysisDataset,AnalysisErrorCode>;
namespace fs=std::filesystem;
constexpr const char* marker="egomotion_metric_policy";
const fs::path root("declared-metric-fixture");
void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
double missing(){return std::numeric_limits<double>::quiet_NaN();}
DenseDoubleArray array(std::string name,std::size_t rows,std::size_t cols,std::vector<double> values) {
    DenseDoubleArray a;a.info.name=std::move(name);a.info.dimensions={rows,cols};
    a.info.original_class_type=6;a.info.original_data_type=9;
    a.info.data_bytes=values.size()*sizeof(double);a.values_column_major=std::move(values);
    a.source_mat_sha256=std::string(64,'a');return a;
}
struct Reader final:IResultReader {
    std::map<std::string,DenseDoubleArray> data;
    mutable std::vector<std::string> calls;
    std::optional<MatErrorCode> policy_failure;
    Result<std::vector<MatVariableInfo>,MatErrorCode> inspect(const fs::path&)const override {
        throw std::logic_error("Must read the declared named variable");
    }
    Result<DenseDoubleArray,MatErrorCode> readFirstCompatible(const fs::path&)const override {
        throw std::logic_error("Must not guess a partial metric variable");
    }
    Result<DenseDoubleArray,MatErrorCode> readNamed(const fs::path& path,std::string_view name)const override {
        calls.emplace_back(name);
        require(path==root/(std::string(name)+".mat"),"Exact source MAT path");
        const auto found=data.find(std::string(name));
        if((name==marker&&policy_failure)||found==data.end()) {
            ErrorDetail d;d.stage=ErrorStage::Read;d.source_path=path.string();
            d.variable=std::string(name);d.message="Synthetic named input unavailable";
            return Result<DenseDoubleArray,MatErrorCode>::failure({
                name==marker&&policy_failure?*policy_failure:MatErrorCode::MissingFile,std::move(d)});
        }
        return Result<DenseDoubleArray,MatErrorCode>::success(found->second);
    }
};
void addMetric(Reader& reader,const std::string& name,std::size_t rows,
                std::size_t cols,std::size_t repetitions,double value=10.0) {
    std::vector<double> values(rows*cols,value),counts(rows*cols,static_cast<double>(repetitions));
    for(std::size_t col=0;col<cols;++col) {
        values[col*rows]=values[(col+1)*rows-1]=0.0;
        counts[col*rows]=counts[(col+1)*rows-1]=0.0;
    }
    reader.data.emplace(name,array(name,rows,cols,std::move(values)));
    const auto validity="valid_"+name;
    reader.data.emplace(validity,array(validity,rows,cols,std::move(counts)));
}
void addRatio(Reader& reader) {
    addMetric(reader,"rottransratio",600,1,1,0.03);
    auto& values=reader.data.at("rottransratio").values_column_major;
    for(const auto& interval:std::array<std::pair<int,int>,3>{{{120,179},{260,319},{400,459}}})
        for(int i=interval.first;i<=interval.second;++i)values[static_cast<std::size_t>(i-1)]=0.005;
}
Reader inputs(AnalysisKind kind,bool auxiliary_ratio=false) {
    Reader reader;
    const bool init=kind==AnalysisKind::AdaptiveInitialization;
    const bool comparison=kind==AnalysisKind::AdaptiveComparison;
    const std::size_t rows=init?35:600,cols=(!init&&!comparison)?4:1,planned=comparison?8:1;
    reader.data.emplace(marker,array(marker,1,4,{1,1,1,static_cast<double>(planned)}));
    addMetric(reader,"allemittransfehler",rows,cols,planned);
    addMetric(reader,"allemitrotfehler",rows,cols,planned);
    if(init||comparison) {
        addMetric(reader,"alleohnetransfehler",rows,1,planned);
        addMetric(reader,"alleohnerotfehler",rows,1,planned);
    }else addMetric(reader,"akrot",600,1,1);
    if(comparison||auxiliary_ratio)addRatio(reader);
    return reader;
}
void removeObservation(Reader& reader,const std::string& name,std::size_t frame,
                        std::size_t column=0,double remaining=0.0) {
    const auto rows=reader.data.at(name).info.dimensions[0];
    const auto index=column*rows+frame-1;
    reader.data.at(name).values_column_major[index]=missing();
    reader.data.at("valid_"+name).values_column_major[index]=remaining;
}
Data analyze(Reader& reader,AnalysisKind kind) {
    return analysis::AdaptiveResultAnalyzer{}.analyze(kind,root,reader);
}
void initialization() {
    auto reader=inputs(AnalysisKind::AdaptiveInitialization);
    removeObservation(reader,"allemittransfehler",5);
    removeObservation(reader,"allemitrotfehler",20);
    const auto result=analyze(reader,AnalysisKind::AdaptiveInitialization);
    require(result.hasValue()&&reader.calls.front()==marker,"Policy must precede metric consumption");
    const auto& data=result.value();
    require(data.metric_policy=="adaptive-atan2-so3-geodesic-v1"&&data.planned_repetitions==1,
            "Dataset distinguishes revised metrics from historical directional errors");
    const auto& t=data.curves[0];const auto& r=data.curves[1];
    require(t.defined.size()==13&&t.defined[4]==0&&std::isnan(t.y[4]),"Declared translation gap");
    require(r.defined[19]==0&&std::isnan(r.y[19])&&r.metric_semantics=="rotation-so3-geodesic-degrees",
            "Declared rotation gap and revised rotation metric identity");
    require(r.defined.front()==1&&r.defined.back()==1&&r.y.front()==0.0&&r.y.back()==0.0&&
            r.structural_endpoint.front()==1&&r.structural_endpoint.back()==1,
            "Structural endpoint zero is distinguishable from a missing metric");
    require(t.inputs.size()==3&&t.inputs[1].role=="valid_repetitions"&&t.inputs[2].role=="metric_policy",
            "Curve traces values, exact repetition counts and policy marker");
    const auto json=analysis::datasetJson(data);
    require(json.hasValue()&&json.value().find("egomotion-analysis-v2")!=std::string::npos&&
        json.value().find("null")!=std::string::npos&&json.value().find("\"defined\":[true,true,true,true,false")!=std::string::npos,
        "Missing observations serialize as JSON null with explicit false validity");
    reader.data.erase(marker);
    const auto legacy=analyze(reader,AnalysisKind::AdaptiveInitialization);
    require(!legacy.hasValue()&&legacy.error().code==AnalysisErrorCode::NonFiniteInput,
            "The same undeclared NaN must remain invalid historical input");
}
void declarations() {
    for(const auto code:{MatErrorCode::InvalidMatHeader,MatErrorCode::PermissionDenied,
                         MatErrorCode::MissingVariable,MatErrorCode::CorruptCompressedData}) {
        auto reader=inputs(AnalysisKind::AdaptiveInitialization);reader.policy_failure=code;
        const auto result=analyze(reader,AnalysisKind::AdaptiveInitialization);
        require(!result.hasValue()&&reader.calls.size()==1,"A failed marker cannot fall back to historical rules");
        require(!result.error().detail.causes.empty(),"Retain the marker Reader cause");
    }
    for(unsigned bad=0;bad<5;++bad) {
        auto reader=inputs(AnalysisKind::AdaptiveInitialization);
        auto& policy=reader.data.at(marker);
        if(bad==0)policy.values_column_major[0]=2;
        if(bad==1)policy.values_column_major[1]=0;
        if(bad==2)policy.values_column_major[3]=8;
        if(bad==3)policy.info.dimensions={4,1};
        if(bad==4)policy.values_column_major[2]=missing();
        const auto result=analyze(reader,AnalysisKind::AdaptiveInitialization);
        require(!result.hasValue()&&reader.calls.size()==1,"Unknown marker shape, schema or metric is rejected first");
    }
    for(unsigned bad=0;bad<10;++bad) {
        auto reader=inputs(AnalysisKind::AdaptiveInitialization);
        auto& values=reader.data.at("allemittransfehler").values_column_major;
        auto& counts=reader.data.at("valid_allemittransfehler").values_column_major;
        switch(bad) {
        case 0:counts[4]=0.5;break;
        case 1:counts[4]=-1;break;
        case 2:counts[4]=2;break;
        case 3:counts[4]=0;break; // finite result without full fixed denominator
        case 4:values[4]=missing();break; // full count cannot accompany missing value
        case 5:counts[4]=0;values[4]=std::numeric_limits<double>::infinity();break;
        case 6:values[0]=missing();break;
        case 7:counts[0]=1;break;
        case 8:reader.data.erase("valid_allemittransfehler");break;
        case 9:reader.data.at("valid_allemittransfehler").info.dimensions={5,7};break;
        }
        require(!analyze(reader,AnalysisKind::AdaptiveInitialization).hasValue(),
                "Declared count/value/endpoint inconsistencies cannot become analysis curves");
    }
    auto legacy=inputs(AnalysisKind::AdaptiveInitialization);legacy.data.erase(marker);
    const auto historical=analyze(legacy,AnalysisKind::AdaptiveInitialization);
    require(historical.hasValue()&&historical.value().curves[0].defined.empty(),
            "Missing marker preserves finite historical analysis");
    const auto json=analysis::datasetJson(historical.value());
    require(json.hasValue()&&json.value().find("egomotion-analysis-v1")!=std::string::npos&&
        json.value().find("historical-rotation-direction-acos-degrees")!=std::string::npos,
        "Historical rotation angle remains explicitly distinct");
}
void comparison() {
    auto reader=inputs(AnalysisKind::AdaptiveComparison);
    removeObservation(reader,"allemittransfehler",100,0,7);
    const auto result=analyze(reader,AnalysisKind::AdaptiveComparison);
    require(result.hasValue()&&result.value().planned_repetitions==8,"Eight-run declaration with last-run ratio");
    const auto& translation=result.value().curves[9];
    require(translation.defined[50]==0&&std::isnan(translation.y[50]),
            "Seven valid repetitions cannot produce a reweighted eight-run mean");
    const auto& ratio=result.value().curves[10];
    require(ratio.defined[50]==1&&ratio.y[50]==0.03,"Ratio uses its single last-run validity count");
    removeObservation(reader,"rottransratio",151);
    const auto unsegmented=analyze(reader,AnalysisKind::AdaptiveComparison);
    require(!unsegmented.hasValue()&&unsegmented.error().code==AnalysisErrorCode::InvalidSegment,
            "A ratio gap in 50:550 must explicitly prevent source segmentation");
}
void smoothing() {
    auto absent=inputs(AnalysisKind::AdaptiveUpdateRates);
    const auto unavailable=analyze(absent,AnalysisKind::AdaptiveUpdateRates);
    require(!unavailable.hasValue()&&unavailable.error().code==AnalysisErrorCode::MissingVariable,
            "New UpdateRates cannot fabricate HistoricalOnly rottransratio");
    // 仅为分析器的独立合成输入提供声明完整的辅助ratio，不扩充Runner save集合。
    auto reader=inputs(AnalysisKind::AdaptiveUpdateRates,true);
    removeObservation(reader,"allemittransfehler",100,1);
    removeObservation(reader,"allemitrotfehler",80,1);
    const auto result=analyze(reader,AnalysisKind::AdaptiveUpdateRates);
    require(result.hasValue(),"Declared gaps may pass through fixed-window source smoothing");
    const auto& rotation=result.value().curves[1];
    const auto& translation=result.value().curves[17];
    for(std::size_t frame=98;frame<=102;++frame)
        require(translation.defined[frame-50]==0&&std::isnan(translation.y[frame-50])&&
                translation.smoothing_defined[frame-1]==0,
                "A missing source value invalidates every full window containing it");
    require(translation.defined[97-50]==1&&translation.y[97-50]==10.0&&
            translation.defined[103-50]==1&&translation.y[103-50]==10.0,
            "Adjacent complete windows retain the original fixed divisor");
    for(std::size_t frame=78;frame<=82;++frame)
        require(rotation.defined[frame-52]==0&&std::isnan(rotation.y[frame-52]),
                "Segment-local rotational smoothing preserves each missing-window gap");
    require(rotation.smoothing_full.front()==0.0&&rotation.smoothing_defined.front()==1,
            "Source smoothing padding remains a defined structural zero");
    require(analysis::datasetJson(result.value()).hasValue(),"Smoothing NaNs have explicit JSON masks");
}
void invalidDataset() {
    for(unsigned bad=0;bad<7;++bad) {
        AnalysisDataset data;AnalysisSeries curve;curve.x={1,2,3};curve.y={1,missing(),3};
        curve.defined={1,0,1};
        switch(bad) {
        case 0:curve.defined.clear();break;
        case 1:curve.defined.pop_back();break;
        case 2:curve.defined[1]=2;break;
        case 3:curve.defined[1]=1;break;
        case 4:curve.y[1]=0.0;break;
        case 5:curve.y[1]=std::numeric_limits<double>::infinity();break;
        case 6:curve.structural_endpoint={0,1,0};break;
        }
        data.curves.push_back(std::move(curve));
        require(!analysis::datasetJson(data).hasValue(),"JSON must reject undeclared/non-binary/inconsistent gaps");
    }
}
} // namespace
int main() {
    try {
        initialization();declarations();comparison();smoothing();invalidDataset();
        std::cout<<"PASS declared partial Adaptive analysis, fixed-denominator counts and gap propagation\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<error.what()<<'\n';return 1;
    }
}
