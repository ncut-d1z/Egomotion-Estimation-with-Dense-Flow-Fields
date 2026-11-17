#pragma once

#include "egomotion/contracts/error.hpp"

namespace egomotion::detail {
// 只在已分配 RunId 的边界补充顶层上下文；不改写内层历史或已有身份。
template<class Code>
Failure<Code> withRunIdIfMissing(Failure<Code> failure,const RunId& run_id) {
    if (!failure.detail.run_id) failure.detail.run_id=run_id;
    return failure;
}
}  // namespace egomotion::detail
