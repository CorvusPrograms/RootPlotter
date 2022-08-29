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
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

using KeyType = std::string;

using AnyHistogram = std::variant<TH1 *, THStack *>;

struct DataSource;

struct HistData {
    AnyHistogram hist;
    DataSource *source;
};

struct DataSource {
    std::string path;
    std::string name;
    std::unordered_set<std::string> tags;
    std::unordered_set<std::string> keys;
    TFile *file = nullptr;
    DataSource() = default;
    DataSource(const std::string &p, const std::string &n) : DataSource(p) {
        name = n;
    }

    DataSource(const std::string &p) : path{p} {
        load();
        getKeys();
    }
    std::string to_string() {
        return fmt::format("DataSource({},{})", path, tags);
    }
    void load();
    void getKeys();
    HistData getHist(const std::string &name);
};

HistData DataSource::getHist(const std::string &name) {
    assert(file != nullptr);
    TH1 *hist = static_cast<TH1 *>(file->Get(name.c_str()));
    return {hist, this};
}

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
    std::optional<std::string> match;
    std::vector<DataSource *> sources;
    SourceSet() = default;
    SourceSet(const std::string &m) : match{m} {}
    SourceSet(const std::vector<DataSource *> dsv) : sources{dsv} {}
    std::string to_string() {
        std::vector<std::string> temp;
        for (const auto &ds : sources) {
            temp.push_back(ds->path);
        }
        if (match)
            return fmt::format("SourceSet({},{})", match.value(), temp);
        else
            return fmt::format("SourceSet({})", temp);
    }
};

void makeBindings(sol::state &lua);

void makeBindings(sol::state &lua) {
    auto data_source_type = lua.new_usertype<DataSource>(
        "DataSource",
        sol::constructors<DataSource(), DataSource(std::string),
                          DataSource(std::string, std::string)>(),
        "path", &DataSource::path, "tags", &DataSource::tags, "keys",
        &DataSource::keys, "get_hist", &DataSource::getHist);
    auto source_set_type = lua.new_usertype<SourceSet>(
        "SourceSet",
        sol::constructors<SourceSet(), SourceSet(std::string),
                          SourceSet(std::vector<DataSource *>)>(),
        "match", &SourceSet::match, "sources", &SourceSet::sources);
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
    xaxis->SetLabelSize(12);
    yaxis->SetLabelSize(12);
    yaxis->SetTitleSize(16);
    xaxis->SetTitleSize(16);
    xaxis->SetLabelFont(43);
    yaxis->SetLabelFont(43);
    xaxis->SetTitleFont(43);
    yaxis->SetTitleFont(43);
    xaxis->SetTitleOffset(1.2);
    yaxis->SetTitleOffset(3);
}


Pad *simplePlot(Pad *pad, std::vector<HistData> data) {
    pad->cd();
    auto legend = new TLegend();
    int i = 0;
    for (const HistData &hd : data) {
        std::visit(
            [legend, &hd, &i](auto &&histogram) {
                // histogram->SetFillColor(gStyle->GetColorPalette(i));
                // histogram->SetLineColor(gStyle->GetColorPalette(i));
                // histogram->SetMarkerColor(gStyle->GetColorPalette(i));
                histogram->Draw("Same");
                legend->AddEntry(histogram, hd.source->name.c_str());
                ++i;
            },
            hd.hist);
    }
    setupLegend(legend);
    fmt::print("Creating histogram\n");
    return pad;
    //    pad->SaveAs("simple.pdf");
};

TH1 *stackTotal(THStack *stack) {
    TList *stackHists = stack->GetHists();
    TH1 *tmpHist = (TH1 *)stackHists->At(0)->Clone();
    tmpHist->Reset();
    for (int i = 0; i < stackHists->GetSize(); ++i) {
        tmpHist->Add((TH1 *)stackHists->At(i));
    }
    return tmpHist;
}

TH1 *getTotals(HistData d) {
    if (std::holds_alternative<THStack *>(d.hist)) {
        return stackTotal(std::get<THStack *>(d.hist));
    } else {
        return std::get<TH1 *>(d.hist);
    }
}

Pad *ratioPlot(Pad *pad, HistData num, HistData den) {
    pad->cd();
    auto ratio_plot = new TGraphAsymmErrors(getTotals(num), getTotals(den));
    ratio_plot->Draw();
    pad->SaveAs("test.pdf");
    return pad;
}

Pad *newPlot() { return new TCanvas(); }

void makeFreeBindings(sol::state &lua) {
    lua["plotters"] = lua.create_table();
    lua["simple"] = simplePlot;
    lua["ratio_plot"] = ratioPlot;
    lua["make_plot"] = newPlot;

    //    auto data_input = lua.new_usertype<PlotData>(
    //        "PlotData",
    //        sol::constructors<PlotData(), PlotData(THStack *), PlotData(TH1
    //        *)>());
}

int main(int argc, char *argv[]) {
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::string);
    makeBindings(lua);
    makeFreeBindings(lua);

    CLI::App app{"Root plotter interface"};
    app.add_option("-f,--file", "A help string");
    CLI11_PARSE(app, argc, argv);
    auto ds1 = DataSource(
        "../RPVResearch/data/08_15_2022_FixedBackground/"
        "2018_RPV2W_mS-450_mB-0.root");
    auto ds2 = DataSource(
        "../RPVResearch/data/08_15_2022_FixedBackground/"
        "2018_RPV2W_mS-650_mB-0.root");
    auto ss1 = SourceSet({&ds1});
    Pad *c = new TCanvas();
    c->Divide(1, 2);
    simplePlot(c->cd(1), {ds1.getHist("WPt"), ds2.getHist("WPt")});
    ratioPlot(c->cd(2), ds2.getHist("WPt"), ds1.getHist("WPt"));
    c->SaveAs("simple.pdf");

    // lua.script_file("example.lua");
    return 0;
}
