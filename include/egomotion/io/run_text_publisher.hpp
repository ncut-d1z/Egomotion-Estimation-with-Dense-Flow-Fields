#pragma once

#include "egomotion/contracts/interfaces.hpp"
#include "egomotion/io/run_directory.hpp"

namespace egomotion::io {

class RunTextPublisher final : public ITextPublisher {
public:
    explicit RunTextPublisher(const RunDirectory& run):run_(run){}
    Status<MatErrorCode> publishNew(
        std::string_view filename,std::string_view contents) override;
private:
    const RunDirectory& run_;
};

}  // namespace egomotion::io
