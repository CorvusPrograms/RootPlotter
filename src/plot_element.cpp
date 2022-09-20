#include "plot_element.h"

#include <TLegend.h>
#include <TList.h>
#include <TStyle.h>

#include <sol/sol.hpp>

#include "util.h"

Histogram::Histogram(DataSource *s, std::unique_ptr<TH1> &&h)
    : source{s}, hist{std::move(h)} {}
void Histogram::addToLegend(TLegend *legend) {
    legend->AddEntry(hist.get(), source->name.c_str());
}

std::string Histogram::getSourceID() const { return source->name; }

void Histogram::setupRanges() {
    auto xr = getRangeX();
    auto yr = getRangeY();
    if (xr) {
        getXAxis()->SetRange(xr->first, xr->second);
    }
    if (yr) {
        getYAxis()->SetRange(yr->first, yr->second);
    }
}
std::string Histogram::getName() const { return hist->GetName(); }
void Histogram::setMinRange(float v) { hist->SetMinimum(v); }
void Histogram::setMaxRange(float v) { hist->SetMaximum(v); }
float Histogram::getMinRange() const { return hist->GetMinimum(); }
float Histogram::getMaxRange() const { return hist->GetMaximum(); }
TH1 *Histogram::getHistogram() { return hist.get(); }
TH1 *Histogram::getTotals() { return hist.get(); }
float Histogram::getIntegral() { return hist->Integral(); }
TAxis *Histogram::getXAxis() { return hist->GetXaxis(); }
TAxis *Histogram::getYAxis() { return hist->GetYaxis(); }
void Histogram::setTitle(const std::string &s) { hist->SetTitle(s.c_str()); }
void Histogram::Draw(const std::string &s) {
    setMarkAtt(hist.get());
    setFillAtt(hist.get());
    setLineAtt(hist.get());
    hist->Draw(s.c_str());
}
void Histogram::setFillStyle() {}
void Histogram::setMarkerStyle() {}
void Histogram::setLineStyle() {}

float Histogram::getMinDomain() {
    auto xaxis = getXAxis();
    return xaxis->GetBinLowEdge(xaxis->GetFirst());
}
float Histogram::getMaxDomain() {
    auto xaxis = getXAxis();
    return xaxis->GetBinUpEdge(xaxis->GetLast());
}
std::string Histogram::to_string() const { return source->name; }

void Histogram::setFillAtt(TAttFill *fill_att) {
    auto &style = source->style;
    if (!(style.mode & Style::Mode::Fill)) {
        return;
    }
    if (source->style.palette_idx) {
        if (style.palette_idx) {
            auto color = gStyle->GetColorPalette(style.palette_idx.value());
            fill_att->SetFillColor(color);
        }
        if (style.fill_style) {
            fill_att->SetFillStyle(style.fill_style.value());
        }
    }
}
void Histogram::setMarkAtt(TAttMarker *mark_att) {
    auto &style = source->style;
    if (!(style.mode | Style::Mode::Marker)) {
        return;
    }
    if (source->style.palette_idx) {
        if (style.palette_idx) {
            auto color = gStyle->GetColorPalette(style.palette_idx.value());
            mark_att->SetMarkerColor(color);
        }
    }
    if (style.marker_style) {
        mark_att->SetMarkerStyle(style.marker_style.value());
    }
    if (style.marker_size) {
        mark_att->SetMarkerSize(style.marker_size.value());
    }
}
void Histogram::setLineAtt(TAttLine *line_att) {
    auto &style = source->style;
    if (!(style.mode | Style::Mode::Line)) {
        return;
    }
    if (source->style.palette_idx) {
        if (style.palette_idx) {
            auto color = gStyle->GetColorPalette(style.palette_idx.value());
            line_att->SetLineColor(color);
        }
    }
    if (style.line_style) {
        line_att->SetLineStyle(style.line_style.value());
    }
    if (style.line_width) {
        line_att->SetLineWidth(style.line_width.value());
    }
}

void bindPlotElements(sol::state &lua) {}

Stack::Stack(const std::vector<DataSource *> s, std::unique_ptr<THStack> &&h,
             std::vector<std::unique_ptr<TH1>> &&hs)
    : sources{s}, hist{std::move(h)}, histos{std::move(hs)} {}

void Stack::addToLegend(TLegend *legend) {
    int i = 0;
    for (TObject *o : *hist->GetHists()) {
        auto h = static_cast<TH1 *>(o);
        legend->AddEntry(h, sources[i]->name.c_str());
        ++i;
    }
}
void Stack::setupRanges() {
    auto xr = getRangeX();
    auto yr = getRangeY();
    if (xr) {
        getXAxis()->SetRange(xr->first, xr->second);
    }
    if (yr) {
        getYAxis()->SetRange(yr->first, yr->second);
    }
}

void Stack::setMinRange(float v) { hist->SetMinimum(v); }
void Stack::setMaxRange(float v) { hist->SetMaximum(v); }
void Stack::setTitle(const std::string &s) { hist->SetTitle(s.c_str()); }
TH1 *Stack::getHistogram() { return hist->GetHistogram(); }
TH1 *Stack::getTotals() {
    TList *stackHists = hist->GetHists();
    TH1 *tmpHist = (TH1 *)stackHists->At(0)->Clone();
    tmpHist->Reset();
    for (int i = 0; i < stackHists->GetSize(); ++i) {
        tmpHist->Add((TH1 *)stackHists->At(i));
    }
    return tmpHist;
}

float Stack::getIntegral() { return getTotals()->Integral(); }
TAxis *Stack::getXAxis() { return hist->GetXaxis(); }
TAxis *Stack::getYAxis() { return hist->GetYaxis(); }
void Stack::Draw(const std::string &s) {
    int i = 0;
    for (TObject *th : *hist->GetHists()) {
        auto h = static_cast<TH1 *>(th);
        if (sources[i]->style.palette_idx) {
            auto pidx = sources[i]->style.palette_idx.value();
            h->SetFillColor(gStyle->GetColorPalette(pidx));
            h->SetMarkerColor(gStyle->GetColorPalette(pidx));
            h->SetLineColor(gStyle->GetColorPalette(pidx));
        }
        ++i;
    }
    hist->Draw((s + " hist").c_str());
}

std::string Stack::getName() const { return hist->GetName(); }

std::string Stack::to_string() const { return sources[0]->name; }
float Stack::getMinDomain() {
    auto xaxis = getXAxis();
    return xaxis->GetBinLowEdge(xaxis->GetFirst());
}
float Stack::getMaxDomain() {
    auto xaxis = getXAxis();
    return xaxis->GetBinUpEdge(xaxis->GetLast());
}

float Stack::getMinRange() const { return hist->GetMinimum(); }
float Stack::getMaxRange() const { return hist->GetMaximum(); }

std::string Stack::getSourceID() const {
    std::string ret = "Total()";
    for (const auto &s : sources) {
        ret += s->name + ",";
    }
    return ret;
}
