// DEC-R6-04(B)：缺测需显式掩码；旧Writer与普通保存仍严格拒绝非有限值。
#include "egomotion/io/run_mat_writer.hpp"
#include "egomotion/io/mat_result_reader.hpp"

#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
using namespace egomotion;
namespace fs=std::filesystem;
void require(bool value,const char* message) {
    if(!value)throw std::runtime_error(message);
}
DenseDoubleArray payload(std::string name,std::vector<double> values) {
    DenseDoubleArray result;
    result.info.name=std::move(name);result.info.dimensions={2,3};
    result.info.original_class_type=6;result.info.original_data_type=9;
    result.info.data_bytes=values.size()*sizeof(double);
    result.values_column_major=std::move(values);
    return result;
}
struct LegacyWriter final:IResultWriter {
    unsigned calls=0;
    Status<MatErrorCode> saveNewVariable(
        const RunId&,std::string_view,const DenseDoubleArray&) override {
        ++calls;return Status<MatErrorCode>::success();
    }
    std::vector<FileEvidence> fileEvidence()const override{return {};}
};
void fallback() {
    LegacyWriter writer;const RunId id{"synthetic"};
    const auto values=payload("metric",{1,2,3,4,5,6});
    require(writer.saveNewMaskedVariable(id,"metric",values,{1,1,1,1,1,1}).hasValue(),
            "All-defined fallback must delegate to the legacy Writer");
    for(const auto& mask:std::vector<std::vector<unsigned char>>{
            {},{1,1,1},{1,1,0,1,1,1},{1,1,2,1,1,1}})
        require(!writer.saveNewMaskedVariable(id,"metric",values,mask).hasValue(),
                "Legacy Writer must not claim partial or malformed mask support");
    require(writer.calls==1,"Unsupported masks must never reach the legacy save call");
}
void actualWriter(const fs::path& sandbox) {
    fs::create_directories(sandbox/"origin");
    auto reserved=io::RunDirectory::create(sandbox);
    require(reserved.hasValue(),"Cannot reserve isolated masked-MAT RunId");
    auto run=std::move(reserved).value();io::RunMatWriter writer(*run);
    const double missing=std::numeric_limits<double>::quiet_NaN();
    const double infinity=std::numeric_limits<double>::infinity();
    const auto before=payload("metric",{0.0,-0.0,missing,2.5,missing,-7.0});
    require(!writer.saveNewVariable(run->runId(),"metric",before).hasValue(),
            "Unmasked save must continue rejecting NaN");
    require(writer.saveNewMaskedVariable(run->runId(),"metric",before,
                                        {1,1,0,1,0,1}).hasValue(),
            "Declared NaNs must traverse the real safe publication path");
    const auto decoded=io::MatResultReader{}.readNamed(
        run->outputRoot()/run->runId().text/"metric.mat","metric");
    require(decoded.hasValue()&&decoded.value().info.dimensions==before.info.dimensions&&
        decoded.value().values_column_major.size()==before.values_column_major.size()&&
        std::memcmp(decoded.value().values_column_major.data(),
                    before.values_column_major.data(),before.info.data_bytes)==0,
        "Readback must preserve every NaN/negative-zero/finite binary64 bit");
    require(writer.fileEvidence().size()==1&&writer.fileEvidence()[0].durably_confirmed,
            "Only the successful masked MAT may have a durable receipt");
    for(unsigned scenario=0;scenario<7;++scenario) {
        const std::string name="rejected_"+std::to_string(scenario);
        auto values=payload(name,{1,2,missing,4,5,6});
        std::vector<unsigned char> mask{1,1,0,1,1,1};
        switch(scenario) {
        case 0:mask.pop_back();break;
        case 1:mask[2]=2;break;
        case 2:mask[2]=1;break;
        case 3:values.values_column_major[2]=0.0;break;
        case 4:values.values_column_major[2]=infinity;break;
        case 5:values.values_column_major[0]=-infinity;break;
        case 6:mask.push_back(1);break;
        }
        require(!writer.saveNewMaskedVariable(run->runId(),name,values,mask).hasValue(),
                "Mask/value mismatch or infinity must be rejected before publication");
        require(!fs::exists(run->outputRoot()/run->runId().text/(name+".mat")),
                "Rejected masked data must not create a final MAT");
        require(writer.fileEvidence().size()==1,"Rejected data must not gain a receipt");
    }
    for(const auto& item:fs::directory_iterator(run->outputRoot()/run->runId().text))
        require(item.path().filename()=="metric.mat","Masked publication must clean its own staging name");
}
} // namespace
int main(int argc,char** argv) {
    try {
        require(argc==2,"Expected isolated build-tree sandbox");
        fallback();actualWriter(fs::absolute(argv[1]));
        std::cout<<"PASS declared missing MAT values and strict legacy Writer fallback\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<error.what()<<'\n';return 1;
    }
}
