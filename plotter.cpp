#include <TCanvas.h>
#include <TFile.h>
#include <TGraphAsymmErrors.h>
#include <TH1.h>
#include <TStyle.h>
#include <THStack.h>
#include <TLegend.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <iostream>
#include <memory>
#include <optional>
#include <sol/sol.hpp>
#include <string>
#include <unordered_set>
#include <vector>

#include "CLI/App.hpp"
#include "glob.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

using KeyType = std::string;

using AnyHistogram = std::variant<TH1 *, THStack *>;

struct DataSource;

struct PlotElement {
  //  std::string xlabel, ylabel;
  virtual void addToLegend(TLegend *legend) = 0;
  virtual TH1 *getHistogram() = 0;
  virtual TH1 *getTotals() = 0;
  virtual float getIntegral() = 0;
  virtual TAxis *getXAxis() = 0;
  virtual TAxis *getYAxis() = 0;
  virtual void Draw(const std::string &s) = 0;
  virtual std::string to_string() const = 0;
  virtual float getMin() = 0;
  virtual float getMax() = 0;
  virtual void setFillAtt(TAttFill *) {};
  virtual void setMarkAtt(TAttMarker *) {};
  virtual void setLineAtt(TAttLine *) {};
  virtual void setTitle(const std::string &s) = 0;

  //   virtual void setXLabel(const std::string &s) { xlabel = s; }
  //   virtual std::string setXLabel(const std::string &s) const { return
  // xlabel; }
  //   virtual void setYLabel(const std::string &s) { ylabel = s; }
  //   virtual std::string setYLabel(const std::string &s) const { return
  // ylabel; }
  virtual ~PlotElement() = default;
};

struct DataSource {
  std::string path;
  std::string name;
  std::unordered_set<std::string> tags;
  std::unordered_set<std::string> keys;
  int pallette_idx = 1;
  TFile *file = nullptr;
  DataSource() = default;
  DataSource(const std::string &p, const std::string &n) : DataSource(p) {
    name = n;
  }
  DataSource(const std::string &p) : path{ p } {
    load();
    getKeys();
  }

  DataSource &setName(const std::string &n) {
    name = n;
    fmt::print("Setting name to {}\n", name);
    return *this;
  }
  DataSource &setPal(int p) {
    pallette_idx = p;
    return *this;
  }

  std::string to_string() {
    return fmt::format("DataSource({},{})", path, tags);
  }
  void load();
  void getKeys();
  TH1 *getHist(const std::string &name);
};

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

void DataSource::getKeys() {
  for (const auto &key : *(file->GetListOfKeys())) {
    keys.insert(key->GetName());
  }
  if (keys.empty()) {
    fmt::print("File {} does not contain any keys", path);
    std::terminate();
  }
}

struct SourceSet {
  std::vector<DataSource *> sources;
  SourceSet() = default;
  SourceSet(const std::vector<DataSource *> dsv) : sources{ dsv } {}
  std::string to_string() {
    std::vector<std::string> temp;
    for (const auto &ds : sources) {
      temp.push_back(ds->path);
    }
    return fmt::format("SourceSet({})", temp);
  }
  std::unordered_set<std::string> getKeys() {
    assert(sources.size() > 0);
    return sources[0]->keys;
  }
};

struct InputData {
  SourceSet *ss = nullptr;
  bool normalize = false;
  float norm_to = 1.0f;
  bool stack = false;
  InputData() = default;
  InputData(SourceSet *s) : ss{ s } {}
  InputData(SourceSet *s, bool n, float nt, bool st)
      : ss{ s }, normalize{ n }, norm_to{ nt }, stack{ st } {}
};

struct PlotterInput {
  std::string name;
  InputData data;
};

struct MatchedKey {
  std::vector<PlotterInput> inputs;
  std::unordered_map<std::string, std::string> captures;
};

