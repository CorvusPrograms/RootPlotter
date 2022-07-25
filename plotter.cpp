#include "TFile.h"
#include <iostream>
#include <fstream>
#include <TH1D.h>
#include <unordered_set>
#include <TCanvas.h>
#include <TGaxis.h>
#include <TStyle.h>
#include <TLegend.h>
#include <algorithm>
#include <TPaveText.h>
#include <TEfficiency.h>
#include <TRatioPlot.h>
#include <exception>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <nlohmann/json.hpp>
#include <regex>
#include <iterator>
#include <THStack.h>

namespace glob {
namespace details {
template <typename Compare, typename Iterator,
          typename ValueType =
              typename std::iterator_traits<Iterator>::value_type>
inline bool match_impl(const Iterator pattern_begin, const Iterator pattern_end,
                       const Iterator data_begin, const Iterator data_end,
                       const ValueType zero_or_more,
                       const ValueType exactly_one) {
  typedef typename std::iterator_traits<Iterator>::value_type type;

  const Iterator null_itr(0);

  Iterator p_itr = pattern_begin;
  Iterator d_itr = data_begin;
  Iterator np_itr = null_itr;
  Iterator nd_itr = null_itr;
  for (;;) {
    if (pattern_end != p_itr) {
      const type c = *(p_itr);
      if ((data_end != d_itr) &&
          (Compare::cmp(c, *(d_itr)) || (exactly_one == c))) {
        ++d_itr;
        ++p_itr;
        continue;
      } else if (zero_or_more == c) {
        while ((pattern_end != p_itr) && (zero_or_more == *(p_itr))) {
          ++p_itr;
        }
        const type d = *(p_itr);
        while ((data_end != d_itr) &&
               !(Compare::cmp(d, *(d_itr)) || (exactly_one == d))) {
          ++d_itr;
        }
        np_itr = p_itr - 1;
        nd_itr = d_itr + 1;
        continue;
      }
    } else if (data_end == d_itr)
      return true;
    if ((data_end == d_itr) || (null_itr == nd_itr))
      return false;
    p_itr = np_itr;
    d_itr = nd_itr;
  }
  return true;
}
typedef char char_t;
struct cs_match {
  static inline bool cmp(const char_t c0, const char_t c1) {
    return (c0 == c1);
  }
};
struct cis_match {
  static inline bool cmp(const char_t c0, const char_t c1) {
    return (std::tolower(c0) == std::tolower(c1));
  }
};
} // namespace details
inline bool match(const std::string &s, const std::string &p,
                  const std::string::value_type match_one_or_more = '*',
                  const std::string::value_type match_exactly_one = '.') {
  return details::match_impl<details::cs_match>(
      std::begin(p), std::end(p), std::begin(s), std::end(s), match_one_or_more,
      match_exactly_one);
}

inline bool isGlob(const std::string &s,
                   const std::string::value_type match_one_or_more = '*',
                   const std::string::value_type match_exactly_one = '.') {
  return s.find(match_exactly_one) != std::string::npos ||
         s.find(match_one_or_more) != std::string::npos;
}
} // namespace glob

struct DataSource {
  std::string path;
  std::string name;
  std::unordered_set<std::string> tags;
  std::unordered_set<std::string> keys;
  TFile *file;
};

struct SourceSet {
  std::string name;
  std::string match;
  std::vector<const DataSource *> sources;
};

struct HistogramInfo {
  std::vector<std::string> set_names;
  std::vector<const SourceSet *> sets;

  struct Instance {
    HistogramInfo &info;
    std::string name;
    int idx_set;
  };
  std::vector<std::unique_ptr<Instance> > instances;

  std::string name;
  std::string title, xlabel, ylabel;
  std::string form;
  std::string mode;
  bool log_x = false, log_y = false;

  nlohmann::json data;
};

struct Histogram {
  TCanvas *canvas;
  const HistogramInfo::Instance *hinstance;
  const SourceSet *set;
};

template <typename T, typename X = T>
void get_toor(const nlohmann::json &j, const std::string &name, T &x, X def) {
  if (j.contains(name)) {
    j.at(name).get_to(x);
  } else {
    x = std::move(def);
  }
}

void from_json(const nlohmann::json &j, DataSource &p) {
  j.at("name").get_to(p.name);
  j.at("path").get_to(p.path);
  get_toor(j, "tags", p.tags, {});
}

void from_json(const nlohmann::json &j, SourceSet &p) {
  j.at("name").get_to(p.name);
  get_toor(j, "match", p.match, {});
}

