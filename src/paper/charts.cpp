#include "charts.hpp"
#include "../analysis/adaptive_windows.hpp"
#include "egomotion/estimators/coefficient_update.hpp"

#include <array>
#include <cmath>
#include <map>
#include <tuple>

namespace egomotion::paper {
namespace {
using experiments::PaperMetric;
constexpr std::size_t kBiasRepeats=40,kBiasFirstLevel=2,kBiasLastLevel=9;
constexpr std::size_t kFirstDisplayedFrame=50,kLastDisplayedFrame=550;
constexpr std::array<std::size_t,4> kHalfWindows{{0,2,5,10}};
constexpr std::array<const char*,4> kPeriodColors{{"black","green","blue","magenta"}};

struct Cell {
    std::size_t seen=0,valid=0;
    double sum=0;
    void add(const PaperMetric& observation,std::size_t repetition) {
        if(repetition!=seen+1)invalid("Missing or repeated paper sample ordinal");
        ++seen;
        if(observation.hasValue()) {
            sum+=observation.value();
            if(!std::isfinite(sum))invalid("Paper sample or fixed-population sum is non-finite");
            ++valid;
        }
    }
    std::optional<double> mean(std::size_t required) const {
        if(seen!=required)invalid("Incomplete paper sample population");
        if(valid!=required)return {};
        return sum/static_cast<double>(required);
    }
};
void meanRow(std::ostream& out,const std::string& condition,std::size_t x,std::size_t slot,
             std::string metric,const Cell& cell,std::size_t repetitions) {
    auto y=cell.mean(repetitions);
    out<<condition<<'\t'<<x<<'\t'<<slot<<'\t'<<metric<<'\t'<<cell.valid<<'\t'<<repetitions<<'\t';
    if(y)out<<*y;else out<<"NA";
    out<<'\n';
}
struct BiasCell {Cell angle; std::size_t directions=0; double noise=0;};
Chart biasChart(const Study& study,std::ostream& means) {
    if(study.bias.size()!=2*kBiasRepeats)invalid("Fig. 1 needs 40 trials in each of two conditions");
    Chart chart{"fig1-source-config",true,{}};
    for(bool occluded:{true,false}) {
        std::map<std::tuple<std::size_t,std::size_t,std::size_t>,BiasCell> cells;
        for(const auto& trial:study.bias)if(trial.occluded==occluded)
            for(const auto& s:trial.samples) {
                auto& bin=cells[{s.level,s.noise_column,s.ywerte_number}];
                if(bin.angle.seen&&(bin.directions!=s.direction_count||bin.noise!=s.noise_multiplier))
                    invalid("Fig. 1 direction count or noise multiplier changed between trials");
                bin.directions=s.direction_count;bin.noise=s.noise_multiplier;
                bin.angle.add(s.direction_degrees,trial.seed);
            }
        constexpr std::size_t kNoiseColumns=3,kKvDMetrics=4;
        if(cells.size()!=(kBiasLastLevel+1)*kNoiseColumns*kKvDMetrics)
            invalid("Fig. 1 output is missing a level, noise column or KvD metric");
        Panel panel{occluded?"A  Occluded directions":"B  Full sphere, proportional noise",{}};
        Curve guide{"100 / sqrt(N)","black",false,false,{}};
        for(std::size_t col=1;col<=3;++col)for(std::size_t y=3;y<=6;++y) {
            Curve curve{y<5?"Original KvD":"Modified KvD",y<5?"red":"green",y%2==0,false,{}};
            curve.label+=(curve.dashed?" rotation, ":" translation, ");
            auto multiplier=table();multiplier<<std::setprecision(2)<<cells.at({0,col,y}).noise;
            curve.label+=multiplier.str();
            for(std::size_t level=0;level<=kBiasLastLevel;++level) {
                const auto& bin=cells.at({level,col,y});
                const double n=static_cast<double>(bin.directions);
                meanRow(means,occluded?"occluded":"proportional",bin.directions,col,
                        "ywerte"+std::to_string(y)+"_direction_degrees",bin.angle,kBiasRepeats);
                if(level<kBiasFirstLevel)continue;
                curve.points.push_back({n,bin.angle.mean(kBiasRepeats)});
                if(col==1&&y==3)guide.points.push_back({n,100/std::sqrt(n)});
            }
            panel.curves.push_back(std::move(curve));
        }
        panel.curves.push_back(std::move(guide));
        chart.panels.push_back(std::move(panel));
    }
    return chart;
}

enum MetricIndex : std::size_t { Translation,Axis,Geodesic };
constexpr std::array<const char*,3> kMetricNames{{
    "translation_degrees","rotation_axis_degrees","rotation_geodesic_degrees"}};
constexpr std::array<const char*,3> kMetricLabels{{
    "Translation direction","Rotation axis","SO(3) rotation distance"}};
using Measurements=std::array<Cell,3>;
struct AdaptiveMeans {
    std::size_t frames,repetitions;
    std::vector<std::vector<Measurements>> lanes;
    std::vector<std::size_t> periods;
    std::vector<std::optional<double>> ratio;
};
AdaptiveMeans adaptiveMeans(const AdaptiveCondition& condition,bool init,bool rates,std::ostream& out) {
    const std::size_t frames=init?35:600, slots=rates?4:2;
    if(condition.repetitions!=(init||rates?1u:8u))invalid("Incorrect planned Adaptive repetitions");
    AdaptiveMeans data{frames,condition.repetitions,
        std::vector<std::vector<Measurements>>(slots,std::vector<Measurements>(frames)),
        std::vector<std::size_t>(slots),std::vector<std::optional<double>>(frames)};
    for(const auto& s:condition.samples) {
        if(s.matlab_frame<2||s.matlab_frame>=frames||s.baseline==0||s.baseline>slots)
            invalid("Adaptive paper sample outside solved frames or baseline range");
        const auto j=s.baseline-1,row=s.matlab_frame-1;
        auto& cells=data.lanes[j][row];
        cells[Translation].add(s.translation_degrees,s.repetition);
        cells[Axis].add(s.rotation_axis_degrees,s.repetition);
        cells[Geodesic].add(s.rotation_geodesic_degrees,s.repetition);
        if(data.periods[j] && data.periods[j]!=s.period)invalid("Inconsistent paper update period");
        data.periods[j]=s.period;
        if(j==0 && s.repetition==data.repetitions && s.ideal_flow_ratio.hasValue())
            data.ratio[row]=s.ideal_flow_ratio.value();
    }
    for(std::size_t j=0;j<slots;++j)for(std::size_t frame=2;frame<frames;++frame)
        for(std::size_t m=0;m<kMetricNames.size();++m)
            meanRow(out,condition.name,frame,j+1,kMetricNames[m],data.lanes[j][frame-1][m],data.repetitions);
    return data;
}

using Segment=analysis::detail::Segment;
std::array<Segment,4> rotationWindows(const AdaptiveMeans& data,bool rates) {
    std::vector<double> ratio(data.frames);
    for(std::size_t frame=kFirstDisplayedFrame;frame<=kLastDisplayedFrame;++frame) {
        if(!data.ratio.at(frame-1))invalid("Missing ideal flow ratio at source frame "+std::to_string(frame));
        ratio[frame-1]=*data.ratio[frame-1];
    }
    auto windows=analysis::detail::fourSourceSegments(ratio,rates?0.02:0.01);
    if(!windows.hasValue())propagate(ExperimentErrorCode::DataMismatch,ErrorStage::Analyze,
                                     "Cannot reconstruct source rotation windows",windows.error());
    return windows.value();
}
void appendWindow(Curve& curve,const AdaptiveMeans& data,std::size_t lane,MetricIndex metric,
                  Segment input,Segment plotted,std::size_t half) {
    if(input.end<input.begin || half>(input.end-input.begin)/2 ||
       plotted.begin<input.begin+half||plotted.end>input.end-half||plotted.begin>plotted.end)
        invalid("Invalid paper smoothing or plotting window");
    std::vector<double> values;
    std::vector<unsigned char> mask;
    for(std::size_t frame=input.begin;frame<=input.end;++frame) {
        const auto value=data.lanes.at(lane).at(frame-1)[metric].mean(data.repetitions);
        values.push_back(value.value_or(0));mask.push_back(value.has_value());
    }
    auto smoothed=analysis::detail::smoothSource(values,mask,half,{},kMetricNames[metric],input.begin);
    if(!smoothed.hasValue())propagate(ExperimentErrorCode::DataMismatch,ErrorStage::Analyze,
                                      "Cannot smooth paper metric",smoothed.error());
    for(std::size_t frame=plotted.begin;frame<=plotted.end;++frame) {
        const auto i=frame-input.begin;
        std::optional<double> y;
        if(smoothed.value().defined[i])y=smoothed.value().values[i];
        curve.points.push_back({static_cast<double>(frame),y,frame==plotted.begin});
    }
}
Panel initialization(const AdaptiveMeans& data,MetricIndex rotation) {
    Panel panel{"Initialization",{}};
    const Segment interior{2,data.frames-1};
    for(auto metric:{Translation,rotation}) {
        Curve line{kMetricLabels[metric],metric==Translation?"blue":"red",false,false,{}};
        appendWindow(line,data,1,metric,interior,interior,0);
        panel.curves.push_back(std::move(line));
    }
    return panel;
}
Panel motionPanel(const AdaptiveMeans& data,MetricIndex metric,std::string title,bool rates,
                  const std::array<Segment,4>& windows) {
    Panel panel{std::move(title),{}};
    const Segment displayed{kFirstDisplayedFrame,kLastDisplayedFrame};
    for(std::size_t j=0;j<data.lanes.size();++j) {
        const std::string label=rates?"Period "+std::to_string(data.periods[j]):j==0?"Original MFA":"Adaptive MFA";
        Curve curve{label,rates?kPeriodColors[j]:j==0?"blue":"red",false,false,{}};
        const std::size_t half=rates?kHalfWindows[j]:0;
        if(metric!=Axis) {
            // 仅旋转轴在零旋转处无定义。测地角保留完整 50..550 窗口。
            appendWindow(curve,data,j,metric,{2,data.frames-1},displayed,half);
        } else {
            for(const auto& segment:windows) {
                appendWindow(curve,data,j,metric,segment,{segment.begin+half,segment.end-half},half);
            }
        }
        panel.curves.push_back(std::move(curve));
    }
    if(!rates) {
        Curve ratio{"Ideal rot/trans flow ratio","green",false,true,{}};
        for(std::size_t f=displayed.begin;f<=displayed.end;++f)
            ratio.points.push_back({static_cast<double>(f),data.ratio[f-1]});
        panel.curves.push_back(std::move(ratio));
    }
    return panel;
}

void saveChart(const Chart& chart,bool svg,Files& files) {
    auto out=table();
    out<<"panel\tseries\taxis\tx\ty\tsegment_start\n";
    for(const auto& p:chart.panels)for(const auto& c:p.curves)for(const auto& point:c.points) {
        out<<p.title<<'\t'<<c.label<<'\t'<<(c.right_axis?"right":"left")<<'\t'<<point.x<<'\t';
        if(point.y)out<<*point.y;else out<<"NA";
        out<<'\t'<<point.starts_segment<<'\n';
    }
    files.save(chart.name+"-curves.tsv",out.str());
#if EGOMOTION_ENABLE_SVG
    if(svg)files.save(chart.name+".svg",chartSvg(chart));
#else
    (void)svg;
#endif
}
} // namespace

void publishFigures(const FigureConfig& config,const Study& study,Files& files) {
    if(config.figure==3||config.figure==4) {
#if EGOMOTION_ENABLE_SVG
        if(config.svg) {
            auto svg=illustrations::paperPairSvg(study.geometry.at(0),study.geometry.at(1));
            if(!svg.hasValue())propagate(ExperimentErrorCode::DataMismatch,ErrorStage::Plot,
                                         "Cannot render paper illustration pair",svg.error());
            files.save("fig"+std::to_string(config.figure)+".svg",svg.value());
        }
#endif
        return;
    }
    auto means=table();
    means<<"condition\tx\tcolumn\tmetric\tvalid\tplanned\tmean\n";
    std::vector<Chart> charts;
    if(config.figure==1)charts.push_back(biasChart(study,means));
    else {
        if(study.adaptive.size()!=(config.figure==2?1u:3u))
            invalid("Incomplete set of Adaptive conditions for this figure");
        for(auto rotation:{Axis,Geodesic})
            charts.push_back({"fig"+std::to_string(config.figure)+(rotation==Axis?"-axis":"-geodesic"),false,{}});
        for(std::size_t i=0;i<study.adaptive.size();++i) {
            const bool init=config.figure==2,rates=i==2;
            const auto data=adaptiveMeans(study.adaptive[i],init,rates,means);
            if(rates) {
                const auto& expected=config.schedule==experiments::PaperUpdateSchedule::Article?
                    experiments::kArticleUpdatePeriods:estimators::kAdaptiveUpdatePeriods;
                for(std::size_t j=0;j<expected.size();++j)
                    if(data.periods.at(j)!=expected[j])invalid("Measured update period differs from figure configuration");
            }
            std::array<Segment,4> windows{};
            if(!init)windows=rotationWindows(data,rates);
            for(std::size_t m=0;m<charts.size();++m) {
                auto& chart=charts[m];
                const auto rotation=m==0?Axis:Geodesic;
                if(init)chart.panels.push_back(initialization(data,rotation));
                else {
                    const std::string condition=rates?study.adaptive[i].name:i==0?"noise 0":"noise 0.1";
                    const std::string letter(1,static_cast<char>('A'+i*2));
                    const std::string next(1,static_cast<char>('B'+i*2));
                    chart.panels.push_back(motionPanel(data,Translation,letter+"  Translation, "+condition,rates,windows));
                    chart.panels.push_back(motionPanel(data,rotation,next+"  "+kMetricLabels[rotation]+", "+condition,rates,windows));
                }
            }
        }
        if(config.figure==5&&config.schedule==experiments::PaperUpdateSchedule::Archive) {
            // 同一套实测比值和归档周期，另给源脚本的线性纵轴视图。
            const auto& axis=charts.front().panels;
            charts.push_back({"fig5-update-archive-axis-linear",false,{axis.at(4),axis.at(5)},false});
        }
    }
    files.save("means.tsv",means.str());
    for(const auto& chart:charts)saveChart(chart,config.svg,files);
}
} // namespace egomotion::paper
