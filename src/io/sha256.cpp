#include "egomotion/io/sha256.hpp"

#include <array>
#include <cerrno>
#include <cstdint>
#include <limits>
#include <string>
#include <sys/types.h>
#include <unistd.h>

namespace egomotion::io {
namespace {
constexpr std::array<std::uint32_t,64> constants{{
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
    0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
    0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
    0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
    0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
    0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
}};
constexpr std::uint32_t rotr(std::uint32_t x,unsigned n) noexcept {
    return (x>>n)|(x<<(32u-n));
}
class Hasher {
public:
    Hasher():state_{{0x6a09e667u,0xbb67ae85u,0x3c6ef372u,0xa54ff53au,
                     0x510e527fu,0x9b05688cu,0x1f83d9abu,0x5be0cd19u}} {}
    bool update(const unsigned char* input,std::size_t length) noexcept {
        if(length>(std::numeric_limits<std::uint64_t>::max()/8u)-bytes_)return false;
        bytes_+=length;
        for(std::size_t i=0;i<length;++i){
            block_[filled_++]=input[i];
            if(filled_==64){transform();filled_=0;}
        }
        return true;
    }
    std::string finalize(){
        const std::uint64_t bit_count=bytes_*8;
        block_[filled_++]=0x80;
        if(filled_>56) {
            while(filled_<64)block_[filled_++]=0;
            transform();filled_=0;
        }
        while(filled_<56)block_[filled_++]=0;
        for(int i=7;i>=0;--i)block_[filled_++]=
            static_cast<unsigned char>((bit_count>>(static_cast<unsigned>(i)*8u))&255u);
        transform();
        constexpr char hex[]="0123456789abcdef";
        std::string out;
        out.reserve(64);
        for(auto word:state_)for(int byte=3;byte>=0;--byte){
            const auto v=static_cast<unsigned char>(
                word>>(static_cast<unsigned>(byte)*8u));
            out.push_back(hex[v>>4]);out.push_back(hex[v&15]);
        }
        return out;
    }
private:
    void transform() noexcept {
        std::uint32_t w[64]{};
        for(unsigned i=0;i<16;++i){
            const unsigned j=4*i;
            w[i]=(std::uint32_t(block_[j])<<24u)|
                 (std::uint32_t(block_[j+1])<<16u)|
                 (std::uint32_t(block_[j+2])<<8u)|
                 std::uint32_t(block_[j+3]);
        }
        for(unsigned i=16;i<64;++i){
            const auto s0=rotr(w[i-15],7)^rotr(w[i-15],18)^(w[i-15]>>3);
            const auto s1=rotr(w[i-2],17)^rotr(w[i-2],19)^(w[i-2]>>10);
            w[i]=w[i-16]+s0+w[i-7]+s1;
        }
        auto a=state_[0],b=state_[1],c=state_[2],d=state_[3];
        auto e=state_[4],f=state_[5],g=state_[6],h=state_[7];
        for(unsigned i=0;i<64;++i){
            const auto S1=rotr(e,6)^rotr(e,11)^rotr(e,25);
            const auto choose=(e&f)^(~e&g);
            const auto temp1=h+S1+choose+constants[i]+w[i];
            const auto S0=rotr(a,2)^rotr(a,13)^rotr(a,22);
            const auto majority=(a&b)^(a&c)^(b&c);
            const auto temp2=S0+majority;
            h=g;g=f;f=e;e=d+temp1;d=c;c=b;b=a;a=temp1+temp2;
        }
        const std::array<std::uint32_t,8> final{{a,b,c,d,e,f,g,h}};
        for(std::size_t i=0;i<8;++i)state_[i]+=final[i];
    }
    std::array<std::uint32_t,8> state_;
    std::array<unsigned char,64> block_{};
    std::size_t filled_=0;
    std::uint64_t bytes_=0;
};
Result<std::string,MatErrorCode> fail(std::string message) {
    ErrorDetail detail;detail.stage=ErrorStage::Output;
    detail.message=std::move(message);
    return Result<std::string,MatErrorCode>::failure(
        {MatErrorCode::ReadbackMismatch,std::move(detail)});
}
}  // namespace

Result<std::string,MatErrorCode> sha256Fd(int fd) {
    if(fd<0)return fail("Cannot SHA-256 an invalid file descriptor");
    Hasher hash;
    std::array<unsigned char,4096> chunk{};
    std::uint64_t offset=0;
    while(true){
        if(offset>static_cast<std::uint64_t>(std::numeric_limits<off_t>::max()))
            return fail("SHA-256 input exceeds supported file offsets");
        const auto n=::pread(fd,chunk.data(),chunk.size(),static_cast<off_t>(offset));
        if(n<0){if(errno==EINTR)continue;return fail("SHA-256 file pread failed");}
        if(n==0)break;
        if(!hash.update(chunk.data(),static_cast<std::size_t>(n)))
            return fail("SHA-256 input length exceeds 64-bit bit count");
        offset+=static_cast<std::uint64_t>(n);
    }
    return Result<std::string,MatErrorCode>::success(hash.finalize());
}
}  // namespace egomotion::io
