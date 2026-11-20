#include "egomotion/analysis/dataset_json.hpp"
#include "egomotion/contracts/diagnostics.hpp"
#include "series_validity.hpp"

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
void numbers(std::ostringstream& stream,const std::vector<double>& a,
              const std::vector<unsigned char>& defined={}) {
    detail::validateDefinedValues(a,defined,"Numeric Analyzer dataset");
    stream<<"[";
    for(std::size_t j=0;j<a.size();++j){
        if(j)stream<<",";
        if(detail::elementDefined(defined,j))stream<<a[j];
        else stream<<"null";
    }
    stream<<"]";
}
void flags(std::ostringstream& stream,const std::vector<unsigned char>& mask,
            std::size_t size,bool default_value) {
    stream<<"[";
    for(std::size_t i=0;i<size;++i) {
        if(i)stream<<",";
        stream<<((mask.empty()?default_value:mask[i]!=0)?"true":"false");
    }
    stream<<"]";
}
} // namespace

Result<std::string,AnalysisErrorCode> datasetJson(const AnalysisDataset& data) {
    try {
        std::ostringstream json;
        json.imbue(std::locale::classic());
        json<<std::setprecision(std::numeric_limits<double>::max_digits10);
        const bool revised=data.metric_policy=="adaptive-atan2-so3-geodesic-v1";
        json<<"{\"schema\":"<<jsonQuote(revised?"egomotion-analysis-v2":"egomotion-analysis-v1")
            <<",\"source_analysis_blob\":"<<jsonQuote(data.source_analysis_blob)
            <<",\"metric_policy\":"<<jsonQuote(data.metric_policy)
            <<",\"planned_repetitions\":"<<data.planned_repetitions<<",\"curves\":[";
        bool first_curve=true;
        for(const auto& c:data.curves){
            detail::validateSeriesValidity(c);
            if(!first_curve)json<<",";
            first_curve=false;
            json<<"{\"id\":"<<jsonQuote(c.name)<<",\"panel\":"<<jsonQuote(c.panel)
                <<",\"x_unit\":"<<jsonQuote(c.x_unit)
                <<",\"y_unit\":"<<jsonQuote(c.y_unit)
                <<",\"x_scale\":"<<jsonQuote(c.x_scale)
                <<",\"y_scale\":"<<jsonQuote(c.y_scale)
                <<",\"metric_semantics\":"<<jsonQuote(c.metric_semantics)
                <<",\"source_begin_matlab\":"<<c.source_begin_matlab
                <<",\"source_end_matlab\":"<<c.source_end_matlab
                <<",\"smoothing_half_window\":"<<c.smoothing_half_window
                <<",\"x\":";
            numbers(json,c.x);
            json<<",\"y\":";
            numbers(json,c.y,c.defined);
            json<<",\"defined\":";
            flags(json,c.defined,c.y.size(),true);
            json<<",\"structural_endpoint\":";
            flags(json,c.structural_endpoint,c.y.size(),false);
            json<<",\"smoothing_full\":";
            numbers(json,c.smoothing_full,c.smoothing_defined);
            json<<",\"smoothing_defined\":";
            flags(json,c.smoothing_defined,c.smoothing_full.size(),true);
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
