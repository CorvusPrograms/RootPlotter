#include "TFile.h"
#include <iostream>
#include <fstream>
#include <TH1D.h>
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
#include <json.hpp>
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

        // set backtrack iterators
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
                  const std::string::value_type match_exatcly_one = '.') {
  return details::match_impl<details::cs_match>(
      std::begin(p), std::end(p), std::begin(s), std::end(s), match_one_or_more,
      match_exatcly_one);
}

} // namespace glob

struct DataSource {
  std::string path;
  std::string name;
  std::string tags;
  TFile *file;
};

struct SourceSet {
  std::string name;
  std::string match;
  std::vector<const DataSource *> sources;
};

namespace std {
template <> struct hash<SourceSet> {
  std::size_t operator()(const SourceSet &ss) {
    return std::hash<std::string> {}
    (ss.name);
  }
};
}

struct HistogramInfo {
  std::vector<std::string> set_names;
  std::vector<const SourceSet *> sets;
  std::string name;
  std::string title, xlabel, ylabel;
  std::string form;
};

struct Histogram {
  TCanvas *canvas;
  const HistogramInfo *hinfo;
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
  get_toor(j, "set_names", p.set_names, {});
}

struct DataOptions {
  bool plotsame;
};

struct Processor {
  virtual void createVisual(Histogram &h, const DataOptions &opts) = 0;
};

std::vector<Histogram> runProcess(Processor *processor,
                                  const std::vector<HistogramInfo> &hinfos,
                                  const std::vector<DataSource> &sources,
                                  const DataOptions &opts) {
  std::vector<Histogram> ret;
  for (const auto &hinfo : hinfos) {
    for (const auto &set : hinfo.sets) {
      ret.push_back(Histogram{ new TCanvas(), &hinfo, set });
      ret.back();
      processor->createVisual(ret.back(), opts);
    }
  }
  return ret;
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
    //gStyle->SetOptStat(0);
    auto &sources = h.set->sources;
    const auto &hinfo = *h.hinfo;
    auto legend = new TLegend();
    bool title_set = false;
    auto stack = new THStack();
    for (const auto &source : sources) {
      if (!source->file->GetListOfKeys()->Contains(hinfo.name.c_str())) {
        throw std::runtime_error(
            fmt::format("Could not find histogram {}", hinfo.name));
      }
      auto hist = (TH1D *)source->file->Get(hinfo.name.c_str());
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

  for (auto &pair : source_sets) {
    for (const DataSource &source : data_sources) {
      if (glob::match(source.name, pair.second.match)) {
        pair.second.sources.push_back(&source);
      }
    }
  }

  std::vector<TFile *> files;
  for (DataSource &file : data_sources) {
    file.file = TFile::Open(file.path.c_str());
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

  std::unique_ptr<SimpleOverlayPlotter> opl(new SimpleOverlayPlotter());
  auto histos = runProcess(opl.get(), histograms, data_sources, {});

  for (auto &h : histos) {
    h.canvas->SaveAs(
        (out_dir + "/" + h.hinfo->name + "_" + h.set->name + ".pdf").c_str());
  }

  return 0;
}
