// 合成逐次观测，只核对聚合、缺测、窗口、面板和入口；不执行论文实验。
#include "../../src/paper/study.hpp"
#include "egomotion/cli/command.hpp"
#include "egomotion/experiments/adaptive_runner.hpp"
#include "egomotion/estimators/adaptive_step.hpp"
#include "egomotion/simulation/flow.hpp"
#include "../support/motion_fixture.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <map>
#include <stdexcept>

namespace {
using namespace egomotion;
using namespace egomotion::paper;
using experiments::PaperMetric;
using experiments::PaperUpdateSchedule;

void require(bool ok,const std::string& message){if(!ok)throw std::runtime_error(message);}
struct MemoryPublisher : ITextPublisher {
    std::map<std::string,std::string> contents;
    Status<MatErrorCode> publishNew(std::string_view name,std::string_view data) override {
        require(contents.emplace(std::string(name),std::string(data)).second,"Duplicate paper output name");
        return Status<MatErrorCode>::success();
    }
    void contains(const std::string& file,const std::string& row) const {
        require(contents.at(file).find(row)!=std::string::npos,"Missing record in "+file+": "+row);
    }
};
PaperMetric number(double value){return PaperMetric::success(value);}
PaperMetric missingAxis() {
    ErrorDetail detail;detail.stage=ErrorStage::Solve;detail.message="synthetic zero rotation";
    return PaperMetric::failure({NumericalErrorCode::UndefinedAngle,std::move(detail)});
}
struct SixRays : IDirectionSampler {
    Result<DirectionSet,NumericalErrorCode> sample(int level) const override {
        require(level==2,"UpdateRates requested Comparison sampling");
        DirectionMatrix axes=DirectionMatrix::Zero(6,3);
        for(int i=0;i<3;++i){axes(i,i)=1;axes(i+3,i)=-1;}
        return DirectionSet::fromOwned(std::move(axes));
    }
};
struct UnitDistances : trajectory::ISphereEnvironment {
    Result<Eigen::VectorXd,NumericalErrorCode> nearnessInUnitSphere(
        const DirectionSet& rays,const Vector3&,const Vector3&) const override {
        return Result<Eigen::VectorXd,NumericalErrorCode>::success(Eigen::VectorXd::Ones(rays.directions().rows()));
    }
};
struct KnownMotion : trajectory::ITrajectoryKinematics {
    Result<std::vector<trajectory::FrameKinematics>,NumericalErrorCode>
    fromPoints(const trajectory::TrajectoryPoints& path) const override {
        std::vector<trajectory::FrameKinematics> frames(static_cast<std::size_t>(path.points.rows()));
        for(std::size_t row=1;row+1<frames.size();++row) {
            frames[row].has_original_matlab_motion=true;
            frames[row].local_translation.x()=2;
            frames[row].relative_rotation.z()=row==2?0:1;
        }
        return Result<std::vector<trajectory::FrameKinematics>,NumericalErrorCode>::success(std::move(frames));
    }
};
struct TwoFrames : estimators::IAdaptiveStepSolver {
    mutable std::size_t calls=0;
    Result<estimators::AdaptiveStepResult,NumericalErrorCode>
    solve(const HarmonicCoefficients& old,const FlowField&) const override {
        using Output=Result<estimators::AdaptiveStepResult,NumericalErrorCode>;
        if(++calls==11) return Output::failure({NumericalErrorCode::SingularSystem,{}});
        estimators::AdaptiveStepResult result;
        result.motion=test_support::motionValue(Vector3::UnitX(),Vector3(0,0,2));
        result.coefficients_next=old;
        return Output::success(std::move(result));
    }
};
void actualFrameEvidence() {
    SixRays rays;UnitDistances environment;KnownMotion truth;TwoFrames solver;
    trajectory::TrajectoryGenerator generator;simulation::NoNoise noise;
    experiments::UpdateRateRunner runner({rays,noise},{generator,truth,environment},&solver);
    std::vector<experiments::AdaptivePaperSample> rows;
    const auto result=runner.observePaperSchedule(PaperUpdateSchedule::Article,
        [&](const auto& sample){rows.push_back(sample);});
    require(!result.hasValue()&&solver.calls==11&&rows.size()==8,
            "Only the first two committed UpdateRates frames may contribute paper samples");
    const double degrees=180/std::acos(-1.0);
    constexpr double regression_margin=1e-9; // 沿用既有 Session 解析角度回归余量。
    for(const auto& row:rows) {
        const bool stationary=row.matlab_frame==3;
        require(row.direction_count==6&&row.translation_degrees.value()==0,"Sample rays or translation identity");
        require(row.ideal_translation_mean==4.0/3 &&
                row.ideal_rotation_mean==(stationary?0:2.0/3) &&
                row.ideal_flow_ratio.value()==(stationary?0:0.5),"Independent six-ray ideal flow ratio");
        require(stationary?(!row.rotation_axis_degrees.hasValue()&&
                            row.rotation_axis_degrees.error().code==NumericalErrorCode::UndefinedAngle):
                           row.rotation_axis_degrees.value()==0,"Same-axis / undefined-axis distinction");
        require(std::abs(row.rotation_geodesic_degrees.value()-(stationary?2:1)*degrees)<regression_margin,
                "Same frame/estimate geodesic is independently defined");
    }
}
void biasAnglesBeforeAveraging() {
    Study study;
    constexpr std::array<double,3> noise{{0.6,2.4,9.6}};
    for(bool occluded:{true,false})for(unsigned seed=1;seed<=40;++seed) {
        BiasTrial trial{occluded,seed,{}};
        for(std::size_t col=1;col<=3;++col)for(std::size_t level=0;level<=9;++level)
            for(std::size_t y=3;y<=6;++y) {
                const std::size_t full_factor=std::size_t{1}<<(2*level);
                trial.samples.push_back({level,col,(occluded?6u:8u)*full_factor,y,6.0*full_factor,
                    noise[col-1],seed<=20?0.0:2.0,number(seed<=20?0.0:180.0)});
            }
        study.bias.push_back(std::move(trial));
    }
    MemoryPublisher output;Files files{output,{}};
    publishFigures({1,svgAvailable(),{},{}},study,files);
    output.contains("means.tsv","proportional\t128\t1\tywerte3_direction_degrees\t40\t40\t90\n");
    output.contains("fig1-source-config-curves.tsv","A  Occluded directions\tOriginal KvD translation, 0.6\tleft\t96\t90");
    output.contains("fig1-source-config-curves.tsv","B  Full sphere, proportional noise\tModified KvD rotation, 9.6\tleft\t128\t90");
    if(svgAvailable())output.contains("fig1-source-config.svg","stroke-dasharray=");
    study.bias.pop_back();
    MemoryPublisher incomplete;Files incomplete_files{incomplete,{}};
    bool rejected=false;
    try {publishFigures({1,false,{},{}},study,incomplete_files);}
    catch(const PaperFailure& e){rejected=e.reason.code==ExperimentErrorCode::DataMismatch;}
    require(rejected&&incomplete.contents.empty(),"A 39-trial group received an average");
}

AdaptiveCondition condition(const std::string& name,std::size_t frames,std::size_t repeats,bool rates) {
    AdaptiveCondition c{name,repeats,{}};
    const std::array<std::size_t,4> periods=name=="update-archive"?
        std::array<std::size_t,4>{{1,5,11,21}}:std::array<std::size_t,4>{{1,5,10,20}};
    for(std::size_t run=1;run<=repeats;++run)for(std::size_t frame=2;frame<frames;++frame) {
        const auto phase=frame%150;
        const double ratio=phase<=2?0.0:phase==3?0.015:1.0;
        for(std::size_t j=0;j<(rates?4u:2u);++j) {
            auto axis=(frame==151||(run==8 && frame==65 && j==1))?missingAxis():number(20+run);
            c.samples.push_back({run,frame,j+1,rates||frames==35?320u:5120u,rates?periods[j]:0,
                rates&&frame%periods[j]==0,ratio,1.0,number(ratio),number(static_cast<double>(frame)),
                std::move(axis),number(frame==151?0:4)});
        }
    }
    return c;
}
void dualMetricsAndWindows() {
    Study study;
    study.adaptive.push_back(condition("comparison-noise-0",600,8,false));
    study.adaptive.push_back(condition("comparison-noise-0p1",600,8,false));
    study.adaptive.push_back(condition("update-article",600,1,true));
    MemoryPublisher output;Files files{output,{}};
    publishFigures({5,svgAvailable(),0,PaperUpdateSchedule::Article},study,files);
    output.contains("means.tsv","comparison-noise-0\t65\t2\trotation_axis_degrees\t7\t8\tNA\n");
    output.contains("means.tsv","comparison-noise-0\t65\t2\trotation_geodesic_degrees\t8\t8\t4\n");
    output.contains("means.tsv","update-article\t151\t1\trotation_geodesic_degrees\t1\t1\t0\n");
    const std::string axis="fig5-axis-curves.tsv";
    output.contains(axis,"B  Rotation axis, noise 0\tAdaptive MFA\tleft\t65\tNA\t0\n");
    output.contains(axis,"B  Rotation axis, noise 0\tAdaptive MFA\tleft\t153\t24.5\t1\n");
    output.contains(axis,"F  Rotation axis, update-article\tPeriod 10\tleft\t159\t21\t1\n");
    output.contains("fig5-geodesic-curves.tsv","B  SO(3) rotation distance, noise 0\tAdaptive MFA\tleft\t151\t0\t0\n");
    output.contains(axis,"E  Translation, update-article\tPeriod 20\tleft\t50\t50\t1\n");
    if(svgAvailable()) {
        output.contains("fig5-axis.svg","id=\"panel5\"");
        output.contains("fig5-geodesic.svg","SO(3) rotation distance");
        output.contains("fig5-axis.svg","Ideal rot/trans flow ratio");
    }
    study.adaptive.pop_back();
    study.adaptive.push_back(condition("update-archive",600,1,true));
    MemoryPublisher archive;Files archive_files{archive,{}};
    publishFigures({5,svgAvailable(),0,PaperUpdateSchedule::Archive},study,archive_files);
    archive.contains("fig5-update-archive-axis-linear-curves.tsv","E  Translation, update-archive\tPeriod 21\tleft\t50\t50\t1");
    if(svgAvailable())require(archive.contents.at("fig5-update-archive-axis-linear.svg").find("10^")==std::string::npos,
                               "Archive E/F view must use linear axes");
    Study initialization;
    initialization.adaptive.push_back(condition("initialization",35,1,false));
    MemoryPublisher init;Files init_files{init,{}};
    publishFigures({2,svgAvailable(),{},{}},initialization,init_files);
    init.contains("fig2-axis-curves.tsv","Translation direction\tleft\t34\t34\t0\n");
    init.contains("fig2-geodesic-curves.tsv","SO(3) rotation distance\tleft\t2\t4\t1\n");
}
void commandContract() {
    using cli::parseCommand;
    for(const auto& schedule:{"article","archive"})
        require(parseCommand({"paper-figure","--figure","5","--periods",schedule,"--seed","0"}).hasValue(),
                "Fig. 5 rejected an explicit schedule/seed");
    require(!parseCommand({"paper-figure","--figure","5","--seed","0"}).hasValue(),"Missing schedule accepted");
    require(!parseCommand({"paper-figure","--figure","5","--periods","article"}).hasValue(),"Missing seed accepted");
    require(!parseCommand({"paper-figure","--figure","2","--seed","1"}).hasValue(),"Unused seed accepted");
    require(!parseCommand({"paper-figure","--figure","1","--output-root","tmp"}).hasValue(),"Output override accepted");
}
} // namespace

int main() try {
    commandContract();actualFrameEvidence();biasAnglesBeforeAveraging();dualMetricsAndWindows();
    std::cout<<"Paper figure synthetic population, metric, period, window and CLI checks passed\n";
    return 0;
} catch(const std::exception& error) {
    std::cerr<<error.what()<<'\n';return 1;
} catch(const PaperFailure& error) {
    std::cerr<<error.reason.detail.message<<'\n';return 1;
}
