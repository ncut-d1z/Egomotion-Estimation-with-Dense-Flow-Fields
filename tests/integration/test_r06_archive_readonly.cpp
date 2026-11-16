// R6 read-only L2 archive schema smoke test. This never creates, modifies,
// renames or deletes anything beneath origin/. It is NOT an independent
// numerical Golden and does NOT compare archived content to reference values.
#include "egomotion/io/mat_result_reader.hpp"
#include "egomotion/io/sha256.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <array>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
using namespace egomotion;
namespace fs=std::filesystem;
void require(bool ok,const std::string& message){
    if(!ok)throw std::runtime_error(message);
}
struct File {
    explicit File(const fs::path& p):fd(::open(p.c_str(),O_RDONLY|O_CLOEXEC|O_NOFOLLOW)) {}
    ~File(){if(fd>=0)::close(fd);}
    File(const File&)=delete;
    File& operator=(const File&)=delete;
    int fd=-1;
};
using Shape=std::pair<std::size_t,std::size_t>;
} // namespace

int main(int argc,char** argv) {
    if(argc!=2)throw std::invalid_argument("Expected archive folder under origin/");
    const fs::path archive=fs::absolute(argv[1]);
    require(fs::exists(archive)&&fs::is_directory(archive)&&
            !fs::is_symlink(fs::symlink_status(archive)),
            "Read-only original MATLAB archive directory is unavailable");
    const io::MatResultReader reader;
    std::map<Shape,std::size_t> observed;
    std::size_t files=0;
    for(const auto& entry:fs::recursive_directory_iterator(archive)) {
        const auto path=entry.path();
        if(path.extension()!=".mat")continue;
        require(!fs::is_symlink(entry.symlink_status())&&
                entry.is_regular_file(),
                "Archived MAT path must be a regular non-symlink file");
        const auto metadata=reader.inspect(path);
        require(metadata.hasValue()&&metadata.value().size()==1,
                "Each historical MAT must expose one decodable metadata variable: "+path.string());
        const auto& info=metadata.value()[0];
        require(info.dimensions.size()==2&&
                info.original_class_type==6&&info.original_data_type==9&&
                !info.is_complex&&!info.is_logical,
                "Historical MAT metadata must be a real two-dimensional double: "+path.string());
        const auto decoded=reader.readFirstCompatible(path);
        require(decoded.hasValue()&&decoded.value().info.name==info.name&&
                decoded.value().info.dimensions==info.dimensions,
                "Single-variable MAT numerical decoding/metadata mismatch: "+path.string());
        const std::size_t rows=info.dimensions[0],cols=info.dimensions[1];
        require(rows>0&&cols>0&&
                decoded.value().values_column_major.size()==rows*cols,
                "Archived values must own exactly rows*columns doubles: "+path.string());
        File source(path);
        require(source.fd>=0,"Historical MAT cannot be opened read-only: "+path.string());
        const auto digest=io::sha256Fd(source.fd);
        require(digest.hasValue()&&digest.value().size()==64,
                "Archived MAT SHA-256 computation failed: "+path.string());
        // A checksum computed from a file is not an independent expected SHA.
        ++observed[{rows,cols}];
        ++files;
    }
    constexpr std::array<std::pair<Shape,std::size_t>,5> reference{{
        {{10,1},80},{{10,3},480},{{35,1},4},{{600,1},14},{{600,4},4}
    }};
    require(files==582,
            "Historical source archive MAT file count differs from the 582-file audited inventory");
    require(observed.size()==reference.size(),
            "Archived MAT shape set differs from the audited five-shape inventory");
    for(const auto& expected:reference) {
        const auto it=observed.find(expected.first);
        require(it!=observed.end()&&it->second==expected.second,
                "Archived MAT shape frequency differs from the previous read-only inventory");
    }
    std::cout<<"L2 archive schema read-only smoke: "<<files
             <<" MAT decoded; five audited shape classes; SHA-256 computed per file"
             <<" (no independent content comparison)\n";
}
