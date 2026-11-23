// 一次性合成版本负例生成器：只用预装matio的可选HDF5能力，不属于生产运行依赖。
// 不读取或复制origin数据，不把该文件称为算法Golden。
#include <matio.h>
#include <array>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace {
using Mat=std::unique_ptr<mat_t,decltype(&Mat_Close)>;
using Variable=std::unique_ptr<matvar_t,decltype(&Mat_VarFree)>;
constexpr std::array<double,6> kValues{{1,-0.0,-3.5,4,0.25,6}};
void require(bool ok,const char* message) {if(!ok)throw std::runtime_error(message);}
void generate(const std::filesystem::path& output) {
    require(!std::filesystem::exists(output),"Refuse existing fixture output");
    Mat file(Mat_CreateVer(output.c_str(),"Synthetic R6 MAT v7.3 rejection fixture; no original research data",MAT_FT_MAT73),Mat_Close);
    require(bool(file),"Fixture generation requires matio built with HDF5 support");
    auto values=kValues;std::size_t dimensions[2]={2,3};
    Variable variable(Mat_VarCreate("value",MAT_C_DOUBLE,MAT_T_DOUBLE,2,dimensions,values.data(),0),Mat_VarFree);
    require(bool(variable),"Cannot allocate synthetic double array");
    require(Mat_VarWrite(file.get(),variable.get(),MAT_COMPRESSION_NONE)==MATIO_E_NO_ERROR,"Fixture write failed");
    require(Mat_Close(file.release())==MATIO_E_NO_ERROR,"Fixture close failed");
}
void verify(const std::filesystem::path& output) {
    Mat file(Mat_Open(output.c_str(),MAT_ACC_RDONLY),Mat_Close);
    require(bool(file)&&Mat_GetVersion(file.get())==MAT_FT_MAT73,"Fixture must reopen as real MAT v7.3");
    std::size_t count=0;char* const* directory=Mat_GetDir(file.get(),&count);
    require(directory&&count==1&&directory[0]&&std::strcmp(directory[0],"value")==0,"Exact one-variable fixture directory");
    Variable value(Mat_VarRead(file.get(),"value"),Mat_VarFree);
    require(value&&value->rank==2&&value->dims&&value->dims[0]==2&&value->dims[1]==3&&
        value->class_type==MAT_C_DOUBLE&&value->data_type==MAT_T_DOUBLE&&
        !value->isComplex&&!value->isLogical&&value->nbytes==sizeof(kValues)&&value->data,
        "Unexpected synthetic MAT v7.3 payload metadata");
    require(std::memcmp(value->data,kValues.data(),sizeof(kValues))==0,"Fixture binary64 column-major payload mismatch");
    std::cout<<"Verified MAT_FT_MAT73, variable=value, shape=2x3, real double, six bitwise values; file="<<output<<'\n';
}
}
int main(int argc,char** argv) {
    try {require(argc==2,"Expected a new fixture output path");generate(argv[1]);verify(argv[1]);return 0;}
    catch(const std::exception& error){std::cerr<<"FAIL fixture generation: "<<error.what()<<'\n';return 1;}
}
