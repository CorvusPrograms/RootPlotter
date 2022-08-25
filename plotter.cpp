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
#include <THStack.h>
#include <sol/sol.hpp>
#include <fmt/core.h>
#include <fmt/ranges.h>

using KeyType = std::string;

struct DataSource {
  std::string path;
  std::unordered_set<std::string> tags;
  std::unordered_set<std::string> keys;
  TFile *file;
  DataSource() = default;
  DataSource(const std::string &p) : path{ p } {
    load();
    getKeys();
  }
  std::string to_string() {
    return fmt::format("DataSource({},{})", path, tags);
  }
  void load();
  void getKeys();
};

void DataSource::load() {
  file = TFile::Open((path).c_str());
  if (!file) {
    fmt::print("Could not open file {}\n", path);
    std::terminate();
  }
}

void DataSource::getKeys() {
  for (const auto &key : *(file->GetListOfKeys())) {
    keys.insert(key->GetName());
  }
  if (keys.empty()) {
    fmt::print("File {} does not contain any keys", path);
    std::terminate();
  }
}

struct SourceSet {
  std::optional<std::string> match;
  std::vector<DataSource *> sources;
  SourceSet() = default;
  SourceSet(const std::string &m) : match{ m } {}
  SourceSet(const std::vector<DataSource *> dsv) : sources{ dsv } {}
  std::string to_string() {
    std::vector<std::string> temp;
    for (const auto &ds : sources) {
      temp.push_back(ds->path);
    }
    if (match)
      return fmt::format("SourceSet({},{})", match.value(), temp);
    else
      return fmt::format("SourceSet({})", temp);
  }
};

void makeBindings(sol::state &lua);

void makeBindings(sol::state &lua) {
  auto data_source_type = lua.new_usertype<DataSource>(
      "DataSource", sol::constructors<DataSource(), DataSource(std::string)>(),
      "path", &DataSource::path, "tags", &DataSource::tags, "keys",
      &DataSource::keys);
  auto source_set_type = lua.new_usertype<SourceSet>(
      "SourceSet", sol::constructors<SourceSet(), SourceSet(std::string),
                                     SourceSet(std::vector<DataSource *>)>(),
      "match", &SourceSet::match, "sources", &SourceSet::sources);
}

static std::vector<std::unique_ptr<DataSource> > sources;
static std::vector<std::unique_ptr<SourceSet> > source_sets;

void createPlot(const std::string &hname, const std::vector<SourceSet *> &ss,
                sol::function f) {}

using Pad = TPad;

Pad* simplePlot(const sol::table& t){
    fmt::print("{}", std::string(t["x"]));
}


int main(int argc, char *argv[]) {
  sol::state lua;
  lua.open_libraries(sol::lib::base, sol::lib::string);
  lua["plotters"] = lua.create_table();
  lua["simple"] = simplePlot;
  makeBindings(lua);
  lua.script_file("example.lua");
  return 0;
}
