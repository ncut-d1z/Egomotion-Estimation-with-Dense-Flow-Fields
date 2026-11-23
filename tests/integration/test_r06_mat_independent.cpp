// 独立 L2 Reader 对照：按 MAT v5 标签解析原始值，参考路径不调用 matio。
// 限定为本仓库单变量、little-endian、zlib、二维 mxDOUBLE 归档。
// 格式依据 MathWorks MAT-File Format；不执行 MATLAB，不作为生产 Reader。
#include "egomotion/io/mat_result_reader.hpp"
#include "egomotion/io/sha256.hpp"

#include "../support/archive_mat_reference.hpp"
#include <fcntl.h>
#include <unistd.h>

#include <cstdint>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
namespace fs=std::filesystem;
using archive_reference::decodeIndependent;
using archive_reference::require;
using archive_reference::kInt32;
using archive_reference::kDouble;
using archive_reference::kDoubleClass;
// 固定原始归档及独立身份清单的实测数量，不是可调通过阈值。
constexpr std::size_t kArchiveFiles=582,kPackedInt32Arrays=80,kDoubleArrays=502;
void checkFile(const fs::path& root,const std::string& relative,const std::string& expected_sha,
               std::uint32_t storage_type,std::size_t rows,std::size_t cols,const std::string& name) {
    using namespace egomotion;
    const fs::path path=root/relative;
    require(!fs::is_symlink(fs::symlink_status(path)),"Reference path cannot be a symlink");
    const auto reference=decodeIndependent(path);
    require(reference.name==name&&reference.rows==rows&&reference.cols==cols&&
            reference.storage_type==storage_type,"Raw fields differ from independent inventory: "+relative);
    // 原始归档可能与当前生成脚本不同；只报告独立解码事实，不修补原值。
    if(name=="xwerte"&&relative.find("/40/")!=std::string::npos)
        std::cout<<std::setprecision(17)<<"ARCHIVE_X file="<<relative<<" row3="<<reference.values.at(2)
                 <<" row10="<<reference.values.at(9)<<'\n';
    std::size_t nan_count=0,infinity_count=0;
    std::string nonfinite_indices;
    for(std::size_t i=0;i<reference.values.size();++i) {
        const double x=reference.values[i];
        nan_count+=std::isnan(x);infinity_count+=std::isinf(x);
        if(!std::isfinite(x)) {
            if(!nonfinite_indices.empty())nonfinite_indices+=",";
            nonfinite_indices+=std::to_string(i+1);
        }
    }
    if(nan_count||infinity_count)
        std::cout<<"ARCHIVE_NONFINITE file="<<relative<<" nan="<<nan_count
                 <<" infinity="<<infinity_count<<" matlab_linear_indices="
                 <<nonfinite_indices<<'\n';
    const int fd=::open(path.c_str(),O_RDONLY|O_CLOEXEC|O_NOFOLLOW);
    require(fd>=0,"Cannot open SHA reference");
    const auto digest=io::sha256Fd(fd);::close(fd);
    require(digest.hasValue()&&digest.value()==expected_sha,"SHA-256 differs from fixed inventory: "+relative);
    const auto decoded=io::MatResultReader{}.readNamed(path,name);
    require(decoded.hasValue(),"Production Reader failed: "+relative);
    const auto& value=decoded.value();
    require(value.source_mat_sha256&&*value.source_mat_sha256==expected_sha,
            "Decoded snapshot identity differs from independent fixed MAT inventory: "+relative);
    require(value.info.dimensions==std::vector<std::size_t>({rows,cols})&&
            value.info.original_class_type==kDoubleClass&&value.info.original_data_type==kDouble&&
            !value.info.is_complex&&!value.info.is_logical&&
            value.info.data_bytes==reference.values.size()*sizeof(double)&&
            value.values_column_major.size()==reference.values.size()&&
            std::memcmp(value.values_column_major.data(),reference.values.data(),value.info.data_bytes)==0,
            "Production decoded values/column order differ bitwise from independent parser: "+relative);
}
} // namespace
int main(int argc,char** argv) {
    static_assert(sizeof(double)==8&&std::numeric_limits<double>::is_iec559,
                  "Independent MAT reference needs IEEE binary64");
    try{
        require(argc==3,"Expected original archive and fixed independent inventory");
        const fs::path root(argv[1]);
        std::ifstream inventory(argv[2]);
        require(inventory.is_open(),"Cannot open independent inventory");
        std::string line;std::set<std::string> files;
        std::size_t int32_arrays=0,double_arrays=0,elements=0;
        while(std::getline(inventory,line)){
            if(line.empty()||line.front()=='#')continue;
            std::istringstream fields(line);
            std::string sha,name,relative;std::uint32_t storage=0;std::size_t rows=0,cols=0;
            require(bool(fields>>sha>>storage>>rows>>cols>>name>>relative),"Malformed reference row");
            std::string trailing;
            require(!(fields>>trailing)&&sha.size()==64&&files.insert(relative).second&&
                    !fs::path(relative).is_absolute()&&relative.find("..")==std::string::npos,
                    "Duplicate, unsafe or malformed reference identity");
            checkFile(root,relative,sha,storage,rows,cols,name);
            int32_arrays+=storage==kInt32;double_arrays+=storage==kDouble;elements+=rows*cols;
        }
        require(files.size()==kArchiveFiles&&int32_arrays==kPackedInt32Arrays&&double_arrays==kDoubleArrays,
                "Independent file/type inventory differs from the original archive");
        std::cout<<"PASS independent L2 Reader: files="<<files.size()<<" elements="<<elements
                 <<" mxDOUBLE/miINT32="<<int32_arrays<<" mxDOUBLE/miDOUBLE="<<double_arrays
                 <<"; SHA-256 and column-major binary64 values exact (candidate reference)\n";
        return 0;
    }catch(const std::exception& error){std::cerr<<"FAIL independent MAT: "<<error.what()<<'\n';return 1;}
}
