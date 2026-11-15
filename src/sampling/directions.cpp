#include "egomotion/sampling/directions.hpp"

#include <Eigen/Core>

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <new>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace egomotion::sampling {
namespace {
using Face = std::array<Vector3, 3>;

template<class Value>
Result<Value, NumericalErrorCode> failure(NumericalErrorCode code,
                                          std::string message,
                                          std::optional<std::size_t> index = std::nullopt) {
    ErrorDetail detail;
    detail.stage = ErrorStage::Simulation;
    detail.message = std::move(message);
    detail.observation_index = index;
    return Result<Value, NumericalErrorCode>::failure({code, std::move(detail)});
}

std::vector<Face> octahedralFaces(bool occluded) {
    const Vector3 top(0,0,1), bottom(0,0,-1);
    const Vector3 east(1,0,0), west(-1,0,0);
    const Vector3 north(0,1,0), south(0,-1,0);
    const std::array<Face,8> faces = {{
        {{top,east,north}}, {{top,east,south}},
        {{top,west,north}}, {{top,west,south}},
        {{bottom,east,north}}, {{bottom,east,south}},
        {{bottom,west,north}}, {{bottom,west,south}}
    }};
    if (!occluded) return {faces.begin(), faces.end()};
    // Original face IDs 2,3,5,6,7,8 (MATLAB 1-based).
    return {faces[1],faces[2],faces[4],faces[5],faces[6],faces[7]};
}
std::vector<Face> icosahedralFaces() {
    const double phi = (1.0+std::sqrt(5.0))/2.0;
    const std::array<double,12> x = {0,0,0,0,-1,-1,1,1,-phi,phi,phi,-phi};
    const std::array<double,12> y = {-1,-1,1,1,-phi,phi,phi,-phi,0,0,0,0};
    const std::array<double,12> z = {-phi,phi,phi,-phi,0,0,0,0,-1,-1,1,1};
    const std::array<std::array<int,3>,20> faces = {{
        {{1,4,9}}, {{1,5,9}}, {{1,8,5}}, {{1,8,10}}, {{1,10,4}},
        {{12,2,5}}, {{12,2,3}}, {{12,3,6}}, {{12,6,9}}, {{12,9,5}},
        {{11,7,10}}, {{11,10,8}}, {{11,8,2}}, {{11,2,3}}, {{11,3,7}},
        {{2,5,8}}, {{10,4,7}}, {{3,6,7}}, {{6,7,4}}, {{6,4,9}}
    }};
    std::vector<Face> result;
    result.reserve(faces.size());
    for (const auto& f : faces) {
        const auto v = [&](int one_based) -> Vector3 {
            const auto i = static_cast<std::size_t>(one_based-1);
            return {x[i],y[i],z[i]};
        };
        result.push_back({v(f[0]), v(f[1]), v(f[2])});
    }
    return result;
}

bool normalizeSource(const Vector3& raw, Vector3& output) {
    // The source uses sqrt(dot(v,v)) rather than a changed normalization rule.
    const double length = std::sqrt(raw.dot(raw));
    if (!std::isfinite(length) || length == 0.0) return false;
    output = raw/length;
    return output.allFinite();
}

// Recursive depth-first A,B,C,D. The source also evaluates midpoint vectors
// at leaf depth; keep that operation order for traceability.
bool emitFace(const Face& vertices, int level, int stop,
              DirectionSet& result, Eigen::Index& row) {
    const auto& x = vertices[0];
    const auto& y = vertices[1];
    const auto& z = vertices[2];
    Vector3 xy, xz, yz;
    if (!normalizeSource((x+y)/2.0, xy) ||
        !normalizeSource((x+z)/2.0, xz) ||
        !normalizeSource((y+z)/2.0, yz)) return false;
    if (level >= stop) {
        Vector3 center;
        if (!normalizeSource((x+y+z)/3.0, center)) return false;
        result.directions.row(row++) = center.transpose();
        return true;
    }
    return emitFace({x,xz,xy}, level+1, stop, result, row) &&
           emitFace({y,xy,yz}, level+1, stop, result, row) &&
           emitFace({z,xz,yz}, level+1, stop, result, row) &&
           emitFace({xy,xz,yz}, level+1, stop, result, row);
}

Result<DirectionSet, NumericalErrorCode> generate(
    int level, std::vector<Face> faces) {
    if (level < 0)
        return failure<DirectionSet>(NumericalErrorCode::InvalidSubdivisionLevel,
                                     "Negative triangle subdivision level");
    std::size_t count = faces.size();
    for (int i=0; i<level; ++i) {
        if (count > static_cast<std::size_t>(
                std::numeric_limits<Eigen::Index>::max())/4)
            return failure<DirectionSet>(NumericalErrorCode::SampleCountOverflow,
                                         "Subdivision sample count overflows Eigen::Index");
        count *= 4;
    }
    if (count > static_cast<std::size_t>(std::numeric_limits<Eigen::Index>::max()))
        return failure<DirectionSet>(NumericalErrorCode::SampleCountOverflow,
                                     "Subdivision row count overflows Eigen::Index");
    try {
        DirectionSet result;
        result.directions.resize(static_cast<Eigen::Index>(count), 3);
        Eigen::Index row=0;
        for (const Face& face : faces) {
            if (!emitFace(face,0,level,result,row))
                return failure<DirectionSet>(NumericalErrorCode::InvalidDirection,
                                             "Triangle midpoint or centroid undefined",
                                             static_cast<std::size_t>(row));
        }
        if (row!=result.directions.rows())
            return failure<DirectionSet>(NumericalErrorCode::InvalidShape,
                                         "Sampler did not emit every expected row");
        return Result<DirectionSet,NumericalErrorCode>::success(std::move(result));
    } catch (const std::bad_alloc&) {
        return failure<DirectionSet>(NumericalErrorCode::ResourceLimitExceeded,
                                     "Cannot allocate direction samples");
    }
}
}  // namespace

Result<DirectionSet,NumericalErrorCode> OctahedralSampler::sample(int level) const {
    return generate(level, octahedralFaces(false));
}
Result<DirectionSet,NumericalErrorCode> OccludedOctahedralSampler::sample(int level) const {
    return generate(level, octahedralFaces(true));
}
Result<DirectionSet,NumericalErrorCode> IcosahedralSampler::sample(int level) const {
    return generate(level, icosahedralFaces());
}

Result<Vector3, NumericalErrorCode> randomUnitDirection(IRandomSource& random) {
    auto first = random.uniform();
    if (!first.hasValue()) {
        ErrorDetail stage;
        stage.stage = ErrorStage::Simulation;
        stage.message = "getvektor: first uniform call failed";
        return Result<Vector3,NumericalErrorCode>::failure(
            wrapFailure(first.error().code, std::move(stage), first.error()));
    }
    const double u1 = first.value();
    if (!std::isfinite(u1) || u1 < 0.0 || u1 >= 1.0)
        return failure<Vector3>(NumericalErrorCode::InvalidRandomSample,
                                "getvektor: invalid first uniform");
    auto second = random.uniform();
    if (!second.hasValue()) {
        ErrorDetail stage;
        stage.stage = ErrorStage::Simulation;
        stage.message = "getvektor: second uniform call failed";
        return Result<Vector3,NumericalErrorCode>::failure(
            wrapFailure(second.error().code, std::move(stage), second.error()));
    }
    const double u2 = second.value();
    if (!std::isfinite(u2) || u2 < 0.0 || u2 >= 1.0)
        return failure<Vector3>(NumericalErrorCode::InvalidRandomSample,
                                "getvektor: invalid second uniform");
    const double pi = std::acos(-1.0);
    const double theta = pi/2.0-std::asin(2.0*u1-1.0);
    const double phi = u2*2.0*pi;
    const Vector3 v(std::sin(theta)*std::cos(phi),
                    std::sin(theta)*std::sin(phi),std::cos(theta));
    if (!v.allFinite())
        return failure<Vector3>(NumericalErrorCode::NonFiniteResult,
                                "getvektor generated a non-finite direction");
    return Result<Vector3,NumericalErrorCode>::success(v);
}

}  // namespace egomotion::sampling
