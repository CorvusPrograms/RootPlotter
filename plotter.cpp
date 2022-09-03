#define SOL_ALL_SAFETIES_ON 1
#include <TCanvas.h>
#include <TError.h>
#include <TFile.h>
#include <TGraphAsymmErrors.h>
#include <TH1.h>
#include <THStack.h>
#include <TLegend.h>
#include <TStyle.h>
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
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"
#include "glob.hpp"

using KeyType = std::string;

using AnyHistogram = std::variant<TH1 *, THStack *>;

struct DataSource;

struct PlotElement {
    std::optional<std::pair<float, float>> xrange = std::nullopt,
                                           yrange = std::nullopt;
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

    virtual void setFillAtt(TAttFill *){};
    virtual void setMarkAtt(TAttMarker *){};
    virtual void setLineAtt(TAttLine *){};
    virtual void setTitle(const std::string &s) = 0;
    virtual void setupRanges() = 0;

    virtual void setRangeX(const std::pair<float, float> &range) {
        xrange = range;
    }
    virtual void setRangeY(const std::pair<float, float> &range) {
        yrange = range;
    }
    virtual std::optional<std::pair<float, float>> getRangeX() const {
        return xrange;
    }
    virtual std::optional<std::pair<float, float>> getRangeY() const {
        return yrange;
    }

    //   virtual void setXLabel(const std::string &s) { xlabel = s; }
    //   virtual std::string setXLabel(const std::string &s) const { return
    // xlabel; }
    //   virtual void setYLabel(const std::string &s) { ylabel = s; }
    //   virtual std::string setYLabel(const std::string &s) const { return
    // ylabel; }
    virtual ~PlotElement() = default;
};

static std::vector<std::unique_ptr<DataSource>> data_sources;
struct DataSource {
    std::unordered_set<std::string> tags;
    std::unordered_set<std::string> keys;
    std::string path;
    std::string name;
    int palette_idx = 1;
    TFile *file = nullptr;
    static DataSource *create(const std::string &p) {
        auto up = std::make_unique<DataSource>(p);
        DataSource *ret = up.get();
        data_sources.push_back(std::move(up));
        return ret;
    }

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

struct SourceSet {
    std::vector<DataSource *> sources;
    SourceSet() = default;
    SourceSet(const std::vector<DataSource *> dsv) : sources{dsv} {}
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

#define Builder(class, name, variable)

struct InputData {
    SourceSet *source_set = nullptr;
    bool normalize = false;
    float norm_to = 1.0f;
    bool stack = false;
    std::optional<std::pair<float, float>> yrange = std::nullopt,
                                           xrange = std::nullopt;
    InputData() = default;
    InputData(SourceSet *s) : source_set{s} {}
    InputData(SourceSet *s, bool n, float nt, bool st)
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

struct Histogram : public PlotElement {
    DataSource *source;
    TH1 *hist;
    Histogram(DataSource *s, TH1 *h) : source{s}, hist{h} {}
    virtual void addToLegend(TLegend *legend) {
        legend->AddEntry(hist, source->name.c_str());
    }
    virtual void setupRanges() {
        auto xr = getRangeX();
        auto yr = getRangeY();
        if (xr) {
            getXAxis()->SetRange(xr->first, xr->second);
        }
        if (yr) {
            getYAxis()->SetRange(yr->first, yr->second);
        }
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
        int pidx = source->palette_idx;
        fill_att->SetFillColor(gStyle->GetColorPalette(pidx));
    }
    virtual void setMarkAtt(TAttMarker *mark_att) {
        int pidx = source->palette_idx;
        mark_att->SetMarkerColor(gStyle->GetColorPalette(pidx));
    }
    virtual void setLineAtt(TAttLine *line_att) {
        int pidx = source->palette_idx;
        line_att->SetLineColor(gStyle->GetColorPalette(pidx));
    }
    virtual ~Histogram() = default;
};

struct Stack : public PlotElement {
    std::vector<DataSource *> sources;
    THStack *hist;
    Stack(const std::vector<DataSource *> s, THStack *h)
        : sources{s}, hist{h} {}

