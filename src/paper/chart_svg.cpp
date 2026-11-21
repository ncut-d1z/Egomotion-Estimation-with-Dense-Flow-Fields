#include "charts.hpp"

#include <algorithm>
#include <cmath>
#include <set>

namespace egomotion::paper {
namespace {
constexpr int kWidth=660,kHeight=415,kLeft=78,kRight=588,kTop=58,kBottom=292;
struct Range {
    double low=std::numeric_limits<double>::infinity(),high=-low;
    void include(double x){low=std::min(low,x);high=std::max(high,x);}
    bool populated() const {return low<=high;}
    void finish(bool log) {
        if(!populated()){low=0;high=1;return;}
        if(log){low=std::floor(low);high=std::ceil(high);}
        if(low==high){low-=1;high+=1;}
    }
    double position(double v,double start,double end) const {return start+(v-low)/(high-low)*(end-start);}
};
std::string escape(const std::string& label) {
    std::string out;
    for(char c:label) {
        if(c=='&')out+="&amp;";
        else if(c=='<')out+="&lt;";
        else if(c=='>')out+="&gt;";
        else out+=c;
    }
    return out;
}
void ticks(std::ostream& out,const Range& range,bool horizontal,bool log,bool secondary=false) {
    const auto axis=secondary?kRight:kLeft;
    constexpr int max_intervals=5;
    const double step=log?std::max(1.0,std::ceil((range.high-range.low)/max_intervals)):
                          (range.high-range.low)/max_intervals;
    for(unsigned i=0;i<=max_intervals;++i) {
        const double value=range.low+i*step;
        if(value>range.high)break;
        const double p=horizontal?range.position(value,kLeft,kRight):range.position(value,kBottom,kTop);
        auto label=table();
        if(log)label<<"10^"<<static_cast<int>(value);
        else label<<std::setprecision(4)<<value;
        if(horizontal)out<<"<path d=\"M"<<p<<' '<<kBottom<<"v5\"/><text x=\""<<p<<"\" y=\""
                         <<kBottom+19<<"\" text-anchor=\"middle\">"<<label.str()<<"</text>\n";
        else out<<"<path d=\"M"<<axis<<' '<<p<<'h'<<(secondary?5:-5)<<"\"/><text x=\""
                <<axis+(secondary?8:-8)<<"\" y=\""<<p+4<<"\" text-anchor=\""<<(secondary?"start":"end")
                <<"\">"<<label.str()<<"</text>\n";
    }
}
void line(std::ostream& out,const Curve& curve,const Range& x,const Range& y,bool log_x) {
    auto path=table();
    path<<std::fixed<<std::setprecision(3);
    std::size_t length=0;
    double first_x=0,first_y=0;
    auto flush=[&] {
        if(length==1)out<<"<circle cx=\""<<first_x<<"\" cy=\""<<first_y<<"\" r=\"1.6\" fill=\""<<curve.color<<"\"/>\n";
        length=0;
    };
    for(const auto& p:curve.points) {
        if(p.starts_segment)flush();
        if(!p.y||*p.y<=0||(log_x&&p.x<=0)){flush();continue;}
        const double px=x.position(log_x?std::log10(p.x):p.x,kLeft,kRight);
        const double py=y.position(std::log10(*p.y),kBottom,kTop);
        if(!std::isfinite(px)||!std::isfinite(py))invalid("Non-finite paper SVG coordinate");
        if(length==0){first_x=px;first_y=py;}
        path<<(length++==0?'M':'L')<<px<<' '<<py<<' ';
    }
    flush();
    out<<"<path d=\""<<path.str()<<"\" fill=\"none\" stroke=\""<<curve.color<<"\" stroke-width=\"1.8\"";
    if(curve.dashed)out<<" stroke-dasharray=\"6 4\"";
    out<<"><title>"<<escape(curve.label)<<"</title></path>\n";
}
void panelSvg(std::ostream& out,const Panel& panel,std::size_t index,bool log_x) {
    Range x,y;
    bool ratio=false;
    for(const auto& curve:panel.curves) {
        ratio=ratio||curve.right_axis;
        for(const auto& point:curve.points) {
            if(!std::isfinite(point.x)||(point.y&&!std::isfinite(*point.y)))invalid("Invalid paper curve value");
            if(!log_x||point.x>0)x.include(log_x?std::log10(point.x):point.x);
            if(!curve.right_axis&&point.y&&*point.y>0)y.include(std::log10(*point.y));
        }
    }
    const bool has_positive=y.populated();
    x.finish(log_x);y.finish(true);
    const Range ratio_limits{-2,2}; // 图 5A–D 右纵轴 0.01..100。
    out<<"<g transform=\"translate("<<(index%2)*kWidth<<' '<<(index/2)*kHeight<<")\">\n"
       <<"<text x=\""<<kLeft<<"\" y=\"24\" font-size=\"18\">"<<escape(panel.title)<<"</text>\n"
       <<"<text x=\""<<kLeft<<"\" y=\"46\">Angle error (degrees)</text>\n"
       <<"<defs><clipPath id=\"panel"<<index<<"\"><rect x=\""<<kLeft<<"\" y=\""<<kTop<<"\" width=\""
       <<kRight-kLeft<<"\" height=\""<<kBottom-kTop<<"\"/></clipPath></defs>\n"
       <<"<g class=\"axis\"><path d=\"M"<<kLeft<<' '<<kTop<<"V"<<kBottom<<"H"<<kRight<<"\"/>\n";
    ticks(out,x,true,log_x);
    if(has_positive)ticks(out,y,false,true);
    if(ratio) {
        out<<"<path d=\"M"<<kRight<<' '<<kTop<<"V"<<kBottom<<"\"/><text x=\""<<kRight
           <<"\" y=\"46\" text-anchor=\"end\">Ideal rot/trans flow ratio</text>\n";
        ticks(out,ratio_limits,false,true,true);
    }
    out<<"</g><g clip-path=\"url(#panel"<<index<<")\">\n";
    for(const auto& curve:panel.curves)line(out,curve,x,curve.right_axis?ratio_limits:y,log_x);
    out<<"</g>";
    if(!has_positive)out<<"<text x=\"100\" y=\"110\">No positive defined angle in the displayed window</text>";
    out<<"<text x=\"333\" y=\"331\" text-anchor=\"middle\">"
       <<(log_x?"Number of sampled directions":"Frame (MATLAB index)")<<"</text>\n";
    // Fig. 1 的三条噪声曲线共享线型，图例明确列出实际乘数。
    std::set<std::string> labels;
    std::size_t ordinal=0;
    for(const auto& curve:panel.curves) {
        auto label=curve.label;
        if(log_x)label=label.substr(0,label.find(','));
        if(!labels.insert(label).second)continue;
        const auto lx=30+(ordinal%3)*210,ly=355+(ordinal/3)*20;
        out<<"<path stroke=\""<<curve.color<<"\" stroke-width=\"2\" "
           <<(curve.dashed?"stroke-dasharray=\"6 4\" ":"")<<"d=\"M"<<lx<<' '<<ly<<"h22\"/>"
           <<"<text x=\""<<lx+27<<"\" y=\""<<ly+4<<"\">"<<escape(label)<<"</text>\n";
        ++ordinal;
    }
    if(log_x)out<<"<text x=\"30\" y=\"403\">Source noise multipliers: 0.6, 2.4, 9.6</text>\n";
    out<<"</g>\n";
}
} // namespace
std::string chartSvg(const Chart& chart) {
    auto out=table();
    const auto cols=std::min<std::size_t>(2,chart.panels.size());
    out<<"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 "<<cols*kWidth<<' '
       <<((chart.panels.size()+1)/2)*kHeight<<"\" role=\"img\"><title>"<<escape(chart.name)
       <<"</title><style>svg{font:12px sans-serif;fill:#161616}.axis path{stroke:#333;fill:none}</style>"
         "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";
    for(std::size_t i=0;i<chart.panels.size();++i)panelSvg(out,chart.panels[i],i,chart.log_x);
    out<<"</svg>\n";
    return out.str();
}
} // namespace egomotion::paper