void from_json(const nlohmann::json &j, HistogramInfo &p) {
  j.at("name").get_to(p.name);
  get_toor(j, "title", p.title, p.name);
  get_toor(j, "xlabel", p.xlabel, p.title);
  get_toor(j, "ylabel", p.ylabel, "Events");
  get_toor(j, "log_x", p.log_x, false);
  get_toor(j, "log_y", p.log_y, false);
  get_toor(j, "mode", p.mode, "default");
  get_toor(j, "set_names", p.set_names, {});
  p.data = j;
}

struct DataOptions {
  bool plotsame;
};

struct Processor {
  virtual void createVisual(Histogram &h, const DataOptions &opts) = 0;
};

struct ProcessMaker {
  std::unordered_map<std::string, std::unique_ptr<Processor> > processors;
  template <typename T> void addProcessor(const std::string &name) {
    processors[name] = std::unique_ptr<T>(new T());
  }
  Processor *getProcessor(const std::string &s) { return processors[s]; }
  const Processor *operator[](const std::string &s) const {
    return processors[s];
  }
}

// Histogram makeRatioPlot(TH1* h1, TH1* h2) {
//   auto canvas = new TCanvas();
//   gStyle->SetPalette(kOcean);
//   gStyle->SetOptTitle(kFALSE);
//   gStyle->SetOptStat(0);
//   auto plot = new TRatioPlot(h1, h2);
//   plot->Draw();
//   plot->GetLowerRefGraph()->SetMinimum(0.8);
//   plot->GetLowerRefGraph()->SetMaximum(1.2);
//   plot->SetSeparationMargin(0.0f);
//   plot->SetH1DrawOpt("PLC PMC");
//   plot->SetH2DrawOpt("PLC PMC");
//   auto legend = plot->GetUpperPad()->BuildLegend(0.15, 0.15, 0.15, .15);
//   legend->SetTextSize(0.05);
//   legend->SetHeader("Legend", "C");
//   return canvas;
// }
struct SimpleOverlayPlotter : public Processor {

  void createVisual(Histogram &h, const DataOptions &opts) override {

    gStyle->SetPalette(kBird);
    // gStyle->SetOptStat(0);
    auto &sources = h.set->sources;
    const auto &hinfo = h.hinstance->info;
    const auto &hinstance = *h.hinstance;
    auto legend = new TLegend();
    bool title_set = false;
    auto stack = new THStack();
    for (const auto &source : sources) {
      if (!source->file->GetListOfKeys()->Contains(hinstance.name.c_str())) {
        throw std::runtime_error(fmt::format(
            "Could not find histogram {} in data source {} with path {}",
            hinfo.name, source->name, source->path));
      }
      auto hist = (TH1D *)source->file->Get(hinstance.name.c_str());
      hist->GetXaxis()->SetTitle(hinfo.xlabel.c_str());
      hist->GetYaxis()->SetTitle(hinfo.ylabel.c_str());
      hist->SetMarkerStyle(kPlus);
      // hist->Draw("Same PLC PMC");
      stack->Add(hist);
      legend->AddEntry(hist, source->name.c_str());
    }
    if (hinfo.log_x) {
      h.canvas->SetLogx();
    }
    if (hinfo.log_y) {
      h.canvas->SetLogy();
    }
    stack->Draw("nostack PLC PMC HIST p");
    stack->SetTitle(hinfo.title.c_str());
    stack->GetXaxis()->SetTitle(hinfo.xlabel.c_str());
    stack->GetYaxis()->SetTitle(hinfo.ylabel.c_str());

    legend->SetX1(0.7);
    legend->SetY1(0.7);
    legend->SetX2(0.85);
    legend->SetY2(0.85);
    legend->SetHeader("Samples", "C");
    legend->Draw();
  }
};

struct StackPlotter : public Processor {
  void createVisual(Histogram &h, const DataOptions &opts) override {
    gStyle->SetPalette(kBird);
    // gStyle->SetOptStat(0);
    auto &sources = h.set->sources;
    const auto &hinfo = h.hinstance->info;
    const auto &hinstance = *h.hinstance;
    const auto &data = h.hinstance->hinfo.data;
    std::unordered_set<std::string> stack, nostack;

    auto legend = new TLegend();
    bool title_set = false;
    auto stack = new THStack();
    auto unstack = new THStack();
    for (const auto &source : sources) {
      if (!source->file->GetListOfKeys()->Contains(hinstance.name.c_str())) {
        throw std::runtime_error(
            fmt::format("Could not find histogram {}", hinfo.name));
      }
      auto hist = (TH1D *)source->file->Get(hinstance.name.c_str());
      hist->GetXaxis()->SetTitle(hinfo.xlabel.c_str());
      hist->GetYaxis()->SetTitle(hinfo.ylabel.c_str());
      hist->SetMarkerStyle(kPlus);
      // hist->Draw("Same PLC PMC");
      stack->Add(hist);
      legend->AddEntry(hist, source->name.c_str());
    }
    stack->Draw("nostack PLC PMC HIST p");
    stack->SetTitle(hinfo.title.c_str());
    stack->GetXaxis()->SetTitle(hinfo.xlabel.c_str());
    stack->GetYaxis()->SetTitle(hinfo.ylabel.c_str());

    legend->SetX1(0.7);
    legend->SetY1(0.7);
    legend->SetX2(0.85);
    legend->SetY2(0.85);
    legend->SetHeader("Samples", "C");
    legend->Draw();
  }
};

