#pragma once

// 测试夹具：构造有明确适用帧/逐轮身份的内存结果，不调用生产聚合或输出代码。
#include "egomotion/experiments/adaptive_runner.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace egomotion::test_support {
inline experiments::AdaptiveRunData completeAdaptiveData(
    std::map<std::string,DenseDoubleArray> fields,std::size_t frames,std::size_t runs=1) {
    experiments::AdaptiveRunData data;
    data.frame_count=frames;data.planned_runs=runs;data.completed_runs=runs;
    data.valid_frames=(frames-2)*runs;
    for(auto& item:fields) {
        auto& values=item.second.values_column_major;
        auto& counts=data.valid_counts[item.first];
        counts.assign(values.size(),item.first=="rottransratio"?1:runs);
        for(std::size_t offset=0;offset<values.size();offset+=frames) {
            values[offset]=0;values[offset+frames-1]=0;
            counts[offset]=0;counts[offset+frames-1]=0;
        }
    }
    data.fields=std::move(fields);
    data.repetition_fields.assign(runs,data.fields);
    return data;
}
} // namespace egomotion::test_support
