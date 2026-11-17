// 六个轴向、成对反向视线的独立解析降维参考；无 Eigen 数值求解依赖。
// norm_koenderink_iteration.m: d48048f37b859bfb19402ca85b0f4c20e76afbfd
// mue_norm_koenderink_iteration.m: 9ffdac1baaecac06db20bcc6849bf4cdd165a484
#include "egomotion/estimators/franz_kvd.hpp"

#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {
using namespace egomotion;
using namespace egomotion::estimators;
using Triple=std::array<long double,3>;
constexpr std::size_t kMinimum=6,kMaximum=200,kRayCount=6;
constexpr long double kAccuracy=1000;
// 使用生产默认 binary64 常量的精确提升，而非改写为另一精度的参数。
constexpr long double kAbsolute=static_cast<long double>(1e-7);
constexpr long double kEpsilon=static_cast<long double>(1e-13);
int checks=0,failures=0;
void check(bool pass,const std::string& message) {
    ++checks;
    if (!pass) { ++failures; std::cerr<<message<<'\n'; }
}
long double length(const Triple& a) {
    return std::sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
}
Triple normalize(Triple value) {
    const long double norm=length(value);
    for (auto& component:value) component/=norm;
    return value;
}
struct Scene {
    Triple mu;
    double scale;
    double tangent_bias=0;
};
struct ReferenceStep {
    long double rms,delta,threshold;
    bool next;
};
std::vector<ReferenceStep> independentReference(const Scene& scene,bool modified) {
    const Triple truth{{scene.scale,2*scene.scale,3*scene.scale}};
    std::array<Triple,3> speed{};
    for (std::size_t axis=0;axis<3;++axis)
        for (std::size_t i=0;i<3;++i)
            if (i!=axis) speed[axis][i]=scene.mu[axis]*truth[i];
    // x 对跖视线的 y 分量加入相同切向扰动，保留旋转项的成对相消。
    speed[0][1]+=scene.tangent_bias*scene.scale;
    Triple t{};
    for (std::size_t i=0;i<3;++i)
        t[i]=(speed[(i+1)%3][i]+speed[(i+2)%3][i])/2;
    t=normalize(t);
    long double previous=0;
    std::vector<ReferenceStep> result;
    for (std::size_t iteration=1;iteration<=kMaximum;++iteration) {
        Triple near{},candidate{};
        for (std::size_t axis=0;axis<3;++axis) {
            long double transverse_dot=0;
            for (std::size_t i=0;i<3;++i)
                if (i!=axis) transverse_dot+=t[i]*speed[axis][i];
            near[axis]=transverse_dot/(1-t[axis]*t[axis]+kEpsilon);
        }
        // 对跖视线使所有旋转交叉和为零；omega 保持其精确 Franz 初值。
        // 每轴两条视线的合并除数为 3，与生产逐条累加的实现相互独立。
        for (std::size_t i=0;i<3;++i) {
            long double transverse_sum=0;
            for (std::size_t j=0;j<3;++j)
                if (j!=i) transverse_sum+=speed[j][i]*(modified?1:near[j]);
            candidate[i]=(transverse_sum+
                          t[i]*near[i]*(modified?1:near[i]))/3;
        }
        t=normalize(candidate);
        long double squared_error=0;
        for (std::size_t axis=0;axis<3;++axis)
            for (std::size_t i=0;i<3;++i)
                if (i!=axis) {
                    const long double difference=near[axis]*t[i]-speed[axis][i];
                    squared_error+=difference*difference;
                }
        const long double rms=std::sqrt(squared_error/3);
        const long double delta=std::abs(rms-previous);
        const long double threshold=rms/(kAccuracy*kRayCount);
        previous=rms;
        // 有界 for + 两个提前退出判断，不复用生产 shouldContinue。
        bool next=iteration<kMaximum;
        if (iteration>=kMinimum && (delta<=threshold || delta<=kAbsolute)) next=false;
        result.push_back({rms,delta,threshold,next});
        if (!next) break;
    }
    return result;
}
FlowField makeField(const Scene& scene) {
    const std::array<double,3> translation{{scene.scale,2*scene.scale,3*scene.scale}};
    const std::array<double,3> rotation{{0.25*scene.scale,-0.5*scene.scale,0.75*scene.scale}};
    FlowField field;
    field.observations.setZero(kRayCount,6);
    for (std::size_t axis=0;axis<3;++axis) {
        for (std::size_t side=0;side<2;++side) {
            const auto row=static_cast<Eigen::Index>(2*axis+side);
            const double sign=side==0?1:-1;
            field.observations(row,static_cast<Eigen::Index>(axis))=sign;
            for (std::size_t i=0;i<3;++i) {
                double cross=0;
                if ((i+2)%3==axis) cross=rotation[(i+1)%3]*sign;
                if ((i+1)%3==axis) cross=-rotation[(i+2)%3]*sign;
                field.observations(row,static_cast<Eigen::Index>(i+3))=
                    (i==axis?0:-static_cast<double>(scene.mu[axis])*translation[i])-cross-
                    (axis==0&&i==1?scene.tangent_bias*scene.scale:0);
            }
        }
    }
    return field;
}
struct Coverage { std::size_t cap=0,dynamic=0,absolute=0,minimum_override=0; };
void inspect(const Scene& scene,bool modified,Coverage& coverage) {
    KvDParameters parameters;
    parameters.retain_trace=true;
    check(parameters.minimum_iterations==kMinimum&&parameters.nominal_maximum_iterations==kMaximum&&
          parameters.accuracy_factor==kAccuracy&&parameters.absolute_threshold==kAbsolute&&
          parameters.nearness_denominator_epsilon==kEpsilon&&parameters.initial_threshold==1e-4,
          "Source defaults changed");
    const FlowField field=makeField(scene);
    const auto expected=independentReference(scene,modified);
    const auto actual=modified?ModifiedKvDEstimator{}.estimate(field,parameters):
                               OriginalKvDEstimator{}.estimate(field,parameters);
    const std::string label=modified?"Modified":"Original";
    std::cout<<label<<" mu="<<scene.mu[0]<<','<<scene.mu[1]<<','<<scene.mu[2]
             <<" scale="<<scene.scale<<" tangent_bias="<<scene.tangent_bias
             <<" reference_iterations="<<expected.size();
    check(actual.hasValue(),label+": solver failed");
    if (!actual.hasValue()) { std::cout<<" FAILED "<<actual.error().detail.message<<'\n'; return; }
    const auto& value=actual.value();
    std::cout<<" actual_iterations="<<value.iteration_count;
    check(value.iteration_count==expected.size(),label+": independent stop count differs");
    check(value.trace.size()==expected.size(),label+": independent trace length differs");
    for (std::size_t i=0;i<value.trace.size()&&i<expected.size();++i) {
        const auto& step=value.trace[i];
        check(step.k==i+1,label+": source iteration index changed");
        check(step.continue_after_iteration==expected[i].next,label+": independent continue differs");
        if (i+1<kMinimum && expected[i].delta<=kAbsolute &&
            step.residual_change<=parameters.absolute_threshold) ++coverage.minimum_override;
    }
    if (value.trace.empty()) { std::cout<<" EMPTY\n"; return; }
    const auto& last=value.trace.back();
    const auto& ref=expected.back();
    const bool cap=value.iteration_count==kMaximum&&last.residual_change>last.active_threshold&&
                   last.residual_change>parameters.absolute_threshold&&
                   ref.delta>ref.threshold&&ref.delta>kAbsolute;
    const bool dynamic=value.iteration_count<kMaximum&&last.residual_change<=last.active_threshold&&
                       last.residual_change>parameters.absolute_threshold&&
                       ref.delta<=ref.threshold&&ref.delta>kAbsolute;
    const bool absolute=value.iteration_count<kMaximum&&last.residual_change<=parameters.absolute_threshold&&
                        last.residual_change>last.active_threshold&&ref.delta<=kAbsolute&&ref.delta>ref.threshold;
    coverage.cap+=cap; coverage.dynamic+=dynamic; coverage.absolute+=absolute;
    std::cout<<" delta="<<last.residual_change<<" dynamic="<<last.active_threshold
             <<" reference_delta="<<ref.delta<<" reference_dynamic="<<ref.threshold
             <<" cap_only="<<cap<<" dynamic_only="<<dynamic<<" absolute_only="<<absolute<<'\n';
}
}  // namespace

