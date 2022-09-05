#pragma once
#include "plot_element.h"

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
