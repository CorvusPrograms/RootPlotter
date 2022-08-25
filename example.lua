function namegroup(match)
   match_idx={}
   idx = 1
   argmatcher = "%((%w+):(.-)%)"
   found,_, name = match:find(argmatcher)
   while found do
      match_idx[name] = idx
      idx = idx + 1
      found,_, name = match:find(argmatcher, found+1)
   end
   return match_idx, match:gsub(argmatcher, "(%2)")
end

function getnamed(s,match)
   local t,  m = namegroup(match)
   local found = {string.find(s,m)}
   found = {unpack(found,3)}
   local ret={}
   for k,v in pairs(t) do
      ret[k] = found[v]
   end
   return ret
end

function expand (s,t)
   s = string.gsub(s, "$(%w+)", function (n) return t[n] end)
   return s
end





base = "/export/scratch/Research/rpvsusy/data/08_15_2022_FixedBackground/"
rpv4 = DataSource:new(base .. "2018_RPV2W_mS-450_mB-0.root")
rpv6 = DataSource:new(base .. "2018_RPV2W_mS-650_mB-0.root")
rpv8 = DataSource:new(base .. "2018_RPV2W_mS-850_mB-0.root")

sig = SourceSet:new({rpv4,rpv6,rpv8});
bgk = SourceSet:new({qcd,tt});

function plot(args)
   print(plt)
   args[1](args)
end
print(simple)

plot{simple, "WPt_(lepnum:%d)Lep",
      data={bgk, sig, sig},
      title="Pt(W), $lepnum Leptons",
      xlabel="W(Pt)", ylabel="Events"
}

