base = "/export/scratch/Research/rpvsusy/data/08_15_2022_FixedBackground/"
base = "../RPVResearch/data/08_15_2022_FixedBackground/"
base = "../RPVResearch/test/AllSamplesAK15/CONDOR_RPV_OUT/"
print("HERE")

every_label={Text:new("#font[72]{CMS Preliminary}", 0.11,0.86):size(0.04), Text:new("Run II 2018", 0.75, 0.91):size(0.05)}

rpv4 = DataSource:new(base .. "2018_RPV2W_mS-450_mB-0.root"):name("RPV 450"):style{palette_idx=0}
rpv8 = DataSource.new(base .. "2018_RPV2W_mS-850_mB-0.root"):name("RPV 850"):style{palette_idx=100}
rpv6 = DataSource.new(base .. "2018_RPV2W_mS-650_mB-0.root"):name("RPV 650"):style{palette_idx=200}
tt = DataSource.new(base .. "2018_TT_mad.root"):name("t#bar{t}_mad"):style{palette_idx=300}
qcd = DataSource.new(base .. "2018_QCD.root"):name("QCD"):style{palette_idx=500}

bkg = SourceSet.new({tt, qcd})
sig = SourceSet.new({rpv4,rpv6,rpv8})

if add_stealth then
   base_out="data_background_plus_stealth"
   base = "data/09_22_22_StealthWithNewCuts/"
   glgl600 = DataSource:new(base .. "2018_stealth_sgetglgl_CN-600.root"):name("Gluon 600"):style{color=9}
   glgl925 = DataSource.new(base .. "2018_stealth_sgetglgl_CN-925.root"):name("Gluon 925"):style{color=8}
   bb600 = DataSource.new(base .. "2018_stealth_sgetbb_CN-600.root"):name("BB 600"):style{color=9}
   bb925 = DataSource.new(base .. "2018_stealth_sgetbb_CN-925.root"):name("BB 925"):style{color=8}
   sig = SourceSet.new({rpv4,rpv6,rpv8, glgl600, glgl925, bb600, bb925})
end




my_palette = palettes.RainBow

toplot_all = {
   {{true, true}, "nbjets_medium", "NBjets Medium Working Point pt30", nil},
--   {{true, true}, "NJets_pt20", "Njets p_{T}>20  ", nil},
--   {{true, true}, "NJets_pt30", "NJets p_{T}>30", nil},
--   {{true, true}, "AK8_NSubJettiness1", "AK8 NSubjetiness 1", nil},
--   {{true, true}, "AK8_NSubJettiness2", "AK8 NSubjetiness 2", nil},
--   {{true, true}, "AK8_NSubJettiness3", "AK8_NSubJettiness 3", nil},
--    {{true, true}, "AK15Pt", "AK15 Jet Pt", nil},
--    {{true, true}, "met", "MET", {0,300}},
--    {{true, true}, "AK15Pt",  "AK15Pt", nil } ,
--    {{true, true}, "RecoSBottomMass",  "RecoSBottomMass", nil } ,
--    {{true, true}, "RecoStopMass",  "RecoStopMass", nil } ,
--    {{true, false}, "RecoStopMassImbalance",  "RecoStopMassImbalance", nil } ,
--    {{true, false}, "RecoSBottomMassImbalance",  "RecoSBottomMassImbalance", nil } ,
--    {{true, true}, "met",  "met", nil } ,
--    {{true, true}, "nbjets_medium",  "nbjets_medium", nil } ,
--    {{true, true}, "nbjets_loose",  "nbjets_loose", nil } ,
--    {{true, true}, "nbjets_medium",  "nbjets_medium", nil } ,
--    {{true, true}, "DeepAK8TagW_medium_wp",  "DeepAK8TagW_medium_wp", nil } ,
--    {{true, true}, "DeepAK8TagW_pt",  "Deep Tag W Pt", nil } ,
-- --   {{true, true}, "WPhi",  "WPhi", nil } ,
--    {{true, true}, "StopPt",  "StopPt", nil } ,
--    {{true, true}, "nAK15Jets",  "nAK15Jets", nil } ,
--    {{true, true}, "AK15_NSubRatio21",  "AK15_NSubRatio21", nil } ,
--    {{true, true}, "AK15_NSubRatio43",  "AK15_NSubRatio43", nil } ,
--    {{true, true}, "AK15_NSubRatio42",  "AK15_NSubRatio42", nil } ,
--    {{true, true}, "AK15_NSubJettiness1",  "AK15_NSubJettiness1", nil } ,
--    {{true, true}, "AK15_NSubJettiness2",  "AK15_NSubJettiness2", nil } ,
--    {{true, true}, "AK15_NSubJettiness3",  "AK15_NSubJettiness3", nil } ,
--    {{true, true}, "AK15_NSubJettiness4",  "AK15_NSubJettiness4", nil } ,
--    {{true, true}, "AK15Pt",  "AK15Pt", nil } ,
--    {{true, true}, "GenSbottomChildrenDistanceAk4",  "GenSbottomChildrenDistanceAk4", nil } ,
--    {{true, true}, "GenWSbottomChildMaxDR",  "GenWSbottomChildMaxDR", nil } ,
--    {{true, true}, "DRGenSbottomChildren",  "DRGenSbottomChildren", nil } ,
--    {{true, true}, "DRGenWSbottom",  "DRGenWSbottom", nil } ,
--    {{true, true}, "GenSbottomChildrenTagWDistance",  "GenSbottomChildrenTagWDistance", nil } ,
--    {{true, true}, "GenStopNearestRecoStop",  "GenStopNearestRecoStop", nil } ,
--    {{true, true}, "HT_pt30",  "HT_pt30", nil } ,
--    {{true, true}, "WPt",  "WPt", nil } ,
-- --   {{true, true}, "WE",  "WE", nil } ,
--    {{true, true}, "NGoodLeptons",  "NGoodLeptons", {0,4} } ,
}

