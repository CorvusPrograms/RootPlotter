print("HERE")
base = "/export/scratch/Research/rpvsusy/data/08_15_2022_FixedBackground/"
base = "../RPVResearch/data/08_15_2022_FixedBackground/"

rpv4 = DataSource:new(base .. "2018_RPV2W_mS-450_mB-0.root"):name("RPV 450"):
   style{
      palette_idx=200, marker_style=2, marker_size=1, fill_style=0, line_style=line_style.solid
   }
rpv8 = DataSource.new(base .. "2018_RPV2W_mS-850_mB-0.root"):name("RPV 850")--:palette_idx(200)
rpv6 = DataSource.new(base .. "2018_RPV2W_mS-650_mB-0.root"):name("RPV 650")--:palette_idx(300)
tt = DataSource.new(base .. "2018_TT.root"):name("t#bar{t}")--:palette_idx(500)
qcd = DataSource.new(base .. "2018_QCD.root"):name("QCD")--:palette_idx(600)

sig = SourceSet.new({rpv4,rpv6,rpv8})
bkg = SourceSet.new({tt,qcd})

my_palette = palettes.RainBow

toplot = {
   { "nbjets_medium", "NBjets Medium Working Point", nil},
   { "met", "MET", {0,300}},
}
for i=1,2 do
   for _ , v in pairs({{"Pt" , "p_{T}"}, {"E", "Energy"}, {"Eta", "#eta"}, {"Phi","#phi"}}) do
      table.insert( toplot,
                    { string.format("Jet_%d_" .. v[1], i),
                      string.format("Jet %d " .. v[2], i), nil})
   end
end


for k,v in pairs(toplot) do
   for _, n in pairs({true, false}) do
      indata =  {InputData:new(bkg):normalize(n):stack(true), InputData:new(sig):normalize(true)}
      add = n and "normed_" or ""
      plot{datamc_ratio, v[1] .. "_*Lep",  indata,
           opts={xlabel=v[2], ylabel="Events", title=v[2], palette=my_palette, xrange = v[3], yrange={1,0}, logy=false},
           outdir = string.format("plots/%sratio/", add)
      }
      plot{simple_plot, v[1] .. "_*Lep", indata
           ,opts={xlabel=v[2], ylabel="Events", title=v[2], palette=my_palette, xrange = v[3], yrange={0,0}, logy=false},
           outdir=string.format("plots/%sstack/", add)
      }
   --   plot{simple_plot, v[1] .. "_*Lep", {InputData:new(rpv4):normalize(true), InputData:new(qcd):normalize(true)},
   --        opts={xlabel=v[2], ylabel="Events", title=v[2], palette=my_palette, xrange = v[3]},
   --        outdir=string.format("plots/%stest/", add)
   --   }
   end
end


x= cutflow("EventCounter_", {"0Lep", "0Lep_HT>700_GenWPt<200"}, rpv4)

