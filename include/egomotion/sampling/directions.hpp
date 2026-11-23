#pragma once

#include "egomotion/contracts/interfaces.hpp"

namespace egomotion::sampling {

// Face order and A/B/C/D recursion match original MATLAB exactly.
class OctahedralSampler final : public IDirectionSampler {
public:
    Result<DirectionSet, NumericalErrorCode> sample(int level) const override;
};
class OccludedOctahedralSampler final : public IDirectionSampler {
public:
    Result<DirectionSet, NumericalErrorCode> sample(int level) const override;
};
class IcosahedralSampler final : public IDirectionSampler {
public:
    Result<DirectionSet, NumericalErrorCode> sample(int level) const override;
};

// getvektor.m consumes exactly two uniforms, theta first, phi second.
Result<Vector3, NumericalErrorCode> randomUnitDirection(IRandomSource& random);

}  // namespace egomotion::sampling
