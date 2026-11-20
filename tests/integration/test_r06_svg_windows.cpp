// 源视窗/双纵轴的直接SVG回归；解析序列化坐标，不把文件存在当作视觉等价。
#include "egomotion/analysis/plot.hpp"
#include "egomotion/analysis/dataset_json.hpp"
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
using namespace egomotion;
namespace fs=std::filesystem;
constexpr const char* kBiasBlob="112aa9eca1fa9728f493d31fc8e59620a8445230";
constexpr const char* kComparisonBlob="3924038f0ccd70de5ea7b60c589afe166da9c8fb";
std::size_t checks=0,failures=0;
void check(bool ok,const std::string& why) {
    ++checks;if(!ok){++failures;std::cerr<<"FAIL "<<why<<'\n';}
}
void require(bool ok,const std::string& why){if(!ok)throw std::runtime_error(why);}
void save(const fs::path& path,const std::string& text) {
    std::ofstream out(path);out<<text;out.close();require(!out.fail(),"Cannot save SVG evidence");
}
struct Publisher final:ITextPublisher {
    std::map<std::string,std::string> files;
    Status<MatErrorCode> publishNew(std::string_view name,std::string_view data) override {
        require(files.emplace(std::string(name),std::string(data)).second,"Unexpected overwrite");
        return Status<MatErrorCode>::success();
    }
};
AnalysisSeries curve(std::string name,std::string panel,std::vector<double> x,
                     std::vector<double> y,bool xlog,bool ylog,bool ratio=false) {
    AnalysisSeries s;s.name=std::move(name);s.panel=std::move(panel);
    s.x=std::move(x);s.y=std::move(y);
    s.x_scale=xlog?"log":"linear";s.y_scale=ylog?"log":"linear";
    s.y_unit=ratio?"rot/trans flow ratio":"angle error in degree";
    return s;
}
std::string render(const fs::path& root,const std::string& label,AnalysisDataset data) {
    const auto before=analysis::datasetJson(data);require(before.hasValue(),"Invalid fixture JSON");
    const fs::path dir=root/label;require(fs::create_directory(dir),"Evidence directory already exists");
    save(dir/"before.json",before.value());
    Publisher publisher;analysis::SvgPlotExporter plotter;
    auto result=plotter.exportPlot(data,publisher);
    check(result.hasValue(),label+" export succeeds");
    const auto after=analysis::datasetJson(data);require(after.hasValue(),"Invalid post-plot JSON");
    save(dir/"after.json",after.value());
    check(before.value()==after.value(),label+" numeric dataset unchanged");
    for(const auto& file:publisher.files)save(dir/file.first,file.second);
    if(!result.hasValue())return {};
    check(result.value().size()==1&&publisher.files.size()==1,label+" one expected panel");
    return publisher.files.empty()?std::string{}:publisher.files.begin()->second;
}
std::string attribute(const std::string& tag,const std::string& name) {
    const auto start=tag.find(name+"=\"");
    if(start==std::string::npos)return {};
    const auto value=start+name.size()+2,end=tag.find('"',value);
    return tag.substr(value,end-value);
}
bool viewportClip(const std::string& svg,const std::string& line) {
    const auto binding=attribute(line,"clip-path");
    if(binding.substr(0,5)!="url(#"||binding.back()!=')')return false;
    const auto id=binding.substr(5,binding.size()-6);
    const auto start=svg.find("<clipPath id=\""+id+"\"");
    if(start==std::string::npos)return false;
    const auto end=svg.find("</clipPath>",start),rect=svg.find("<rect ",start);
    if(rect==std::string::npos||rect>end)return false;
    const auto tag=svg.substr(rect,svg.find('>',rect)-rect+1);
    return std::stod(attribute(tag,"x"))==95&&std::stod(attribute(tag,"y"))==70&&
           std::stod(attribute(tag,"width"))==985&&std::stod(attribute(tag,"height"))==560;
}
using Point=std::pair<double,double>;
using Polyline=std::vector<Point>;
std::vector<Polyline> lines(const std::string& svg,const std::string& label) {
    const std::regex pattern("<polyline [^>]*>");
    std::vector<Polyline> result;
    for(std::sregex_iterator i(svg.begin(),svg.end(),pattern),end;i!=end;++i) {
        const std::string tag=i->str();
        check(viewportClip(svg,tag),label+" every curve is clipped to the viewport");
        std::istringstream input(attribute(tag,"points"));
        Polyline points;double x=0,y=0;char comma=0;
        while(input>>x>>comma>>y){require(comma==',',"Malformed SVG points");points.emplace_back(x,y);}
        result.push_back(std::move(points));
    }
    return result;
}
void point(const Polyline& line,std::size_t index,Point expected,const std::string& why) {
    check(index<line.size()&&line[index]==expected,why);
}
void bias(const fs::path& root) {
    AnalysisDataset data;data.source_analysis_blob=kBiasBlob;
    data.curves.push_back(curve("boundary-crossing","bias-loglog",
        {1,100,std::sqrt(300000000.0),3000000,300000000},
        {0.06,0.06,std::sqrt(12.0),200,200},true,true));
    data.curves.push_back(curve("zero-gap","bias-loglog",
        {100,1000,10000,100000,1000000},{1,2,0,4,8},true,true));
    data.curves.push_back(curve("below-window","bias-loglog",
        {100,3000000},{0.006,0.006},true,true));
    const auto svg=render(root,"bias",data);const auto paths=lines(svg,"bias");
    check(paths.size()==4,"Bias zero splits the line; off-window data retained for clipping");
    if(paths.size()==4) {
        check(paths[0].size()==5,"Crossing line keeps both off-window endpoints");
        point(paths[0],1,{95,630},"Bias fixed lower corner");
        point(paths[0],2,{587.5,350},"Bias geometric midpoint of fixed log window");
        point(paths[0],3,{1080,70},"Bias fixed upper corner");
        check(paths[0].front().first<95&&paths[0].back().first>1080,"Outliers cannot rescale Bias x axis");
        check(paths[1].size()==2&&paths[2].size()==2,"Zero is a gap, never a clamped visible point");
        check(paths[3].size()==2&&paths[3][0].second>630,"Below-window positive values are not clamped");
    }
    data.curves={curve("all-zero","bias-loglog",{100,3000000},{0,0},true,true)};
    const auto empty=render(root,"bias-zero",data);
    check(!empty.empty()&&lines(empty,"bias-zero").empty(),"Fixed Bias axes allow a blank all-zero plot");
}
void comparison(const fs::path& root,const std::string& panel) {
    AnalysisDataset data;data.source_analysis_blob=kComparisonBlob;
    data.curves.push_back(curve("angle",panel,{50,300,550},{1,10,100},false,true));
    data.curves.push_back(curve("ratio",panel,{50,175,300,425,550},
        {0.001,0.01,1,100,1000},false,true,true));
    const auto svg=render(root,panel,data);const auto paths=lines(svg,panel);
    check(paths.size()==2,panel+" independent primary and ratio curves");
    if(paths.size()==2) {
        point(paths[0],0,{95,630},panel+" primary min independent of ratio");
        point(paths[0],1,{587.5,350},panel+" primary midpoint");
        point(paths[0],2,{1080,70},panel+" primary max independent of ratio");
        point(paths[1],0,{95,770},panel+" lower outlier lies outside clip");
        point(paths[1],1,{341.25,630},panel+" ratio 0.01 is lower boundary");
        point(paths[1],2,{587.5,350},panel+" ratio 1 is log midpoint");
        point(paths[1],3,{833.75,70},panel+" ratio 100 is upper boundary");
        point(paths[1],4,{1080,-70},panel+" upper outlier lies outside clip");
    }
}
void automatic(const fs::path& root) {
    AnalysisDataset data;data.source_analysis_blob="unrecognized-source";
    data.curves={curve("unknown","bias-loglog",{100,1000},{1,10},true,true)};
    auto paths=lines(render(root,"unknown-source",data),"unknown-source");
    check(paths.size()==1,"Unknown source is not assigned Bias limits by panel name alone");
    if(paths.size()==1){point(paths[0],0,{95,630},"Automatic minimum");point(paths[0],1,{1080,70},"Automatic maximum");}
    data.source_analysis_blob="67cb613328dec10105a6c383ce17025c4901e4c9";
    data.curves={curve("rates","rotation",{50,300,550},{0,5,10},false,false)};
    paths=lines(render(root,"rates-linear",data),"rates-linear");
    check(paths.size()==1,"UpdateRates retains zero on linear axis");
    if(paths.size()==1){point(paths[0],0,{95,630},"Linear zero");point(paths[0],1,{587.5,350},"Linear midpoint");}
}
bool identical(const std::vector<double>& a,const std::vector<double>& b) {
    return a.size()==b.size()&&(a.empty()||std::memcmp(a.data(),b.data(),a.size()*sizeof(double))==0);
}
void invalidSeries(const fs::path& root) {
    const double nan=std::numeric_limits<double>::quiet_NaN();
    const double inf=std::numeric_limits<double>::infinity();
    // 已知固定视窗和普通自动视窗均不能把无效数值过滤成成功。
    for(const bool known:{false,true})for(int scenario=0;scenario<7;++scenario) {
        AnalysisDataset data;data.source_analysis_blob=known?kBiasBlob:"unrecognized-source";
        auto s=curve("invalid","bias-loglog",{100,1000},{1,10},true,true);
        switch(scenario) {
        case 0:s.x[0]=nan;break;
        case 1:s.y[0]=nan;break;
        case 2:s.x[0]=inf;break;
        case 3:s.y[0]=-inf;break;
        case 4:s.y={nan,nan};break;
        case 5:s.x.clear();s.y.clear();break;
        case 6:s.y.pop_back();break;
        }
        const std::string label=std::string(known?"fixed":"auto")+"-invalid-"+std::to_string(scenario);
        data.curves.push_back(s);
        Publisher publisher;analysis::SvgPlotExporter plotter;
        const auto result=plotter.exportPlot(data,publisher);
        check(!result.hasValue()&&result.error().code==PlotErrorCode::InvalidSeries&&
              result.error().detail.stage==ErrorStage::Plot,label+" explicit InvalidSeries");
        check(publisher.files.empty(),label+" publishes no file");
        check(identical(data.curves[0].x,s.x)&&identical(data.curves[0].y,s.y),label+" preserves raw input bits");
        std::ostringstream evidence;
        evidence<<"source="<<data.source_analysis_blob<<"\nscenario="<<scenario<<"\nx=";
        for(double v:s.x)evidence<<v<<' ';
        evidence<<"\ny=";for(double v:s.y)evidence<<v<<' ';
        evidence<<"\nresult="<<(result.hasValue()?"unexpected success":std::string(errorName(result.error().code)))
                <<"\npublished="<<publisher.files.size()<<'\n';
        save(root/(label+".txt"),evidence.str());
    }
}
void declaredGaps(const fs::path& root) {
    AnalysisDataset data;data.source_analysis_blob="declared-synthetic-metric";
    data.metric_policy="adaptive-atan2-so3-geodesic-v1";data.planned_repetitions=1;
    const auto nan=std::numeric_limits<double>::quiet_NaN();
    auto series=curve("declared-gap","rotation",{1,2,3,4,5},{1,2,nan,4,5},false,false);
    series.defined={1,1,0,1,1};series.metric_semantics="rotation-so3-geodesic-degrees";
    data.curves.push_back(series);
    const auto svg=render(root,"declared-gap",data);
    const auto paths=lines(svg,"declared-gap");
    check(paths.size()==2&&paths[0].size()==2&&paths[1].size()==2,
          "Declared missing sample divides polylines instead of bridging the gap");
    check(svg.find("rotation-so3-geodesic-degrees")!=std::string::npos,
          "SVG retains revised rotation metric semantics");
    for(unsigned invalid=0;invalid<5;++invalid) {
        auto candidate=data;
        auto& current=candidate.curves[0];
        if(invalid==0)current.defined.clear();
        if(invalid==1)current.defined.pop_back();
        if(invalid==2)current.defined[2]=2;
        if(invalid==3)current.y[2]=0.0;
        if(invalid==4)current.y[2]=std::numeric_limits<double>::infinity();
        Publisher publisher;analysis::SvgPlotExporter plotter;
        const auto result=plotter.exportPlot(candidate,publisher);
        check(!result.hasValue()&&result.error().code==PlotErrorCode::InvalidSeries&&publisher.files.empty(),
              "SVG rejects invalid or undeclared NaN/Inf/finite-zero gap masks");
    }
}
} // namespace
int main(int argc,char** argv) {
    try {
        require(argc==3,"Expected isolated build root and source SHA");
        const fs::path base=fs::absolute(argv[1]);fs::create_directories(base);
        std::string pattern=(base/"svg-windows-XXXXXX").string();
        require(::mkdtemp(pattern.data())!=nullptr,"Cannot allocate fresh evidence directory");
        const fs::path root=pattern;
        save(root/"identity.txt",std::string("source_sha=")+argv[2]+"\nseed=none\nfixed synthetic SVG geometry\n");
        std::cout<<"RunId="<<root.filename()<<" evidence="<<root<<'\n';
        bias(root);comparison(root,"rotation");comparison(root,"translation");automatic(root);invalidSeries(root);declaredGaps(root);
        std::cout<<"checks="<<checks<<" failures="<<failures<<'\n';
        return failures?1:0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
