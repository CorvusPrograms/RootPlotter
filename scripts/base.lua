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

function overwrite_table(t1,t2)
   local newtable = {}
   for k,v in pairs(t1) do
      newtable[k] = v
   end
   for k,v in pairs(t2) do
      newtable[k] = v
   end
   return newtable
end


function MD(...)
   return InputData:new(...)
end

function simple_plot(args)
   local pattern = args[1]
   local data = args[2]
   x = expand_data(data, args[1])
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
   x = expand_data(data,  pattern)
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
   x = expand_data(data, pattern)
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
      for i=2,#final do
         ratio_plot(bot, final[i], final[1],
                    create_options(
                       overwrite_table(args.opts,
                                       {
                                          title="", ylabel="Data/MC", yrange={0,1.5}}))
         )
      end
      simple(top, final,  create_options(overwrite_table(args.opts, {xlabel=nil})))
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
      save_name = args.outdir ..  captures.HISTNAME .. ".pdf"
      plotpad.save(v[2], save_name)
   end
end


NEED_TO_PLOT = {}

function plot(args)
   table.insert(NEED_TO_PLOT, args)
end

function execute_deferred_plots()
   total = 0
   start_time = os.time()
   print(string.format("Executing %d plot groups.", #NEED_TO_PLOT))
   for i , args in ipairs(NEED_TO_PLOT) do
      local fun=args[1]
      table.remove(args, 1)
      ret = fun(args)
      for k , v in ipairs(ret) do
         total = total + 1
         captures = v[1]
         io.write("                                                                            \r")
         io.write(string.format("Plot [%d:%d/%d], currently plotting histogram %s\r", total, i , #NEED_TO_PLOT, captures.HISTNAME))
         io.flush()
         name = captures.HISTNAME
         save_name = args.outdir ..  captures.HISTNAME .. ".pdf"
         plotpad.save(v[2], save_name)
      end
   end
   io.write("                                                                            \r")
   io.write(string.format("Generated %d plots in %d seconds.\n", total, os.time() - start_time))
end
