#include <TFile.h>
#include <iostream>
#include <TH1D.h>
#include <TCanvas.h>
#include <TCanvas.h>
#include <TGaxis.h>
#include <TStyle.h>
#include <TLegend.h>
#include <TEfficiency.h>
#include <exception>


struct FormatInfo{
    std::string ratio_title="";
};



std::vector<TEfficiency*> getEfficiencies(T* h1, std::vector<T*> other_h){
    std::vector<TEfficiency*> ret;
    for(const auto& h: other_h){
        if(TEfficiency::CheckConsistency(h1, h)){
            ret.push_back(new TEfficiency(h1,h));
        } else {
            throw std::runtime_error("Passed histograms are not compatible");
        }
    }
    return ret;
}

template<typename T>
TCanvas* ratioPlotter(T* h1, std::vector<T*> other_h, FormatInfo& format){
    auto canvas = new TCanvas("Canvas", "Canvas", 800,800);

    gStyle->SetOptTitle(kFALSE);
    gStyle->SetOptStat(0);
    gStyle->SetPalette(kOcean);


    canvas->Divide(1,2,0,0);


    auto p1 = canvas->cd(1);
    p1->cd();
    p1->SetPad(0, 0.3f, 0.96f, 0.98f);
    p1->SetBottomMargin(0.0f);

    h1->SetStats(0);
    h1->Draw("SAME PLC PMC");
    h1->SetLineWidth(2);
    h1->GetYaxis()->SetTitleSize(20);
    h1->GetYaxis()->SetTitleFont(43);
    h1->GetYaxis()->SetTitleOffset(1.55);

    TAxis *axis = h1->GetYaxis();
    axis->ChangeLabel(1, -1, -1, -1, -1, -1, " ");
    axis->SetLabelFont(43); // Absolute font size in pixel (precision 3)
    axis->SetLabelSize(15);


    for(T* h: other_h){
        h->Draw("Same PLC");
        h->SetStats(0);
        h->SetLineWidth(2);
    }

    auto p2 = canvas->cd(2);
    p2->SetPad(0, 0.05, 0.96, 0.3);
    p2->SetTopMargin(0);
    p2->SetBottomMargin(0.2);


    /*
    for(T* h: other_h){
        auto div = (T*)h1->Clone();
        div->Divide(h);
        div->SetMaximum(1.35);
        div->SetMinimum(0.8);
        div->Draw("epSame");
        div->Sumw2();
        div->SetStats(0);
        // Ratio plot (h3) settings
        div->SetTitle(""); // Remove the ratio title
        div->GetYaxis()->SetTitle(format.ratio_title.c_str());
        div->GetYaxis()->SetNdivisions(505);
        div->GetYaxis()->SetTitleSize(20);
        div->GetYaxis()->SetTitleFont(43);
        div->GetYaxis()->SetTitleOffset(1.55);
        div->GetYaxis()->SetLabelFont(43); // Absolute font size in pixel (precision 3)
        div->GetYaxis()->SetLabelSize(15);
        div->GetXaxis()->SetTitleSize(20);
        div->GetXaxis()->SetTitleFont(43);
        div->GetXaxis()->SetTitleOffset(4.);
        div->GetXaxis()->SetLabelFont(43); // Absolute font size in pixel (precision 3)
        div->GetXaxis()->SetLabelSize(15);
    }
    */




    canvas->cd(1);
    auto legend = new TLegend(0.7,0.85, 1.0f,1.0f);
    legend->SetTextSize(0.05);

    legend->SetHeader("Legend","C"); // option "C" allows to center the header
    legend->AddEntry(h1, h1->GetTitle());
    for(T* h: other_h){
        legend->AddEntry(h, h->GetTitle());
    }
    legend->Draw();
    return canvas;
}

int main(){
    auto h1 = new TH1D("h1","h1", 100, -3, 3);
    auto h2 = new TH1D("h2","h2", 100, -3, 3);
    auto h3 = new TH1D("h3","h3", 100, -3, 3);
    FormatInfo f;
    f.ratio_title="Ratio";
    h1->FillRandom("gaus", 10000);
    h2->FillRandom("gaus", 10000);
    h3->FillRandom("gaus", 10000);
    auto ret = ratioPlotter(h1,{h2,h3},f);
    ret->SaveAs("test.pdf");
    return 0 ;
}

