// 仅合成合同数据；真实独立 RunDirectory/MAT Writer/Manifest/RNG，非完整实验。
#include "egomotion/contracts/diagnostics.hpp"
#include "egomotion/experiments/run_context.hpp"
#include "egomotion/io/mat_result_reader.hpp"
#include "egomotion/io/run_mat_writer.hpp"
#include "egomotion/io/run_text_publisher.hpp"
#include "egomotion/io/sha256.hpp"
#include "egomotion/sampling/random.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
using namespace egomotion;
using experiments::ExperimentKind;
namespace fs=std::filesystem;
constexpr std::uint32_t kSeedA=7,kSeedB=11;
std::size_t checks=0,random_samples=0,read_values=0;
void require(bool pass,const std::string& message) {
    ++checks;
    if (!pass) throw std::runtime_error(message);
}
std::string digest(const fs::path& path) {
    const int fd=::open(path.c_str(),O_RDONLY|O_NOFOLLOW|O_CLOEXEC);
    require(fd>=0,"Cannot open test artifact for SHA256");
    const auto hash=io::sha256Fd(fd); ::close(fd);
    require(hash.hasValue(),"Cannot hash test artifact");
    return hash.value();
}
std::string textFile(const fs::path& path) {
    std::ifstream input(path,std::ios::binary);
    require(input.is_open(),"Missing manifest");
    return {std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>()};
}
struct FailSecondWrite final:IResultWriter {
    io::RunMatWriter& real;
    bool inject=false;
    std::size_t calls=0;
    explicit FailSecondWrite(io::RunMatWriter& writer):real(writer) {}
    Status<MatErrorCode> saveNewVariable(const RunId& id,std::string_view name,
                                        const DenseDoubleArray& values) override {
        ++calls;
        if (inject&&calls==2) {
            ErrorDetail detail; detail.stage=ErrorStage::Output; detail.run_id=id;
            detail.variable=std::string(name); detail.message="Interleaved B second-write fault";
            return Status<MatErrorCode>::failure({MatErrorCode::WriteFailed,std::move(detail)});
        }
        return real.saveNewVariable(id,name,values);
    }
    std::vector<FileEvidence> fileEvidence() const override { return real.fileEvidence(); }
};
struct Box {
    std::unique_ptr<io::RunDirectory> directory;
    std::unique_ptr<io::RunMatWriter> writer;
    std::unique_ptr<FailSecondWrite> forwarding;
    std::unique_ptr<io::RunTextPublisher> publisher;
    std::unique_ptr<RunContext> context;
    fs::path path;
    Box(const fs::path& root,const std::string& sha,bool first) {
        auto made=io::RunDirectory::create(root);
        require(made.hasValue(),"Cannot reserve independent RunId");
        directory=std::move(made).value();
        path=directory->outputRoot()/directory->runId().text;
        writer=std::make_unique<io::RunMatWriter>(*directory);
        forwarding=std::make_unique<FailSecondWrite>(*writer);
        forwarding->inject=!first;
        publisher=std::make_unique<io::RunTextPublisher>(*directory);
        require(publisher->publishNew("SYNTHETIC_TEST_ONLY.txt",
            "Interleaved RunContext contract fixtures, not scientific experiment results.\n").hasValue(),
            "Cannot mark synthetic output");
        const auto kind=first?ExperimentKind::AdaptiveInitialization:ExperimentKind::AdaptiveUpdateRates;
        const auto seed=first?kSeedA:kSeedB;
        // 原脚本身份仅标记对应 save schema，不声称执行了这些脚本。
        const std::string blob=first?"a3154ae391be82ccf48c9f3a03c55109dde85f65":
                                     "4629901c1233cdee5d6d22c917603720ffd1fab4";
        context=std::make_unique<RunContext>(directory->runId(),
            std::make_unique<sampling::Mt19937RandomSource>(seed),*forwarding,*publisher,
            directory->outputRoot(),SourceProvenance{sha,blob,seed},kind,first?1:8);
        std::cout<<(first?"A":"B")<<" RunId="<<directory->runId().text<<" seed="<<seed<<" path="<<path<<'\n';
    }
};
using Files=std::map<std::string,std::string>;
Files files(const Box& box) {
    Files result;
    for (const auto& entry:fs::directory_iterator(box.path)) {
        require(entry.is_regular_file(),"Unexpected artifact type");
        result.emplace(entry.path().filename().string(),digest(entry.path()));
    }
    return result;
}
std::string state(RunContext& run) {
    const auto& d=run.manifestDraft();
    std::ostringstream result;
    result<<run.runId().text<<' '<<run.approvedOutputRoot()<<' '<<static_cast<int>(run.experimentKind())
          <<' '<<run.provenance().repository_commit_sha<<' '<<run.provenance().matlab_source_blob_sha
          <<' '<<run.provenance().rng_seed.value_or(0)<<' '<<static_cast<int>(d.status)
          <<' '<<d.attempted<<' '<<d.completed<<' '<<d.valid<<' '<<d.failed<<' '<<d.undefined
          <<' '<<d.expected_repetitions;
    for (const auto& name:d.files_writer_confirmed) result<<' '<<name;
    for (const auto& error:d.errors) result<<' '<<jsonFailure(error,6);
    for (const auto& receipt:run.resultWriter().fileEvidence())
        result<<' '<<receipt.variable<<' '<<receipt.filename<<' '<<receipt.sha256
              <<' '<<receipt.size_bytes<<' '<<receipt.durably_confirmed;
    return result.str();
}
std::map<std::string,DenseDoubleArray> fields(bool first) {
    const auto kind=first?ExperimentKind::AdaptiveInitialization:ExperimentKind::AdaptiveUpdateRates;
    std::map<std::string,DenseDoubleArray> result;
    std::size_t field=0;
    for (const auto& name:experiments::currentSaveFields(kind)) {
        DenseDoubleArray value;
        const std::size_t rows=first?35:600,cols=!first&&name!="akrot"?4:1;
        value.info.name=name; value.info.dimensions={rows,cols};
        value.info.original_class_type=6; value.info.original_data_type=9;
        value.info.data_bytes=rows*cols*sizeof(double);
        for (std::size_t i=0;i<rows*cols;++i)
            value.values_column_major.push_back((first?1.0:8.0)+static_cast<double>(field)/16+static_cast<double>(i)/1024);
        result.emplace(name,std::move(value)); ++field;
    }
    return result;
}
void randomPair(Box& a,Box& b,sampling::Mt19937RandomSource& ref_a,sampling::Mt19937RandomSource& ref_b) {
    auto compare=[](IRandomSource& actual,IRandomSource& expected,bool normal) {
        const auto x=normal?actual.normal():actual.uniform();
        const auto y=normal?expected.normal():expected.uniform();
        require(x.hasValue()&&y.hasValue()&&x.value()==y.value(),"Per-run RNG sequence changed");
        ++random_samples;
    };
    for (std::size_t i=0;i<8;++i) {
        compare(a.context->randomSource(),ref_a,i%2==0);
        compare(b.context->randomSource(),ref_b,i%3==0);
        compare(b.context->randomSource(),ref_b,true);
        compare(a.context->randomSource(),ref_a,false);
    }
}
void readBack(const Box& box,const std::map<std::string,DenseDoubleArray>& expected,std::size_t count) {
    const auto names=experiments::currentSaveFields(box.context->experimentKind());
    require(box.writer->fileEvidence().size()==count,"Wrong per-run receipt count");
    for (std::size_t i=0;i<names.size();++i) {
        const auto path=box.path/(names[i]+".mat");
        require(fs::exists(path)==(i<count),"Other run filled an unattempted field");
        if (i>=count) continue;
        const auto actual=io::MatResultReader{}.readNamed(path,names[i]);
        require(actual.hasValue(),"Real MAT readback failed");
        const auto& target=expected.at(names[i]);
        require(actual.value().info.dimensions==target.info.dimensions&&
                actual.value().values_column_major==target.values_column_major,"Cross-run MAT values or dimensions");
        read_values+=target.values_column_major.size();
        const auto& receipt=box.writer->receipts().at(names[i]);
        require(receipt.durably_confirmed&&receipt.sha256==digest(path),"Receipt must identify own durable MAT");
    }
}
void verify(const fs::path& root,const std::string& sha) {
    fs::create_directories(root/"origin"); // 全新测试沙箱的空标记，绝不操作真实原始目录。
    Box a(root,sha,true),b(root,sha,false);
    require(a.directory->runId().text!=b.directory->runId().text,"RunIds must be independent");
    sampling::Mt19937RandomSource ref_a(kSeedA),ref_b(kSeedB);
    randomPair(a,b,ref_a,ref_b);
    const auto b_initial=state(*b.context);
    require(a.context->noteCompletedMemory(1,33).hasValue(),"Synthetic A memory completion");
    require(state(*b.context)==b_initial,"A memory completion changed B");
    const auto a_ready=state(*a.context);
    const auto a_files=files(a);
    b.context->noteAttempt(); b.context->noteAttempt(); b.context->noteAttempt();
    b.context->noteValid(); b.context->noteValid(); b.context->noteUndefined();
    const auto a_data=fields(true),b_data=fields(false);
    const auto failed=b.context->writeCurrentResults(b_data);
    require(!failed.hasValue()&&failed.error().code==ExperimentErrorCode::OutputFailed,"B write fault did not fire");
    require(b.forwarding->calls==2&&b.writer->receipts().size()==1,"B must retain exactly one real MAT");
    require(state(*a.context)==a_ready&&files(a)==a_files,"B counters/write failure contaminated A");
    require(!a.context->writeCurrentResults(b_data).hasValue(),"A accepted B output schema");
    require(a.forwarding->calls==0&&state(*a.context)==a_ready&&files(a)==a_files,"Schema rejection changed A");
    const auto b_partial=state(*b.context);
    const auto b_partial_files=files(b);
    require(a.context->writeCurrentResults(a_data).hasValue(),"A cannot write after B failure");
    require(state(*b.context)==b_partial&&files(b)==b_partial_files,"A writes changed B");
    const auto a_written=state(*a.context);
    const auto a_written_files=files(a);
    require(b.context->publishFailureRecord(failed.error()).hasValue(),"B truthful Incomplete publication failed");
    require(state(*a.context)==a_written&&files(a)==a_written_files,"B audit publication changed A");
    randomPair(a,b,ref_a,ref_b);
    const auto b_final=state(*b.context);
    const auto b_final_files=files(b);
    require(!b.context->publishCompleteOutcome().hasValue(),"Incomplete B became Complete");
    const auto complete=a.context->publishCompleteOutcome();
    require(complete.hasValue()&&complete.value().status==RunStatus::Complete&&
            complete.value().run_id.text==a.directory->runId().text&&complete.value().valid==33,
            "A must independently publish Complete");
    require(state(*b.context)==b_final&&files(b)==b_final_files,"A Complete changed B");
    const auto a_final=state(*a.context);
    const auto a_final_files=files(a);
    require(!a.context->publishCompleteOutcome().hasValue()&&
            !a.context->writeCurrentResults(a_data).hasValue(),"Duplicate publication succeeded");
    require(!b.writer->saveNewVariable(a.directory->runId(),"akrot",b_data.at("akrot")).hasValue(),
            "B Writer accepted foreign RunId");
    require(state(*a.context)==a_final&&files(a)==a_final_files&&
            state(*b.context)==b_final&&files(b)==b_final_files,"Rejected operations changed independent runs");
    randomPair(a,b,ref_a,ref_b);
    readBack(a,a_data,4); readBack(b,b_data,1);
    const auto ma=textFile(a.path/"manifest.json"),mb=textFile(b.path/"manifest.json");
    require(ma.find("\"completeness\":\"Complete\"")!=std::string::npos&&
            ma.find("\"algorithm_rng_seed\":7")!=std::string::npos&&ma.find(b.directory->runId().text)==std::string::npos,
            "A Manifest mixed another run");
    require(mb.find("\"completeness\":\"Incomplete\"")!=std::string::npos&&
            mb.find("\"algorithm_rng_seed\":11")!=std::string::npos&&mb.find(a.directory->runId().text)==std::string::npos&&
            mb.find("WriteFailed")!=std::string::npos&&mb.find("Interleaved B second-write fault")!=std::string::npos,
            "B Manifest lost its own failure chain");
    require(b.context->manifestDraft().attempted==3&&b.context->manifestDraft().valid==2&&
            b.context->manifestDraft().undefined==1&&b.context->manifestDraft().failed==1&&
            b.context->manifestDraft().completed==0,"B count ledger changed");
    require(a_final_files.size()==6&&b_final_files.size()==3,"Unexpected files in isolated RunIds");
    require(digest(a.path/"allemittransfehler.mat")!=digest(b.path/"allemittransfehler.mat"),
            "Same filename unexpectedly points to same content");
    std::cout<<"A manifest_sha256="<<digest(a.path/"manifest.json")
             <<" B manifest_sha256="<<digest(b.path/"manifest.json")<<'\n';
}
}  // namespace

int main(int argc,char** argv) {
    try {
        require(argc==3,"Expected sandbox root and tested source SHA");
        verify(fs::path(argv[1])/"交替 实例",argv[2]);
        std::cout<<"RunContext interleaving checks="<<checks<<" rng_samples="<<random_samples
                 <<" read_values="<<read_values<<" failures=0\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr<<"RunContext interleaving checks="<<checks<<" failure="<<error.what()<<'\n';
        return 1;
    }
}
