#include "egomotion/experiments/manifest_json.hpp"
#include "egomotion/contracts/diagnostics.hpp"
#include "egomotion/experiments/output_values.hpp"
#include "../run_diagnostics.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <limits>
#include <locale>
#include <map>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace egomotion::experiments {
namespace {
template<class T>
Result<T,ExperimentErrorCode> fail(const RunContext& run,std::string why) {
    ErrorDetail detail;
    detail.stage=ErrorStage::Output;
    detail.run_id=run.runId();
    detail.message=std::move(why);
    return Result<T,ExperimentErrorCode>::failure(
        {ExperimentErrorCode::OutputFailed,std::move(detail)});
}
std::string kindName(ExperimentKind kind) {
    switch(kind){
    case ExperimentKind::BiasOccluded:return "bias-occluded";
    case ExperimentKind::BiasProportional:return "bias-proportional";
    case ExperimentKind::AdaptiveInitialization:return "adaptive-init";
    case ExperimentKind::AdaptiveComparison:return "adaptive-comparison";
    case ExperimentKind::AdaptiveUpdateRates:return "adaptive-update-rates";
    }
    return "unknown";
}
bool validSha256(const std::string& sha) {
    if(sha.size()!=64)return false;
    for(char c:sha)if(!((c>='0'&&c<='9')||(c>='a'&&c<='f')))return false;
    return true;
}
bool adaptiveKind(ExperimentKind kind) {
    return kind==ExperimentKind::AdaptiveInitialization||
           kind==ExperimentKind::AdaptiveComparison||
           kind==ExperimentKind::AdaptiveUpdateRates;
}
bool validBuildSha(const std::string& sha) {
    return sha.size()==40&&std::all_of(sha.begin(),sha.end(),[](char c){
        return (c>='0'&&c<='9')||(c>='a'&&c<='f');
    });
}
std::string adaptivePolicyJson(std::size_t repetitions) {
    return "{\"version\":1,\"decisions\":[\"DEC-R6-01\",\"DEC-R6-02\"],"
        "\"translation\":{\"metric\":\"direction_angle\","
        "\"formula\":\"atan2(norm(cross(unit_truth,unit_estimate)),dot(unit_truth,unit_estimate))\","
        "\"normalization\":\"maximum_component_scaling\",\"output_unit\":\"degrees\","
        "\"zero_vector\":\"undefined\"},"
        "\"rotation\":{\"metric\":\"SO3_geodesic_angle\","
        "\"relative_rotation\":\"Exp(truth_rotvec)^T*Exp(estimated_rotvec)\","
        "\"angle\":\"atan2(norm(vee(R-R^T))/2,(trace(R)-1)/2),R=relative_rotation\","
        "\"input\":\"per-frame rotation vectors in the same coordinate frame\","
        "\"input_unit\":\"radians\",\"output_unit\":\"degrees\","
        "\"range_degrees\":[0,180],\"zero_vector\":\"identity_rotation\"},"
        "\"aggregation\":\"fixed_planned_repetitions_require_all_values\","
        "\"aggregation_denominator\":"+std::to_string(repetitions)+
        ",\"rottransratio\":\"last_repetition_only_not_averaged\","
        "\"missing_values\":{\"mat\":\"NaN_with_explicit_mask\",\"json\":\"null\"},"
        "\"validity_arrays\":\"valid_<source_field>,same_shape,valid_repetition_count\","
        "\"structural_endpoints\":\"source_zero,valid_count_zero,defined_mask_one\","
        "\"source_equivalence\":\"metric_semantics_changed_from_MATLAB_normrelfehler\","
        "\"policy_mat_variable\":\"egomotion_metric_policy\","
        "\"policy_mat_layout\":[\"schema_version\",\"direction_atan2\",\"rotation_SO3\",\"planned_repetitions\"],"
        "\"policy_mat_values\":[1,1,1,"+std::to_string(repetitions)+"]}";
}
void appendAdaptiveMetadata(std::string& json,const RunContext& run) {
    json+=",\"computation_complete\":";
    json+=run.adaptiveComputationComplete()?"true":"false";
    json+=",\"statistics_complete\":";
    json+=run.adaptiveStatisticsComplete()?"true":"false";
    json+=",\"aggregation_denominator\":"+
        std::to_string(run.manifestDraft().expected_repetitions);
    json+=",\"metric_policy\":"+adaptivePolicyJson(run.manifestDraft().expected_repetitions);
    json+=",\"metric_validity\":";
    json+=run.adaptiveMetricValidityJson().empty()?"null":run.adaptiveMetricValidityJson();
}
bool appendValues(std::ostream& json,const std::vector<double>& values) {
    json<<'[';
    for(std::size_t i=0;i<values.size();++i) {
        if(i)json<<',';
        if(std::isfinite(values[i]))json<<values[i];
        else if(std::isnan(values[i]))json<<"null";
        else return false;
    }
    json<<']';
    return true;
}
void appendDimensions(std::ostream& json,const DenseDoubleArray& value) {
    json<<"\"dimensions\":["<<value.info.dimensions[0]<<','<<value.info.dimensions[1]<<']';
}
} // namespace

