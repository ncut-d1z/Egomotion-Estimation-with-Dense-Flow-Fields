#include "detail.hpp"
#include "egomotion/contracts/diagnostics.hpp"

#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <new>
#include <sstream>
#include <stdexcept>

namespace egomotion::illustrations {
namespace detail {
Status<NumericalErrorCode> validate(const Data& data) {
    const auto& s=data.surface;
    if(s.rows<2||s.columns<2||s.columns>std::numeric_limits<std::size_t>::max()/s.rows||
       s.vertices.size()!=s.rows*s.columns||s.z_masked.size()!=s.vertices.size()||
       s.row_angles.size()!=s.rows||s.column_angles.size()!=s.columns)
        return Status<NumericalErrorCode>::failure(failure(NumericalErrorCode::InvalidShape,
                                                           "Invalid illustration grid dimensions"));
    if(data.kind!="circle"&&data.kind!="sinusoidal"&&data.kind!="harmonic")
        return Status<NumericalErrorCode>::failure(failure(NumericalErrorCode::InvalidShape,
                                                           "Unknown illustration kind"));
    if((data.kind=="harmonic")!=data.harmonic.has_value()||
       (data.harmonic&&(data.unnormalized_yy.size()!=s.vertices.size()||data.rho.size()!=s.vertices.size()||
                        !data.trajectory.empty()||data.harmonic->degree<0||data.harmonic->order<0||
                        data.harmonic->order>data.harmonic->degree))||
       (!data.harmonic&&(!data.unnormalized_yy.empty()||!data.rho.empty()||data.trajectory.empty())))
        return Status<NumericalErrorCode>::failure(failure(NumericalErrorCode::InvalidShape,
                                                           "Illustration fields do not match kind"));
    for(std::size_t i=0;i<s.vertices.size();++i) {
        const auto& p=s.vertices[i];
        const bool valid_z=s.z_masked[i]?!data.harmonic&&p.y()<=0&&std::isnan(p.z()):
                                        std::isfinite(p.z());
        if(!std::isfinite(p.x())||!std::isfinite(p.y())||!valid_z||
           (!data.harmonic&&s.z_masked[i]!=(p.y()<=0)))
            return Status<NumericalErrorCode>::failure(failure(NumericalErrorCode::NonFiniteResult,
                                                               "Non-finite or invalid source mask in surface"));
    }
    for(const auto& p:data.trajectory)
        if(!p.allFinite())return Status<NumericalErrorCode>::failure(failure(
            NumericalErrorCode::NonFiniteResult,"Non-finite trajectory coordinate"));
    for(const auto* values:{&s.row_angles,&s.column_angles,&data.unnormalized_yy,&data.rho})
        for(double value:*values)
            if(!std::isfinite(value))return Status<NumericalErrorCode>::failure(failure(
                NumericalErrorCode::NonFiniteResult,"Non-finite illustration numeric field"));
    if(!std::isfinite(data.amplitude_scale)||!std::isfinite(data.azimuth_degrees)||
       !std::isfinite(data.elevation_degrees)||!std::isfinite(data.camera_zoom)||data.camera_zoom<=0)
        return Status<NumericalErrorCode>::failure(failure(NumericalErrorCode::NonFiniteResult,
                                                           "Invalid illustration scale or camera"));
    if(data.harmonic&&data.amplitude_scale<=0)
        return Status<NumericalErrorCode>::failure(failure(NumericalErrorCode::ZeroNorm,
                                                           "Illustration sampled harmonic scale is zero"));
    return Status<NumericalErrorCode>::success();
}
} // namespace detail
namespace {
void numbers(std::ostream& out,const std::vector<double>& values) {
    out<<'[';
    for(std::size_t i=0;i<values.size();++i){if(i)out<<',';out<<values[i];}
    out<<']';
}
void strings(std::ostream& out,const std::vector<std::string>& values) {
    out<<'[';
    for(std::size_t i=0;i<values.size();++i){if(i)out<<',';out<<jsonQuote(values[i]);}
    out<<']';
}
void vertices(std::ostream& out,const std::vector<Vector3>& values,const std::vector<bool>* mask=nullptr) {
    out<<'[';
    for(std::size_t i=0;i<values.size();++i) {
        if(i)out<<',';
        const auto& p=values[i];out<<'['<<p.x()<<','<<p.y()<<',';
        if(mask&&(*mask)[i])out<<"null";else out<<p.z();
        out<<']';
    }
    out<<']';
}
} // namespace
Result<std::string,NumericalErrorCode> dataJson(const Data& data) {
    using Json=Result<std::string,NumericalErrorCode>;
    auto valid=detail::validate(data);
    if(!valid.hasValue())return Json::failure(std::move(valid).error());
    try {
        std::ostringstream out;out.imbue(std::locale::classic());
        out<<std::setprecision(std::numeric_limits<double>::max_digits10);
        out<<"{\"schema\":\"egomotion-illustration-v1\",\"kind\":"<<jsonQuote(data.kind)
           <<",\"source_script_blob\":"<<jsonQuote(data.source_script_blob)
           <<",\"source_title\":"<<jsonQuote(data.source_title)<<",\"helper_source_blobs\":";
        strings(out,data.helper_source_blobs);
        out<<",\"surface\":{\"rows\":"<<data.surface.rows<<",\"columns\":"<<data.surface.columns
           <<",\"layout\":\"column-major\",\"row_angle_name\":"<<jsonQuote(data.surface.row_angle_name)
           <<",\"column_angle_name\":"<<jsonQuote(data.surface.column_angle_name)
           <<",\"angle_unit\":\"radian\",\"row_angles\":";
        numbers(out,data.surface.row_angles);out<<",\"column_angles\":";
        numbers(out,data.surface.column_angles);out<<",\"vertices_xyz\":";
        vertices(out,data.surface.vertices,&data.surface.z_masked);
        out<<",\"z_masked\":[";
        for(std::size_t i=0;i<data.surface.z_masked.size();++i){if(i)out<<',';out<<(data.surface.z_masked[i]?"true":"false");}
        out<<"]},\"trajectory_xyz\":";vertices(out,data.trajectory);
        out<<",\"harmonic\":";
        if(data.harmonic) {
            out<<"{\"degree\":"<<data.harmonic->degree<<",\"order\":"<<data.harmonic->order
               <<",\"amplitude_scale\":"<<data.amplitude_scale<<",\"unnormalized_yy\":";
            numbers(out,data.unnormalized_yy);out<<",\"rho\":";numbers(out,data.rho);out<<'}';
        }else out<<"null";
        out<<",\"view\":{\"azimuth_degrees\":"<<data.azimuth_degrees<<",\"elevation_degrees\":"
           <<data.elevation_degrees<<",\"camera_zoom\":"<<data.camera_zoom;
        if(!data.view_reference.empty())out<<",\"source\":"<<jsonQuote(data.view_reference);
        out<<"}}\n";
        if(!out.good())
            return Json::failure(detail::failure(NumericalErrorCode::ResourceLimitExceeded,
                                                "Cannot format complete illustration JSON"));
        return Json::success(out.str());
    }catch(const std::bad_alloc&) {
        return Json::failure(detail::failure(NumericalErrorCode::ResourceLimitExceeded,"Illustration JSON allocation failed"));
    }catch(const std::length_error&) {
        return Json::failure(detail::failure(NumericalErrorCode::ResourceLimitExceeded,"Illustration JSON capacity exceeded"));
    }
}
} // namespace egomotion::illustrations
