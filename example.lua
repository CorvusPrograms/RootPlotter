base = "/export/scratch/Research/rpvsusy/data/08_15_2022_FixedBackground/"
base = "../RPVResearch/data/08_15_2022_FixedBackground/"

rpv4 = DataSource.create(base .. "2018_RPV2W_mS-450_mB-0.root"):name("RPV 450"):palette_idx(100)
rpv8 = DataSource.create(base .. "2018_RPV2W_mS-850_mB-0.root"):name("RPV 850"):palette_idx(200)
rpv6 = DataSource.create(base .. "2018_RPV2W_mS-650_mB-0.root"):name("RPV 650"):palette_idx(300)
tt = DataSource.create(base .. "2018_TT.root"):name("TT"):palette_idx(500)
qcd = DataSource.create(base .. "2018_QCD.root"):name("QCD"):palette_idx(600)


sig = SourceSet.new({rpv4,rpv6,rpv8})
bkg = SourceSet.new({tt,qcd})

toplot = {"nbjets_medium"}

options.outdir = "plots/ratio/"
-- for k,v in pairs(toplot) do
--    plot{datamc_ratio, v .. "_*Lep",
--         {
--            InputData:new(bkg):normalize(true):norm_to(10):stack(true):xrange(1,2),
--            InputData:new(sig):normalize(true):norm_to(10):xrange(1,2)
--         }, opts={xlabel="MET", ylabel="Events", title="Missing Transverse Momentum", palette=palettes.RainBow}
--    }
-- end
for k,v in pairs(toplot) do
   options.outdir = "plots/ratio/"
   plot{datamc_ratio, v .. "_*Lep",
        {
           InputData:new(bkg):normalize(false):stack(true),
           InputData:new(sig):normalize(false),
        }, opts={
           xlabel=v,
           ylabel="Events",
           title=v,
           palette=palettes.RainBow,
           yrange={1,10e14},
           logy=true
                }
   }
end
