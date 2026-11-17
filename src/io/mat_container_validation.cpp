#include "mat_container_validation.hpp"

#include <zlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdint>
#include <string>

namespace egomotion::io::detail {
namespace {
// MAT Level 5 的容器常量。压缩元素后没有普通元素的8字节填充。
constexpr std::size_t kHeaderBytes=128,kTagBytes=8,kChunkBytes=8192;
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
class Inflater final {
public:
    z_stream stream{};
    const int initialized=::inflateInit(&stream);
    ~Inflater(){if(initialized==Z_OK)::inflateEnd(&stream);}
    Inflater()=default;
    Inflater(const Inflater&)=delete;
    Inflater& operator=(const Inflater&)=delete;
};
bool completeZlib(int fd,off_t offset,std::uint32_t bytes) {
    Inflater inflater;
    if(inflater.initialized!=Z_OK)return false;
    auto& stream=inflater.stream;
    std::array<unsigned char,kChunkBytes> input{},output{};
    std::uint32_t remaining=bytes;
    for(;;){
        if(stream.avail_in==0&&remaining!=0){
            const auto count=std::min<std::size_t>(remaining,input.size());
            if(!readAt(fd,offset,input.data(),count))return false;
            offset+=static_cast<off_t>(count);remaining-=static_cast<std::uint32_t>(count);
            stream.next_in=input.data();stream.avail_in=static_cast<uInt>(count);
        }
        const auto input_before=stream.avail_in;
        stream.next_out=output.data();stream.avail_out=static_cast<uInt>(output.size());
        const int status=::inflate(&stream,Z_NO_FLUSH);
        // 必须消耗完整声明长度并验证 Adler-32；不能以“已经填满数值数组”结束。
        if(status==Z_STREAM_END)return remaining==0&&stream.avail_in==0;
        if(status!=Z_OK||(stream.avail_in==input_before&&stream.avail_out==output.size()))return false;
    }
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
        if(type==kCompressed&&!completeZlib(fd,offset,bytes))
            return failure(MatErrorCode::CorruptCompressedData,path,variable,"Incomplete, corrupt or trailing zlib stream");
        offset+=static_cast<off_t>(bytes);
        const auto padding=type==kMatrix?(kTagBytes-bytes%kTagBytes)%kTagBytes:0;
        if(static_cast<std::uint64_t>(info.st_size-offset)<padding)
            return failure(MatErrorCode::CorruptCompressedData,path,variable,"Truncated MAT element padding");
        offset+=static_cast<off_t>(padding);
    }
    return Status<MatErrorCode>::success();
}
} // namespace egomotion::io::detail
