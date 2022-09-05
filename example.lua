base = "/export/scratch/Research/rpvsusy/data/08_15_2022_FixedBackground/"
base = "../RPVResearch/data/08_15_2022_FixedBackground/"

rpv4 = DataSource.create(base .. "2018_RPV2W_mS-450_mB-0.root"):name("RPV 450"):palette_idx(100)
rpv8 = DataSource.create(base .. "2018_RPV2W_mS-850_mB-0.root"):name("RPV 850"):palette_idx(200)
rpv6 = DataSource.create(base .. "2018_RPV2W_mS-650_mB-0.root"):name("RPV 650"):palette_idx(300)
tt = DataSource.create(base .. "2018_TT.root"):name("TT"):palette_idx(500)
qcd = DataSource.create(base .. "2018_QCD.root"):name("QCD"):palette_idx(600)


sig = SourceSet.new({rpv4,rpv6,rpv8})
bkg = SourceSet.new({tt,qcd})

toplot = {
   { "nbjets_medium", "NBjets Medium Working Point", nil},
   { "met", "MET", {0,300}}
}

options.outdir = "plots/ratio/"
default_inputs =  {InputData:new(bkg):normalize(false):stack(true), InputData:new(sig):normalize(false)}

options.outdir = "plots/ratio/"

for k,v in pairs(toplot) do
   plot{simple_plot, v[1] .. "_*Lep", default_inputs
        , opts={
           xlabel=v[2],
           ylabel="Events",
           title=v[2],
           palette=palettes.RainBow,
           xrange = v[3],
           yrange={1,0},
           logy=true
        }
   }
end
