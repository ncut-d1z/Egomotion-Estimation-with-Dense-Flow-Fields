#include "egomotion/experiments/output_contract.hpp"

#include <cmath>
#include <cstddef>
#include <limits>
#include <string>
#include <utility>

namespace egomotion::experiments {
namespace {
Status<ExperimentErrorCode> fail(std::string message, std::string field = {}) {
    ErrorDetail detail;
    detail.stage=ErrorStage::Output;
    detail.message=std::move(message);
    detail.variable=std::move(field);
    return Status<ExperimentErrorCode>::failure(
        {ExperimentErrorCode::DataMismatch,std::move(detail)});
}
bool safeName(const std::string& name) {
    if (name.empty()) return false;
    for (unsigned char c: name) {
        if (!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||
              (c>='0'&&c<='9')||c=='_')) return false;
    }
    return true;
}
bool validShape(ExperimentKind kind, const std::string& name,
                const DenseDoubleArray& value) {
    if (value.info.dimensions.size()!=2)return false;
    const std::size_t rows=value.info.dimensions[0],cols=value.info.dimensions[1];
    if (rows==0||cols==0)return false;
    if (kind==ExperimentKind::AdaptiveInitialization && rows!=35)return false;
    if ((kind==ExperimentKind::AdaptiveComparison ||
         kind==ExperimentKind::AdaptiveUpdateRates) && rows!=600)return false;
    if (kind==ExperimentKind::BiasOccluded ||
        kind==ExperimentKind::BiasProportional)
        return cols==(name=="xwerte"?1:3);
    if (kind==ExperimentKind::AdaptiveUpdateRates)
        return cols==(name=="akrot"?1:4);
    return cols==1;
}
}  // namespace

std::vector<std::string> currentSaveFields(ExperimentKind kind) {
    switch (kind) {
    case ExperimentKind::BiasOccluded:
    case ExperimentKind::BiasProportional:
        return {"xwerte","ywerte1","ywerte2","ywerte3",
                "ywerte4","ywerte5","ywerte6"};
    case ExperimentKind::AdaptiveInitialization:
        return {"allemittransfehler","allemitrotfehler",
                "alleohnetransfehler","alleohnerotfehler"};
    case ExperimentKind::AdaptiveComparison:
        return {"allemittransfehler","allemitrotfehler",
                "alleohnetransfehler","alleohnerotfehler","rottransratio"};
    case ExperimentKind::AdaptiveUpdateRates:
        return {"allemittransfehler","allemitrotfehler","akrot"};
    }
    return {};
}

namespace {
Status<ExperimentErrorCode> validateOutput(
    ExperimentKind kind, const std::map<std::string,DenseDoubleArray>& fields,
    const std::map<std::string,std::vector<unsigned char>>* defined) {
    const auto planned=currentSaveFields(kind);
    if (planned.empty() || fields.size()!=planned.size())
        return fail("Output batch does not have exactly the current save set");
    if(defined&&defined->size()!=fields.size())
        return fail("Adaptive output masks do not cover exactly the source fields");
    std::size_t bias_rows=0;
    for (const auto& name:planned) {
        auto found=fields.find(name);
        if (found==fields.end())
            return fail("Required current save field is missing",name);
        const auto& array=found->second;
        if (!safeName(name)||array.info.name!=name)
            return fail("Output name and internal MAT variable identity differ",name);
        if (!validShape(kind,name,array))
            return fail("Current save field has wrong source dimensions",name);
        const auto rows=array.info.dimensions[0];
        const auto cols=array.info.dimensions[1];
        if (kind==ExperimentKind::BiasOccluded ||
            kind==ExperimentKind::BiasProportional) {
            if (bias_rows==0)bias_rows=rows;
            if (bias_rows!=rows)
                return fail("Bias current-save rows are inconsistent",name);
        }
        if (cols>std::numeric_limits<std::size_t>::max()/rows ||
            rows*cols>std::numeric_limits<std::size_t>::max()/sizeof(double))
            return fail("Output matrix dimensions exceed addressable bytes",name);
        const std::size_t n=rows*cols;
        if (array.values_column_major.size()!=n ||
            array.info.original_class_type!=6 ||
            array.info.original_data_type!=9 ||
            array.info.is_complex || array.info.is_logical ||
            array.info.data_bytes!=n*sizeof(double))
            return fail("Output data shape, MATLAB double type or bytes are inconsistent",name);
        const std::vector<unsigned char>* mask=nullptr;
        if(defined) {
            const auto found_mask=defined->find(name);
            if(found_mask==defined->end()||found_mask->second.size()!=n)
                return fail("Adaptive validity mask has wrong shape",name);
            mask=&found_mask->second;
        }
        for(std::size_t i=0;i<n;++i) {
            const auto flag=mask?(*mask)[i]:1;
            if((flag==1&&!std::isfinite(array.values_column_major[i]))||
               (flag==0&&!std::isnan(array.values_column_major[i]))||flag>1)
                return fail("Output contains an undeclared or invalid non-finite numeric element",name);
        }
    }
    return Status<ExperimentErrorCode>::success();
}
}  // namespace

Status<ExperimentErrorCode> validateCurrentOutput(
    ExperimentKind kind,const std::map<std::string,DenseDoubleArray>& fields) {
    return validateOutput(kind,fields,nullptr);
}

Status<ExperimentErrorCode> validateMaskedAdaptiveOutput(
    ExperimentKind kind,const std::map<std::string,DenseDoubleArray>& fields,
    const std::map<std::string,std::vector<unsigned char>>& defined) {
    if(kind==ExperimentKind::BiasOccluded||kind==ExperimentKind::BiasProportional)
        return fail("Masked output is supported only for the Adaptive metric policy");
    return validateOutput(kind,fields,&defined);
}
}  // namespace egomotion::experiments
