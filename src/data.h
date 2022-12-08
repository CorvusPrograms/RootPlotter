#pragma once

#include <TPad.h>
#include <fmt/format.h>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "style.h"
#include "verbosity.h"

class TFile;
class TH1;

namespace rootp {

struct DataSource;

struct SourceSet {
    std::vector<DataSource *> sources;

    std::unordered_set<std::string> common_keys;

    SourceSet() = default;
    SourceSet(const std::vector<DataSource *> dsv) : sources{dsv} {
        initKeys();
    }
    // std::string to_string();
    std::unordered_set<std::string> getKeys() const { return common_keys; }
    const std::vector<DataSource *> getSources() const { return sources; }
    ~SourceSet() = default;
    void initKeys();
};

struct DataSource {
    std::unordered_set<std::string> tags;
    std::unordered_set<std::string> keys;

    std::string path;
    std::string name;

    Style style;
    TFile *file = nullptr;


    DataSource(const std::string &p, const std::string &n) : DataSource(p) {
        name = n;
    }

    DataSource(const std::string &p) : path{p} {
        load();
        loadKeys();
    }

    DataSource &setName(const std::string &n) {
        name = n;
        return *this;
    }

    void load();
    void loadKeys();
    std::shared_ptr<TH1> getHist(const std::string &name) const;

    virtual ~DataSource() = default;
};

struct PlotData {
    std::shared_ptr<TH1> hist;
    Style style;
    std::string source_name;
    std::string name;
};

inline PlotData getData(const DataSource &s, const std::string &name) {
    return PlotData{s.getHist(name), s.style, s.name, name};
}

std::vector<PlotData> getData(const SourceSet &s, const std::string &name);

std::unordered_map<std::string, std::vector<PlotData>> extractMatchingHistos(
    const SourceSet &set, const std::string &pattern);

}  // namespace rootp