Result<std::string,ExperimentErrorCode> makeCompleteManifestJson(
    const RunContext& run,const std::vector<FileEvidence>& published) {
    const auto& draft=run.manifestDraft();
    const auto planned=run.expectedOutputNames().empty()
        ?currentSaveFields(run.experimentKind()):run.expectedOutputNames();
    // A build-local SOURCE_COMMIT macro, NOT the later Git HEAD in a
    // working tree, provides the executable's actual provenance.
    const auto& build_commit=run.provenance().repository_commit_sha;
    if(build_commit.size()!=40||
       !std::all_of(build_commit.begin(),build_commit.end(),[](char c){
           return (c>='0'&&c<='9')||(c>='a'&&c<='f');
       }))
        return fail<std::string>(run,"Build source commit identity is not a verified 40-hex SHA");
    if(!run.adaptiveMetricValidityJson().empty())
        return fail<std::string>(run,"Adaptive metric evidence requires Manifest v2");
    if(planned.empty()||published.size()!=planned.size()||
       draft.files_writer_confirmed.size()!=planned.size()||
       draft.expected_repetitions==0||
       draft.completed!=draft.expected_repetitions||
       draft.attempted!=draft.expected_repetitions||
       draft.valid==0||draft.failed!=0||draft.undefined!=0||!draft.errors.empty())
        return fail<std::string>(run,"Incomplete run cannot be serialized as Complete");
    try{
        std::map<std::string,FileEvidence> by_name;
        for(const auto& file:published)
            if(!by_name.emplace(file.variable,file).second)
                return fail<std::string>(run,"Duplicate evidence for MAT output variable");
        for(const auto& name:planned){
            if(std::find(draft.files_writer_confirmed.begin(),
                         draft.files_writer_confirmed.end(),name)==
               draft.files_writer_confirmed.end())
                return fail<std::string>(run,"Writer did not confirm every expected save name");
            const auto found=by_name.find(name);
            if(found==by_name.end()||
               found->second.filename!=name+".mat"||
               !validSha256(found->second.sha256)||
               !found->second.durably_confirmed||
               found->second.size_bytes==0)
                return fail<std::string>(run,"Missing or unconfirmed MAT file evidence");
        }
        std::string json="{\"schema\":\"egomotion-run-manifest-v1\"";
        json+=",\"run_id\":"+jsonQuote(run.runId().text);
        json+=",\"experiment_kind\":"+jsonQuote(kindName(run.experimentKind()));
        json+=",\"completeness\":\"Complete\"";
        json+=",\"repo_sha\":"+jsonQuote(run.provenance().repository_commit_sha);
        json+=",\"source_blob_sha\":"+jsonQuote(run.provenance().matlab_source_blob_sha);
        json+=",\"algorithm_rng_seed\":";
        if(run.provenance().rng_seed)
            json+=std::to_string(*run.provenance().rng_seed);
        else json+="null";
        json+=",\"expected_repetitions\":"+std::to_string(draft.expected_repetitions);
        json+=",\"attempted\":"+std::to_string(draft.attempted);
        json+=",\"completed\":"+std::to_string(draft.completed);
        json+=",\"valid\":"+std::to_string(draft.valid);
        json+=",\"failed\":"+std::to_string(draft.failed);
        json+=",\"undefined\":"+std::to_string(draft.undefined);
        json+=",\"errors\":[],\"mat_files\":[";
        bool first=true;
        for(const auto& name:planned){
            if(!first)json+=",";
            first=false;
            const auto& f=by_name.at(name);
            json+="{\"variable\":"+jsonQuote(f.variable);
            json+=",\"file\":"+jsonQuote(f.filename);
            json+=",\"bytes\":"+std::to_string(f.size_bytes);
            json+=",\"sha256\":"+jsonQuote(f.sha256)+"}";
        }
        json+="]}\n";
        return Result<std::string,ExperimentErrorCode>::success(std::move(json));
    }catch(const std::bad_alloc&){
        return fail<std::string>(run,"Manifest JSON allocation failed");
    }catch(const std::length_error&){
        return fail<std::string>(run,"Manifest JSON exceeds container limits");
    }
}