std::vector<Histogram> runProcess(const ProcessMaker &pm,
                                  const std::vector<HistogramInfo> &hinfos,
                                  const std::vector<DataSource> &sources,
                                  const DataOptions &opts) {
  std::vector<Histogram> ret;
  for (const auto &hinfo : hinfos) {
    for (const auto &instance : hinfo.instances) {
      ret.push_back(Histogram{ new TCanvas(), instance.get(),
                               hinfo.sets[instance->idx_set] });
      ret.back();
      pm[hinfo.mode]->createVisual(ret.back(), opts);
    }
  }
  return ret;
}

int main(int argc, char *argv[]) {
  std::vector<DataSource> data_sources;
  std::vector<HistogramInfo> histograms;

  TH1::AddDirectory(kFALSE);
  std::string config_file_name(argv[1]);
  std::string out_dir(argv[2]);
  fmt::print("Running RootPlotter with configuration file {}\n",
             config_file_name);

  std::ifstream config(config_file_name);
  nlohmann::json data;
  config >> data;
  fmt::print("Done reading configuration");

  std::unordered_map<std::string, SourceSet> source_sets;

  for (const auto &i : data.at("source_sets")) {
    SourceSet s = i.get<SourceSet>();
    source_sets.insert({ s.name, std::move(s) });
  }

  for (const auto &i : data.at("infiles")) {
    data_sources.push_back(i.get<DataSource>());
  }

  for (const auto &i : data.at("histos")) {
    histograms.push_back(i.get<HistogramInfo>());
  }
  std::string base_in_path = data.at("base_in_path");

  for (auto &pair : source_sets) {
    for (const DataSource &source : data_sources) {
      if (glob::match(source.name, pair.second.match)) {
        pair.second.sources.push_back(&source);
      }
    }
  }

  std::vector<TFile *> files;
  for (DataSource &file : data_sources) {
    std::string inpath = base_in_path + "/" + file.path;
    fmt::print("Opening file {}\n", inpath);
    file.file = TFile::Open((inpath).c_str());
    if (!file.file) {
      throw std::runtime_error(fmt::format("Could not open file {}", inpath));
    }
    for (const auto &key : *(file.file->GetListOfKeys())) {
      file.keys.insert(key->GetName());
    }
    if (file.keys.empty()) {
      throw std::runtime_error(
          fmt::format("File {} does not contain any keys", inpath));
    }
  }

  for (auto &hinfo : histograms) {
    if (hinfo.set_names.empty()) {
      for (const auto &pair : source_sets) {
        hinfo.sets.push_back(&pair.second);
      }
    } else {
      for (const auto &set : hinfo.set_names) {
        hinfo.sets.push_back(&source_sets.at(set));
      }
    }
  }

  for (auto &hinfo : histograms) {
    if (glob::isGlob(hinfo.name)) {
      for (int i = 0; i < hinfo.sets.size(); ++i) {
        for (const auto &key : hinfo.sets[i]->sources[0]->keys) {
          if (glob::match(key, hinfo.name)) {
            hinfo.instances.push_back(std::unique_ptr<HistogramInfo::Instance>(
                new HistogramInfo::Instance{ hinfo, key, i }));
          }
        }
      }
    } else {
      for (int i = 0; i < hinfo.sets.size(); ++i) {
        hinfo.instances.push_back(std::unique_ptr<HistogramInfo::Instance>(
            new HistogramInfo::Instance{ hinfo, hinfo.name, i }));
      }
    }
  }

  ProcessMaker pm;
  pm.add<StackPlotter>("stack");
  pm.add<SimpleOverlayPlotter>("default");
  auto histos = runProcess(pm, histograms, data_sources, {});

  fmt::print("{}", data_sources[0].keys);

  for (auto &h : histos) {
    h.canvas->SaveAs((out_dir + "/" + h.hinstance->name + "_" + h.set->name +
                      ".pdf").c_str());
  }

  return 0;
}
