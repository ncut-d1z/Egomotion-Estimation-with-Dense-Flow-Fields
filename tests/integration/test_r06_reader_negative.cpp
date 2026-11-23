// 合成 MAT 边界用例仅写 build 沙箱；保留每次 RunId 和所有失败文件。
#include "egomotion/io/mat_result_reader.hpp"
#include "egomotion/io/run_directory.hpp"
#include "egomotion/io/sha256.hpp"

#include <matio.h>
#include <fcntl.h>
#include <unistd.h>

#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
using namespace egomotion;
namespace fs=std::filesystem;
using Variable=std::unique_ptr<matvar_t,decltype(&Mat_VarFree)>;
using Mat=std::unique_ptr<mat_t,decltype(&Mat_Close)>;
using Bytes=std::vector<unsigned char>;
const std::array<double,6> kValues{{1,-0.0,-3.5,4,0.25,6}};

void require(bool value,const std::string& message) {
    if(!value)throw std::runtime_error(message);
}
Variable numeric(const char* name,matio_classes cls,matio_types type,void* data,
                 std::vector<std::size_t> dims={2,3},int flags=0) {
    Variable result(Mat_VarCreate(name,cls,type,static_cast<int>(dims.size()),
                                 dims.data(),data,flags),Mat_VarFree);
    require(bool(result),"Cannot create fixture variable");return result;
}
Variable real(const char* name) {
    auto data=kValues;
    return numeric(name,MAT_C_DOUBLE,MAT_T_DOUBLE,data.data());
}
struct Fixture {
    std::string name;
    Variable variable;
    bool accepted;
    MatErrorCode error=MatErrorCode::UnsupportedClassOrType;
};
std::vector<Fixture> fixtures() {
    std::vector<Fixture> out;
    auto doubles=kValues;
    std::array<float,6> singles{{1,2,3,4,5,6}};
    std::array<mat_int32_t,6> integers{{1,-2,3,-4,5,6}};
    std::array<mat_uint8_t,6> logical{{0,1,0,1,0,1}};
    std::array<mat_uint8_t,6> chars{{'a','b','c','d','e','f'}};
    mat_complex_split_t complex{doubles.data(),doubles.data()};
    out.push_back({"double",real("value"),true});
    out.push_back({"single",numeric("value",MAT_C_SINGLE,MAT_T_SINGLE,singles.data()),false});
    out.push_back({"int32",numeric("value",MAT_C_INT32,MAT_T_INT32,integers.data()),false});
    out.push_back({"logical",numeric("value",MAT_C_UINT8,MAT_T_UINT8,logical.data(),{2,3},MAT_F_LOGICAL),false});
    out.push_back({"complex",numeric("value",MAT_C_DOUBLE,MAT_T_DOUBLE,&complex,{2,3},MAT_F_COMPLEX),false});
    // 明示实际字符编码；matio1.5.26把UINT8 char写成压缩UTF8时会高估根长度。
    // 错误根长度另由独立raw夹具检查，不混入“合法但不支持的char”场景。
    out.push_back({"char",numeric("value",MAT_C_CHAR,MAT_T_UTF8,chars.data()),false});
    out.push_back({"rank3",numeric("value",MAT_C_DOUBLE,MAT_T_DOUBLE,doubles.data(),{2,1,3}),false,MatErrorCode::InvalidShape});
    out.push_back({"empty",numeric("value",MAT_C_DOUBLE,MAT_T_DOUBLE,nullptr,{0,3}),false,MatErrorCode::InvalidShape});

    auto child=real("nested");
    matvar_t* cell_data[]={child.get()};
    auto cell=numeric("value",MAT_C_CELL,MAT_T_CELL,cell_data,{1,1});
    child.release(); // matio 接管 cell 的子变量；不是借用栈上的 numeric 数据。
    out.push_back({"cell",std::move(cell),false});
    std::size_t dims[]={1,1};const char* fields[]={"nested"};
    Variable structure(Mat_VarCreateStruct("value",2,dims,fields,1),Mat_VarFree);
    require(bool(structure),"Cannot create struct fixture");
    child=real("nested");
    require(Mat_VarSetStructFieldByName(structure.get(),"nested",0,child.get())==nullptr,
            "Unexpected existing struct child");
    child.release();
    out.push_back({"struct",std::move(structure),false});
    mat_uint32_t row_indices[]={0,1},column_offsets[]={0,1,1,2};
    double sparse_values[]={2,3};
    mat_sparse_t sparse{2,row_indices,2,column_offsets,4,2,sparse_values};
    out.push_back({"sparse",numeric("value",MAT_C_SPARSE,MAT_T_DOUBLE,&sparse),false});
    return out;
}
void save(const fs::path& path,const std::vector<matvar_t*>& variables,
          matio_compression compression,mat_ft version=MAT_FT_MAT5) {
    require(!fs::exists(path),"Refuse fixture overwrite");
    Mat file(Mat_CreateVer(path.c_str(),"Synthetic R6 Reader boundary fixture",version),Mat_Close);
    require(bool(file),"Cannot create isolated MAT fixture");
    for(auto* variable:variables)
        require(Mat_VarWrite(file.get(),variable,compression)==MATIO_E_NO_ERROR,
                "Fixture MAT write failed");
    require(Mat_Close(file.release())==MATIO_E_NO_ERROR,"Fixture MAT close failed");
}
std::string digest(const fs::path& path) {
    const int fd=::open(path.c_str(),O_RDONLY|O_NOFOLLOW|O_CLOEXEC);
    require(fd>=0,"Cannot open fixture identity");
    const auto hash=io::sha256Fd(fd);::close(fd);
    require(hash.hasValue(),"Cannot hash fixture");return hash.value();
}
void checkFixture(const fs::path& path,const Fixture& fixture) {
    const auto before=digest(path);
    io::MatResultReader reader;
    const auto metadata=reader.inspect(path);
    require(metadata.hasValue()&&metadata.value().size()==1,"inspect must enumerate unsupported valid types");
    const auto& info=metadata.value().front();
    std::vector<std::size_t> shape(fixture.variable->dims,
                                 fixture.variable->dims+fixture.variable->rank);
    require(info.name=="value"&&info.dimensions==shape&&
            info.original_class_type==fixture.variable->class_type&&
            info.is_complex==bool(fixture.variable->isComplex)&&
            info.is_logical==bool(fixture.variable->isLogical),"Metadata changed unsupported array structure");
    for(const auto compatible:{false,true}){
        const auto result=compatible?reader.readFirstCompatible(path):reader.readNamed(path,"value");
        if(fixture.accepted){
            require(result.hasValue()&&result.value().values_column_major.size()==kValues.size()&&
                    std::memcmp(result.value().values_column_major.data(),kValues.data(),kValues.size()*sizeof(double))==0,
                    "Supported fixture values/negative zero changed");
        }else{
            require(!result.hasValue(),"Reader silently accepted unsupported fixture");
            require(result.error().code==fixture.error,"Wrong typed error for unsupported fixture: "+
                    std::string(errorName(result.error().code))+" "+result.error().detail.message);
            require(result.error().detail.stage==ErrorStage::Decode&&
                    result.error().detail.source_path==path.string()&&
                    result.error().detail.variable=="value","Reader error lost source/stage/variable");
        }
    }
    require(digest(path)==before,"Reader changed its input file");
    std::cout<<"SHA256 "<<before<<' '<<path.filename().string()<<'\n';
}
Bytes loadBytes(const fs::path& path) {
    std::ifstream in(path,std::ios::binary);
    require(in.is_open(),"Cannot read fixture bytes");
    return Bytes(std::istreambuf_iterator<char>(in),{});
}
void saveBytes(const fs::path& path,const Bytes& bytes) {
    require(!fs::exists(path),"Refuse damaged fixture overwrite");
    std::ofstream out(path,std::ios::binary);
    out.write(reinterpret_cast<const char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()));
    out.close();require(bool(out),"Cannot save synthetic damaged fixture");
}
void requireRejected(const fs::path& path) {
    const auto before=digest(path);
    io::MatResultReader reader;
    const auto named=reader.readNamed(path,"value");
    require(!named.hasValue(),"Damaged MAT accepted by named Reader");
    require(named.error().detail.source_path==path.string()&&named.error().detail.variable=="value",
            "Rejected named MAT lost its requested source/variable identity");
    require(!reader.readFirstCompatible(path).hasValue(),"Damaged MAT accepted by compatible Reader");
    require(!reader.scanFile(path,io::ScanMode::VerifyDecoded).hasValue(),
            "Full dictionary scan accepted a damaged MAT container");
    require(digest(path)==before,"Rejected input was modified");
    std::cout<<"SHA256 "<<before<<' '<<path.filename().string()<<'\n';
}
struct Cases {
    std::size_t total=0,failed=0;
    template<class Function> void run(const std::string& name,Function check) {
        ++total;
        try{check();std::cout<<"PASS "<<name<<'\n';}
        catch(const std::exception& error){++failed;std::cerr<<"FAIL "<<name<<": "<<error.what()<<'\n';}
    }
};
void damagedCases(const fs::path& directory,Cases& cases) {
    // 仅由本测试创建的新文件派生；绝不截断、复制或重写原始归档。
    constexpr std::size_t kMatHeaderBytes=128,kMissingPayloadBytes=8;
    for(const std::string mode:{"plain","zlib"}){
        auto bytes=loadBytes(directory/(mode+"-double.mat"));
        require(bytes.size()>kMatHeaderBytes+kMissingPayloadBytes,"Unexpected fixture length");
        bytes.resize(bytes.size()-kMissingPayloadBytes);
        const auto path=directory/(mode+"-truncated.mat");saveBytes(path,bytes);
        cases.run(mode+"-truncated",[&]{requireRejected(path);});
    }
    auto bytes=loadBytes(directory/"zlib-double.mat");
    bytes.back()^=1; // 故意破坏 zlib Adler-32；要求损坏数据不能成为成功输入。
    const auto checksum=directory/"zlib-checksum.mat";saveBytes(checksum,bytes);
    cases.run("zlib-checksum",[&]{requireRejected(checksum);});
    bytes.resize(kMatHeaderBytes/2);
    const auto header=directory/"short-header.mat";saveBytes(header,bytes);
    cases.run("short-header",[&]{requireRejected(header);});
}
void directoryCases(const fs::path& directory,Cases& cases) {
    io::MatResultReader reader;
    const auto first=real("first"),second=real("second");
    const auto multiple=directory/"multiple.mat";
    save(multiple,{first.get(),second.get()},MAT_COMPRESSION_ZLIB);
    cases.run("multiple-variable-selection",[&]{
        const auto before=digest(multiple);
        const auto metadata=reader.inspect(multiple);
        require(metadata.hasValue()&&metadata.value().size()==2,"Missing multiple-variable metadata");
        require(reader.readNamed(multiple,"first").hasValue()&&reader.readNamed(multiple,"second").hasValue(),
                "Named reader cannot select either variable");
        const auto compatible=reader.readFirstCompatible(multiple);
        require(!compatible.hasValue()&&compatible.error().code==MatErrorCode::AmbiguousVariable,
                "Compatible Reader guessed a variable from multiple candidates");
        require(digest(multiple)==before,"Selection changed multiple-variable input");
    });
    const auto v4=directory/"v4.mat";
    save(v4,{first.get()},MAT_COMPRESSION_NONE,MAT_FT_MAT4);
    cases.run("unsupported-v4",[&]{
        const auto before=digest(v4);
        const auto result=reader.readNamed(v4,"first");
        require(!result.hasValue()&&result.error().code==MatErrorCode::UnsupportedClassOrType,
                "Numeric Reader accepted unsupported MAT version");
        require(digest(v4)==before,"Version rejection changed input");
    });
    cases.run("missing-file-variable-and-invalid-name",[&]{
        for(const auto& path:{directory/"missing.mat",fs::path{},
                              directory/fs::path(std::string("invalid\0path",12))}){
            const auto missing=reader.readNamed(path,"value");
            require(!missing.hasValue()&&missing.error().code==MatErrorCode::MissingFile,"MissingFile mismatch");
            require(missing.error().detail.stage==ErrorStage::Read&&
                    missing.error().detail.source_path==path.string()&&
                    missing.error().detail.variable=="value","Unreadable path lost requested MAT identity");
        }
        for(const auto& name:{std::string(),std::string("no_such_variable"),std::string("bad\0name",8)}){
            const auto absent=reader.readNamed(multiple,name);
            require(!absent.hasValue()&&absent.error().code==MatErrorCode::MissingVariable,"MissingVariable mismatch");
            require(absent.error().detail.stage==ErrorStage::Read&&
                    absent.error().detail.source_path==multiple.string()&&
                    absent.error().detail.variable==name,"Rejected variable lost exact requested MAT identity");
        }
    });
    const auto empty=directory/"empty-directory.mat";
    save(empty,{},MAT_COMPRESSION_NONE);
    cases.run("empty-directory-request-identity",[&]{
        const auto before=digest(empty);
        const auto missing=reader.readNamed(empty,"value");
        require(!missing.hasValue()&&missing.error().code==MatErrorCode::InvalidMatHeader&&
                missing.error().detail.stage==ErrorStage::Read&&
                missing.error().detail.source_path==empty.string()&&
                missing.error().detail.variable=="value","Empty MAT directory lost requested variable identity");
        require(digest(empty)==before,"Empty directory rejection modified source");
    });
}
} // namespace
int main(int argc,char** argv) {
    try{
        require(argc==2,"Expected isolated build-tree sandbox");
        const fs::path root=fs::absolute(argv[1])/"中文 空格";
        fs::create_directories(root/"origin");
        auto allocated=io::RunDirectory::create(root);
        require(allocated.hasValue(),"Cannot allocate fixture RunId");
        auto run=std::move(allocated).value();
        const auto directory=run->outputRoot()/run->runId().text;
        std::cout<<"RunId="<<run->runId().text<<" fixture_dir="<<directory<<'\n';
        Cases cases;
        for(const auto compression:{MAT_COMPRESSION_NONE,MAT_COMPRESSION_ZLIB}){
            const std::string mode=compression==MAT_COMPRESSION_ZLIB?"zlib":"plain";
            for(const auto& fixture:fixtures()){
                const auto path=directory/(mode+"-"+fixture.name+".mat");
                save(path,{fixture.variable.get()},compression);
                cases.run(mode+"-"+fixture.name,[&]{checkFixture(path,fixture);});
            }
        }
        damagedCases(directory,cases);directoryCases(directory,cases);
        std::cout<<"Reader boundary scenarios="<<cases.total<<" failed="<<cases.failed<<'\n';
        return cases.failed==0?0:1;
    }catch(const std::exception& error){std::cerr<<"FAIL fixture setup: "<<error.what()<<'\n';return 1;}
}
