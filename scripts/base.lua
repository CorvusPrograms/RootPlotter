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
         pad = Pad:new()
         simple(pad, elements,
                create_options(overwrite_table(args.opts or {}, {title=v.captures.HISTNAME})))
         table.insert(
            ret,
            {v.captures, pad}
         )
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
      pad = Pad:new()
      final=finalize_input_data(v.inputs)
      ratio_plot(pad, final[2], final[1])
      table.insert(ret, {v.captures, pad })
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
      final, filled =finalize_input_data(v.inputs)
      if filled then
         pad = Pad:new();
         pad:divide(1,2)
         top = pad:get_subpad(1)
         bot = pad:get_subpad(2)

         top:set_rect(0, 0.3, 1, 1)
         top:set_margin_top(0.1)
         top:set_margin_bot(0)
         top:set_margin_right(0.05)

         bot:set_rect(0, 0, 1, 0.3)
         bot:set_margin_bot(0.25)
         bot:set_margin_top(0)
         bot:set_margin_right(0.05)
         simple(top, final,
                create_options(overwrite_table(args.opts or {}, {title=v.captures.HISTNAME, xlabel=nil})))
         for i=2,#final do
          --  ratio_plot(bot, final,
          --             create_options(
          --                overwrite_table(
          --                   args.opts or {},
          --                   {title="",
          --                    ylabel="Data/MC",
          --                    yrange={0,1.5}}))
          -- )
         end
         pad:update()
         table.insert(ret, {v.captures, pad})
         collectgarbage()
      end
   end
   collectgarbage()
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
      v[2].save(BASE_OUTPUT_PATH .. "/" .. save_name)
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
   collectgarbage()
   for i , args in ipairs(NEED_TO_PLOT) do
      local fun=args[1]
      table.remove(args, 1)
      ret = fun(args)
      collectgarbage()
      for k , v in ipairs(ret) do
         total = total + 1
         captures = v[1]
         if VERBOSITY < 2 then
            io.write("                                                                            \r")
         end
         collectgarbage()
         io.write(string.format("Plot [%d:%d/%d], currently plotting histogram %s%s", total, i , #NEED_TO_PLOT, captures.HISTNAME,  VERBOSITY <2 and '\r' or '\n'))
         if VERBOSITY < 2 then
            io.flush()
         end
         name = captures.HISTNAME
         save_name = args.outdir ..  captures.HISTNAME .. ".pdf"
         --   plotpad.save(v[2], save_name)
         v[2]:save(OUTPUT_BASE_PATH .. "/" .. save_name)
      end
   end
   if VERBOSITY < 2 then
      io.write("                                                                            \r")
   end
   io.write(string.format("Generated %d plots in %d seconds.\n", total, os.time() - start_time))
end


