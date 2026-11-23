#pragma once

#include "egomotion/contracts/result.hpp"

#include <Eigen/Core>

#include <cstddef>
#include <vector>

namespace egomotion::estimators {

struct KvDParameters {
    Scalar nearness_denominator_epsilon = 1e-13;
    Scalar initial_threshold = 1e-4;
    Scalar absolute_threshold = 1e-7;
    Scalar accuracy_factor = 1000.0;
    std::size_t minimum_iterations = 6;
    std::size_t nominal_maximum_iterations = 200;
    bool retain_trace = false; // optional detailed trace; no change to algorithm
};
struct FranzInitializationTrace {
    Vector3 translation_responses = Vector3::Zero();
    Vector3 translation_denominators = Vector3::Zero();
    Vector3 rotation_responses = Vector3::Zero();
    Vector3 rotation_denominators = Vector3::Zero();
    MotionEstimate before_translation_normalization;
    Scalar initial_translation_norm = 0;
};
struct KvDIterationTrace {
    std::size_t k = 0;  // MATLAB x, 1-based
    Vector3 translation_before_sign = Vector3::Zero();
    Vector3 translation_after_sign = Vector3::Zero();
    Vector3 rotation_before = Vector3::Zero();
    Eigen::VectorXd nearness_denominators;
    Eigen::VectorXd nearness_raw;
    Scalar nearness_sum_raw = 0;
    bool sign_flipped = false;
    Eigen::VectorXd nearness_after_sign;
    Vector3 translation_sum_a = Vector3::Zero();
    Vector3 translation_sum_b = Vector3::Zero();
    Vector3 translation_sum_c = Vector3::Zero();
    Vector3 translation_candidate_raw = Vector3::Zero();
    Scalar translation_candidate_norm = 0;
    Vector3 translation_candidate_normalized = Vector3::Zero();
    Vector3 rotation_sum_a = Vector3::Zero();
    Vector3 rotation_sum_b = Vector3::Zero();
    Vector3 rotation_sum_c = Vector3::Zero();
    Vector3 rotation_after = Vector3::Zero();
    Vector3 translation_after_commit = Vector3::Zero();
    Eigen::Matrix<double,Eigen::Dynamic,3> predicted_flow_xyz;
    Scalar residual_mse = 0;
    Scalar residual_rms = 0;
    Scalar previous_rms = 0;
    Scalar residual_change = 0;
    Scalar active_threshold = 0;
    bool continue_after_iteration = false;
};
struct KvDResult {
    MotionEstimate motion;
    Eigen::VectorXd final_nearness;
    Scalar final_nearness_mean = 0;
    Vector3 final_translation_direction_before_scale = Vector3::Zero();
    std::size_t iteration_count = 0;
    FranzInitializationTrace initial_franz;
    std::vector<KvDIterationTrace> trace;
};

struct KvDTranslationUpdate {
    Vector3 mean_a = Vector3::Zero();
    Vector3 mean_b = Vector3::Zero();
    Vector3 mean_c = Vector3::Zero();
    Vector3 raw_translation = Vector3::Zero();
};
class IKvDTranslationRule {
public:
    virtual ~IKvDTranslationRule() = default;
    // near和旧运动均为本轮翻号后的快照；策略不得归一化或提交状态。
    virtual Result<KvDTranslationUpdate,NumericalErrorCode> compute(
        const FlowField&, const Eigen::VectorXd& nearness,
        const MotionEstimate& old_motion) const = 0;
};
class OriginalKvDTranslationRule final : public IKvDTranslationRule {
public:
    Result<KvDTranslationUpdate,NumericalErrorCode> compute(
        const FlowField&, const Eigen::VectorXd&, const MotionEstimate&) const override;
};
class ModifiedKvDTranslationRule final : public IKvDTranslationRule {
public:
    Result<KvDTranslationUpdate,NumericalErrorCode> compute(
        const FlowField&, const Eigen::VectorXd&, const MotionEstimate&) const override;
};
class KvDIterationEngine {
public:
    // 每次调用仅借用策略，不保存流场、策略引用或跨调用迭代状态。
    Result<KvDResult,NumericalErrorCode> estimate(
        const FlowField&, const KvDParameters&, const IKvDTranslationRule&) const;
};

// 两种求解结果分别保留：Franz只返回运动，KvD还返回near/迭代trace。
// 不把Adaptive单步强制塞入一个丢失其状态结果的通用求解器。
class IFranzMatchedFilter {
public:
    virtual ~IFranzMatchedFilter() = default;
    virtual Result<MotionEstimate,NumericalErrorCode> estimate(const FlowField&) const = 0;
};
class IKvDEstimator {
public:
    virtual ~IKvDEstimator() = default;
    virtual Result<KvDResult,NumericalErrorCode> estimate(
        const FlowField&, const KvDParameters&) const = 0;
};

class FranzMatchedFilter final : public IFranzMatchedFilter {
public:
    Result<MotionEstimate,NumericalErrorCode> estimate(const FlowField& field) const override;
    Result<FranzInitializationTrace,NumericalErrorCode>
    estimateWithTrace(const FlowField& field) const;
};
class OriginalKvDEstimator final : public IKvDEstimator {
public:
    Result<KvDResult,NumericalErrorCode> estimate(
        const FlowField& field, const KvDParameters& parameters) const override;
private:
    KvDIterationEngine engine_;
    OriginalKvDTranslationRule translation_rule_;
};
class ModifiedKvDEstimator final : public IKvDEstimator {
public:
    Result<KvDResult,NumericalErrorCode> estimate(
        const FlowField& field, const KvDParameters& parameters) const override;
private:
    KvDIterationEngine engine_;
    ModifiedKvDTranslationRule translation_rule_;
};

}  // namespace egomotion::estimators
