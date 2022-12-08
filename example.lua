base = "../RPVResearch/test/AllSamplesAK15/CONDOR_RPV_OUT/"
print("HERE")

rpv4 = DataSource:new(base .. "2018_RPV2W_mS-450_mB-0.root"):name("RPV 450"):style{color=1}
rpv8 = DataSource.new(base .. "2018_RPV2W_mS-850_mB-0.root"):name("RPV 850"):style{color=2}
rpv6 = DataSource.new(base .. "2018_RPV2W_mS-650_mB-0.root"):name("RPV 650"):style{color=3}
tt = DataSource.new(base .. "2018_TT_mad.root"):name("t#bar{t}_mad"):style{color=4, mode = 4}
qcd = DataSource.new(base .. "2018_QCD.root"):name("QCD"):style{color=5, mode = 4}

bkg = SourceSet.new({tt, qcd})
sig = SourceSet.new({rpv4,rpv6,rpv8})
print(bkg)

test = get_histos(sig, "NJets_pt30")
test2 = get_histos(bkg, "NJets_pt30")

print(test)

for key,set in pairs(test) do
   dp = DrawPad:new()
   legend = plotting.new_legend(dp)

   plotting.stack(dp, 0 , test2[key], Options:new())
   plotting.simple(dp, 0 , set, Options:new())

   plotting.add_to_legend(legend,  set)
   plotting.add_to_legend(legend,  test2[key])
   plotting.add_legend_to_pad(legend, dp, 0);
   plotting.save_pad(dp, "testout/" .. key .. ".pdf")
   
end


test = { hello="yes"}