Result<std::string,ExperimentErrorCode> makeAdaptiveMetricValidityJson(
    const RunContext& run,const AdaptiveRunData& source) {
    if(!adaptiveKind(run.experimentKind())||
       source.planned_runs!=run.manifestDraft().expected_repetitions)
        return fail<std::string>(run,"Adaptive metric evidence does not match this run plan");
    auto validated=makeAdaptiveOutput(run.experimentKind(),source);
    if(!validated.hasValue())
        return Result<std::string,ExperimentErrorCode>::failure(
            egomotion::detail::withRunIdIfMissing(std::move(validated).error(),run.runId()));
    try {
        const auto names=currentSaveFields(run.experimentKind());
        std::ostringstream json;
        json.imbue(std::locale::classic());
        json<<std::setprecision(std::numeric_limits<double>::max_digits10);
        json<<"{\"layout\":\"column-major\",\"indexing\":\"MATLAB_1_based\","
            <<"\"frame_count\":"<<source.frame_count
            <<",\"planned_runs\":"<<source.planned_runs
            <<",\"completed_runs\":"<<source.completed_runs
            <<",\"valid_frames\":"<<source.valid_frames
            <<",\"undefined_frames\":"<<source.undefined_frames<<",\"fields\":{";
        bool first=true;
        for(const auto& name:names) {
            if(!first)json<<',';
            first=false;
            const auto& value=source.fields.at(name);
            json<<jsonQuote(name)<<":{";
            appendDimensions(json,value);
            json<<",\"valid_counts\":[";
            const auto& counts=source.valid_counts.at(name);
            for(std::size_t i=0;i<counts.size();++i) {
                if(i)json<<',';
                json<<counts[i];
            }
            json<<"],\"values_column_major\":";
            if(!appendValues(json,value.values_column_major))
                return fail<std::string>(run,"Infinite Adaptive value cannot be serialized as missing");
            json<<'}';
        }
        json<<"},\"repetitions\":[";
        for(std::size_t repetition=0;repetition<source.repetition_fields.size();++repetition) {
            if(repetition)json<<',';
            json<<"{\"matlab_run_index\":"<<repetition+1<<",\"fields\":{";
            first=true;
            for(const auto& name:names) {
                if(!first)json<<',';
                first=false;
                const auto& value=source.repetition_fields[repetition].at(name);
                json<<jsonQuote(name)<<":{";
                appendDimensions(json,value);
                json<<",\"values_column_major\":";
                if(!appendValues(json,value.values_column_major))
                    return fail<std::string>(run,"Infinite Adaptive repetition value is invalid");
                json<<'}';
            }
            json<<"}}";
        }
        json<<"],\"undefined_metrics\":[";
        first=true;
        for(const auto& missing:source.undefined_metrics) {
            if(!first)json<<',';
            first=false;
            const auto reason=egomotion::detail::withRunIdIfMissing(missing.reason,run.runId());
            json<<"{\"variable\":"<<jsonQuote(missing.variable)
                <<",\"matlab_run_index\":"<<missing.matlab_run_index
                <<",\"matlab_frame_index\":"<<missing.matlab_frame_index
                <<",\"matlab_column_index\":"<<missing.matlab_column_index
                <<",\"reason\":"<<jsonFailure(reason,5)<<'}';
        }
        json<<"]}";
        if(!json.good())return fail<std::string>(run,"Cannot format complete Adaptive metric evidence");
        return Result<std::string,ExperimentErrorCode>::success(json.str());
    } catch(const std::bad_alloc&) {
        return fail<std::string>(run,"Cannot allocate Adaptive metric evidence JSON");
    } catch(const std::length_error&) {
        return fail<std::string>(run,"Adaptive metric evidence JSON exceeds container limits");
    } catch(const std::out_of_range&) {
        return fail<std::string>(run,"Adaptive metric evidence has inconsistent field names");
    }
}

