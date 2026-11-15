#include "egomotion/contracts/diagnostics.hpp"
#include "egomotion/contracts/interfaces.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace {
void require(bool condition) {
    if (!condition) throw std::runtime_error("R01 contract assertion failed");
}
}  // namespace

int main() {
    using namespace egomotion;
    using OwnedResult = Result<std::unique_ptr<int>, NumericalErrorCode>;
    using UnitStatus = Status<MatErrorCode>;

    static_require(!std::is_default_constructible<OwnedResult>::value);
    static_require(!std::is_default_constructible<UnitStatus>::value);
    static_require(!std::is_copy_assignable<OwnedResult>::value);
    static_require(!std::is_move_assignable<UnitStatus>::value);

    auto good = OwnedResult::success(std::make_unique<int>(7));
    require(good.hasValue());
    const auto& borrowed = good.value();
    require(*borrowed == 7);
    auto extracted = std::move(good).value();
    require(*extracted == 7);

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
    require(wrapped.detail.causes.front().domain == "MatErrorCode");
    require(wrapped.detail.causes.front().code == "InvalidShape");
    require(wrapped.detail.causes.front().observation_index == 0);
    require(original.detail.causes.empty());

    ErrorDetail second;
    second.stage = ErrorStage::Solve;
    second.message = "dependent process cannot continue";
    auto twice = wrapFailure(ExperimentErrorCode::IncompleteRun, second, wrapped);
    auto it = twice.detail.causes.begin();
    require(it->code == "DataMismatch");
    ++it;
    require(it->code == "InvalidShape");
    ++it;
    require(it == twice.detail.causes.end());

    const std::string json = jsonFailure(twice, 5);
    require(json.find("\"error_domain\":\"ExperimentErrorCode\"") != std::string::npos);
    require(json.find("\"causes\":[") != std::string::npos);
    require(json.find("\"observation_index\":0") != std::string::npos);
    require(jsonQuote("x\"\n\\") == "\"x\\\"\\n\\\\\"");

    auto failure = OwnedResult::failure(
        {NumericalErrorCode::EmptyInput, ErrorDetail{}});
    bool threw = false;
    try { (void)failure.value(); }
    catch (const std::logic_error& exception) {
        threw = std::string(exception.what()) == "Result::value(): no success value";
    }
    require(threw);

    auto status = UnitStatus::success();
    threw = false;
    try { (void)status.error(); }
    catch (const std::logic_error& exception) {
        threw = std::string(exception.what()) == "Status::error(): no failure value";
    }
    require(threw);
    require(!UnitStatus::failure(
        {MatErrorCode::MissingFile, ErrorDetail{}}).hasValue());
}
