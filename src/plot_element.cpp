#include "plot_element.h"

#include <sol/sol.hpp>
#include <TLegend.h>
#include <TStyle.h>
#include <TList.h>
#include "util.h"

Histogram::Histogram(DataSource *s, TH1 *h) : source{s}, hist{h} {}
void Histogram::addToLegend(TLegend *legend) {
    legend->AddEntry(hist, source->name.c_str());
}
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
TH1 *Histogram::getHistogram() { return hist; }
TH1 *Histogram::getTotals() { return hist; }
float Histogram::getIntegral() { return hist->Integral(); }
TAxis *Histogram::getXAxis() { return hist->GetXaxis(); }
TAxis *Histogram::getYAxis() { return hist->GetYaxis(); }
void Histogram::setTitle(const std::string &s) { hist->SetTitle(s.c_str()); }
void Histogram::Draw(const std::string &s) {
    setMarkAtt(hist);
    setFillAtt(hist);
    setLineAtt(hist);
    hist->Draw(s.c_str());
}
void Histogram::setFillStyle() {}
void Histogram::setMarkerStyle() {}
void Histogram::setLineStyle() {}

float Histogram::getMin() {
    auto xaxis = getXAxis();
    return xaxis->GetBinLowEdge(xaxis->GetFirst());
}
float Histogram::getMax() {
    auto xaxis = getXAxis();
    return xaxis->GetBinUpEdge(xaxis->GetLast());
}
std::string Histogram::to_string() const { return source->name; }

void Histogram::setFillAtt(TAttFill *fill_att) {
    int pidx = source->palette_idx;
    fill_att->SetFillColor(gStyle->GetColorPalette(pidx));
}
void Histogram::setMarkAtt(TAttMarker *mark_att) {
    int pidx = source->palette_idx;
    mark_att->SetMarkerColor(gStyle->GetColorPalette(pidx));
}
void Histogram::setLineAtt(TAttLine *line_att) {
    int pidx = source->palette_idx;
    line_att->SetLineColor(gStyle->GetColorPalette(pidx));
}

void bindPlotElements(sol::state &lua) {}

Stack::Stack(const std::vector<DataSource *> s, THStack *h)
    : sources{s}, hist{h} {}

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
        auto pidx = sources[i]->palette_idx;
        h->SetFillColor(gStyle->GetColorPalette(pidx));
        h->SetMarkerColor(gStyle->GetColorPalette(pidx));
        h->SetLineColor(gStyle->GetColorPalette(pidx));
        ++i;
    }
    hist->Draw((s + " hist").c_str());
}
std::string Stack::to_string() const { return sources[0]->name; }
float Stack::getMin() {
    auto xaxis = getXAxis();
    return xaxis->GetBinLowEdge(xaxis->GetFirst());
}
float Stack::getMax() {
    auto xaxis = getXAxis();
    return xaxis->GetBinUpEdge(xaxis->GetLast());
}