struct Histogram : public PlotElement {
  DataSource *source;
  TH1 *hist;
  Histogram(DataSource *s, TH1 *h) : source{ s }, hist{ h } {}
  virtual void addToLegend(TLegend *legend) {
    fmt::print("During legend creation source  is {} {}", fmt::ptr(source),
               source->name);
    legend->AddEntry(hist, source->name.c_str());
  }
  virtual TH1 *getHistogram() { return hist; }
  virtual TH1 *getTotals() { return hist; }
  virtual float getIntegral() { return hist->Integral(); }
  virtual TAxis *getXAxis() { return hist->GetXaxis(); }
  virtual TAxis *getYAxis() { return hist->GetYaxis(); }
  virtual void setTitle(const std::string &s) { hist->SetTitle(s.c_str()); }
  virtual void Draw(const std::string &s) {
    setMarkAtt(hist);
    setFillAtt(hist);
    setLineAtt(hist);
    hist->Draw(s.c_str());
  }
  void setFillStyle() {}
  void setMarkerStyle() {}
  void setLineStyle() {}

  virtual float getMin() {
    auto xaxis = getXAxis();
    return xaxis->GetBinLowEdge(xaxis->GetFirst());
  }
  virtual float getMax() {
    auto xaxis = getXAxis();
    return xaxis->GetBinUpEdge(xaxis->GetLast());
  }
  virtual std::string to_string() const { return source->name; }

  virtual void setFillAtt(TAttFill *fill_att) {
    int pidx = source->pallette_idx;
    fill_att->SetFillColor(gStyle->GetColorPalette(pidx));
  }
  virtual void setMarkAtt(TAttMarker *mark_att) {
    int pidx = source->pallette_idx;
    mark_att->SetMarkerColor(gStyle->GetColorPalette(pidx));
  }
  virtual void setLineAtt(TAttLine *line_att) {
    int pidx = source->pallette_idx;
    line_att->SetLineColor(gStyle->GetColorPalette(pidx));
  }
  virtual ~Histogram() = default;
};

struct Stack : public PlotElement {
  std::vector<DataSource *> sources;
  THStack *hist;
  Stack(const std::vector<DataSource *> s, THStack *h)
      : sources{ s }, hist{ h } {}
  virtual void addToLegend(TLegend *legend) {
    int i = 0;
    for (const auto &h : *hist) {
      legend->AddEntry(h, sources[i]->name.c_str());
      ++i;
    }
  }
  virtual void setTitle(const std::string &s) { hist->SetTitle(s.c_str()); }
  virtual TH1 *getHistogram() { return hist->GetHistogram(); }
  virtual TH1 *getTotals() {
    TList *stackHists = hist->GetHists();
    TH1 *tmpHist = (TH1 *)stackHists->At(0)->Clone();
    tmpHist->Reset();
    for (int i = 0; i < stackHists->GetSize(); ++i) {
      tmpHist->Add((TH1 *)stackHists->At(i));
    }
    return tmpHist;
  }

  virtual float getIntegral() { return getTotals()->Integral(); }
  virtual TAxis *getXAxis() { return hist->GetXaxis(); }
  virtual TAxis *getYAxis() { return hist->GetYaxis(); }
  virtual void Draw(const std::string &s) {
    int i = 0;
    for (const auto &th : *hist) {
      auto h = static_cast<TH1 *>(th);
      auto pidx = sources[i]->pallette_idx;
      h->SetFillColor(gStyle->GetColorPalette(pidx));
      h->SetMarkerColor(gStyle->GetColorPalette(pidx));
      h->SetLineColor(gStyle->GetColorPalette(pidx));
      ++i;
    }
    hist->Draw(s.c_str());
  }
  virtual std::string to_string() const { return sources[0]->name; }
  virtual float getMin() {
    auto xaxis = getXAxis();
    return xaxis->GetBinLowEdge(xaxis->GetFirst());
  }
  virtual float getMax() {
    auto xaxis = getXAxis();
    return xaxis->GetBinUpEdge(xaxis->GetLast());
  }
  virtual ~Stack() = default;
};

struct PlotOptions {
  using SOType = std::optional<std::string>;
  SOType title, xlabel, ylabel;
  bool show_stats = false;
  bool logx = false, logy = false;
  int palette = kRainBow;
};

void makeBindings(sol::state &lua);