int main() {
    std::cout<<std::setprecision(18);
    std::array<Coverage,2> coverage{};
    // 固定有限集合，无 RNG、参数搜索或完整科学实验；保留全部场景输出与失败。
    constexpr std::array<long double,5> near_values{{1,4,16,64,256}};
    for (auto a:near_values) for (auto b:near_values) for (auto c:near_values)
        for (double scale:{1.0,1e-12}) for (std::size_t method=0;method<2;++method)
            inspect({{{a,b,c}},scale},method==1,coverage[method]);
    // 物理一致的无噪声族可能直接收敛至零；明确固定扰动提供非零残差地板。
    for (double bias:{1.0,4.0,16.0}) for (std::size_t method=0;method<2;++method)
        inspect({{{1,4,16}},1.0,bias},method==1,coverage[method]);
    for (std::size_t method=0;method<2;++method) {
        const auto& counts=coverage[method];
        std::cout<<(method==0?"Original":"Modified")<<" cap="<<counts.cap
                 <<" dynamic="<<counts.dynamic<<" absolute="<<counts.absolute
                 <<" minimum_override="<<counts.minimum_override<<'\n';
        check(counts.cap>0,"Missing actual default 200-iteration cap coverage");
        check(counts.dynamic>0,"Missing exclusive dynamic threshold exit");
        check(counts.absolute>0,"Missing exclusive absolute threshold exit");
        check(counts.minimum_override>0,"Missing actual minimum-iteration override");
    }
    std::cout<<"KvD stopping checks="<<checks<<" failures="<<failures<<'\n';
    return failures==0?0:1;
}
