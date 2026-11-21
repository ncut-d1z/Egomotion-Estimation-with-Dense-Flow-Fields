#include "io_fault_injection.hpp"

#include <cerrno>
#include <cstdarg>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>

extern "C" {
int __real_openat(int,const char*,int,...);
ssize_t __real_write(int,const void*,size_t);
int __real_fsync(int);
int __real_linkat(int,const char*,int,const char*,int);
int __real_unlinkat(int,const char*,int);
int __real_fstatat(int,const char*,struct stat*,int);
}

namespace io_fault_test {
namespace {
struct Active {
    bool armed=false;
    Point point=Point::None;
    int run_fd=-1;
    int staging_fd=-1;
    const char* final_name=nullptr;
    Observed events;
} active;
bool isStaging(const char* name) {
    return name&&(std::strncmp(name,".r5_",4)==0||
                  std::strncmp(name,".text_",6)==0);
}
int fail(int error=EIO) {
    ++active.events.triggered;
    errno=error;
    return -1;
}
bool selected(Point point) {return active.armed&&active.point==point;}
void createCompetitor(int dir,const char* name) {
    const int fd=__real_openat(dir,name,O_CREAT|O_EXCL|O_WRONLY|O_CLOEXEC,0600);
    if(fd<0)return;
    const auto size=sizeof(competitor_contents)-1;
    const auto written=__real_write(fd,competitor_contents,size);
    active.events.competitor_created=written==static_cast<ssize_t>(size)&&
        ::fstat(fd,&active.events.competitor_identity)==0;
    ::close(fd);
}
} // namespace

Scope::Scope(Point point,int run_fd,const char* final_name) {
    if(active.armed)throw std::logic_error("Nested I/O fault injection is forbidden");
    active=Active{};
    active.armed=true;
    active.point=point;
    active.run_fd=run_fd;
    active.final_name=final_name;
}
Scope::~Scope(){active.armed=false;}
Observed Scope::observed() const {return active.events;}
} // namespace io_fault_test

// --wrap 只替换这个测试程序所链接目标的未解析引用。
// 已编译动态 libmatio 的内部调用不被包装；不能声称注入了其 close/解码错误。
extern "C" int __wrap_openat(int dir,const char* name,int flags,...) {
    using namespace io_fault_test;
    mode_t mode=0;
    if(flags&O_CREAT){
        va_list args;va_start(args,flags);
        mode=va_arg(args,mode_t);va_end(args);
    }
    const bool staging=active.armed&&dir==active.run_fd&&isStaging(name)&&
                       (flags&O_CREAT)&&(flags&O_EXCL);
    if(staging&&selected(Point::OpenStaging))return fail(EACCES);
    const int fd=__real_openat(dir,name,flags,mode);
    if(staging&&fd>=0)active.staging_fd=fd;
    return fd;
}
extern "C" ssize_t __wrap_write(int fd,const void* data,size_t count) {
    using namespace io_fault_test;
    if(active.armed&&fd==active.staging_fd){
        if(selected(Point::WriteError))return fail();
        if(selected(Point::WriteZero)){++active.events.triggered;return 0;}
        if(active.events.triggered==0){
            if(selected(Point::WriteInterrupted))return fail(EINTR);
            if(selected(Point::WriteShort)&&count>1){
                ++active.events.triggered;
                return __real_write(fd,data,1);
            }
        }
    }
    return __real_write(fd,data,count);
}
extern "C" int __wrap_fsync(int fd) {
    using namespace io_fault_test;
    if(active.armed){
        if(fd==active.run_fd){
            const auto call=++active.events.directory_syncs;
            if((selected(Point::DirectorySyncFirst)&&call==1)||
               (selected(Point::DirectorySyncLast)&&call==2))return fail();
        }else if(fd==active.staging_fd){
            ++active.events.file_syncs;
            if(selected(Point::FileSync))return fail();
        }
    }
    return __real_fsync(fd);
}
extern "C" int __wrap_linkat(int old_dir,const char* old_name,
                              int new_dir,const char* new_name,int flags) {
    using namespace io_fault_test;
    const bool target=active.armed&&old_dir==active.run_fd&&
        new_dir==active.run_fd&&isStaging(old_name)&&
        std::strcmp(new_name,active.final_name)==0;
    if(target&&selected(Point::LinkError))return fail();
    if(target&&selected(Point::LinkCollision)){
        ++active.events.triggered;
        createCompetitor(new_dir,new_name);
    }
    const int result=__real_linkat(old_dir,old_name,new_dir,new_name,flags);
    if(target&&result==0)active.events.linked=true;
    return result;
}
extern "C" int __wrap_unlinkat(int dir,const char* name,int flags) {
    using namespace io_fault_test;
    if(active.armed&&dir==active.run_fd&&isStaging(name)){
        const auto call=++active.events.unlink_calls;
        if(selected(Point::UnlinkPersistent)||
           (selected(Point::UnlinkOnce)&&call==1))return fail();
    }
    return __real_unlinkat(dir,name,flags);
}
extern "C" int __wrap_fstatat(int dir,const char* name,struct stat* info,int flags) {
    using namespace io_fault_test;
    if(selected(Point::PublishedIdentity)&&active.events.linked&&
       dir==active.run_fd&&std::strcmp(name,active.final_name)==0)return fail();
    return __real_fstatat(dir,name,info,flags);
}
