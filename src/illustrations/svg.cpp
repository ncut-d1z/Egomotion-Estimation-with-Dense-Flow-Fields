#include "detail.hpp"

#include <Eigen/Geometry>
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <ios>
#include <limits>
#include <locale>
#include <new>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace egomotion::illustrations {
namespace {
using Files=Result<std::vector<std::string>,PlotErrorCode>;
std::string xml(const std::string& value) {
    std::string out;
    for(char c:value) {
        switch(c){case '&':out+="&amp;";break;case '<':out+="&lt;";break;
        case '>':out+="&gt;";break;case '"':out+="&quot;";break;
        case '\'':out+="&apos;";break;default:out+=c;}
    }
    return out;
}
struct Camera {
    Vector3 right,up,forward;
    double scale;
    double center_y=420;
    Vector3 project(const Vector3& p)const {
        return {450+scale*right.dot(p),center_y-scale*up.dot(p),forward.dot(p)};
    }
};
Camera camera(const Data& data,bool paper_trajectory) {
    const double radians=std::acos(-1.0)/180;
    const double az=data.azimuth_degrees*radians,el=data.elevation_degrees*radians;
    // MATLAB azimuth从负y轴起算；相机方向与屏幕基向量正交。
    Camera out{{std::cos(az),std::sin(az),0},
               {-std::sin(az)*std::sin(el),std::cos(az)*std::sin(el),std::cos(el)},
               {std::sin(az)*std::cos(el),-std::cos(az)*std::cos(el),std::sin(el)},0};
    double radius=0;
    for(std::size_t i=0;i<data.surface.vertices.size();++i)
        if(!data.surface.z_masked[i])radius=std::max(radius,data.surface.vertices[i].norm());
    for(const auto& p:data.trajectory)radius=std::max(radius,p.norm());
    if(!(radius>0)||!std::isfinite(radius))throw std::invalid_argument("Invalid illustration display extent");
    // 径向面需给原camzoom(1.5)预留画幅；像素尺寸不改变源数值几何。
    constexpr double kTrajectoryViewportRadius=290,kHarmonicViewportRadius=210;
    double viewport_radius=data.harmonic?kHarmonicViewportRadius:kTrajectoryViewportRadius;
    if(paper_trajectory) {
        // 双面板内为单位球的三条刻度轴预留空间；图形和轴共用等比例投影。
        constexpr double kPaperUnitRadius=245,kPaperCenterY=370;
        viewport_radius=kPaperUnitRadius;
        out.center_y=kPaperCenterY;
    }
    out.scale=viewport_radius*data.camera_zoom/radius;
    if(!std::isfinite(out.scale))throw std::invalid_argument("Illustration projection scale overflow");
    return out;
}
struct Primitive {double depth;std::string svg;};
Primitive triangle(const Vector3& a,const Vector3& b,const Vector3& c,
                   const Camera& view,std::size_t face) {
    const std::array<Vector3,3> points{{view.project(a),view.project(b),view.project(c)}};
    Vector3 normal=(b-a).cross(c-a);
    if(!normal.allFinite())throw std::invalid_argument("Illustration surface normal overflow");
    const double length=normal.norm();
    if(!std::isfinite(length))throw std::invalid_argument("Illustration surface normal length overflow");
    const double light=length>0?std::abs(normal.dot(view.forward)/length):0;
    if(!std::isfinite(light))throw std::invalid_argument("Illustration surface lighting overflow");
    const double green_value=130+95*light,blue_value=160+85*light;
    // 保留原光照/颜色公式；整数转换前只核对可表示性，不钳制亮度。
    if(!(green_value>=std::numeric_limits<int>::min()&&green_value<=std::numeric_limits<int>::max()&&
         blue_value>=std::numeric_limits<int>::min()&&blue_value<=std::numeric_limits<int>::max()))
        throw std::invalid_argument("Illustration surface color conversion overflow");
    const int green=static_cast<int>(green_value),blue=static_cast<int>(blue_value);
    std::ostringstream out;
    out.exceptions(std::ios::badbit|std::ios::failbit);
    out.imbue(std::locale::classic());out<<std::fixed<<std::setprecision(3);
    out<<"<polygon data-face=\""<<face<<"\" fill=\"rgb(70,"<<green<<','<<blue
       <<")\" stroke=\"#527286\" stroke-width=\"0.25\" points=\"";
    for(const auto& p:points)out<<p.x()<<','<<p.y()<<' ';
    out<<"\"/>\n";
    return {(points[0].z()+points[1].z()+points[2].z())/3,out.str()};
}
Primitive paperQuad(const std::array<Vector3,4>& vertices,const Camera& view,std::size_t face,
                    double min_z,double max_z) {
    // 论文原图采用高度着色的四边形网格。数值顶点始终取自源几何。
    const Vector3 center=(vertices[0]+vertices[1]+vertices[2]+vertices[3])/4.0;
    const Vector3 normal=(vertices[1]-vertices[0]).cross(vertices[3]-vertices[0]);
    const double length=normal.norm(),depth=view.forward.dot(center);
    if(!center.allFinite()||!std::isfinite(length)||!std::isfinite(depth)||!(max_z>min_z))
        throw std::invalid_argument("Invalid paper surface projection");
    const double height=(center.z()-min_z)/(max_z-min_z);
    const double light=length>0?0.55+0.45*std::abs(normal.dot(view.forward)/length):0.55;
    const auto channel=[&](double peak) {
        return static_cast<int>(255*light*std::clamp(1.5-std::abs(4*height-peak),0.0,1.0));
    };
    std::ostringstream out;
    out.exceptions(std::ios::badbit|std::ios::failbit);
    out.imbue(std::locale::classic());out<<std::fixed<<std::setprecision(3);
    out<<"<polygon data-face=\""<<face<<"\" fill=\"rgb("<<channel(3)<<','<<channel(2)<<','<<channel(1)
       <<")\" stroke=\"#222\" stroke-width=\"0.65\" points=\"";
    for(const auto& p:vertices){const auto v=view.project(p);out<<v.x()<<','<<v.y()<<' ';}
    out<<"\"/>\n";
    return {depth,out.str()};
}
std::vector<Primitive> primitives(const Data& data,const Camera& view,bool paper) {
    std::vector<Primitive> out;
    const auto& s=data.surface;
    double min_z=std::numeric_limits<double>::infinity(),max_z=-min_z;
    if(paper)for(std::size_t i=0;i<s.vertices.size();++i)if(!s.z_masked[i]){
        min_z=std::min(min_z,s.vertices[i].z());max_z=std::max(max_z,s.vertices[i].z());
    }
    for(std::size_t col=0;col+1<s.columns;++col)
        for(std::size_t row=0;row+1<s.rows;++row) {
            const auto a=col*s.rows+row,b=a+1,c=a+s.rows+1,d=a+s.rows;
            if(s.z_masked[a]||s.z_masked[b]||s.z_masked[c]||s.z_masked[d])continue;
            const auto face=col*(s.rows-1)+row;
            if(paper) {
                out.push_back(paperQuad({s.vertices[a],s.vertices[b],s.vertices[c],s.vertices[d]},
                                         view,face,min_z,max_z));
                continue;
            }
            out.push_back(triangle(s.vertices[a],s.vertices[b],s.vertices[c],view,face));
            out.push_back(triangle(s.vertices[a],s.vertices[c],s.vertices[d],view,face));
        }
    for(std::size_t i=1;i<data.trajectory.size();++i) {
        const auto a=view.project(data.trajectory[i-1]),b=view.project(data.trajectory[i]);
        std::ostringstream line;
        line.exceptions(std::ios::badbit|std::ios::failbit);
        line.imbue(std::locale::classic());line<<std::fixed<<std::setprecision(3);
        line<<"<line data-trajectory-segment=\""<<i<<"\" x1=\""<<a.x()<<"\" y1=\""<<a.y()
            <<"\" x2=\""<<b.x()<<"\" y2=\""<<b.y()<<"\" stroke=\""<<(paper?"#304fbe":"#ab2637")
            <<"\" stroke-width=\""<<(paper?"5":"2.4")<<"\"/>\n";
        out.push_back({(a.z()+b.z())/2,line.str()});
    }
    std::stable_sort(out.begin(),out.end(),[](const auto& a,const auto& b){return a.depth<b.depth;});
    return out;
}
void trajectoryAxes(std::ostream& out,const Camera& view,bool reverse_x_labels) {
    // Fig. 3 保留单位球底面的 x、y 轴与左侧竖直轴。Fig. 4 源脚本 axis off。
    const double near_x=std::copysign(1.0,view.forward.x());
    const double near_y=std::copysign(1.0,view.forward.y());
    const Vector3 x_end{-near_x,near_y,-1},y_end{near_x,-near_y,-1};
    const bool x_is_left=view.project(x_end).x()<view.project(y_end).x();
    const Vector3 vertical=x_is_left?x_end:y_end;
    auto axis=[&](char name,Eigen::Index dimension,Vector3 point,int per_unit,int last_tick,double dx,double dy) {
        point[dimension]=-1;
        const auto start=view.project(point);
        point[dimension]=1;
        const auto end=view.project(point);
        const int sign=(name=='x'&&reverse_x_labels)?-1:1;
        out<<"<g data-axis=\""<<name<<"\" data-tick-sign=\""<<sign<<"\"><path d=\"M "<<start.x()<<" "<<start.y()
           <<" L "<<end.x()<<" "<<end.y()<<"\" stroke=\"#222\"/>\n";
        for(int index=-last_tick;index<=last_tick;++index) {
            point[dimension]=static_cast<double>(index)/per_unit;
            const auto projected=view.project(point);
            out<<"<path d=\"M "<<projected.x()<<" "<<projected.y()<<" l "<<dx<<" "<<dy
               <<"\" stroke=\"#222\"/><text x=\""<<projected.x()+2*dx<<"\" y=\""
               <<projected.y()+2*dy+4<<"\" text-anchor=\""<<(dx<0?"end":"start")
               <<"\">"<<static_cast<double>(sign*index)/per_unit<<"</text>\n";
        }
        out<<"</g>\n";
    };
    out<<"<g font-family=\"sans-serif\" font-size=\"14\" fill=\"#222\">\n";
    // 原图 A 的 x 底轴只标中间三个刻度，B 的 y 底轴只标中间三个。
    axis('x',0,{0,near_y,-1},2,reverse_x_labels?1:2,x_is_left?-4:4,5);
    axis('y',1,{near_x,0,-1},2,reverse_x_labels?2:1,x_is_left?4:-4,5);
    axis('z',2,vertical,5,4,-6,-2);
    out<<"</g>\n";
}
std::string render(const Data& data,const std::string& panel={}) {
    const bool paper_trajectory=!panel.empty()&&!data.harmonic;
    const auto view=camera(data,paper_trajectory);
    const auto elements=primitives(data,view,!panel.empty());
    const auto clip="scene"+panel;
    std::ostringstream out;out.imbue(std::locale::classic());
    out.exceptions(std::ios::badbit|std::ios::failbit);
    out<<"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"900\" height=\"800\" viewBox=\"0 0 900 800\">\n"
          "<rect width=\"900\" height=\"800\" fill=\"white\"/>\n"
          "<defs><clipPath id=\""<<clip<<"\"><rect x=\"30\" y=\"80\" width=\"840\" height=\"680\"/></clipPath></defs>\n";
    std::string heading=data.source_title.empty()?"spherical harmonic illustration":data.source_title;
    if(!panel.empty())heading=panel+"   "+(data.harmonic?
        (data.harmonic->degree==1?"Monopole and dipole":"Monopole and quadrupole"):
        (data.kind=="circle"?"Circular trajectory within the sphere":"Sinusoidal trajectory within the sphere"));
    out<<"<text x=\"450\" y=\"34\" text-anchor=\"middle\" font-family=\"sans-serif\" font-size=\"20\">"
       <<xml(heading)<<"</text>\n<text x=\"450\" y=\"60\" text-anchor=\"middle\" font-size=\"14\">"<<xml(data.kind);
    if(data.harmonic)out<<" degree="<<data.harmonic->degree<<" order="<<data.harmonic->order;
    out<<"</text>\n<g clip-path=\"url(#"<<clip<<")\">\n";
    if(paper_trajectory)trajectoryAxes(out,view,data.reverse_x_tick_labels);
    for(const auto& element:elements)out<<element.svg;
    out<<"</g>\n<metadata>source script blob: "<<xml(data.source_script_blob)
       <<"; column-major mesh; orthographic equal-scale projection; flat shading and depth sorting; "
         "MATLAB Phong lighting/pixel equivalence not asserted; azimuth="<<data.azimuth_degrees
       <<"; elevation="<<data.elevation_degrees<<"; zoom="<<data.camera_zoom;
    if(!data.view_reference.empty())out<<"; view source: "<<xml(data.view_reference);
    for(const auto& helper:data.helper_source_blobs)out<<"; helper blob: "<<xml(helper);
    out<<"</metadata>\n</svg>\n";
    return out.str();
}
Files invalid(const Failure<NumericalErrorCode>& cause) {
    ErrorDetail d;d.stage=ErrorStage::Plot;d.message="Invalid illustration data";
    return Files::failure(wrapFailure(PlotErrorCode::InvalidSeries,std::move(d),cause));
}
} // namespace
Result<std::string,NumericalErrorCode> paperPairSvg(const Data& left,const Data& right) {
    using Output=Result<std::string,NumericalErrorCode>;
    for(const Data* data:{&left,&right}) {
        auto valid=detail::validate(*data);
        if(!valid.hasValue())return Output::failure(valid.error());
    }
    if(!((left.kind=="circle"&&right.kind=="sinusoidal")||
         (left.harmonic&&right.harmonic&&left.harmonic->degree==1&&right.harmonic->degree==2)))
        return Output::failure(detail::failure(NumericalErrorCode::InvalidShape,"Unexpected paper panel pair"));
    try {
        return Output::success("<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 1800 800\">"+
            render(left,"A")+"<g transform=\"translate(900 0)\">"+render(right,"B")+"</g></svg>\n");
    }catch(const std::invalid_argument& e) {
        return Output::failure(detail::failure(NumericalErrorCode::NonFiniteResult,e.what()));
    }
}
Files SvgExporter::exportSvg(const Data& data,ITextPublisher& publisher) {
    auto valid=detail::validate(data);
    if(!valid.hasValue())return invalid(valid.error());
    try {
        auto json=dataJson(data);
        if(!json.hasValue())return invalid(json.error());
        const auto svg=render(data); // 完成两个载荷后才开始发布。
        for(const auto& file:std::array<std::pair<std::string,std::string>,2>{{
                {"illustration.json",std::move(json).value()},{"illustration.svg",svg}}}) {
            auto status=publisher.publishNew(file.first,file.second);
            if(!status.hasValue()) {
                ErrorDetail d;d.stage=ErrorStage::Plot;d.message="Illustration publication failed: "+file.first;
                const auto code=status.error().code==MatErrorCode::OutputExists||
                    status.error().code==MatErrorCode::OutputPathForbidden?PlotErrorCode::OutputForbidden:
                                                                         PlotErrorCode::WriteFailed;
                return Files::failure(wrapFailure(code,std::move(d),status.error()));
            }
        }
        return Files::success({"illustration.json","illustration.svg"});
    }catch(const std::invalid_argument& e) {
        return invalid(detail::failure(NumericalErrorCode::NonFiniteResult,e.what()));
    }catch(const std::exception& e) {
        ErrorDetail d;d.stage=ErrorStage::Plot;d.message=std::string("Illustration SVG failure: ")+e.what();
        return Files::failure({PlotErrorCode::WriteFailed,std::move(d)});
    }
}
} // namespace egomotion::illustrations
