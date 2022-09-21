#include "plotters.h"

#include <fmt/format.h>

#include <filesystem>
#include <sol/sol.hpp>

#include "TAxis.h"
#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TLegend.h"
#include "TPad.h"
#include "TStyle.h"
#include "util.h"
#include "verbosity.h"

Pad::Pad() {
    p = new TCanvas();
    vPrintHigh("Creating owning pad {}: {}\n", fmt::ptr(this), fmt::ptr(p));
    owning = true;
}
Pad::Pad(TVirtualPad *pad) {
    p = pad;
    vPrintHigh("Creating nonowning pad {}: {}\n", fmt::ptr(this), fmt::ptr(p));
}

TVirtualPad *Pad::get() { return p; }
void Pad::cd() { get()->cd(); }
Pad Pad::getChild(int i) {
    auto ret = Pad(get()->GetPad(i));
    ret.drawn_elements = drawn_elements;
    return ret;
}

void Pad::setMarginTop(float f) { get()->SetTopMargin(f); }
void Pad::setMarginBottom(float f) { get()->SetBottomMargin(f); }
void Pad::setMarginRight(float f) { get()->SetRightMargin(f); }
void Pad::setMarginLeft(float f) { get()->SetLeftMargin(f); }
void Pad::divide(int i, int j) { get()->Divide(i, j); }
void Pad::update() { get()->Update(); }
void Pad::setRect(float f1, float f2, float f3, float f4) {
    get()->SetPad(f1, f2, f3, f4);
}
void Pad::save(const std::string &s) {
    vPrintHigh("Saving to file {}\n", s);
    std::filesystem::path path(s);
    // path /= s;
    std::filesystem::path parent = path.parent_path();
    if (!std::filesystem::is_directory(parent)) {
        vPrintHigh("Creating directory {}\n", parent.string());
        std::filesystem::create_directories(parent);
    }
    // assert(p != nullptr);
    get()->SaveAs(path.string().c_str());
}
Pad::~Pad() {
    if (owning) {
        vPrintHigh("Deleting owning pad {}: {}\n", fmt::ptr(this), fmt::ptr(p));
        delete p;
    } else {
        vPrintHigh("Deleting nonowning pad {}: {}\n", fmt::ptr(this),
                   fmt::ptr(p));
    }
}

Pad &simplePlot(Pad &pad, std::shared_ptr<PlotElementCollection> data,
                const PlotOptions &opts) {
    vPrintHigh("Executing simple plot\n");
    pad.cd();
    gStyle->SetPalette(opts.palette);
    if (!opts.show_stats) {
        gStyle->SetOptStat(0);
    }
    auto legend = new TLegend();
    int i = 0;
    for (auto &pe : *data) {
        if (opts.logx) {
            pad.get()->SetLogx();
        }
        if (opts.logy) {
            pad.get()->SetLogy();
        }
        if (i > 0) {
            pe->Draw("Same");
        } else {
            pe->Draw("");
        }
        maybe_fun(opts.title, [&pe](auto &&s) { pe->setTitle(s); });
        maybe_fun(opts.xlabel,
                  [&pe](auto &&s) { pe->getXAxis()->SetTitle(s.c_str()); });
        maybe_fun(opts.ylabel,
                  [&pe](auto &&s) { pe->getYAxis()->SetTitle(s.c_str()); });
        setAxisProperties(pe->getXAxis(), pe->getYAxis());

        maybe_fun(opts.title, [&pe](auto &&s) { pe->setTitle(s); });

        maybe_fun(opts.xrange, [&pe](auto &&s) {
            pe->getXAxis()->SetRangeUser(s.first, s.second);
        });
        maybe_fun(
            opts.yrange,
            [&pe](auto &&s) {
                pe->getYAxis()->SetRangeUser(s.first, s.second);
                pe->setMinRange(s.first);
                if (s.first < s.second) {
                    pe->setMaxRange(s.second);
                }
            },
            [&pe]() {
                pe->setMinRange(
                    std::max(pe->getMinRange() - 0.0001, 0.0000000001));
            });

        //  pe->addToLegend(legend);
        ++i;
    }
    setupLegend(legend);
    pad.drawn_elements.push_back(data);
    return pad;
}

Pad &ratioPlot(Pad &pad, std::shared_ptr<PlotElementCollection> plots,
               PlotOptions &opts) {
    if (plots->size() < 2) {
        throw std::runtime_error(
            "Attempted to create a ratio plot without at least 2 plots");
    }
    PlotElement *den = plots->at(0).get();
    for (std::size_t i = 1; i < plots->size(); ++i) {
        PlotElement *num = plots->at(i).get();
        pad.cd();
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
        if (opts.title) {
            ratio_plot->SetTitle(opts.title.value().c_str());
        }
        if (opts.xlabel) {
            xaxis->SetTitle(opts.xlabel->c_str());
        }
        if (opts.ylabel) {
            yaxis->SetTitle(opts.ylabel->c_str());
        }
        if (opts.xrange) {
            xaxis->SetLimits(opts.xrange->first, opts.xrange->second);
        } else {
            ratio_plot->GetHistogram()->GetXaxis()->SetLimits(
                opts.xrange->first, opts.xrange->second);
            xaxis->SetLimits(num->getMinDomain(), num->getMaxDomain());
        }

        if (opts.yrange) {
            yaxis->SetRangeUser(opts.yrange->first, opts.yrange->second);
        } else {
            yaxis->SetRangeUser(0, 1.5);
        }
    }
    pad.get()->SetFrameBorderMode(0);
    pad.get()->SetBorderMode(0);
    pad.get()->SetBorderSize(0);
    return pad;
}

