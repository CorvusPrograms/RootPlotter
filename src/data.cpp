#include "data.h"

#include <TFile.h>
#include <TH1D.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <sol/sol.hpp>
#include <thread>

#include "glob.hpp"
#include "util.h"
#include "verbosity.h"

namespace rootp {

void SourceSet::initKeys() {
    std::unordered_set<std::string> ret = sources[0]->keys;
    for (std::size_t i = 1; i < sources.size(); ++i) {
        for (const auto &k : sources[i]->keys) {
            if (ret.count(k) == 0) {
                ret.erase(k);
            }
        }
    }
    common_keys = std::move(ret);
}

std::shared_ptr<TH1> DataSource::getHist(const std::string &name) const {
    assert(file != nullptr);
    auto ret = std::shared_ptr<TH1>(
        static_cast<TH1 *>(file->Get<TH1>(name.c_str())->Clone()));
    return ret;
};

void DataSource::load() {
    file = TFile::Open((path).c_str());
    if (!file) {
        vRuntimeError("Could not open file {}", path);
    }
}

void DataSource::loadKeys() {
    for (const auto &key : *(file->GetListOfKeys())) {
        keys.insert(key->GetName());
    }
    vPrint(VerbosityLevel::High, "Extracted {} keys from file {}\n",
           std::size(keys), path);
    if (keys.empty()) {
        vRuntimeError(
            "File {} does not contain any keys. Please check that the root "
            "file actually contains histograms.",
            path);
    }
}

std::vector<PlotData> getData(const SourceSet &s, const std::string &name) {
    std::vector<PlotData> ret;
    for (const auto ds : s.getSources()) {
        ret.push_back(getData(*ds, name));
    }
    return ret;
}

std::unordered_map<std::string, std::vector<PlotData>> extractMatchingHistos(
    const SourceSet &set, const std::string &pattern) {
    std::unordered_map<std::string, std::vector<PlotData>> ret;
    for (const auto &k : set.common_keys) {
        if (glob::match(k, pattern)) {
            std::vector<PlotData> histos = getData(set, k);
            ret.insert({k, std::move(histos)});
        }
    }
    return ret;
}
}  // namespace rootp
