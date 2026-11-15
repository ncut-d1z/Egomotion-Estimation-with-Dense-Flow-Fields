#include "egomotion/analysis/result_analyzer.hpp"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {
using namespace egomotion;
void require(bool condition,const char* reason){
    if(!condition)throw std::runtime_error(reason);
}
DenseDoubleArray array(std::string name,std::size_t rows,std::size_t cols,
                       std::vector<double> numbers) {
    require(numbers.size()==rows*cols,"Invalid synthetic test data");
    DenseDoubleArray result;
    result.info.name=std::move(name);
    result.info.dimensions={rows,cols};
    result.info.original_class_type=6;
    result.info.original_data_type=9;
    result.info.data_bytes=rows*cols*sizeof(double);
    result.values_column_major=std::move(numbers);
    return result;
}
struct FakeReader final:IResultReader{
    std::map<std::string,DenseDoubleArray> data;
    Result<std::vector<MatVariableInfo>,MatErrorCode>
    inspect(const std::filesystem::path&)const override{
        return Result<std::vector<MatVariableInfo>,MatErrorCode>::success({});
    }
    Result<DenseDoubleArray,MatErrorCode> readNamed(
        const std::filesystem::path& path,std::string_view name)const override{
        const auto it=data.find(path.string());
        if(it==data.end()){
            ErrorDetail d;d.stage=ErrorStage::Read;d.source_path=path.string();
            d.variable=std::string(name);d.message="Synthetic MAT file does not exist";
            return Result<DenseDoubleArray,MatErrorCode>::failure(
                {MatErrorCode::MissingFile,std::move(d)});
        }
        return Result<DenseDoubleArray,MatErrorCode>::success(it->second);
    }
    Result<DenseDoubleArray,MatErrorCode> readFirstCompatible(
        const std::filesystem::path& path)const override{
        return readNamed(path,"unknown");
    }
    void add(const std::filesystem::path& root,const std::string& name,
             DenseDoubleArray a){
        data.emplace((root/(name+".mat")).string(),std::move(a));
    }
};
} // namespace

int main(int argc,char** argv){
    using namespace egomotion;
    namespace fs=std::filesystem;
    if(argc!=2)throw std::invalid_argument("R5 analysis needs one isolated build-tree root");
    const fs::path sandbox=fs::absolute(argv[1]);
    const fs::path bias=sandbox/"bias";
    fs::create_directories(bias/"1");
    fs::create_directories(bias/"2");
    FakeReader reader;
    reader.add(bias/"1","xwerte",array("xwerte",3,1,{6,24,96}));
    reader.add(bias/"2","xwerte",array("xwerte",3,1,{7,28,112}));
    for(int k=1;k<=2;++k){
        const fs::path dir=bias/std::to_string(k);
        for(int n=3;n<=6;++n){
            const std::string name="ywerte"+std::to_string(n);
            const std::vector<double> values=k==1?
                std::vector<double>{2,4,6,1,1,1,1,1,1}:
                std::vector<double>{4,8,10,1,1,1,1,1,1};
            reader.add(dir,name,array(name,3,3,values));
        }
    }
    analysis::MatResultAnalyzer analyzer;
    auto result=analyzer.analyze(AnalysisKind::Bias,bias,reader);
    require(result.hasValue(),"Bias K=2 analysis failed");
    const auto& curves=result.value().curves;
    require(curves.size()==13,"Bias must yield exactly 12 data curves + one reference");
    require(curves[0].x.size()==1&&curves[0].x[0]==112.0,
            "The last run's original xwerte must be preserved");
    require(curves[1].y.size()==1&&curves[1].y[0]==8.0,
            "K=2 ywerte3 column-1 mean must equal 8 at MATLAB row 3");
    require(curves[1].inputs.size()==3,
            "K=2 bias curve must trace two source y-MAT plus last-run x-MAT");
    require(!curves[1].inputs[0].mat_sha256,
            "Cannot invent a SHA256 digest when fake reader did not verify it");

    const fs::path rates=sandbox/"rates";
    reader.add(rates,"allemittransfehler",
               array("allemittransfehler",600,4,std::vector<double>(2400,1)));
    reader.add(rates,"allemitrotfehler",
               array("allemitrotfehler",600,4,std::vector<double>(2400,1)));
    auto missing=analyzer.analyze(AnalysisKind::AdaptiveUpdateRates,rates,reader);
    require(!missing.hasValue()&&
            missing.error().code==AnalysisErrorCode::MissingVariable,
            "UpdateRates without historical rottransratio must fail explicitly");
    bool observed_mat=false;
    for(const auto& frame:missing.error().detail.causes)
        observed_mat|=frame.domain=="MatErrorCode";
    require(observed_mat,"MAT underlying missing-file frame must remain in error chain");
}
