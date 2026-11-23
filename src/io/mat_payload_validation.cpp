#include "mat_payload_validation.hpp"

#include <zlib.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstring>
#include <limits>
#include <string>

namespace egomotion::io::detail {
namespace {
// MAT v5格式常量；块大小只控制流式缓冲，不是输入/性能预算。
constexpr std::size_t kChunkBytes=8192,kTagBytes=8;
constexpr std::uint32_t kInt8=1,kInt32=5,kUint32=6,kSingle=7,kDouble=9,kMatrix=14;
constexpr std::uint32_t kDoubleClass=6,kLastNumericClass=15,kComplexFlag=0x0800;
struct InvalidPayload {MatErrorCode code;const char* message;};
void require(bool condition,MatErrorCode code,const char* message){
    if(!condition)throw InvalidPayload{code,message};
}
std::uint32_t word(const unsigned char* bytes,bool little){
    std::uint32_t value=0;
    for(unsigned i=0;i<4;++i)value|=std::uint32_t(bytes[little?i:3-i])<<(8*i);
    return value;
}
bool readAt(int fd,off_t offset,unsigned char* bytes,std::size_t size){
    while(size!=0){
        const auto count=::pread(fd,bytes,size,offset);
        if(count<0&&errno==EINTR)continue;
        if(count<=0)return false;
        offset+=count;bytes+=count;size-=static_cast<std::size_t>(count);
    }
    return true;
}
class PayloadStream final {
public:
    PayloadStream(int fd,off_t offset,std::uint64_t bytes,bool compressed)
        :fd_(fd),offset_(offset),remaining_(bytes),compressed_(compressed){
        if(compressed_){initialized_=::inflateInit(&z_)==Z_OK;healthy_=initialized_;}
    }
    ~PayloadStream(){if(initialized_)::inflateEnd(&z_);}
    PayloadStream(const PayloadStream&)=delete;
    PayloadStream& operator=(const PayloadStream&)=delete;
    bool read(unsigned char* destination,std::size_t count){
        while(count!=0){
            if(position_==available_&&!fill())return false;
            const auto take=std::min(count,available_-position_);
            if(destination){std::memcpy(destination,output_.data()+position_,take);destination+=take;}
            count-=take;position_+=take;
        }
        return healthy_;
    }
    bool finish(){
        unsigned char extra=0;
        if(read(&extra,1))return false; // 禁止忽略多余解压字节或第二个元素。
        return healthy_&&ended_;
    }
private:
    bool fill(){
        position_=available_=0;
        if(!healthy_||ended_)return false;
        if(!compressed_){
            if(remaining_==0){ended_=true;return false;}
            available_=static_cast<std::size_t>(std::min<std::uint64_t>(remaining_,output_.size()));
            healthy_=readAt(fd_,offset_,output_.data(),available_);
            offset_+=static_cast<off_t>(available_);remaining_-=available_;
            return healthy_;
        }
        while(healthy_&&!ended_&&available_==0){
            if(z_.avail_in==0&&remaining_!=0){
                const auto count=static_cast<std::size_t>(std::min<std::uint64_t>(remaining_,input_.size()));
                if(!readAt(fd_,offset_,input_.data(),count)){healthy_=false;break;}
                offset_+=static_cast<off_t>(count);remaining_-=count;
                z_.next_in=input_.data();z_.avail_in=static_cast<uInt>(count);
            }
            const auto before=z_.avail_in;
            z_.next_out=output_.data();z_.avail_out=static_cast<uInt>(output_.size());
            const int status=::inflate(&z_,Z_NO_FLUSH);
            available_=output_.size()-z_.avail_out;
            if(status==Z_STREAM_END){
                ended_=true;healthy_=remaining_==0&&z_.avail_in==0;
            }else if(status!=Z_OK||(before==z_.avail_in&&available_==0))healthy_=false;
        }
        return healthy_&&available_!=0;
    }
    int fd_;off_t offset_;std::uint64_t remaining_;
    bool compressed_,initialized_=false,healthy_=true,ended_=false;
    z_stream z_{};
    std::array<unsigned char,kChunkBytes> input_{},output_{};
    std::size_t position_=0,available_=0;
};
struct Element {
    std::uint32_t type=0,bytes=0;
    std::size_t consumed=0,padding=0;
    bool small=false;
    std::array<unsigned char,4> inline_data{};
};
Element nextElement(PayloadStream& stream,std::uint64_t& remaining,bool little){
    std::array<unsigned char,kTagBytes> raw{};
    require(remaining>=raw.size()&&stream.read(raw.data(),raw.size()),
            MatErrorCode::CorruptCompressedData,"Truncated MAT matrix subelement tag");
    remaining-=raw.size();
    const auto packed=word(raw.data(),little);
    Element out;out.small=(packed>>16)!=0;
    out.type=out.small?packed&0xffff:packed;
    out.bytes=out.small?packed>>16:word(raw.data()+4,little);
    if(out.small){
        require(out.bytes<=4,MatErrorCode::CorruptCompressedData,"Invalid MAT small-element length");
        std::copy(raw.begin()+4,raw.end(),out.inline_data.begin());
    }else{
        out.padding=(kTagBytes-out.bytes%kTagBytes)%kTagBytes;
        const auto occupied=std::uint64_t(out.bytes)+out.padding;
        require(occupied<=remaining,MatErrorCode::CorruptCompressedData,"MAT subelement exceeds matrix boundary");
        remaining-=occupied;
    }
    return out;
}
void readElement(PayloadStream& stream,Element& field,unsigned char* out,std::size_t count){
    require(count<=field.bytes-field.consumed,MatErrorCode::CorruptCompressedData,"MAT element read exceeds declared size");
    if(field.small){if(out)std::memcpy(out,field.inline_data.data()+field.consumed,count);}
    else require(stream.read(out,count),MatErrorCode::CorruptCompressedData,"Truncated MAT element payload");
    field.consumed+=count;
}
void finishElement(PayloadStream& stream,Element& field){
    readElement(stream,field,nullptr,field.bytes-field.consumed);
    require(stream.read(nullptr,field.padding),MatErrorCode::CorruptCompressedData,"Truncated MAT element padding");
}
std::uint32_t flags(PayloadStream& stream,std::uint64_t& remaining,bool little){
    auto field=nextElement(stream,remaining,little);
    require(field.type==kUint32&&field.bytes==8,MatErrorCode::InvalidMatHeader,"MAT array flags require two uint32 words");
    std::array<unsigned char,8> data{};readElement(stream,field,data.data(),data.size());
    finishElement(stream,field);return word(data.data(),little);
}
std::size_t dimensions(PayloadStream& stream,std::uint64_t& remaining,bool little,std::uint32_t cls){
    auto field=nextElement(stream,remaining,little);
    require(field.type==kInt32&&field.bytes!=0&&field.bytes%4==0,
            MatErrorCode::InvalidShape,"MAT dimensions require signed int32 elements");
    std::size_t count=1;
    for(std::uint32_t dimension=0;dimension<field.bytes/4;++dimension){
        std::array<unsigned char,4> data{};readElement(stream,field,data.data(),data.size());
        const auto value=word(data.data(),little);
        require(value<=static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max()),
                MatErrorCode::InvalidShape,"Negative MAT dimension");
        require(value==0||count<=std::numeric_limits<std::size_t>::max()/value,
                MatErrorCode::SizeOverflow,"MAT dimensions overflow element count before decoding");
        count*=value;
    }
    finishElement(stream,field);
    if(cls==kDoubleClass)
        require(count<=std::numeric_limits<std::size_t>::max()/sizeof(double),
                MatErrorCode::SizeOverflow,"MAT dimensions overflow decoded double bytes before allocation");
    return count;
}
std::size_t numericWidth(std::uint32_t type){
    switch(type){
    case 1:case 2:return 1;
    case 3:case 4:return 2;
    case 5:case 6:case 7:return 4;
    case kDouble:case 12:case 13:return 8;
    default:return 0;
    }
}
bool selectedName(PayloadStream& stream,Element& name,std::string_view requested){
    if(requested.empty()){finishElement(stream,name);return true;}
    bool matches=true,terminated=false;
    std::size_t compared=0;
    std::array<unsigned char,128> chunk{}; // 只比较名字，不按文件声明长度分配字符串。
    while(name.consumed<name.bytes){
        const auto offset=name.consumed;
        const auto count=std::min<std::size_t>(name.bytes-offset,chunk.size());
        readElement(stream,name,chunk.data(),count);
        // matio按C字符串查名；首个NUL之后仍消费全部原始字节，但不参与选择。
        for(std::size_t i=0;i<count&&!terminated;++i){
            if(chunk[i]==0){terminated=true;break;}
            if(compared>=requested.size()||chunk[i]!=static_cast<unsigned char>(requested[compared]))
                matches=false;
            ++compared;
        }
    }
    finishElement(stream,name);return matches&&compared==requested.size();
}
void numericPayload(PayloadStream& stream,std::uint64_t& remaining,bool little,
                    std::uint32_t cls,std::size_t count,bool selected){
    auto field=nextElement(stream,remaining,little);
    const auto width=numericWidth(field.type);
    require(width!=0,MatErrorCode::UnsupportedClassOrType,"Unsupported MAT numeric storage tag");
    require(!(selected&&cls==kDoubleClass&&field.type==kSingle),MatErrorCode::UnsupportedClassOrType,
            "Implicit single storage to double decoding is prohibited");
    require(count<=std::numeric_limits<std::size_t>::max()/width,
            MatErrorCode::SizeOverflow,"MAT numeric storage byte count overflows");
    require(count*width==field.bytes,MatErrorCode::InvalidShape,"MAT storage length differs from dimensions and element width");
    finishElement(stream,field);
}
void validateMatrix(PayloadStream& stream,bool little,std::string_view requested){
    std::array<unsigned char,kTagBytes> root{};
    require(stream.read(root.data(),root.size())&&word(root.data(),little)==kMatrix,
            MatErrorCode::CorruptCompressedData,"MAT payload does not contain a complete miMATRIX");
    std::uint64_t remaining=word(root.data()+4,little);
    const auto array_flags=flags(stream,remaining,little),cls=array_flags&0xff;
    const auto count=dimensions(stream,remaining,little,cls);
    auto name=nextElement(stream,remaining,little);
    require(name.type==kInt8,MatErrorCode::InvalidMatHeader,"MAT matrix name requires an int8 element");
    const bool selected=selectedName(stream,name,requested);
    if(cls>=kDoubleClass&&cls<=kLastNumericClass){
        numericPayload(stream,remaining,little,cls,count,selected);
        if((array_flags&kComplexFlag)!=0)numericPayload(stream,remaining,little,cls,count,selected);
        require(remaining==0,MatErrorCode::CorruptCompressedData,"Unexpected bytes after MAT numeric payload");
    }else{
        // 不宣称支持struct/cell/sparse：只检查当前层边界，业务类型仍由Reader拒绝。
        while(remaining!=0){auto field=nextElement(stream,remaining,little);finishElement(stream,field);}
    }
    require(stream.finish(),MatErrorCode::CorruptCompressedData,"MAT matrix length or complete compressed stream mismatch");
}
} // namespace

Status<MatErrorCode> validateMat5Payload(
    int fd,off_t offset,std::uint64_t bytes,bool compressed,bool little_endian,
    const std::filesystem::path& path,std::string_view variable){
    try{
        PayloadStream stream(fd,offset,bytes,compressed);
        validateMatrix(stream,little_endian,variable);
        return Status<MatErrorCode>::success();
    }catch(const InvalidPayload& invalid){
        ErrorDetail detail;detail.stage=ErrorStage::Decode;detail.source_path=path.string();
        detail.variable=std::string(variable);detail.message=invalid.message;
        return Status<MatErrorCode>::failure({invalid.code,std::move(detail)});
    }
}
} // namespace egomotion::io::detail
