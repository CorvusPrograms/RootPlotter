require "util"

NEED_TO_PLOT = {}


function co_wrapper(f, ... )
   local forward = {...} 
   local co = coroutine.create(function () f(table.unpack(forward)) end) 
   return function ()
      local status, res  = coroutine.resume(co)
      return res
    end
end

function sigbkg(tbl)
   hist_glob = tbl[1]
   sig = tbl[2]
   bkg = tbl[3]
   options = tbl[4]
   subpath = tbl[5]
   sig_set = get_histos(sig, hist_glob)
   bkg_set = get_histos(bkg, hist_glob)
   for key,set in pairs(sig_set) do
      transforms.sort_integral(bkg_set[key])
      dp = DrawPad:new()
      legend = plotting.new_legend(dp)
      plotting.stack(dp, 0 , bkg_set[key], options:plot_title(key))
      plotting.simple(dp, 0 , set, Options:new())
      plotting.add_to_legend(legend,  set)
      plotting.add_to_legend(legend,  bkg_set[key])
      plotting.add_legend_to_pad(legend, dp, 0);
      plotting.save_pad(dp, OUTPUT_BASE_PATH .. "/" .. subpath .. "/".. key .. ".pdf")
      coroutine.yield(key)
   end
end




function simple_hist(tbl)
   hist_glob = tbl[1]
   sig = tbl[2]
   normed=tbl[3] or 1
   options = tbl[4]
   subpath = tbl[5]
   sig_set = get_histos(sig, hist_glob)
   for key,set in pairs(sig_set) do
      if normed then
         to_plot = transforms.norm_to(set, normed)
      else 
         to_plot = set
      end
      dp = DrawPad:new()
      legend = plotting.new_legend(dp)
      plotting.simple(dp, 0 , to_plot, options:plot_title(key))
      plotting.add_to_legend(legend,  to_plot)
      plotting.add_legend_to_pad(legend, dp, 0);
      plotting.save_pad(dp, OUTPUT_BASE_PATH .. "/" .. subpath .. "/" .. key .. ".pdf")
      coroutine.yield(key)
   end
end


function plot(args)
   table.insert(NEED_TO_PLOT, args)
end

function execute_deferred_plots()
   VLOG(1, "Starting execution of deferred plots")
   total = 0
   group = 0
   start_time = os.time()
   io.write(string.format("Executing %d plot groups.\n", #NEED_TO_PLOT))
   for i , args in ipairs(NEED_TO_PLOT) do
      group = group + 1
      local fun=args[1]
      table.remove(args, 1)
      for key in co_wrapper(fun, args) do
         total = total + 1
         if VERBOSITY < 2 then
            io.write(string.rep(" ", 100) .. "\r")
         end
         collectgarbage()
         io.write(string.format("Plotting [%d:%d/%d] -- %s%s", total, group,  #NEED_TO_PLOT, key,  VERBOSITY <2 and '\r' or '\n'))
         if VERBOSITY < 2 then
            io.flush()
         end
      end
      if VERBOSITY < 2 then
         io.write(string.rep(" ", 100) .. "\r")
      end
   end
   io.write(string.format("Generated %d plots in %d seconds.\n", total, os.time() - start_time))
end


colors = {
   new_color("#1f77b4"),
   new_color("#ff7f0e"),
   new_color("#2ca02c"),
   new_color("#d62728"),
   new_color("#9467bd"),
   new_color("#8c564b"),
   new_color("#e377c2"),
   new_color("#7f7f7f"),
   new_color("#bcbd22"),
   new_color("#17becf")
}

