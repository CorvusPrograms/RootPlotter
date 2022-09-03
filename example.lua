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
base = "../RPVResearch/data/08_15_2022_FixedBackground/"
x = DataSource.create(base .. "2018_RPV2W_mS-850_mB-0.root")


rpv8 = DataSource.create(base .. "2018_RPV2W_mS-850_mB-0.root"):name("RPV 850"):palette_idx(200)
rpv4 = DataSource.create(base .. "2018_RPV2W_mS-450_mB-0.root"):name("RPV 450"):palette_idx(50)
rpv6 = DataSource.create(base .. "2018_RPV2W_mS-650_mB-0.root"):name("RPV 650"):palette_idx(100)
tt = DataSource.create(base .. "2018_TT.root"):name("TT"):palette_idx(300)
qcd = DataSource.create(base .. "2018_QCD.root"):name("QCD"):palette_idx(400)


sig = SourceSet.new({rpv4,rpv6,rpv8})
bkg = SourceSet.new({qcd,tt})

function MD(...)
   return InputData:new(...)
end

function simple_plot(args)
   local pattern = args[1]
   local data = args[2]
   local t = data[1]:source_set():get_keys()
   x = expand_data(data, t, args[1])
   ret = {}
   for k,v in ipairs(x) do
      pad = make_pad()
      table.insert(
         ret,
         {v.captures,
          simple(pad,
                 finalize_input_data(v.inputs),
                 create_options(args.opts))})
   end
   return ret
end

function ratio(args)
   local pattern = args[1]
   local data = args[2]
   x = expand_data(data, data[1].source:get_keys(), pattern)
   ret = {}
   for k,v in ipairs(x) do
      pad = make_pad()
      final=finalize_input_data(v.inputs)
      table.insert(ret, {v.captures, ratio_plot(pad, final[2], final[1])})
   end
   return ret
end

function datamc_ratio(args)
   local pattern = args[1]
   local data = args[2]
   x = expand_data(data, data[1]:source_set():get_keys(), pattern)
   ret = {}
   for k,v in ipairs(x) do
      pad = make_pad();
      plotpad.divide(pad, 1,2,0,0,0)
      top = plotpad.cd(pad,1)
      bot = plotpad.cd(pad,2)

      plotpad.rect(top, 0, 0.3, 1, 1)
      plotpad.m_top(top, 0.1)
      plotpad.m_bot(top, 0)
      plotpad.m_right(top, 0.05)

      plotpad.rect(bot, 0, 0, 1, 0.3)
      plotpad.m_bot(bot, 0.25)
      plotpad.m_top(bot, 0)
      plotpad.m_right(bot, 0.05)

      final=finalize_input_data(v.inputs)

      simple(top, final,  create_options(args.opts))
      for i=2,#final do
         ratio_plot(bot, final[i], final[1], create_options{table.unpack(args.opts)})
      end
      plotpad.update(pad)
      table.insert(ret, {v.captures, pad})
   end
   return ret
end



options = {}

function plot(args)
   local fun=args[1]
   table.remove(args, 1)
   ret = fun(args)
   for k , v in ipairs(ret) do
      captures = v[1]
      name = captures.HISTNAME
      save_name = options.outdir ..  captures.HISTNAME .. ".pdf"
      plotpad.save(v[2], save_name)
   end
end



options.outdir = "out1/"
-- plot{ratio, "met" .. "_*Lep",
--      {MD(sig, false ,3,false) , MD(sig, false ,3,false)}
options.outdir = "out2/"


toplot = {"nbjets_medium", "met", "WPt", "WPhi"}
for k,v in pairs(toplot) do
   plot{datamc_ratio,  v .. "_*Lep",
        {
           InputData:new(bkg):normalize(true):norm_to(10):stack(true):xrange(1,2),
           InputData:new(sig):normalize(true):norm_to(10):xrange(1,2)
        }, opts={xlabel=v, ylabel="Events", xrange={0,100,100}, title="HEHEHHEHEHE"}
   }
end
