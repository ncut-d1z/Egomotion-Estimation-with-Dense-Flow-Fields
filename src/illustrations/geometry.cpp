#include "detail.hpp"
#include "egomotion/trajectory/trajectory.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <new>
#include <stdexcept>

namespace egomotion::illustrations {
namespace {
// 固定参数来自原illustrations三脚本；详见analysis LLD §3.1。
constexpr std::size_t kSphereQuality=50,kHarmonicDivisions=40,kTrajectoryPoints=600;
constexpr double kHeight=0.3,kCircleRadius=0.6,kSinusoidalAmplitude=0.5,kPathLength=1;
constexpr std::size_t kPeriods=2;
constexpr double kRadialBase=5,kRadialAmplitude=3;

SurfaceGrid grid(std::size_t divisions,const std::string& row_name,
                 double row_first,double row_span,const std::string& col_name,
                 double col_first,double col_span) {
    SurfaceGrid out;out.rows=out.columns=divisions+1;
    out.row_angle_name=row_name;out.column_angle_name=col_name;
    for(std::size_t i=0;i<=divisions;++i) {
        const double fraction=static_cast<double>(i)/static_cast<double>(divisions);
        out.row_angles.push_back(row_first+row_span*fraction);
        out.column_angles.push_back(col_first+col_span*fraction);
    }
    out.vertices.resize(out.rows*out.columns);
    out.z_masked.assign(out.vertices.size(),false);
    return out;
}
DataResult trajectoryPicture(bool sinusoidal) {
    const double pi=std::acos(-1.0);
    Data out;out.kind=sinusoidal?"sinusoidal":"circle";
    out.source_script_blob=sinusoidal?"06022612e9249c532f4dabae5658efa9ece4c10d":
                                      "c6bdbff07cdfff796c105f299fb728b15783ac7c";
    out.helper_source_blobs={sinusoidal?"e2857131361619593f13f1bcd2cf2207f1a90493":
                                        "6d4bd8a7d0b7cb516b1ab6e80061617b386977bd"};
    // 正弦原脚本同样使用该标题；用kind另行标明真实轨迹类别。
    out.source_title="circular trajectory within the sphere";
    out.azimuth_degrees=-37.5;out.elevation_degrees=30; // 文档给出的三维默认视角
    out.surface=grid(kSphereQuality,"latitude",-pi/2,pi,"longitude",-pi,2*pi);
    auto& surface=out.surface;
    for(std::size_t col=0;col<surface.columns;++col) {
        const double az=surface.column_angles[col];
        // 以精确子午线/极点零值离散单位球，避免sin(±pi)舍入改变y<=0遮罩；
        // 未取得MATLAB sphere内部端点舍入运行证据，不承诺其逐位相同。
        const double sine=(col==0||col==kSphereQuality||col==kSphereQuality/2)?0:std::sin(az);
        for(std::size_t row=0;row<surface.rows;++row) {
            const double el=surface.row_angles[row];
            const double radius=(row==0||row==kSphereQuality)?0:std::cos(el);
            const auto index=col*surface.rows+row;
            auto& p=surface.vertices[index];
            p=Vector3(radius*std::cos(az),radius*sine,std::sin(el));
            surface.z_masked[index]=p.y()<=0;
            if(surface.z_masked[index])p.z()=std::numeric_limits<double>::quiet_NaN();
        }
    }
    trajectory::TrajectoryGenerator generator;
    auto points=sinusoidal?generator.sinusoidal(kTrajectoryPoints,kSinusoidalAmplitude,
                                               kPathLength,kPeriods,kHeight):
                           generator.circle(kTrajectoryPoints,kCircleRadius,kHeight);
    if(!points.hasValue())return DataResult::failure(std::move(points).error());
    for(Eigen::Index i=0;i<points.value().points.rows();++i)
        out.trajectory.emplace_back(points.value().points.row(i).transpose());
    return DataResult::success(std::move(out));
}
double associatedLegendre(int degree,int order,double x) {
    // MATLAB legendre默认unnorm并含(-1)^m相位。
    // P_m^m=(-1)^m(2m-1)!!(1-x²)^(m/2)；升degree用DLMF 14.10.3。
    if(order>0&&(x==1||x== -1))return 0;
    double previous=1;
    const double root=std::sqrt(1-x*x);
    for(std::int64_t m=1;m<=order;++m) {
        previous*=-(2.0*static_cast<double>(m)-1)*root;
        if(!std::isfinite(previous))return previous;
    }
    if(degree==order)return previous;
    double current=x*(2.0*order+1)*previous;
    for(std::int64_t n=static_cast<std::int64_t>(order)+2;n<=degree;++n) {
        const double nd=static_cast<double>(n);
        const double next=((2*nd-1)*x*current-(nd+order-1)*previous)/(nd-order);
        if(!std::isfinite(next))return next;
        previous=current;current=next;
    }
    return current;
}
DataResult harmonicPicture(const HarmonicConfig& config) {
    if(config.degree<0||config.order<0||config.order>config.degree)
        return DataResult::failure(detail::failure(NumericalErrorCode::InvalidShape,
                                                    "Illustration needs 0 <= order <= degree"));
    const double pi=std::acos(-1.0);
    Data out;out.kind="harmonic";out.harmonic=config;
    out.source_script_blob="d7f97e0fe15c761f33f6ebd152610ec4144561e9";
    out.azimuth_degrees=40;out.elevation_degrees=30;out.camera_zoom=1.5;
    out.surface=grid(kHarmonicDivisions,"theta",0,pi,"phi",0,2*pi);
    const auto& surface=out.surface;
    std::vector<double> legendre;
    for(double theta:surface.row_angles) {
        const double value=associatedLegendre(config.degree,config.order,std::cos(theta));
        if(!std::isfinite(value))return DataResult::failure(detail::failure(
            NumericalErrorCode::NonFiniteResult,"Unnormalized associated Legendre overflow"));
        legendre.push_back(value);
    }
    for(double phi:surface.column_angles)
        for(double value:legendre) {
            const double yy=value*std::cos(config.order*phi);
            out.unnormalized_yy.push_back(yy);
            out.amplitude_scale=std::max(out.amplitude_scale,std::abs(yy));
        }
    if(!(out.amplitude_scale>0))return DataResult::failure(detail::failure(
        NumericalErrorCode::ZeroNorm,"Illustration sampled harmonic scale is zero"));
    for(std::size_t col=0;col<surface.columns;++col)
        for(std::size_t row=0;row<surface.rows;++row) {
            const auto index=col*surface.rows+row;
            // 保留原5+3*yy/scale的double运算顺序；溢出不能改成归一化数学基。
            const double rho=kRadialBase+kRadialAmplitude*out.unnormalized_yy[index]/out.amplitude_scale;
            const double r=rho*std::sin(surface.row_angles[row]);
            out.rho.push_back(rho);
            out.surface.vertices[index]=Vector3(r*std::cos(surface.column_angles[col]),
                r*std::sin(surface.column_angles[col]),rho*std::cos(surface.row_angles[row]));
        }
    return DataResult::success(std::move(out));
}
} // namespace
DataResult generate(const Config& config) {
    try {
        auto result=std::holds_alternative<HarmonicConfig>(config)?
            harmonicPicture(std::get<HarmonicConfig>(config)):
            trajectoryPicture(std::holds_alternative<SinusoidalConfig>(config));
        if(!result.hasValue())return result;
        auto valid=detail::validate(result.value());
        if(!valid.hasValue())return DataResult::failure(std::move(valid).error());
        return result;
    }catch(const std::bad_alloc&) {
        return DataResult::failure(detail::failure(NumericalErrorCode::ResourceLimitExceeded,
                                                    "Illustration allocation failed"));
    }catch(const std::length_error&) {
        return DataResult::failure(detail::failure(NumericalErrorCode::ResourceLimitExceeded,
                                                    "Illustration container capacity exceeded"));
    }
}
} // namespace egomotion::illustrations
