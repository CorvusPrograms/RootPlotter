#include <sol/sol.hpp>
#include <unordered_set>
#include <string>
#include <optional>
#include <vector>
#include <iostream>
#include <memory>
#include <TFile.h>
#include <TCanvas.h>
#include <TH1.h>
#include <sol/sol.hpp>
#include <fmt/core.h>
#include <fmt/ranges.h>

using KeyType = std:: string;

struct DataSource {
  std::string path;
  std::unordered_set<std::string> tags;
  std::unordered_set<std::string> keys;
  TFile *file;
    DataSource() = default;
    DataSource(const std::string& p): path{p}{}
    std::string to_string(){
        return fmt::format("DataSource({},{})", path,tags);
    }
};


struct SourceSet {
    std::optional<std::string> match;
    std::vector<DataSource *> sources;
    SourceSet() = default;
    SourceSet(const std::string& m): match{m}{}
    SourceSet(const std::vector<DataSource*> dsv): sources{dsv}{}
     std::string to_string(){
         std::vector<std::string> temp;
         for(const auto& ds: sources){
             temp.push_back(ds->path);
         }
         if(match)
         return fmt::format("SourceSet({},{})", match.value(), temp);
         else
         return fmt::format("SourceSet({})",  temp);
     }
};

struct PlotData{
    SourceSet* s;
};

void makeBindings(sol::state& lua);

void makeBindings(sol::state& lua){
    auto data_source_type =
        lua.new_usertype<DataSource>("DataSource",
                                     sol::constructors< DataSource(), DataSource(std::string)>(),
                                      "path", &DataSource::path, 
                                      "tags", &DataSource::tags,
                                      "keys", &DataSource::keys
                                      );
    auto source_set_type =
        lua.new_usertype<SourceSet>("SourceSet",
                sol::constructors<SourceSet(), SourceSet(std::string), SourceSet(std::vector<DataSource*>)>(),
                                     "match", &SourceSet::match,
                                     "sources", &SourceSet::sources
                                      );

}

struct SourceHistos{
    SourceSet* ss;
    std::vector<std::unique_ptr<TH1>> histos;
    std::unique_ptr<THStack> stack;
};


struct Normalize{
   SourceHistos  operator()(const SourceHistos& sh, float to){}
   SourceHistos  operator()(const SourceHistos& sh){}
};




// class DataSetOperator{
//     std::vector<SourceSet*> sets;
// public:
//     virtual const std::vector<SourceSet*>& getSets() const {return sets;} 
//     virtual void setSets(std::vector<SourceSet*>& s){ sets = s;}
//     virtual float produce(TH1* hist) {return 0 ;} 
//     virtual void apply(TH1* hist) {}
//     virtual ~DataSetOperator() = default;
// };
// 
//     
// class Integral : public DataSetOperator {
// public:
//     float produce(TH1* h) override { return h->Integral();}
//     virtual ~Integral() = default;
// };
// 
// 
// class Normalizer: public DataSetOperator {
//     virtual ~Normalizer() = default;
//     float norm_to = 1;
//     DataSetOperator* op = nullptr;
//     Normalizer(float to):norm_to{to}{}
//     Normalizer(DataSetOperator* op): op{op}{}
//     void apply(TH1* hist) override {
//         if (op) {
//             norm_to = op->produce(hist);
//         }
//         hist->Scale(norm_to / hist->Integral());
//     };
// };

using Pad = TVirtualPad;


//     TCanvas c;
//     std::vector<std::unique_ptr<DataSetOperator>> dso;
//
//     void create();
// };

struct SimplePlotArgs{
    std::string xlabel, ylabel, title, hname, ;
};

Pad simplePlot(SimplePlotArgs args){

}

// class SimplePlot{
//     struct Args{
//         std::string hist_name;
// 
//     } args;
//     void create(TCanvas& c, Context& con);
// };
// 
// void SimplePlot::create(TCanvas& c, Context& con){
//     std::unorderd_map<std::string, std::vector<TH1*>> histograms;
//     for (const auto &source : sources) {
//       if (!source->tags.count(stack_tag))
//         continue;
//       if (!source->file->GetListOfKeys()->Contains(hinstance.name.c_str())) {
//         throw std::runtime_error(fmt::format(
//             "Could not find histogram {} in data source {} with path {}",
//             hinfo.name, source->name, source->path));
//       }
//       auto hist = (TH1D *)source->file->Get(hinstance.name.c_str());
// 
// }
    
int main(int argc, char *argv[])
{

    sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::string);
    makeBindings(lua);
    lua.script_file("test.lua");
    return 0;
}
