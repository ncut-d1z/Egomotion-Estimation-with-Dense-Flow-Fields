#pragma once

#include <sys/stat.h>

// 仅供 Linux 链接层回归测试；生产库及 CLI 不提供故障开关。
namespace io_fault_test {
enum class Point {
    None, OpenStaging, WriteError, WriteZero, WriteInterrupted, WriteShort,
    FileSync, LinkError, LinkCollision, PublishedIdentity,
    DirectorySyncFirst, UnlinkOnce, UnlinkPersistent, DirectorySyncLast
};
struct Observed {
    unsigned triggered=0;
    unsigned file_syncs=0;
    unsigned directory_syncs=0;
    unsigned unlink_calls=0;
    bool linked=false;
    bool competitor_created=false;
    struct stat competitor_identity{};
};

class Scope {
public:
    Scope(Point point,int run_fd,const char* final_name);
    ~Scope();
    Scope(const Scope&)=delete;
    Scope& operator=(const Scope&)=delete;
    Observed observed() const;
};
inline constexpr char competitor_contents[]="concurrent writer must survive";
} // namespace io_fault_test
