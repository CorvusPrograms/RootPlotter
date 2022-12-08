base = "../RPVResearch/test/AllSamplesAK15/CONDOR_RPV_OUT/"
print("HERE")

rpv4 = DataSource:new(base .. "2018_RPV2W_mS-450_mB-0.root"):name("RPV 450"):style{palette_idx=0}
rpv8 = DataSource.new(base .. "2018_RPV2W_mS-850_mB-0.root"):name("RPV 850"):style{palette_idx=100}
rpv6 = DataSource.new(base .. "2018_RPV2W_mS-650_mB-0.root"):name("RPV 650"):style{color=1}
tt = DataSource.new(base .. "2018_TT_mad.root"):name("t#bar{t}_mad"):style{palette_idx=300}
qcd = DataSource.new(base .. "2018_QCD.root"):name("QCD"):style{palette_idx=500}

bkg = SourceSet.new({tt, qcd})
sig = SourceSet.new({rpv4,rpv6,rpv8})
print(bkg)

test = { hello="yes"}
prt(test)
