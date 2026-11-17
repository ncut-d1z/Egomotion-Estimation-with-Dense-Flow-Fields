// 独立解析参考：不链接 Analyzer，不调用生产平滑/分段/JSON 实现。
// 三次多项式及对称和的推导见 R6_ANALYSIS_FINITE_VALIDATION_2026-11-17.md。
#include "egomotion/io/run_mat_writer.hpp"
#include <matio.h>

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
namespace fs=std::filesystem;
using namespace egomotion;
constexpr std::size_t kFrames=600;
constexpr std::array<std::size_t,4> kHalfWindows{{0,2,5,10}};
constexpr std::array<const char*,4> kVariables{{
    "allemittransfehler","allemitrotfehler","alleohnetransfehler","alleohnerotfehler"}};
struct Bounds {std::size_t first,last;};
// 手工固定边界，绝不重跑生产阈值扫描作为参考。
constexpr std::array<Bounds,4> kComparison{{{50,120},{181,260},{320,400},{460,550}}};
constexpr std::array<Bounds,4> kRates{{{50,111},{190,250},{330,390},{470,550}}};

void require(bool ok,const std::string& why) {
    if(!ok)throw std::runtime_error(why);
}
std::int64_t polynomial(std::size_t variable,std::size_t column,std::size_t frame,
                        std::size_t half=0) {
    const auto a=static_cast<std::int64_t>((variable+1)*4+column);
    const auto b=a+1,c=a+2,k=static_cast<std::int64_t>(frame);
    const auto f=static_cast<std::int64_t>(half);
    // mean[p(k+j)] = p(k) + (a*k+b)*f*(f+1), j=-f..f。
    return 1000*a+a*k*k*k+3*b*k*k+c*k+(a*k+b)*f*(f+1);
}
double ratio(std::size_t frame) {
    if(frame==110)return 0.02; // 等于 UpdateRates 阈值不进入低值区。
    if(frame==180)return 0.01; // 等于 Comparison 阈值不离开低值区。
    for(const auto b:std::array<Bounds,3>{{{120,179},{260,319},{400,459}}})
        if(frame>=b.first&&frame<=b.last)return 0.005;
    for(const auto b:std::array<Bounds,3>{{{110,189},{250,329},{390,469}}})
        if(frame>=b.first&&frame<=b.last)return 0.015;
    return 0.03;
}
std::vector<double> values(std::size_t variable,std::size_t column,Bounds range,
                           std::size_t half=0,bool zero_edges=false) {
    std::vector<double> out;
    for(auto k=range.first;k<=range.last;++k) {
        const bool edge=zero_edges&&(k<range.first+half||k>range.last-half);
        out.push_back(edge?0.0:static_cast<double>(polynomial(variable,column,k,half)));
    }
    return out;
}
void jsonString(std::ostream& out,const std::string& value) {
    // 测试路径由 CMake 创建；拒绝控制字符，std::quoted 负责引号及反斜杠。
    for(unsigned char c:value)require(c>=0x20,"Control character in fixture identity");
    out<<std::quoted(value);
}
void numbers(std::ostream& out,const std::vector<double>& values) {
    out<<'[';
    for(std::size_t i=0;i<values.size();++i){if(i)out<<',';out<<values[i];}
    out<<']';
}
void input(std::ostream& out,const fs::path& root,const std::string& variable,
           const std::string& role) {
    out<<"{\"path\":";jsonString(out,(root/(variable+".mat")).string());
    out<<",\"variable\":";jsonString(out,variable);
    out<<",\"matlab_run_ordinal\":1,\"role\":";jsonString(out,role);
    out<<",\"mat_sha256\":null}";
}
struct Oracle {
    std::ofstream out;
    fs::path input_root;
    bool first=true;
    Oracle(const fs::path& file,fs::path root,bool rates):out(file),input_root(std::move(root)) {
        out.imbue(std::locale::classic());out<<std::setprecision(17);
        require(out.good(),"Cannot create oracle JSON");
        out<<"{\"schema\":\"egomotion-analysis-v1\",\"source_analysis_blob\":\""
           <<(rates?"67cb613328dec10105a6c383ce17025c4901e4c9":
                     "3924038f0ccd70de5ea7b60c589afe166da9c8fb")<<"\",\"curves\":[";
    }
    void curve(const std::string& id,const std::string& panel,Bounds range,
               const std::string& variable,const std::vector<double>& y,bool rates,
               bool threshold=false,std::size_t half=0,const std::vector<double>& full={}) {
        if(!first)out<<',';
        first=false;
        out<<"{\"id\":";jsonString(out,id);
        out<<",\"panel\":";jsonString(out,panel);
        out<<",\"x_unit\":\"MATLAB 1-based frame\",\"y_unit\":";
        jsonString(out,variable=="rottransratio"?"rot/trans flow ratio":"angle error in degree");
        out<<",\"x_scale\":\"linear\",\"y_scale\":\""<<(rates?"linear":"log")
           <<"\",\"source_begin_matlab\":"<<range.first<<",\"source_end_matlab\":"<<range.last
           <<",\"smoothing_half_window\":"<<half<<",\"x\":[";
        for(auto k=range.first;k<=range.last;++k){if(k!=range.first)out<<',';out<<k;}
        out<<"],\"y\":";numbers(out,y);
        out<<",\"smoothing_full\":";numbers(out,full);
        out<<",\"helper_source_blob_shas\":[";
        if(half>0)out<<"\"eced66659205a6386ec84723e37a46517d06c017\"";
        out<<"]";
        out<<",\"inputs\":[";input(out,input_root,variable,"y_values");
        if(threshold){out<<',';input(out,input_root,"rottransratio","segment_threshold");}
        out<<"]}";
    }
    void finish(){out<<"]}\n";out.close();require(!out.fail(),"Cannot finish oracle JSON");}
};
void comparisonOracle(Oracle& oracle) {
    for(std::size_t zone=0;zone<kComparison.size();++zone) {
        const auto range=kComparison[zone];
        oracle.curve("without_rotation_segment_"+std::to_string(zone+1),"rotation",range,
            kVariables[3],values(3,0,range),false,true);
        oracle.curve("with_rotation_segment_"+std::to_string(zone+1),"rotation",range,
            kVariables[1],values(1,0,range),false,true);
    }
    constexpr Bounds plotted{50,550};
    oracle.curve("without_translation","translation",plotted,kVariables[2],values(2,0,plotted),false);
    oracle.curve("with_translation","translation",plotted,kVariables[0],values(0,0,plotted),false);
    std::vector<double> ratios;
    for(auto k=plotted.first;k<=plotted.last;++k)ratios.push_back(ratio(k));
    for(const std::string panel:{"rotation","translation"})
        oracle.curve("ratio_"+panel,panel,plotted,"rottransratio",ratios,false);
}
void ratesOracle(Oracle& oracle) {
    for(std::size_t zone=0;zone<kRates.size();++zone) {
        const auto full=kRates[zone];
        for(std::size_t col=0;col<kHalfWindows.size();++col) {
            const auto half=kHalfWindows[col];
            const Bounds plotted{full.first+half,full.last-half};
            oracle.curve("rotation_period_"+std::to_string(col+1)+"_segment_"+std::to_string(zone+1),
                "rotation",plotted,kVariables[1],values(1,col,plotted,half),true,true,half,
                values(1,col,full,half,true));
        }
    }
    constexpr Bounds plotted{50,550};
    for(std::size_t col=0;col<kHalfWindows.size();++col) {
        const auto half=kHalfWindows[col];
        oracle.curve("translation_period_"+std::to_string(col+1),"translation",plotted,
            kVariables[0],values(0,col,plotted,half),true,false,half,
            values(0,col,{1,kFrames},half,true));
    }
}
void fixture(const fs::path& root,bool rates) {
    const RunId id{rates?"rates_fixture":"comparison_fixture"};
    auto allocated=io::RunDirectory::create(root,id);
    require(allocated.hasValue(),"Cannot allocate unique fixture RunId");
    auto directory=std::move(allocated).value();
    io::RunMatWriter writer(*directory);
    const auto input_root=directory->outputRoot()/id.text;
    const std::size_t variable_count=rates?2:4,columns=rates?4:1;
    for(std::size_t v=0;v<=variable_count;++v) {
        const bool is_ratio=v==variable_count;
        DenseDoubleArray array;
        array.info.name=is_ratio?"rottransratio":kVariables[v];
        array.info.dimensions={kFrames,is_ratio?1:columns};
        array.info.original_class_type=MAT_C_DOUBLE;
        array.info.original_data_type=MAT_T_DOUBLE;
        for(std::size_t col=0;col<array.info.dimensions[1];++col)
            for(std::size_t k=1;k<=kFrames;++k)
                array.values_column_major.push_back(is_ratio?ratio(k):static_cast<double>(polynomial(v,col,k)));
        array.info.data_bytes=array.values_column_major.size()*sizeof(double);
        require(writer.saveNewVariable(id,array.info.name,array).hasValue(),"Cannot publish synthetic MAT");
    }
    Oracle oracle(root/"evidence"/(id.text+".expected.json"),input_root,rates);
    if(rates)ratesOracle(oracle);else comparisonOracle(oracle);
    oracle.finish();
    for(const auto& pair:writer.receipts())
        std::cout<<id.text<<'/'<<pair.second.filename<<" sha256="<<pair.second.sha256
                 <<" bytes="<<pair.second.bytes<<'\n';
}
} // namespace
int main(int argc,char** argv) {
    try {
        require(argc==2,"Expected isolated fixture repository root");
        const fs::path root=fs::absolute(argv[1]);
        require(fs::is_directory(root/"origin")&&fs::is_empty(root/"origin"),"Fixture origin must be empty");
        require(!fs::exists(root/"output"),"Refuse to reuse fixture outputs");
        require(fs::is_directory(root/"evidence"),"Missing evidence directory");
        fixture(root,false);fixture(root,true);
        std::cout<<"Independent cubic oracle; deterministic; no RNG; exact integer means\n";
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
