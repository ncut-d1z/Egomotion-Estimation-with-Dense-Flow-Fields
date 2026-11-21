#include "study.hpp"

#include "egomotion/contracts/diagnostics.hpp"
#include "egomotion/experiments/production_runner.hpp"
#include "egomotion/sampling/random.hpp"

#include <array>

namespace egomotion::paper {
namespace {
using namespace experiments;
constexpr unsigned kBiasTrials=40;
constexpr int kBiasMaximumLevel=9;

void metric(std::ostream& out,const PaperMetric& value) {
    if(value.hasValue())out<<value.value()<<"\tok";
    else out<<"NA\t"<<errorName(value.error().code);
}

std::string biasTable(const BiasTrial& trial) {
    auto out=table();
    out<<"seed\tlevel\tnoise_column\tnoise_multiplier\tN\tsource_xwerte\tywerte_number"
           "\tsource_chord\tdirection_degrees\tdirection_status\n";
    for(const auto& s:trial.samples) {
        out<<trial.seed<<'\t'<<s.level<<'\t'<<s.noise_column<<'\t'<<s.noise_multiplier<<'\t'
           <<s.direction_count<<'\t'<<s.source_xwerte<<'\t'<<s.ywerte_number<<'\t'<<s.source_chord<<'\t';
        metric(out,s.direction_degrees);out<<'\n';
    }
    return out.str();
}

std::string adaptiveTable(const AdaptiveCondition& condition) {
    auto out=table();
    out<<"repetition\tmatlab_frame\tbaseline\tN\tperiod\trefreshed\tideal_rotation_mean"
           "\tideal_translation_mean\tideal_flow_ratio\tratio_status\ttranslation_degrees"
           "\ttranslation_status\trotation_axis_degrees\taxis_status\trotation_geodesic_degrees\tgeodesic_status\n";
    for(const auto& s:condition.samples) {
        out<<s.repetition<<'\t'<<s.matlab_frame<<'\t'<<s.baseline<<'\t'<<s.direction_count<<'\t'
           <<s.period<<'\t'<<s.refreshed<<'\t'<<s.ideal_rotation_mean<<'\t'<<s.ideal_translation_mean<<'\t';
        metric(out,s.ideal_flow_ratio);out<<'\t';metric(out,s.translation_degrees);out<<'\t';
        metric(out,s.rotation_axis_degrees);out<<'\t';metric(out,s.rotation_geodesic_degrees);out<<'\n';
    }
    return out.str();
}

// 每次独立试验先保存已求解样本和求解状态。中途失败也保留前缀与原因，
// 其余独立试验继续；有任何失败则整图不发布均值或成功回执。
struct Trials {
    Study study;
    Files& files;
    std::optional<Failure<ExperimentErrorCode>> first_failure;
    std::size_t attempted=0,failed=0;
    template<class R>
    void record(const std::string& name,const R& result,const std::string& rows) {
        ++attempted;
        if(!result.hasValue()) {
            ++failed;
            files.save(name+"-failure.json",jsonFailure(result.error(),5)+"\n");
            if(!first_failure)first_failure=result.error();
        }
        files.save(name+".tsv",rows);
    }

    void bias() {
        const estimators::FranzMatchedFilter franz;
        const estimators::OriginalKvDEstimator original;
        const estimators::ModifiedKvDEstimator modified;
        const BiasEstimators solvers{franz,original,modified};
        const sampling::OccludedOctahedralSampler masked;
        const sampling::OctahedralSampler full;
        const simulation::MeanMagnitudeGaussianNoise mean;
        const simulation::ComponentProportionalGaussianNoise component;
        const BiasExperimentRunner occluded(masked,mean,solvers),proportional(full,component,solvers);
        for(bool is_occluded:{true,false}) {
            const auto& runner=is_occluded?occluded:proportional;
            for(std::uint32_t seed=1;seed<=kBiasTrials;++seed) {
                BiasTrial trial{is_occluded,seed,{}};
                sampling::Mt19937RandomSource rng(seed);
                auto result=runner.compute({seed,kBiasMaximumLevel},rng,{},
                    [&](const BiasPaperSample& sample){trial.samples.push_back(sample);});
                const std::string prefix=is_occluded?"occluded-":"proportional-";
                record(prefix+std::to_string(seed),result,biasTable(trial));
                study.bias.push_back(std::move(trial));
            }
        }
    }

    void adaptive(const FigureConfig& config) {
        const sampling::IcosahedralSampler sampler;
        const simulation::MeanMagnitudeGaussianNoise noise;
        const trajectory::TrajectoryGenerator generator;
        const trajectory::TrajectoryKinematics kinematics;
        const trajectory::SphereEnvironment environment;
        const AdaptiveSampling services{sampler,noise};
        const AdaptiveScene scene{generator,kinematics,environment};
        auto collect=[&](const std::string& name,std::size_t repetitions,auto run) {
            AdaptiveCondition condition{name,repetitions,{}};
            auto result=run([&](const AdaptivePaperSample& s){condition.samples.push_back(s);});
            record(name,result,adaptiveTable(condition));
            study.adaptive.push_back(std::move(condition));
        };
        if(config.figure==2) {
            collect("initialization",1,[&](const AdaptivePaperObserver& observe){
                return InitializationRunner(services,scene).compute({}, {},observe);
            });
            return;
        }
        for(double noise_scale:{0.0,0.1}) {
            // 成对噪声条件从同一明确种子开始，各条件内八轮连续消费。
            sampling::Mt19937RandomSource rng(*config.comparison_seed);
            collect(noise_scale==0?"comparison-noise-0":"comparison-noise-0p1",8,
                [&](const AdaptivePaperObserver& observe){
                    return AdaptiveComparisonRunner(services,scene).compute({noise_scale},rng,{},observe);
                });
        }
        collect(*config.schedule==PaperUpdateSchedule::Article?"update-article":"update-archive",1,
            [&](const AdaptivePaperObserver& observe){
                return UpdateRateRunner(services,scene).observePaperSchedule(*config.schedule,observe);
            });
    }

