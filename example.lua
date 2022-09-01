function dump(o)
   if type(o) == 'table' then
      local s = '{ '
      for k,v in pairs(o) do
         if type(k) ~= 'number' then k = '"'..k..'"' end
         s = s .. '['..k..'] = ' .. dump(v) .. ','
      end
      return s .. '} '
   else
      return tostring(o)
   end
end

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



x=1

base = "/export/scratch/Research/rpvsusy/data/08_15_2022_FixedBackground/"
rpv8 = DataSource:new(base .. "2018_RPV2W_mS-850_mB-0.root"):set_name("RPV 850"):set_pal(200)
rpv4 = DataSource:new(base .. "2018_RPV2W_mS-450_mB-0.root"):set_name("RPV 450"):set_pal(1)
rpv6 = DataSource:new(base .. "2018_RPV2W_mS-650_mB-0.root"):set_name("RPV 650"):set_pal(100)

tt = DataSource:new(base .. "2018_TT.root"):set_name("TT"):set_pal(300)
qcd = DataSource:new(base .. "2018_QCD.root"):set_name("QCD"):set_pal(400)

sig = SourceSet:new({rpv4,rpv6,rpv8})
bkg = SourceSet:new({qcd,tt})

function MD(...)
   return InputData:new(...)
end

function simple_plot(args)
   local pattern = args[1]
   local data = args[2]
   x = expand_data(data, data[1].source:get_keys(), pattern)
   ret = {}
   for k,v in ipairs(x) do
      table.insert(ret, {v.captures, simple(make_plot(), finalize_input_data(v.inputs), create_options({x=1, y=3}))})
   end
   return ret
end

function ratio(args)
   local pattern = args[1]
   local data = args[2]
   x = expand_data(data, data[1].source:get_keys(), pattern)
   ret = {}
   for k,v in ipairs(x) do
      final=finalize_input_data(v.inputs)
      table.insert(ret, {v.captures, ratio_plot(make_plot(), final[2], final[1])})
   end
   return ret
end


options = {}

function plot(args)
   local fun=args[1]
   table.remove(args, 1)
   print(dump(args))
   ret = fun(args)
   for k , v in ipairs(ret) do
      captures = v[1]
      name = captures.ALL
      save_name = options.outdir ..  captures.ALL .. ".pdf"
      save_pad(v[2], save_name)
   end
end

options.outdir = "out1/"
-- plot{ratio, "met" .. "_*Lep",
--      {MD(sig, false ,3,false) , MD(sig, false ,3,false)}
options.outdir = "out2/"
plot{simple_plot, "nbjets_medium" .. "_*Lep",
     {
        MD(sig, true , 1,false),
        MD(bkg, true, 1, false)
     }
     , opts={}
}


