#pragma once

// 测试专用L3候选：仅标准库long double标量代数，不调用Eigen或生产数值函数。
// 推导/固定输入/来源边界见verification/adaptive/session_multiframe_reference.md。
#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace adaptive_reference {
using Real = long double;
template<std::size_t N> using Vector = std::array<Real, N>;
template<std::size_t N> using Matrix = std::array<Vector<N>, N>;
using Vec = Vector<3>;
using Coefficients = Vector<9>; // 00,1-1,10,11,2-2,2-1,20,21,22
struct Observation { Vec direction, flow; };
using Field = std::vector<Observation>;
inline Real pi() { return std::acos(Real(-1)); }
inline Real dot(const Vec& a, const Vec& b) {
    Real sum = 0; for (std::size_t j = 0; j < 3; ++j) sum += a[j] * b[j]; return sum;
}
inline Vec cross(const Vec& a, const Vec& b) {
    return {a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]};
}
inline Vec unit(const Vec& v) {
    const Real length = std::sqrt(dot(v, v));
    if (!(length > 0) || !std::isfinite(length)) throw std::runtime_error("Reference zero/nonfinite vector");
    return {v[0]/length, v[1]/length, v[2]/length};
}
inline Real angleDegrees(const Vec& a, const Vec& b) {
    const auto x=unit(a),y=unit(b);
    const auto perpendicular=cross(x,y);
    return std::atan2(std::sqrt(dot(perpendicular,perpendicular)),dot(x,y))*180/pi();
}
// DEC-R6-02：每帧旋转向量代表Exp(w)，用独立四元数乘法核对SO(3)测地角。
// 此参考不调用Eigen/Sophus或生产指标函数；零向量映射单位旋转。
inline Vector<4> rotationQuaternion(const Vec& vector) {
    const Real length=std::sqrt(dot(vector,vector));
    if(!std::isfinite(length))throw std::runtime_error("Reference nonfinite rotation");
    if(length==0)return {1,0,0,0};
    const Real scale=std::sin(length/2)/length;
    return {std::cos(length/2),scale*vector[0],scale*vector[1],scale*vector[2]};
}
inline Real rotationErrorDegrees(const Vec& a,const Vec& b) {
    const auto x=rotationQuaternion(a),y=rotationQuaternion(b);
    const Real scalar=x[0]*y[0]+x[1]*y[1]+x[2]*y[2]+x[3]*y[3];
    const Vec imaginary{
        x[0]*y[1]-x[1]*y[0]-x[2]*y[3]+x[3]*y[2],
        x[0]*y[2]+x[1]*y[3]-x[2]*y[0]-x[3]*y[1],
        x[0]*y[3]-x[1]*y[2]+x[2]*y[1]-x[3]*y[0]};
    return 2*std::atan2(std::sqrt(dot(imaginary,imaginary)),std::abs(scalar))*180/pi();
}
struct NoisyField {
    Field field;
    Real mean_magnitude=0;
};
// nonpropfehler：先完整求平均模长，再按每行x/y/z加噪，最后作切向投影。
// normals为固定输入，不在参考实现中调用生产RNG/噪声函数。
inline NoisyField nonproportionalNoise(const Field& ideal,Real scale,const std::vector<Real>& normals) {
    if(ideal.empty()||normals.size()!=3*ideal.size())throw std::runtime_error("Reference noise shape");
    NoisyField result;
    for(const auto& row:ideal)result.mean_magnitude+=std::sqrt(dot(row.flow,row.flow));
    result.mean_magnitude/=static_cast<Real>(ideal.size());
    for(std::size_t row=0;row<ideal.size();++row) {
        const auto& d=ideal[row].direction;
        Vec perturbed=ideal[row].flow;
        for(std::size_t axis=0;axis<3;++axis)perturbed[axis]+=normals[3*row+axis]*scale*result.mean_magnitude;
        const Real radial=dot(perturbed,d);
        for(std::size_t axis=0;axis<3;++axis)perturbed[axis]-=radial*d[axis];
        result.field.push_back({d,perturbed});
    }
    return result;
}
inline Coefficients basis(const Vec& d) {
    const Real x=d[0], y=d[1], z=d[2];
    const Real one=std::sqrt(3/(4*pi())), mixed=std::sqrt(15/(4*pi()));
    return {1/std::sqrt(4*pi()), -one*y, one*z, -one*x, mixed*x*y,
        -mixed*y*z, std::sqrt(5/(16*pi()))*(3*z*z-1), -mixed*x*z,
        std::sqrt(15/(16*pi()))*(x*x-y*y)};
}
inline Real evaluate(const Coefficients& coefficients, const Vec& direction) {
    const auto y=basis(direction);
    Real result=0; for(std::size_t j=0;j<y.size();++j)result+=coefficients[j]*y[j];
    return result;
}
inline Coefficients initial() { Coefficients c{}; c[0]=1; return c; }

