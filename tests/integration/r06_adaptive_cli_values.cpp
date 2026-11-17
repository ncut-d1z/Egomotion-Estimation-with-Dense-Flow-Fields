// 真实初始化CLI产物：独立MAT标签/值复核；历史L2只报告差异，不自定验收阈值。
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

namespace {
using archive_reference::require;
namespace fs=std::filesystem;
constexpr std::size_t kInitializationFrames=35;
constexpr std::array<const char*,4> kFields{{"allemittransfehler","allemitrotfehler",
                                          "alleohnetransfehler","alleohnerotfehler"}};
}
int main(int argc,char** argv) {
    try {
        require(argc==4,"Expected generated directory, readonly archive and new CSV path");
        const fs::path generated=argv[1],archive=argv[2],report=argv[3];
        require(!fs::exists(report),"Refuse to overwrite reference evidence");
        std::ofstream csv(report);require(csv.is_open(),"Cannot create reference report");
        csv<<"variable,matlab_row,cpp_value,historical_l2_value,absolute_difference\n"<<std::setprecision(21);
        egomotion::io::MatResultReader reader;
        std::size_t checks=0,values=0;
        for(const auto* name:kFields) {
            const auto filename=std::string(name)+".mat";
            const auto independent=archive_reference::decodeIndependent(generated/filename);
            const auto historical=archive_reference::decodeIndependent(archive/filename);
            const auto loaded=reader.readNamed(generated/filename,name);
            require(loaded.hasValue(),"Production Reader rejected generated MAT");
            for(const auto* reference:{&independent,&historical}) {
                require(reference->name==name&&reference->rows==kInitializationFrames&&reference->cols==1&&
                        reference->values.size()==kInitializationFrames,
                        "Initialization name/35x1 shape changed");++checks;
            }
            require(independent.storage_type==archive_reference::kDouble,"New MAT must store real double payload");++checks;
            require(loaded.value().values_column_major.size()==kInitializationFrames,"Reader size changed");++checks;
            long double maximum_difference=0;
            for(std::size_t row=0;row<kInitializationFrames;++row) {
                const double actual=independent.values[row],previous=historical.values[row];
                require(std::isfinite(actual)&&actual>=0&&actual<=180,"Invalid generated angle error");++checks;
                require(std::memcmp(&actual,&loaded.value().values_column_major[row],sizeof(double))==0,
                        "Independent MAT value differs from Reader");++checks;
                require(std::isfinite(previous),"Historical initialization reference is nonfinite");++checks;
                if(row==0||row+1==kInitializationFrames){require(actual==0,"Source endpoint zero changed");++checks;}
                const long double difference=std::abs(static_cast<long double>(actual)-previous);
                maximum_difference=std::max(maximum_difference,difference);
                csv<<name<<','<<row+1<<','<<actual<<','<<previous<<','<<difference<<'\n';
                ++values;
            }
            std::cout<<std::setprecision(21)<<name<<" historical_l2_max_abs_difference="<<maximum_difference
                     <<" (diagnostic, not DEC-005 acceptance)\n";
        }
        csv.close();require(!csv.fail(),"Failed to persist all reference values");
        std::cout<<"Initialization independent MAT checks="<<checks<<" values="<<values<<" failures=0\n";
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