Result<std::string,ExperimentErrorCode> makeAdaptiveManifestJson(
    const RunContext& run,const std::vector<FileEvidence>& published) {
    const auto& draft=run.manifestDraft();
    if(!adaptiveKind(run.experimentKind())||!run.adaptiveComputationComplete()||
       draft.failed!=0||!draft.errors.empty())
        return fail<std::string>(run,"Adaptive publication requires every solver repetition and unchanged metric evidence");
    if(!validBuildSha(run.provenance().repository_commit_sha))
        return fail<std::string>(run,"Build source commit identity is not a verified 40-hex SHA");
    try {
        std::vector<std::string> planned{"egomotion_metric_policy"};
        for(const auto& name:currentSaveFields(run.experimentKind())) {
            planned.push_back("valid_"+name);planned.push_back(name);
        }
        if(planned!=run.expectedOutputNames()||published.size()!=planned.size()||
           draft.files_writer_confirmed!=planned)
            return fail<std::string>(run,"Adaptive policy, validity and source outputs lack the complete expected receipts");
        std::map<std::string,FileEvidence> by_name;
        for(const auto& file:published)
            if(!by_name.emplace(file.variable,file).second)
                return fail<std::string>(run,"Duplicate evidence for Adaptive MAT output variable");
        for(const auto& name:planned) {
            const auto found=by_name.find(name);
            if(found==by_name.end()||found->second.filename!=name+".mat"||
               !validSha256(found->second.sha256)||!found->second.durably_confirmed||
               found->second.size_bytes==0)
                return fail<std::string>(run,"Adaptive MAT output evidence is missing or unconfirmed");
        }
        std::string json="{\"schema\":\"egomotion-run-manifest-v2\"";
        json+=",\"run_id\":"+jsonQuote(run.runId().text);
        json+=",\"experiment_kind\":"+jsonQuote(kindName(run.experimentKind()));
        json+=",\"completeness\":"+jsonQuote(run.adaptiveStatisticsComplete()?"Complete":"Incomplete");
        json+=",\"repo_sha\":"+jsonQuote(run.provenance().repository_commit_sha);
        json+=",\"source_blob_sha\":"+jsonQuote(run.provenance().matlab_source_blob_sha);
        json+=",\"algorithm_rng_seed\":";
        json+=run.provenance().rng_seed?std::to_string(*run.provenance().rng_seed):"null";
        json+=",\"expected_repetitions\":"+std::to_string(draft.expected_repetitions);
        json+=",\"attempted\":"+std::to_string(draft.attempted);
        json+=",\"completed\":"+std::to_string(draft.completed);
        json+=",\"valid\":"+std::to_string(draft.valid);
        json+=",\"failed\":"+std::to_string(draft.failed);
        json+=",\"undefined\":"+std::to_string(draft.undefined);
        appendAdaptiveMetadata(json,run);
        json+=",\"errors\":[],\"mat_files\":[";
        bool first=true;
        for(const auto& name:planned) {
            if(!first)json+=",";
            first=false;
            const auto& file=by_name.at(name);
            json+="{\"variable\":"+jsonQuote(file.variable);
            json+=",\"file\":"+jsonQuote(file.filename);
            json+=",\"bytes\":"+std::to_string(file.size_bytes);
            json+=",\"sha256\":"+jsonQuote(file.sha256)+",\"durably_confirmed\":true}";
        }
        json+="]}\n";
        return Result<std::string,ExperimentErrorCode>::success(std::move(json));
    } catch(const std::bad_alloc&) {
        return fail<std::string>(run,"Adaptive Manifest JSON allocation failed");
    } catch(const std::length_error&) {
        return fail<std::string>(run,"Adaptive Manifest JSON exceeds container limits");
    }
}

