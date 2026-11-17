#include "mat_container_validation.hpp"
#include "mat_payload_validation.hpp"

#include <sys/stat.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cstdint>
#include <string>

namespace egomotion::io::detail {
namespace {
// MAT Level 5 的容器常量。压缩元素后没有普通元素的8字节填充。
constexpr std::size_t kHeaderBytes=128,kTagBytes=8;
constexpr std::uint32_t kMatrix=14,kCompressed=15;
Status<MatErrorCode> failure(MatErrorCode code,const std::filesystem::path& path,
                            std::string_view variable,const char* message) {
    ErrorDetail detail;
    detail.stage=ErrorStage::Decode;detail.source_path=path.string();
    detail.variable=std::string(variable);detail.message=message;
    return Status<MatErrorCode>::failure({code,std::move(detail)});
}
bool readAt(int fd,off_t offset,unsigned char* data,std::size_t size) {
    while(size!=0){
        const auto count=::pread(fd,data,size,offset);
        if(count<0&&errno==EINTR)continue;
        if(count<=0)return false;
        size-=static_cast<std::size_t>(count);data+=count;offset+=count;
    }
    return true;
}
std::uint32_t u32(const unsigned char* bytes,bool little_endian) {
    std::uint32_t value=0;
    for(unsigned i=0;i<4;++i)
        value|=std::uint32_t(bytes[little_endian?i:3-i])<<(8*i);
    return value;
}
} // namespace

Status<MatErrorCode> validateMat5Container(int fd,const std::filesystem::path& path,
                                        std::string_view variable) {
    struct stat info{};
    std::array<unsigned char,kHeaderBytes> header{};
    if(::fstat(fd,&info)!=0||!S_ISREG(info.st_mode)||info.st_size<static_cast<off_t>(header.size())||
       !readAt(fd,0,header.data(),header.size()))
        return failure(MatErrorCode::InvalidMatHeader,path,variable,"MAT v5 header is incomplete or file is not regular");
    const bool little=header[126]=='I'&&header[127]=='M';
    const bool big=header[126]=='M'&&header[127]=='I';
    if((!little&&!big)||header[little?124:125]!=0||header[little?125:124]!=1)
        return failure(MatErrorCode::InvalidMatHeader,path,variable,"Invalid MAT v5 endian/version fields");
    off_t offset=kHeaderBytes;
    while(offset<info.st_size){
        std::array<unsigned char,kTagBytes> tag{};
        if(info.st_size-offset<static_cast<off_t>(tag.size())||!readAt(fd,offset,tag.data(),tag.size()))
            return failure(MatErrorCode::CorruptCompressedData,path,variable,"Truncated MAT top-level tag");
        const auto type=u32(tag.data(),little),bytes=u32(tag.data()+4,little);
        if(type!=kMatrix&&type!=kCompressed)
            return failure(MatErrorCode::InvalidMatHeader,path,variable,"Invalid MAT top-level element type");
        offset+=static_cast<off_t>(tag.size());
        if(static_cast<std::uint64_t>(info.st_size-offset)<bytes)
            return failure(MatErrorCode::CorruptCompressedData,path,variable,"MAT element exceeds actual file length");
        const bool compressed=type==kCompressed;
        auto payload=validateMat5Payload(fd,compressed?offset:offset-static_cast<off_t>(kTagBytes),
            std::uint64_t(bytes)+(compressed?0:kTagBytes),compressed,little,path,variable);
        if(!payload.hasValue())return payload;
        offset+=static_cast<off_t>(bytes);
        const auto padding=type==kMatrix?(kTagBytes-bytes%kTagBytes)%kTagBytes:0;
        if(static_cast<std::uint64_t>(info.st_size-offset)<padding)
            return failure(MatErrorCode::CorruptCompressedData,path,variable,"Truncated MAT element padding");
        offset+=static_cast<off_t>(padding);
    }
    return Status<MatErrorCode>::success();
}
} // namespace egomotion::io::detail
