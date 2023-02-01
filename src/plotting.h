#pragma once

#include <TCanvas.h>
#include <TH1.h>
#include <THStack.h>
#include <TLegend.h>

#include <variant>

#include "data.h"
#include "style.h"

namespace rootp {

namespace transforms {
std::shared_ptr<TH1> normalize(const TH1 *hist, float val);

template <typename Iter, typename IterOut>
void createNormed(Iter begin, Iter end, IterOut out, float val) {
    std::transform(begin, end, out, [val](const PlotData &p) {
        PlotData n{normalize(p.hist.get(), val), p.style, p.source_name,
                   p.name};
        return n;
    });
}
template <typename Iter>
auto removeEmpty(Iter begin, Iter end) {
    return std::remove_if(begin, end, [](const PlotData &p) {
        return p.hist->GetEntries() == 0;
    });
}

template <typename Iter>
void sortIntegral(Iter begin, Iter end) {
    std::sort(begin, end, [](const auto &x, const auto &y) {
        return x.hist->Integral() < y.hist->Integral();
    });
}
}  // namespace transforms

struct DrawPad {
    std::unique_ptr<TVirtualPad> pad = std::make_unique<TCanvas>();
    bool init = false;
    std::variant<TH1 *, THStack *> master;
    float min_y, max_y;
    std::vector<std::shared_ptr<TObject>> objects;
};

inline void setupLegend(TLegend *legend) {
    legend->SetX1(0.7);
    legend->SetY1(0.7);
    legend->SetX2(0.90);
    legend->SetY2(0.90);
    legend->SetHeader("Samples", "C");
    legend->Draw();
}

// void setFillAtt(const Style &style, TAttFill *fill_att);
// void setMarkAtt(const Style &style, TAttMarker *mark_att);
// void setLineAtt(const Style &style, TAttLine *line_att);

struct CommonOptions {
    std::pair<float, float> xrange;
    std::pair<float, float> yrange;
    std::optional<std::string> plot_title;
    std::optional<std::string> x_label;
    std::optional<std::string> y_label;
    std::optional<float> normalize;
    bool logx = false;
    bool logy = false;
};

template <typename T>
void applyCommonOptions(T *hist, const CommonOptions &opts) {
    if (opts.x_label) hist->GetXaxis()->SetTitle(opts.x_label->c_str());
    if (opts.y_label) hist->GetYaxis()->SetTitle(opts.y_label->c_str());
    if (opts.plot_title) hist->SetTitle(opts.plot_title->c_str());
}

void applyCommonOptions(TVirtualPad *pad, const CommonOptions &opts);

void plotStandard(DrawPad &dp, int subpad, const std::vector<PlotData> &data,
                  const CommonOptions &options);
void plotStack(DrawPad &dp, int subpad, const std::vector<PlotData> &data,
               const CommonOptions &options);

TLegend *newLegend(DrawPad &p);
void addToLegend(TLegend *, const std::vector<PlotData> &data);
void addLegendToPad(TLegend *, DrawPad &p, int subpad);

void savePadTo(TVirtualPad *p, const std::filesystem::path &path);
void saveDrawPad(const DrawPad &p, const std::filesystem::path &path);
void saveDrawPad(const DrawPad &p, const std::string &path);

struct PadDescription {
    using DrawFunc = std::function<void(DrawPad &dp, int subpad,
                                        const std::vector<PlotData> &data,
                                        const CommonOptions &options)>;

    CommonOptions options;
    std::vector<std::pair<DrawFunc, std::vector<PlotData>>> drawers;
};

struct PlotDescription {
    std::pair<int, int> structure;
    std::unordered_map<int, PadDescription> pads;
};

void executePlot(DrawPad &p, const PlotDescription &plot);

}  // namespace rootp