--for i=1,5 do
--   for _ , v in pairs({{"Pt" , "p_{T}"}}) do
--      table.insert( toplot_all,
--                    { {true, true}, string.format("Jet_%d_" .. v[1], i),
--                       string.format("Jet %d " .. v[2], i), nil})
--   end
--end





for k,v in pairs(toplot_all) do
   for _, n in pairs({true, false}) do
      lep_cut = v[1][2] and "_.Lep" or "_0Lep"
      add = n and "normed_" or ""
      plot{simple_plot, v[2] .. lep_cut,  {InputData:new(bkg):normalize(n):stack(true), InputData:new(sig):normalize(n)},
           opts={xlabel=v[3], ylabel="Events", palette=my_palette, xrange = v[4], logy=true, yrange={0.01,0}},
           outdir = string.format("%s/%sstack_bkg/", base_out, add)
      }
      plot{simple_plot, v[2] .. lep_cut, {InputData:new(sig):normalize(n)}
           ,opts={xlabel=v[3], ylabel="Events", palette=my_palette, xrange = v[4]},
           outdir=string.format("%s/%sstack/", base_out, add)
      }
   end
end



for k,v in pairs(toplot_all) do
   for _, c in ipairs({".MedW", ".BJet", "gte.Jet"}) do
      cut = "_.Lep_" .. c
      plot{simple_plot, v[2] .. cut,  {InputData:new(bkg):stack(true), InputData:new(sig)},
           opts={xlabel=v[3], ylabel="Events", palette=my_palette, xrange = v[4], logy=true, yrange={0.01,0}},
           outdir = string.format("%s/%sstack_bkg/", base_out, "")
      }
      plot{datamc_ratio, v[2] .. cut,  {InputData:new(bkg):stack(true), InputData:new(sig)},
           opts={xlabel=v[3], ylabel="Events", palette=my_palette, xrange = v[4], logy=true, yrange={0.01,0}},
           outdir = string.format("%s/%sdatamc_ratio/", base_out, "")
      }
   end
end

for k,v in pairs(toplot_all) do
   for _, partial_cut in pairs({".BJet_.MedW", "gte.Jet_.BJet", "gte.Jet_.MedW" ,"gte.Jet_.BJet_.MedW"}) do
      cut = "_.Lep_" .. partial_cut
   plot{simple_plot, v[2] .. cut,  {InputData:new(bkg):stack(true), InputData:new(sig)},
        opts={xlabel=v[3], ylabel="Events", palette=my_palette, xrange = v[4], logy=true, yrange={0.01,0}},
        outdir = string.format("%s/%sstack_bkg/", base_out, "")
   }
   plot{datamc_ratio, v[2] .. cut,  {InputData:new(bkg):stack(true), InputData:new(sig)},
        opts={xlabel=v[3], ylabel="Events", palette=my_palette, xrange = v[4], logy=true, yrange={0.01,0}},
        outdir = string.format("%s/%sdatamc_ratio/", base_out, "")
   }
   end
end

