#pragma once

#include "egomotion/contracts/diagnostics.hpp"

#include <ostream>
#include <string_view>
#include <utility>

namespace egomotion::cli::streams {
inline constexpr int output_failure_exit=6;

// 同时覆盖短写、延迟到flush的失败及调用方开启的ostream异常；不清除失败位。
inline bool writeAndFlush(std::ostream& stream,std::string_view text) noexcept {
    try {
        stream<<text;
        stream.flush();
        return stream.good();
    } catch(...) {
        return false;
    }
}

template<class Code>
bool diagnostic(std::ostream& err,const Failure<Code>& failure,int exit_code) noexcept {
    try {
        return writeAndFlush(err,jsonFailure(failure,exit_code)+"\n");
    } catch(...) {
        // 诊断分配或同一通道再次失败时不能递归输出、抛出或宣称已持久化。
        return false;
    }
}

template<class Code>
int report(std::ostream& err,const Failure<Code>& failure,int exit_code) noexcept {
    return diagnostic(err,failure,exit_code)?exit_code:output_failure_exit;
}

inline int outputFailure(std::ostream& err,std::string_view message,
                         const RunId* run_id=nullptr) noexcept {
    try {
        ErrorDetail detail;detail.stage=ErrorStage::Output;
        detail.message=message;
        if(run_id)detail.run_id=*run_id;
        return report(err,Failure<ExperimentErrorCode>{
            ExperimentErrorCode::OutputFailed,std::move(detail)},output_failure_exit);
    } catch(...) {
        return output_failure_exit;
    }
}

inline int success(std::ostream& out,std::ostream& err,std::string_view text,
                   const RunId* run_id=nullptr) noexcept {
    if(writeAndFlush(out,text))return 0;
    return outputFailure(err,"CLI stdout write or flush failed",run_id);
}
} // namespace egomotion::cli::streams
