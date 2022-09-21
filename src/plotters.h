#pragma once
#include "plot_element.h"

struct Pad {
    TVirtualPad *p;
    std::vector<std::shared_ptr<PlotElementCollection>> drawn_elements;
    Pad();
    Pad(TVirtualPad *p);
    TVirtualPad *get();
    void cd();
    Pad *getChild(int i);
    void setMarginTop(float f);
    void setMarginBottom(float f);
    void setMarginRight(float f);
    void setMarginLeft(float f);
    void divide(int i, int j);
    void update();
    void setRect(float f1, float f2, float f3, float f4);
    void save(const std::string &s);
    virtual ~Pad() = default;
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

Pad &simplePlot(Pad &pad, std::shared_ptr<PlotElementCollection> &data,
                const PlotOptions &opts);

Pad &ratioPlot(Pad &pad, std::shared_ptr<PlotElementCollection> &plots,
               PlotOptions &opts);
void printTotals(std::vector<std::unique_ptr<PlotElement>> &data, bool entries);

Pad *newPlot(int w, int h);
Pad *newPlot();
void setupLegend(TLegend *legend);
void setAxisProperties(TAxis *xaxis, TAxis *yaxis);

namespace sol {
class state;
}
void bindPlotters(sol::state &lua);
