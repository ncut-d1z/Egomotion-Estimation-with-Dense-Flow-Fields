#include "egomotion/analysis/plot.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <limits>
#include <locale>
#include <map>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace egomotion::analysis {
namespace {
using Files=Result<std::vector<std::string>,PlotErrorCode>;
Files failure(PlotErrorCode code,std::string message) {
    ErrorDetail detail;detail.stage=ErrorStage::Plot;detail.message=std::move(message);
    return Files::failure({code,std::move(detail)});
}
std::string xml(std::string_view s) {
    std::string result;
    for(char c:s){
        switch(c){
        case '&': result+="&amp;";break;
        case '<': result+="&lt;";break;
        case '>': result+="&gt;";break;
        case '"': result+="&quot;";break;
        case '\'':result+="&apos;";break;
        default:result+=c;break;
        }
    }
    return result;
}
bool safeName(const std::string& name) {
    if(name.empty()||name.size()>64)return false;
    for(unsigned char c:name)
        if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||
             (c>='0'&&c<='9')||c=='_'||c=='-'))return false;
    return true;
}
bool rightAxis(const AnalysisSeries& s){
    return s.y_unit=="rot/trans flow ratio";
}
struct Bounds{
    double minimum=std::numeric_limits<double>::infinity();
    double maximum=-std::numeric_limits<double>::infinity();
    void offer(double value){
        if(std::isfinite(value)){
            minimum=std::min(minimum,value);
            maximum=std::max(maximum,value);
        }
    }
    bool valid()const noexcept{
        return std::isfinite(minimum)&&std::isfinite(maximum);
    }
};
double transformed(double value,bool logarithmic) {
    if(!std::isfinite(value)||(logarithmic&&value<=0.0))
        return std::numeric_limits<double>::quiet_NaN();
    return logarithmic?std::log10(value):value;
}
void applySourceWindows(const std::string& panel,const std::string& source_blob,
                        const std::vector<const AnalysisSeries*>& curves,
                        Bounds& x,Bounds& y,Bounds& ratio) {
    // 原 graphs_bias_test.m:70–71；这些是显示范围，不是数值截断阈值。
    if(source_blob=="112aa9eca1fa9728f493d31fc8e59620a8445230"&&panel=="bias-loglog") {
        constexpr double x_min=100,x_max=3000000,y_min=0.06,y_max=200;
        x={std::log10(x_min),std::log10(x_max)};
        y={std::log10(y_min),std::log10(y_max)};
    }
    // 原 Comparison 两幅 plotyy 图仅显式固定右轴，左轴保持自动范围。
    if(source_blob=="3924038f0ccd70de5ea7b60c589afe166da9c8fb"&&
       (panel=="rotation"||panel=="translation")&&
       std::any_of(curves.begin(),curves.end(),[](const auto* s){return rightAxis(*s);})) {
        constexpr double ratio_min=0.01,ratio_max=100;
        ratio={std::log10(ratio_min),std::log10(ratio_max)};
    }
}
std::string renderPanel(const std::string& panel,
                        const std::vector<const AnalysisSeries*>& curves,
                        const std::string& source_blob) {
    constexpr double left=95,right=1080,top=70,bottom=630;
    Bounds bx,by,byRight;
    const bool xlog=curves.front()->x_scale=="log";
    const bool ylog=curves.front()->y_scale=="log";
    for(const auto* curve:curves){
        if(curve->x.empty()||curve->x.size()!=curve->y.size())
            throw std::invalid_argument("An SVG curve is empty or has different X and Y lengths");
        const bool is_right=rightAxis(*curve);
        for(std::size_t i=0;i<curve->x.size();++i){
            // 有限log零/负值可形成显示空隙；NaN/Inf不是合法空白图。
            if(!std::isfinite(curve->x[i])||!std::isfinite(curve->y[i]))
                throw std::invalid_argument("An SVG curve has non-finite input coordinates");
            const double x=transformed(curve->x[i],xlog);
            const double y=transformed(curve->y[i],curve->y_scale=="log");
            if(std::isfinite(x)&&std::isfinite(y)){
                bx.offer(x);
                if(is_right)byRight.offer(y);else by.offer(y);
            }
        }
    }
    applySourceWindows(panel,source_blob,curves,bx,by,byRight);
    if(!bx.valid()||!by.valid())
        throw std::invalid_argument("SVG panel has no valid primary curve coordinates");
    const double xrange=bx.maximum>bx.minimum?bx.maximum-bx.minimum:1;
    const double yrange=by.maximum>by.minimum?by.maximum-by.minimum:1;
    const double rightRange=byRight.maximum>byRight.minimum?
        byRight.maximum-byRight.minimum:1;
    const std::array<const char*,10> colors{{
        "#2864b4","#cb3b31","#2f8459","#a05ac2","#c48b23",
        "#467d8c","#b5487c","#6b7380","#6e8b3d","#98644b"
    }};
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out<<std::fixed<<std::setprecision(4);
    out<<"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"1200\" height=\"720\" "
       <<"viewBox=\"0 0 1200 720\" role=\"img\" aria-label=\""
       <<xml(panel)<<"\">\n";
    out<<"<rect width=\"1200\" height=\"720\" fill=\"white\"/>\n";
    // SVG裁剪保留跨窗口线段；不能删去越界端点或改写AnalysisDataset。
    out<<"<defs><clipPath id=\"plot-window\"><rect x=\""<<left<<"\" y=\""<<top
       <<"\" width=\""<<(right-left)<<"\" height=\""<<(bottom-top)
       <<"\"/></clipPath></defs>\n";
    out<<"<text x=\"95\" y=\"35\" font-size=\"23\">"<<xml(panel)<<"</text>\n";
    out<<"<path d=\"M"<<left<<" "<<top<<" V"<<bottom<<" H"<<right
       <<"\" fill=\"none\" stroke=\"#222\" stroke-width=\"2\"/>\n";
    if(byRight.valid())
        out<<"<path d=\"M"<<right<<" "<<top<<" V"<<bottom
           <<"\" fill=\"none\" stroke=\"#666\" stroke-width=\"1\"/>\n";
    out<<"<text x=\"95\" y=\"680\" font-size=\"15\">"
       <<(xlog?"log10(X)":"source MATLAB frame / xwerte")<<"</text>\n";
    out<<"<text x=\"95\" y=\"54\" font-size=\"13\">"
       <<(ylog?"log10(Y)":"Y")<<"</text>\n";
    if(byRight.valid())
        out<<"<text x=\"1090\" y=\"54\" font-size=\"13\">ratio (right)</text>\n";

    std::size_t series_index=0;
    for(const auto* series:curves){
        const auto color=colors[series_index%colors.size()];
        const bool right_y=rightAxis(*series);
        const Bounds& axis=right_y?byRight:by;
        const double span=right_y?rightRange:yrange;
        std::string points;
        auto flush=[&](){
            if(!points.empty()){
                out<<"<polyline fill=\"none\" stroke=\""<<color
                   <<"\" stroke-width=\"1.5\" clip-path=\"url(#plot-window)\" points=\""
                   <<points<<"\"/>\n";
                points.clear();
            }
        };
        for(std::size_t i=0;i<series->x.size();++i){
            const double x=transformed(series->x[i],xlog);
            const double y=transformed(series->y[i],series->y_scale=="log");
            if(!std::isfinite(x)||!std::isfinite(y)){flush();continue;}
            const double px=left+(x-bx.minimum)/xrange*(right-left);
            const double py=bottom-(y-axis.minimum)/span*(bottom-top);
            std::ostringstream position;position.imbue(std::locale::classic());
            position<<std::fixed<<std::setprecision(3)<<px<<","<<py<<" ";
            points+=position.str();
        }
        flush();
        if(series_index<25)
            out<<"<text x=\""<<(left+(series_index%4)*245)
               <<"\" y=\""<<(top+18+(series_index/4)*18)
               <<"\" font-size=\"11\" fill=\""<<color<<"\">"
               <<xml(series->name)<<"</text>\n";
        ++series_index;
    }
    out<<"<metadata>original analysis .m blob: "<<xml(source_blob)
       <<"; derived from read-only MAT values; numeric evidence separate</metadata>\n";
    out<<"</svg>\n";
    return out.str();
}
} // namespace