    void geometry(unsigned figure) {
        using namespace illustrations;
        // Fig. 4 只确定次数。两幅均选实部 m=1 作示例，保留 l,m 和网格。
        const std::array<Config,2> configs=figure==3?
            std::array<Config,2>{CircleConfig{},SinusoidalConfig{}}:
            std::array<Config,2>{HarmonicConfig{1,1},HarmonicConfig{2,1}};
        for(const auto& config:configs) {
            auto data=generate(config);
            if(!data.hasValue())propagate(ExperimentErrorCode::DataMismatch,ErrorStage::Simulation,
                "Cannot generate paper illustration",data.error());
            auto scene=std::move(data).value();
            if(figure==3) {
                // Fig. 3A/B 第 15 页底轴投影给出的整度近似。归档轨迹脚本未指定 view。
                constexpr double kCircleAzimuth=60,kCircleElevation=24;
                constexpr double kSinusoidalAzimuth=55,kSinusoidalElevation=22;
                const bool circle=std::holds_alternative<CircleConfig>(config);
                scene.azimuth_degrees=circle?kCircleAzimuth:kSinusoidalAzimuth;
                scene.elevation_degrees=circle?kCircleElevation:kSinusoidalElevation;
                scene.view_reference="10.1371/journal.pone.0128413.g003 (estimated from published axes)";
            }
            auto json=dataJson(scene);
            if(!json.hasValue())propagate(ExperimentErrorCode::DataMismatch,ErrorStage::Output,
                "Cannot encode paper geometry",json.error());
            files.save(study.geometry.empty()?"geometry-A.json":"geometry-B.json",json.value());
            study.geometry.push_back(std::move(scene));
            ++attempted;
        }
    }
};
} // namespace

std::vector<std::string> sourceBlobs(unsigned figure) {
    switch(figure) {
    case 1:return {"00a3e974fbf068ccd1af4e541a659862beffcad5","fc3c4f0a7a417171415fb5e3d869a744ba24d04f",
                   "112aa9eca1fa9728f493d31fc8e59620a8445230"};
    case 2:return {"a3154ae391be82ccf48c9f3a03c55109dde85f65","8816675ef5a4eb6767919df36ee11f95de36dac6"};
    case 3:return {"c6bdbff07cdfff796c105f299fb728b15783ac7c","06022612e9249c532f4dabae5658efa9ece4c10d"};
    case 4:return {"d7f97e0fe15c761f33f6ebd152610ec4144561e9"};
    case 5:return {"c9548c83fd0a2ce13d96394e98f6c7573119a7d5","4629901c1233cdee5d6d22c917603720ffd1fab4",
                   "3924038f0ccd70de5ea7b60c589afe166da9c8fb","67cb613328dec10105a6c383ce17025c4901e4c9",
                   "eced66659205a6386ec84723e37a46517d06c017"};
    default:return {};
    }
}

bool svgAvailable() noexcept { return EGOMOTION_ENABLE_SVG!=0; }

FigureFiles produce(const FigureConfig& config,ITextPublisher& publisher) {
    try {
        if(config.figure<1||config.figure>5||
           config.comparison_seed.has_value()!=(config.figure==5)||
           config.schedule.has_value()!=(config.figure==5))
            invalid("Paper Fig. 5 requires a seed and schedule; other figures take neither");
        if(config.svg && !svgAvailable())invalid("Paper SVG backend is unavailable");
        if(config.schedule && *config.schedule!=experiments::PaperUpdateSchedule::Archive &&
           *config.schedule!=experiments::PaperUpdateSchedule::Article)invalid("Unknown update schedule");
        Files files{publisher,{}};
        Trials trials{{},files,std::nullopt};
        if(config.figure==1)trials.bias();
        else if(config.figure==2||config.figure==5)trials.adaptive(config);
        else trials.geometry(config.figure);
        files.save("case-counts.json","{\"attempted\":"+std::to_string(trials.attempted)+
            ",\"completed\":"+std::to_string(trials.attempted-trials.failed)+
            ",\"failed\":"+std::to_string(trials.failed)+"}\n");
        if(trials.first_failure)return FigureFiles::failure(std::move(*trials.first_failure));
        publishFigures(config,trials.study,files);
        return FigureFiles::success(std::move(files.names));
    } catch(const PaperFailure& failure) {
        return FigureFiles::failure(failure.reason);
    }
}
} // namespace egomotion::paper