    virtual void addToLegend(TLegend *legend) {
        int i = 0;
        for (TObject *o : *hist->GetHists()) {
            auto h = static_cast<TH1 *>(o);
            legend->AddEntry(h, sources[i]->name.c_str());
            ++i;
        }
    }
    virtual void setupRanges() {
        auto xr = getRangeX();
        auto yr = getRangeY();
        if (xr) {
            getXAxis()->SetRange(xr->first, xr->second);
        }
        if (yr) {
            getYAxis()->SetRange(yr->first, yr->second);
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
        for (TObject *th : *hist->GetHists()) {
            auto h = static_cast<TH1 *>(th);
            auto pidx = sources[i]->palette_idx;
            h->SetFillColor(gStyle->GetColorPalette(pidx));
            h->SetMarkerColor(gStyle->GetColorPalette(pidx));
            h->SetLineColor(gStyle->GetColorPalette(pidx));
            ++i;
        }
        hist->Draw((s + " hist").c_str());
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
    SOType title = std::nullopt, xlabel = std::nullopt, ylabel = std::nullopt;
    bool show_stats = false;
    bool logx = false, logy = false;
    std::optional<std::pair<float, float>> xrange = std::nullopt,
                                           yrange = std::nullopt;
    int palette = kRainBow;
};

void makeBindings(sol::state &lua);

template <typename T, typename X = void>
struct underlying_optional {
    using type = T;
};
template <typename T>
struct underlying_optional<
    T, typename std::enable_if<
           std::is_same_v<std::optional<typename T::value_type>, T>>::type> {
    using type = typename T::value_type;
};

void makeBindings(sol::state &lua) {
#define BUILD(C, var)                                                         \
#var,                                                                     \
        sol::overload(                                                        \
            [](C *c, const underlying_optional <decltype(C::var)>::type &v) { \
                c->var = v;                                                   \
                return c;                                                     \
            },                                                                \
            [](C *c) { return c->var; })

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

    auto plot_options_type = lua.new_usertype<PlotOptions>(
        "PlotOptions",
        BUILD(PlotOptions, xlabel),
        BUILD(PlotOptions, ylabel),
        BUILD(PlotOptions, title),
        BUILD(PlotOptions, show_stats),
        BUILD(PlotOptions, logx),
        BUILD(PlotOptions, logy),
        BUILD(InputData, yrange),
        "xrange" , [](InputData *c, float x, float y) {     
                  c->xrange = {x,y};
                  return c;                                            
              },                                                       
        "yrange" , [](InputData *c, float x, float y) {     
                  c->yrange = {x,y};
                  return c;                                            
              },                                                       
        BUILD(PlotOptions, palette));
// clang-format on
#undef BUILD
}

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

void setAxisProperties(TAxis *xaxis, TAxis *yaxis) {
    assert(yaxis && xaxis);
    xaxis->SetLabelSize(12);
    yaxis->SetLabelSize(12);
    yaxis->SetTitleSize(16);
    xaxis->SetTitleSize(16);
    xaxis->SetLabelFont(43);
    yaxis->SetLabelFont(43);
    xaxis->SetTitleFont(43);
    yaxis->SetTitleFont(43);
}

inline TH1 *getHistogram(TH1 *h) { return h; }
inline TH1 *getHistogram(THStack *h) { return h->GetHistogram(); }

Pad *simplePlot(Pad *pad, std::vector<std::unique_ptr<PlotElement>> &data,
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
        if (opts.title) {
            pe->setTitle(opts.title.value());
        }
        if (opts.xlabel) {
            pe->getXAxis()->SetTitle(opts.xlabel->c_str());
        }
        if (opts.ylabel) {
            pe->getYAxis()->SetTitle(opts.ylabel->c_str());
        }
        setAxisProperties(pe->getXAxis(), pe->getYAxis());
        if (opts.xrange) {
            pe->getXAxis()->SetRangeUser(opts.xrange->first,
                                         opts.xrange->second);
        }
        if (opts.yrange) {
            pe->getYAxis()->SetRangeUser(opts.yrange->first,
                                         opts.yrange->second);
        }
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
    num->setMarkAtt(ratio_plot);
    num->setLineAtt(ratio_plot);
    ratio_plot->Draw();
    auto xaxis = ratio_plot->GetXaxis();
    auto yaxis = ratio_plot->GetYaxis();
    setAxisProperties(xaxis, yaxis);
    if (opts.xlabel) {
        xaxis->SetTitle(opts.xlabel->c_str());
    }
    if (opts.ylabel) {
        yaxis->SetTitle(opts.ylabel->c_str());
    }
    if (opts.xrange) {
        xaxis->SetLimits(opts.xrange->first, opts.xrange->second);
    } else {
        xaxis->SetLimits(num->getMin(), num->getMax());
    }
    if (opts.yrange) {
        yaxis->SetLimits(opts.yrange->first, opts.yrange->second);
    } else {
        yaxis->SetRangeUser(0, 1.5);
    }
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

std::vector<std::unique_ptr<PlotElement>> finalizeManyInputData(
    const std::vector<PlotterInput> &input) {
    std::vector<std::unique_ptr<PlotElement>> ret;
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
    lua["make_pad"] = newPlot;
    lua["finalize_input_data"] =
        sol::overload(finalizeManyInputData, finalizeInputData);
    lua["expand_data"] = expand;
    lua["plotpad"] = lua.create_table();
    lua["plotpad"]["save"] = [](Pad *p, const std::string &s) {
        p->SaveAs(s.c_str());
    };
    lua["plotpad"]["divide"] = [](Pad *p, int i, int j) { p->Divide(i, j); };
    lua["plotpad"]["divide"] = &TVirtualPad::Divide;
    lua["plotpad"]["rect"] = [](Pad *p, float f1, float f2, float f3,
                                float f4) { p->SetPad(f1, f2, f3, f4); };
    lua["plotpad"]["m_top"] = [](Pad &p, float f) { p.SetTopMargin(f); };
    lua["plotpad"]["m_bot"] = [](Pad &p, float f) { p.SetBottomMargin(f); };
    lua["plotpad"]["m_right"] = [](Pad &p, float f) { p.SetRightMargin(f); };
    lua["plotpad"]["m_left"] = [](Pad &p, float f) { p.SetLeftMargin(f); };
    lua["plotpad"]["cd"] = [](Pad *p, int i) { return p->cd(i); };
    lua["plotpad"]["update"] = [](Pad *p) { return p->Update(); };
    lua["create_options"] = [](const sol::table& params) {
        PlotOptions po;
        sol::optional<std::string> xlabel = params["xlabel"];
        if (xlabel) {
            po.xlabel = xlabel.value();
        }
        sol::optional<std::string> ylabel = params["ylabel"];
        if (ylabel) {
            po.ylabel = ylabel.value();
        }
        sol::optional<std::string> title = params["title"];
        if (title) {
            po.title = title.value();
        }
        auto xrt = params.get<sol::optional<sol::table>>("xrange");
        if (xrt) {
            using sof = sol::optional<float>;
            auto [xl, xu] = xrt.value().get<sof, sof>(1, 2);
            if (xl && xu) {
                fmt::print("Range ({},{})\n", xl.value(), xu.value());
                po.xrange = {xl.value(), xu.value()};
            }
        }
        po.logx = params["logx"].get_or(false);
        po.logy = params["logy"].get_or(false);
        po.palette = params["palette"].get_or(kRainBow);
        return po;
    };

    //    auto data_input = lua.new_usertype<PlotData>(
    //        "PlotData",
    //        sol::constructors<PlotData(), PlotData(THStack *), PlotData(TH1
    //        *)>());
}

int main(int argc, char *argv[]) {
    // TH1::AddDirectory(kFALSE);
    sol::state lua;
    gErrorIgnoreLevel = kFatal;

    lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table);
    makeBindings(lua);
    makeFreeBindings(lua);

    CLI::App app{"Root plotter interface"};
    app.add_option("-f,--file", "A help string");
    CLI11_PARSE(app, argc, argv);
    lua.script_file("example.lua");
    return 0;
}
