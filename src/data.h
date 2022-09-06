#pragma once

#include <THStack.h>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fmt/format.h>

#include "glob.hpp"
#include "plot_element.h"

class TFile;
class TH1;
struct PlotElement;
struct Histogram;
struct Stack;

struct DataSource {
    std::unordered_set<std::string> tags;
    std::unordered_set<std::string> keys;
    std::string path;
    std::string name;
    int palette_idx;
    TFile *file = nullptr;
    static DataSource *create(const std::string &p);

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
    DataSource &setPal(int p) {
        palette_idx = p;
        return *this;
    }

    //    std::string to_string() {
    //        return fmt::format("DataSource({}, {},{})", name, path, tags);
    //    }
    void load();
    void loadKeys();
    TH1 *getHist(const std::string &name);
};

struct SourceSet {
    std::vector<DataSource *> sources;
    std::unordered_set<std::string> common_keys;
    SourceSet() = default;
    SourceSet(const std::vector<DataSource *> dsv) : sources{dsv} {
        initKeys();
    }
    static std::shared_ptr<SourceSet> create(
        const std::vector<DataSource *> dsv);
    //std::string to_string();
    std::unordered_set<std::string> getKeys() const;
    void initKeys();
};

struct InputData {
    std::shared_ptr<SourceSet> source_set;
    bool normalize = false;
    float norm_to = 1.0f;
    bool stack = false;
    std::optional<std::pair<float, float>> yrange = std::nullopt,
                                           xrange = std::nullopt;

    InputData() = default;
    InputData(std::shared_ptr<SourceSet> s) : source_set{s}{}
    

    InputData(std::shared_ptr<SourceSet> s, bool n, float nt, bool st)
        : source_set{s}, normalize{n}, norm_to{nt}, stack{st} {}
};

struct PlotterInput {
    std::string name;
    InputData data;
};

struct MatchedKey {
    std::vector<PlotterInput> inputs;
    std::unordered_map<std::string, std::string> captures;
};

namespace sol {
class state;
}
void bindData(sol::state &lua);

std::vector<MatchedKey> expand(std::vector<InputData> in,
                               const std::string &pattern);

std::vector<std::unique_ptr<PlotElement>> finalizeInputData(
    const PlotterInput &input);

std::vector<std::unique_ptr<PlotElement>> finalizeManyInputData(
    const std::vector<PlotterInput> &input);

namespace {
class state;
}
void bindPlotters(sol::state &lua);
void bindPalettes(sol::state &lua);
