// 真实初始化CLI产物：独立MAT标签/值/次数复核；历史方向误差仅诊断。
// 新 SO(3) 旋转角与历史旋转轴方向角不同，不能用两者差值声明等价。
#include "egomotion/io/mat_result_reader.hpp"
#include "../support/archive_mat_reference.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

namespace {
using archive_reference::require;
namespace fs=std::filesystem;
constexpr std::size_t kInitializationFrames=35;
constexpr std::array<const char*,4> kFields{{"allemittransfehler","allemitrotfehler",
                                          "alleohnetransfehler","alleohnerotfehler"}};
constexpr const char* kMetricPolicy="adaptive-atan2-so3-geodesic-v1";
}
int main(int argc,char** argv) {
    try {
        require(argc==4,"Expected generated directory, readonly archive and new CSV path");
        const fs::path generated=argv[1],archive=argv[2],report=argv[3];
        require(!fs::exists(report),"Refuse to overwrite reference evidence");
        std::ofstream csv(report);require(csv.is_open(),"Cannot create reference report");
        csv<<"variable,matlab_row,metric_policy,valid_count,cpp_value,historical_l2_value,comparison,absolute_difference\n"
           <<std::setprecision(21);
        egomotion::io::MatResultReader reader;
        std::size_t checks=0,values=0;
        const auto marker=archive_reference::decodeIndependent(generated/"egomotion_metric_policy.mat");
        const auto marker_read=reader.readNamed(generated/"egomotion_metric_policy.mat","egomotion_metric_policy");
        require(marker.name=="egomotion_metric_policy"&&marker.rows==1&&marker.cols==4&&
                marker.storage_type==archive_reference::kDouble&&marker.values==std::vector<double>({1,1,1,1})&&
                marker_read.hasValue()&&marker_read.value().values_column_major==marker.values,
                "Initialization policy marker must independently decode to [1,1,1,1]");++checks;
        for(const auto* name:kFields) {
            const auto filename=std::string(name)+".mat";
            const auto validity_name="valid_"+std::string(name);
            const auto independent=archive_reference::decodeIndependent(generated/filename);
            const auto validity=archive_reference::decodeIndependent(generated/(validity_name+".mat"));
            const auto historical=archive_reference::decodeIndependent(archive/filename);
            const auto loaded=reader.readNamed(generated/filename,name);
            const auto validity_loaded=reader.readNamed(generated/(validity_name+".mat"),validity_name);
            require(loaded.hasValue(),"Production Reader rejected generated MAT");
            for(const auto* reference:{&independent,&historical}) {
                require(reference->name==name&&reference->rows==kInitializationFrames&&reference->cols==1&&
                        reference->values.size()==kInitializationFrames,
                        "Initialization name/35x1 shape changed");++checks;
            }
            require(independent.storage_type==archive_reference::kDouble,"New MAT must store real double payload");++checks;
            require(loaded.value().values_column_major.size()==kInitializationFrames,"Reader size changed");++checks;
            require(validity.name==validity_name&&validity.rows==kInitializationFrames&&validity.cols==1&&
                    validity.storage_type==archive_reference::kDouble&&validity.values.size()==kInitializationFrames&&
                    validity_loaded.hasValue()&&validity_loaded.value().values_column_major==validity.values,
                    "Validity companion must decode independently with the unchanged shape");++checks;
            const bool rotation=std::string(name).find("rotfehler")!=std::string::npos;
            long double maximum_difference=0;
            std::size_t comparable=0,missing=0;
            for(std::size_t row=0;row<kInitializationFrames;++row) {
                const double actual=independent.values[row],previous=historical.values[row];
                const double count=validity.values[row];
                const bool endpoint=row==0||row+1==kInitializationFrames;
                require(count==0||count==1,"Initialization validity count must be zero or one");++checks;
                require(endpoint?(actual==0&&count==0):
                    ((count==1&&std::isfinite(actual)&&actual>=0&&actual<=180)||(count==0&&std::isnan(actual))),
                    "Only explicitly missing interior angles may be NaN; structural endpoints remain zero");++checks;
                require(std::memcmp(&actual,&loaded.value().values_column_major[row],sizeof(double))==0,
                        "Independent MAT value differs from Reader");++checks;
                require(std::isfinite(previous),"Historical initialization reference is nonfinite");++checks;
                csv<<name<<','<<row+1<<','<<kMetricPolicy<<','<<count<<',';
                if(std::isnan(actual)){csv<<"missing";++missing;}else csv<<actual;
                csv<<','<<previous<<',';
                if(endpoint)csv<<"structural_endpoint,";
                else if(rotation)csv<<"different_rotation_metric,";
                else if(std::isnan(actual))csv<<"missing,";
                else {
                    const long double difference=std::abs(static_cast<long double>(actual)-previous);
                    maximum_difference=std::max(maximum_difference,difference);++comparable;
                    csv<<"direction_diagnostic_only,"<<difference;
                }
                csv<<'\n';
                ++values;
            }
            std::cout<<std::setprecision(21)<<name<<" missing="<<missing;
            if(rotation)std::cout<<" historical_l2_comparison=not_comparable_to_axis_angle";
            else std::cout<<" historical_l2_compared_values="<<comparable
                          <<" historical_l2_max_abs_difference="<<maximum_difference
                          <<" (diagnostic, not DEC-005 acceptance)";
            std::cout<<'\n';
        }
        csv.close();require(!csv.fail(),"Failed to persist all reference values");
        std::cout<<"Initialization independent MAT checks="<<checks<<" values="<<values<<" failures=0\n";
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
