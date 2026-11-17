// L3：轨迹直接标量公式；关联勒让德用Rodrigues有限多项式，独立于生产递推。
#include "egomotion/illustrations/illustration.hpp"
#include "egomotion/cli/command.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <limits>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace {
using namespace egomotion;
namespace ill=illustrations;
std::size_t checks=0,failures=0;
long double max_absolute=0,max_scaled=0;
void check(bool value,const std::string& why) {
    ++checks;if(!value){++failures;std::cerr<<"FAIL "<<why<<'\n';}
}
void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
void close(double actual,long double expected,const std::string& why) {
    const long double difference=std::abs(static_cast<long double>(actual)-expected);
    max_absolute=std::max(max_absolute,difference);
    max_scaled=std::max(max_scaled,difference/(1+std::abs(expected)));
    // 工程回归阈值；不是DEC-005正式验收或MATLAB运行证据。
    check(std::isfinite(actual)&&difference<=1e-9L*(1+std::abs(expected)),why);
}
#if EGOMOTION_ENABLE_SVG
std::size_t occurrences(const std::string& text,const std::string& needle) {
    std::size_t count=0,pos=0;
    while((pos=text.find(needle,pos))!=std::string::npos){++count;pos+=needle.size();}
    return count;
}
struct Publisher final:ITextPublisher {
    std::map<std::string,std::string> files;
    std::size_t fail_at=0,calls=0;
    MatErrorCode code=MatErrorCode::WriteFailed;
    Status<MatErrorCode> publishNew(std::string_view name,std::string_view value) override {
        ++calls;
        if(calls==fail_at) {
            ErrorDetail detail;detail.stage=ErrorStage::Output;detail.message="Injected publication failure";
            return Status<MatErrorCode>::failure({code,std::move(detail)});
        }
        require(files.emplace(std::string(name),std::string(value)).second,"Unexpected overwrite");
        return Status<MatErrorCode>::success();
    }
};
#endif
ill::Data generated(const ill::Config& config) {
    auto result=ill::generate(config);require(result.hasValue(),"Geometry generation failed");
    return std::move(result).value();
}
void trajectory(bool sinusoidal) {
    const auto data=sinusoidal?generated(ill::SinusoidalConfig{}):generated(ill::CircleConfig{});
    const auto& grid=data.surface;
    check(data.source_title=="circular trajectory within the sphere","Original title retained for both paths");
    check(data.helper_source_blobs.size()==1,"Trajectory helper provenance");
    check(grid.rows==51&&grid.columns==51&&grid.vertices.size()==2601,"sphere(50) shape");
    check(data.trajectory.size()==600,"All 600 trajectory samples");
    const long double pi=std::acos(-1.L);
    for(std::size_t j=0;j<600;++j) {
        const long double fraction=static_cast<long double>(j)/600;
        const auto& p=data.trajectory[j];
        close(p.x(),sinusoidal?fraction-.5L:.6L*std::cos(2*pi*fraction),"Trajectory x");
        close(p.y(),sinusoidal?.5L*std::sin(4*pi*fraction):.6L*std::sin(2*pi*fraction),"Trajectory y");
        close(p.z(),.3L,"Trajectory z");
    }
    check(data.trajectory.front()!=data.trajectory.back(),"No synthetic duplicate final point");
    std::size_t masked=0;
    for(std::size_t col=0;col<51;++col)for(std::size_t row=0;row<51;++row) {
        const auto index=col*51+row;
        const long double el=pi*(static_cast<long double>(row)/50-.5L);
        const long double az=pi*(static_cast<long double>(col)/25-1);
        const bool boundary=row==0||row==50||col==0||col==25||col==50;
        const bool mask=col<=25||col==50||row==0||row==50;
        const long double radius=row==0||row==50?0:std::cos(el);
        const auto& p=grid.vertices[index];
        close(grid.row_angles[row],el,"Latitude");close(grid.column_angles[col],az,"Longitude");
        close(p.x(),radius*std::cos(az),"Sphere x unaffected by mask");
        close(p.y(),boundary?0:radius*std::sin(az),"Sphere y unaffected by mask");
        check(grid.z_masked[index]==mask,"y<=0 mask at column-major index");
        if(mask){++masked;check(std::isnan(p.z()),"Only masked z is NaN");}
        else {close(p.z(),std::sin(el),"Sphere z");close(p.squaredNorm(),1,"Unit sphere");}
    }
    check(masked==1425,"Exact geometric meridian/pole mask count");
}
long double factorial(int n) {long double out=1;for(int i=2;i<=n;++i)out*=i;return out;}
long double rodrigues(int degree,int order,long double x) {
    // d^m/dx^m P_l(x)，由Rodrigues展开后逐项求导；不调用生产递推或Eigen。
    long double sum=0;
    for(int k=0;k<=(degree-order)/2;++k) {
        const auto sign=k%2==0?1.L:-1.L;
        const int power=degree-2*k-order;
        sum+=sign*factorial(2*degree-2*k)*std::pow(x,power)/
            (factorial(k)*factorial(degree-k)*factorial(power));
    }
    return (order%2==0?1.L:-1.L)*std::pow(1-x*x,order/2.L)*sum/std::pow(2.L,degree);
}
void harmonic(int degree,int order) {
    const auto data=generated(ill::HarmonicConfig{degree,order});
    check(data.surface.rows==41&&data.surface.columns==41,"41x41 harmonic mesh");
    check(data.trajectory.empty()&&data.source_title.empty(),"Harmonic has no trajectory/source title");
    const long double pi=std::acos(-1.L);
    std::vector<long double> yy;long double scale=0;
    for(int col=0;col<=40;++col)for(int row=0;row<=40;++row) {
        const long double theta=pi*row/40,phi=pi*col/20;
        const long double value=rodrigues(degree,order,std::cos(theta))*std::cos(order*phi);
        yy.push_back(value);scale=std::max(scale,std::abs(value));
    }
    close(data.amplitude_scale,scale,"Full sampled unnormalized scale");
    for(std::size_t i=0;i<yy.size();++i) {
        const long double theta=pi*(i%41)/40,phi=pi*(i/41)/20;
        const long double radius=5+3*yy[i]/scale;
        close(data.unnormalized_yy[i],yy[i],"Independent Rodrigues YY");
        close(data.rho[i],radius,"Original radial mapping");
        close(data.surface.vertices[i].x(),radius*std::sin(theta)*std::cos(phi),"Harmonic x");
        close(data.surface.vertices[i].y(),radius*std::sin(theta)*std::sin(phi),"Harmonic y");
        close(data.surface.vertices[i].z(),radius*std::cos(theta),"Harmonic z");
        check(!data.surface.z_masked[i],"Harmonic surface unmasked");
    }
    if(degree==0)for(double radius:data.rho)check(radius==8,"Constant harmonic radius eight");
    if(degree==1&&order==1)close(data.unnormalized_yy[20],-1,"Condon-Shortley odd-order sign");
}
#if EGOMOTION_ENABLE_SVG
void drawing(const ill::Config& config) {
    const auto data=generated(config);
    const auto before=ill::dataJson(data);require(before.hasValue(),"Valid numeric JSON");
    Publisher publisher;ill::SvgExporter exporter;
    const auto result=exporter.exportSvg(data,publisher);
    require(result.hasValue(),"Valid SVG export");
    check(result.value()==std::vector<std::string>{"illustration.json","illustration.svg"},"Exact publication list");
    check(ill::dataJson(data).value()==before.value(),"Export does not change numeric dataset");
    check(publisher.files.at("illustration.json")==before.value(),"Published JSON is the headless dataset");
    const auto& svg=publisher.files.at("illustration.svg");
    check(occurrences(svg,"<polygon ")== (data.harmonic?3200:2208),"All unmasked surface triangles");
    check(occurrences(svg,"<line ")== (data.harmonic?0:599),"All trajectory segments");
    check(svg.find(data.source_script_blob)!=std::string::npos,"Source script in SVG metadata");
    for(const auto& blob:data.helper_source_blobs)
        check(svg.find(blob)!=std::string::npos,"Helper source in SVG metadata");
    check(svg.find("clip-path=\"url(#scene)\"")!=std::string::npos,"Scene clipping");
    check(svg.find("nan")==std::string::npos&&svg.find("inf")==std::string::npos,"No nonfinite SVG coordinates");
    const std::regex polygon("<polygon [^>]*points=\"([^\"]+)\"");
    for(std::sregex_iterator it(svg.begin(),svg.end(),polygon),end;it!=end;++it) {
        std::istringstream coordinates((*it)[1].str());double x=0,y=0;char comma=0;
        std::size_t vertices=0;
        while(coordinates>>x>>comma>>y) {
            ++vertices;
            check(comma==','&&x>=30&&x<=870&&y>=80&&y<=760,
                  "Entire source mesh fits SVG scene after original zoom");
        }
        check(vertices==3,"Each emitted polygon has three vertices");
    }
    check(occurrences(before.value(),"null")== (data.harmonic?0:1426),"JSON null only source z mask and absent harmonic");
    for(const auto code:{MatErrorCode::WriteFailed,MatErrorCode::OutputExists,MatErrorCode::OutputPathForbidden}) {
        Publisher failed;failed.fail_at=2;failed.code=code;
        const auto error=exporter.exportSvg(data,failed);
        check(!error.hasValue()&&error.error().code==
            (code==MatErrorCode::WriteFailed?PlotErrorCode::WriteFailed:PlotErrorCode::OutputForbidden),
            "Publication failure mapped, never reported complete");
        check(failed.files.size()==1&&failed.files.at("illustration.json")==before.value(),"First file preserved after second failure");
    }
}
#endif
void rejectedData(const ill::Data& data) {
    check(!ill::dataJson(data).hasValue(),"Invalid numeric data rejected");
#if EGOMOTION_ENABLE_SVG
    Publisher publisher;ill::SvgExporter exporter;
    auto result=exporter.exportSvg(data,publisher);
    check(!result.hasValue()&&result.error().code==PlotErrorCode::InvalidSeries,"Invalid drawing classified");
    check(publisher.calls==0,"Invalid drawing never publishes partial file");
#endif
}
void negative() {
    for(auto config:{ill::HarmonicConfig{-1,0},ill::HarmonicConfig{2,-1},ill::HarmonicConfig{1,2}}) {
        const auto result=ill::generate(config);
        check(!result.hasValue()&&result.error().code==NumericalErrorCode::InvalidShape,"Invalid degree/order rejected");
    }
    auto overflow=ill::generate(ill::HarmonicConfig{200,200});
    check(!overflow.hasValue()&&overflow.error().code==NumericalErrorCode::NonFiniteResult,"Unnormalized overflow explicit");
    const auto circle=generated(ill::CircleConfig{}),harmonic=generated(ill::HarmonicConfig{2,1});
    auto data=circle;data.surface.rows=0;rejectedData(data);
    data=circle;data.surface.vertices[0].x()=std::numeric_limits<double>::infinity();rejectedData(data);
    data=circle;data.surface.vertices[0].z()=0;rejectedData(data);
    data=circle;data.surface.z_masked[0]=false;data.surface.vertices[0].z()=0;rejectedData(data);
    data=circle;data.surface.vertices[26*51+25].z()=std::numeric_limits<double>::quiet_NaN();rejectedData(data);
    data=circle;data.trajectory[0].y()=std::numeric_limits<double>::quiet_NaN();rejectedData(data);
    data=harmonic;data.amplitude_scale=0;rejectedData(data);
    data=harmonic;data.unnormalized_yy[0]=std::numeric_limits<double>::infinity();rejectedData(data);
    data=harmonic;data.camera_zoom=0;rejectedData(data);
    data=harmonic;data.surface.z_masked[0]=true;rejectedData(data);
}
void optionalBackend() {
    const cli::ApplicationCompositionRoot app;
    std::ostringstream out,err;
    check(cli::run({"illustrate","--kind","circle"},out,err,app)==0&&err.str().empty(),
          "Headless numeric geometry needs no Reader/Runner/plot backend");
    require(out.str()==ill::dataJson(generated(ill::CircleConfig{})).value(),"Headless geometry dataset exact");
    out.str("");err.str("");
    check(cli::run({"illustrate","--kind","circle","--format","svg"},out,err,app)==7&&out.str().empty(),
          "Absent optional SVG backend explicit failure");
}
} // namespace
int main() {
    try {
        trajectory(false);trajectory(true);
        for(int degree=0;degree<=8;++degree)for(int order=0;order<=degree;++order)harmonic(degree,order);
#if EGOMOTION_ENABLE_SVG
        drawing(ill::CircleConfig{});drawing(ill::SinusoidalConfig{});
        drawing(ill::HarmonicConfig{0,0});drawing(ill::HarmonicConfig{3,2});
#endif
        negative();optionalBackend();
        std::cout<<std::setprecision(21)<<"checks="<<checks<<" failures="<<failures
                 <<" max_absolute="<<max_absolute<<" max_scaled="<<max_scaled<<'\n';
        return failures?1:0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
