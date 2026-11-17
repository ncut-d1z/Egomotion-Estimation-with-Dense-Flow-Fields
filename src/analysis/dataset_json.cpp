#include "egomotion/analysis/dataset_json.hpp"
#include "egomotion/contracts/diagnostics.hpp"

#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace egomotion::analysis {
namespace {
Result<std::string,AnalysisErrorCode> fail(std::string message) {
    ErrorDetail d;d.stage=ErrorStage::Analyze;d.message=std::move(message);
    return Result<std::string,AnalysisErrorCode>::failure({
        AnalysisErrorCode::UndefinedStatistic,std::move(d)});
}
void numbers(std::ostringstream& stream,const std::vector<double>& a) {
    stream<<"[";
    for(std::size_t j=0;j<a.size();++j){
        if(j)stream<<",";
        if(!std::isfinite(a[j]))
            throw std::invalid_argument("Numeric Analyzer dataset has non-finite value");
        stream<<a[j];
    }
    stream<<"]";
}
} // namespace

Result<std::string,AnalysisErrorCode> datasetJson(const AnalysisDataset& data) {
    try {
        std::ostringstream json;
        json.imbue(std::locale::classic());
        json<<std::setprecision(std::numeric_limits<double>::max_digits10);
        json<<"{\"schema\":\"egomotion-analysis-v1\",\"source_analysis_blob\":"
            <<jsonQuote(data.source_analysis_blob)<<",\"curves\":[";
        bool first_curve=true;
        for(const auto& c:data.curves){
            if(c.x.size()!=c.y.size())
                return fail("Analysis curve x/y lengths differ");
            if(!first_curve)json<<",";
            first_curve=false;
            json<<"{\"id\":"<<jsonQuote(c.name)<<",\"panel\":"<<jsonQuote(c.panel)
                <<",\"x_unit\":"<<jsonQuote(c.x_unit)
                <<",\"y_unit\":"<<jsonQuote(c.y_unit)
                <<",\"x_scale\":"<<jsonQuote(c.x_scale)
                <<",\"y_scale\":"<<jsonQuote(c.y_scale)
                <<",\"source_begin_matlab\":"<<c.source_begin_matlab
                <<",\"source_end_matlab\":"<<c.source_end_matlab
                <<",\"smoothing_half_window\":"<<c.smoothing_half_window
                <<",\"x\":";
            numbers(json,c.x);
            json<<",\"y\":";
            numbers(json,c.y);
            json<<",\"smoothing_full\":";
            numbers(json,c.smoothing_full);
            json<<",\"helper_source_blob_shas\":[";
            for(std::size_t i=0;i<c.helper_source_blob_shas.size();++i){
                if(i)json<<",";
                json<<jsonQuote(c.helper_source_blob_shas[i]);
            }
            json<<"]";
            json<<",\"inputs\":[";
            bool first_input=true;
            for(const auto& p:c.inputs){
                if(!first_input)json<<",";
                first_input=false;
                json<<"{\"path\":"<<jsonQuote(p.path.string())
                    <<",\"variable\":"<<jsonQuote(p.variable)
                    <<",\"matlab_run_ordinal\":"<<p.matlab_run_ordinal
                    <<",\"role\":"<<jsonQuote(p.role)
                    <<",\"mat_sha256\":";
                if(p.mat_sha256)json<<jsonQuote(*p.mat_sha256);
                else json<<"null"; // deliberately NOT a fabricated digest
                json<<"}";
            }
            json<<"]}";
        }
        json<<"]}\n";
        return Result<std::string,AnalysisErrorCode>::success(json.str());
    }catch(const std::invalid_argument& e){
        return fail(e.what());
    }catch(const std::bad_alloc&){
        return fail("Cannot allocate numeric analysis JSON");
    }catch(const std::length_error&){
        return fail("Numeric analysis JSON exceeds container capacity");
    }
}
}  // namespace egomotion::analysis
