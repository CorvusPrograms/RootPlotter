#include "data.h"

#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <sol/sol.hpp>
#include <thread>
#include <typeinfo>

#include "util.h"
#include "verbosity.h"

namespace rootp {

void SourceSet::initKeys() {
    assert(!sources.empty());
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
}

void DataSource::load() {
    file = TFile::Open((path).c_str());
    if (!file) {
        vRuntimeError("Could not open file {}", path);
    }
    if (!subdir.empty()) {
        bool success = file->cd(subdir.c_str());
        if (!success) {
            vRuntimeError("Could not open file subdirectory {}", subdir);
        }
    }
}

std::unordered_set<std::string> allKeys(TDirectory *file,
                                        const std::string &current = "") {
    std::unordered_set<std::string> keys;
    for (const auto &key : *(file->GetListOfKeys())) {
        if (key->IsFolder()) {
            std::string next =
                current + (current.empty() ? "" : "/") + key->GetName();
            auto dir = file->Get<TDirectory>(key->GetName());
            std::unordered_set<std::string> thisfolder = allKeys(dir, next);
            keys.merge(thisfolder);
        } else {
            keys.insert(current + "/" + key->GetName());
        }
    }
    return keys;
}
void DataSource::loadKeys() {
    keys = allKeys(file);
    vPrint(VerbosityLevel::High, "Extracted {} keys from file {}\n",
           std::size(keys), path);
    if (keys.empty()) {
        vRuntimeError(
            "File {} does not contain any keys. Please check that the root "
            "file actually contains histograms.",
            path);
    }
}


}  // namespace rootp
