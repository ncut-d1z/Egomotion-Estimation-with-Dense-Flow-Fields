#include "../support/adaptive_scalar_reference.hpp"
#include "../support/archive_mat_reference.hpp"
#include "egomotion/contracts/diagnostics.hpp"
#include "egomotion/estimators/adaptive_session.hpp"
#include "egomotion/experiments/production_runner.hpp"
#include "egomotion/experiments/run_context.hpp"
#include "egomotion/io/mat_result_reader.hpp"
#include "egomotion/io/run_directory.hpp"
#include "egomotion/io/run_mat_writer.hpp"
#include "egomotion/io/run_text_publisher.hpp"
#include "egomotion/io/sha256.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace egomotion;
using namespace egomotion::experiments;
namespace ref=adaptive_reference;
namespace fs=std::filesystem;
using ref::Real;
using Values=std::vector<Real>;
using Fields=std::map<std::string,Values>;
constexpr std::size_t kDirections=12,kAxes=3;
constexpr double kNoise=0.1;
// 复用既有flow、Step和Runner合同回归余量，不是DEC-005正式验收阈值。
constexpr Real kFlowMargin=1e-12L,kStepMargin=2e-8L,kOutputMargin=1e-9L;
std::size_t checks=0,total_steps=0,published_values=0;
Real max_step=0,max_flow=0,max_output=0;
void require(bool pass,const std::string& why) { ++checks;if(!pass)throw std::runtime_error(why); }
void near(Real actual,Real expected,Real allowance,const std::string& key,Real& maximum,bool scaled=false) {
    const Real delta=std::abs(actual-expected);
    maximum=std::max(maximum,delta);
    if(!std::isfinite(delta)||delta>allowance*(scaled?1+std::abs(expected):1))
        std::cerr<<std::setprecision(21)<<key<<" actual="<<actual<<" expected="<<expected<<'\n';
    require(std::isfinite(delta)&&delta<=allowance*(scaled?1+std::abs(expected):1),key);
}
struct Mode {ExperimentKind kind;ref::Mode reference;std::size_t frames,runs,comparisons;const char* name;};
const std::array<Mode,3> modes{{
    {ExperimentKind::AdaptiveInitialization,ref::Mode::Initialization,35,1,2,"initialization"},
    {ExperimentKind::AdaptiveComparison,ref::Mode::Comparison,600,8,2,"comparison"},
    {ExperimentKind::AdaptiveUpdateRates,ref::Mode::UpdateRates,600,1,4,"update-rates"}}};
