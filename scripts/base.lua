require "cutflow"
require "util"

function simple_plot(args)
   VLOG(2, "Running simple plot with args %s", tprint(args))
   local pattern = args[1]
   local data = args[2]
   x = expand_data(data, args[1])
   ret = {}
   for k,v in ipairs(x) do
      elements, filled = finalize_input_data(v.inputs)
      if filled then
         pad = make_pad()
         table.insert(
            ret,
            {v.captures,
             simple(pad, elements,
                    create_options(overwrite_table(args.opts or {}, {title=v.captures.HISTNAME})
                    )
         )})
      end
   end
   return ret
end

function ratio(args)
   VLOG(2, "Running ratio plot with args %s", tprint(args))
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
   VLOG(2, "Running datamc_ratio plot with args %s", tprint(args))
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
      final, filled =finalize_input_data(v.inputs)
      if filled then
         simple(top, final,
                create_options(overwrite_table(args.opts or {}, {title=v.captures.HISTNAME, xlabel=nil})))
         for i=2,#final do
            ratio_plot(bot, final[i], final[1],
                       create_options(
                          overwrite_table(
                             args.opts or {},
                             {title="",
                              ylabel="Data/MC",
                              yrange={0,1.5}}))
            )
         end
         plotpad.update(pad)
         table.insert(ret, {v.captures, pad})
      end
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
   VLOG(1, "Starting execution of deferred plots")
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
         if VERBOSITY < 2 then
            io.write(string.rep(" ", 100) .. "\r")
         end
         io.write(string.format("Plot [%d:%d/%d], currently plotting histogram %s%s", total, i , #NEED_TO_PLOT, captures.HISTNAME,  VERBOSITY <2 and '\r' or '\n'))
         if VERBOSITY < 2 then
            io.flush()
         end
         name = captures.HISTNAME
         save_name = args.outdir ..  captures.HISTNAME .. ".pdf"
         plotpad.save(v[2], save_name)
      end
   end
   if VERBOSITY < 2 then
      io.write(string.rep(" ", 100) .. "\r")
   end
   io.write(string.format("Generated %d plots in %d seconds.\n", total, os.time() - start_time))
end


