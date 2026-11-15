#include "egomotion/contracts/diagnostics.hpp"
#include "egomotion/contracts/interfaces.hpp"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

int main() {
    using namespace egomotion;
    using OwnedResult = Result<std::unique_ptr<int>, NumericalErrorCode>;
    using UnitStatus = Status<MatErrorCode>;

    static_assert(!std::is_default_constructible<OwnedResult>::value);
    static_assert(!std::is_default_constructible<UnitStatus>::value);
    static_assert(!std::is_copy_assignable<OwnedResult>::value);
    static_assert(!std::is_move_assignable<UnitStatus>::value);

    auto good = OwnedResult::success(std::make_unique<int>(7));
    assert(good.hasValue());
    const auto& borrowed = good.value();
    assert(*borrowed == 7);
    auto extracted = std::move(good).value();
    assert(*extracted == 7);

    ErrorDetail source;
    source.stage = ErrorStage::Decode;
    source.message = "unexpected dims";
    source.source_path = "two words.mat";
    source.variable = "xwerte";
    source.observation_index = 0;
    Failure<MatErrorCode> original{MatErrorCode::InvalidShape, source};
    ErrorDetail outer;
    outer.stage = ErrorStage::Analyze;
    outer.message = "failed to decode data";
    auto wrapped = wrapFailure(ExperimentErrorCode::DataMismatch, outer, original);
    assert(wrapped.detail.causes.front().domain == "MatErrorCode");
    assert(wrapped.detail.causes.front().code == "InvalidShape");
    assert(wrapped.detail.causes.front().observation_index == 0);
    assert(original.detail.causes.empty());

    ErrorDetail second;
    second.stage = ErrorStage::Solve;
    second.message = "dependent process cannot continue";
    auto twice = wrapFailure(ExperimentErrorCode::IncompleteRun, second, wrapped);
    auto it = twice.detail.causes.begin();
    assert(it->code == "DataMismatch");
    ++it;
    assert(it->code == "InvalidShape");
    ++it;
    assert(it == twice.detail.causes.end());

    const std::string json = jsonFailure(twice, 5);
    assert(json.find("\"error_domain\":\"ExperimentErrorCode\"") != std::string::npos);
    assert(json.find("\"causes\":[") != std::string::npos);
    assert(json.find("\"observation_index\":0") != std::string::npos);
    assert(jsonQuote("x\"\n\\") == "\"x\\\"\\n\\\\\"");

    auto failure = OwnedResult::failure(
        {NumericalErrorCode::EmptyInput, ErrorDetail{}});
    bool threw = false;
    try { (void)failure.value(); }
    catch (const std::logic_error& exception) {
        threw = std::string(exception.what()) == "Result::value(): no success value";
    }
    assert(threw);

    auto status = UnitStatus::success();
    threw = false;
    try { (void)status.error(); }
    catch (const std::logic_error& exception) {
        threw = std::string(exception.what()) == "Status::error(): no failure value";
    }
    assert(threw);
    assert(!UnitStatus::failure(
        {MatErrorCode::MissingFile, ErrorDetail{}}).hasValue());
}