// Pad *newPlot(int w, int h) {
//     auto c = new TCanvas();
//     c->SetCanvasSize(w, h);
//     return c;
// }
// Pad *newPlot() {
//     auto c = new TCanvas();
//     return c;
// }
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

void bindPlotters(sol::state &lua) {
    auto pad_type = lua.new_usertype<Pad>(
        "Pad", "set_margin_top", &Pad::setMarginTop, "set_margin_bot",
        &Pad::setMarginBottom, "set_margin_right", &Pad::setMarginRight,
        "set_margin_left", &Pad::setMarginLeft, "cd", &Pad::cd, "get_subpad",
        &Pad::getChild, "set_rect", &Pad::setRect, "update", &Pad::update,
        "divide", &Pad::divide, "save", &Pad::save);

    lua["plotters"] = lua.create_table();
    lua["simple"] = simplePlot;
    lua["ratio_plot"] = ratioPlot;
    lua["print_totals"] = printTotals;
    //  lua["make_pad"] = sol::overload<Pad *(), Pad *(int, int)>(newPlot,
    //  newPlot); lua["plotpad"] = lua.create_table(); lua["plotpad"]["save"] =
    //  [&lua](Pad *p, const std::string &s) {
    //      std::string outbasepath = lua["OUTPUT_BASE_PATH"];
    //      std::filesystem::path path(outbasepath);
    //      path /= s;
    //      std::filesystem::path parent = path.parent_path();
    //      if (!std::filesystem::is_directory(parent)) {
    //          std::filesystem::create_directories(parent);
    //      }
    //      p->SaveAs(path.string().c_str());
    //  };
    //  lua["plotpad"]["divide"] = [](Pad *p, int i, int j) { p->Divide(i, j);
    //  }; lua["plotpad"]["divide"] = &TVirtualPad::Divide;
    //  lua["plotpad"]["rect"] = [](Pad *p, float f1, float f2, float f3,
    //                              float f4) { p->SetPad(f1, f2, f3, f4); };
    //  lua["plotpad"]["m_top"] = [](Pad &p, float f) { p.SetTopMargin(f); };
    //  lua["plotpad"]["m_bot"] = [](Pad &p, float f) { p.SetBottomMargin(f); };
    //  lua["plotpad"]["m_right"] = [](Pad &p, float f) { p.SetRightMargin(f);
    //  }; lua["plotpad"]["m_left"] = [](Pad &p, float f) { p.SetLeftMargin(f);
    //  }; lua["plotpad"]["cd"] = [](Pad *p, int i) { return p->cd(i); };
    //  lua["plotpad"]["update"] = [](Pad *p) { return p->Update(); };
    lua["create_options"] = [](const sol::table &params) {
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
                po.xrange = {xl.value(), xu.value()};
            }
        }
        auto yrt = params.get<sol::optional<sol::table>>("yrange");
        if (yrt) {
            using sof = sol::optional<float>;
            auto [yl, yu] = yrt.value().get<sof, sof>(1, 2);
            if (yl && yu) {
                po.yrange = {yl.value(), yu.value()};
            }
        }
        po.logx = params["logx"].get_or(false);
        po.logy = params["logy"].get_or(false);
        po.palette = params["palette"].get_or(kRainBow);
        return po;
    };

    auto plot_options_type = lua.new_usertype<PlotOptions>(
        "PlotOptions", BUILD(PlotOptions, xlabel), BUILD(PlotOptions, ylabel),
        BUILD(PlotOptions, title), BUILD(PlotOptions, show_stats),
        BUILD(PlotOptions, logx), BUILD(PlotOptions, logy),
        BUILD(InputData, yrange), "xrange",
        [](InputData *c, float x, float y) {
            c->xrange = {x, y};
            return c;
        },
        "yrange",
        [](InputData *c, float x, float y) {
            c->yrange = {x, y};
            return c;
        },
        BUILD(PlotOptions, palette));
}

void printTotals(std::shared_ptr<PlotElementCollection> data, bool entries) {
    if (entries) {
        for (const auto &pe : *data) {
            fmt::print("({},{}) -- Totals: {}\n", pe->getSourceID(),
                       pe->getName(), pe->getTotals()->GetEntries());
        }
    } else {
        for (const auto &pe : *data) {
            fmt::print("({},{}) -- Totals: {}\n", pe->getSourceID(),
                       pe->getName(), pe->getTotals()->Integral());
        }
    }
}
