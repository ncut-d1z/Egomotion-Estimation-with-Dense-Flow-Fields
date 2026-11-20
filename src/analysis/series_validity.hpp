#pragma once

#include "egomotion/analysis/analyzer.hpp"

#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace egomotion::analysis::detail {

inline bool elementDefined(const std::vector<unsigned char>& defined,std::size_t i) {
    return defined.empty()||defined[i]==1;
}
inline void validateDefinedValues(const std::vector<double>& values,
                                  const std::vector<unsigned char>& defined,
                                  const char* label) {
    if(!defined.empty()&&defined.size()!=values.size())
        throw std::invalid_argument(std::string(label)+" defined mask length differs");
    for(std::size_t i=0;i<values.size();++i) {
        if(!defined.empty()&&defined[i]>1)
            throw std::invalid_argument(std::string(label)+" defined mask is not binary");
        if(elementDefined(defined,i)?!std::isfinite(values[i]):!std::isnan(values[i]))
            throw std::invalid_argument(std::string(label)+
                " requires finite defined values and explicitly declared NaN gaps");
    }
}
inline void validateSeriesValidity(const AnalysisSeries& curve) {
    if(curve.x.size()!=curve.y.size())
        throw std::invalid_argument("Analysis curve x/y lengths differ");
    validateDefinedValues(curve.x,{},"Analysis x");
    validateDefinedValues(curve.y,curve.defined,"Analysis y");
    validateDefinedValues(curve.smoothing_full,curve.smoothing_defined,"Analysis smoothing");
    if(!curve.structural_endpoint.empty()) {
        if(curve.structural_endpoint.size()!=curve.y.size())
            throw std::invalid_argument("Structural endpoint mask length differs");
        for(std::size_t i=0;i<curve.y.size();++i)
            if(curve.structural_endpoint[i]>1||
               (curve.structural_endpoint[i]&&
                (!elementDefined(curve.defined,i)||curve.y[i]!=0.0)))
                throw std::invalid_argument("Structural endpoints must be defined source zero values");
    }
}

} // namespace egomotion::analysis::detail