// 把源CS展开写成 c00*Y00 + l·d + dᵀQd，按A=SRS作参数代换。
// 这是独立三维多项式运算，不复制生产IR递推或其5×5旋转块。
inline Coefficients rotate(const Coefficients& c, const Matrix<3>& r, bool source_phase=true) {
    const Real one=std::sqrt(3/(4*pi())), mixed=std::sqrt(15/(4*pi()));
    const Real zonal=std::sqrt(5/(16*pi())), difference=std::sqrt(15/(16*pi()));
    const Vec linear{-one*c[3],-one*c[1],one*c[2]};
    Matrix<3> q{{{-zonal*c[6]+difference*c[8], mixed*c[4]/2, -mixed*c[7]/2},
                 {mixed*c[4]/2, -zonal*c[6]-difference*c[8], -mixed*c[5]/2},
                 {-mixed*c[7]/2, -mixed*c[5]/2, 2*zonal*c[6]}}};
    const Vec signs=source_phase?Vec{-1,-1,1}:Vec{1,1,1};
    Matrix<3> a{}, transformed{}; Vec changed_linear{};
    for(std::size_t i=0;i<3;++i)for(std::size_t j=0;j<3;++j)a[i][j]=signs[i]*r[i][j]*signs[j];
    for(std::size_t i=0;i<3;++i) {
        for(std::size_t j=0;j<3;++j)changed_linear[i]+=a[j][i]*linear[j];
        for(std::size_t j=0;j<3;++j)for(std::size_t k=0;k<3;++k)for(std::size_t l=0;l<3;++l)
            transformed[i][j]+=a[k][i]*q[k][l]*a[l][j];
    }
    return {c[0],-changed_linear[1]/one,changed_linear[2]/one,-changed_linear[0]/one,
        2*transformed[0][1]/mixed,-2*transformed[1][2]/mixed,transformed[2][2]/(2*zonal),
        -2*transformed[0][2]/mixed,(transformed[0][0]-transformed[1][1])/(2*difference)};
}
inline Vector<6> solve(Matrix<6> a, Vector<6> b) {
    // 独立部分选主元Gauss-Jordan消元；生产使用Eigen FullPivLU。
    for(std::size_t column=0;column<6;++column) {
        std::size_t pivot=column;
        for(std::size_t row=column+1;row<6;++row)
            if(std::abs(a[row][column])>std::abs(a[pivot][column]))pivot=row;
        if(a[pivot][column]==0)throw std::runtime_error("Singular independent reference system");
        std::swap(a[column],a[pivot]); std::swap(b[column],b[pivot]);
        const Real divisor=a[column][column];
        for(std::size_t j=0;j<6;++j)a[column][j]/=divisor;
        b[column]/=divisor;
        for(std::size_t row=0;row<6;++row)if(row!=column) {
            const Real multiplier=a[row][column];
            for(std::size_t j=0;j<6;++j)a[row][j]-=multiplier*a[column][j];
            b[row]-=multiplier*b[column];
        }
    }
    for(const auto value:b)if(!std::isfinite(value))throw std::runtime_error("Nonfinite reference solve");
    return b;
}
struct Step {
    std::vector<Real> raw, after_sign, normalized, recomputed;
    Real mean=0, mse=0;
    Matrix<6> matrix{};
    Vector<6> rhs{}, solved{};
    Vec translation{}, rotation{};
    Coefficients next{};
};
inline Step step(const Coefficients& old, const Field& field) {
    if(field.empty())throw std::runtime_error("Empty reference field");
    Step result; const Real count=static_cast<Real>(field.size());
    for(const auto& observation:field) {
        result.raw.push_back(evaluate(old,observation.direction));
        result.mean+=result.raw.back()/count;
    }
    if(result.mean==0||!std::isfinite(result.mean))throw std::runtime_error("Reference invalid nearness mean");
    // 原s在翻号后保持不变；mu=raw/abs(s)，负均值分支不改成正均值。
    for(const auto near:result.raw) {
        result.after_sign.push_back(result.mean<0?-near:near);
        result.normalized.push_back(near/std::abs(result.mean));
    }
    for(std::size_t j=0;j<6;++j)result.matrix[j][j]=1;
    for(std::size_t n=0;n<field.size();++n) {
        const auto& d=field[n].direction; const auto& p=field[n].flow;
        const Real mu=result.normalized[n];
        const Matrix<3> skew{{{0,-d[2],d[1]},{d[2],0,-d[0]},{-d[1],d[0],0}}};
        const auto rhs_rotation=cross(p,d);
        for(std::size_t i=0;i<3;++i) {
            result.rhs[i]-=p[i]/count; result.rhs[i+3]+=rhs_rotation[i]/count;
            for(std::size_t j=0;j<3;++j) {
                result.matrix[i][j]-=mu*d[i]*d[j]/count;
                result.matrix[i+3][j+3]-=d[i]*d[j]/count;
                result.matrix[i][j+3]-=skew[i][j]/count;
                result.matrix[i+3][j]+=mu*skew[i][j]/count;
            }
        }
    }
    result.solved=solve(result.matrix,result.rhs);
    result.translation=unit({result.solved[0],result.solved[1],result.solved[2]});
    result.rotation={result.solved[3],result.solved[4],result.solved[5]};
    Coefficients numerator{},denominator{};
    // oneit_adaptiv_matched_filter.m中的nulldiv=1e-11是算法常量，不是测试容差。
    constexpr Real source_denominator_epsilon=1e-11L;
    for(std::size_t n=0;n<field.size();++n) {
        const auto& d=field[n].direction; const auto& p=field[n].flow;
        const Real projection=dot(result.translation,d);
        const auto rotation_flow=cross(result.rotation,d);
        const Real divisor=1-projection*projection+source_denominator_epsilon;
        if(!(divisor>0))throw std::runtime_error("Reference new-near denominator");
        Real new_near=0;
        for(std::size_t j=0;j<3;++j) {
            const Real prediction=-result.normalized[n]*result.mean*(result.translation[j]-projection*d[j])-rotation_flow[j];
            const Real difference=p[j]-prediction;
            result.mse+=difference*difference/count;
            new_near-=result.translation[j]*(p[j]+rotation_flow[j])/divisor;
        }
        result.recomputed.push_back(new_near);
        const auto y=basis(d);
        for(std::size_t j=0;j<9;++j) { numerator[j]+=y[j]*new_near; denominator[j]+=y[j]*y[j]; }
    }
    // (sum/N)*4pi在分子分母中解析约去；仍是逐通道投影，不改成全矩阵最小二乘。
    for(std::size_t j=0;j<9;++j) {
        if(denominator[j]==0)throw std::runtime_error("Reference zero projection norm");
        result.next[j]=numerator[j]/denominator[j];
    }
    return result;
}
enum class Mode { Initialization, Comparison, UpdateRates };
struct Frame {
    Coefficients prepared_primary{};
    std::vector<Coefficients> prepared_comparisons;
    Step primary;
    std::vector<Step> comparisons;
};
struct Session {
    Coefficients primary=initial();
    std::array<Coefficients,4> baselines{{initial(),initial(),initial(),initial()}};
    Frame advance(Mode mode,std::size_t matlab_frame,const Matrix<3>& rotation,const Field& field) {
        Frame result;
        result.prepared_primary=mode==Mode::Initialization?primary:rotate(primary,rotation);
        if(mode==Mode::UpdateRates) {
            // 源many_parameter先全部旋转，再按MATLAB帧号刷新，比较输出dumi不回写。
            constexpr std::array<std::size_t,4> periods{{1,5,11,21}};
            for(std::size_t j=0;j<periods.size();++j) {
                baselines[j]=rotate(baselines[j],rotation);
                if(matlab_frame%periods[j]==0)baselines[j]=result.prepared_primary;
                result.prepared_comparisons.push_back(baselines[j]);
            }
        } else result.prepared_comparisons={initial(),result.prepared_primary};
        for(const auto& candidate:result.prepared_comparisons)result.comparisons.push_back(step(candidate,field));
        result.primary=step(result.prepared_primary,field);
        primary=result.primary.next;
        return result;
    }
};
} // namespace adaptive_reference
