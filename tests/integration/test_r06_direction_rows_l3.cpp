// R6: compare all 714 ordered source directions (2142 coordinates) with
// verification/geometry/source_direction_levels_0_2.v1.json.
// Reference was independently transcribed and cross-checked with Wolfram.
// This is an unapproved L3 candidate; do not report PASS before native CTest.
// No file writes or runtime generation of reference directions occur here.
#include "egomotion/sampling/directions.hpp"

#include <Eigen/Core>
#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
using namespace egomotion;
namespace fs=std::filesystem;

void require(bool ok,const std::string& why) {
    if(!ok)throw std::runtime_error(why);
}
std::string loadFixture(const fs::path& path) {
    std::ifstream in(path,std::ios::binary);
    require(static_cast<bool>(in),
            "Independent ordered-direction JSON is not readable: "+path.string());
    std::ostringstream buffer;
    buffer<<in.rdbuf();
    require(static_cast<bool>(in)||in.eof(),"Cannot read full direction fixture");
    auto result=buffer.str();
    require(result.find("\"schema\": \"egomotion.source-geometric-direction-reference.v1\"")
            !=std::string::npos,
            "Wrong or unexpectedly changed independent direction reference schema");
    return result;
}
class ArrayCursor {
public:
    ArrayCursor(const std::string& text,std::size_t begin,std::size_t end):
        text_(text),position_(begin),end_(end) {}
    void expect(char wanted){
        skip();
        require(position_<end_&&text_[position_]==wanted,
                "Independent direction JSON array syntax mismatch");
        ++position_;
    }
    double number() {
        skip();
        const std::size_t begin=position_;
        while(position_<end_){
            const char c=text_[position_];
            if ((c>='0'&&c<='9')||c=='.'||c=='-'||c=='+'||
                c=='e'||c=='E')++position_;
            else break;
        }
        require(position_>begin,"Direction reference coordinate is missing");
        std::istringstream value(text_.substr(begin,position_-begin));
        value.imbue(std::locale::classic());
        double parsed=0;
        value>>std::noskipws>>parsed;
        require(static_cast<bool>(value)&&
                value.peek()==std::char_traits<char>::eof()&&
                std::isfinite(parsed),"Direction reference coordinate invalid");
        return parsed;
    }
    void finish() {
        skip();
        require(position_==end_,"Independent direction array has trailing data");
    }
private:
    void skip(){
        while(position_<end_&&(text_[position_]==' '||text_[position_]=='\n'||
              text_[position_]=='\r'||text_[position_]=='\t'))++position_;
    }
    const std::string& text_;
    std::size_t position_;
    std::size_t end_;
};
std::size_t token(const std::string& text,std::string_view key,
                  std::size_t from,std::size_t to) {
    const auto found=text.find(key,from);
    require(found!=std::string::npos&&found<to,
            "Independent source JSON key is absent or outside its parent");
    return found;
}
void checkCase(const std::string& text,std::string_view mode,
               int level,const DirectionSet& computed,
               std::size_t bounds_start,std::size_t bounds_end) {
    const auto level_key="\""+std::to_string(level)+"\"";
    const auto level_start=token(text,level_key,bounds_start,bounds_end);
    const auto next_level=level<2?token(text,"\""+std::to_string(level+1)+"\"",
                                   level_start+level_key.size(),bounds_end):
                              bounds_end;
    const auto field=token(text,"\"directions_ordered\"",level_start,next_level);
    const auto opening=token(text,"[",field,next_level);
    const auto closing=text.find(']',opening);
    require(closing!=std::string::npos&&closing<next_level,
            "Independent ordered rows array cannot be delimited");
    ArrayCursor cursor(text,opening, next_level);
    cursor.expect('[');
    const Eigen::Index n=computed.directions.rows();
    const std::size_t count=static_cast<std::size_t>(n);
    const std::size_t expected_base=mode=="octa"?8:mode=="occluded"?6:20;
    const std::size_t factor=static_cast<std::size_t>(1)<<(2*level);
    require(count==expected_base*factor,
            "Sampler returned incorrect number of source leaf rows");
    for(std::size_t row=0;row<count;++row) {
        cursor.expect('[');
        for(int col=0;col<3;++col) {
            const double reference=cursor.number();
            const double actual=computed.directions(
                static_cast<Eigen::Index>(row),col);
            require(std::isfinite(actual)&&
                    std::abs(actual-reference)<=1e-12,
                    std::string(mode)+" level="+std::to_string(level)+
                    " row="+std::to_string(row)+" coord="+std::to_string(col)+
                    " differs from independently checked source order");
            if(col<2)cursor.expect(',');
        }
        cursor.expect(']');
        if(row+1<count)cursor.expect(',');
    }
    cursor.expect(']');
    // The object may include diagnostic metadata; do not mistake a second
    // directions_ordered array or an unconsumed reference row for success.
    (void)closing;
}
} // namespace

int main(int argc,char** argv) {
    require(argc==2,"Expected read-only independent JSON fixture path");
    const auto text=loadFixture(fs::path(argv[1]));
    const auto top=token(text,"\"levels\"",0,text.size());
    const std::array<std::string_view,3> keys{{"octa","occluded","icosa"}};
    sampling::OctahedralSampler octa;
    sampling::OccludedOctahedralSampler occluded;
    sampling::IcosahedralSampler icosa;
    std::size_t confirmed=0;
    for(std::size_t kind=0;kind<keys.size();++kind){
        const auto start=token(text,"\""+std::string(keys[kind])+"\"",top,text.size());
        const auto end=(kind+1<keys.size())
            ?token(text,"\""+std::string(keys[kind+1])+"\"",start+1,text.size())
            :text.size();
        for(int level=0;level<=2;++level) {
            auto computed=kind==0?octa.sample(level):
                          kind==1?occluded.sample(level):icosa.sample(level);
            require(computed.hasValue(),"Sampler failed on an approved subdivision");
            checkCase(text,keys[kind],level,computed.value(),start,end);
            confirmed+=static_cast<std::size_t>(computed.value().directions.rows());
        }
    }
    require(confirmed==714,
            "Full source-order comparison must cover exactly 714 independently stored rows");
}
