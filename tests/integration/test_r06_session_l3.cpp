#include "../support/adaptive_scalar_reference.hpp"
#include "../support/flow_fixture.hpp"
#include "../support/motion_fixture.hpp"
#include "egomotion/estimators/adaptive_session.hpp"

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

namespace {
using namespace egomotion;
using namespace egomotion::estimators;
namespace ref=adaptive_reference;
namespace fs=std::filesystem;
using ref::Real;
constexpr Real step_margin=2e-8L; // 沿用test_r06_adaptive_l3.cpp，非DEC-005正式阈值。
constexpr Real angle_margin=1e-9L; // 沿用source motion error固定参考测试。
constexpr std::size_t first_frame=2,last_frame=43;
constexpr std::array<std::size_t,4> periods{{1,5,11,21}};
std::size_t checks=0,frames=0,solves=0,failures=0,rollback_cases=0;
Real max_absolute=0,max_scaled=0,max_angle=0;
void require(bool good,const std::string& message) {
    ++checks; if(!good)throw std::runtime_error(message);
}
void near(Real actual,Real expected,const std::string& key,bool angle=false) {
    ++checks;
    const Real absolute=std::abs(actual-expected),scaled=absolute/(1+std::abs(expected));
    if(std::isfinite(absolute)) {
        if(angle)max_angle=std::max(max_angle,absolute);
        else { max_absolute=std::max(max_absolute,absolute); max_scaled=std::max(max_scaled,scaled); }
    }
    if(!std::isfinite(actual)||!std::isfinite(expected)||scaled>(angle?angle_margin:step_margin)) {
        ++failures;
        if(failures<=12)std::cerr<<std::setprecision(21)<<key<<" actual="<<actual<<" expected="<<expected<<'\n';
    }
}
constexpr std::array<std::array<int,2>,9> positions{{
    {{0,0}},{{1,0}},{{1,1}},{{1,2}},{{2,0}},{{2,1}},{{2,2}},{{2,3}},{{2,4}}}};
ref::Coefficients coefficients(const HarmonicCoefficients& c) {
    ref::Coefficients result{};
    for(std::size_t j=0;j<positions.size();++j)result[j]=c.values()(positions[j][0],positions[j][1]);
    for(int i=0;i<3;++i)for(int j=2*i+1;j<5;++j)require(c.values()(i,j)==0,"Unused source layout must stay zero");
    return result;
}
template<class A,class B> void values(const A& a,const B& b,const std::string& key) {
    require(a.size()==b.size(),key+" shape");
    for(std::size_t i=0;i<a.size();++i)near(a[i],b[i],key+"["+std::to_string(i)+"]");
}
ref::Vec vector(const Vector3& value) { return {value.x(),value.y(),value.z()}; }
ref::Step trace(const AdaptiveStepResult& value) {
    ref::Step result;
    const auto& t=value.trace;
    for(Eigen::Index i=0;i<t.near_raw.size();++i) {
        result.raw.push_back(t.near_raw[i]); result.after_sign.push_back(t.near_after_sign[i]);
        result.normalized.push_back(t.near_normalized[i]); result.recomputed.push_back(t.recomputed_near[i]);
    }
    result.mean=t.original_near_mean; result.mse=value.error_mse;
    result.translation=vector(value.motion.translation()); result.rotation=vector(value.motion.rotation());
    result.next=coefficients(value.coefficients_next);
    values(coefficients(t.coefficients_next),result.next,"Trace coefficient copy");
    near(t.error_mse,result.mse,"Trace MSE copy");
    for(std::size_t i=0;i<6;++i) {
        result.rhs[i]=t.rhs[static_cast<Eigen::Index>(i)];
        result.solved[i]=t.solved_motion[static_cast<Eigen::Index>(i)];
        for(std::size_t j=0;j<6;++j)result.matrix[i][j]=t.system_matrix(static_cast<Eigen::Index>(i),static_cast<Eigen::Index>(j));
    }
    for(int i=0;i<3;++i)for(int j=0;j<3;++j) {
        near(t.m1(i,j),result.matrix[i][j],"M1 trace"); near(t.m2(i,j),result.matrix[i][j+3],"M2 trace");
        near(t.m3(i,j),result.matrix[i+3][j],"M3 trace"); near(t.m4(i,j),result.matrix[i+3][j+3],"M4 trace");
    }
    for(int i=0;i<3;++i)near(t.translation_raw[i],result.solved[i],"Raw translation trace");
    return result;
}
void compare(const ref::Step& actual,const ref::Step& expected,const std::string& key) {
    values(actual.raw,expected.raw,key+" near_raw"); values(actual.after_sign,expected.after_sign,key+" after_sign");
    values(actual.normalized,expected.normalized,key+" normalized"); near(actual.mean,expected.mean,key+" mean");
    for(std::size_t i=0;i<6;++i)values(actual.matrix[i],expected.matrix[i],key+" matrix row"+std::to_string(i));
    values(actual.rhs,expected.rhs,key+" rhs"); values(actual.solved,expected.solved,key+" solved");
    values(actual.translation,expected.translation,key+" translation"); values(actual.rotation,expected.rotation,key+" rotation");
    near(actual.mse,expected.mse,key+" mse"); values(actual.recomputed,expected.recomputed,key+" new_near");
    values(actual.next,expected.next,key+" next_coeff");
}
struct TracedProduction final:IAdaptiveStepSolver {
    AdaptiveMfaStepSolver production;
    mutable std::vector<ref::Coefficients> inputs;
    mutable std::vector<ref::Step> outputs;
    Result<AdaptiveStepResult,NumericalErrorCode> solve(const HarmonicCoefficients& old,const FlowField& flow) const override {
        inputs.push_back(coefficients(old));
        auto result=production.solve(old,flow);
        if(result.hasValue()) { outputs.push_back(trace(result.value())); ++solves; }
        return result; // 原生产结果原样转交，不替换求解或修改数值。
    }
    void clear() const { inputs.clear(); outputs.clear(); }
};
Real rounded(Real value) { return static_cast<double>(value); }
ref::Matrix<3> rotation(std::size_t frame) {
    const std::array<ref::Matrix<3>,6> rotations{{
        {{{1,0,0},{0,0,-1},{0,1,0}}}, {{{0,-1,0},{1,0,0},{0,0,1}}},
        {{{0,0,1},{0,1,0},{-1,0,0}}}, {{{1,0,0},{0,0,1},{0,-1,0}}},
        {{{0,1,0},{-1,0,0},{0,0,1}}}, {{{1.0L/9,8.0L/9,4.0L/9},{8.0L/9,1.0L/9,-4.0L/9},{-4.0L/9,4.0L/9,-7.0L/9}}}}};
    auto result=rotations[(frame-first_frame)%rotations.size()];
    for(auto& row:result)for(auto& value:row)value=rounded(value);
    return result;
}
ref::Field observations(std::size_t stream,std::size_t frame) {
    ref::Field result;
    const Real phi=(1+std::sqrt(5.0L))/2, length=std::sqrt(1+phi*phi);
    const Real drift=static_cast<Real>(frame)/128;
    const ref::Vec t=stream==0?ref::Vec{1+drift,0.25L,-0.5L}:ref::Vec{0.25L,1+drift,0.125L};
    const ref::Vec w=stream==0?ref::Vec{0.125L,0.25L+drift/8,0.5L}:ref::Vec{-0.5L,0.125L,0.25L+drift/8};
    for(std::size_t cycle=0;cycle<3;++cycle)for(int sign1:{-1,1})for(int sign2:{-1,1}) {
        ref::Vec d{},p{};
        d[(cycle+1)%3]=rounded(sign1/length); d[(cycle+2)%3]=rounded(sign2*phi/length);
        const Real x=d[0],y=d[1],z=d[2];
        const Real mu=1+static_cast<Real>(stream)/4+drift+0.11L*x-0.07L*y+0.05L*z+
            0.03L*x*y-0.02L*y*z+0.04L*x*z+0.015L*(3*z*z-1)+0.025L*(x*x-y*y);
        const Real projected=ref::dot(t,d); const auto rotational=ref::cross(w,d);
        for(std::size_t j=0;j<3;++j)p[j]=rounded(-mu*(t[j]-projected*d[j])-rotational[j]);
        result.push_back({d,p});
    }
    return result;
}
AdaptiveFrameInput input(std::size_t stream,std::size_t frame,const ref::Field& field,const ref::Matrix<3>& r) {
    AdaptiveFrameInput result; result.matlab_frame_index=frame;
    FlowMatrix matrix(static_cast<Eigen::Index>(field.size()),6);
    for(std::size_t i=0;i<field.size();++i)for(std::size_t j=0;j<3;++j) {
        matrix(static_cast<Eigen::Index>(i),static_cast<Eigen::Index>(j))=static_cast<double>(field[i].direction[j]);
        matrix(static_cast<Eigen::Index>(i),static_cast<Eigen::Index>(j+3))=static_cast<double>(field[i].flow[j]);
    }
    for(int i=0;i<3;++i)for(int j=0;j<3;++j)result.source_coefficient_rotation(i,j)=static_cast<double>(r[i][j]);
    result.observations=test_support::flowValue(matrix);
    // 固定误差比较轴与信号不共线，避免用未定义角度替代成功序列；不是原轨迹真值。
    result.truth=test_support::motionValue(stream==0?Vector3::UnitX():Vector3::UnitY(),
                                           stream==0?Vector3::UnitZ():Vector3::UnitX());
    return result;
}
template<class Values> void writeArray(std::ostream& out,const Values& values) {
    out<<'['; bool first=true;
    for(const auto& v:values) { if(!first)out<<','; out<<v; first=false; } out<<']';
}
template<class Rows> void writeMatrix(std::ostream& out,const Rows& rows) {
    out<<'['; bool first=true;
    for(const auto& row:rows) { if(!first)out<<','; writeArray(out,row); first=false; } out<<']';
}
void writeStep(std::ostream& out,const ref::Step& s) {
    out<<"{\"mean\":"<<s.mean<<",\"mse\":"<<s.mse<<",\"near_raw\":"; writeArray(out,s.raw);
    out<<",\"near_after_sign\":"; writeArray(out,s.after_sign);
    out<<",\"near_normalized\":"; writeArray(out,s.normalized);
    out<<",\"matrix\":"; writeMatrix(out,s.matrix); out<<",\"rhs\":"; writeArray(out,s.rhs);
    out<<",\"solved\":"; writeArray(out,s.solved); out<<",\"translation\":"; writeArray(out,s.translation);
    out<<",\"rotation\":"; writeArray(out,s.rotation); out<<",\"new_near\":"; writeArray(out,s.recomputed);
    out<<",\"coefficients_next\":"; writeArray(out,s.next); out<<'}';
}
void writeRecord(std::ostream& out,std::size_t mode,std::size_t stream,std::size_t frame,
                 const ref::Field& field,const ref::Matrix<3>& r,const std::vector<ref::Coefficients>& inputs,
                 const std::vector<ref::Step>& steps,const ref::Session& state,
                 const std::vector<Real>& translation_errors,const std::vector<Real>& rotation_errors) {
    out<<"{\"mode\":"<<mode<<",\"stream\":"<<stream<<",\"matlab_frame\":"<<frame<<",\"rotation\":";
    writeMatrix(out,r); out<<",\"observations\":[";
    for(std::size_t j=0;j<field.size();++j) {
        if(j)out<<',';
        out<<"{\"direction\":"; writeArray(out,field[j].direction);
        out<<",\"flow\":"; writeArray(out,field[j].flow); out<<'}';
    }
    out<<"],\"step_inputs_comparisons_then_primary\":"; writeMatrix(out,inputs); out<<",\"steps\":[";
    for(std::size_t j=0;j<steps.size();++j) { if(j)out<<','; writeStep(out,steps[j]); }
    out<<"],\"primary_state\":"; writeArray(out,state.primary);
    out<<",\"baseline_state\":"; writeMatrix(out,state.baselines);
    out<<",\"comparison_translation_error_deg\":"; writeArray(out,translation_errors);
    out<<",\"comparison_rotation_error_deg\":"; writeArray(out,rotation_errors); out<<"}\n";
}
ref::Session state(const AdaptiveMfaSession& session) {
    ref::Session result; result.primary=coefficients(session.primaryCoefficients());
    for(std::size_t j=0;j<4;++j)result.baselines[j]=coefficients(session.updateRateBaselines()[j]);
    return result;
}
void compareState(const AdaptiveMfaSession& actual,const ref::Session& expected,const std::string& key) {
    const auto snapshot=state(actual);
    values(snapshot.primary,expected.primary,key+" main");
    for(std::size_t j=0;j<4;++j)values(snapshot.baselines[j],expected.baselines[j],key+" baseline"+std::to_string(j));
}
bool distinguish(const ref::Coefficients& a,const ref::Coefficients& b) {
    for(std::size_t j=0;j<a.size();++j)if(std::abs(a[j]-b[j])>step_margin*(1+std::abs(b[j])))return true;
    return false;
}
void rollback(AdaptiveMfaSession& actual,const ref::Session& expected,const AdaptiveFrameInput& good) {
    const HarmonicMatrix before_primary=actual.primaryCoefficients().values();
    std::array<HarmonicMatrix,4> before_baselines;
    for(std::size_t j=0;j<4;++j)before_baselines[j]=actual.updateRateBaselines()[j].values();
    auto bad=good; bad.truth=test_support::motionValue(Vector3::UnitX(),Vector3::Zero());
    const auto failed=actual.processFrame(bad);
    require(!failed.hasValue()&&failed.error().code==ExperimentErrorCode::SolverFailed,"Real comparison angle failure");
    bool undefined=false;
    for(const auto& cause:failed.error().detail.causes)undefined|=cause.code=="UndefinedAngle";
    require(undefined,"Preserve real UndefinedAngle root");
    compareState(actual,expected,"Independent state after failed frame");
    const auto same=[](const HarmonicMatrix& a,const HarmonicMatrix& b) {
        return std::memcmp(a.data(),b.data(),static_cast<std::size_t>(a.size())*sizeof(double))==0;
    };
    require(same(before_primary,actual.primaryCoefficients().values()),"Failure commits no primary bits");
    for(std::size_t j=0;j<4;++j)
        require(same(before_baselines[j],actual.updateRateBaselines()[j].values()),"Failure commits no baseline bits");
    ++rollback_cases;
}
void sequence(std::size_t mode_index,std::size_t stream,std::ostream& reference,std::ostream& actual_log) {
    const auto mode=static_cast<AdaptiveSessionMode>(mode_index);
    const auto reference_mode=static_cast<ref::Mode>(mode_index);
    ref::Session expected;
    TracedProduction traced; AdaptiveMfaSession actual(mode,&traced);
    std::array<std::size_t,4> refreshed{},discarded{};
    std::size_t phase_distinguished=0,clock_distinguished=0;
    for(std::size_t frame=first_frame;frame<=last_frame;++frame) {
        const auto field=observations(stream,frame); const auto r=rotation(frame);
        const auto in=input(stream,frame,field,r);
        const auto before=expected;
        const std::string key="mode="+std::to_string(mode_index)+" stream="+std::to_string(stream)+" frame="+std::to_string(frame);
        compareState(actual,before,key+" before");
        if(frame==5||frame==11||frame==21||frame==42)rollback(actual,before,in);
        traced.clear();
        const auto model=expected.advance(reference_mode,frame,r,field);
        auto wanted_inputs=model.prepared_comparisons; wanted_inputs.push_back(model.prepared_primary);
        auto wanted_steps=model.comparisons; wanted_steps.push_back(model.primary);
        std::vector<Real> wanted_translation_errors,wanted_rotation_errors;
        for(const auto& comparison:model.comparisons) {
            wanted_translation_errors.push_back(ref::angleDegrees(vector(in.truth.translation()),comparison.translation));
            wanted_rotation_errors.push_back(ref::angleDegrees(vector(in.truth.rotation()),comparison.rotation));
        }
        writeRecord(reference,mode_index,stream,frame,field,r,wanted_inputs,wanted_steps,expected,
                    wanted_translation_errors,wanted_rotation_errors);
        const auto result=actual.processFrame(in);
        require(result.hasValue(),key+(result.hasValue()?"":" "+result.error().detail.message));
        const auto& output=result.value();
        writeRecord(actual_log,mode_index,stream,frame,field,r,traced.inputs,traced.outputs,state(actual),
                    {output.comparison_translation_error_deg.begin(),output.comparison_translation_error_deg.end()},
                    {output.comparison_rotation_error_deg.begin(),output.comparison_rotation_error_deg.end()});
        require(traced.outputs.size()==wanted_steps.size()&&traced.inputs.size()==wanted_inputs.size(),key+" solve count");
        for(std::size_t j=0;j<wanted_steps.size();++j) {
            values(traced.inputs[j],wanted_inputs[j],key+" prepared"+std::to_string(j));
            compare(traced.outputs[j],wanted_steps[j],key+" solve"+std::to_string(j));
        }
        compare(trace(result.value().primary_step),model.primary,key+" returned primary");
        compareState(actual,expected,key+" committed");
        require(result.value().matlab_frame_index==frame,"MATLAB frame identity");
        require(result.value().comparison_motion.size()==model.comparisons.size(),"Comparison output count");
        for(std::size_t j=0;j<model.comparisons.size();++j) {
            values(vector(result.value().comparison_motion[j].translation()),model.comparisons[j].translation,key+" comparison t");
            values(vector(result.value().comparison_motion[j].rotation()),model.comparisons[j].rotation,key+" comparison w");
            near(result.value().comparison_translation_error_deg[j],wanted_translation_errors[j],key+" t angle",true);
            near(result.value().comparison_rotation_error_deg[j],wanted_rotation_errors[j],key+" w angle",true);
        }
        if(frame==first_frame) {
            AdaptiveMfaSession default_session(mode);
            const auto default_result=default_session.processFrame(in);
            require(default_result.hasValue(),"Default real solver entry");
            compare(trace(default_result.value().primary_step),model.primary,key+" default solver");
            compareState(default_session,expected,key+" default state");
        }
        if(distinguish(ref::rotate(before.primary,r),ref::rotate(before.primary,r,false)))++phase_distinguished;
        if(reference_mode==ref::Mode::UpdateRates)for(std::size_t j=0;j<4;++j) {
            if(frame%periods[j]==0)++refreshed[j];
            if(distinguish(model.prepared_comparisons[j],model.comparisons[j].next))++discarded[j];
            const auto wrong=(frame-1)%periods[j]==0?model.prepared_primary:ref::rotate(before.baselines[j],r);
            if(distinguish(model.prepared_comparisons[j],wrong))++clock_distinguished;
        }
        ++frames;
    }
    require(phase_distinguished>0,"Fixed sequence distinguishes source phase from physical rotation");
    if(reference_mode==ref::Mode::UpdateRates) {
        require(refreshed==std::array<std::size_t,4>{42,8,3,2},"Source 1/5/11/21 refresh counts");
        require(clock_distinguished>0,"Fixed sequence distinguishes off-by-one refresh");
        for(const auto count:discarded)require(count>0,"Every baseline distinguishes discarded dumi from committed next coeff");
    }
    std::cout<<"mode="<<mode_index<<" stream="<<stream<<" frames=42 phase_discriminators="<<phase_distinguished
             <<" clock_discriminators="<<clock_distinguished<<'\n';
}
void calibrateReference() {
    // 原有独立Wolfram正/负均值L3；检查参考计算本身，非从生产输出生成Expected。
    const std::array<ref::Vec,9> directions{{{1,0,0},{0,1,0},{0,0,1},{-1,0,0},{0,-1,0},{0,0,-1},
        {0.6L,0.8L,0},{0,0.6L,0.8L},{0.8L,0,0.6L}}};
    const std::array<ref::Vector<6>,2> solved{{{1.0440740740740742L,0,0,0.2L,-0.14285714285714285L,0.09090909090909091L},
        {-0.4335987799002173L,0.03091308986341341L,0.02497154227677647L,0.1700641552425584L,-0.6211414741551088L,0.27568585009435687L}}};
    const std::array<ref::Coefficients,2> projected{{{2.9133889593362294L,-0.6180893315904281L,1.3917243227832163L,-1.1661376351613801L,1.9068567351030574L,-1.9259253024649212L,0.4669505734978958L,-2.6441746726441058L,-0.6961943190127998L},
        {2.2612991792774464L,-0.007125870166276713L,0.11112745632137397L,6.070985848625198L,1.3506863304311263L,-0.987244049847206L,0.41288036211613593L,-0.9821710657209377L,-0.8153944297541761L}}};
    for(std::size_t branch=0;branch<2;++branch) {
        const Real sign=branch==0?1:-1, one=std::sqrt(3/(4*ref::pi()));
        ref::Coefficients old{}; old[0]=sign*std::sqrt(4*ref::pi()); old[1]=sign/(4*one); old[2]=sign/(5*one); old[3]=-sign/(3*one);
        ref::Field field;
        for(const auto& d:directions) {
            const Real mu=sign*(1+d[0]/3-d[1]/4+d[2]/5);
            const auto w=ref::cross({1.0L/5,-1.0L/7,1.0L/11},d);
            ref::Vec flow{};
            for(std::size_t j=0;j<3;++j)flow[j]=-mu*((j==0?1:0)-d[0]*d[j])-w[j];
            field.push_back({d,flow});
        }
        const auto actual=ref::step(old,field);
        values(actual.solved,solved[branch],"Independent Wolfram solved anchor");
        values(actual.next,projected[branch],"Independent Wolfram fitted anchor");
        near(actual.mse,branch==0?0:0.21140062257948003L,"Independent Wolfram MSE anchor");
    }
    ref::Coefficients field{};
    field[0]=2*std::sqrt(4*ref::pi()); field[2]=std::sqrt(4*ref::pi()/3);
    const auto x_quarter_turn=rotation(first_frame);
    near(ref::evaluate(ref::rotate(field,x_quarter_turn),{0,1,0}),1,"Source action of 2+z at ey");
    near(ref::evaluate(ref::rotate(field,x_quarter_turn,false),{0,1,0}),3,"Physical action must remain distinct");
}
} // namespace
int main(int argc,char** argv) {
    try {
        require(argc==2,"Sandbox base required"); fs::create_directories(argv[1]);
        std::string pattern=(fs::absolute(argv[1])/"独立多帧-XXXXXX").string();
        require(::mkdtemp(pattern.data())!=nullptr,"Exclusive retained sandbox");
        const fs::path root(pattern);
        std::ofstream reference(root/"reference.jsonl"),actual(root/"actual.jsonl");
        reference.exceptions(std::ios::badbit|std::ios::failbit); actual.exceptions(std::ios::badbit|std::ios::failbit);
        reference<<std::setprecision(std::numeric_limits<Real>::max_digits10);
        actual<<std::setprecision(std::numeric_limits<Real>::max_digits10);
        std::cout<<"Independent Session L3 sandbox="<<root<<'\n';
        calibrateReference();
        for(std::size_t mode=0;mode<3;++mode)for(std::size_t stream=0;stream<2;++stream)sequence(mode,stream,reference,actual);
        reference.close(); actual.close();
        std::cout<<std::setprecision(21)<<"Session independent L3 checks="<<checks<<" frames="<<frames<<" real_solves="<<solves
                 <<" rollback_cases="<<rollback_cases<<" failures="<<failures<<" max_abs="<<max_absolute
                 <<" max_scaled="<<max_scaled<<" max_angle_abs="<<max_angle<<'\n';
        return failures==0?0:1;
    } catch(const std::exception& error) { std::cerr<<"Independent Session L3 failure: "<<error.what()<<'\n'; return 1; }
}
