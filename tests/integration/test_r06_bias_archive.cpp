// 历史Bias曲线的独立参考：原MAT标签解码 + long double标量均值。
// 不运行MATLAB，不用生产Reader/Analyzer生成参考，不运行新的Bias实验。
#include "egomotion/analysis/result_analyzer.hpp"
#include "egomotion/io/mat_result_reader.hpp"
#include "egomotion/io/sha256.hpp"
#include "../support/archive_mat_reference.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace egomotion;
namespace fs=std::filesystem;
using archive_reference::require;
using archive_reference::Reference;
// graphs_bias_test.m及固定归档：40次，10行，3列，四组被绘制的y。
constexpr std::size_t kRuns=40,kRows=10,kColumns=3,kKinds=4,kFirstRow=3;
constexpr const char* kAnalysisBlob="112aa9eca1fa9728f493d31fc8e59620a8445230";
// 沿用既有R6 Bias回归余量；不是DEC-005正式数值阈值。
constexpr long double kRegressionTolerance=1e-9L;
constexpr std::array<const char*,kKinds> kYNames{{"ywerte3","ywerte4","ywerte5","ywerte6"}};
std::size_t checks=0,failures=0,files=0,input_values=0,curve_points=0,contributors=0;
long double maximum_error=0;
void check(bool ok,const std::string& why) {
    ++checks;
    if(!ok){++failures;std::cerr<<why<<'\n';}
}
void closeTo(double actual,long double expected,const std::string& why) {
    const auto delta=std::abs(static_cast<long double>(actual)-expected);
    maximum_error=std::max(maximum_error,delta);
    check(std::isfinite(actual)&&std::isfinite(expected)&&delta<=kRegressionTolerance,why);
}
std::string sha256(const fs::path& path) {
    const int fd=::open(path.c_str(),O_RDONLY|O_CLOEXEC|O_NOFOLLOW);
    require(fd>=0,"Cannot open read-only input for hashing: "+path.string());
    const auto hash=io::sha256Fd(fd);
    ::close(fd);
    require(hash.hasValue(),"Cannot hash input: "+path.string());
    return hash.value();
}
std::map<std::string,std::string> identities(const fs::path& inventory) {
    std::ifstream in(inventory);
    require(in.is_open(),"Cannot read fixed MAT identity inventory");
    std::map<std::string,std::string> result;
    std::string line;
    while(std::getline(in,line)) {
        if(line.empty()||line.front()=='#')continue;
        std::istringstream fields(line);
        std::string hash,name,path,extra;
        std::size_t storage=0,rows=0,cols=0;
        require(bool(fields>>hash>>storage>>rows>>cols>>name>>path)&&!(fields>>extra),
                "Invalid fixed identity row");
        require(hash.size()==64&&!fs::path(path).is_absolute()&&path.find("..")==std::string::npos&&
                result.emplace(path,hash).second,"Invalid or duplicated fixed file identity");
    }
    require(result.size()==582,"Unexpected full inventory count");
    return result;
}
struct ReferenceRun {
    Reference x;
    std::array<Reference,kKinds> y;
};
struct GroupReference {
    std::array<ReferenceRun,kRuns> runs;
    std::map<fs::path,std::string> input_hashes;
};
Reference readReference(const fs::path& archive,const fs::path& path,
                        const std::string& name,std::size_t columns,
                        const std::map<std::string,std::string>& inventory,
                        GroupReference& group) {
    const auto relative=path.lexically_relative(archive).generic_string();
    const auto expected=inventory.find(relative);
    require(expected!=inventory.end(),"Input missing from fixed inventory: "+relative);
    const auto hash=sha256(path);
    require(hash==expected->second,"Original SHA-256 mismatch: "+relative);
    group.input_hashes.emplace(path,hash);
    auto ref=archive_reference::decodeIndependent(path);
    require(ref.name==name&&ref.rows==kRows&&ref.cols==columns,
            "Independent input shape/name mismatch: "+relative);
    for(double value:ref.values)require(std::isfinite(value),"Nonfinite Bias reference");
    ++files;input_values+=ref.values.size();
    return ref;
}
GroupReference readGroup(const fs::path& archive,const fs::path& group,
                          const std::map<std::string,std::string>& inventory) {
    GroupReference result;
    for(std::size_t k=0;k<kRuns;++k) {
        const auto root=group/std::to_string(k+1);
        auto& run=result.runs[k];
        run.x=readReference(archive,root/"xwerte.mat","xwerte",1,inventory,result);
        for(std::size_t kind=0;kind<kKinds;++kind)
            run.y[kind]=readReference(archive,root/(std::string(kYNames[kind])+".mat"),
                                      kYNames[kind],kColumns,inventory,result);
    }
    return result;
}
struct RecordingReader final:IResultReader {
    io::MatResultReader reader;
    mutable std::vector<std::pair<fs::path,std::string>> calls;
    Result<std::vector<MatVariableInfo>,MatErrorCode> inspect(const fs::path& path)const override {
        return reader.inspect(path);
    }
    Result<DenseDoubleArray,MatErrorCode> readNamed(const fs::path& path,std::string_view name)const override {
        calls.emplace_back(path,name);
        return reader.readNamed(path,name);
    }
    Result<DenseDoubleArray,MatErrorCode> readFirstCompatible(const fs::path& path)const override {
        throw std::logic_error("Bias must read the exact named source variable: "+path.string());
    }
};
void provenance(const AnalysisInput& input,const fs::path& root,
                const std::string& name,std::size_t run,const std::string& role) {
    ++contributors;
    check(input.path==root/std::to_string(run)/(name+".mat")&&input.variable==name&&
          input.matlab_run_ordinal==run&&input.role==role,"Incorrect curve contributor identity");
    // Reader当前未绑定稳定读取快照的原文件摘要；独立测试摘要不能填进生产对象。
    check(!input.mat_sha256,"Analyzer invented a digest without Reader snapshot binding");
}
void curveMetadata(const AnalysisSeries& curve,const std::string& name) {
    check(curve.name==name&&curve.panel=="bias-loglog","Incorrect Bias curve identity");
    check(curve.source_begin_matlab==kFirstRow&&curve.source_end_matlab==kRows,
          "Incorrect source MATLAB plotting range");
    check(curve.x_scale=="log"&&curve.y_scale=="log"&&
          curve.x_unit=="original MATLAB xwerte","Incorrect Bias axes");
    check(curve.smoothing_half_window==0&&curve.smoothing_full.empty()&&
          curve.helper_source_blob_shas.empty(),"Bias must not claim smoothing helpers");
    require(curve.x.size()==kRows-kFirstRow+1&&curve.y.size()==curve.x.size(),
            "Incorrect historical curve length");
}
void compareCurves(const AnalysisDataset& data,const GroupReference& ref,const fs::path& root) {
    check(data.source_analysis_blob==kAnalysisBlob,"Wrong original graph source identity");
    require(data.curves.size()==1+kKinds*kColumns,"Expected all thirteen source curves");
    const auto& last_x=ref.runs.back().x.values;
    for(std::size_t index=0;index<data.curves.size();++index) {
        const auto& curve=data.curves[index];
        const bool theoretical=index==0;
        const auto kind=theoretical?0:(index-1)/kColumns,col=theoretical?0:(index-1)%kColumns;
        curveMetadata(curve,theoretical?"100/sqrt(xwerte_last)":
                      std::string(kYNames[kind])+".column"+std::to_string(col+1));
        check(curve.y_unit==(theoretical?"reference scaled error":
              "scaled translation/rotation vector error"),"Wrong metric unit");
        require(curve.inputs.size()==(theoretical?1:kRuns+1),"Missing curve contributors");
        if(!theoretical)
            for(std::size_t k=0;k<kRuns;++k)
                provenance(curve.inputs[k],root,kYNames[kind],k+1,"y_member");
        provenance(curve.inputs.back(),root,"xwerte",kRuns,"x_last");
        for(std::size_t plotted=0;plotted<curve.y.size();++plotted) {
            const auto row=plotted+kFirstRow-1;
            check(curve.x[plotted]==last_x[row],"Archive x must come from last stored run");
            long double expected=100.0L/std::sqrt(static_cast<long double>(last_x[row]));
            if(!theoretical) {
                std::array<long double,kRuns> samples{};
                for(std::size_t k=0;k<kRuns;++k)samples[k]=ref.runs[k].y[kind].values[col*kRows+row];
                expected=std::accumulate(samples.begin(),samples.end(),0.0L)/kRuns;
            }
            closeTo(curve.y[plotted],expected,"Historical scalar mean/reference differs");
            ++curve_points;
            std::cout<<"POINT group="<<root.filename().string()<<" id="<<curve.name
                     <<" matlab_row="<<row+1<<" x="<<curve.x[plotted]<<" actual="<<curve.y[plotted]
                     <<" independent="<<expected<<" abs_error="
                     <<std::abs(static_cast<long double>(curve.y[plotted])-expected)<<'\n';
        }
    }
}
void verifyGroup(const fs::path& archive,const std::string& name,
                 const std::map<std::string,std::string>& inventory) {
    const auto root=archive/"bias_test/produced_data"/name;
    const auto ref=readGroup(archive,root,inventory);
    RecordingReader reader;
    analysis::MatResultAnalyzer analyzer;
    const auto actual=analyzer.analyze(AnalysisKind::Bias,root,reader);
    require(actual.hasValue(),"Historical Bias analysis failed: "+name);
    require(reader.calls.size()==kRuns*(1+kKinds),"Unexpected Bias read count");
    for(std::size_t k=0;k<kRuns;++k)
        for(std::size_t field=0;field<=kKinds;++field) {
            const std::string variable=field?kYNames[field-1]:"xwerte";
            const auto& call=reader.calls[k*(1+kKinds)+field];
            check(call.first==root/std::to_string(k+1)/(variable+".mat")&&call.second==variable,
                  "Reader order/named-variable differs from original source");
        }
    compareCurves(actual.value(),ref,root);
    // 区分归档与当前生成公式：比例噪声归档行3是128，不能替换成6*4^2=96。
    check(actual.value().curves[0].x.front()==(name=="proportional_error"?128.0:96.0),
          "Historical x was replaced with a generated sampling formula");
    for(const auto& pair:ref.input_hashes)
        check(sha256(pair.first)==pair.second,"Original MAT bytes changed after analysis");
    std::cout<<"GROUP "<<name<<" runs="<<kRuns<<" curves="<<actual.value().curves.size()
             <<" files="<<ref.input_hashes.size()<<" checks_so_far="<<checks<<'\n';
}
} // namespace
int main(int argc,char** argv) {
    static_assert(sizeof(double)==8&&std::numeric_limits<double>::is_iec559,
                  "Historical reference needs IEEE binary64");
    static_assert(std::numeric_limits<long double>::digits>std::numeric_limits<double>::digits,
                  "Independent accumulation requires wider long double on this target");
    try {
        require(argc==3,"Expected original archive root and fixed SHA-256 inventory");
        std::cout<<std::setprecision(std::numeric_limits<long double>::max_digits10);
        const auto inventory=identities(argv[2]);
        for(const std::string name:{"occluded_regions","proportional_error"})
            verifyGroup(fs::absolute(argv[1]),name,inventory);
        check(files==400&&input_values==10400&&curve_points==208&&contributors==986,
              "Historical coverage count changed");
        std::cout<<"Historical Bias checks="<<checks<<" files="<<files<<" input_values="<<input_values
                 <<" xy_pairs="<<curve_points<<" contributors="<<contributors
                 <<" maximum_abs_error="<<maximum_error<<" failures="<<failures<<'\n';
        return failures==0?0:1;
    }catch(const std::exception& e){std::cerr<<"Historical Bias failed: "<<e.what()<<'\n';return 1;}
}
