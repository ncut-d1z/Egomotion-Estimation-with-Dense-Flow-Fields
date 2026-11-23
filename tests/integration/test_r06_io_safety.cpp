// R6 output-safety negatives. All mutations are limited to a caller-supplied
// CMake binary-tree sandbox; no original archive or user output is touched.
// Native execution requires H-11 approval and is NOT performed by this edit.
#include "egomotion/io/run_directory.hpp"

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <new>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

namespace allocation_probe {
bool armed=false;
bool triggered=false;
std::size_t remaining=0;
}

// 仅在 create 调用期间逐个注入一次分配失败；故障后立即恢复分配，
// 不让错误对象、目录检查和回归断言消耗同一次故障。
void* operator new(std::size_t size) {
    if(allocation_probe::armed) {
        if(allocation_probe::remaining==0) {
            allocation_probe::armed=false;
            allocation_probe::triggered=true;
            throw std::bad_alloc();
        }
        --allocation_probe::remaining;
    }
    if(void* memory=std::malloc(size?size:1))return memory;
    throw std::bad_alloc();
}
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete[](void* memory) noexcept { std::free(memory); }
void operator delete(void* memory,std::size_t) noexcept { std::free(memory); }
void operator delete[](void* memory,std::size_t) noexcept { std::free(memory); }

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
struct CloseDirectory {
    void operator()(DIR* directory)const noexcept { if(directory)::closedir(directory); }
};
std::set<int> openDescriptors() {
    std::unique_ptr<DIR,CloseDirectory> directory(::opendir("/proc/self/fd"));
    require(directory!=nullptr,"Cannot inspect Linux file-descriptor ownership");
    const int observer=::dirfd(directory.get());
    require(observer>=0,"Cannot identify the descriptor snapshot's own directory");
    std::set<int> descriptors;
    for(;;) {
        errno=0;
        const auto* entry=::readdir(directory.get());
        if(!entry) {
            require(errno==0,"Cannot finish the file-descriptor snapshot");
            break;
        }
        if(entry->d_name[0]=='.')continue;
        char* end=nullptr;
        const long descriptor=std::strtol(entry->d_name,&end,10);
        require(end!=entry->d_name&&*end=='\0'&&descriptor>=0&&descriptor<=INT_MAX,
                "Unexpected entry in /proc/self/fd");
        if(descriptor!=observer)descriptors.insert(static_cast<int>(descriptor));
    }
    return descriptors;
}
void checkAllocationOwnership(const fs::path& sandbox) {
    std::string pattern=(sandbox/"allocation_XXXXXX").string();
    require(::mkdtemp(pattern.data())!=nullptr,"Cannot reserve fresh allocation-fault evidence");
    const fs::path evidence=pattern;
    const std::optional<RunId> requested{RunId{std::string(64,'r')}};
    std::size_t failures_after_creation=0,failures=0;
    // 扫描全部分配点直到一次未触发故障的成功创建；不依赖参数求值顺序
    // 或某个固定分配序号。各次试验保留自己的目录，绝不清理旧证据。
    for(std::size_t point=0;;++point) {
        const fs::path root=evidence/("point_"+std::to_string(point));
        fs::create_directories(root/"origin");
        const fs::path reserved=root/"output"/requested->text;
        const auto before=openDescriptors();
        std::unique_ptr<io::RunDirectory> held;
        bool threw=false;
        allocation_probe::remaining=point;
        allocation_probe::triggered=false;
        allocation_probe::armed=true;
        try {
            auto created=io::RunDirectory::create(root,requested);
            allocation_probe::armed=false;
            if(created.hasValue())held=std::move(created).value();
        } catch(const std::bad_alloc&) {
            allocation_probe::armed=false;
            threw=true;
        } catch(...) {
            allocation_probe::armed=false;
            throw;
        }
        const bool injected=allocation_probe::triggered;
        const bool succeeded=held!=nullptr;
        if(injected) {
            ++failures;
            if(!succeeded&&fs::is_directory(reserved))++failures_after_creation;
        }
        if(succeeded) {
            const auto owned=openDescriptors();
            require(owned.size()==before.size()+2&&
                    std::includes(owned.begin(),owned.end(),before.begin(),before.end())&&
                    owned.count(held->runFd())==1&&before.count(held->runFd())==0,
                    "A live RunDirectory must own exactly two additional directory descriptors");
            struct stat actual{},expected{};
            const int flags=::fcntl(held->runFd(),F_GETFD);
            require(::fstat(held->runFd(),&actual)==0&&::stat(reserved.c_str(),&expected)==0&&
                    actual.st_dev==expected.st_dev&&actual.st_ino==expected.st_ino&&
                    flags>=0&&(flags&FD_CLOEXEC)!=0,
                    "Successful construction must retain the reserved RunId inode and close-on-exec flag");
            held.reset();
        }
        require(openDescriptors()==before,
                "Allocation failure or successful destruction must release every acquired descriptor");
        if(!injected) {
            require(succeeded&&!threw,"The allocation sweep must end with a normal successful reservation");
            break;
        }
    }
    require(failures>0&&failures_after_creation>0,
            "The sweep must exercise allocation failure after the RunId directory was created");
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
    checkAllocationOwnership(sandbox);
}
