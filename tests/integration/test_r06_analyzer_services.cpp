// P9.8：具体分析组件、借用Reader与只消费结果的Exporter；合成输入不是科学实验。
#include "egomotion/analysis/result_analyzer.hpp"
#include "egomotion/analysis/dataset_json.hpp"
#include "egomotion/analysis/plot.hpp"

#include <array>
#include <filesystem>
#include <iostream>
#include <map>
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace {
using namespace egomotion;
namespace fs=std::filesystem;
static_assert(std::is_empty_v<analysis::BiasResultAnalyzer> &&
              std::is_empty_v<analysis::AdaptiveResultAnalyzer>,"Analyzers hold no cross-run data");
std::size_t checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
using Data=Result<AnalysisDataset,AnalysisErrorCode>;
const std::array<AnalysisKind,4> kinds{{AnalysisKind::Bias,AnalysisKind::AdaptiveInitialization,
    AnalysisKind::AdaptiveComparison,AnalysisKind::AdaptiveUpdateRates}};
enum class Fault {None,Failure,Allocation,Length,Filesystem,Range};
struct Reader final:IResultReader {
    std::map<std::string,DenseDoubleArray> data;
    mutable std::vector<std::string> calls;Fault fault=Fault::None;std::size_t fail_at=1;
    Result<std::vector<MatVariableInfo>,MatErrorCode> inspect(const fs::path&) const override {
        throw std::logic_error("Analyzer must read named numerical inputs");
    }
    Result<DenseDoubleArray,MatErrorCode> readFirstCompatible(const fs::path&) const override {
        throw std::logic_error("Analyzer must not guess variables");
    }
    Result<DenseDoubleArray,MatErrorCode> readNamed(const fs::path& path,std::string_view name) const override {
        calls.push_back(path.string());
        if(calls.size()==fail_at) {
            if(fault==Fault::Allocation)throw std::bad_alloc{};
            if(fault==Fault::Length)throw std::length_error("fixture length");
            if(fault==Fault::Filesystem)throw fs::filesystem_error("fixture filesystem",path,
                std::make_error_code(std::errc::permission_denied));
            if(fault==Fault::Range)throw std::out_of_range("fixture range");
            if(fault==Fault::Failure){
                ErrorDetail d;d.stage=ErrorStage::Read;d.source_path=path.string();d.variable=name;
                d.message="source reader failure";d.run_id=RunId{"reader_leaf"};d.observation_index=3;
                return Result<DenseDoubleArray,MatErrorCode>::failure({MatErrorCode::MissingVariable,std::move(d)});
            }
        }
        const auto it=data.find(path.string());
        if(it==data.end()&&name=="egomotion_metric_policy") {
            ErrorDetail d;d.stage=ErrorStage::Read;d.source_path=path.string();d.variable=name;
            d.message="Legacy source fixture has no revised metric policy";
            return Result<DenseDoubleArray,MatErrorCode>::failure(
                {MatErrorCode::MissingFile,std::move(d)});
        }
        require(it!=data.end(),"Expected input path and variable order");
        require(it->second.info.name==name,"Exact named variable");
        return Result<DenseDoubleArray,MatErrorCode>::success(it->second);
    }
};
void add(Reader& reader,const fs::path& root,const std::string& name,std::size_t rows,
         std::size_t columns,double base) {
    DenseDoubleArray a;a.info.name=name;a.info.dimensions={rows,columns};
    a.info.original_class_type=6;a.info.original_data_type=9;a.info.data_bytes=rows*columns*sizeof(double);
    a.source_mat_sha256=std::string(64,'a');a.values_column_major.resize(rows*columns);
    for(std::size_t col=0;col<columns;++col)for(std::size_t row=0;row<rows;++row)
        a.values_column_major[col*rows+row]=base+static_cast<double>(col);
    reader.data.emplace((root/(name+".mat")).string(),std::move(a));
}
Reader inputs(AnalysisKind kind,const fs::path& root) {
    Reader r;
    if(kind==AnalysisKind::Bias){
        for(std::size_t run=1;run<=2;++run){const auto dir=root/std::to_string(run);
            add(r,dir,"xwerte",4,1,run==1?100:400);
            for(int index=3;index<=6;++index)add(r,dir,"ywerte"+std::to_string(index),4,3,run==1?2:6);
        }return r;
    }
    const bool init=kind==AnalysisKind::AdaptiveInitialization,rates=kind==AnalysisKind::AdaptiveUpdateRates;
    const std::size_t rows=init?35:600,cols=rates?4:1;
    add(r,root,"allemittransfehler",rows,cols,11);add(r,root,"allemitrotfehler",rows,cols,22);
    if(!rates){add(r,root,"alleohnetransfehler",rows,1,33);add(r,root,"alleohnerotfehler",rows,1,44);}
    if(!init){
        add(r,root,"rottransratio",rows,1,0.03);
        auto& ratio=r.data.at((root/"rottransratio.mat").string()).values_column_major;
        for(const auto& [first,last]:std::array<std::pair<int,int>,3>{{{120,179},{260,319},{400,459}}})
            for(int frame=first;frame<=last;++frame)ratio[frame-1]=0.005;
    }return r;
}
std::vector<std::string> expectedCalls(AnalysisKind kind,const fs::path& root) {
    std::vector<std::string> paths;auto add=[&](const fs::path& dir,const char* name){paths.push_back((dir/(std::string(name)+".mat")).string());};
    if(kind==AnalysisKind::Bias){for(const char* run:{"1","2"})for(const char* name:{"xwerte","ywerte3","ywerte4","ywerte5","ywerte6"})add(root/run,name);return paths;}
    add(root,"egomotion_metric_policy");
    add(root,"allemittransfehler");add(root,"allemitrotfehler");
    if(kind!=AnalysisKind::AdaptiveInitialization)add(root,"rottransratio");
    if(kind!=AnalysisKind::AdaptiveUpdateRates){add(root,"alleohnetransfehler");add(root,"alleohnerotfehler");}
    return paths;
}
Data direct(AnalysisKind kind,const fs::path& root,Reader& reader,
            const analysis::BiasResultAnalyzer& bias,const analysis::AdaptiveResultAnalyzer& adaptive) {
    if(kind==AnalysisKind::Bias)return bias.aggregate(root,reader);
    return adaptive.analyze(kind,root,reader);
}
std::string json(const AnalysisDataset& data){auto value=analysis::datasetJson(data);require(value.hasValue(),"Dataset JSON");return std::move(value).value();}
struct Publisher final:ITextPublisher {
    Status<MatErrorCode> publishNew(std::string_view,std::string_view) override {
        throw std::logic_error("Rejecting exporter cannot publish");
    }
};
struct RejectingExporter final:IPlotExporter {
    Result<std::vector<std::string>,PlotErrorCode> exportPlot(const AnalysisDataset& data,ITextPublisher&) override {
        require(!data.curves.empty(),"Exporter consumes completed analysis");
        ErrorDetail d;d.stage=ErrorStage::Plot;d.message="synthetic exporter failure";
        return Result<std::vector<std::string>,PlotErrorCode>::failure({PlotErrorCode::WriteFailed,std::move(d)});
    }
};
void complete(const fs::path& base) {
    const analysis::BiasResultAnalyzer bias;const analysis::AdaptiveResultAnalyzer adaptive;
    analysis::MatResultAnalyzer facade;const std::array<std::size_t,4> expected{{13,2,12,20}};
    std::array<std::string,4> first;
    for(int pass=0;pass<2;++pass)for(std::size_t k=0;k<kinds.size();++k){
        const auto root=base/std::to_string(k);auto reader=inputs(kinds[k],root);
        auto result=direct(kinds[k],root,reader,bias,adaptive);
        require(result.hasValue()&&result.value().curves.size()==expected[k],"Exact source curve counts");
        require(reader.calls==expectedCalls(kinds[k],root),"Source read order");
        const auto snapshot=json(result.value());if(pass==0)first[k]=snapshot;else require(snapshot==first[k],"Interleaved analyzer reuse");
        auto forwarded=inputs(kinds[k],root);const auto same=facade.analyze(kinds[k],root,forwarded);
        require(same.hasValue()&&snapshot==json(same.value()),"Facade preserves all numerical and provenance fields");
        require(forwarded.calls==reader.calls,"Facade preserves read order");
        if(k==0){const auto& curves=result.value().curves;require(curves[0].x==std::vector<double>({400,400})&&
            curves[0].y==std::vector<double>({5,5})&&curves[1].y==std::vector<double>({4,4}),"Last x and exact two-run mean");}
        if(k==1)require(result.value().curves[0].y==std::vector<double>(13,11)&&
            result.value().curves[1].y==std::vector<double>(35,22),"Initialization source windows");
        reader.data.clear();require(json(result.value())==snapshot,"Result owns arrays and provenance after reader data destruction");
        Publisher publisher;RejectingExporter exporter;const auto failure=exporter.exportPlot(result.value(),publisher);
        require(!failure.hasValue()&&json(result.value())==snapshot,"Export failure preserves numeric analysis");
    }
}
void boundaries(const fs::path& base) {
    const analysis::BiasResultAnalyzer bias;const analysis::AdaptiveResultAnalyzer adaptive;
    analysis::MatResultAnalyzer facade;
    for(std::size_t k=0;k<kinds.size();++k){
        const auto root=base/std::to_string(k);const auto paths=expectedCalls(kinds[k],root);
        for(Fault fault:{Fault::Failure,Fault::Allocation,Fault::Length,Fault::Filesystem,Fault::Range})
            for(std::size_t where:{std::size_t{1},paths.size()}){
                auto reader=inputs(kinds[k],root);reader.fault=fault;reader.fail_at=where;
                const auto result=direct(kinds[k],root,reader,bias,adaptive);
                const auto expected=fault==Fault::Failure?AnalysisErrorCode::MissingVariable:
                    fault==Fault::Allocation?AnalysisErrorCode::UndefinedStatistic:
                    fault==Fault::Filesystem?AnalysisErrorCode::MissingResult:AnalysisErrorCode::ShapeMismatch;
                require(!result.hasValue()&&result.error().code==expected&&reader.calls.size()==where,"Failure prefix and classification");
                if(fault==Fault::Failure){const auto& cause=result.error().detail.causes;
                    require(!cause.empty()&&cause.front().domain=="MatErrorCode"&&cause.front().code=="MissingVariable"&&
                        cause.front().source_path==paths[where-1]&&cause.front().variable==fs::path(paths[where-1]).stem().string()&&
                        cause.front().stage==ErrorStage::Read&&cause.front().message=="source reader failure"&&cause.front().run_id&&
                        cause.front().run_id->text=="reader_leaf"&&cause.front().observation_index==3,"Full Reader failure origin");}
                auto other=inputs(kinds[k],root);other.fault=fault;other.fail_at=where;
                const auto forwarded=facade.analyze(kinds[k],root,other);
                require(!forwarded.hasValue()&&forwarded.error().code==result.error().code&&
                    forwarded.error().detail.message==result.error().detail.message&&other.calls==reader.calls,
                    "Facade has identical component error boundary");
            }
        Reader empty;const auto failure=direct(kinds[k],{},empty,bias,adaptive);
        require(!failure.hasValue()&&failure.error().code==AnalysisErrorCode::MissingResult&&empty.calls.empty(),"Empty root before Reader");
    }
    Reader reader;
    for(const auto kind:{AnalysisKind::Bias,static_cast<AnalysisKind>(-1)}){
        const auto result=adaptive.analyze(kind,base,reader);
        require(!result.hasValue()&&result.error().code==AnalysisErrorCode::UnsupportedExperiment&&reader.calls.empty(),"Adaptive rejects unsupported kind before Reader");
    }
}
} // namespace
int main(int argc,char** argv){
    try{
        require(argc==2,"Isolated sandbox argument required");const auto base=fs::absolute(argv[1])/"分析 内存";
        fs::create_directories(base/"0"/"1");fs::create_directories(base/"0"/"2");
        complete(base);boundaries(base);
        std::cout<<"analyzer service checks="<<checks<<"; modes=4; passes=2; reader faults=40\n";return 0;
    }catch(const std::exception& e){std::cerr<<"analyzer services: "<<e.what()<<'\n';return 1;}
}
