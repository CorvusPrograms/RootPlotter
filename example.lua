base = "../RPVResearch/test/AllSamplesAK15/CONDOR_RPV_OUT/"

rpv4 = DataSource:new(base .. "2018_RPV2W_mS-450_mB-0.root"):name("RPV 450"):style{color=1}
rpv8 = DataSource.new(base .. "2018_RPV2W_mS-850_mB-0.root"):name("RPV 850"):style{color=2}
rpv6 = DataSource.new(base .. "2018_RPV2W_mS-650_mB-0.root"):name("RPV 650"):style{color=3}
tt = DataSource.new(base .. "2018_TT_mad.root"):name("t#bar{t}_mad"):style{color=4, mode = 4}
qcd = DataSource.new(base .. "2018_QCD.root"):name("QCD"):style{color=5, mode = 4}

bkg = SourceSet.new({tt, qcd})
sig = SourceSet.new({rpv4,rpv6,rpv8})



plot{sigbkg, "NJets_pt30*", sig, bkg, Options:new():logy(true):y_label("Weight Events")}
plot{sigbkg, "NJets_pt20*", sig, bkg, Options:new():logy(true):y_label("Weight Events")}
plot{sigbkg, "nbjets*", sig, bkg, Options:new():logy(true):y_label("Weight Events")}

execute_deferred_plots()

