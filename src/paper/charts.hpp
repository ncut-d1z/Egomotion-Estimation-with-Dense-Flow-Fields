#pragma once

#include "study.hpp"

namespace egomotion::paper {

struct Point { double x; std::optional<double> y; bool starts_segment=false; };
struct Curve {
    std::string label,color;
    bool dashed=false,right_axis=false;
    std::vector<Point> points;
};
struct Panel { std::string title; std::vector<Curve> curves; };
struct Chart {
    std::string name;
    bool log_x=false;
    std::vector<Panel> panels;
    bool log_y=true;
};

// data 模式与 SVG 模式共用同一组含缺测标记的数值坐标。
std::string chartSvg(const Chart&);
} // namespace egomotion::paper
