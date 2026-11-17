#include "egomotion/experiments/adaptive_runner.hpp"

#include "egomotion/estimators/adaptive_session.hpp"
#include "egomotion/sampling/directions.hpp"
#include "egomotion/simulation/flow.hpp"
#include "egomotion/trajectory/trajectory.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace egomotion::experiments {
namespace {
using Output=Result<AdaptiveRunData,ExperimentErrorCode>;

enum class Mode { Initialization, Comparison, UpdateRates };
constexpr std::size_t kComparisonRuns=8;
constexpr std::size_t kInitializationFrames=35;
constexpr std::size_t kRegularFrames=600;
constexpr std::size_t kUpdateColumns=4;

std::string modeName(Mode m) {
    switch (m) {
    case Mode::Initialization: return "Initialization";
    case Mode::Comparison: return "Comparison";
    case Mode::UpdateRates: return "UpdateRates";
    }
    return "UnknownAdaptiveMode";
}

Output fail(ExperimentErrorCode code, std::string message) {
    ErrorDetail detail;
    detail.stage=ErrorStage::Simulation;
    detail.message=std::move(message);
    return Output::failure({code,std::move(detail)});
}

template<class T>
Output fromNumeric(const Result<T,NumericalErrorCode>& value,
                   const std::string& stage, Mode mode, std::size_t run,
                   std::size_t matlab_frame=0) {
    ErrorDetail detail;
    detail.stage=ErrorStage::Simulation;
    detail.message=modeName(mode)+": "+stage+
                   " (run_matlab_1_based="+std::to_string(run);
    if (matlab_frame!=0)
        detail.message+=", frame_matlab_1_based="+std::to_string(matlab_frame);
    detail.message+=")";
    // A frame index is not a ray index. Preserve the inner observation_index.
    return Output::failure(wrapFailure(
        ExperimentErrorCode::SolverFailed,std::move(detail),value.error()));
}
Output fromSession(const Failure<ExperimentErrorCode>& value, Mode mode,
                   std::size_t run, std::size_t matlab_frame) {
    ErrorDetail detail;
    detail.stage=ErrorStage::Simulation;
    detail.message=modeName(mode)+": Adaptive Session failure (run_matlab_1_based="+
                   std::to_string(run)+", frame_matlab_1_based="+
                   std::to_string(matlab_frame)+")";
    return Output::failure(wrapFailure(
        ExperimentErrorCode::SolverFailed,std::move(detail),value));
}

DenseDoubleArray allocateArray(std::string name, std::size_t rows,
                               std::size_t cols) {
    // The source stores double[rows x cols], in MATLAB column-major order.
    if (rows==0 || cols==0 || cols>std::numeric_limits<std::size_t>::max()/rows)
        throw std::length_error("Invalid Adaptive output matrix dimensions");
    const std::size_t size=rows*cols;
    if (size>std::numeric_limits<std::size_t>::max()/sizeof(double))
        throw std::length_error("Adaptive output exceeds addressable byte count");
    DenseDoubleArray array;
    array.info.name=std::move(name);
    array.info.dimensions={rows,cols};
    array.info.original_class_type=6; // MAT_C_DOUBLE, without exposing matio
    array.info.original_data_type=9;  // MAT_T_DOUBLE
    array.info.class_name="double";
    array.info.data_type_name="double";
    array.info.data_bytes=size*sizeof(double);
    array.values_column_major.assign(size,0.0);
    return array;
}
void addField(AdaptiveRunData& output, std::string name,
              std::size_t rows, std::size_t cols) {
    auto inserted=output.fields.emplace(name,allocateArray(name,rows,cols));
    if (!inserted.second)
        throw std::logic_error("Duplicate Adaptive current save name");
}

AdaptiveRunData newResult(Mode mode) {
    AdaptiveRunData out;
    out.frame_count=mode==Mode::Initialization?kInitializationFrames:kRegularFrames;
    out.planned_runs=mode==Mode::Comparison?kComparisonRuns:1;
    const std::size_t n=out.frame_count;
    const std::size_t cols=mode==Mode::UpdateRates?kUpdateColumns:1;
    addField(out,"allemittransfehler",n,cols);
    addField(out,"allemitrotfehler",n,cols);
    if (mode!=Mode::UpdateRates) {
        addField(out,"alleohnetransfehler",n,1);
        addField(out,"alleohnerotfehler",n,1);
    }
    if (mode==Mode::Comparison)
        addField(out,"rottransratio",n,1);
    if (mode==Mode::UpdateRates)
        addField(out,"akrot",n,1);
    return out;
}
void write(AdaptiveRunData& data,const std::string& field,
           std::size_t row,std::size_t column,double value) {
    auto& array=data.fields.at(field);
    const std::size_t rows=array.info.dimensions.at(0);
    const std::size_t columns=array.info.dimensions.at(1);
    if (row>=rows || column>=columns || !std::isfinite(value))
        throw std::logic_error("Invalid Adaptive output index or non-finite statistic");
    array.values_column_major[column*rows+row]=value;
}
void accumulate(AdaptiveRunData& aggregate,const AdaptiveRunData& one,
                bool include_ratio) {
    for (auto& item:aggregate.fields) {
        const auto it=one.fields.find(item.first);
        if (it==one.fields.end())throw std::logic_error("Missing Adaptive run field");
        auto& target=item.second.values_column_major;
        const auto& values=it->second.values_column_major;
        if (target.size()!=values.size())
            throw std::logic_error("Adaptive aggregate shape mismatch");
        if (item.first=="rottransratio") {
            if (include_ratio) target=values; // last run ONLY, never average
            continue;
        }
        for (std::size_t i=0;i<target.size();++i)
            target[i]+=values[i];
    }
}
void divideComparisonErrors(AdaptiveRunData& data) {
    // This is called ONLY after all eight complete runs have succeeded.
    for (auto& pair:data.fields) {
        if (pair.first=="rottransratio")continue;
        for (double& x:pair.second.values_column_major)
            x/=static_cast<double>(kComparisonRuns);
    }
}
Output compute(Mode mode, double noise_scale, IRandomSource* rng,
               const AdaptiveProgress& progress,
               const estimators::IAdaptiveStepSolver* injected_solver) {
    if (mode==Mode::Comparison &&
        (!std::isfinite(noise_scale)||rng==nullptr))
        return fail(ExperimentErrorCode::InvalidParameters,
                    "Comparison requires finite noise scale and injected RNG");
    try {
        AdaptiveRunData aggregate=newResult(mode);
        const trajectory::TrajectoryGenerator generator;
        const trajectory::TrajectoryKinematics kinematics;
        const trajectory::SphereEnvironment sphere;
        const sampling::IcosahedralSampler sampler;
        const simulation::FlowSynthesizer flow_synthesizer;
        const simulation::MeanMagnitudeGaussianNoise noise_model;

        Result<trajectory::TrajectoryPoints,NumericalErrorCode> points=
            mode==Mode::Initialization
            ? generator.circle(kInitializationFrames,0.6,0.3)
            : generator.sinusoidal(kRegularFrames,0.5,1.0,2,0.3);
        if (!points.hasValue())
            return fromNumeric(points,"trajectory generation",mode,1);
        auto motions=kinematics.fromPoints(points.value());
        if (!motions.hasValue())
            return fromNumeric(motions,"trajectory kinematics",mode,1);

        const int level=mode==Mode::Comparison?4:2;
        auto sampled=sampler.sample(level);
        if (!sampled.hasValue())
            return fromNumeric(sampled,"icosahedral sampling",mode,1);
        const DirectionSet& directions=sampled.value();
        const std::size_t n=aggregate.frame_count;
        const auto& frames=motions.value();
        if (frames.size()!=n)
            return fail(ExperimentErrorCode::DataMismatch,
                        "Trajectory and frame count do not match");

        for (std::size_t run=0;run<aggregate.planned_runs;++run) {
            if(progress)progress(AdaptiveProgressEvent::RunStarted);
            AdaptiveRunData current=newResult(mode);
            estimators::AdaptiveMfaSession session(
                mode==Mode::Initialization
                ? estimators::AdaptiveSessionMode::Initialization
                : (mode==Mode::Comparison
                    ? estimators::AdaptiveSessionMode::Comparison
                    : estimators::AdaptiveSessionMode::UpdateRates),injected_solver);
            // MATLAB i=2..N-1 corresponds to C++ j=1..N-2. Row zero and the
            // last row MUST remain original preallocated zeros, not estimates.
            for (std::size_t row=1;row+1<n;++row) {
                const std::size_t matlab_i=row+1;
                if (!frames[row].has_original_matlab_motion)
                    return fail(ExperimentErrorCode::DataMismatch,
                                "Expected source-valid trajectory interior frame");
                const Vector3 position=points.value().points.row(
                    static_cast<Eigen::Index>(row)).transpose();
                auto nearness=sphere.nearnessInUnitSphere(
                    directions,position,frames[row].absolute_rotation);
                if (!nearness.hasValue())
                    return fromNumeric(nearness,"sphere nearness",mode,
                                       run+1,matlab_i);
                auto ideal=flow_synthesizer.synthesize(
                    directions,nearness.value(),
                    frames[row].local_translation,frames[row].relative_rotation);
                if (!ideal.hasValue())
                    return fromNumeric(ideal,"ideal spherical flow",mode,
                                       run+1,matlab_i);
                // Read IDEAL component means before moving the owning flow.
                // Nonpropfehler changes observed flow but not these statistics.
                const double numerator=ideal.value().mean_rotational_magnitude;
                const double denominator=ideal.value().mean_translational_magnitude;
                // All source scenario outputs are N×6; no Writer/RunId here.
                FlowField observations=std::move(ideal).value().flow;
                double ratio=0;
                if (mode==Mode::Comparison) {
                    if (!(denominator>0.0)||!std::isfinite(numerator)||
                        !std::isfinite(denominator))
                        return fail(ExperimentErrorCode::SolverFailed,
                                    "Comparison ideal component ratio undefined");
                    ratio=numerator/denominator;
                    if (!std::isfinite(ratio))
                        return fail(ExperimentErrorCode::SolverFailed,
                                    "Comparison ideal component ratio overflow");
                    auto noisy=noise_model.apply(observations,noise_scale,*rng);
                    if (!noisy.hasValue())
                        return fromNumeric(noisy,"nonpropfehler",mode,
                                           run+1,matlab_i);
                    observations=std::move(noisy).value();
                    write(current,"rottransratio",row,0,ratio);
                }
                Matrix3 coefficient_rotation=Matrix3::Identity();
                if (mode!=Mode::Initialization) {
                    // Original vordrehmatrix=rodrigues(reldrehvektoren(i-1,:)).
                    // 'i-1' maps to the preceding C++ index row-1.
                    auto prev=trajectory::rotationMatrixFromVector(
                        frames[row-1].relative_rotation);
                    if (!prev.hasValue())
                        return fromNumeric(prev,"previous-frame coefficient rotation",
                                           mode,run+1,matlab_i);
                    // The adapter accepts the explicit harmonic-argument
                    // rotation. Source SHRotate is NOT automatically identical
                    // under a different CS phase convention; R6 checks this.
                    coefficient_rotation=prev.value();
                }
                estimators::AdaptiveFrameInput frame;
                frame.matlab_frame_index=matlab_i;
                frame.coefficient_argument_rotation=coefficient_rotation;
                frame.observations=std::move(observations);
                frame.truth.translation=frames[row].local_translation;
                frame.truth.rotation=frames[row].relative_rotation;

                auto computed=session.processFrame(frame);
                if (!computed.hasValue())
                    return fromSession(computed.error(),mode,run+1,matlab_i);
                const auto& values=computed.value();
                const std::size_t expected=mode==Mode::UpdateRates?4:2;
                if (values.comparison_translation_error_deg.size()!=expected||
                    values.comparison_rotation_error_deg.size()!=expected)
                    return fail(ExperimentErrorCode::DataMismatch,
                                "Adaptive Session returned wrong baseline count");

                if (mode==Mode::UpdateRates) {
                    const double angle_degrees=360.0*frames[row].relative_rotation.norm()/
                        (2.0*std::acos(-1.0));
                    if (!std::isfinite(angle_degrees))
                        return fail(ExperimentErrorCode::SolverFailed,
                                    "UpdateRates akrot is non-finite");
                    write(current,"akrot",row,0,angle_degrees);
                    for (std::size_t k=0;k<expected;++k) {
                        write(current,"allemittransfehler",row,k,
                              values.comparison_translation_error_deg[k]);
                        write(current,"allemitrotfehler",row,k,
                              values.comparison_rotation_error_deg[k]);
                    }
                } else {
                    write(current,"alleohnerotfehler",row,0,
                          values.comparison_rotation_error_deg[0]);
                    write(current,"alleohnetransfehler",row,0,
                          values.comparison_translation_error_deg[0]);
                    write(current,"allemitrotfehler",row,0,
                          values.comparison_rotation_error_deg[1]);
                    write(current,"allemittransfehler",row,0,
                          values.comparison_translation_error_deg[1]);
                }
                if(progress)progress(AdaptiveProgressEvent::FrameCompleted);
            }
            // A complete run contributes atomically to the aggregate.
            // Only the final planned run may supply rottransratio.
            accumulate(aggregate,current,run+1==aggregate.planned_runs);
            ++aggregate.completed_runs;
            if(progress)progress(AdaptiveProgressEvent::RunCompleted);
        }
        if (mode==Mode::Comparison) {
            if (aggregate.completed_runs!=kComparisonRuns)
                return fail(ExperimentErrorCode::IncompleteRun,
                            "Comparison cannot divide partial eight-run results");
            divideComparisonErrors(aggregate);
        }
        return Output::success(std::move(aggregate));
    } catch (const std::bad_alloc&) {
        return fail(ExperimentErrorCode::SolverFailed,
                    "Adaptive memory-only Runner allocation failure");
    } catch (const std::length_error&) {
        return fail(ExperimentErrorCode::SolverFailed,
                    "Adaptive memory-only Runner container exceeded maximum size");
    } catch (const std::logic_error& error) {
        return fail(ExperimentErrorCode::DataMismatch,
                    std::string("Adaptive memory-only output contract: ")+error.what());
    }
}
}  // namespace

Result<AdaptiveRunData,ExperimentErrorCode>
AdaptiveExperimentRunner::computeInitialization(
    const AdaptiveInitializationConfig&,const AdaptiveProgress& progress) const {
    return compute(Mode::Initialization,0.0,nullptr,progress,injected_solver_);
}
Result<AdaptiveRunData,ExperimentErrorCode>
AdaptiveExperimentRunner::computeComparison(
    const AdaptiveComparisonConfig& config,IRandomSource& rng,
    const AdaptiveProgress& progress) const {
    return compute(Mode::Comparison,config.noise_scale,&rng,progress,injected_solver_);
}
Result<AdaptiveRunData,ExperimentErrorCode>
AdaptiveExperimentRunner::computeUpdateRates(
    const AdaptiveUpdateRatesConfig&,const AdaptiveProgress& progress) const {
    return compute(Mode::UpdateRates,0.0,nullptr,progress,injected_solver_);
}

}  // namespace egomotion::experiments
