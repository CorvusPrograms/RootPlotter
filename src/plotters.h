#pragma once
#include "plot_element.h"

struct PlotOptions {
    using SOType = std::optional<std::string>;
    SOType title = std::nullopt, xlabel = std::nullopt, ylabel = std::nullopt;
    bool show_stats = false;
    bool logx = false, logy = false;
    std::optional<std::pair<float, float>> xrange = std::nullopt,
                                           yrange = std::nullopt;
    int palette = kRainBow;
};

Pad *simplePlot(Pad *pad, std::vector<std::unique_ptr<PlotElement>> &data,
                const PlotOptions &opts);

Pad *ratioPlot(Pad *pad, PlotElement *num, PlotElement *den, PlotOptions &opts);

Pad *newPlot(int w, int h);
Pad *newPlot();
void setupLegend(TLegend *legend);
void setAxisProperties(TAxis *xaxis, TAxis *yaxis);

namespace sol {
class state;
}
void bindPlotters(sol::state &lua);