Files SvgPlotExporter::exportPlot(const AnalysisDataset& data,
                                  ITextPublisher& publisher) {
    if(data.curves.empty())
        return failure(PlotErrorCode::InvalidSeries,"No curves to render");
    try {
        std::map<std::string,std::vector<const AnalysisSeries*>> panels;
        for(const auto& s:data.curves){
            if(!safeName(s.panel))
                return failure(PlotErrorCode::InvalidSeries,
                               "Unsafe or unknown curve panel name");
            panels[s.panel].push_back(&s);
        }
        std::vector<std::pair<std::string,std::string>> prepared;
        for(const auto& group:panels)
            prepared.emplace_back(group.first+".svg",
                                  renderPanel(group.first,group.second,
                                              data.source_analysis_blob));
        std::vector<std::string> published;
        published.reserve(prepared.size());
        for(const auto& file:prepared){
            auto status=publisher.publishNew(file.first,file.second);
            if(!status.hasValue()){
                ErrorDetail d;d.stage=ErrorStage::Plot;
                d.message="SVG panel publication failed: "+file.first;
                const auto code=status.error().code==MatErrorCode::OutputPathForbidden||
                                status.error().code==MatErrorCode::OutputExists
                               ?PlotErrorCode::OutputForbidden:PlotErrorCode::WriteFailed;
                return Files::failure(wrapFailure(code,std::move(d),status.error()));
            }
            published.push_back(file.first);
        }
        return Files::success(std::move(published));
    }catch(const std::invalid_argument& e){
        return failure(PlotErrorCode::InvalidSeries,e.what());
    }catch(const std::bad_alloc&){
        return failure(PlotErrorCode::WriteFailed,"SVG renderer memory allocation failed");
    }catch(const std::length_error&){
        return failure(PlotErrorCode::InvalidSeries,"SVG panel data too large");
    }
}
} // namespace egomotion::analysis
