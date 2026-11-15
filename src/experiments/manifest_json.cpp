#include "egomotion/experiments/manifest_json.hpp"
#include "egomotion/contracts/diagnostics.hpp"

#include <algorithm>
#include <cstddef>
#include <map>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>

namespace egomotion::experiments {
namespace {
template<class T>
Result<T,ExperimentErrorCode> fail(std::string why) {
    ErrorDetail detail;
    detail.stage=ErrorStage::Output;
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
} // namespace

Result<std::string,ExperimentErrorCode> makeCompleteManifestJson(
    const RunContext& run,const std::vector<FileEvidence>& published) {
    const auto& draft=run.manifestDraft();
    const auto planned=currentSaveFields(run.experimentKind());
    // A build-local SOURCE_COMMIT macro, NOT the later Git HEAD in a
    // working tree, provides the executable's actual provenance.
    if(run.provenance().repository_commit_sha.size()!=40||
       !validSha256(run.provenance().repository_commit_sha+
                    run.provenance().repository_commit_sha.substr(0,24)))
        return fail<std::string>("Build source commit identity is not a verified 40-hex SHA");
    if(planned.empty()||published.size()!=planned.size()||
       draft.files_writer_confirmed.size()!=planned.size()||
       draft.expected_repetitions==0||
       draft.completed!=draft.expected_repetitions||
       draft.attempted!=draft.expected_repetitions||
       draft.failed!=0||draft.undefined!=0||!draft.errors.empty())
        return fail<std::string>("Incomplete run cannot be serialized as Complete");
    try{
        std::map<std::string,FileEvidence> by_name;
        for(const auto& file:published)
            if(!by_name.emplace(file.variable,file).second)
                return fail<std::string>("Duplicate evidence for MAT output variable");
        for(const auto& name:planned){
            if(std::find(draft.files_writer_confirmed.begin(),
                         draft.files_writer_confirmed.end(),name)==
               draft.files_writer_confirmed.end())
                return fail<std::string>("Writer did not confirm every expected save name");
            const auto found=by_name.find(name);
            if(found==by_name.end()||
               found->second.filename!=name+".mat"||
               !validSha256(found->second.sha256)||
               !found->second.durably_confirmed||
               found->second.size_bytes==0)
                return fail<std::string>("Missing or unconfirmed MAT file evidence");
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
        return fail<std::string>("Manifest JSON allocation failed");
    }catch(const std::length_error&){
        return fail<std::string>("Manifest JSON exceeds container limits");
    }
}

}  // namespace egomotion::experiments
