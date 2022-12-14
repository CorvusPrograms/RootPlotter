#include "plotting.h"

#include <TStyle.h>

namespace rootp {
void setFillAtt(const Style &style, TAttFill *fill_att) {
    if (!(style.mode & Style::Mode::Fill)) {
        fill_att->SetFillColorAlpha(0, 0);
        return;
    }
    if (style.color) {
        auto color = style.color.value();
        fill_att->SetFillColor(color);
    } else if (style.palette_idx) {
        auto color = gStyle->GetColorPalette(style.palette_idx.value());
        fill_att->SetFillColor(color);
    }
    if (style.fill_style) {
        fill_att->SetFillStyle(style.fill_style.value());
    }
}

void setMarkAtt(const Style &style, TAttMarker *mark_att) {
    if (!(style.mode | Style::Mode::Marker)) {
        return;
    }
    if (style.color) {
        auto color = style.color.value();
        mark_att->SetMarkerColor(color);
    } else if (style.palette_idx) {
        auto color = gStyle->GetColorPalette(style.palette_idx.value());
        mark_att->SetMarkerColor(color);
    }
    if (style.marker_style) {
        mark_att->SetMarkerStyle(style.marker_style.value());
    }
    if (style.marker_size) {
        mark_att->SetMarkerSize(style.marker_size.value());
    }
}

void setLineAtt(const Style &style, TAttLine *line_att) {
    if (!(style.mode | Style::Mode::Line)) {
        return;
    }
    if (style.color) {
        auto color = style.color.value();
        line_att->SetLineColor(color);
    } else if (style.palette_idx) {
        auto color = gStyle->GetColorPalette(style.palette_idx.value());
        line_att->SetLineColor(color);
    }
    if (style.line_style) {
        line_att->SetLineStyle(style.line_style.value());
    }
    if (style.line_width) {
        line_att->SetLineWidth(style.line_width.value());
    }
}

void applyCommonOptions(TVirtualPad *pad, const CommonOptions &opts) {
    if (opts.logx) pad->SetLogx();
    if (opts.logy) pad->SetLogy();
}
void savePadTo(TVirtualPad *p, const std::filesystem::path &path) {
    std::filesystem::path parent = path.parent_path();
    if (!std::filesystem::is_directory(parent)) {
        std::filesystem::create_directories(parent);
    }
    vPrint(VerbosityLevel::Medium, "Saving to path {}\n", path.string());
    p->SaveAs(path.string().c_str());
}
void saveDrawPad(const DrawPad &p, const std::filesystem::path &path) {
    savePadTo(p.pad.get(), path);
}
void saveDrawPad(const DrawPad &p, const std::string &path) {
    saveDrawPad(p, std::filesystem::path(path));
}

void auto_range(DrawPad &dp, float other_min, float other_max) {
    float old_min = dp.master->GetMinimum(0.000001);
    float old_max = dp.master->GetMaximum();
    dp.min_y = std::min(old_min, other_min);
    dp.max_y = std::max(old_max, other_max);
    //  dp.master->SetMinimum(dp.min_y);
    //  dp.master->SetMaximum(dp.max_y);
}

void plotStandard(DrawPad &dp, int subpad, const std::vector<PlotData> &data,
                  const CommonOptions &options) {
    auto pad = dp.pad.get();
    pad->cd(subpad);
    int i = 0;
    for (const auto &d : data) {
        if (!dp.master) {
            dp.master = d.hist.get();
        }
        applyCommonOptions(d.hist.get(), options);
        applyCommonOptions(pad, options);
        dp.objects.emplace_back(d.hist);

        setMarkAtt(d.style, d.hist.get());
        setLineAtt(d.style, d.hist.get());
        setFillAtt(d.style, d.hist.get());
        d.hist->Draw("Same E");
        auto_range(dp, d.hist->GetMinimum(0.00001), d.hist->GetMaximum());
        ++i;
        pad->Update();
    }
}

void addToLegend(TLegend *l, const std::vector<PlotData> &data) {
    for (const auto &d : data) {
        l->AddEntry(d.hist.get(), d.source_name.c_str());
    }
};

void addLegendToPad(TLegend *legend, DrawPad &p, int subpad) {
    p.pad->cd(subpad);
    setupLegend(legend);
}
TLegend *newLegend(DrawPad &p) {
    auto legend = new TLegend;
    p.objects.emplace_back(legend);
    return legend;
}

void plotStack(DrawPad &dp, int subpad, const std::vector<PlotData> &data,
               const CommonOptions &options) {
    auto pad = dp.pad.get();
    pad->cd(subpad);
    auto stack = new THStack;
    int i = 0;
    bool needs_fill = false;
    for (const PlotData &d : data) {
        applyCommonOptions(d.hist.get(), options);
        setMarkAtt(d.style, d.hist.get());
        setLineAtt(d.style, d.hist.get());
        setFillAtt(d.style, d.hist.get());
        stack->Add(d.hist.get());
        if (d.style.mode & Style::Mode::Fill) {
            needs_fill = true;
        }
        ++i;
        dp.objects.emplace_back(d.hist);
    }
    std::string hist_options = "";
    if (!dp.master) {
        hist_options += "SAME";
    }
    if (needs_fill) {
        hist_options += "hist";
    }
    stack->Draw(hist_options.c_str());
    if (!dp.master) {
        dp.master = stack->GetHistogram();
    }
    auto_range(dp, stack->GetMinimum(), stack->GetMaximum());
    stack->SetMinimum(0.0001);
    stack->SetMaximum(dp.max_y);
    applyCommonOptions(pad, options);
    applyCommonOptions(stack, options);
}

void executePlot(DrawPad &p, const PlotDescription &plot) {
    p.pad->Divide(plot.structure.first, plot.structure.second);
    for (const auto &[pad_idx, desc] : plot.pads) {
        for (const auto &[func, data] : desc.drawers) {
            func(p, pad_idx, data, desc.options);
        }
    }
}

namespace transforms {
std::shared_ptr<TH1> normalize(const TH1 *hist, float val) {
    auto ret = std::unique_ptr<TH1>(static_cast<TH1 *>(hist->Clone()));
    auto integral = ret->Integral();
    ret->Scale(val / integral);
    return ret;
}
}  // namespace transforms
}  // namespace rootp
