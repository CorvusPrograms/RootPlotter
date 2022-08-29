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
rpv4.name="RPV4"
rpv6 = DataSource:new(base .. "2018_RPV2W_mS-650_mB-0.root")
rpv6.name="RPV6"
rpv8 = DataSource:new(base .. "2018_RPV2W_mS-850_mB-0.root")
rpv8.name="RPV8"

sig = SourceSet:new({rpv4,rpv6,rpv8});
bgk = SourceSet:new({qcd,tt});

function MD(...)
   return InputData:new(...)
end

function plot(args)
   local pattern = args[2]
   local data = args[3]
   x = expand_data(data, data[2].source:get_keys(), pattern)
   for k,v in ipairs(x) do
      print(v.captures.ALL)
      y = finalize_input_data(v.inputs)
      pad = simple(make_plot(), y)
      save = v.captures.ALL .. ".pdf"
      print(save)
     save_pad(pad, save)
   end
end

wannaplot = {"WPt", "CA12Pt", "HT_pt20", "met", "nbjets_loose"}

print("Starting loop")
for i,v in ipairs(wannaplot) do
   plot{simple, v .. "_*Lep",
        {MD(bgk), MD(sig) },
   }
end


