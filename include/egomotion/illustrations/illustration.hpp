#pragma once

#include "egomotion/analysis/plot.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace egomotion::illustrations {

struct CircleConfig {};
struct SinusoidalConfig {};
struct HarmonicConfig { int degree=0; int order=0; };
using Config=std::variant<CircleConfig,SinusoidalConfig,HarmonicConfig>;

struct SurfaceGrid {
    std::size_t rows=0,columns=0;
    std::string row_angle_name,column_angle_name;
    std::vector<double> row_angles,column_angles; // 弧度
    std::vector<Vector3> vertices; // MATLAB列主序：column*rows+row
    // 仅源脚本有意设置的z=NaN遮罩可序列化为JSON null。
    std::vector<bool> z_masked;
};
struct Data {
    std::string kind,source_script_blob,source_title;
    std::vector<std::string> helper_source_blobs;
    SurfaceGrid surface;
    std::vector<Vector3> trajectory;
    std::optional<HarmonicConfig> harmonic;
    std::vector<double> unnormalized_yy,rho; // 同一列主序网格
    double amplitude_scale=0;
    double azimuth_degrees=0,elevation_degrees=0,camera_zoom=1;
};

using DataResult=Result<Data,NumericalErrorCode>;
// 纯数值几何，无MAT、求解器、RNG、绘图或文件输出副作用。
DataResult generate(const Config&);
Result<std::string,NumericalErrorCode> dataJson(const Data&);

class IExporter {
public:
    virtual ~IExporter()=default;
    virtual Result<std::vector<std::string>,PlotErrorCode> exportSvg(
        const Data&,ITextPublisher&)=0;
};
class SvgExporter final:public IExporter {
public:
    Result<std::vector<std::string>,PlotErrorCode> exportSvg(
        const Data&,ITextPublisher&) override;
};
} // namespace egomotion::illustrations
