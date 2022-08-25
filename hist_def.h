#pragma once

#include <unordered_set>
#include <string>
#include <vector>
#include <memory>
#include <TFile.h>
#include <TCanvas.h>
#include <sol/sol.hpp>

using KeyType = std:: string;

struct DataSource {
  std::string path;
  std::string name;
  std::unordered_set<std::string> tags;
  std::unordered_set<std::string> keys;
  TFile *file;
    DataSource() = default;
    DataSource(const std::string& p): path{p}{}
    DataSource(const std::string& p, const std::string& n):path {p}, name{n}{}
};

struct SourceSet {
    using IdType = std::string;
  IdType name;
  std::string match;
  std::vector<const DataSource *> sources;
    SourceSet() = default;
    SourceSet(const std::string& n): name{n}{}
    SourceSet(const std::string& n, const std::string& m):name {n}, match{m}{}
};



void makeBindings(sol::state& lua);


// class SourceManager{
//     using KeyType = std::string
//     std::unordered_map<KeyType, std::unique_ptr<DataSource>> sources;
//     std::unordered_map<KeyType, std::unique_ptr<SourceSet>> sets;
// 
//     const auto& getSource(const KeyType& k) const {return *sources.at(k);}
//     const auto& getSet(const KeyType& k) const {return *sets.at(k);}
// 
// };
