#pragma once

#include <THStack.h>
#include <fmt/format.h>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "glob.hpp"
#include "plot_element.h"

class TFile;
class TH1;
struct PlotElement;
struct Histogram;
struct Stack;

struct Style {
    enum StyleMode {
        none = 0,
        line = 1 << 0,
        marker = 1 << 1,
        fill = 1 << 2,
    };

    StyleMode mode = StyleMode::marker;

    using StyleId_t = int;
    std::optional<int> palette_idx = std::nullopt;
    std::optional<int> color = std::nullopt;
    std::optional<StyleId_t> marker_style = std::nullopt;
    std::optional<float> marker_size = std::nullopt;
    std::optional<StyleId_t> line_style = std::nullopt;
    std::optional<int> line_width = std::nullopt;
    std::optional<StyleId_t> fill_style = std::nullopt;
};

struct DataSource;

struct SourceSet {
    std::vector<DataSource *> sources;
    std::unordered_set<std::string> common_keys;
    SourceSet() = default;
    SourceSet(const std::vector<DataSource *> dsv) : sources{dsv} {
        initKeys();
    }
    // std::string to_string();
    virtual std::unordered_set<std::string> getKeys() const;
    virtual std::vector<DataSource *> getSources();
    virtual ~SourceSet() = default;
    void initKeys();
};

struct DataSource : virtual SourceSet {
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

    virtual std::unordered_set<std::string> getKeys() const;
    virtual std::vector<DataSource *> getSources();

    void load();
    void loadKeys();
    TH1 *getHist(const std::string &name);

    //   std::string to_string(){
    //       return fmt::format("Datasource at {}\n", fmt::ptr(this));
    //   }

    //   DataSource &operator=(const DataSource &ds) {
    //       fmt::print("Copy constructing data source at {} from {}\n ",
    //                  fmt::ptr(this), fmt::ptr(&ds));
    //       return *this;
    //   }
    //   DataSource(const DataSource &ds) {
    //       fmt::print("Copy constructing data source at {} from {}\n ",
    //                  fmt::ptr(this), fmt::ptr(&ds));
    //   }
    virtual ~DataSource() {}
};

struct InputData {
    SourceSet *source_set;
    bool normalize = false;
    float norm_to = 1.0f;
    bool stack = false;
    std::optional<std::pair<float, float>> yrange = std::nullopt,
                                           xrange = std::nullopt;

    InputData() = default;
    InputData(SourceSet *s) : source_set{s} {}
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

void bindMarkerStyles(sol::state &lua);
void bindLineStyles(sol::state &lua);
void bindFillStyles(sol::state &lua);
void bindPalettes(sol::state &lua);

void bindGraphicalData(sol::state &lua);