Result<std::string,ExperimentErrorCode> makeFailureManifestJson(
    const RunContext& run,const std::vector<FileEvidence>& published) {
    const auto& d=run.manifestDraft();
    if(d.errors.empty())
        return fail<std::string>(run,"Cannot publish failure Manifest without error event");
    try{
        const bool adaptive=adaptiveKind(run.experimentKind());
        const bool has_partial=d.completed>0||d.valid>0||!published.empty()||
            (adaptive&&(d.undefined>0||!run.adaptiveMetricValidityJson().empty()));
        std::string json=adaptive?"{\"schema\":\"egomotion-run-manifest-v2\"":
                                  "{\"schema\":\"egomotion-run-manifest-v1\"";
        json+=",\"run_id\":"+jsonQuote(run.runId().text);
        json+=",\"experiment_kind\":"+jsonQuote(kindName(run.experimentKind()));
        json+=",\"completeness\":";
        json+=jsonQuote(has_partial?"Incomplete":"Failed");
        json+=",\"repo_sha\":"+jsonQuote(run.provenance().repository_commit_sha);
        json+=",\"source_blob_sha\":"+jsonQuote(run.provenance().matlab_source_blob_sha);
        json+=",\"algorithm_rng_seed\":";
        if(run.provenance().rng_seed)
            json+=std::to_string(*run.provenance().rng_seed);
        else json+="null";
        json+=",\"expected_repetitions\":"+std::to_string(d.expected_repetitions);
        json+=",\"attempted\":"+std::to_string(d.attempted);
        json+=",\"completed\":"+std::to_string(d.completed);
        json+=",\"valid\":"+std::to_string(d.valid);
        json+=",\"failed\":"+std::to_string(d.failed);
        json+=",\"undefined\":"+std::to_string(d.undefined);
        if(adaptive)appendAdaptiveMetadata(json,run);
        json+=",\"errors\":[";
        bool first=true;
        for(const auto& err:d.errors){
            if(!first)json+=",";
            first=false;
            json+=jsonFailure(err,err.detail.stage==ErrorStage::Output?6:5);
        }
        json+="],\"mat_files\":[";
        first=true;
        for(const auto& f:published){
            if(!first)json+=",";
            first=false;
            json+="{\"variable\":"+jsonQuote(f.variable);
            json+=",\"file\":"+jsonQuote(f.filename);
            json+=",\"sha256\":";
            if(validSha256(f.sha256))json+=jsonQuote(f.sha256);
            else json+="null";
            json+=",\"bytes\":"+std::to_string(f.size_bytes);
            json+=",\"durably_confirmed\":";
            json+=f.durably_confirmed?"true":"false";
            json+="}";
        }
        json+="]}\n";
        return Result<std::string,ExperimentErrorCode>::success(std::move(json));
    }catch(const std::bad_alloc&){
        return fail<std::string>(run,"Cannot allocate failure Manifest JSON");
    }catch(const std::length_error&){
        return fail<std::string>(run,"Failure Manifest JSON exceeds container limits");
    }
}

}  // namespace egomotion::experiments
