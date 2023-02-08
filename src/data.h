#pragma once

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TPad.h>
#include <fmt/format.h>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "style.h"
#include "verbosity.h"
#include "glob.hpp"

class TFile;
class TH1;
class TH2;

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
    std::string subdir;

    Style style;
    TFile *file = nullptr;

    DataSource(const std::string &p, const std::string &n) : DataSource(p) {
        name = n;
    }
    DataSource(const std::string &p, const std::string &n, const std::string &s)
        : DataSource(p, n) {
        subdir = s;
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

    TClass getKeyClass(const std::string &keyname);

    template <typename T>
    std::shared_ptr<T> getHist(const std::string &name) const {
        assert(file != nullptr);
        auto ret = std::shared_ptr<T>(
            static_cast<T *>(file->Get<T>(name.c_str())->Clone()));
        return ret;
    }

    virtual ~DataSource() = default;
};

template <typename T>
struct PlotData {
    std::shared_ptr<T> hist;
    Style style;
    std::string source_name;
    std::string name;
    PlotData(const std::shared_ptr<T> &hist, const Style &style,
             const std::string &source_name, const std::string &name)
        : hist{hist}, style{style}, source_name{source_name}, name{name} {}
};

template <typename T>
PlotData<T> getData(const DataSource &s, const std::string &name) {
    return PlotData<T>{s.getHist<T>(name), s.style, s.name, name};
}

template <typename T>
std::vector<PlotData<T>> getData(const SourceSet &s, const std::string &name) {
    std::vector<PlotData<T>> ret;
    for (const auto ds : s.getSources()) {
        ret.push_back(getData<T>(*ds, name));
    }
    return ret;
}

template <typename T>
std::unordered_map<std::string, std::vector<PlotData<T>>> extractMatchingHistos(
    const SourceSet &set, const std::string &pattern) {
    std::unordered_map<std::string, std::vector<PlotData<T>>> ret;
    for (const auto &k : set.common_keys) {
        if (glob::match(k, pattern)) {
            auto histos = getData<T>(set, k);
            ret.insert({k, std::move(histos)});
        }
    }
    return ret;
}
}  // namespace rootp
