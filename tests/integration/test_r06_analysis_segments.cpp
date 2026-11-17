#include "egomotion/analysis/result_analyzer.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {
using namespace egomotion;
namespace fs=std::filesystem;
void require(bool condition,const char* message) {
    if(!condition)throw std::runtime_error(message);
}
DenseDoubleArray matrix(std::string name,std::size_t rows,std::size_t columns,
                        std::vector<double> values) {
    require(values.size()==rows*columns,"Synthetic MAT array size mismatch");
    DenseDoubleArray out;
    out.info.name=std::move(name);
    out.info.dimensions={rows,columns};
    out.info.original_class_type=6;
    out.info.original_data_type=9;
    out.info.data_bytes=rows*columns*sizeof(double);
    out.values_column_major=std::move(values);
    return out;
}
std::vector<double> sourceSequence(std::size_t columns) {
    std::vector<double> result(600*columns);
    for(std::size_t col=0;col<columns;++col)
        for(std::size_t row=0;row<600;++row)
            result[col*600+row]=1000.0*static_cast<double>(col)+
                                 static_cast<double>(row+1);
    return result;
}
std::vector<double> sourceRatios() {
    std::vector<double> values(600,0.03);
    constexpr std::array<std::pair<int,int>,3> intervals{{
        {120,179},{260,319},{400,459}
    }};
    for(const auto& pair:intervals)
        for(int frame=pair.first;frame<=pair.second;++frame)
            values[static_cast<std::size_t>(frame-1)]=0.005;
    // Source comparison threshold is 0.01; equality at frame 121
    // must not create a new transition.
    values[120]=0.01;
    return values;
}
struct SyntheticReader final:IResultReader {
    std::map<std::string,DenseDoubleArray> data;
    Result<std::vector<MatVariableInfo>,MatErrorCode>
    inspect(const fs::path&) const override {
        return Result<std::vector<MatVariableInfo>,MatErrorCode>::success({});
    }
    Result<DenseDoubleArray,MatErrorCode> readNamed(
        const fs::path& path,std::string_view name) const override {
        const auto found=data.find(path.string());
        if(found==data.end()){
            ErrorDetail detail;detail.stage=ErrorStage::Read;
            detail.source_path=path.string();detail.variable=std::string(name);
            detail.message="Synthetic historical MAT is missing";
            return Result<DenseDoubleArray,MatErrorCode>::failure({
                MatErrorCode::MissingFile,std::move(detail)});
        }
        return Result<DenseDoubleArray,MatErrorCode>::success(found->second);
    }
    Result<DenseDoubleArray,MatErrorCode>
    readFirstCompatible(const fs::path& path) const override {
        return readNamed(path,"synthetic");
    }
    void add(const fs::path& root,std::string name,
             std::size_t columns,std::vector<double> values) {
        const std::string filename=name+".mat";
        data.emplace((root/filename).string(),
                     matrix(std::move(name),600,columns,std::move(values)));
    }
};
void populate(SyntheticReader& reader,const fs::path& root,
              bool update_rates,std::vector<double> ratio) {
    const std::size_t columns=update_rates?4:1;
    reader.add(root,"allemittransfehler",columns,sourceSequence(columns));
    reader.add(root,"allemitrotfehler",columns,sourceSequence(columns));
    if(!update_rates){
        reader.add(root,"alleohnetransfehler",1,sourceSequence(1));
        reader.add(root,"alleohnerotfehler",1,sourceSequence(1));
    }
    reader.add(root,"rottransratio",1,std::move(ratio));
}
void verifyComparisonSegments() {
    const fs::path root("r06_synthetic_comparison");
    SyntheticReader reader;
    populate(reader,root,false,sourceRatios());
    analysis::MatResultAnalyzer analyzer;
    auto result=analyzer.analyze(AnalysisKind::AdaptiveComparison,root,reader);
    require(result.hasValue(),"Comparison source four-zone fixture must succeed");
    const auto& curves=result.value().curves;
    require(curves.size()==12,
            "Comparison requires eight rotation, two translation and two ratio curves");
    for(const auto& curve:curves)
        require(curve.helper_source_blob_shas.empty(),"Comparison must not claim a smoothing source");
    constexpr std::array<std::size_t,4> starts{{50,180,320,460}};
    constexpr std::array<std::size_t,4> ends{{120,260,400,550}};
    for(std::size_t zone=0;zone<4;++zone)
        for(std::size_t paired=0;paired<2;++paired) {
            const auto& c=curves[2*zone+paired];
            require(c.source_begin_matlab==starts[zone]&&
                    c.source_end_matlab==ends[zone],
                    "Comparison rotation segmentation deviates from original MATLAB");
            require(c.x.size()==ends[zone]-starts[zone]+1,
                    "Rotation intervals must use inclusive 1-based source frame indices");
            require(c.inputs.size()==2&&
                    c.inputs[1].variable=="rottransratio",
                    "Rotation segments require input error MAT and ratio MAT provenance");
        }
    for(std::size_t j=8;j<12;++j)
        require(curves[j].x.size()==501&&
                curves[j].source_begin_matlab==50&&
                curves[j].source_end_matlab==550,
                "Comparison translation and right-axis ratio remain full 501-frame curves");
}
void verifyRatesSmoothing() {
    const fs::path root("r06_synthetic_rates");
    SyntheticReader reader;
    populate(reader,root,true,sourceRatios());
    analysis::MatResultAnalyzer analyzer;
    auto result=analyzer.analyze(AnalysisKind::AdaptiveUpdateRates,root,reader);
    require(result.hasValue(),"UpdateRates source four-zone fixture must succeed");
    const auto& curves=result.value().curves;
    require(curves.size()==20,
            "UpdateRates requires sixteen segmented rotation and four translation curves");
    const auto& rotation=curves[1];
    for(const auto& curve:curves) {
        const std::vector<std::string> helpers=curve.smoothing_half_window?
            std::vector<std::string>{"eced66659205a6386ec84723e37a46517d06c017"}:
            std::vector<std::string>{};
        require(curve.helper_source_blob_shas==helpers,"Wrong per-curve smoothing source identity");
    }
    require(rotation.name=="rotation_period_2_segment_1"&&
            rotation.source_begin_matlab==52&&
            rotation.source_end_matlab==118&&
            rotation.x.size()==67&&rotation.y.size()==67&&
            rotation.smoothing_half_window==2,
            "Rotation local halfwidth=2 must trim two samples at each segment edge");
    require(rotation.smoothing_full.size()==71&&
            rotation.smoothing_full[0]==0&&rotation.smoothing_full[1]==0&&
            rotation.smoothing_full[69]==0&&rotation.smoothing_full[70]==0,
            "Segment-local smoothing must keep original zero-valued boundary");
    require(std::abs(rotation.y[0]-1052.0)<1e-12,
            "Rotational first smooth interior value differs from hand reference");
    const auto& translation=curves[19];
    require(translation.name=="translation_period_4"&&
            translation.x.size()==501&&translation.y.size()==501&&
            translation.source_begin_matlab==50&&
            translation.source_end_matlab==550&&
            translation.smoothing_half_window==10,
            "Translation is full-length smoothing followed by source 50:550 selection");
    require(translation.smoothing_full.size()==600&&
            translation.smoothing_full[9]==0&&
            translation.smoothing_full[590]==0&&
            std::abs(translation.y[0]-3050.0)<1e-12,
            "Full-length translation smoothing and zero edges differ from source");
}
void verifyRejections() {
    analysis::MatResultAnalyzer analyzer;
    SyntheticReader all_high;
    const fs::path root("r06_invalid_zone");
    populate(all_high,root,false,std::vector<double>(600,0.03));
    auto rejected=analyzer.analyze(AnalysisKind::AdaptiveComparison,root,all_high);
    require(!rejected.hasValue()&&
            rejected.error().code==AnalysisErrorCode::InvalidSegment,
            "Missing original four source regions must not silently be repaired");

    SyntheticReader nonfinite;
    const fs::path bad_root("r06_invalid_ratio");
    auto ratio=sourceRatios();
    ratio[49]=std::numeric_limits<double>::quiet_NaN();
    populate(nonfinite,bad_root,false,std::move(ratio));
    auto failed=analyzer.analyze(AnalysisKind::AdaptiveComparison,bad_root,nonfinite);
    require(!failed.hasValue()&&
            failed.error().code==AnalysisErrorCode::NonFiniteInput,
            "Non-finite ratio must fail before any threshold segmentation");
}
} // namespace

int main() {
    verifyComparisonSegments();
    verifyRatesSmoothing();
    verifyRejections();
}
