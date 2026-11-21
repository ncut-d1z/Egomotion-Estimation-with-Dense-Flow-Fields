// R6 contract coverage: Result/Status owned rvalue access and full wrapper
// chain. No MAT I/O or numerical solver is required for this test.
#include "egomotion/contracts/interfaces.hpp"
#include "egomotion/analysis/analyzer.hpp"
#include "egomotion/contracts/diagnostics.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
using namespace egomotion;

void check(bool value,const char* reason) {
    if(!value)throw std::runtime_error(reason);
}
using ArrayResult=Result<DenseDoubleArray,MatErrorCode>;
using PointerResult=Result<std::unique_ptr<int>,NumericalErrorCode>;
using MatStatus=Status<MatErrorCode>;

static_assert(!std::is_default_constructible<ArrayResult>::value,
              "Result cannot silently represent an unknown state");
static_assert(!std::is_copy_assignable<ArrayResult>::value,
              "Result has no copy assignment contract");
static_assert(!std::is_move_assignable<MatStatus>::value,
              "Status has no move assignment contract");
static_assert(std::is_same<decltype(std::declval<const ArrayResult&>().value()),
                           const DenseDoubleArray&>::value,
              "Lvalue Result borrows only");
static_assert(std::is_same<decltype(std::declval<ArrayResult&&>().value()),
                           DenseDoubleArray>::value,
              "Rvalue Result must extract an owned value, never T&&");
static_assert(std::is_same<decltype(std::declval<ArrayResult&&>().error()),
                           Failure<MatErrorCode>>::value,
              "Rvalue Result error must extract full owned Failure");
static_assert(std::is_same<decltype(std::declval<MatStatus&&>().error()),
                           Failure<MatErrorCode>>::value,
              "Rvalue Status error must own its frames");
static_assert(std::is_same<decltype(std::declval<PointerResult&&>().value()),
                           std::unique_ptr<int>>::value,
              "Move-only Result must extract its pointer safely");

DenseDoubleArray makeArray() {
    DenseDoubleArray a;
    a.info.name="independent";
    a.info.dimensions={2,2};
    a.info.original_class_type=6;
    a.info.original_data_type=9;
    a.info.data_bytes=4*sizeof(double);
    a.values_column_major={1.5,2.5,3.5,4.5};
    return a;
}
void owningSuccess() {
    // The temporary Result is destroyed at the semicolon, the values remain.
    DenseDoubleArray owned=ArrayResult::success(makeArray()).value();
    check(owned.info.name=="independent"&&owned.info.dimensions.size()==2,
          "Extracted owned MAT array metadata was lost");
    check(owned.values_column_major==std::vector<double>({1.5,2.5,3.5,4.5}),
          "Extracted MAT array values must outlive temporary Result");
    auto pointer=PointerResult::success(std::make_unique<int>(42)).value();
    check(pointer&&*pointer==42,
          "Move-only rvalue extraction must not dangle after temporary destruction");
    auto stable=ArrayResult::success(makeArray());
    const auto& borrowed=stable.value();
    check(borrowed.info.name=="independent"&&
          borrowed.values_column_major[2]==3.5,
          "Valid lvalue Result must retain borrowed storage");
}
void failureChains() {
    ErrorDetail bottom;
    bottom.stage=ErrorStage::Read;
    bottom.message="MAT metadata malformed";
    bottom.source_path="original.mat";
    bottom.variable="xwerte";
    bottom.observation_index=7;
    const Failure<MatErrorCode> source{MatErrorCode::InvalidMatHeader,bottom};

    ErrorDetail middle;
    middle.stage=ErrorStage::Decode;
    middle.message="numeric read required by Analyzer";
    auto decoded=wrapFailure(AnalysisErrorCode::ShapeMismatch,
                             std::move(middle),source);
    ErrorDetail outer;
    outer.stage=ErrorStage::Analyze;
    outer.message="cannot compute historical source curves";
    auto wrapped=wrapFailure(AnalysisErrorCode::UndefinedStatistic,
                             std::move(outer),decoded);

    auto result=Result<int,AnalysisErrorCode>::failure(std::move(wrapped));
    auto extracted=std::move(result).error();
    check(extracted.code==AnalysisErrorCode::UndefinedStatistic,
          "Extracted outer error identity was lost");
    auto it=extracted.detail.causes.begin();
    check(it!=extracted.detail.causes.end()&&
          it->domain=="AnalysisErrorCode"&&it->code=="ShapeMismatch",
          "Nearest intermediate error frame is missing");
    ++it;
    check(it!=extracted.detail.causes.end()&&
          it->domain=="MatErrorCode"&&it->code=="InvalidMatHeader"&&
          it->stage==ErrorStage::Read&&
          it->source_path=="original.mat"&&it->variable=="xwerte"&&
          it->observation_index&&*it->observation_index==7,
          "Original MAT failure frame was truncated or rewritten");
    ++it;
    check(it==extracted.detail.causes.end(),
          "Error chain contains an invented wrapper");
    check(source.detail.causes.empty(),
          "wrapFailure must not mutate its original failure");
    const auto json=jsonFailure(extracted,5);
    check(json.find("\"error_domain\":\"AnalysisErrorCode\"")!=
          std::string::npos&&
          json.find("\"error_code\":\"UndefinedStatistic\"")!=std::string::npos&&
          json.find("\"observation_index\":7")!=std::string::npos,
          "JSONL diagnostics dropped required source error identity");

    ErrorDetail status_detail;
    status_detail.stage=ErrorStage::Output;
    status_detail.message="staging write failed";
    auto status=MatStatus::failure({MatErrorCode::WriteFailed,
                                   std::move(status_detail)});
    const auto error=std::move(status).error();
    check(error.code==MatErrorCode::WriteFailed&&
          error.detail.stage==ErrorStage::Output&&
          error.detail.message=="staging write failed",
          "Status rvalue must own the full output failure");
}
} // namespace

int main() {
    owningSuccess();
    failureChains();
}