Real rounded(Real value) {return static_cast<double>(value);}
ref::Vec translation(std::size_t row) {return {1+Real(row+1)/128,0.25L,-0.5L};}
ref::Vec rotationVector(std::size_t row) {
    return row==0?ref::Vec{0,0,0}:ref::Vec{0.125L,0.25L+Real(row+1)/1024,0.5L};
}
ref::Field directions() {
    ref::Field result;const Real phi=(1+std::sqrt(5.0L))/2,length=std::sqrt(1+phi*phi);
    for(std::size_t cycle=0;cycle<3;++cycle)for(int a:{-1,1})for(int b:{-1,1}) {
        ref::Vec d{};d[(cycle+1)%3]=rounded(a/length);d[(cycle+2)%3]=rounded(b*phi/length);
        result.push_back({d,{0,0,0}});
    }
    return result;
}
Real nearness(std::size_t run,std::size_t row,const ref::Vec& d) {
    const Real x=d[0],y=d[1],z=d[2];
    return rounded(1+Real(run)/4+Real(row+1)/128+0.11L*x-0.07L*y+0.05L*z+
        0.03L*x*y-0.02L*y*z+0.04L*x*z+0.015L*(3*z*z-1)+0.025L*(x*x-y*y)+
        0.06L*x*z*z+0.025L*y*x*x);
}
// 固定测试输入；不声称位置/局部运动/近度构成原轨迹或单位球场景。
struct Scene final:trajectory::ITrajectoryGenerator,trajectory::ITrajectoryKinematics,
                   trajectory::ISphereEnvironment,IDirectionSampler {
    Mode mode;mutable std::size_t near_calls=0;
    explicit Scene(Mode m):mode(m) {}
    Result<trajectory::TrajectoryPoints,NumericalErrorCode> points(std::size_t count) const {
        require(count==mode.frames,"Original frame count");
        trajectory::TrajectoryPoints out;out.points.resize(count,3);
        for(std::size_t row=0;row<count;++row)out.points.row(row)<<double(row)/1024,0.125,0.25;
        return Result<trajectory::TrajectoryPoints,NumericalErrorCode>::success(std::move(out));
    }
    Result<trajectory::TrajectoryPoints,NumericalErrorCode> circle(std::size_t n,double r,double h) const override {
        require(mode.frames==35&&r==0.6&&h==0.3,"Source circle configuration");return points(n);
    }
    Result<trajectory::TrajectoryPoints,NumericalErrorCode> sinusoidal(std::size_t n,double a,double l,std::size_t p,double h) const override {
        require(mode.frames==600&&a==0.5&&l==1&&p==2&&h==0.3,"Source sinusoidal configuration");return points(n);
    }
    Result<std::vector<trajectory::FrameKinematics>,NumericalErrorCode> fromPoints(const trajectory::TrajectoryPoints& points) const override {
        require(points.points.rows()==static_cast<Eigen::Index>(mode.frames),"Scene frame count");
        std::vector<trajectory::FrameKinematics> out(mode.frames);
        for(std::size_t row=1;row+1<mode.frames;++row) {
            const auto t=translation(row),w=rotationVector(row);
            for(int j=0;j<3;++j){out[row].local_translation[j]=static_cast<double>(t[j]);out[row].relative_rotation[j]=static_cast<double>(w[j]);}
            out[row].has_original_matlab_motion=true;
        }
        return Result<std::vector<trajectory::FrameKinematics>,NumericalErrorCode>::success(std::move(out));
    }
    Result<DirectionSet,NumericalErrorCode> sample(int level) const override {
        require(level==(mode.runs==8?4:2),"Original requested sampling level");
        const auto fixed=directions();DirectionMatrix matrix(kDirections,3);
        for(std::size_t row=0;row<kDirections;++row)for(int j=0;j<3;++j)matrix(row,j)=static_cast<double>(fixed[row].direction[j]);
        return DirectionSet::fromOwned(std::move(matrix));
    }
    Result<Eigen::VectorXd,NumericalErrorCode> nearnessInUnitSphere(const DirectionSet& d,const Vector3& p,const Vector3& w) const override {
        const std::size_t ordinal=near_calls++,row=ordinal%(mode.frames-2)+1,run=ordinal/(mode.frames-2);
        require(run<mode.runs&&p.x()==double(row)/1024&&w.isZero(0)&&d.directions().rows()==kDirections,"Scene source order");
        Eigen::VectorXd out(kDirections);const auto fixed=directions();
        for(std::size_t j=0;j<kDirections;++j)out[j]=static_cast<double>(nearness(run,row,fixed[j].direction));
        return Result<Eigen::VectorXd,NumericalErrorCode>::success(std::move(out));
    }
};
Real normalSample(std::size_t ordinal) {return Real(static_cast<int>((17*ordinal+13)%29)-14)/8;}
struct FixedRandom final:IRandomSource {
    std::size_t calls=0;
    Result<Scalar,NumericalErrorCode> normal() override {return Result<Scalar,NumericalErrorCode>::success(static_cast<double>(normalSample(calls++)));}
    Result<Scalar,NumericalErrorCode> uniform() override {throw std::runtime_error("Unexpected uniform draw");}
    Status<NumericalErrorCode> reseed(std::uint32_t) override {throw std::runtime_error("Unexpected reseed");}
};
ref::Matrix<3> previousRotation(std::size_t row) {
    const auto w=rotationVector(row-1);const Real angle=std::sqrt(ref::dot(w,w));
    ref::Matrix<3> out{};for(std::size_t j=0;j<3;++j)out[j][j]=1;
    if(angle==0)return out;
    const auto axis=ref::unit(w);const Real c=std::cos(angle),s=std::sin(angle);
    for(std::size_t column=0;column<3;++column) {
        ref::Vec basis{};basis[column]=1;const auto cross=ref::cross(axis,basis);
        for(std::size_t line=0;line<3;++line)out[line][column]=c*basis[line]+s*cross[line]+(1-c)*axis[line]*axis[column];
    }
    return out;
}
struct Input {ref::Field flow;Real ratio=0;};
Input referenceInput(Mode mode,std::size_t run,std::size_t row) {
    Input out;out.flow=directions();const auto t=translation(row),w=rotationVector(row);
    Real total_t=0,total_w=0;
    for(auto& point:out.flow) {
        const auto& d=point.direction;const Real mu=nearness(run,row,d),dot=ref::dot(t,d);
        auto rotational=ref::cross(w,d);ref::Vec translational{};
        for(std::size_t j=0;j<3;++j){translational[j]=-mu*(t[j]-dot*d[j]);point.flow[j]=translational[j]-rotational[j];}
        total_t+=std::sqrt(ref::dot(translational,translational));total_w+=std::sqrt(ref::dot(rotational,rotational));
    }
    out.ratio=total_w/total_t;
    if(mode.runs==8) {
        Values normals;const std::size_t begin=(run*(mode.frames-2)+row-1)*kDirections*kAxes;
        for(std::size_t j=0;j<kDirections*kAxes;++j)normals.push_back(normalSample(begin+j));
        out.flow=ref::nonproportionalNoise(out.flow,static_cast<Real>(kNoise),normals).field;
    }
    return out;
}
template<class T> void append(Values& out,const T& values){for(const auto v:values)out.push_back(v);}
Values flat(const ref::Step& s) {
    Values out{s.mean,s.mse};append(out,s.raw);append(out,s.after_sign);append(out,s.normalized);append(out,s.recomputed);
    for(const auto& row:s.matrix)append(out,row);
    append(out,s.rhs);append(out,s.solved);append(out,s.translation);append(out,s.rotation);append(out,s.next);return out;
}
ref::Coefficients coefficients(const HarmonicCoefficients& c) {
    ref::Coefficients out{};std::size_t next=0;
    for(int degree=0;degree<3;++degree)for(int order=0;order<5;++order) {
        if(order<=2*degree)out[next++]=c.values()(degree,order);
        else require(c.values()(degree,order)==0,"Unused coefficient slot");
    }
    return out;
}
Values flat(const estimators::AdaptiveStepResult& step) {
    const auto& t=step.trace;Values out{t.original_near_mean,step.error_mse};
    append(out,t.near_raw);append(out,t.near_after_sign);append(out,t.near_normalized);append(out,t.recomputed_near);
    for(int row=0;row<6;++row)for(int column=0;column<6;++column)out.push_back(t.system_matrix(row,column));
    append(out,t.rhs);append(out,t.solved_motion);append(out,step.motion.translation());append(out,step.motion.rotation());
    append(out,coefficients(step.coefficients_next));return out;
}
template<class T> void jsonArray(std::ostream& out,const T& values) {
    out<<'[';bool first=true;for(const auto v:values){if(!first)out<<',';first=false;out<<v;}out<<']';
}
Fields emptyFields(Mode m) {
    Fields out;const std::size_t columns=m.comparisons==4?4:1;
    for(const auto* name:{"allemittransfehler","allemitrotfehler"})out[name]=Values(m.frames*columns);
    if(m.comparisons==4)out["akrot"]=Values(m.frames);
    else for(const auto* name:{"alleohnetransfehler","alleohnerotfehler"})out[name]=Values(m.frames);
    if(m.runs==8)out["rottransratio"]=Values(m.frames);
    return out;
}
Real angle(const ref::Vec& truth,const ref::Vec& estimate) {
    const auto cross=ref::cross(truth,estimate);
    return std::atan2(std::sqrt(ref::dot(cross,cross)),ref::dot(truth,estimate))*180/ref::pi();
}
struct VerifiedStep final:estimators::IAdaptiveStepSolver {
    Mode mode;std::ostream& trace;
    mutable std::size_t calls=0;
    mutable ref::Session model;
    mutable ref::Frame frame;
    mutable Input input;
    mutable Fields expected;
    mutable Values ratio_first;
    estimators::AdaptiveMfaStepSolver real;
    VerifiedStep(Mode m,std::ostream& log):mode(m),trace(log),expected(emptyFields(m)),ratio_first(m.frames) {}
    void nextFrame(std::size_t run,std::size_t row) const {
        if(row==1)model=ref::Session{};
        input=referenceInput(mode,run,row);
        frame=model.advance(mode.reference,row+1,previousRotation(row),input.flow);
        const auto t=translation(row),w=rotationVector(row);
        for(std::size_t j=0;j<mode.comparisons;++j) {
            const auto& motion=frame.comparisons[j];
            const std::string prefix=mode.comparisons==4||j==1?"allemit":"alleohne";
            const auto offset=row+(mode.comparisons==4?j*mode.frames:0);
            expected[prefix+"transfehler"][offset]+=angle(t,motion.translation);
            expected[prefix+"rotfehler"][offset]+=ref::rotationErrorDegrees(w,motion.rotation);
        }
        if(mode.comparisons==4)expected["akrot"][row]=std::sqrt(ref::dot(w,w))*180/ref::pi();
        if(mode.runs==8){if(run==0)ratio_first[row]=input.ratio;if(run+1==8)expected["rottransratio"][row]=input.ratio;}
    }
    Result<estimators::AdaptiveStepResult,NumericalErrorCode> solve(const HarmonicCoefficients& old,const FlowField& flow) const override {
        const auto ordinal=calls++,slots=mode.comparisons+1;
        const auto run=ordinal/(slots*(mode.frames-2)),row=(ordinal/slots)%(mode.frames-2)+1,slot=ordinal%slots;
        require(run<mode.runs,"Original repeated loop limit");if(slot==0)nextFrame(run,row);
        require(flow.observations().rows()==kDirections,"Full synthetic field forwarded");
        for(std::size_t n=0;n<kDirections;++n)for(std::size_t axis=0;axis<3;++axis) {
            require(flow.observations()(n,axis)==input.flow[n].direction[axis],"Direction order unchanged");
            near(flow.observations()(n,axis+3),input.flow[n].flow[axis],kFlowMargin,"Flow/noise",max_flow);
        }
        const auto incoming=coefficients(old);
        const auto& wanted=slot==mode.comparisons?frame.prepared_primary:frame.prepared_comparisons[slot];
        for(std::size_t i=0;i<9;++i)near(incoming[i],wanted[i],kStepMargin,"Input coefficient",max_step,true);
        const auto& reference=slot==mode.comparisons?frame.primary:frame.comparisons[slot];
        const auto result=real.solve(old,flow);
        require(result.hasValue(),result.hasValue()?"":"Real Step: "+result.error().detail.message);
        const auto actual=flat(result.value()),target=flat(reference);
        trace<<"{\"mode\":\""<<mode.name<<"\",\"run\":"<<run+1<<",\"frame\":"<<row+1<<",\"slot\":"<<slot<<",\"input\":";
        jsonArray(trace,incoming);trace<<",\"expected\":";jsonArray(trace,target);trace<<",\"actual\":";jsonArray(trace,actual);trace<<"}\n";
        require(actual.size()==target.size(),"Step node dimensions");
        for(std::size_t i=0;i<actual.size();++i)near(actual[i],target[i],kStepMargin,"Step node "+std::to_string(i),max_step,true);
        ++total_steps;return result;
    }
    Fields finalReference() const {
        require(calls==mode.runs*(mode.frames-2)*(mode.comparisons+1),"All real Steps completed");
        Fields out=expected;
        if(mode.runs==8) {
            for(auto& field:out)if(field.first!="rottransratio")for(auto& value:field.second)value/=8;
            require(std::abs(out.at("rottransratio")[1]-ratio_first[1])>kOutputMargin,"Fixed input distinguishes final-run ratio");
        }
        return out;
    }
};
std::string readText(const fs::path& p) {
    std::ifstream in(p);require(in.is_open(),"Read saved text");
    return std::string(std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>());
}
std::string digest(const fs::path& p) {
    const int fd=::open(p.c_str(),O_RDONLY|O_NOFOLLOW|O_CLOEXEC);require(fd>=0,"Read MAT identity");
    const auto hash=io::sha256Fd(fd);::close(fd);require(hash.hasValue(),"SHA-256");return hash.value();
}
void verifySaved(const fs::path& root,const io::RunMatWriter& writer,const RunContext& context,
                 Mode mode,const Fields& expected,std::ostream& evidence) {
    const auto manifest=readText(root/"manifest.json");const auto receipts=writer.fileEvidence();
    require(receipts.size()==2*expected.size()+1,"Current save set and validity metadata");
    require(!context.manifestDraft().files_writer_confirmed.empty()&&
            context.manifestDraft().files_writer_confirmed.front()=="egomotion_metric_policy",
            "Policy publication precedes data");
    const auto policy=archive_reference::decodeIndependent(root/"egomotion_metric_policy.mat");
    require(policy.name=="egomotion_metric_policy"&&policy.rows==1&&policy.cols==4&&
            policy.values==std::vector<double>{1,1,1,static_cast<double>(mode.runs)},
            "Independent metric-policy payload");
    for(const auto& field:expected) {
        const auto file=root/(field.first+".mat");const auto independent=archive_reference::decodeIndependent(file);
        const auto production=io::MatResultReader{}.readNamed(file,field.first);
        require(production.hasValue(),"Read produced MAT");
        const auto columns=field.second.size()/mode.frames;
        require(independent.name==field.first&&independent.rows==mode.frames&&independent.cols==columns&&
            independent.storage_type==archive_reference::kDouble&&independent.values.size()==field.second.size(),"Independent MAT metadata");
        const auto& actual=production.value().values_column_major;
        const auto validity_name="valid_"+field.first;
        const auto validity=archive_reference::decodeIndependent(root/(validity_name+".mat"));
        const auto validity_read=io::MatResultReader{}.readNamed(root/(validity_name+".mat"),validity_name);
        require(validity.name==validity_name&&validity.rows==mode.frames&&validity.cols==columns&&
                validity.values.size()==actual.size()&&validity_read.hasValue()&&
                validity.values==validity_read.value().values_column_major,
                "Independent same-shape validity readback");
        require(actual.size()==independent.values.size()&&std::memcmp(actual.data(),independent.values.data(),actual.size()*sizeof(double))==0,
            "Independent v5 decoder and production Reader bitwise agree");
        for(std::size_t i=0;i<actual.size();++i) {
            const bool endpoint=i%mode.frames==0||i%mode.frames+1==mode.frames;
            if(endpoint)require(actual[i]==0,"Source zero endpoints");
            require(validity.values[i]==(endpoint?0.0:(field.first=="rottransratio"?1.0:static_cast<double>(mode.runs))),
                    "Complete synthetic data retains full planned count at every applicable cell");
            near(actual[i],field.second[i],kOutputMargin,"Published "+field.first+" index "+std::to_string(i),max_output);++published_values;
        }
        const auto hash=digest(file);bool receipt_found=false;
        for(const auto& receipt:receipts)if(receipt.variable==field.first)receipt_found=receipt.durably_confirmed&&receipt.sha256==hash&&receipt.size_bytes==fs::file_size(file);
        require(receipt_found&&manifest.find(hash)!=std::string::npos,"Manifest/receipt matches actual file");
        evidence<<"{\"mode\":\""<<mode.name<<"\",\"variable\":\""<<field.first<<"\",\"rows\":"<<mode.frames<<",\"columns\":"<<columns<<",\"sha256\":\""<<hash<<"\",\"expected\":";
        jsonArray(evidence,field.second);evidence<<",\"actual\":";jsonArray(evidence,actual);evidence<<"}\n";
    }
    std::size_t mats=0;for(const auto& entry:fs::directory_iterator(root))mats+=entry.path().extension()==".mat";
    require(mats==2*expected.size()+1,"Only source fields, validity companions and policy are present");
}
void execute(Mode mode,const fs::path& base,const std::string& sha,std::ostream& nodes,std::ostream& arrays) {
    Scene fixture(mode);const simulation::MeanMagnitudeGaussianNoise noise;
    VerifiedStep step(mode,nodes);
    ProductionExperimentRunner runner({fixture,noise},{fixture,fixture,fixture},&step);
    fs::create_directories(base/"origin");auto made=io::RunDirectory::create(base);
    require(made.hasValue(),"Exclusive sandbox RunId");auto directory=std::move(made).value();
    io::RunMatWriter writer(*directory);io::RunTextPublisher publisher(*directory);
    const auto root=directory->outputRoot()/directory->runId().text;
    require(publisher.publishNew("SYNTHETIC_INPUTS.txt","Fixed synthetic scene and RNG; real numerical Step/Session/Runner/MAT. Not the original trajectory or a scientific experiment.\n").hasValue(),"Synthetic marker");
    auto random=std::make_unique<FixedRandom>();const auto* counts=random.get();
    RunContext context(directory->runId(),std::move(random),writer,publisher,directory->outputRoot(),
        {sha,"synthetic-input:real-step-independent-delivery",std::nullopt},mode.kind,mode.runs);
    std::cout<<"Delivery RunId="<<directory->runId().text<<" mode="<<mode.name<<" path="<<root<<'\n';
    auto outcome=mode.kind==ExperimentKind::AdaptiveInitialization?runner.runAdaptiveInitialization({},context):
        (mode.kind==ExperimentKind::AdaptiveComparison?runner.runAdaptiveComparison({kNoise},context):runner.runAdaptiveUpdateRates({},context));
    if(!outcome.hasValue()) {
        const auto saved=context.publishFailureRecord(outcome.error());
        std::cerr<<jsonFailure(outcome.error(),5)<<" failure_manifest="<<saved.hasValue()<<'\n';
    }
    require(outcome.hasValue(),"Synthetic real-step delivery must finish");
    const auto valid=mode.runs*(mode.frames-2);const auto& result=outcome.value();
    require(result.status==RunStatus::Complete&&result.attempted==mode.runs&&result.completed==mode.runs&&
        result.valid==valid&&result.failed==0&&result.undefined==0,"Exact Complete counts");
    require(fixture.near_calls==valid&&counts->calls==(mode.runs==8?valid*kDirections*kAxes:0),"Scene and continuous RNG counts");
    const auto manifest=readText(root/"manifest.json");
    require(manifest.find(sha)!=std::string::npos&&manifest.find("synthetic-input:real-step-independent-delivery")!=std::string::npos&&
        manifest.find("\"completeness\":\"Complete\"")!=std::string::npos,"Persisted synthetic execution identity");
    verifySaved(root,writer,context,mode,step.finalReference(),arrays);
    std::cout<<"Delivery complete mode="<<mode.name<<" valid="<<valid<<" completed="<<result.completed
             <<" steps="<<step.calls<<" normal_draws="<<counts->calls<<'\n';
}
} // namespace
int main(int argc,char** argv) {
    try {
        require(argc==3,"Sandbox root and source SHA required");fs::create_directories(argv[1]);
        std::string pattern=(fs::absolute(argv[1])/"真实数值交付-XXXXXX").string();
        require(::mkdtemp(pattern.data())!=nullptr,"Exclusive retained fixture directory");
        const fs::path root(pattern);std::ofstream nodes(root/"step-nodes.jsonl"),arrays(root/"published-values.jsonl");
        nodes.exceptions(std::ios::badbit|std::ios::failbit);arrays.exceptions(std::ios::badbit|std::ios::failbit);
        nodes<<std::setprecision(std::numeric_limits<Real>::max_digits10);arrays<<std::setprecision(std::numeric_limits<Real>::max_digits10);
        std::cout<<"Independent delivery sandbox="<<root<<'\n';
        for(const auto mode:modes)execute(mode,root,argv[2],nodes,arrays);
        nodes.close();arrays.close();
        std::cout<<std::setprecision(21)<<"Adaptive independent delivery checks="<<checks<<" real_steps="<<total_steps
            <<" published_values="<<published_values<<" max_step_abs="<<max_step<<" max_flow_abs="<<max_flow<<" max_output_abs="<<max_output<<'\n';
        return 0;
    } catch(const std::exception& e){std::cerr<<"Independent delivery failure: "<<e.what()<<'\n';return 1;}
}
