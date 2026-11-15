#pragma once

#include "egomotion/contracts/interfaces.hpp"

#include <cstdint>
#include <random>

namespace egomotion::sampling {

// Deterministic for an implementation/toolchain, not MATLAB RandStream-equivalent.
class Mt19937RandomSource final : public IRandomSource {
public:
    explicit Mt19937RandomSource(std::uint32_t seed) : generator_(seed) {}
    void reseed(std::uint32_t seed) {
        generator_.seed(seed);
        normal_distribution_.reset();
        uniform_distribution_.reset();
    }
    Result<Scalar, NumericalErrorCode> uniform() override;
    Result<Scalar, NumericalErrorCode> normal() override;

private:
    std::mt19937 generator_;
    std::uniform_real_distribution<double> uniform_distribution_{0.0, 1.0};
    std::normal_distribution<double> normal_distribution_{0.0, 1.0};
};

}  // namespace egomotion::sampling
