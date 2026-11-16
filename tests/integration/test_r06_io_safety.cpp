// R6 output-safety negatives. All mutations are limited to a caller-supplied
// CMake binary-tree sandbox; no original archive or user output is touched.
// Native execution requires H-11 approval and is NOT performed by this edit.
#include "egomotion/io/run_directory.hpp"

#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
using namespace egomotion;
namespace fs=std::filesystem;
void require(bool ok,const char* message){
    if(!ok)throw std::runtime_error(message);
}
void expectForbidden(Result<std::unique_ptr<io::RunDirectory>,MatErrorCode>&& x,
                     const char* reason) {
    require(!x.hasValue()&&x.error().code==MatErrorCode::OutputPathForbidden,reason);
}
void linkDirectoryIfAbsent(const fs::path& target,const fs::path& link) {
    std::error_code ec;
    const auto status=fs::symlink_status(link,ec);
    if(ec&&ec!=std::errc::no_such_file_or_directory)
        throw fs::filesystem_error("Cannot inspect test symlink",link,ec);
    if(!fs::exists(status))
        fs::create_directory_symlink(target,link);
    require(fs::is_symlink(fs::symlink_status(link)),
            "Test symlink precondition must not resolve to a regular directory");
}
} // namespace
int main(int argc,char** argv) {
    require(argc==2,"Expected one isolated build-tree output-safety directory");
    const fs::path sandbox=fs::absolute(argv[1]);

    const fs::path safe=sandbox/"safe";
    fs::create_directories(safe/"origin");
    auto run=io::RunDirectory::create(safe);
    require(run.hasValue(),
            "A normal trusted repository must reserve one exclusive RunId");
    const auto id=run.value()->runId();
    auto duplicate=io::RunDirectory::create(safe,id);
    require(!duplicate.hasValue()&&duplicate.error().code==MatErrorCode::OutputExists,
            "Existing RunId must fail without overwriting output");
    expectForbidden(io::RunDirectory::create(safe,RunId{"../origin"}),
                    "Parent path escape must be rejected before RunId allocation");
    expectForbidden(io::RunDirectory::create(safe,RunId{"bad/slash"}),
                    "RunId separators must never be treated as paths");

    const fs::path link_source=sandbox/"symlink_origin";
    fs::create_directories(link_source);
    linkDirectoryIfAbsent(safe/"origin",link_source/"origin");
    expectForbidden(io::RunDirectory::create(link_source),
                    "Symlink origin directory must not satisfy trusted archive precondition");

    const fs::path link_output=sandbox/"symlink_output";
    const fs::path outside=sandbox/"outside";
    fs::create_directories(link_output/"origin");
    fs::create_directories(outside);
    linkDirectoryIfAbsent(outside,link_output/"output");
    expectForbidden(io::RunDirectory::create(link_output),
                    "Symlink output directory must not redirect reserved writes");

    const fs::path root_link=sandbox/"root_link";
    linkDirectoryIfAbsent(safe,root_link);
    expectForbidden(io::RunDirectory::create(root_link),
                    "Symlink repository root must be rejected by O_NOFOLLOW");

    require(!fs::exists(outside/"manifest.json"),
            "Rejected output link must not write a foreign Manifest");
}
