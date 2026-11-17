#include "detail.hpp"

#include <Eigen/Geometry>
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
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
    Vector3 project(const Vector3& p)const {
        return {450+scale*right.dot(p),420-scale*up.dot(p),forward.dot(p)};
    }
};
Camera camera(const Data& data) {
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
    constexpr double kViewportRadius=290; // SVG显示尺寸，不改变源数值几何。
    out.scale=kViewportRadius*data.camera_zoom/radius;
    if(!std::isfinite(out.scale))throw std::invalid_argument("Illustration projection scale overflow");
    return out;
}
struct Primitive {double depth;std::string svg;};
Primitive triangle(const Vector3& a,const Vector3& b,const Vector3& c,
                   const Camera& view,std::size_t face) {
    const std::array<Vector3,3> points{{view.project(a),view.project(b),view.project(c)}};
    Vector3 normal=(b-a).cross(c-a);
    const double length=normal.norm();
    const double light=length>0?std::abs(normal.dot(view.forward)/length):0;
    const int green=static_cast<int>(130+95*light),blue=static_cast<int>(160+85*light);
    std::ostringstream out;out.imbue(std::locale::classic());out<<std::fixed<<std::setprecision(3);
    out<<"<polygon data-face=\""<<face<<"\" fill=\"rgb(70,"<<green<<','<<blue
       <<")\" stroke=\"#527286\" stroke-width=\"0.25\" points=\"";
    for(const auto& p:points)out<<p.x()<<','<<p.y()<<' ';
    out<<"\"/>\n";
    return {(points[0].z()+points[1].z()+points[2].z())/3,out.str()};
}
std::vector<Primitive> primitives(const Data& data,const Camera& view) {
    std::vector<Primitive> out;
    const auto& s=data.surface;
    for(std::size_t col=0;col+1<s.columns;++col)
        for(std::size_t row=0;row+1<s.rows;++row) {
            const auto a=col*s.rows+row,b=a+1,c=a+s.rows+1,d=a+s.rows;
            if(s.z_masked[a]||s.z_masked[b]||s.z_masked[c]||s.z_masked[d])continue;
            const auto face=col*(s.rows-1)+row;
            out.push_back(triangle(s.vertices[a],s.vertices[b],s.vertices[c],view,face));
            out.push_back(triangle(s.vertices[a],s.vertices[c],s.vertices[d],view,face));
        }
    for(std::size_t i=1;i<data.trajectory.size();++i) {
        const auto a=view.project(data.trajectory[i-1]),b=view.project(data.trajectory[i]);
        std::ostringstream line;line.imbue(std::locale::classic());line<<std::fixed<<std::setprecision(3);
        line<<"<line data-trajectory-segment=\""<<i<<"\" x1=\""<<a.x()<<"\" y1=\""<<a.y()
            <<"\" x2=\""<<b.x()<<"\" y2=\""<<b.y()<<"\" stroke=\"#ab2637\" stroke-width=\"2.4\"/>\n";
        out.push_back({(a.z()+b.z())/2,line.str()});
    }
    std::stable_sort(out.begin(),out.end(),[](const auto& a,const auto& b){return a.depth<b.depth;});
    return out;
}
std::string render(const Data& data) {
    const auto view=camera(data);
    const auto elements=primitives(data,view);
    std::ostringstream out;out.imbue(std::locale::classic());
    out<<"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"900\" height=\"800\" viewBox=\"0 0 900 800\">\n"
          "<rect width=\"900\" height=\"800\" fill=\"white\"/>\n"
          "<defs><clipPath id=\"scene\"><rect x=\"30\" y=\"80\" width=\"840\" height=\"680\"/></clipPath></defs>\n";
    const auto heading=data.source_title.empty()?"spherical harmonic illustration":data.source_title;
    out<<"<text x=\"450\" y=\"34\" text-anchor=\"middle\" font-family=\"sans-serif\" font-size=\"20\">"
       <<xml(heading)<<"</text>\n<text x=\"450\" y=\"60\" text-anchor=\"middle\" font-size=\"14\">"<<xml(data.kind);
    if(data.harmonic)out<<" degree="<<data.harmonic->degree<<" order="<<data.harmonic->order;
    out<<"</text>\n<g clip-path=\"url(#scene)\">\n";
    for(const auto& element:elements)out<<element.svg;
    out<<"</g>\n<metadata>source script blob: "<<xml(data.source_script_blob)
       <<"; column-major mesh; orthographic equal-scale projection; flat shading and depth sorting; "
         "MATLAB Phong lighting/pixel equivalence not asserted; azimuth="<<data.azimuth_degrees
       <<"; elevation="<<data.elevation_degrees<<"; zoom="<<data.camera_zoom;
    for(const auto& helper:data.helper_source_blobs)out<<"; helper blob: "<<xml(helper);
    out<<"</metadata>\n</svg>\n";
    return out.str();
}
Files invalid(const Failure<NumericalErrorCode>& cause) {
    ErrorDetail d;d.stage=ErrorStage::Plot;d.message="Invalid illustration data";
    return Files::failure(wrapFailure(PlotErrorCode::InvalidSeries,std::move(d),cause));
}
} // namespace
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