void makeBindings(sol::state &lua) {
  auto data_source_type = lua.new_usertype<DataSource>(
      "DataSource", sol::constructors<DataSource(), DataSource(std::string),
                                      DataSource(std::string, std::string)>(),
      "name", &DataSource::name, "path", &DataSource::path, "tags",
      &DataSource::tags, "keys", &DataSource::keys, "get_hist",
      &DataSource::getHist, "set_name", &DataSource::setName, "set_pal",
      &DataSource::setPal);
  auto source_set_type = lua.new_usertype<SourceSet>(
      "SourceSet",
      sol::constructors<SourceSet(), SourceSet(std::vector<DataSource *>)>(),
      "sources", &SourceSet::sources, "get_keys", &SourceSet::getKeys);
  auto input_data = lua.new_usertype<InputData>(
      "InputData",
      sol::constructors<InputData(), InputData(SourceSet *),
                        InputData(SourceSet *, bool, float, bool)>(),
      "source", &InputData::ss, "normalize", &InputData::normalize, "norm_to",
      &InputData::norm_to, "stack", &InputData::stack);
  auto matched_type =
      lua.new_usertype<MatchedKey>("MatchedKey", "inputs", &MatchedKey::inputs,
                                   "captures", &MatchedKey::captures);
  auto plot_options_type = lua.new_usertype<PlotOptions>(
      "PlotOptions", "xlabel", &PlotOptions::xlabel, "ylabel",
      &PlotOptions::ylabel, "title", &PlotOptions::title, "show_stats",
      &PlotOptions::show_stats, "logx", &PlotOptions::logx, "logy",
      &PlotOptions::logy, "palette", &PlotOptions::palette);
}

static std::vector<std::unique_ptr<DataSource> > sources;
static std::vector<std::unique_ptr<SourceSet> > source_sets;

// struct PlotData {
//     enum class Type { Stack, Histo } type;
//     TH1 *histo;
//     THStack *stack;
// };

using Pad = TVirtualPad;

void setupLegend(TLegend *legend) {
  legend->SetX1(0.7);
  legend->SetY1(0.7);
  legend->SetX2(0.90);
  legend->SetY2(0.90);
  legend->SetHeader("Samples", "C");
  legend->Draw();
}

void setAxisProperties(TAxis *yaxis, TAxis *xaxis) {
  assert(yaxis && xaxis);
  //   xaxis->SetLabelSize(12);
  //   yaxis->SetLabelSize(12);
  //   yaxis->SetTitleSize(16);
  //   xaxis->SetTitleSize(16);
  //   xaxis->SetLabelFont(43);
  //   yaxis->SetLabelFont(43);
  //   xaxis->SetTitleFont(43);
  //   yaxis->SetTitleFont(43);
  //   xaxis->SetTitleOffset(1.2);
  //   yaxis->SetTitleOffset(3);
}

inline TH1 *getHistogram(TH1 *h) { return h; }
inline TH1 *getHistogram(THStack *h) { return h->GetHistogram(); }

Pad *simplePlot(Pad *pad, std::vector<std::unique_ptr<PlotElement> > &data,
                const PlotOptions &opts) {
  pad->cd();
  if (!opts.show_stats) {
    gStyle->SetOptStat(0);
  }
  gStyle->SetPalette(opts.palette);
  auto legend = new TLegend();
  int i = 0;
  for (auto &pe : data) {
    if (i > 0) {
      pe->Draw("Same");
    } else {
      pe->Draw("");
    }
    //     if (opts.title) {
    //       pe->setTitle(opts.title.value());
    //     }
    //     if (opts.xlabel) {
    //       pe->getXAxis()->SetTitle(opts.xlabel->c_str());
    //     }
    //     if (opts.ylabel) {
    //       pe->getYAxis()->SetTitle(opts.ylabel->c_str());
    //     }
    setAxisProperties(pe->getXAxis(), pe->getYAxis());
    pe->addToLegend(legend);
    ++i;
  }
  if (opts.logx) {
    pad->SetLogx();
  }
  if (opts.logy) {
    pad->SetLogx();
  }
  setupLegend(legend);
  return pad;
}

