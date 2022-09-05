#include "data.h"

#include "TFile.h"
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <sol/sol.hpp>
#include "util.h"

static std::vector<std::unique_ptr<DataSource>> data_sources;

DataSource *DataSource::create(const std::string &p) {
    auto up = std::make_unique<DataSource>(p);
    DataSource *ret = up.get();
    data_sources.push_back(std::move(up));
    return ret;
}

TH1 *DataSource::getHist(const std::string &name) {
    assert(file != nullptr);
    TH1 *hist = static_cast<TH1 *>(file->Get(name.c_str()));
    return hist;
};

void DataSource::load() {
    file = TFile::Open((path).c_str());
    if (!file) {
        fmt::print("Could not open file {}\n", path);
        std::terminate();
    }
}

void DataSource::loadKeys() {
    for (const auto &key : *(file->GetListOfKeys())) {
        keys.insert(key->GetName());
    }
    if (keys.empty()) {
        fmt::print("File {} does not contain any keys", path);
        std::terminate();
    }
}

std::string SourceSet::to_string() {
    std::vector<std::string> temp;
    for (const auto &ds : sources) {
        temp.push_back(ds->path);
    }
    return fmt::format("SourceSet({})", temp);
}

void bindData(sol::state &lua) {
    // clang-format off
    auto data_source_type = lua.new_usertype<DataSource>(
        "DataSource",
        "create", sol::factories(&DataSource::create),
        BUILD(DataSource, name),
        BUILD(DataSource, path),
        BUILD(DataSource, tags),
        BUILD(DataSource, keys),
        BUILD(DataSource, palette_idx));
        //     "name", sol::as_function(&DataSource::name),

    auto source_set_type = lua.new_usertype<SourceSet>(
        "SourceSet",
        sol::constructors<SourceSet(), SourceSet(std::vector<DataSource *>)>(),
        BUILD(SourceSet, sources),
        "get_keys", &SourceSet::getKeys);

    auto input_data = lua.new_usertype<InputData>(
        "InputData", sol::constructors<InputData(), InputData(SourceSet *),
                          InputData(SourceSet *, bool, float, bool)>(),
        BUILD(InputData, source_set),
        BUILD(InputData, normalize),
        BUILD(InputData, norm_to),
        BUILD(InputData, yrange),
        "xrange" , [](InputData *c, float x, float y) {     
                  c->xrange = {x,y};
                  return c;                                            
              },                                                       
        "yrange" , [](InputData *c, float x, float y) {     
                  c->yrange = {x,y};
                  return c;                                            
              },                                                       
        BUILD(InputData, stack));

    auto matched_type = lua.new_usertype<MatchedKey>(
        "MatchedKey",
        "inputs", sol::readonly(&MatchedKey::inputs),
        "captures", sol::readonly(&MatchedKey::captures));
    // clang-format on
}

std::vector<std::unique_ptr<PlotElement>> finalizeInputData(
    const PlotterInput &input) {
    assert(input.data.source_set);

    std::vector<std::unique_ptr<PlotElement>> ret;
    std::vector<DataSource *> sources;
    auto stack = new THStack();
    for (DataSource *source : input.data.source_set->sources) {
        TH1 *hist = source->getHist(input.name);
        if (input.data.normalize) {
            hist->Scale(input.data.norm_to / hist->Integral());
        }
        if (input.data.stack) {
            if (input.data.normalize) {
                hist->Scale(input.data.norm_to / hist->Integral());
            }
            stack->Add(hist);
            sources.push_back(source);
        } else {
            auto h = std::make_unique<Histogram>(source, hist);
            if (input.data.xrange) {
                h->setRangeX(input.data.xrange.value());
            }
            if (input.data.yrange) {
                h->setRangeY(input.data.yrange.value());
            }
            ret.push_back(std::move(h));
        }
    }
    if (input.data.stack) {
        auto h = std::make_unique<Stack>(sources, stack);
        if (input.data.xrange) {
            h->setRangeX(input.data.xrange.value());
        }
        if (input.data.yrange) {
            h->setRangeY(input.data.yrange.value());
        }
        ret.push_back(std::move(h));
    }
    return ret;
}

std::vector<MatchedKey> expand(std::vector<InputData> in,
                               const std::unordered_set<std::string> &keys,
                               const std::string &pattern) {
    if (!glob::isGlob(pattern)) {
        MatchedKey single;
        for (const auto &input : in) {
            single.inputs.push_back({pattern, input});
            single.captures["HISTNAME"] = pattern;
        }
        return {single};
    }
    std::vector<MatchedKey> ret;
    for (const auto &k : keys) {
        if (glob::match(k, pattern)) {
            MatchedKey single;
            for (const auto &input : in) {
                single.inputs.push_back({k, input});
                single.captures["HISTNAME"] = k;
            }
            ret.push_back(single);
        }
    }
    return ret;
}

std::vector<std::unique_ptr<PlotElement>> finalizeManyInputData(
    const std::vector<PlotterInput> &input) {
    std::vector<std::unique_ptr<PlotElement>> ret;
    for (const auto &d : input) {
        auto one_set = finalizeInputData(d);
        std::move(one_set.begin(), one_set.end(), std::back_inserter(ret));
    }
    return ret;
}
