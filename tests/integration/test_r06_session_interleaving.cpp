#include "../support/flow_fixture.hpp"
#include "../support/motion_fixture.hpp"
// 状态隔离测试：真实单步/Session；独立运行是调度不变性的参考，不是数值 Golden。
#include "egomotion/estimators/adaptive_session.hpp"

#include <array>
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
using namespace egomotion;
using namespace egomotion::estimators;
constexpr std::size_t kFirstFrame=2,kLastFrame=22;
std::size_t checks=0,successful_frames=0,rejected_frames=0;
void require(bool pass,const std::string& message) {
    ++checks;
    if (!pass) throw std::runtime_error(message);
}
template<class A,class B>
void same(const A& a,const B& b,const std::string& label) {
    require(a.rows()==b.rows()&&a.cols()==b.cols(),label+": shape");
    require(a.size()==0||std::memcmp(a.data(),b.data(),static_cast<std::size_t>(a.size())*sizeof(double))==0,
            label+": values differ bitwise");
}
struct State {
    HarmonicMatrix primary;
    std::array<HarmonicMatrix,4> baselines;
};
State snapshot(const AdaptiveMfaSession& session) {
    State result;
    result.primary=session.primaryCoefficients().values();
    for (std::size_t i=0;i<4;++i) result.baselines[i]=session.updateRateBaselines()[i].values();
    return result;
}
void sameState(const AdaptiveMfaSession& session,const State& state,const std::string& label) {
    same(session.primaryCoefficients().values(),state.primary,label+" primary");
    for (std::size_t i=0;i<4;++i)
        same(session.updateRateBaselines()[i].values(),state.baselines[i],label+" baseline");
}
AdaptiveFrameInput input(std::size_t stream,std::size_t index) {
    AdaptiveFrameInput result;
    result.matlab_frame_index=index;
    if (stream==0)
        result.source_coefficient_rotation<<0,-1,0,1,0,0,0,0,1;
    else
        result.source_coefficient_rotation<<1,0,0,0,0,-1,0,1,0;
    if (index%2==1) result.source_coefficient_rotation.transposeInPlace();
    // 固定参考轴与信号不共线，使角度处于定义域内部；不宣称原轨迹/科学真值。
    result.truth=test_support::motionValue(stream==0?Vector3::UnitX():Vector3::UnitY(),
        stream==0?Vector3::UnitZ():Vector3::UnitX());
    const std::array<double,3> t=stream==0?std::array<double,3>{{1,0.25,-0.5}}:
                                                  std::array<double,3>{{0.25,1,0.125}};
    const std::array<double,3> w=stream==0?std::array<double,3>{{0.125,0.25,0.5}}:
                                                  std::array<double,3>{{-0.5,0.125,0.25}};
    const double phi=(1+std::sqrt(5.0))/2,norm=std::sqrt(1+phi*phi);
    FlowMatrix raw(12,6);
    Eigen::Index row=0;
    for (int cyclic=0;cyclic<3;++cyclic) for (double sign1:{-1.0,1.0}) for (double sign2:{-1.0,1.0}) {
        std::array<double,3> d{};
        d[static_cast<std::size_t>((cyclic+1)%3)]=sign1/norm;
        d[static_cast<std::size_t>((cyclic+2)%3)]=sign2*phi/norm;
        double dot=0;
        for (std::size_t i=0;i<3;++i) dot+=t[i]*d[i];
        const double near=(stream==0?1.0:1.5)+static_cast<double>(index-kFirstFrame)/128;
        for (std::size_t i=0;i<3;++i) {
            raw(row,static_cast<Eigen::Index>(i))=d[i];
            raw(row,static_cast<Eigen::Index>(i+3))=
                -near*(t[i]-dot*d[i])-(w[(i+1)%3]*d[(i+2)%3]-w[(i+2)%3]*d[(i+1)%3]);
        }
        ++row;
    }
    result.observations=test_support::flowValue(raw);
    return result;
}
void sameOutput(const AdaptiveFrameResult& a,const AdaptiveFrameResult& b) {
    require(a.matlab_frame_index==b.matlab_frame_index,"Frame identity");
    same(a.primary_step.motion.translation(),b.primary_step.motion.translation(),"primary translation");
    same(a.primary_step.motion.rotation(),b.primary_step.motion.rotation(),"primary rotation");
    same(a.primary_step.coefficients_next.values(),b.primary_step.coefficients_next.values(),"next coefficients");
    require(a.primary_step.error_mse==b.primary_step.error_mse,"MSE changed with schedule");
    const auto& x=a.primary_step.trace; const auto& y=b.primary_step.trace;
    same(x.near_raw,y.near_raw,"raw near"); same(x.near_after_sign,y.near_after_sign,"signed near");
    same(x.near_normalized,y.near_normalized,"normalized near");
    require(x.original_near_mean==y.original_near_mean&&x.error_mse==y.error_mse,"trace scalar");
    same(x.m1,y.m1,"M1"); same(x.m2,y.m2,"M2"); same(x.m3,y.m3,"M3"); same(x.m4,y.m4,"M4");
    same(x.system_matrix,y.system_matrix,"M"); same(x.rhs,y.rhs,"RHS");
    same(x.solved_motion,y.solved_motion,"raw solution");
    same(x.translation_raw,y.translation_raw,"raw translation");
    same(x.recomputed_near,y.recomputed_near,"new near");
    same(x.coefficients_next.values(),y.coefficients_next.values(),"trace coefficients");
    require(a.comparison_motion.size()==b.comparison_motion.size(),"comparison count");
    for (std::size_t i=0;i<a.comparison_motion.size();++i) {
        same(a.comparison_motion[i].translation(),b.comparison_motion[i].translation(),"comparison translation");
        same(a.comparison_motion[i].rotation(),b.comparison_motion[i].rotation(),"comparison rotation");
    }
    for(const auto* pair:{&a.comparison_translation_error_deg,&a.comparison_rotation_error_deg}) {
        const auto& other=pair==&a.comparison_translation_error_deg
            ?b.comparison_translation_error_deg:b.comparison_rotation_error_deg;
        require(pair->size()==other.size(),"Metric result count");
        for(std::size_t i=0;i<pair->size();++i) {
            const auto& x=(*pair)[i];const auto& y=other[i];
            require(x.hasValue()==y.hasValue(),"Metric validity changed with schedule");
            if(x.hasValue())require(x.value()==y.value(),"Metric value changed with schedule");
            else require(x.error().code==y.error().code&&x.error().detail.message==y.error().detail.message,
                         "Metric failure changed with schedule");
        }
    }
}
struct Record { AdaptiveFrameResult output; State state; };
std::vector<Record> isolated(AdaptiveSessionMode mode,std::size_t stream) {
    AdaptiveMfaSession session(mode);
    std::vector<Record> result;
    for (std::size_t i=kFirstFrame;i<=kLastFrame;++i) {
        auto output=session.processFrame(input(stream,i));
        require(output.hasValue(),"Isolated stream="+std::to_string(stream)+" frame="+std::to_string(i)+
                (output.hasValue()?"":" "+output.error().detail.message));
        ++successful_frames;
        result.push_back({std::move(output).value(),snapshot(session)});
    }
    return result;
}
void injectedFailure(AdaptiveMfaSession& owner,const AdaptiveMfaSession& peer,
                     std::size_t stream,std::size_t index) {
    const auto before=snapshot(owner),peer_before=snapshot(peer);
    auto bad=input(stream,index);
    if (stream==0) bad.observations=FlowField{};
    else {
        auto zero_flow=bad.observations.observations();
        zero_flow.rightCols(3).setZero();
        bad.observations=test_support::flowValue(zero_flow);
    }
    const auto failed=owner.processFrame(bad);
    require(!failed.hasValue()&&failed.error().code==ExperimentErrorCode::SolverFailed,
            "Injected frame must fail");
    bool root=false;
    for (const auto& cause:failed.error().detail.causes)
        root|=cause.domain=="NumericalErrorCode"&&cause.code==(stream==0?"EmptyInput":"ZeroNorm");
    require(root,"Wrong failure root");
    sameState(owner,before,"failed owner"); sameState(peer,peer_before,"failed peer");
    const auto skipped=owner.processFrame(input(stream,index+1));
    require(!skipped.hasValue()&&skipped.error().code==ExperimentErrorCode::InvalidParameters,
            "Failed frame incorrectly advanced private clock");
    sameState(owner,before,"out-of-order owner"); sameState(peer,peer_before,"out-of-order peer");
    rejected_frames+=2;
}
void verifyMode(AdaptiveSessionMode mode) {
    const auto a_reference=isolated(mode,0),b_reference=isolated(mode,1);
    AdaptiveMfaSession a(mode),b(mode);
    std::array<std::size_t,2> next{{kFirstFrame,kFirstFrame}};
    std::array<bool,2> failed_once{{false,false}};
    auto advance=[&](std::size_t stream) {
        if (next[stream]>kLastFrame) return;
        auto& owner=stream==0?a:b; auto& peer=stream==0?b:a;
        const auto& reference=stream==0?a_reference:b_reference;
        const std::size_t index=next[stream];
        if (!failed_once[stream]&&index==(stream==0?5U:11U)) {
            injectedFailure(owner,peer,stream,index);
            failed_once[stream]=true;
            return; // 另一实例可继续运行；此实例下次仍须重试相同帧。
        }
        const auto peer_before=snapshot(peer);
        const auto frame=input(stream,index),saved=frame;
        const auto output=owner.processFrame(frame);
        require(output.hasValue(),"Interleaved stream="+std::to_string(stream)+" frame="+std::to_string(index));
        sameOutput(output.value(),reference[index-kFirstFrame].output);
        sameState(owner,reference[index-kFirstFrame].state,"isolated reference");
        sameState(peer,peer_before,"peer after successful frame");
        same(frame.observations.observations(),saved.observations.observations(),"input flow");
        same(frame.source_coefficient_rotation,saved.source_coefficient_rotation,"input rotation");
        ++next[stream]; ++successful_frames;
    };
    advance(0); advance(0); // 刻意令两个实例的 MATLAB 帧时钟不同步。
    std::size_t rounds=0;
    while (next[0]<=kLastFrame||next[1]<=kLastFrame) {
        require(++rounds<2*kLastFrame,"Interleaved schedule made no progress");
        advance(1); advance(0); advance(1); advance(0);
    }
    require(failed_once[0]&&failed_once[1],"Both failure branches must run");
    require((a.primaryCoefficients().values()-b.primaryCoefficients().values()).norm()>0,
            "Different streams unexpectedly share the same final state");
    std::cout<<"mode="<<static_cast<int>(mode)<<" isolated_success=42 interleaved_success=42 rejected=4\n";
}
}  // namespace

int main() {
    int failures=0;
    for (auto mode:{AdaptiveSessionMode::Initialization,AdaptiveSessionMode::Comparison,AdaptiveSessionMode::UpdateRates}) {
        try { verifyMode(mode); }
        catch (const std::exception& error) { ++failures; std::cerr<<"mode="<<static_cast<int>(mode)<<" "<<error.what()<<'\n'; }
    }
    std::cout<<"Session interleaving checks="<<checks<<" successful_frames="<<successful_frames
             <<" rejected_frames="<<rejected_frames<<" failures="<<failures<<'\n';
    return failures==0?0:1;
}