Pad *ratioPlot(Pad *pad, PlotElement *num, PlotElement *den,
               PlotOptions &opts) {
  pad->cd();
  if (!opts.show_stats) {
    gStyle->SetOptStat(0);
  }
  gStyle->SetPalette(opts.palette);
  auto ratio_plot =
      new TGraphAsymmErrors(num->getTotals(), den->getTotals(), "pois");
  ratio_plot->Draw();
  num->setMarkAtt(ratio_plot);
  num->setLineAtt(ratio_plot);
  auto xaxis = ratio_plot->GetXaxis();
  auto yaxis = ratio_plot->GetYaxis();
  if (opts.xlabel) {
    xaxis->SetTitle(opts.xlabel->c_str());
  }
  if (opts.ylabel) {
    yaxis->SetTitle(opts.ylabel->c_str());
  }
  yaxis->SetRangeUser(0, 1.5);
  xaxis->SetLimits(num->getMin(), num->getMax());
  if (opts.logx) {
    pad->SetLogx();
  }
  if (opts.logy) {
    pad->SetLogx();
  }
  ratio_plot->Draw("SAME");
  return pad;
}

Pad *newPlot() { return new TCanvas(); }

std::vector<MatchedKey> expand(std::vector<InputData> in,
                               const std::unordered_set<std::string> &keys,
                               const std::string &pattern) {
  if (!glob::isGlob(pattern)) {
    MatchedKey single;
    for (const auto &input : in) {
      single.inputs.push_back({ pattern, input });
      single.captures["ALL"] = pattern;
    }
    return { single };
  }
  std::vector<MatchedKey> ret;
  for (const auto &k : keys) {
    if (glob::match(k, pattern)) {
      MatchedKey single;
      for (const auto &input : in) {
        single.inputs.push_back({ k, input });
        single.captures["ALL"] = k;
      }
      ret.push_back(single);
    }
  }
  return ret;
}

std::vector<std::unique_ptr<PlotElement> >
finalizeInputData(const PlotterInput &input) {
  assert(input.data.ss);

  std::vector<std::unique_ptr<PlotElement> > ret;
  std::vector<DataSource *> sources;
  auto stack = new THStack();
  for (DataSource *source : input.data.ss->sources) {
    TH1 *hist = source->getHist(input.name);
    if (input.data.normalize) {
      hist->Scale(input.data.norm_to / hist->Integral());
    }
    if (input.data.stack) {
      stack->Add(hist);
      sources.push_back(source);
    } else {
      ret.push_back(std::make_unique<Histogram>(source, hist));
    }
  }
  if (input.data.stack) {
    ret.push_back(std::make_unique<Stack>(sources, stack));
  }
  return ret;
}

std::vector<std::unique_ptr<PlotElement> >
finalizeManyInputData(const std::vector<PlotterInput> &input) {
  std::vector<std::unique_ptr<PlotElement> > ret;
  for (const auto &d : input) {
    auto one_set = finalizeInputData(d);
    std::move(one_set.begin(), one_set.end(), std::back_inserter(ret));
  }
  return ret;
}

void makeFreeBindings(sol::state &lua) {
  lua["plotters"] = lua.create_table();
  lua["simple"] = simplePlot;
  lua["ratio_plot"] = ratioPlot;
  lua["make_plot"] = newPlot;
  lua["create_options"] = [](sol::table params) {
    PlotOptions po;
    //    sol::optional<std::string> xlabel = params["xlabel"];
    //    if (xlabel) {
    //      po.xlabel = xlabel.value();
    //    }
    //    sol::optional<std::string> ylabel = params["ylabel"];
    //    if (ylabel) {
    //      po.ylabel = ylabel.value();
    //    }
    //    sol::optional<std::string> title = params["title"];
    //    if (title) {
    //      po.title = title.value();
    //    }
    //    po.logx = params["logx"].get_or(false);
    //    po.logy = params["logy"].get_or(false);
    //    po.palette = params["palette"].get_or(kRainBow);
    return po;
  };
  lua["finalize_input_data"] =
      sol::overload(finalizeManyInputData, finalizeInputData);
  lua["expand_data"] = expand;
  lua["save_pad"] = [](Pad *p, const std::string &s) { p->SaveAs(s.c_str()); };

  //    auto data_input = lua.new_usertype<PlotData>(
  //        "PlotData",
  //        sol::constructors<PlotData(), PlotData(THStack *), PlotData(TH1
  //        *)>());
}

int main(int argc, char *argv[]) {
  // TH1::AddDirectory(kFALSE);
  sol::state lua;

  lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table);
  makeBindings(lua);
  makeFreeBindings(lua);

  CLI::App app{ "Root plotter interface" };
  app.add_option("-f,--file", "A help string");
  CLI11_PARSE(app, argc, argv);
  lua.script_file("example.lua");
  return 0;
}
