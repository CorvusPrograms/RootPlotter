#include "plotters.h"

#include <fmt/format.h>

#include <filesystem>
#include <sol/sol.hpp>

#include "TCanvas.h"
#include "TAxis.h"
#include "TGraphAsymmErrors.h"
#include "TLegend.h"
#include "TPad.h"
#include "TStyle.h"
#include "util.h"

Pad *simplePlot(Pad *pad, std::vector<std::unique_ptr<PlotElement>> &data,
                const PlotOptions &opts) {
    pad->cd();
    gStyle->SetPalette(opts.palette);
    if (!opts.show_stats) {
        gStyle->SetOptStat(0);
    }
    auto legend = new TLegend();
    int i = 0;
    for (auto &pe : data) {
        if (opts.logx) {
            pad->SetLogx();
        }
        if (opts.logy) {
            pad->SetLogy();
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
            }

        );

        pe->addToLegend(legend);
        ++i;
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
        ratio_plot->GetHistogram()->GetXaxis()->SetLimits(opts.xrange->first,
                                                          opts.xrange->second);
        xaxis->SetLimits(num->getMinDomain(), num->getMaxDomain());
    }

    if (opts.yrange) {
        yaxis->SetRangeUser(opts.yrange->first, opts.yrange->second);
    } else {
        yaxis->SetRangeUser(0, 1.5);
    }
    pad->SetFrameBorderMode(0);
    pad->SetBorderMode(0);
    pad->SetBorderSize(0);
    return pad;
}

Pad *newPlot(int w, int h) {
    auto c = new TCanvas();
    c->SetCanvasSize(w, h);
    return c;
}
Pad *newPlot() {
    auto c = new TCanvas();
    return c;
}
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
    lua["plotters"] = lua.create_table();
    lua["simple"] = simplePlot;
    lua["ratio_plot"] = ratioPlot;
    lua["make_pad"] = sol::overload<Pad *(), Pad *(int, int)>(newPlot, newPlot);
    lua["plotpad"] = lua.create_table();
    lua["plotpad"]["save"] = [](Pad *p, const std::string &s) {
        std::filesystem::path path(s);
        std::filesystem::path parent = path.parent_path();
        if (!std::filesystem::is_directory(parent)) {
            std::filesystem::create_directories(parent);
        }
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

void bindPalettes(sol::state &lua) {
    lua["palettes"] = lua.create_table();
    lua["palettes"]["DeepSea"] = 51;
    lua["palettes"]["GreyScale"] = 52;
    lua["palettes"]["DarkBodyRadiator"] = 53;
    lua["palettes"]["BlueYellow"] = 54;
    lua["palettes"]["RainBow"] = 55;
    lua["palettes"]["InvertedDarkBodyRadiator"] = 56;
    lua["palettes"]["Bird"] = 57;
    lua["palettes"]["Cubehelix"] = 58;
    lua["palettes"]["GreenRedViolet"] = 59;
    lua["palettes"]["BlueRedYellow"] = 60;
    lua["palettes"]["Ocean"] = 61;
    lua["palettes"]["ColorPrintableOnGrey"] = 62;
    lua["palettes"]["Alpine"] = 63;
    lua["palettes"]["Aquamarine"] = 64;
    lua["palettes"]["Army"] = 65;
    lua["palettes"]["Atlantic"] = 66;
    lua["palettes"]["Aurora"] = 67;
    lua["palettes"]["Avocado"] = 68;
    lua["palettes"]["Beach"] = 69;
    lua["palettes"]["BlackBody"] = 70;
    lua["palettes"]["BlueGreenYellow"] = 71;
    lua["palettes"]["BrownCyan"] = 72;
    lua["palettes"]["CMYK"] = 73;
    lua["palettes"]["Candy"] = 74;
    lua["palettes"]["Cherry"] = 75;
    lua["palettes"]["Coffee"] = 76;
    lua["palettes"]["DarkRainBow"] = 77;
    lua["palettes"]["DarkTerrain"] = 78;
    lua["palettes"]["Fall"] = 79;
    lua["palettes"]["FruitPunch"] = 80;
    lua["palettes"]["Fuchsia"] = 81;
    lua["palettes"]["GreyYellow"] = 82;
    lua["palettes"]["GreenBrownTerrain"] = 83;
    lua["palettes"]["GreenPink"] = 84;
    lua["palettes"]["Island"] = 85;
    lua["palettes"]["Lake"] = 86;
    lua["palettes"]["LightTemperature"] = 87;
    lua["palettes"]["LightTerrain"] = 88;
    lua["palettes"]["Mint"] = 89;
    lua["palettes"]["Neon"] = 90;
    lua["palettes"]["Pastel"] = 91;
    lua["palettes"]["Pearl"] = 92;
    lua["palettes"]["Pigeon"] = 93;
    lua["palettes"]["Plum"] = 94;
    lua["palettes"]["RedBlue"] = 95;
    lua["palettes"]["Rose"] = 96;
    lua["palettes"]["Rust"] = 97;
    lua["palettes"]["SandyTerrain"] = 98;
    lua["palettes"]["Sienna"] = 99;
    lua["palettes"]["Solar"] = 100;
    lua["palettes"]["SouthWest"] = 101;
    lua["palettes"]["StarryNight"] = 102;
    lua["palettes"]["Sunset"] = 103;
    lua["palettes"]["TemperatureMap"] = 104;
    lua["palettes"]["Thermometer"] = 105;
    lua["palettes"]["Valentine"] = 106;
    lua["palettes"]["VisibleSpectrum"] = 107;
    lua["palettes"]["WaterMelon"] = 108;
    lua["palettes"]["Cool"] = 109;
    lua["palettes"]["Copper"] = 110;
    lua["palettes"]["GistEarth"] = 111;
    lua["palettes"]["Viridis"] = 112;
    lua["palettes"]["Cividis"] = 113;
}
